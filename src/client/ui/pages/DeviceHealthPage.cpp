#include "DeviceHealthPage.h"

#include "ui/widgets/MetricCard.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QtMath>

namespace {
QFrame *createSection(const QString &title, QWidget *parent)
{
    auto *section = new QFrame(parent);
    section->setObjectName("deviceHealthSection");
    section->setStyleSheet("QFrame#deviceHealthSection { background:#ffffff; border:1px solid #d9dee5; border-radius:3px; }");
    auto *layout = new QVBoxLayout(section);
    layout->setContentsMargins(16, 13, 16, 16);
    layout->setSpacing(12);
    auto *heading = new QLabel(title, section);
    heading->setStyleSheet("color:#243447; font-size:14px; font-weight:600;");
    layout->addWidget(heading);
    return section;
}

QLabel *createValueLabel(QWidget *parent)
{
    auto *label = new QLabel(QStringLiteral("--"), parent);
    label->setStyleSheet("color:#182230; font-size:13px; font-weight:600;");
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

QLabel *createFieldLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setStyleSheet("color:#667085; font-size:13px;");
    return label;
}
}

DeviceHealthPage::DeviceHealthPage(QWidget *parent)
    : QWidget(parent)
    , m_deviceIdLabel(nullptr), m_modelLabel(nullptr), m_firmwareLabel(nullptr), m_ipLabel(nullptr)
    , m_cpuTempCard(nullptr), m_fpgaTempCard(nullptr), m_ohVoltageCard(nullptr), m_ohCurrentCard(nullptr)
    , m_rollCard(nullptr), m_tiltCard(nullptr), m_storageCard(nullptr)
    , m_timeSyncLabel(nullptr), m_timeOffsetLabel(nullptr)
{
    setupUI();
}

DeviceHealthPage::~DeviceHealthPage() = default;

void DeviceHealthPage::updateConnectionStatus(bool ohPuOk, bool fpgaArmOk)
{
    Q_UNUSED(ohPuOk)
    Q_UNUSED(fpgaArmOk)
}

void DeviceHealthPage::updateTemperature(double cpuTemp, double fpgaTemp)
{
    m_cpuTempCard->setValue(cpuTemp);
    m_cpuTempCard->setStatus(cpuTemp > 70.0 ? "danger" : cpuTemp > 60.0 ? "warning" : "normal");
    m_fpgaTempCard->setValue(fpgaTemp);
    m_fpgaTempCard->setStatus(fpgaTemp > 70.0 ? "danger" : fpgaTemp > 60.0 ? "warning" : "normal");
}

void DeviceHealthPage::updatePower(double voltage, double current)
{
    m_ohVoltageCard->setValue(voltage);
    m_ohVoltageCard->setStatus(voltage < 22.0 || voltage > 26.0 ? "danger" : "normal");
    m_ohCurrentCard->setValue(current);
}

void DeviceHealthPage::updateAttitude(double roll, double tilt)
{
    m_rollCard->setValue(roll);
    m_rollCard->setStatus(qAbs(roll) > 5.0 ? "danger" : qAbs(roll) > 2.0 ? "warning" : "normal");
    m_tiltCard->setValue(tilt);
    m_tiltCard->setStatus(qAbs(tilt) > 5.0 ? "danger" : qAbs(tilt) > 2.0 ? "warning" : "normal");
}

void DeviceHealthPage::updateStorage(double usageRatio)
{
    m_storageCard->setValue(usageRatio * 100.0);
    m_storageCard->setStatus(usageRatio > 0.9 ? "danger" : usageRatio > 0.7 ? "warning" : "normal");
}

void DeviceHealthPage::updateTimeSync(bool locked, double offsetUs)
{
    m_timeSyncLabel->setText(locked ? QStringLiteral("已锁定") : QStringLiteral("未锁定"));
    m_timeSyncLabel->setStyleSheet(locked ? "color:#16713b; font-size:13px; font-weight:600;" : "color:#b42318; font-size:13px; font-weight:600;");
    m_timeOffsetLabel->setText(QStringLiteral("%1 μs").arg(offsetUs, 0, 'f', 2));
}

void DeviceHealthPage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 18);
    mainLayout->setSpacing(12);
    auto *title = new QLabel(QStringLiteral("设备状态"), this);
    title->setStyleSheet("color:#182230; font-size:22px; font-weight:600;");
    auto *subtitle = new QLabel(QStringLiteral("集中查看主机、测量链路、安装姿态、供电和时间基准。"), this);
    subtitle->setStyleSheet("color:#667085; font-size:13px;");
    subtitle->setWordWrap(true);
    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);
    auto *source = new QLabel(QStringLiteral("数据来源  ·  设备遥测协议（待 Zynq 端接入）"), this);
    source->setStyleSheet("color:#52606d; background:#edf3f7; border:1px solid #d7e3ec; padding:7px 10px; font-size:12px;");
    source->setToolTip(QStringLiteral("温度、供电、姿态、存储和时间同步值应来自雷达工业主板遥测帧；当前协议尚未完成接入。"));
    mainLayout->addWidget(source);
    createDeviceInfoSection();
    createTemperatureSection();
    createPowerSection();
    createAttitudeSection();
    createTimeSyncSection();
}

