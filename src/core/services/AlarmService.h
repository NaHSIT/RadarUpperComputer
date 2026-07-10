#ifndef ALARMSERVICE_H
#define ALARMSERVICE_H

#include <QObject>
#include <QList>
#include <QString>
#include "domain/AlarmEvent.h"

/**
 * @brief 告警管理服务
 *
 * 负责告警的生成、管理和查询
 */
class AlarmService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int alarmCount READ alarmCount NOTIFY alarmCountChanged)
    Q_PROPERTY(int activeAlarmCount READ activeAlarmCount NOTIFY alarmCountChanged)

public:
    explicit AlarmService(QObject *parent = nullptr);
    ~AlarmService() override;

    // 告警查询
    int alarmCount() const { return m_alarms.size(); }
    int activeAlarmCount() const;
    QList<AlarmEvent*> alarms() const { return m_alarms; }
    QList<AlarmEvent*> activeAlarms() const;

    // 告警操作
    Q_INVOKABLE void acknowledgeAlarm(const QString &alarmId, const QString &user);
    Q_INVOKABLE void resolveAlarm(const QString &alarmId);
    Q_INVOKABLE void clearResolvedAlarms();

    // 告警生成
    void raiseAlarm(AlarmSeverity severity, AlarmSource source,
                    const QString &code, const QString &title,
                    const QString &description = QString());

signals:
    void alarmRaised(AlarmEvent *alarm);
    void alarmAcknowledged(const QString &alarmId, const QString &user);
    void alarmResolved(const QString &alarmId);
    void alarmCountChanged(int count);

private:
    QList<AlarmEvent*> m_alarms;
};

#endif // ALARMSERVICE_H
