#include "DeviceHealthPage.h"
#include "widgets/MetricCard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>

DeviceHealthPage::DeviceHealthPage(QWidget *parent)
    : QWidget(parent)
    , m_deviceIdLabel(nullptr)
    , m_modelLabel(nullptr)
    , m_firmwareLabel(nullptr)
    , m_ipLabel(nullptr)
    , m_cpuTempCard(nullptr)
    , m_fpgaTempCard(nullptr)
    , m_ohVoltageCard(nullptr)
    , m_ohCurrentCard(nullptr)
    , m_rollCard(nullptr)
    , m_tiltCard(nullptr)
    , m_storageCard(nullptr)
    , m_timeSyncLabel(nullptr)
    , m_timeOffsetLabel(nullptr)
{
    setupUI();
}

DeviceHealthPage::~DeviceHealthPage()
{
}

void DeviceHealthPage::updateConnectionStatus(bool ohPuOk, bool fpgaArmOk)
{
    // 更新连接状态显示
    Q_UNUSED(ohPuOk)
    Q_UNUSED(fpgaArmOk)
}

void DeviceHealthPage::updateTemperature(double cpuTemp, double fpgaTemp)
{
    if (m_cpuTempCard) {
        m_cpuTempCard->setValue(cpuTemp);
        m_cpuTempCard->setStatus(cpuTemp > 70 ? "danger" : (cpuTemp > 60 ? "warning" : "normal"));
    }
    if (m_fpgaTempCard) {
        m_fpgaTempCard->setValue(fpgaTemp);
        m_fpgaTempCard->setStatus(fpgaTemp > 70 ? "danger" : (fpgaTemp > 60 ? "warning" : "normal"));
    }
}

void DeviceHealthPage::updatePower(double ohVoltage, double ohCurrent)
{
    if (m_ohVoltageCard) {
        m_ohVoltageCard->setValue(ohVoltage);
        m_ohVoltageCard->setStatus(ohVoltage < 22 || ohVoltage > 26 ? "danger" : "normal");
    }
    if (m_ohCurrentCard) {
        m_ohCurrentCard->setValue(ohCurrent);
    }
}

void DeviceHealthPage::updateAttitude(double roll, double tilt)
{
    if (m_rollCard) {
        m_rollCard->setValue(roll);
        m_rollCard->setStatus(qAbs(roll) > 5 ? "danger" : (qAbs(roll) > 2 ? "warning" : "normal"));
    }
    if (m_tiltCard) {
        m_tiltCard->setValue(tilt);
        m_tiltCard->setStatus(qAbs(tilt) > 5 ? "danger" : (qAbs(tilt) > 2 ? "warning" : "normal"));
    }
}

void DeviceHealthPage::updateStorage(double usageRatio)
{
    if (m_storageCard) {
        m_storageCard->setValue(usageRatio * 100);
        m_storageCard->setStatus(usageRatio > 0.9 ? "danger" : (usageRatio > 0.7 ? "warning" : "normal"));
    }
}

void DeviceHealthPage::updateTimeSync(bool locked, double offsetUs)
{
    if (m_timeSyncLabel) {
        m_timeSyncLabel->setText(locked ? "已锁定" : "未锁定");
        m_timeSyncLabel->setStyleSheet(locked ? "color: green;" : "color: red;");
    }
    if (m_timeOffsetLabel) {
        m_timeOffsetLabel->setText(QString("%1 μs").arg(offsetUs, 0, 'f', 2));
    }
}

void DeviceHealthPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // 页面标题
    QLabel *titleLabel = new QLabel("设备健康", this);
    titleLabel->setStyleSheet("color: #333; font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // 设备信息
    createDeviceInfoSection();

    // 温控
    createTemperatureSection();

    // 电源
    createPowerSection();

    // 姿态
    createAttitudeSection();

    // 存储
    createStorageSection();

    // 时间同步
    createTimeSyncSection();

    mainLayout->addStretch();
}

