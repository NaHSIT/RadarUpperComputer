#include "DashboardPage.h"

#include "ui/widgets/AlarmList.h"
#include "ui/widgets/BeamHealthGrid.h"
#include "ui/widgets/MetricCard.h"
#include "ui/widgets/RangeGateTable.h"
#include "ui/widgets/WindRoseWidget.h"
#include "ui/widgets/WindTrendChart.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace {
QLabel *sectionTitle(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setStyleSheet("color:#263442; font-size:14px; font-weight:600; padding:0 0 8px;");
    return label;
}

QFrame *panel(QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName("dashboardPanel");
    frame->setStyleSheet("QFrame#dashboardPanel { background:#ffffff; border:1px solid #d9dee5; border-radius:4px; }");
    return frame;
}
}

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent), m_windSpeedCard(nullptr), m_windDirectionCard(nullptr), m_confidenceCard(nullptr)
    , m_validGatesCard(nullptr), m_blindRatioCard(nullptr), m_alarmCountCard(nullptr)
    , m_windTrendChart(nullptr), m_windRoseWidget(nullptr), m_beamHealthGrid(nullptr)
    , m_alarmList(nullptr), m_gateTable(nullptr), m_refreshTimer(new QTimer(this))
{
    setupUI();
    connect(m_refreshTimer, &QTimer::timeout, this, &DashboardPage::onRefreshTimer);
    m_refreshTimer->start(1000);
}

DashboardPage::~DashboardPage() = default;

void DashboardPage::updateWindData(double windSpeed, double windDirection, double confidence, int validGates, double blindRatio, int alarmCount)
{
    m_windSpeedCard->setValue(windSpeed); m_windSpeedCard->setStatus(windSpeed > 25.0 ? "warning" : "normal");
    m_windDirectionCard->setValue(windDirection);
    m_confidenceCard->setValue(confidence); m_confidenceCard->setStatus(confidence < 50.0 ? "danger" : confidence < 80.0 ? "warning" : "normal");
    m_validGatesCard->setValue(validGates); m_validGatesCard->setStatus(validGates < 20 ? "warning" : "normal");
    m_blindRatioCard->setValue(blindRatio * 100.0); m_blindRatioCard->setStatus(blindRatio > 0.1 ? "danger" : blindRatio > 0.05 ? "warning" : "normal");
    m_alarmCountCard->setValue(alarmCount); m_alarmCountCard->setStatus(alarmCount > 0 ? "danger" : "normal");
    m_windTrendChart->addDataPoint(windSpeed);
    m_windRoseWidget->setWindDirection(windDirection);
}

void DashboardPage::updateBeamStatus(int index, const QString &status, double cnr, int validGates)
{
    BeamHealthGrid::BeamStatus beam{QStringLiteral("LOS%1").arg(index + 1), index * 72.0, status, cnr, validGates};
    m_beamHealthGrid->setBeamStatus(index, beam);
}

void DashboardPage::updateAlarms(const QVector<AlarmList::AlarmItem> &alarms) { m_alarmList->setAlarms(alarms); }
void DashboardPage::updateGateData(const QVector<RangeGateTable::GateData> &data) { m_gateTable->setGateData(data); }

void DashboardPage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 18);
    mainLayout->setSpacing(14);
    auto *header = new QHBoxLayout();
    auto *heading = new QVBoxLayout();
    auto *title = new QLabel(QStringLiteral("运行总览"), this);
    title->setStyleSheet("color:#182230; font-size:22px; font-weight:600;");
    auto *subtitle = new QLabel(QStringLiteral("实时查看设备工作状态、近地层风场与活动告警"), this);
    subtitle->setStyleSheet("color:#667085; font-size:12px; padding-top:3px;");
    heading->addWidget(title); heading->addWidget(subtitle);
    header->addLayout(heading); header->addStretch();
    auto *stamp = new QLabel(QStringLiteral("数据刷新：1 秒"), this);
    stamp->setStyleSheet("color:#526b80; background:#eaf1f6; border:1px solid #cbdbe7; padding:6px 10px; font-size:12px;");
    header->addWidget(stamp);
    mainLayout->addLayout(header);
    createMetricCards();
    createChartArea();
    createDataTableArea();
}

