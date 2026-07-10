#include "DeviceService.h"

#include "communication/TcpDataSource.h"

#include <QDebug>
#include <QtEndian>

#include <cstring>

namespace {

bool readFloatLE(const QByteArray &data, int *offset, float *value)
{
    if (*offset < 0 || data.size() - *offset < static_cast<int>(sizeof(float))) return false;
    quint32 bits = 0;
    std::memcpy(&bits, data.constData() + *offset, sizeof(bits));
    bits = qFromLittleEndian(bits);
    std::memcpy(value, &bits, sizeof(*value));
    *offset += sizeof(float);
    return true;
}

bool readUInt64LE(const QByteArray &data, int *offset, quint64 *value)
{
    if (*offset < 0 || data.size() - *offset < static_cast<int>(sizeof(quint64))) return false;
    std::memcpy(value, data.constData() + *offset, sizeof(*value));
    *value = qFromLittleEndian(*value);
    *offset += sizeof(quint64);
    return true;
}

}

DeviceService::DeviceService(QObject *parent)
    : QObject(parent)
    , m_connectionState(ConnectionState::Offline)
    , m_deviceInfo(new RadarDevice(this))
    , m_currentProfile(new WindProfile(this))
    , m_deviceHealth(new DeviceHealth(this))
    , m_dataSource(new TcpDataSource(this))
    , m_frameParser(new FrameParser(this))
    , m_sequence(1)
    , m_port(5000)
{
    connect(m_dataSource, &TcpDataSource::stateChanged,
            this, &DeviceService::updateConnectionState);
    connect(m_dataSource, &TcpDataSource::bytesReceived,
            this, &DeviceService::onBytesReceived);
    connect(m_dataSource, &TcpDataSource::errorOccurred,
            this, &DeviceService::onConnectionError);
    connect(m_frameParser, &FrameParser::errorOccurred,
            this, &DeviceService::onConnectionError);
}

DeviceService::~DeviceService()
{
    disconnectDevice();
}

bool DeviceService::connectDevice(const QString &ip, int port)
{
    if (m_connectionState == ConnectionState::Connecting) {
        emit errorOccurred(QStringLiteral("设备正在连接，请等待当前连接操作完成。"));
        return false;
    }
    if (ip.isEmpty() || port <= 0 || port > 65535) {
        emit errorOccurred(QStringLiteral("设备 IP 地址或 TCP 端口无效。"));
        return false;
    }

    m_port = port;
    m_deviceInfo->setIpAddress(ip);
    return m_dataSource->connectToHost(ip, static_cast<quint16>(port));
}

void DeviceService::disconnectDevice()
{
    m_dataSource->disconnectFromHost();
}

bool DeviceService::reconnect()
{
    if (m_connectionState != ConnectionState::Offline) return false;
    return connectDevice(m_deviceInfo->ipAddress(), m_port);
}

QVariantMap DeviceService::getParameters() const
{
    qWarning() << "DeviceService: parameter read protocol is not available";
    return {};
}

bool DeviceService::setParameters(const QVariantMap &params)
{
    if (!validateParameters(params)) return false;
    emit errorOccurred(QStringLiteral("设备参数写入协议尚未接入。"));
    return false;
}

bool DeviceService::validateParameters(const QVariantMap &params) const
{
    Q_UNUSED(params)
    return true;
}

void DeviceService::onBytesReceived(const QByteArray &data)
{
    processFrame(data);
}

void DeviceService::onConnectionError(const QString &error)
{
    emit errorOccurred(error);
    if (m_connectionState == ConnectionState::Online) {
        updateConnectionState(ConnectionState::ProtocolError);
    } else if (m_connectionState == ConnectionState::Connecting
               || m_connectionState == ConnectionState::DataTimeout) {
        updateConnectionState(ConnectionState::Offline);
    }
}

void DeviceService::processFrame(const QByteArray &frame)
{
    const auto frames = m_frameParser->parse(frame);
    for (const Frame &received : frames) {
        switch (received.commandCode()) {
        case CommandCode::ResponseSuccess:
            break;
        case CommandCode::PushWindProfile:
            if (updateWindProfile(received.payload)) {
                emit windProfileUpdated(m_currentProfile);
            } else {
                emit errorOccurred(QStringLiteral("风场数据帧格式无效"));
            }
            break;
        case CommandCode::PushDeviceHealth:
            emit deviceHealthUpdated(m_deviceHealth);
            break;
        case CommandCode::ResponseError:
            emit errorOccurred(QStringLiteral("雷达端返回命令错误"));
            break;
        default:
            break;
        }
    }
}

