#ifndef DEVICEHEALTHPAGE_H
#define DEVICEHEALTHPAGE_H

#include <QWidget>
#include <QLabel>

class MetricCard;

/**
 * @brief 设备健康页面
 *
 * 显示设备状态、温控、电源、姿态等信息
 */
class DeviceHealthPage : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceHealthPage(QWidget *parent = nullptr);
    ~DeviceHealthPage() override;

    // 数据更新
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

    // 设备信息
    QLabel *m_deviceIdLabel;
    QLabel *m_modelLabel;
    QLabel *m_firmwareLabel;
    QLabel *m_ipLabel;

    // 温控
    MetricCard *m_cpuTempCard;
    MetricCard *m_fpgaTempCard;

    // 电源
    MetricCard *m_ohVoltageCard;
    MetricCard *m_ohCurrentCard;

    // 姿态
    MetricCard *m_rollCard;
    MetricCard *m_tiltCard;

    // 存储
    MetricCard *m_storageCard;

    // 时间同步
    QLabel *m_timeSyncLabel;
    QLabel *m_timeOffsetLabel;
};

#endif // DEVICEHEALTHPAGE_H
