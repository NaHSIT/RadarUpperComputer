#include "DeviceService.h"
#include <QDebug>

DeviceService::DeviceService(QObject *parent)
    : QObject(parent)
    , m_connectionState(ConnectionState::Offline)
    , m_deviceInfo(new RadarDevice(this))
    , m_currentProfile(new WindProfile(this))
    , m_deviceHealth(new DeviceHealth(this))
    , m_port(1000)  // 默认端口
{
}

DeviceService::~DeviceService()
{
    disconnectDevice();
}

bool DeviceService::connectDevice(const QString &ip, int port)
{
    if (m_connectionState == ConnectionState::Connecting) {
        qWarning() << "DeviceService: Already connecting";
        return false;
    }

    if (ip.isEmpty() || port <= 0) {
        qWarning() << "DeviceService: Invalid IP or port";
        emit errorOccurred("Invalid IP or port");
        return false;
    }

    // 保存端口号用于重连
    m_port = port;

    updateConnectionState(ConnectionState::Connecting);
    m_deviceInfo->setIpAddress(ip);

    // TODO: 使用 TcpDataSource 连接设备
    // m_dataSource->connectToHost(ip, port);

    qWarning() << "DeviceService: connectDevice() not implemented yet";
    updateConnectionState(ConnectionState::Offline);
    emit errorOccurred("connectDevice() not implemented yet");
    return false;
}

void DeviceService::disconnectDevice()
{
    // TODO: 断开连接
    updateConnectionState(ConnectionState::Offline);
}

bool DeviceService::reconnect()
{
    if (m_connectionState == ConnectionState::Offline) {
        return connectDevice(m_deviceInfo->ipAddress(), m_port);
    }
    return false;
}

QVariantMap DeviceService::getParameters() const
{
    QVariantMap params;
    // TODO: 从设备读取参数
    qWarning() << "DeviceService: getParameters() not implemented yet";
    return params;
}

bool DeviceService::setParameters(const QVariantMap &params)
{
    if (!validateParameters(params)) {
        return false;
    }

    // TODO: 向设备写入参数
    qWarning() << "DeviceService: setParameters() not implemented yet";
    emit errorOccurred("setParameters() not implemented yet");
    return false;
}

bool DeviceService::validateParameters(const QVariantMap &params) const
{
    // TODO: 参数校验
    Q_UNUSED(params)
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
