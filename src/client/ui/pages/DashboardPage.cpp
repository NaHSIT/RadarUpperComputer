#include "DashboardPage.h"
#include "ui/widgets/MetricCard.h"
#include "ui/widgets/WindTrendChart.h"
#include "ui/widgets/WindRoseWidget.h"
#include "ui/widgets/BeamHealthGrid.h"
#include "ui/widgets/AlarmList.h"
#include "ui/widgets/RangeGateTable.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
    , m_windSpeedCard(nullptr)
    , m_windDirectionCard(nullptr)
    , m_confidenceCard(nullptr)
    , m_validGatesCard(nullptr)
    , m_blindRatioCard(nullptr)
    , m_alarmCountCard(nullptr)
    , m_windTrendChart(nullptr)
    , m_windRoseWidget(nullptr)
    , m_beamHealthGrid(nullptr)
    , m_alarmList(nullptr)
    , m_gateTable(nullptr)
    , m_refreshTimer(new QTimer(this))
{
    setupUI();

    // 启动刷新定时器
    connect(m_refreshTimer, &QTimer::timeout, this, &DashboardPage::onRefreshTimer);
    m_refreshTimer->start(1000);  // 每秒刷新
}

DashboardPage::~DashboardPage()
{
}

void DashboardPage::updateWindData(double windSpeed, double windDirection, double confidence,
                                    int validGates, double blindRatio, int alarmCount)
{
    // 更新指标卡
    if (m_windSpeedCard) {
        m_windSpeedCard->setValue(windSpeed);
        m_windSpeedCard->setStatus(windSpeed > 25 ? "warning" : "normal");
    }

    if (m_windDirectionCard) {
        m_windDirectionCard->setValue(windDirection);
    }

    if (m_confidenceCard) {
        m_confidenceCard->setValue(confidence);
        m_confidenceCard->setStatus(confidence < 50 ? "danger" : (confidence < 80 ? "warning" : "normal"));
    }

    if (m_validGatesCard) {
        m_validGatesCard->setValue(validGates);
        m_validGatesCard->setStatus(validGates < 20 ? "warning" : "normal");
    }

    if (m_blindRatioCard) {
        m_blindRatioCard->setValue(blindRatio * 100);
        m_blindRatioCard->setStatus(blindRatio > 0.1 ? "danger" : (blindRatio > 0.05 ? "warning" : "normal"));
    }

    if (m_alarmCountCard) {
        m_alarmCountCard->setValue(alarmCount);
        m_alarmCountCard->setStatus(alarmCount > 0 ? "danger" : "normal");
    }

    // 更新风速趋势图
    if (m_windTrendChart) {
        m_windTrendChart->addDataPoint(windSpeed);
    }

    // 更新风向罗盘
    if (m_windRoseWidget) {
        m_windRoseWidget->setWindDirection(windDirection);
    }
}

void DashboardPage::updateBeamStatus(int index, const QString &status, double cnr, int validGates)
{
    if (m_beamHealthGrid) {
        BeamHealthGrid::BeamStatus beamStatus;
        beamStatus.beamId = QString("LOS%1").arg(index + 1);
        beamStatus.azimuthDeg = index * 72.0;
        beamStatus.status = status;
        beamStatus.cnrAvg = cnr;
        beamStatus.validGates = validGates;
        m_beamHealthGrid->setBeamStatus(index, beamStatus);
    }
}

void DashboardPage::updateAlarms(const QVector<AlarmList::AlarmItem> &alarms)
{
    if (m_alarmList) {
        m_alarmList->setAlarms(alarms);
    }
}

void DashboardPage::updateGateData(const QVector<RangeGateTable::GateData> &data)
{
    if (m_gateTable) {
        m_gateTable->setGateData(data);
    }
}

void DashboardPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // 页面标题
    QLabel *titleLabel = new QLabel("总览", this);
    titleLabel->setStyleSheet("color: #333; font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // 第一行：指标卡
    createMetricCards();

    // 第二行：图表区域
    createChartArea();

    // 第三行：数据表格
    createDataTableArea();

    mainLayout->addStretch();
}

