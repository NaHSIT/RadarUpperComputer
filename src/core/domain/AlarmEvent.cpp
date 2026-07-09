#include "AlarmEvent.h"
#include <QUuid>

AlarmEvent::AlarmEvent(QObject *parent)
    : QObject(parent)
    , m_severity(AlarmSeverity::Info)
    , m_source(AlarmSource::Device)
{
    m_alarmId = QUuid::createUuid().toString().remove('{').remove('}');
}

AlarmEvent::~AlarmEvent()
{
}

void AlarmEvent::setAlarmId(const QString &id) { m_alarmId = id; }
void AlarmEvent::setSeverity(AlarmSeverity severity) { m_severity = severity; emit updated(); }
void AlarmEvent::setSource(AlarmSource source) { m_source = source; emit updated(); }
void AlarmEvent::setCode(const QString &code) { m_code = code; emit updated(); }
void AlarmEvent::setTitle(const QString &title) { m_title = title; emit updated(); }
void AlarmEvent::setDescription(const QString &description) { m_description = description; emit updated(); }
void AlarmEvent::setRecommendedAction(const QString &action) { m_recommendedAction = action; emit updated(); }
void AlarmEvent::setFirstSeen(const QDateTime &time) { m_firstSeen = time; }
void AlarmEvent::setLastSeen(const QDateTime &time) { m_lastSeen = time; }
void AlarmEvent::setRelatedDataFrameId(const QString &id) { m_relatedDataFrameId = id; }

void AlarmEvent::acknowledge(const QString &user)
{
    if (!m_acknowledgedBy.isEmpty()) return;
    m_acknowledgedBy = user;
    m_acknowledgedAt = QDateTime::currentDateTimeUtc();
    emit acknowledged(user);
    emit updated();
}

void AlarmEvent::resolve()
{
    if (m_resolvedAt.isValid()) return;
    m_resolvedAt = QDateTime::currentDateTimeUtc();
    emit resolved();
    emit updated();
}

void AlarmEvent::updateLastSeen()
{
    m_lastSeen = QDateTime::currentDateTimeUtc();
    emit updated();
}

QJsonObject AlarmEvent::toJson() const
{
    QJsonObject json;
    json["alarmId"] = m_alarmId;
    json["severity"] = static_cast<int>(m_severity);
    json["source"] = static_cast<int>(m_source);
    json["code"] = m_code;
    json["title"] = m_title;
    json["description"] = m_description;
    json["recommendedAction"] = m_recommendedAction;
    json["firstSeen"] = m_firstSeen.toString(Qt::ISODate);
    json["lastSeen"] = m_lastSeen.toString(Qt::ISODate);
    json["acknowledgedBy"] = m_acknowledgedBy;
    json["acknowledgedAt"] = m_acknowledgedAt.toString(Qt::ISODate);
    json["resolvedAt"] = m_resolvedAt.toString(Qt::ISODate);
    json["relatedDataFrameId"] = m_relatedDataFrameId;
    return json;
}

void AlarmEvent::fromJson(const QJsonObject &json)
{
    m_alarmId = json["alarmId"].toString();
    m_severity = static_cast<AlarmSeverity>(json["severity"].toInt());
    m_source = static_cast<AlarmSource>(json["source"].toInt());
    m_code = json["code"].toString();
    m_title = json["title"].toString();
    m_description = json["description"].toString();
    m_recommendedAction = json["recommendedAction"].toString();
    m_firstSeen = QDateTime::fromString(json["firstSeen"].toString(), Qt::ISODate);
    m_lastSeen = QDateTime::fromString(json["lastSeen"].toString(), Qt::ISODate);
    m_acknowledgedBy = json["acknowledgedBy"].toString();
    m_acknowledgedAt = QDateTime::fromString(json["acknowledgedAt"].toString(), Qt::ISODate);
    m_resolvedAt = QDateTime::fromString(json["resolvedAt"].toString(), Qt::ISODate);
    m_relatedDataFrameId = json["relatedDataFrameId"].toString();

    emit updated();
}
