#ifndef DEVICEHEALTH_H
#define DEVICEHEALTH_H

#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QString>

#include "RadarTypes.h"

class DeviceHealth : public QObject
{
    Q_OBJECT

public:
    explicit DeviceHealth(QObject *parent = nullptr);
    ~DeviceHealth() override;

    bool ohPuLinkOk() const { return m_ohPuLinkOk; }
    bool fpgaArmLinkOk() const { return m_fpgaArmLinkOk; }
    bool fieldbusOk() const { return m_fieldbusOk; }
    bool mqttOk() const { return m_mqttOk; }

    bool laserOk() const { return m_laserOk; }
    bool windowClean() const { return m_windowClean; }
    bool occlusionDetected() const { return m_occlusionDetected; }

    double ohVoltage() const { return m_ohVoltage; }
    double ohCurrent() const { return m_ohCurrent; }
    double duVoltage() const { return m_duVoltage; }
    double duCurrent() const { return m_duCurrent; }

    double cpuTemp() const { return m_cpuTemp; }
    double fpgaTemp() const { return m_fpgaTemp; }
    bool fanOk() const { return m_fanOk; }

    double rollDeg() const { return m_rollDeg; }
    double tiltDeg() const { return m_tiltDeg; }
    bool phaseConsistent() const { return m_phaseConsistent; }

    double storageUsageRatio() const { return m_storageUsageRatio; }
    double storageErrorRate() const { return m_storageErrorRate; }

    TimeSource timeSource() const { return m_timeSource; }
    double timeOffsetUs() const { return m_timeOffsetUs; }
    bool timeLocked() const { return m_timeLocked; }

    quint16 faultBits() const { return m_faultBits; }
    bool hasFault() const { return m_faultBits != 0; }

    void setOhPuLinkOk(bool ok);
    void setFpgaArmLinkOk(bool ok);
    void setFieldbusOk(bool ok);
    void setMqttOk(bool ok);
    void setLaserOk(bool ok);
    void setWindowClean(bool clean);
    void setOcclusionDetected(bool detected);
    void setOhVoltage(double voltage);
    void setOhCurrent(double current);
    void setDuVoltage(double voltage);
    void setDuCurrent(double current);
    void setCpuTemp(double temp);
    void setFpgaTemp(double temp);
    void setFanOk(bool ok);
    void setRollDeg(double roll);
    void setTiltDeg(double tilt);
    void setPhaseConsistent(bool consistent);
    void setStorageUsageRatio(double ratio);
    void setStorageErrorRate(double rate);
    void setTimeSource(TimeSource source);
    void setTimeOffsetUs(double offset);
    void setTimeLocked(bool locked);
    void setFaultBits(quint16 bits);
    void setFaultBit(int bit, bool set);
    bool getFaultBit(int bit) const;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

signals:
    void healthUpdated();
    void faultBitsChanged(quint16 bits);

private:
    bool m_ohPuLinkOk;
    bool m_fpgaArmLinkOk;
    bool m_fieldbusOk;
    bool m_mqttOk;

    bool m_laserOk;
    bool m_windowClean;
    bool m_occlusionDetected;

    double m_ohVoltage;
    double m_ohCurrent;
    double m_duVoltage;
    double m_duCurrent;

    double m_cpuTemp;
    double m_fpgaTemp;
    bool m_fanOk;

    double m_rollDeg;
    double m_tiltDeg;
    bool m_phaseConsistent;

    double m_storageUsageRatio;
    double m_storageErrorRate;

    TimeSource m_timeSource;
    double m_timeOffsetUs;
    bool m_timeLocked;

    quint16 m_faultBits;
};

#endif // DEVICEHEALTH_H
