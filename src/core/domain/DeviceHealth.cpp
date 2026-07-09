#include "DeviceHealth.h"

DeviceHealth::DeviceHealth(QObject *parent)
    : QObject(parent)
    , m_ohPuLinkOk(false)
    , m_fpgaArmLinkOk(false)
    , m_fieldbusOk(false)
    , m_mqttOk(false)
    , m_laserOk(false)
    , m_windowClean(true)
    , m_occlusionDetected(false)
    , m_ohVoltage(0)
    , m_ohCurrent(0)
    , m_duVoltage(0)
    , m_duCurrent(0)
    , m_cpuTemp(0)
    , m_fpgaTemp(0)
    , m_fanOk(true)
    , m_rollDeg(0)
    , m_tiltDeg(0)
    , m_phaseConsistent(true)
    , m_storageUsageRatio(0)
    , m_storageErrorRate(0)
    , m_timeSource(TimeSource::Local)
    , m_timeOffsetUs(0)
    , m_timeLocked(false)
    , m_faultBits(0)
{
}

DeviceHealth::~DeviceHealth()
{
}

void DeviceHealth::setOhPuLinkOk(bool ok) { m_ohPuLinkOk = ok; emit healthUpdated(); }
void DeviceHealth::setFpgaArmLinkOk(bool ok) { m_fpgaArmLinkOk = ok; emit healthUpdated(); }
void DeviceHealth::setFieldbusOk(bool ok) { m_fieldbusOk = ok; emit healthUpdated(); }
void DeviceHealth::setMqttOk(bool ok) { m_mqttOk = ok; emit healthUpdated(); }
void DeviceHealth::setLaserOk(bool ok) { m_laserOk = ok; emit healthUpdated(); }
void DeviceHealth::setWindowClean(bool clean) { m_windowClean = clean; emit healthUpdated(); }
void DeviceHealth::setOcclusionDetected(bool detected) { m_occlusionDetected = detected; emit healthUpdated(); }
void DeviceHealth::setOhVoltage(double voltage) { m_ohVoltage = voltage; emit healthUpdated(); }
void DeviceHealth::setOhCurrent(double current) { m_ohCurrent = current; emit healthUpdated(); }
void DeviceHealth::setDuVoltage(double voltage) { m_duVoltage = voltage; emit healthUpdated(); }
void DeviceHealth::setDuCurrent(double current) { m_duCurrent = current; emit healthUpdated(); }
void DeviceHealth::setCpuTemp(double temp) { m_cpuTemp = temp; emit healthUpdated(); }
void DeviceHealth::setFpgaTemp(double temp) { m_fpgaTemp = temp; emit healthUpdated(); }
void DeviceHealth::setFanOk(bool ok) { m_fanOk = ok; emit healthUpdated(); }
void DeviceHealth::setRollDeg(double roll) { m_rollDeg = roll; emit healthUpdated(); }
void DeviceHealth::setTiltDeg(double tilt) { m_tiltDeg = tilt; emit healthUpdated(); }
void DeviceHealth::setPhaseConsistent(bool consistent) { m_phaseConsistent = consistent; emit healthUpdated(); }
void DeviceHealth::setStorageUsageRatio(double ratio) { m_storageUsageRatio = ratio; emit healthUpdated(); }
void DeviceHealth::setStorageErrorRate(double rate) { m_storageErrorRate = rate; emit healthUpdated(); }
void DeviceHealth::setTimeSource(TimeSource source) { m_timeSource = source; emit healthUpdated(); }
void DeviceHealth::setTimeOffsetUs(double offset) { m_timeOffsetUs = offset; emit healthUpdated(); }
void DeviceHealth::setTimeLocked(bool locked) { m_timeLocked = locked; emit healthUpdated(); }

void DeviceHealth::setFaultBits(quint16 bits)
{
    if (m_faultBits != bits) {
        m_faultBits = bits;
        emit faultBitsChanged(bits);
        emit healthUpdated();
    }
}

