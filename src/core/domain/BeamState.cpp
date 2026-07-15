#include "BeamState.h"
#include <QJsonArray>

BeamState::BeamState(QObject *parent)
    : QObject(parent)
    , m_beamId(BeamId::LOS1)
    , m_azimuthDeg(0)
    , m_elevationDeg(90)
    , m_carrierFrequencyHz(24.0e9)
    , m_enabled(true)
    , m_status(BeamStatus::Normal)
    , m_phaseErrorDeg(0)
{
}

BeamState::~BeamState()
{
}

void BeamState::setBeamId(BeamId id) { m_beamId = id; }
void BeamState::setAzimuthDeg(double azimuth) { m_azimuthDeg = azimuth; }
void BeamState::setElevationDeg(double elevation) { m_elevationDeg = elevation; }
void BeamState::setCarrierFrequencyHz(double frequency) { m_carrierFrequencyHz = frequency; }
void BeamState::setEnabled(bool enabled) { m_enabled = enabled; }
void BeamState::setPhaseErrorDeg(double error) { m_phaseErrorDeg = error; }
void BeamState::setLastUpdateTime(const QDateTime &time) { m_lastUpdateTime = time; }

void BeamState::setStatus(BeamStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void BeamState::setCnrDbByGate(const QVector<double> &cnr) { m_cnrDbByGate = cnr; }
void BeamState::setRwsByGate(const QVector<double> &rws) { m_rwsByGate = rws; }
void BeamState::setConfidenceByGate(const QVector<double> &confidence) { m_confidenceByGate = confidence; }

void BeamState::appendCnr(double cnr) { m_cnrDbByGate.append(cnr); }
void BeamState::appendRws(double rws) { m_rwsByGate.append(rws); }
void BeamState::appendConfidence(double confidence) { m_confidenceByGate.append(confidence); }

void BeamState::clearData()
{
    m_cnrDbByGate.clear();
    m_rwsByGate.clear();
    m_confidenceByGate.clear();
    emit dataUpdated();
}

QJsonObject BeamState::toJson() const
{
    QJsonObject json;
    json["beamId"] = static_cast<int>(m_beamId);
    json["azimuthDeg"] = m_azimuthDeg;
    json["elevationDeg"] = m_elevationDeg;
    json["carrierFrequencyHz"] = m_carrierFrequencyHz;
    json["enabled"] = m_enabled;
    json["status"] = static_cast<int>(m_status);
    json["phaseErrorDeg"] = m_phaseErrorDeg;

    QJsonArray cnrArray, rwsArray, confArray;
    for (double v : m_cnrDbByGate) cnrArray.append(v);
    for (double v : m_rwsByGate) rwsArray.append(v);
    for (double v : m_confidenceByGate) confArray.append(v);

    json["cnrDbByGate"] = cnrArray;
    json["rwsByGate"] = rwsArray;
    json["confidenceByGate"] = confArray;

    return json;
}

void BeamState::fromJson(const QJsonObject &json)
{
    m_beamId = static_cast<BeamId>(json["beamId"].toInt());
    m_azimuthDeg = json["azimuthDeg"].toDouble();
    m_elevationDeg = json["elevationDeg"].toDouble(90.0);
    m_carrierFrequencyHz = json["carrierFrequencyHz"].toDouble(24.0e9);
    m_enabled = json["enabled"].toBool();
    m_status = static_cast<BeamStatus>(json["status"].toInt());
    m_phaseErrorDeg = json["phaseErrorDeg"].toDouble();

    m_cnrDbByGate.clear();
    for (const auto &v : json["cnrDbByGate"].toArray()) {
        m_cnrDbByGate.append(v.toDouble());
    }

    m_rwsByGate.clear();
    for (const auto &v : json["rwsByGate"].toArray()) {
        m_rwsByGate.append(v.toDouble());
    }

    m_confidenceByGate.clear();
    for (const auto &v : json["confidenceByGate"].toArray()) {
        m_confidenceByGate.append(v.toDouble());
    }

    emit dataUpdated();
}