bool DeviceService::updateWindProfile(const QByteArray &payload)
{
    constexpr int kFixedHeaderSize = 8 + 1 + 2 + 4 + 4 + 3;
    if (payload.size() < kFixedHeaderSize) return false;

    int offset = 0;
    quint64 timestampMs = 0;
    if (!readUInt64LE(payload, &offset, &timestampMs)) return false;

    const auto quality = static_cast<unsigned char>(payload.at(offset++));
    const int gateCount = (static_cast<unsigned char>(payload.at(offset)) << 8)
        | static_cast<unsigned char>(payload.at(offset + 1));
    offset += 2;

    float resolutionM = 0.0F;
    float maxRangeM = 0.0F;
    if (gateCount <= 0 || gateCount > 256
        || !readFloatLE(payload, &offset, &resolutionM)
        || !readFloatLE(payload, &offset, &maxRangeM)) return false;
    Q_UNUSED(maxRangeM)

    offset += 3;
    const int requiredSize = offset + gateCount * (static_cast<int>(sizeof(float)) * 3 + 1)
        + static_cast<int>(sizeof(float)) * 2;
    if (payload.size() < requiredSize) return false;

    QVector<float> speeds(gateCount), directions(gateCount), verticalSpeeds(gateCount);
    QVector<unsigned char> confidences(gateCount);
    for (float &speed : speeds) if (!readFloatLE(payload, &offset, &speed)) return false;
    for (float &direction : directions) if (!readFloatLE(payload, &offset, &direction)) return false;
    for (float &verticalSpeed : verticalSpeeds) if (!readFloatLE(payload, &offset, &verticalSpeed)) return false;
    for (unsigned char &confidence : confidences) confidence = static_cast<unsigned char>(payload.at(offset++));

    float snrDb = 0.0F;
    float turbulence = 0.0F;
    if (!readFloatLE(payload, &offset, &snrDb) || !readFloatLE(payload, &offset, &turbulence)) return false;

    m_currentProfile->setTimestampUtc(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(timestampMs), Qt::UTC));
    m_currentProfile->setTimeQuality(static_cast<TimeQuality>(quality));
    m_currentProfile->setRangeResolutionM(qRound(resolutionM));
    m_currentProfile->setGateCount(gateCount);
    m_currentProfile->setHubHeightM(resolutionM);
    m_currentProfile->setHubWindSpeedMps(speeds.first());
    m_currentProfile->setHubWindDirectionDeg(directions.first());
    m_currentProfile->setRawsMps(verticalSpeeds.first());
    m_currentProfile->clearRangeGates();

    for (int index = 0; index < gateCount; ++index) {
        auto *gate = new RangeGate(m_currentProfile);
        const double distanceM = (index + 1) * resolutionM;
        gate->setGateIndex(index);
        gate->setDistanceM(distanceM);
        gate->setHeightM(distanceM);
        gate->setWindSpeedMps(speeds.at(index));
        gate->setWindDirectionDeg(directions.at(index));
        gate->setRadialWindSpeedMps({verticalSpeeds.at(index)});
        gate->setCnrDb({snrDb});
        gate->setTurbulenceIntensity(turbulence);
        gate->setConfidence(confidences.at(index));
        gate->appendStatusCode(confidences.at(index) >= 50 ? StatusCode::Valid : StatusCode::LowConfidence);
        m_currentProfile->addRangeGate(gate);
    }

    emit m_currentProfile->dataUpdated();
    return true;
}

void DeviceService::updateConnectionState(ConnectionState state)
{
    if (m_connectionState == state) return;
    m_connectionState = state;
    m_deviceInfo->setConnectionState(state);
    emit connectionStateChanged(state);

    if (state == ConnectionState::Online) {
        m_dataSource->sendBytes(m_frameParser->buildFrame(
            CommandCode::QueryDeviceInfo, m_sequence++, QByteArray()));
        m_dataSource->sendBytes(m_frameParser->buildFrame(
            CommandCode::ControlStartMeasure, m_sequence++, QByteArray()));
        m_dataSource->sendBytes(m_frameParser->buildFrame(
            CommandCode::QueryWindProfile, m_sequence++, QByteArray()));
    }
}
