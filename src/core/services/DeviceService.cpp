#include "DeviceService.h"

DeviceService::DeviceService(QObject *parent)
    : QObject(parent)
    , m_connectionState(ConnectionState::Offline)
    , m_deviceInfo(new RadarDevice(this))
    , m_currentProfile(new WindProfile(this))
    , m_deviceHealth(new DeviceHealth(this))
{
}

DeviceService::~DeviceService()
{
    disconnectDevice();
}

bool DeviceService::connectDevice(const QString &ip, int port)
{
    if (m_connectionState == ConnectionState::Connecting) {
        return false;
    }

    updateConnectionState(ConnectionState::Connecting);

    // TODO: 使用 TcpDataSource 连接设备
    // m_dataSource->connectToHost(ip, port);

    return true;
}

void DeviceService::disconnectDevice()
{
    // TODO: 断开连接
    updateConnectionState(ConnectionState::Offline);
}

bool DeviceService::reconnect()
{
    if (m_connectionState == ConnectionState::Offline) {
        return connectDevice(m_deviceInfo->ipAddress(), 1000);
    }
    return false;
}

QVariantMap DeviceService::getParameters() const
{
    QVariantMap params;
    // TODO: 从设备读取参数
    return params;
}

bool DeviceService::setParameters(const QVariantMap &params)
{
    if (!validateParameters(params)) {
        return false;
    }

    // TODO: 向设备写入参数
    return true;
}

bool DeviceService::validateParameters(const QVariantMap &params) const
{
    // TODO: 参数校验
    return true;
}

void DeviceService::onBytesReceived(const QByteArray &data)
{
    // TODO: 解析数据帧
    processFrame(data);
}

void DeviceService::onConnectionError(const QString &error)
{
    emit errorOccurred(error);
    updateConnectionState(ConnectionState::ProtocolError);
}

void DeviceService::processFrame(const QByteArray &frame)
{
    // TODO: 处理数据帧
    Q_UNUSED(frame)
}

void DeviceService::updateConnectionState(ConnectionState state)
{
    if (m_connectionState != state) {
        m_connectionState = state;
        m_deviceInfo->setConnectionState(state);
        emit connectionStateChanged(state);
    }
}
