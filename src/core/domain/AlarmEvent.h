#ifndef ALARMEVENT_H
#define ALARMEVENT_H

#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include "RadarTypes.h"

/**
 * @brief 告警事件模型
 *
 * 描述告警事件的详细信息
 */
class AlarmEvent : public QObject
{
    Q_OBJECT

public:
    explicit AlarmEvent(QObject *parent = nullptr);
    ~AlarmEvent() override;

    // 属性访问器
    QString alarmId() const { return m_alarmId; }
    AlarmSeverity severity() const { return m_severity; }
    AlarmSource source() const { return m_source; }
    QString code() const { return m_code; }
    QString title() const { return m_title; }
    QString description() const { return m_description; }
    QString recommendedAction() const { return m_recommendedAction; }
    QDateTime firstSeen() const { return m_firstSeen; }
    QDateTime lastSeen() const { return m_lastSeen; }
    QString acknowledgedBy() const { return m_acknowledgedBy; }
    QDateTime acknowledgedAt() const { return m_acknowledgedAt; }
    QDateTime resolvedAt() const { return m_resolvedAt; }
    QString relatedDataFrameId() const { return m_relatedDataFrameId; }
    bool isAcknowledged() const { return !m_acknowledgedBy.isEmpty(); }
    bool isResolved() const { return m_resolvedAt.isValid(); }

    // 设置方法
    void setAlarmId(const QString &id);
    void setSeverity(AlarmSeverity severity);
    void setSource(AlarmSource source);
    void setCode(const QString &code);
    void setTitle(const QString &title);
    void setDescription(const QString &description);
    void setRecommendedAction(const QString &action);
    void setFirstSeen(const QDateTime &time);
    void setLastSeen(const QDateTime &time);
    void setRelatedDataFrameId(const QString &id);

    // 操作
    void acknowledge(const QString &user);
    void resolve();
    void updateLastSeen();

    // 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

signals:
    void acknowledged(const QString &user);
    void resolved();
    void updated();

private:
    QString m_alarmId;
    AlarmSeverity m_severity;
    AlarmSource m_source;
    QString m_code;
    QString m_title;
    QString m_description;
    QString m_recommendedAction;
    QDateTime m_firstSeen;
    QDateTime m_lastSeen;
    QString m_acknowledgedBy;
    QDateTime m_acknowledgedAt;
    QDateTime m_resolvedAt;
    QString m_relatedDataFrameId;
};

#endif // ALARMEVENT_H