void DashboardPage::createMetricCards()
{
    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(12);

    // 创建 6 个指标卡
    m_windSpeedCard = new MetricCard(this);
    m_windSpeedCard->setData("轮毂高度风速", 0.0, "m/s");
    cardsLayout->addWidget(m_windSpeedCard);

    m_windDirectionCard = new MetricCard(this);
    m_windDirectionCard->setData("轮毂高度风向", 0.0, "°");
    cardsLayout->addWidget(m_windDirectionCard);

    m_confidenceCard = new MetricCard(this);
    m_confidenceCard->setData("整体置信度", 0.0, "%");
    cardsLayout->addWidget(m_confidenceCard);

    m_validGatesCard = new MetricCard(this);
    m_validGatesCard->setData("有效层数", 0.0, "层");
    cardsLayout->addWidget(m_validGatesCard);

    m_blindRatioCard = new MetricCard(this);
    m_blindRatioCard->setData("当前盲区率", 0.0, "%");
    cardsLayout->addWidget(m_blindRatioCard);

    m_alarmCountCard = new MetricCard(this);
    m_alarmCountCard->setData("严重告警数", 0.0, "个");
    cardsLayout->addWidget(m_alarmCountCard);

    // 添加到主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addLayout(cardsLayout);
    }
}

void DashboardPage::createChartArea()
{
    QHBoxLayout *chartLayout = new QHBoxLayout();
    chartLayout->setSpacing(16);

    // 左侧：风速趋势图
    QVBoxLayout *leftLayout = new QVBoxLayout();
    QLabel *trendLabel = new QLabel("实时风速趋势", this);
    trendLabel->setStyleSheet("color: #333; font-size: 12px; font-weight: bold;");
    leftLayout->addWidget(trendLabel);

    m_windTrendChart = new WindTrendChart(this);
    leftLayout->addWidget(m_windTrendChart);

    connect(m_windTrendChart, &WindTrendChart::timeWindowChanged,
            this, &DashboardPage::onTimeWindowChanged);

    chartLayout->addLayout(leftLayout, 2);

    // 中间：风向罗盘
    QVBoxLayout *centerLayout = new QVBoxLayout();
    QLabel *roseLabel = new QLabel("风向罗盘", this);
    roseLabel->setStyleSheet("color: #333; font-size: 12px; font-weight: bold;");
    centerLayout->addWidget(roseLabel);

    m_windRoseWidget = new WindRoseWidget(this);
    centerLayout->addWidget(m_windRoseWidget, 0, Qt::AlignCenter);

    chartLayout->addLayout(centerLayout, 1);

    // 右侧：五波束健康矩阵
    QVBoxLayout *rightLayout = new QVBoxLayout();
    m_beamHealthGrid = new BeamHealthGrid(this);
    rightLayout->addWidget(m_beamHealthGrid);

    connect(m_beamHealthGrid, &BeamHealthGrid::beamClicked,
            this, &DashboardPage::beamClicked);

    chartLayout->addLayout(rightLayout, 2);

    // 添加到主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addLayout(chartLayout);
    }
}

void DashboardPage::createDataTableArea()
{
    QHBoxLayout *tableLayout = new QHBoxLayout();
    tableLayout->setSpacing(16);

    // 左侧：分层风场表
    QVBoxLayout *leftLayout = new QVBoxLayout();
    m_gateTable = new RangeGateTable(this);
    leftLayout->addWidget(m_gateTable);

    connect(m_gateTable, &RangeGateTable::gateClicked,
            this, &DashboardPage::gateClicked);

    tableLayout->addLayout(leftLayout, 2);

    // 右侧：告警列表
    QVBoxLayout *rightLayout = new QVBoxLayout();
    m_alarmList = new AlarmList(this);
    rightLayout->addWidget(m_alarmList);

    connect(m_alarmList, &AlarmList::alarmClicked,
            this, &DashboardPage::alarmClicked);

    tableLayout->addLayout(rightLayout, 1);

    // 添加到主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addLayout(tableLayout);
    }
}

void DashboardPage::onRefreshTimer()
{
    // 这里可以触发数据更新请求
    // 实际数据会通过 updateWindData 等方法更新
}

void DashboardPage::onTimeWindowChanged(const QString &window)
{
    // 时间窗口变化时，可以重新加载历史数据
    Q_UNUSED(window)
}
