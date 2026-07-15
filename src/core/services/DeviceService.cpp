#include "DeviceService.h"

#include "communication/TcpDataSource.h"

#include <QDebug>
#include <QtEndian>
#include <QtMath>

#include <cstring>
#include <algorithm>
#include <limits>

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

bool readUInt16LE(const QByteArray &data, int *offset, quint16 *value)
{
    if (*offset < 0 || data.size() - *offset < static_cast<int>(sizeof(quint16))) return false;
    std::memcpy(value, data.constData() + *offset, sizeof(*value));
    *value = qFromLittleEndian(*value);
    *offset += sizeof(*value);
    return true;
}

bool readInt16LE(const QByteArray &data, int *offset, qint16 *value)
{
    quint16 bits = 0;
    if (!readUInt16LE(data, offset, &bits)) return false;
    std::memcpy(value, &bits, sizeof(*value));
    return true;
}

bool readUInt32LE(const QByteArray &data, int *offset, quint32 *value)
{
    if (*offset < 0 || data.size() - *offset < static_cast<int>(sizeof(quint32))) return false;
    std::memcpy(value, data.constData() + *offset, sizeof(*value));
    *value = qFromLittleEndian(*value);
    *offset += sizeof(*value);
    return true;
}

bool readDoubleLE(const QByteArray &data, int *offset, double *value)
{
    quint64 bits = 0;
    if (!readUInt64LE(data, offset, &bits)) return false;
    std::memcpy(value, &bits, sizeof(*value));
    return true;
}