void DeviceHealth::setFaultBit(int bit, bool set)
{
    if (set) {
        m_faultBits |= (1 << bit);
    } else {
        m_faultBits &= ~(1 << bit);
    }
    emit faultBitsChanged(m_faultBits);
    emit healthUpdated();
}

bool DeviceHealth::getFaultBit(int bit) const
{
    return (m_faultBits >> bit) & 1;
}

QJsonObject DeviceHealth::toJson() const
{
    QJsonObject json;

    QJsonObject comm;
    comm["ohPuLink"] = m_ohPuLinkOk;
    comm["fpgaArmLink"] = m_fpgaArmLinkOk;
    comm["fieldbus"] = m_fieldbusOk;
    comm["mqtt"] = m_mqttOk;
    json["communication"] = comm;

    QJsonObject optics;
    optics["laser"] = m_laserOk;
    optics["window"] = m_windowClean;
    optics["occlusion"] = m_occlusionDetected;
    json["optics"] = optics;

    QJsonObject power;
    power["ohVoltage"] = m_ohVoltage;
    power["ohCurrent"] = m_ohCurrent;
    power["duVoltage"] = m_duVoltage;
    power["duCurrent"] = m_duCurrent;
    json["power"] = power;

    QJsonObject thermal;
    thermal["cpuTemp"] = m_cpuTemp;
    thermal["fpgaTemp"] = m_fpgaTemp;
    thermal["fan"] = m_fanOk;
    json["thermal"] = thermal;

    QJsonObject attitude;
    attitude["rollDeg"] = m_rollDeg;
    attitude["tiltDeg"] = m_tiltDeg;
    attitude["phaseConsistent"] = m_phaseConsistent;
    json["attitude"] = attitude;

    QJsonObject storage;
    storage["usageRatio"] = m_storageUsageRatio;
    storage["errorRate"] = m_storageErrorRate;
    json["storage"] = storage;

    QJsonObject timeSync;
    timeSync["source"] = static_cast<int>(m_timeSource);
    timeSync["offsetUs"] = m_timeOffsetUs;
    timeSync["locked"] = m_timeLocked;
    json["timeSync"] = timeSync;

    json["faultBits"] = m_faultBits;

    return json;
}

void DeviceHealth::fromJson(const QJsonObject &json)
{
    QJsonObject comm = json["communication"].toObject();
    m_ohPuLinkOk = comm["ohPuLink"].toBool();
    m_fpgaArmLinkOk = comm["fpgaArmLink"].toBool();
    m_fieldbusOk = comm["fieldbus"].toBool();
    m_mqttOk = comm["mqtt"].toBool();

    QJsonObject optics = json["optics"].toObject();
    m_laserOk = optics["laser"].toBool();
    m_windowClean = optics["window"].toBool();
    m_occlusionDetected = optics["occlusion"].toBool();

    QJsonObject power = json["power"].toObject();
    m_ohVoltage = power["ohVoltage"].toDouble();
    m_ohCurrent = power["ohCurrent"].toDouble();
    m_duVoltage = power["duVoltage"].toDouble();
    m_duCurrent = power["duCurrent"].toDouble();

    QJsonObject thermal = json["thermal"].toObject();
    m_cpuTemp = thermal["cpuTemp"].toDouble();
    m_fpgaTemp = thermal["fpgaTemp"].toDouble();
    m_fanOk = thermal["fan"].toBool();

    QJsonObject attitude = json["attitude"].toObject();
    m_rollDeg = attitude["rollDeg"].toDouble();
    m_tiltDeg = attitude["tiltDeg"].toDouble();
    m_phaseConsistent = attitude["phaseConsistent"].toBool();

    QJsonObject storage = json["storage"].toObject();
    m_storageUsageRatio = storage["usageRatio"].toDouble();
    m_storageErrorRate = storage["errorRate"].toDouble();

    QJsonObject timeSync = json["timeSync"].toObject();
    m_timeSource = static_cast<TimeSource>(timeSync["source"].toInt());
    m_timeOffsetUs = timeSync["offsetUs"].toDouble();
    m_timeLocked = timeSync["locked"].toBool();

    m_faultBits = json["faultBits"].toInt();

    emit healthUpdated();
}
