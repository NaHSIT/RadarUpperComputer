#include "WindFieldPage.h"

#include "ui/widgets/RangeGateTable.h"
#include "ui/widgets/WindTrendChart.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace {
QFrame *createPanel(QWidget *parent)
{
    auto *panel = new QFrame(parent);
    panel->setObjectName("windFieldPanel");
    panel->setStyleSheet("QFrame#windFieldPanel { background:#ffffff; border:1px solid #d9dee5; border-radius:4px; }");
    return panel;
}
}

WindFieldPage::WindFieldPage(QWidget *parent)
    : QWidget(parent)
    , m_timeWindowCombo(nullptr)
    , m_resolutionCombo(nullptr)
    , m_headerLayout(nullptr)
    , m_windSpeedChart(nullptr)
    , m_windDirectionChart(nullptr)
    , m_gateTable(nullptr)
{
    setupUI();
}

WindFieldPage::~WindFieldPage() = default;

void WindFieldPage::updateWindData(double speed, double direction, double confidence)
{
    Q_UNUSED(confidence)
    m_windSpeedChart->addDataPoint(speed);
    m_windDirectionChart->addDataPoint(direction);
}

void WindFieldPage::updateGateData(const QVector<RangeGateTable::GateData> &data)
{
    m_gateTable->setGateData(data);
}

void WindFieldPage::onTimeWindowChanged(const QString &window) { Q_UNUSED(window) }
void WindFieldPage::onResolutionChanged(const QString &resolution) { Q_UNUSED(resolution) }

void WindFieldPage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 18);
    mainLayout->setSpacing(14);

    m_headerLayout = new QHBoxLayout();
    auto *heading = new QVBoxLayout();
    auto *title = new QLabel(QStringLiteral("风场数据"), this);
    title->setStyleSheet("color:#182230; font-size:22px; font-weight:600;");
    auto *subtitle = new QLabel(QStringLiteral("按时间窗口查看风速、风向趋势及分层测量结果"), this);
    subtitle->setStyleSheet("color:#667085; font-size:12px; padding-top:3px;");
    subtitle->setWordWrap(true);
    heading->addWidget(title);
    heading->addWidget(subtitle);
    m_headerLayout->addLayout(heading, 1);
    mainLayout->addLayout(m_headerLayout);
    createHeader();
    createCharts();
    createTable();
}

void WindFieldPage::createHeader()
{
    auto *timeLabel = new QLabel(QStringLiteral("时间窗口"), this);
    timeLabel->setStyleSheet("color:#52606d; font-size:12px;");
    m_timeWindowCombo = new QComboBox(this);
    m_timeWindowCombo->addItems({QStringLiteral("实时"), QStringLiteral("1 分钟"), QStringLiteral("10 分钟"), QStringLiteral("1 小时")});
    m_timeWindowCombo->setMinimumWidth(96);
    m_timeWindowCombo->setStyleSheet("QComboBox { min-height:28px; padding:0 8px; border:1px solid #bfc9d4; border-radius:3px; }");
    connect(m_timeWindowCombo, &QComboBox::currentTextChanged, this, &WindFieldPage::onTimeWindowChanged);

    auto *resolutionLabel = new QLabel(QStringLiteral("距离分辨率"), this);
    resolutionLabel->setStyleSheet("color:#52606d; font-size:12px; margin-left:8px;");
    m_resolutionCombo = new QComboBox(this);
    m_resolutionCombo->addItems({"10 m", "20 m"});
    m_resolutionCombo->setMinimumWidth(76);
    m_resolutionCombo->setStyleSheet("QComboBox { min-height:28px; padding:0 8px; border:1px solid #bfc9d4; border-radius:3px; }");
    connect(m_resolutionCombo, &QComboBox::currentTextChanged, this, &WindFieldPage::onResolutionChanged);

    m_headerLayout->addWidget(timeLabel);
    m_headerLayout->addWidget(m_timeWindowCombo);
    m_headerLayout->addWidget(resolutionLabel);
    m_headerLayout->addWidget(m_resolutionCombo);
}

void WindFieldPage::createCharts()
{
    auto *row = new QHBoxLayout();
    row->setSpacing(14);
    auto *speedPanel = createPanel(this);
    auto *speedLayout = new QVBoxLayout(speedPanel);
    speedLayout->setContentsMargins(14, 12, 14, 14);
    auto *speedTitle = new QLabel(QStringLiteral("风速趋势"), speedPanel);
    speedTitle->setStyleSheet("color:#263442; font-size:14px; font-weight:600;");
    speedLayout->addWidget(speedTitle);
    m_windSpeedChart = new WindTrendChart(speedPanel);
    speedLayout->addWidget(m_windSpeedChart);
    row->addWidget(speedPanel, 1);

    auto *directionPanel = createPanel(this);
    auto *directionLayout = new QVBoxLayout(directionPanel);
    directionLayout->setContentsMargins(14, 12, 14, 14);
    auto *directionTitle = new QLabel(QStringLiteral("风向趋势"), directionPanel);
    directionTitle->setStyleSheet("color:#263442; font-size:14px; font-weight:600;");
    directionLayout->addWidget(directionTitle);
    m_windDirectionChart = new WindTrendChart(directionPanel);
    directionLayout->addWidget(m_windDirectionChart);
    row->addWidget(directionPanel, 1);
    static_cast<QVBoxLayout *>(layout())->addLayout(row);
}

void WindFieldPage::createTable()
{
    auto *panel = createPanel(this);
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(14, 12, 14, 14);
    m_gateTable = new RangeGateTable(panel);
    panelLayout->addWidget(m_gateTable);
    static_cast<QVBoxLayout *>(layout())->addWidget(panel);
}
