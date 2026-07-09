#include "DataService.h"
#include <algorithm>

DataService::DataService(QObject *parent)
    : QObject(parent)
    , m_currentProfile(new WindProfile(this))
    , m_deviceHealth(new DeviceHealth(this))
    , m_cleanupTimer(new QTimer(this))
    , m_historyMinutes(60)
{
    // 定期清理历史数据
    connect(m_cleanupTimer, &QTimer::timeout, this, &DataService::onHistoryCleanup);
    m_cleanupTimer->start(60000);  // 每分钟清理一次
}

DataService::~DataService()
{
    qDeleteAll(m_alarms);
    qDeleteAll(m_windHistory);
}

void DataService::updateWindProfile(WindProfile *profile)
{
    // 保存到历史记录
    auto *historyProfile = new WindProfile(this);
    historyProfile->fromJson(profile->toJson());
    m_windHistory.append(historyProfile);

    // 更新当前数据
    m_currentProfile->fromJson(profile->toJson());

    // 清理旧数据
    cleanupHistory();

    emit windProfileUpdated(m_currentProfile);
}

void DataService::updateDeviceHealth(DeviceHealth *health)
{
    m_deviceHealth->fromJson(health->toJson());
    emit deviceHealthUpdated(m_deviceHealth);
}

void DataService::addAlarm(AlarmEvent *alarm)
{
    m_alarms.prepend(alarm);
    emit alarmRaised(alarm);
}

QList<AlarmEvent*> DataService::activeAlarms() const
{
    QList<AlarmEvent*> active;
    for (const auto &alarm : m_alarms) {
        if (!alarm->isResolved()) {
            active.append(alarm);
        }
    }
    return active;
}

QVector<QPointF> DataService::windSpeedHistory(int minutes) const
{
    QVector<QPointF> history;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (minutes * 60 * 1000);

    for (const auto &profile : m_windHistory) {
        qint64 timestamp = profile->timestampUtc().toMSecsSinceEpoch();
        if (timestamp >= cutoffTime) {
            history.append(QPointF(timestamp, profile->hubWindSpeedMps()));
        }
    }

    return history;
}

QVector<double> DataService::windDirectionHistory(int minutes) const
{
    QVector<double> history;
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (minutes * 60 * 1000);

    for (const auto &profile : m_windHistory) {
        qint64 timestamp = profile->timestampUtc().toMSecsSinceEpoch();
        if (timestamp >= cutoffTime) {
            history.append(profile->hubWindDirectionDeg());
        }
    }

    return history;
}

void DataService::onHistoryCleanup()
{
    cleanupHistory();
}

void DataService::cleanupHistory()
{
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (m_historyMinutes * 60 * 1000);

    for (int i = m_windHistory.size() - 1; i >= 0; --i) {
        if (m_windHistory[i]->timestampUtc().toMSecsSinceEpoch() < cutoffTime) {
            delete m_windHistory.takeAt(i);
        }
    }
}
