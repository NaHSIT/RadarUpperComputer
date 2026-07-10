#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include <QObject>
#include <QVariantMap>
#include <QByteArray>
#include <QString>

#include "domain/RadarTypes.h"
#include "domain/RadarDevice.h"
#include "domain/WindProfile.h"
#include "domain/DeviceHealth.h"

class DeviceService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ConnectionState connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStateChanged)

public:
    explicit DeviceService(QObject *parent = nullptr);
    ~DeviceService() override;

    Q_INVOKABLE bool connectDevice(const QString &ip, int port);
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE bool reconnect();

    ConnectionState connectionState() const { return m_connectionState; }
    bool isConnected() const { return m_connectionState == ConnectionState::Online; }
    RadarDevice *deviceInfo() const { return m_deviceInfo; }
    WindProfile *currentWindProfile() const { return m_currentProfile; }
    DeviceHealth *deviceHealth() const { return m_deviceHealth; }

    Q_INVOKABLE QVariantMap getParameters() const;
    Q_INVOKABLE bool setParameters(const QVariantMap &params);
    Q_INVOKABLE bool validateParameters(const QVariantMap &params) const;

signals:
    void connectionStateChanged(ConnectionState state);
    void windProfileUpdated(WindProfile *profile);
    void deviceHealthUpdated(DeviceHealth *health);
    void errorOccurred(const QString &error);

private slots:
    void onBytesReceived(const QByteArray &data);
    void onConnectionError(const QString &error);

private:
    void processFrame(const QByteArray &frame);
    void updateConnectionState(ConnectionState state);

    ConnectionState m_connectionState;
    RadarDevice *m_deviceInfo;
    WindProfile *m_currentProfile;
    DeviceHealth *m_deviceHealth;
    class TcpDataSource *m_dataSource;
    int m_port;
};

#endif // DEVICESERVICE_H
