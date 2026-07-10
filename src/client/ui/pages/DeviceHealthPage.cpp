#include "DeviceHealthPage.h"

#include "ui/widgets/MetricCard.h"

#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QtMath>

namespace {
QFrame *section(const QString &name, QWidget *parent)
{
    auto *box = new QFrame(parent);
    box->setObjectName("healthSection");
    box->setStyleSheet("QFrame#healthSection { background:#ffffff; border:1px solid #d9dee5; border-radius:4px; }");
    auto *layout = new QVBoxLayout(box); layout->setContentsMargins(14, 12, 14, 14); layout->setSpacing(10);
    auto *title = new QLabel(name, box); title->setStyleSheet("color:#263442; font-size:14px; font-weight:600;"); layout->addWidget(title);
    return box;
}
QLabel *valueLabel(QWidget *parent) { auto *label = new QLabel("--", parent); label->setStyleSheet("color:#182230; font-size:13px; font-weight:600;"); return label; }
}

DeviceHealthPage::DeviceHealthPage(QWidget *parent)
    : QWidget(parent), m_deviceIdLabel(nullptr), m_modelLabel(nullptr), m_firmwareLabel(nullptr), m_ipLabel(nullptr)
    , m_cpuTempCard(nullptr), m_fpgaTempCard(nullptr), m_ohVoltageCard(nullptr), m_ohCurrentCard(nullptr)
    , m_rollCard(nullptr), m_tiltCard(nullptr), m_storageCard(nullptr), m_timeSyncLabel(nullptr), m_timeOffsetLabel(nullptr)
{ setupUI(); }
DeviceHealthPage::~DeviceHealthPage() = default;
void DeviceHealthPage::updateConnectionStatus(bool ohPuOk, bool fpgaArmOk) { Q_UNUSED(ohPuOk) Q_UNUSED(fpgaArmOk) }
void DeviceHealthPage::updateTemperature(double cpu, double fpga) { m_cpuTempCard->setValue(cpu); m_cpuTempCard->setStatus(cpu > 70 ? "danger" : cpu > 60 ? "warning" : "normal"); m_fpgaTempCard->setValue(fpga); m_fpgaTempCard->setStatus(fpga > 70 ? "danger" : fpga > 60 ? "warning" : "normal"); }
void DeviceHealthPage::updatePower(double voltage, double current) { m_ohVoltageCard->setValue(voltage); m_ohVoltageCard->setStatus(voltage < 22 || voltage > 26 ? "danger" : "normal"); m_ohCurrentCard->setValue(current); }
void DeviceHealthPage::updateAttitude(double roll, double tilt) { m_rollCard->setValue(roll); m_rollCard->setStatus(qAbs(roll) > 5 ? "danger" : qAbs(roll) > 2 ? "warning" : "normal"); m_tiltCard->setValue(tilt); m_tiltCard->setStatus(qAbs(tilt) > 5 ? "danger" : qAbs(tilt) > 2 ? "warning" : "normal"); }
void DeviceHealthPage::updateStorage(double ratio) { m_storageCard->setValue(ratio * 100); m_storageCard->setStatus(ratio > .9 ? "danger" : ratio > .7 ? "warning" : "normal"); }
void DeviceHealthPage::updateTimeSync(bool locked, double offset) { m_timeSyncLabel->setText(locked ? QStringLiteral("已锁定") : QStringLiteral("未锁定")); m_timeSyncLabel->setStyleSheet(locked ? "color:#16713b; font-size:13px; font-weight:600;" : "color:#b42318; font-size:13px; font-weight:600;"); m_timeOffsetLabel->setText(QStringLiteral("%1 μs").arg(offset, 0, 'f', 2)); }
void DeviceHealthPage::setupUI()
{
    auto *main = new QVBoxLayout(this); main->setContentsMargins(24,20,24,18); main->setSpacing(14);
    auto *title = new QLabel(QStringLiteral("设备状态"), this); title->setStyleSheet("color:#182230; font-size:22px; font-weight:600;"); main->addWidget(title);
    auto *subtitle = new QLabel(QStringLiteral("查看主机、测量链路、姿态、电源和时间同步的实时状态"), this);
    subtitle->setStyleSheet("color:#667085; font-size:12px; padding-top:2px;");
    subtitle->setWordWrap(true);
    main->addWidget(subtitle);
    createDeviceInfoSection();
    auto *metrics = new QGridLayout(); metrics->setHorizontalSpacing(10); metrics->setVerticalSpacing(10);
    createTemperatureSection(); createPowerSection(); createAttitudeSection(); createStorageSection();
    Q_UNUSED(metrics)
    createTimeSyncSection(); main->addStretch();
}
void DeviceHealthPage::createDeviceInfoSection()
{
    auto *box = section(QStringLiteral("设备信息"), this); auto *form = new QGridLayout(); form->setColumnStretch(1,1); form->setColumnStretch(3,1);
    m_deviceIdLabel=valueLabel(box); m_modelLabel=valueLabel(box); m_firmwareLabel=valueLabel(box); m_ipLabel=valueLabel(box);
    const QStringList labels={QStringLiteral("设备编号"),QStringLiteral("设备型号"),QStringLiteral("固件版本"),QStringLiteral("设备地址")}; const QList<QLabel*> values={m_deviceIdLabel,m_modelLabel,m_firmwareLabel,m_ipLabel};
    for(int i=0;i<4;++i){auto *label=new QLabel(labels[i]+QStringLiteral("："),box);label->setStyleSheet("color:#667085; font-size:13px;");form->addWidget(label,i/2*1,(i%2)*2);form->addWidget(values[i],i/2*1,(i%2)*2+1);} static_cast<QVBoxLayout*>(box->layout())->addLayout(form); static_cast<QVBoxLayout*>(layout())->addWidget(box);
}
void DeviceHealthPage::createTemperatureSection()
{
    auto *box=section(QStringLiteral("温度监测"),this); auto *row=new QHBoxLayout(); m_cpuTempCard=new MetricCard(box);m_cpuTempCard->setData(QStringLiteral("CPU 温度"),0,QStringLiteral("°C"));m_fpgaTempCard=new MetricCard(box);m_fpgaTempCard->setData(QStringLiteral("FPGA 温度"),0,QStringLiteral("°C"));row->addWidget(m_cpuTempCard);row->addWidget(m_fpgaTempCard);row->addStretch();static_cast<QVBoxLayout*>(box->layout())->addLayout(row);static_cast<QVBoxLayout*>(layout())->addWidget(box);
}
void DeviceHealthPage::createPowerSection()
{
    auto *box=section(QStringLiteral("供电状态"),this);auto *row=new QHBoxLayout();m_ohVoltageCard=new MetricCard(box);m_ohVoltageCard->setData(QStringLiteral("光端机电压"),0,"V");m_ohCurrentCard=new MetricCard(box);m_ohCurrentCard->setData(QStringLiteral("光端机电流"),0,"A");row->addWidget(m_ohVoltageCard);row->addWidget(m_ohCurrentCard);row->addStretch();static_cast<QVBoxLayout*>(box->layout())->addLayout(row);static_cast<QVBoxLayout*>(layout())->addWidget(box);
}
void DeviceHealthPage::createAttitudeSection()
{
    auto *box=section(QStringLiteral("安装姿态"),this);auto *row=new QHBoxLayout();m_rollCard=new MetricCard(box);m_rollCard->setData(QStringLiteral("横滚角"),0,QStringLiteral("°"));m_tiltCard=new MetricCard(box);m_tiltCard->setData(QStringLiteral("俯仰角"),0,QStringLiteral("°"));row->addWidget(m_rollCard);row->addWidget(m_tiltCard);m_storageCard=new MetricCard(box);m_storageCard->setData(QStringLiteral("磁盘使用率"),0,"%");row->addWidget(m_storageCard);row->addStretch();static_cast<QVBoxLayout*>(box->layout())->addLayout(row);static_cast<QVBoxLayout*>(layout())->addWidget(box);
}
void DeviceHealthPage::createStorageSection() {}
void DeviceHealthPage::createTimeSyncSection()
{
    auto *box=section(QStringLiteral("时间同步"),this);auto *row=new QHBoxLayout();auto *a=new QLabel(QStringLiteral("同步状态："),box);a->setStyleSheet("color:#667085; font-size:13px;");m_timeSyncLabel=valueLabel(box);auto *b=new QLabel(QStringLiteral("时间偏移："),box);b->setStyleSheet("color:#667085; font-size:13px; margin-left:32px;");m_timeOffsetLabel=valueLabel(box);row->addWidget(a);row->addWidget(m_timeSyncLabel);row->addWidget(b);row->addWidget(m_timeOffsetLabel);row->addStretch();static_cast<QVBoxLayout*>(box->layout())->addLayout(row);static_cast<QVBoxLayout*>(layout())->addWidget(box);
}
