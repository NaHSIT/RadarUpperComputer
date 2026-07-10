#ifndef DATASERVICE_H
#define DATASERVICE_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <QVector>
#include <QPointF>
#include <QString>
#include "domain/WindProfile.h"
#include "domain/DeviceHealth.h"
#include "domain/AlarmEvent.h"

/**
 * @brief 数据服务
 *
 * 管理实时数据和历史数据
 */
class DataService : public QObject
{
    Q_OBJECT

public:
    explicit DataService(QObject *parent = nullptr);
    ~DataService() override;

    // 数据更新
    void updateWindProfile(WindProfile *profile);
    void updateDeviceHealth(DeviceHealth *health);
    void addAlarm(AlarmEvent *alarm);

    // 数据查询
    WindProfile* currentWindProfile() const { return m_currentProfile; }
    DeviceHealth* deviceHealth() const { return m_deviceHealth; }
    QList<AlarmEvent*> activeAlarms() const;

    // 历史数据
    QVector<QPointF> windSpeedHistory(int minutes = 10) const;
    QVector<double> windDirectionHistory(int minutes = 10) const;

signals:
    void windProfileUpdated(WindProfile *profile);
    void deviceHealthUpdated(DeviceHealth *health);
    void alarmRaised(AlarmEvent *alarm);
    void alarmResolved(const QString &alarmId);

private slots:
    void onHistoryCleanup();

private:
    void cleanupHistory();

    WindProfile *m_currentProfile;
    DeviceHealth *m_deviceHealth;
    QList<AlarmEvent*> m_alarms;
    QList<WindProfile*> m_windHistory;
    QTimer *m_cleanupTimer;
    int m_historyMinutes;
};

#endif // DATASERVICE_H
