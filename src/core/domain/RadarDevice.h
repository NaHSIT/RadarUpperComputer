#ifndef RADARDEVICE_H
#define RADARDEVICE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include "RadarTypes.h"

/**
 * @brief 雷达设备模型
 *
 * 描述雷达设备的基本信息和状态
 */
class RadarDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId)
    Q_PROPERTY(QString model READ model WRITE setModel)
    Q_PROPERTY(QString firmwareVersion READ firmwareVersion WRITE setFirmwareVersion)
    Q_PROPERTY(QString hardwareVersion READ hardwareVersion WRITE setHardwareVersion)
    Q_PROPERTY(QString ipAddress READ ipAddress WRITE setIpAddress)
    Q_PROPERTY(ConnectionState connectionState READ connectionState WRITE setConnectionState)
    Q_PROPERTY(WorkMode workMode READ workMode WRITE setWorkMode)

public:
    explicit RadarDevice(QObject *parent = nullptr);
    ~RadarDevice() override;

    // 属性访问器
    QString deviceId() const { return m_deviceId; }
    QString model() const { return m_model; }
    QString firmwareVersion() const { return m_firmwareVersion; }
    QString hardwareVersion() const { return m_hardwareVersion; }
    QString ipAddress() const { return m_ipAddress; }
    QString netmask() const { return m_netmask; }
    QString gateway() const { return m_gateway; }
    QString macAddress() const { return m_macAddress; }
    TimeSource timeSource() const { return m_timeSource; }
    WorkMode workMode() const { return m_workMode; }
    ConnectionState connectionState() const { return m_connectionState; }
    qint64 uptimeMs() const { return m_uptimeMs; }

    // 设置方法
    void setDeviceId(const QString &id);
    void setModel(const QString &model);
    void setFirmwareVersion(const QString &version);
    void setHardwareVersion(const QString &version);
    void setIpAddress(const QString &ip);
    void setNetmask(const QString &netmask);
    void setGateway(const QString &gateway);
    void setMacAddress(const QString &mac);
    void setTimeSource(TimeSource source);
    void setWorkMode(WorkMode mode);
    void setConnectionState(ConnectionState state);
    void setUptimeMs(qint64 uptime);

    // 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

signals:
    void connectionStateChanged(ConnectionState state);
    void workModeChanged(WorkMode mode);
    void deviceInfoUpdated();

private:
    QString m_deviceId;
    QString m_model;
    QString m_firmwareVersion;
    QString m_hardwareVersion;
    QString m_ipAddress;
    QString m_netmask;
    QString m_gateway;
    QString m_macAddress;
    TimeSource m_timeSource;
    WorkMode m_workMode;
    ConnectionState m_connectionState;
    qint64 m_uptimeMs;
};

#endif // RADARDEVICE_H
