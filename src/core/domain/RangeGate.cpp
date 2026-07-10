#include "RangeGate.h"
#include <QJsonArray>

RangeGate::RangeGate(QObject *parent)
    : QObject(parent)
    , m_gateIndex(0)
    , m_distanceM(0)
    , m_heightM(0)
    , m_windSpeedMps(0)
    , m_windDirectionDeg(0)
    , m_turbulenceIntensity(0)
    , m_verticalShear(0)
    , m_horizontalShear(0)
    , m_veerDegPerM(0)
    , m_confidence(0)
{
}

RangeGate::~RangeGate()
{
}

void RangeGate::setGateIndex(int index) { m_gateIndex = index; }
void RangeGate::setDistanceM(double distance) { m_distanceM = distance; }
void RangeGate::setHeightM(double height) { m_heightM = height; }
void RangeGate::setWindSpeedMps(double speed) { m_windSpeedMps = speed; }
void RangeGate::setWindDirectionDeg(double direction) { m_windDirectionDeg = direction; }
void RangeGate::setRadialWindSpeedMps(const QVector<double> &rws) { m_radialWindSpeedMps = rws; }
void RangeGate::setCnrDb(const QVector<double> &cnr) { m_cnrDb = cnr; }
void RangeGate::setTurbulenceIntensity(double ti) { m_turbulenceIntensity = ti; }
void RangeGate::setVerticalShear(double shear) { m_verticalShear = shear; }
void RangeGate::setHorizontalShear(double shear) { m_horizontalShear = shear; }
void RangeGate::setVeerDegPerM(double veer) { m_veerDegPerM = veer; }
void RangeGate::setConfidence(double confidence) { m_confidence = confidence; }
void RangeGate::setStatusFlags(const QVector<StatusCode> &flags) { m_statusFlags = flags; }

void RangeGate::appendStatusCode(StatusCode code) { m_statusFlags.append(code); }
void RangeGate::clearStatusFlags() { m_statusFlags.clear(); }

QJsonObject RangeGate::toJson() const
{
    QJsonObject json;
    json["gateIndex"] = m_gateIndex;
    json["distanceM"] = m_distanceM;
    json["heightM"] = m_heightM;
    json["windSpeedMps"] = m_windSpeedMps;
    json["windDirectionDeg"] = m_windDirectionDeg;
    json["turbulenceIntensity"] = m_turbulenceIntensity;
    json["verticalShear"] = m_verticalShear;
    json["horizontalShear"] = m_horizontalShear;
    json["veerDegPerM"] = m_veerDegPerM;
    json["confidence"] = m_confidence;

    QJsonArray rwsArray, cnrArray, flagsArray;
    for (double v : m_radialWindSpeedMps) rwsArray.append(v);
    for (double v : m_cnrDb) cnrArray.append(v);
    for (auto v : m_statusFlags) flagsArray.append(static_cast<int>(v));

    json["radialWindSpeedMps"] = rwsArray;
    json["cnrDb"] = cnrArray;
    json["statusFlags"] = flagsArray;

    return json;
}

void RangeGate::fromJson(const QJsonObject &json)
{
    m_gateIndex = json["gateIndex"].toInt();
    m_distanceM = json["distanceM"].toDouble();
    m_heightM = json["heightM"].toDouble();
    m_windSpeedMps = json["windSpeedMps"].toDouble();
    m_windDirectionDeg = json["windDirectionDeg"].toDouble();
    m_turbulenceIntensity = json["turbulenceIntensity"].toDouble();
    m_verticalShear = json["verticalShear"].toDouble();
    m_horizontalShear = json["horizontalShear"].toDouble();
    m_veerDegPerM = json["veerDegPerM"].toDouble();
    m_confidence = json["confidence"].toDouble();

    m_radialWindSpeedMps.clear();
    for (const auto &v : json["radialWindSpeedMps"].toArray()) {
        m_radialWindSpeedMps.append(v.toDouble());
    }

    m_cnrDb.clear();
    for (const auto &v : json["cnrDb"].toArray()) {
        m_cnrDb.append(v.toDouble());
    }

    m_statusFlags.clear();
    for (const auto &v : json["statusFlags"].toArray()) {
        m_statusFlags.append(static_cast<StatusCode>(v.toInt()));
    }

    emit dataUpdated();
}