void DeviceHealthPage::createDeviceInfoSection()
{
    auto *section = createSection(QStringLiteral("设备信息"), this);
    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(10);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(3, 1);
    m_deviceIdLabel = createValueLabel(section);
    m_modelLabel = createValueLabel(section);
    m_firmwareLabel = createValueLabel(section);
    m_ipLabel = createValueLabel(section);
    grid->addWidget(createFieldLabel(QStringLiteral("设备编号"), section), 0, 0);
    grid->addWidget(m_deviceIdLabel, 0, 1);
    grid->addWidget(createFieldLabel(QStringLiteral("设备型号"), section), 0, 2);
    grid->addWidget(m_modelLabel, 0, 3);
    grid->addWidget(createFieldLabel(QStringLiteral("固件版本"), section), 1, 0);
    grid->addWidget(m_firmwareLabel, 1, 1);
    grid->addWidget(createFieldLabel(QStringLiteral("设备地址"), section), 1, 2);
    grid->addWidget(m_ipLabel, 1, 3);
    static_cast<QVBoxLayout *>(section->layout())->addLayout(grid);
    static_cast<QVBoxLayout *>(layout())->addWidget(section);
}

void DeviceHealthPage::createTemperatureSection()
{
    auto *section = createSection(QStringLiteral("温度监测"), this);
    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(10);
    m_cpuTempCard = new MetricCard(section); m_cpuTempCard->setData(QStringLiteral("CPU 温度"), 0.0, QStringLiteral("°C"));
    m_fpgaTempCard = new MetricCard(section); m_fpgaTempCard->setData(QStringLiteral("FPGA 温度"), 0.0, QStringLiteral("°C"));
    m_cpuTempCard->setToolTip(QStringLiteral("工业主板 CPU 温度；>60°C 警告，>70°C 异常。数据来自设备遥测。"));
    m_fpgaTempCard->setToolTip(QStringLiteral("Zynq/FPGA 片上温度；>60°C 警告，>70°C 异常。数据来自设备遥测。"));
    grid->addWidget(m_cpuTempCard, 0, 0);
    grid->addWidget(m_fpgaTempCard, 0, 1);
    static_cast<QVBoxLayout *>(section->layout())->addLayout(grid);
    static_cast<QVBoxLayout *>(layout())->addWidget(section);
}

void DeviceHealthPage::createPowerSection()
{
    auto *section = createSection(QStringLiteral("供电状态"), this);
    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(10);
    m_ohVoltageCard = new MetricCard(section); m_ohVoltageCard->setData(QStringLiteral("光端机电压"), 0.0, "V");
    m_ohCurrentCard = new MetricCard(section); m_ohCurrentCard->setData(QStringLiteral("光端机电流"), 0.0, "A");
    m_ohVoltageCard->setToolTip(QStringLiteral("光端机供电电压，当前规则 22–26 V 为正常。数据来自电源监测遥测。"));
    m_ohCurrentCard->setToolTip(QStringLiteral("光端机实时工作电流。异常阈值待根据实机功耗标定。"));
    grid->addWidget(m_ohVoltageCard, 0, 0);
    grid->addWidget(m_ohCurrentCard, 0, 1);
    static_cast<QVBoxLayout *>(section->layout())->addLayout(grid);
    static_cast<QVBoxLayout *>(layout())->addWidget(section);
}

void DeviceHealthPage::createAttitudeSection()
{
    auto *section = createSection(QStringLiteral("安装姿态与资源"), this);
    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(10);
    m_rollCard = new MetricCard(section); m_rollCard->setData(QStringLiteral("横滚角"), 0.0, QStringLiteral("°"));
    m_tiltCard = new MetricCard(section); m_tiltCard->setData(QStringLiteral("俯仰角"), 0.0, QStringLiteral("°"));
    m_storageCard = new MetricCard(section); m_storageCard->setData(QStringLiteral("磁盘使用率"), 0.0, "%");
    m_rollCard->setToolTip(QStringLiteral("雷达绕前后轴的横滚角；绝对值 >2° 警告，>5° 异常。"));
    m_tiltCard->setToolTip(QStringLiteral("雷达前后俯仰偏差；绝对值 >2° 警告，>5° 异常。"));
    m_storageCard->setToolTip(QStringLiteral("雷达主板数据盘使用比例；>70% 警告，>90% 异常。"));
    grid->addWidget(m_rollCard, 0, 0);
    grid->addWidget(m_tiltCard, 0, 1);
    grid->addWidget(m_storageCard, 0, 2);
    static_cast<QVBoxLayout *>(section->layout())->addLayout(grid);
    static_cast<QVBoxLayout *>(layout())->addWidget(section);
}

void DeviceHealthPage::createStorageSection() {}

void DeviceHealthPage::createTimeSyncSection()
{
    auto *section = createSection(QStringLiteral("时间同步"), this);
    auto *row = new QHBoxLayout();
    row->setSpacing(10);
    row->addWidget(createFieldLabel(QStringLiteral("同步状态"), section));
    m_timeSyncLabel = createValueLabel(section);
    row->addWidget(m_timeSyncLabel);
    row->addSpacing(28);
    row->addWidget(createFieldLabel(QStringLiteral("时间偏移"), section));
    m_timeOffsetLabel = createValueLabel(section);
    row->addWidget(m_timeOffsetLabel);
    row->addStretch();
    static_cast<QVBoxLayout *>(section->layout())->addLayout(row);
    static_cast<QVBoxLayout *>(layout())->addWidget(section);
}