double meanFinite(const QVector<double> &values)
{
    double sum = 0.0;
    int count = 0;
    for (double value : values) {
        if (!qIsFinite(value)) continue;
        sum += value;
        ++count;
    }
    return count > 0 ? sum / count : qQNaN();
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
        case CommandCode::PushRadialRay:
            if (updateRadialScan(received.sequence, received.payload)) {
                emit windProfileUpdated(m_currentProfile);
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
    m_currentProfile->setRetrievalMethod(WindRetrievalMethod::LegacyProfile);
    m_currentProfile->setSourceScanId(0);
    m_currentProfile->clearRangeGates();
    m_currentProfile->clearBeamStates();

    for (int index = 0; index < gateCount; ++index) {
        auto *gate = new RangeGate(m_currentProfile);
        const double distanceM = (index + 1) * resolutionM;
        gate->setGateIndex(index);
        gate->setDistanceM(distanceM);
        gate->setHeightM(distanceM);
        gate->setWindSpeedMps(speeds.at(index));
        gate->setWindDirectionDeg(directions.at(index));
        const double directionRad = qDegreesToRadians(static_cast<double>(directions.at(index)));
        gate->setEastwardWindMps(-speeds.at(index) * qSin(directionRad));
        gate->setNorthwardWindMps(-speeds.at(index) * qCos(directionRad));
        gate->setUpwardWindMps(verticalSpeeds.at(index));
        gate->setValidBeamCount(0);
        gate->setRadialWindSpeedMps({});
        gate->setCnrDb({snrDb});
        gate->setTurbulenceIntensity(turbulence);
        gate->setConfidence(confidences.at(index));
        gate->appendStatusCode(confidences.at(index) >= 50 ? StatusCode::Valid : StatusCode::Invalid);
        m_currentProfile->addRangeGate(gate);
    }

    emit m_currentProfile->dataUpdated();
    return true;
}

bool DeviceService::updateRadialScan(quint32 scanId, const QByteArray &payload)
{
    constexpr int kHeaderSize = 56;
    if (payload.size() < kHeaderSize) return false;

    int offset = 0;
    quint16 rayIndex = 0;
    quint16 rayCount = 0;
    quint16 gateCount = 0;
    if (!readUInt16LE(payload, &offset, &rayIndex)
        || !readUInt16LE(payload, &offset, &rayCount)
        || !readUInt16LE(payload, &offset, &gateCount)) return false;
    if (rayCount == 0 || rayCount > 256 || rayIndex >= rayCount || gateCount == 0 || gateCount > 256) return false;

    const int beamId = static_cast<unsigned char>(payload.at(offset++));
    const int timeQuality = static_cast<unsigned char>(payload.at(offset++));
    Q_UNUSED(timeQuality)
    float azimuthDeg = 0.0F;
    float elevationDeg = 0.0F;
    float startRangeM = 0.0F;
    float gateSpacingM = 0.0F;
    float prfHz = 0.0F;
    float nyquistVelocityMps = 0.0F;
    double frequencyHz = 0.0;
    quint32 calibrationVersion = 0;
    quint32 fieldMask = 0;
    quint32 rayQualityFlags = 0;
    quint32 reserved = 0;
    if (!readFloatLE(payload, &offset, &azimuthDeg)
        || !readFloatLE(payload, &offset, &elevationDeg)
        || !readFloatLE(payload, &offset, &startRangeM)
        || !readFloatLE(payload, &offset, &gateSpacingM)
        || !readFloatLE(payload, &offset, &prfHz)
        || !readFloatLE(payload, &offset, &nyquistVelocityMps)
        || !readDoubleLE(payload, &offset, &frequencyHz)
        || !readUInt32LE(payload, &offset, &calibrationVersion)
        || !readUInt32LE(payload, &offset, &fieldMask)
        || !readUInt32LE(payload, &offset, &rayQualityFlags)
        || !readUInt32LE(payload, &offset, &reserved)) return false;
    Q_UNUSED(prfHz)
    Q_UNUSED(nyquistVelocityMps)
    Q_UNUSED(calibrationVersion)
    Q_UNUSED(rayQualityFlags)
    Q_UNUSED(reserved)
    if (beamId < 0 || beamId > 4 || !qIsFinite(azimuthDeg) || !qIsFinite(elevationDeg)
        || elevationDeg < 0.0F || elevationDeg > 90.0F || gateSpacingM <= 0.0F
        || !(fieldMask & 0x01U)) return false;

    BeamObservation observation;
    observation.beamId = beamId;
    observation.azimuthDeg = azimuthDeg;
    observation.elevationDeg = elevationDeg;
    observation.carrierFrequencyHz = frequencyHz;
    observation.startRangeM = startRangeM;
    observation.gateSpacingM = gateSpacingM;
    observation.radialVelocityMps = QVector<double>(gateCount, qQNaN());
    observation.cnrDb = QVector<double>(gateCount, qQNaN());
    observation.confidencePct = QVector<double>(gateCount, 0.0);

    for (int bit = 0; bit <= 6; ++bit) {
        if (!(fieldMask & (1U << bit))) continue;
        for (int gate = 0; gate < gateCount; ++gate) {
            if (bit == 0 || bit == 1 || bit == 3) {
                qint16 value = 0;
                if (!readInt16LE(payload, &offset, &value)) return false;
                if (bit == 0 && value != std::numeric_limits<qint16>::min()) {
                    observation.radialVelocityMps[gate] = value / 100.0;
                } else if (bit == 1 && value != std::numeric_limits<qint16>::min()) {
                    observation.cnrDb[gate] = value / 100.0;
                }
            } else if (bit == 2 || bit == 5) {
                quint16 value = 0;
                if (!readUInt16LE(payload, &offset, &value)) return false;
            } else if (bit == 4) {
                if (offset >= payload.size()) return false;
                const int value = static_cast<unsigned char>(payload.at(offset++));
                observation.confidencePct[gate] = value == 255 ? 0.0 : value;
            } else if (bit == 6) {
                float value = 0.0F;
                if (!readFloatLE(payload, &offset, &value)) return false;
            }
        }
    }

    QVector<BeamObservation> &scan = m_pendingRadialScans[scanId];
    auto existing = std::find_if(scan.begin(), scan.end(), [beamId](const BeamObservation &beam) {
        return beam.beamId == beamId;
    });
    if (existing == scan.end()) scan.append(observation);
    else *existing = observation;
    m_expectedRayCounts[scanId] = rayCount;

    if (m_pendingRadialScans.size() > 8) {
        m_pendingRadialScans.clear();
        m_expectedRayCounts.clear();
        return false;
    }
    if (scan.size() < rayCount) return false;

    const QVector<BeamObservation> completeScan = scan;
    m_pendingRadialScans.remove(scanId);
    m_expectedRayCounts.remove(scanId);
    return rayCount == 5 && applyFixedBeamRetrieval(scanId, completeScan);
}

bool DeviceService::applyFixedBeamRetrieval(quint32 scanId, const QVector<BeamObservation> &beams)
{
    const QVector<WindVectorLevel> levels = FiveBeamWindRetrieval::retrieve(beams);
    if (levels.isEmpty()) return false;

    m_currentProfile->setTimestampUtc(QDateTime::currentDateTimeUtc());
    m_currentProfile->setTimeQuality(TimeQuality::Synchronized);
    m_currentProfile->setGateCount(levels.size());
    m_currentProfile->setRetrievalMethod(WindRetrievalMethod::FiveBeamLeastSquares);
    m_currentProfile->setSourceScanId(scanId);
    m_currentProfile->clearRangeGates();
    m_currentProfile->clearBeamStates();

    for (const BeamObservation &observation : beams) {
        auto *beam = new BeamState(m_currentProfile);
        beam->setBeamId(static_cast<BeamId>(observation.beamId));
        beam->setAzimuthDeg(observation.azimuthDeg);
        beam->setElevationDeg(observation.elevationDeg);
        beam->setCarrierFrequencyHz(observation.carrierFrequencyHz);
        beam->setRwsByGate(observation.radialVelocityMps);
        beam->setCnrDbByGate(observation.cnrDb);
        beam->setConfidenceByGate(observation.confidencePct);
        const double averageCnr = meanFinite(observation.cnrDb);
        int finiteRadialCount = 0;
        for (double radial : observation.radialVelocityMps) {
            if (qIsFinite(radial)) ++finiteRadialCount;
        }
        const double completeness = observation.radialVelocityMps.isEmpty() ? 0.0
            : static_cast<double>(finiteRadialCount) / observation.radialVelocityMps.size();
        beam->setStatus(averageCnr >= -22.0 && completeness >= 0.95
            ? BeamStatus::Normal : BeamStatus::WeakSignal);
        beam->setLastUpdateTime(m_currentProfile->timestampUtc());
        m_currentProfile->addBeamState(beam);
    }

    int firstValidIndex = -1;
    for (const WindVectorLevel &level : levels) {
        auto *gate = new RangeGate(m_currentProfile);
        gate->setGateIndex(level.gateIndex);
        gate->setDistanceM(level.distanceM);
        gate->setHeightM(level.heightAglM);
        gate->setEastwardWindMps(level.eastwardMps);
        gate->setNorthwardWindMps(level.northwardMps);
        gate->setUpwardWindMps(level.upwardMps);
        gate->setWindSpeedMps(level.horizontalSpeedMps);
        gate->setWindDirectionDeg(level.windFromDirectionDeg);
        gate->setCnrDb({level.cnrMeanDb});
        gate->setConfidence(level.confidencePct);
        gate->setRetrievalResidualMps(level.residualRmsMps);
        gate->setValidBeamCount(level.validBeamCount);
        QVector<double> radial;
        for (const BeamObservation &beam : beams) {
            radial.append(level.gateIndex < beam.radialVelocityMps.size()
                ? beam.radialVelocityMps.at(level.gateIndex) : qQNaN());
        }
        gate->setRadialWindSpeedMps(radial);
        if (level.valid) gate->appendStatusCode(StatusCode::Valid);
        else if (level.validBeamCount < 3) gate->appendStatusCode(StatusCode::InsufficientBeams);
        else if (level.cnrMeanDb < -22.0) gate->appendStatusCode(StatusCode::LowCNR);
        else gate->appendStatusCode(StatusCode::Invalid);
        m_currentProfile->addRangeGate(gate);
        if (firstValidIndex < 0 && level.valid) firstValidIndex = level.gateIndex;
    }

    if (levels.size() > 1) {
        m_currentProfile->setRangeResolutionM(qRound(levels.at(1).heightAglM - levels.first().heightAglM));
    }
    const WindVectorLevel &representative = levels.at(firstValidIndex >= 0 ? firstValidIndex : 0);
    m_currentProfile->setHubHeightM(representative.heightAglM);
    m_currentProfile->setHubWindSpeedMps(representative.horizontalSpeedMps);
    m_currentProfile->setHubWindDirectionDeg(representative.windFromDirectionDeg);
    m_currentProfile->setRawsMps(representative.upwardMps);
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
        m_dataSource->sendBytes(m_frameParser->buildFrame(
            CommandCode::QueryRadialScan, m_sequence++, QByteArray()));
    }
}
