#include "WindProfile.h"
#include <algorithm>

WindProfile::WindProfile(QObject *parent)
    : QObject(parent)
    , m_timeQuality(TimeQuality::Synchronized)
    , m_rangeResolutionM(10)
    , m_gateCount(0)
    , m_hubHeightM(0)
    , m_lidarHeightM(0)
    , m_rollDeg(0)
    , m_tiltDeg(0)
    , m_hubWindSpeedMps(0)
    , m_hubWindDirectionDeg(0)
    , m_rawsMps(0)
{
}

WindProfile::~WindProfile()
{
    qDeleteAll(m_rangeGates);
    qDeleteAll(m_beamStates);
}

void WindProfile::setTimestampUtc(const QDateTime &time) { m_timestampUtc = time; }
void WindProfile::setTimeQuality(TimeQuality quality) { m_timeQuality = quality; }
void WindProfile::setRangeResolutionM(int resolution) { m_rangeResolutionM = resolution; }
void WindProfile::setGateCount(int count) { m_gateCount = count; }
void WindProfile::setHubHeightM(double height) { m_hubHeightM = height; }
void WindProfile::setLidarHeightM(double height) { m_lidarHeightM = height; }
void WindProfile::setRollDeg(double roll) { m_rollDeg = roll; }
void WindProfile::setTiltDeg(double tilt) { m_tiltDeg = tilt; }
void WindProfile::setHubWindSpeedMps(double speed) { m_hubWindSpeedMps = speed; }
void WindProfile::setHubWindDirectionDeg(double direction) { m_hubWindDirectionDeg = direction; }
void WindProfile::setRawsMps(double raws) { m_rawsMps = raws; }

void WindProfile::addRangeGate(RangeGate *gate)
{
    gate->setParent(this);
    m_rangeGates.append(gate);
    emit rangeGatesChanged();
}

void WindProfile::addBeamState(BeamState *beam)
{
    beam->setParent(this);
    m_beamStates.append(beam);
    emit beamStatesChanged();
}

void WindProfile::clearRangeGates()
{
    qDeleteAll(m_rangeGates);
    m_rangeGates.clear();
    emit rangeGatesChanged();
}

void WindProfile::clearBeamStates()
{
    qDeleteAll(m_beamStates);
    m_beamStates.clear();
    emit beamStatesChanged();
}

int WindProfile::validGateCount() const
{
    int count = 0;
    for (const auto &gate : m_rangeGates) {
        if (gate->confidence() >= 50) {
            count++;
        }
    }
    return count;
}

double WindProfile::blindRatio() const
{
    if (m_rangeGates.isEmpty()) return 0;
    return 1.0 - static_cast<double>(validGateCount()) / m_rangeGates.size();
}

double WindProfile::confidence() const
{
    if (m_rangeGates.isEmpty()) return 0;
    double sum = 0;
    for (const auto &gate : m_rangeGates) {
        sum += gate->confidence();
    }
    return sum / m_rangeGates.size();
}

RangeGate* WindProfile::findGateByHeight(double height) const
{
    for (auto &gate : m_rangeGates) {
        if (qAbs(gate->heightM() - height) < m_rangeResolutionM / 2.0) {
            return gate;
        }
    }
    return nullptr;
}

BeamState* WindProfile::findBeamById(BeamId id) const
{
    for (auto &beam : m_beamStates) {
        if (beam->beamId() == id) {
            return beam;
        }
    }
    return nullptr;
}

QJsonObject WindProfile::toJson() const
{
    QJsonObject json;
    json["timestampUtc"] = m_timestampUtc.toString(Qt::ISODate);
    json["timeQuality"] = static_cast<int>(m_timeQuality);
    json["rangeResolutionM"] = m_rangeResolutionM;
    json["gateCount"] = m_gateCount;
    json["hubHeightM"] = m_hubHeightM;
    json["lidarHeightM"] = m_lidarHeightM;
    json["rollDeg"] = m_rollDeg;
    json["tiltDeg"] = m_tiltDeg;
    json["hubWindSpeedMps"] = m_hubWindSpeedMps;
    json["hubWindDirectionDeg"] = m_hubWindDirectionDeg;
    json["rawsMps"] = m_rawsMps;

    QJsonArray gatesArray;
    for (const auto &gate : m_rangeGates) {
        gatesArray.append(gate->toJson());
    }
    json["rangeGates"] = gatesArray;

    QJsonArray beamsArray;
    for (const auto &beam : m_beamStates) {
        beamsArray.append(beam->toJson());
    }
    json["beamStates"] = beamsArray;

    return json;
}

void WindProfile::fromJson(const QJsonObject &json)
{
    m_timestampUtc = QDateTime::fromString(json["timestampUtc"].toString(), Qt::ISODate);
    m_timeQuality = static_cast<TimeQuality>(json["timeQuality"].toInt());
    m_rangeResolutionM = json["rangeResolutionM"].toInt();
    m_gateCount = json["gateCount"].toInt();
    m_hubHeightM = json["hubHeightM"].toDouble();
    m_lidarHeightM = json["lidarHeightM"].toDouble();
    m_rollDeg = json["rollDeg"].toDouble();
    m_tiltDeg = json["tiltDeg"].toDouble();
    m_hubWindSpeedMps = json["hubWindSpeedMps"].toDouble();
    m_hubWindDirectionDeg = json["hubWindDirectionDeg"].toDouble();
    m_rawsMps = json["rawsMps"].toDouble();

    clearRangeGates();
    for (const auto &gateJson : json["rangeGates"].toArray()) {
        auto *gate = new RangeGate(this);
        gate->fromJson(gateJson.toObject());
        m_rangeGates.append(gate);
    }

    clearBeamStates();
    for (const auto &beamJson : json["beamStates"].toArray()) {
        auto *beam = new BeamState(this);
        beam->fromJson(beamJson.toObject());
        m_beamStates.append(beam);
    }

    emit dataUpdated();
}
