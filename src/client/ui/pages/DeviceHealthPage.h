#ifndef DEVICEHEALTHPAGE_H
#define DEVICEHEALTHPAGE_H

#include <QWidget>
#include <QLabel>

class MetricCard;

class DeviceHealthPage : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceHealthPage(QWidget *parent = nullptr);
    ~DeviceHealthPage() override;

    void updateConnectionStatus(bool ohPuOk, bool fpgaArmOk);
    void updateTemperature(double cpuTemp, double fpgaTemp);
    void updatePower(double ohVoltage, double ohCurrent);
    void updateAttitude(double roll, double tilt);
    void updateStorage(double usageRatio);
    void updateTimeSync(bool locked, double offsetUs);

private:
    void setupUI();
    void createDeviceInfoSection();
    void createTemperatureSection();
    void createPowerSection();
    void createAttitudeSection();
    void createStorageSection();
    void createTimeSyncSection();

    QLabel *m_deviceIdLabel;
    QLabel *m_modelLabel;
    QLabel *m_firmwareLabel;
    QLabel *m_ipLabel;

    MetricCard *m_cpuTempCard;
    MetricCard *m_fpgaTempCard;
    MetricCard *m_ohVoltageCard;
    MetricCard *m_ohCurrentCard;
    MetricCard *m_rollCard;
    MetricCard *m_tiltCard;
    MetricCard *m_storageCard;

    QLabel *m_timeSyncLabel;
    QLabel *m_timeOffsetLabel;
};

#endif // DEVICEHEALTHPAGE_H