void DashboardPage::createMetricCards()
{
    auto *cards = new QHBoxLayout(); cards->setSpacing(10);
    const struct { MetricCard **card; QString title; QString unit; } definitions[] = {
        {&m_windSpeedCard, QStringLiteral("轮毂风速"), "m/s"}, {&m_windDirectionCard, QStringLiteral("轮毂风向"), QStringLiteral("°")},
        {&m_confidenceCard, QStringLiteral("数据置信度"), "%"}, {&m_validGatesCard, QStringLiteral("有效测量层"), QStringLiteral("层")},
        {&m_blindRatioCard, QStringLiteral("盲区比例"), "%"}, {&m_alarmCountCard, QStringLiteral("活动告警"), QStringLiteral("条")}
    };
    for (const auto &definition : definitions) {
        *definition.card = new MetricCard(this);
        (*definition.card)->setData(definition.title, 0.0, definition.unit);
        cards->addWidget(*definition.card);
    }
    layout()->addItem(cards);
}

void DashboardPage::createChartArea()
{
    auto *row = new QHBoxLayout(); row->setSpacing(14);
    auto *trendPanel = panel(this); auto *trendLayout = new QVBoxLayout(trendPanel);
    trendLayout->setContentsMargins(14, 12, 14, 14); trendLayout->addWidget(sectionTitle(QStringLiteral("风速趋势"), trendPanel));
    m_windTrendChart = new WindTrendChart(trendPanel); trendLayout->addWidget(m_windTrendChart);
    connect(m_windTrendChart, &WindTrendChart::timeWindowChanged, this, &DashboardPage::onTimeWindowChanged);
    row->addWidget(trendPanel, 3);

    auto *directionPanel = panel(this); auto *directionLayout = new QVBoxLayout(directionPanel);
    directionLayout->setContentsMargins(14, 12, 14, 14); directionLayout->addWidget(sectionTitle(QStringLiteral("当前风向"), directionPanel));
    m_windRoseWidget = new WindRoseWidget(directionPanel); directionLayout->addWidget(m_windRoseWidget, 1, Qt::AlignCenter);
    row->addWidget(directionPanel, 1);

    auto *beamPanel = panel(this); auto *beamLayout = new QVBoxLayout(beamPanel);
    beamLayout->setContentsMargins(0, 0, 0, 0); m_beamHealthGrid = new BeamHealthGrid(beamPanel); beamLayout->addWidget(m_beamHealthGrid);
    connect(m_beamHealthGrid, &BeamHealthGrid::beamClicked, this, [this](int index, const QString &) { emit beamClicked(index); });
    row->addWidget(beamPanel, 2);
    layout()->addItem(row);
}

void DashboardPage::createDataTableArea()
{
    auto *row = new QHBoxLayout(); row->setSpacing(14);
    auto *gatePanel = panel(this); auto *gateLayout = new QVBoxLayout(gatePanel);
    gateLayout->setContentsMargins(14, 12, 14, 14); m_gateTable = new RangeGateTable(gatePanel); gateLayout->addWidget(m_gateTable);
    connect(m_gateTable, &RangeGateTable::gateClicked, this, &DashboardPage::gateClicked); row->addWidget(gatePanel, 2);
    auto *alarmPanel = panel(this); auto *alarmLayout = new QVBoxLayout(alarmPanel);
    alarmLayout->setContentsMargins(14, 12, 14, 14); m_alarmList = new AlarmList(alarmPanel); alarmLayout->addWidget(m_alarmList);
    connect(m_alarmList, &AlarmList::alarmClicked, this, &DashboardPage::alarmClicked); row->addWidget(alarmPanel, 1);
    layout()->addItem(row);
}

void DashboardPage::onRefreshTimer() {}
void DashboardPage::onTimeWindowChanged(const QString &window) { Q_UNUSED(window) }
