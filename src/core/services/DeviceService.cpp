#include "DeviceService.h"

#include "communication/TcpDataSource.h"

#include <QDebug>

DeviceService::DeviceService(QObject *parent)
    : QObject(parent)
    , m_connectionState(ConnectionState::Offline)
    , m_deviceInfo(new RadarDevice(this))
    , m_currentProfile(new WindProfile(this))
    , m_deviceHealth(new DeviceHealth(this))
    , m_dataSource(new TcpDataSource(this))
    , m_port(1000)
{
    connect(m_dataSource, &TcpDataSource::stateChanged,
            this, &DeviceService::updateConnectionState);
    connect(m_dataSource, &TcpDataSource::bytesReceived,
            this, &DeviceService::onBytesReceived);
    connect(m_dataSource, &TcpDataSource::errorOccurred,
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
    if (m_connectionState != ConnectionState::Offline) {
        updateConnectionState(ConnectionState::ProtocolError);
    }
}

void DeviceService::processFrame(const QByteArray &frame)
{
    Q_UNUSED(frame)
    // Frame parsing is introduced once the radar communication protocol is finalized.
}

void DeviceService::updateConnectionState(ConnectionState state)
{
    if (m_connectionState == state) return;
    m_connectionState = state;
    m_deviceInfo->setConnectionState(state);
    emit connectionStateChanged(state);
}
