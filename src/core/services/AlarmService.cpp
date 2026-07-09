#include "AlarmService.h"

AlarmService::AlarmService(QObject *parent)
    : QObject(parent)
{
}

AlarmService::~AlarmService()
{
    qDeleteAll(m_alarms);
}

int AlarmService::activeAlarmCount() const
{
    int count = 0;
    for (const auto &alarm : m_alarms) {
        if (!alarm->isResolved()) {
            count++;
        }
    }
    return count;
}

QList<AlarmEvent*> AlarmService::activeAlarms() const
{
    QList<AlarmEvent*> active;
    for (const auto &alarm : m_alarms) {
        if (!alarm->isResolved()) {
            active.append(alarm);
        }
    }
    return active;
}

void AlarmService::acknowledgeAlarm(const QString &alarmId, const QString &user)
{
    for (auto &alarm : m_alarms) {
        if (alarm->alarmId() == alarmId) {
            alarm->acknowledge(user);
            emit alarmAcknowledged(alarmId, user);
            return;
        }
    }
}

void AlarmService::resolveAlarm(const QString &alarmId)
{
    for (auto &alarm : m_alarms) {
        if (alarm->alarmId() == alarmId) {
            alarm->resolve();
            emit alarmResolved(alarmId);
            emit alarmCountChanged(alarmCount());
            return;
        }
    }
}

void AlarmService::clearResolvedAlarms()
{
    for (int i = m_alarms.size() - 1; i >= 0; --i) {
        if (m_alarms[i]->isResolved()) {
            delete m_alarms.takeAt(i);
        }
    }
    emit alarmCountChanged(alarmCount());
}

void AlarmService::raiseAlarm(AlarmSeverity severity, AlarmSource source,
                               const QString &code, const QString &title,
                               const QString &description)
{
    auto *alarm = new AlarmEvent(this);
    alarm->setSeverity(severity);
    alarm->setSource(source);
    alarm->setCode(code);
    alarm->setTitle(title);
    alarm->setDescription(description);
    alarm->setFirstSeen(QDateTime::currentDateTimeUtc());
    alarm->setLastSeen(QDateTime::currentDateTimeUtc());

    m_alarms.prepend(alarm);

    emit alarmRaised(alarm);
    emit alarmCountChanged(alarmCount());
}
