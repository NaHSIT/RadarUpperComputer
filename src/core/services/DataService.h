#ifndef DATASERVICE_H
#define DATASERVICE_H

#include <QObject>
#include <QList>
#include <QPointF>
#include <QSqlDatabase>
#include <QVector>

#include "domain/AlarmEvent.h"
#include "domain/DeviceHealth.h"
#include "domain/WindProfile.h"

class DataService : public QObject
{
    Q_OBJECT

public:
    struct WindSeries {
        QVector<QPointF> speedMps;
        QVector<QPointF> directionDeg;
    };

    explicit DataService(QObject *parent = nullptr);
    ~DataService() override;

    bool initialize(const QString &dataDirectory = QString());
    bool isDatabaseReady() const { return m_database.isOpen(); }
    QString databasePath() const { return m_databasePath; }

    void updateWindProfile(WindProfile *profile);
    void updateDeviceHealth(DeviceHealth *health);
    void addAlarm(AlarmEvent *alarm);

    WindProfile *currentWindProfile() const { return m_currentProfile; }
    DeviceHealth *deviceHealth() const { return m_deviceHealth; }
    QList<AlarmEvent *> activeAlarms() const;

    WindSeries queryWindSeries(int windowSeconds, int maximumPoints = 720) const;
    QVector<QPointF> windSpeedHistory(int minutes = 10) const;
    QVector<double> windDirectionHistory(int minutes = 10) const;

signals:
    void windProfileUpdated(WindProfile *profile);
    void deviceHealthUpdated(DeviceHealth *health);
    void alarmRaised(AlarmEvent *alarm);
    void alarmResolved(const QString &alarmId);
    void storageError(const QString &message);
    void historyStored(qint64 timestampMs);

private:
    bool createSchema();
    bool persistWindProfile(const WindProfile *profile);
    bool persistDeviceHealth(const DeviceHealth *health);
    bool persistAlarm(const AlarmEvent *alarm);
    QByteArray compressedJson(const QJsonObject &object) const;

    WindProfile *m_currentProfile;
    DeviceHealth *m_deviceHealth;
    QList<AlarmEvent *> m_alarms;
    QString m_connectionName;
    QString m_databasePath;
    QSqlDatabase m_database;
    qint64 m_lastStoredTimestampMs;
};

#endif // DATASERVICE_H