void DeviceHealthPage::createDeviceInfoSection()
{
    QGroupBox *group = new QGroupBox("设备信息", this);
    group->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #ddd; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }");

    QGridLayout *gridLayout = new QGridLayout(group);

    m_deviceIdLabel = new QLabel("--", group);
    m_modelLabel = new QLabel("--", group);
    m_firmwareLabel = new QLabel("--", group);
    m_ipLabel = new QLabel("--", group);

    gridLayout->addWidget(new QLabel("设备编号:", group), 0, 0);
    gridLayout->addWidget(m_deviceIdLabel, 0, 1);
    gridLayout->addWidget(new QLabel("型号:", group), 0, 2);
    gridLayout->addWidget(m_modelLabel, 0, 3);

    gridLayout->addWidget(new QLabel("固件版本:", group), 1, 0);
    gridLayout->addWidget(m_firmwareLabel, 1, 1);
    gridLayout->addWidget(new QLabel("IP 地址:", group), 1, 2);
    gridLayout->addWidget(m_ipLabel, 1, 3);

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(group);
    }
}

void DeviceHealthPage::createTemperatureSection()
{
    QGroupBox *group = new QGroupBox("温控状态", this);
    group->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #ddd; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }");

    QHBoxLayout *hLayout = new QHBoxLayout(group);

    m_cpuTempCard = new MetricCard(group);
    m_cpuTempCard->setData("CPU 温度", 0.0, "°C");
    hLayout->addWidget(m_cpuTempCard);

    m_fpgaTempCard = new MetricCard(group);
    m_fpgaTempCard->setData("FPGA 温度", 0.0, "°C");
    hLayout->addWidget(m_fpgaTempCard);

    hLayout->addStretch();

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(group);
    }
}

void DeviceHealthPage::createPowerSection()
{
    QGroupBox *group = new QGroupBox("电源状态", this);
    group->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #ddd; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }");

    QHBoxLayout *hLayout = new QHBoxLayout(group);

    m_ohVoltageCard = new MetricCard(group);
    m_ohVoltageCard->setData("OH 电压", 0.0, "V");
    hLayout->addWidget(m_ohVoltageCard);

    m_ohCurrentCard = new MetricCard(group);
    m_ohCurrentCard->setData("OH 电流", 0.0, "A");
    hLayout->addWidget(m_ohCurrentCard);

    hLayout->addStretch();

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(group);
    }
}

void DeviceHealthPage::createAttitudeSection()
{
    QGroupBox *group = new QGroupBox("姿态状态", this);
    group->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #ddd; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }");

    QHBoxLayout *hLayout = new QHBoxLayout(group);

    m_rollCard = new MetricCard(group);
    m_rollCard->setData("Roll", 0.0, "°");
    hLayout->addWidget(m_rollCard);

    m_tiltCard = new MetricCard(group);
    m_tiltCard->setData("Tilt", 0.0, "°");
    hLayout->addWidget(m_tiltCard);

    hLayout->addStretch();

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(group);
    }
}

void DeviceHealthPage::createStorageSection()
{
    QGroupBox *group = new QGroupBox("存储状态", this);
    group->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #ddd; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }");

    QHBoxLayout *hLayout = new QHBoxLayout(group);

    m_storageCard = new MetricCard(group);
    m_storageCard->setData("存储使用率", 0.0, "%");
    hLayout->addWidget(m_storageCard);

    hLayout->addStretch();

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(group);
    }
}

void DeviceHealthPage::createTimeSyncSection()
{
    QGroupBox *group = new QGroupBox("时间同步", this);
    group->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #ddd; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }");

    QGridLayout *gridLayout = new QGridLayout(group);

    m_timeSyncLabel = new QLabel("--", group);
    m_timeOffsetLabel = new QLabel("--", group);

    gridLayout->addWidget(new QLabel("同步状态:", group), 0, 0);
    gridLayout->addWidget(m_timeSyncLabel, 0, 1);
    gridLayout->addWidget(new QLabel("时间偏差:", group), 0, 2);
    gridLayout->addWidget(m_timeOffsetLabel, 0, 3);

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(group);
    }
}
