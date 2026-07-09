#include "RadarDevice.h"

RadarDevice::RadarDevice(QObject *parent)
    : QObject(parent)
    , m_timeSource(TimeSource::NTP)
    , m_workMode(WorkMode::Standby)
    , m_connectionState(ConnectionState::Offline)
    , m_uptimeMs(0)
{
}

RadarDevice::~RadarDevice()
{
}

void RadarDevice::setDeviceId(const QString &id)
{
    if (m_deviceId != id) {
        m_deviceId = id;
        emit deviceInfoUpdated();
    }
}

void RadarDevice::setModel(const QString &model)
{
    if (m_model != model) {
        m_model = model;
        emit deviceInfoUpdated();
    }
}

void RadarDevice::setFirmwareVersion(const QString &version)
{
    if (m_firmwareVersion != version) {
        m_firmwareVersion = version;
        emit deviceInfoUpdated();
    }
}

void RadarDevice::setHardwareVersion(const QString &version)
{
    if (m_hardwareVersion != version) {
        m_hardwareVersion = version;
        emit deviceInfoUpdated();
    }
}

void RadarDevice::setIpAddress(const QString &ip)
{
    if (m_ipAddress != ip) {
        m_ipAddress = ip;
        emit deviceInfoUpdated();
    }
}

void RadarDevice::setNetmask(const QString &netmask)
{
    if (m_netmask != netmask) {
        m_netmask = netmask;
        emit deviceInfoUpdated();
    }
}

void RadarDevice::setGateway(const QString &gateway)
{
    if (m_gateway != gateway) {
        m_gateway = gateway;
        emit deviceInfoUpdated();
    }
}

void RadarDevice::setMacAddress(const QString &mac)
{
    if (m_macAddress != mac) {
        m_macAddress = mac;
        emit deviceInfoUpdated();
    }
}

void RadarDevice::setTimeSource(TimeSource source)
{
    if (m_timeSource != source) {
        m_timeSource = source;
        emit deviceInfoUpdated();
    }
}

void RadarDevice::setWorkMode(WorkMode mode)
{
    if (m_workMode != mode) {
        m_workMode = mode;
        emit workModeChanged(mode);
    }
}

void RadarDevice::setConnectionState(ConnectionState state)
{
    if (m_connectionState != state) {
        m_connectionState = state;
        emit connectionStateChanged(state);
    }
}

void RadarDevice::setUptimeMs(qint64 uptime)
{
    if (m_uptimeMs != uptime) {
        m_uptimeMs = uptime;
        emit deviceInfoUpdated();
    }
}

QJsonObject RadarDevice::toJson() const
{
    QJsonObject json;
    json["deviceId"] = m_deviceId;
    json["model"] = m_model;
    json["firmwareVersion"] = m_firmwareVersion;
    json["hardwareVersion"] = m_hardwareVersion;
    json["ipAddress"] = m_ipAddress;
    json["netmask"] = m_netmask;
    json["gateway"] = m_gateway;
    json["macAddress"] = m_macAddress;
    json["timeSource"] = static_cast<int>(m_timeSource);
    json["workMode"] = static_cast<int>(m_workMode);
    json["connectionState"] = static_cast<int>(m_connectionState);
    json["uptimeMs"] = m_uptimeMs;
    return json;
}

void RadarDevice::fromJson(const QJsonObject &json)
{
    m_deviceId = json["deviceId"].toString();
    m_model = json["model"].toString();
    m_firmwareVersion = json["firmwareVersion"].toString();
    m_hardwareVersion = json["hardwareVersion"].toString();
    m_ipAddress = json["ipAddress"].toString();
    m_netmask = json["netmask"].toString();
    m_gateway = json["gateway"].toString();
    m_macAddress = json["macAddress"].toString();
    m_timeSource = static_cast<TimeSource>(json["timeSource"].toInt());
    m_workMode = static_cast<WorkMode>(json["workMode"].toInt());
    m_connectionState = static_cast<ConnectionState>(json["connectionState"].toInt());
    m_uptimeMs = json["uptimeMs"].toVariant().toLongLong();

    emit deviceInfoUpdated();
}
