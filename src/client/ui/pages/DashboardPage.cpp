#include "DashboardPage.h"

#include "ui/widgets/AlarmList.h"
#include "ui/widgets/BeamHealthGrid.h"
#include "ui/widgets/MetricCard.h"
#include "ui/widgets/RangeGateTable.h"
#include "ui/widgets/WindRoseWidget.h"
#include "ui/widgets/WindTrendChart.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QtMath>

namespace {
QLabel *sectionTitle(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setStyleSheet("color:#172b3d; font-size:16px; font-weight:600; padding:0 0 8px;");
    return label;
}

QFrame *panel(QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName("dashboardPanel");
    frame->setStyleSheet("QFrame#dashboardPanel { background:#ffffff; border:1px solid #dce4ec; border-radius:6px; }");
    return frame;
}
}

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent), m_windSpeedCard(nullptr), m_windDirectionCard(nullptr)
    , m_validGatesCard(nullptr), m_blindRatioCard(nullptr), m_alarmCountCard(nullptr)
    , m_windTrendChart(nullptr), m_windDirectionChart(nullptr), m_windRoseWidget(nullptr), m_beamHealthGrid(nullptr)
    , m_alarmList(nullptr), m_gateTable(nullptr), m_refreshTimer(new QTimer(this))
    , m_dataSourceValue(nullptr)
    , m_speedSeriesTitle(nullptr)
    , m_directionIndicatorTitle(nullptr)
    , m_speedHistoryWindowCombo(nullptr)
    , m_directionHistoryWindowCombo(nullptr)
    , m_speedHistoryWindowSeconds(60)
    , m_directionHistoryWindowSeconds(60)
    , m_displayHeightAglM(-1.0)
{
    setupUI();
    connect(m_refreshTimer, &QTimer::timeout, this, &DashboardPage::onRefreshTimer);
    m_refreshTimer->start(1000);
}

DashboardPage::~DashboardPage() = default;

void DashboardPage::updateWindData(double windSpeed, double windDirection, double heightAglM,
                                   int validGates, double blindRatio, int alarmCount)
{
    Q_UNUSED(blindRatio)
    if (heightAglM >= 0.0 && qAbs(heightAglM - m_displayHeightAglM) > 0.1) {
        m_displayHeightAglM = heightAglM;
        const QString height = QString::number(heightAglM, 'f', 0);
        m_windSpeedCard->setTitle(QStringLiteral("风速"));
        m_windSpeedCard->setToolTip(QStringLiteral("<b>风速 U<sub>h</sub></b><br>"
            "<b>当前高度：</b>%1 m AGL。<br>"
            "<b>定义：</b>当前展示高度的水平风速大小。<br>"
            "<b>计算：</b>U<sub>h</sub> = √(U<sub>E</sub>² + V<sub>N</sub>²)。<br>"
            "<b>数据来源：</b>0x8105 五波束径向速度，经加权最小二乘反演。").arg(height));
        m_windDirectionCard->setTitle(QStringLiteral("风向"));
        m_windDirectionCard->setToolTip(QStringLiteral("<b>风向 D<sub>met</sub></b><br>"
            "<b>当前高度：</b>%1 m AGL。<br>"
            "<b>定义：</b>当前展示高度的水平风之气象学来向。<br>"
            "<b>计算：</b>D<sub>met</sub> = mod[atan2(-U<sub>E</sub>, -V<sub>N</sub>)·180/π + 360, 360]。<br>"
            "<b>方位基准：</b>雷达 0° 轴指向真北，方位角顺时针增大。").arg(height));
        if (m_speedSeriesTitle) {
            m_speedSeriesTitle->setText(QStringLiteral("风速时序"));
            m_speedSeriesTitle->setToolTip(QStringLiteral("当前序列为 %1 m AGL 高度处的水平风速；横轴为观测时间，纵轴单位为 m/s。").arg(height));
        }
        if (m_directionIndicatorTitle) {
            m_directionIndicatorTitle->setText(QStringLiteral("风向"));
            m_directionIndicatorTitle->setToolTip(QStringLiteral("当前显示 %1 m AGL 高度处的气象学来向。").arg(height));
        }
    }
    m_windSpeedCard->setValue(windSpeed); m_windSpeedCard->setStatus(windSpeed > 25.0 ? "warning" : "normal");
    m_windDirectionCard->setValue(windDirection);
    m_validGatesCard->setValue(validGates); m_validGatesCard->setStatus(validGates < 20 ? "warning" : "normal");
    m_alarmCountCard->setValue(alarmCount); m_alarmCountCard->setStatus(alarmCount > 0 ? "danger" : "normal");
    m_windTrendChart->addDataPoint(windSpeed);
    if (m_windDirectionChart) m_windDirectionChart->addDataPoint(windDirection);
    m_windRoseWidget->setWindDirection(windDirection);
}

void DashboardPage::updateBeamStatus(int index, const QString &beamId, double azimuthDeg,
                                     double elevationDeg, const QString &status,
                                     double cnr, int validGates)
{
    if (!m_beamHealthGrid) return;
    BeamHealthGrid::BeamStatus beam{beamId, azimuthDeg, elevationDeg, status, cnr, validGates};
    m_beamHealthGrid->setBeamStatus(index, beam);
}

void DashboardPage::setDataSource(const QString &source)
{
    if (m_dataSourceValue) m_dataSourceValue->setText(source);
}

void DashboardPage::updateAlarmCount(int count)
{
    if (!m_alarmCountCard) return;
    m_alarmCountCard->setValue(count);
    m_alarmCountCard->setStatus(count > 0 ? "danger" : "normal");
}

void DashboardPage::setSpeedHistory(const QVector<QPointF> &speedMps, int windowSeconds)
{
    m_speedHistoryWindowSeconds = qBound(60, windowSeconds, 24 * 60 * 60);
    if (m_windTrendChart) {
        m_windTrendChart->setWindowSeconds(m_speedHistoryWindowSeconds);
        if (!speedMps.isEmpty()) m_windTrendChart->setData(speedMps);
    }
}

void DashboardPage::setDirectionHistory(const QVector<QPointF> &directionDeg, int windowSeconds)
{
    m_directionHistoryWindowSeconds = qBound(60, windowSeconds, 24 * 60 * 60);
    if (m_windDirectionChart) {
        m_windDirectionChart->setWindowSeconds(m_directionHistoryWindowSeconds);
        if (!directionDeg.isEmpty()) m_windDirectionChart->setData(directionDeg);
    }
}

void DashboardPage::updateAlarms(const QVector<AlarmList::AlarmItem> &alarms) { m_alarmList->setAlarms(alarms); }
void DashboardPage::updateGateData(const QVector<RangeGateTable::GateData> &data) { m_gateTable->setGateData(data); }

void DashboardPage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 24, 28, 22);
    mainLayout->setSpacing(16);
    auto *header = new QHBoxLayout();
    auto *heading = new QVBoxLayout();
    auto *title = new QLabel(QStringLiteral("运行总览"), this);
    title->setStyleSheet("color:#172b3d; font-size:24px; font-weight:600;");
    auto *subtitle = new QLabel(QStringLiteral("实时查看设备工作状态、近地层风场与活动告警"), this);
    subtitle->setStyleSheet("color:#475467; font-size:14px; padding-top:4px;");
    heading->addWidget(title); heading->addWidget(subtitle);
    header->addLayout(heading); header->addStretch();
    auto *stamp = new QLabel(QStringLiteral("实时数据  ·  1 秒刷新"), this);
    stamp->setStyleSheet("color:#13795b; background:#e8f7f0; border:1px solid #b6e3d1; border-radius:4px; padding:8px 12px; font-size:13px; font-weight:600;");
    header->addWidget(stamp);
    mainLayout->addLayout(header);

    auto *context = new QFrame(this);
    context->setObjectName("observationContext");
    context->setStyleSheet("QFrame#observationContext { background:#eef5f8; border:1px solid #cfe0e8; border-radius:4px; }");
    auto *contextLayout = new QHBoxLayout(context);
    contextLayout->setContentsMargins(14, 9, 14, 9);
    contextLayout->setSpacing(24);
    auto *orientation = new QLabel(QStringLiteral("安装基准  ·  雷达正北朝向（0°）"), context);
    orientation->setStyleSheet("color:#263442; font-size:13px; font-weight:600;");
    orientation->setToolTip(QStringLiteral("<b>安装方位基准</b><br>"
        "采用局地 ENU（East-North-Up）坐标系。雷达机械/电气方位零轴与真北重合，安装航向偏差设为 0°；方位角从真北起算并顺时针增大。"));
    contextLayout->addWidget(orientation);
    auto *directionRule = new QLabel(QStringLiteral("风向定义  ·  气象学来向"), context);
    directionRule->setStyleSheet("color:#263442; font-size:13px; font-weight:600;");
    directionRule->setToolTip(QStringLiteral("<b>气象学风向 D<sub>met</sub></b><br>"
        "表示水平风的来向，单位为度，取值范围 [0, 360)。"
        "D<sub>met</sub> = mod[atan2(-U<sub>E</sub>, -V<sub>N</sub>)·180/π + 360, 360]。"));
    contextLayout->addWidget(directionRule);
    m_dataSourceValue = new QLabel(QStringLiteral("数据来源  ·  等待雷达数据"), context);
    m_dataSourceValue->setStyleSheet("color:#344054; font-size:13px;");
    m_dataSourceValue->setToolTip(QStringLiteral("<b>数据谱系</b><br>"
        "标识当前产品使用的上行消息类型、原始观测量和反演方法。"
        "0x8105 表示五波束径向速度射线；上位机按同一扫描号组帧后执行加权最小二乘三分量反演。"));
    contextLayout->addWidget(m_dataSourceValue, 1);
    mainLayout->addWidget(context);
    createMetricCards();
    createChartArea();
    createDataTableArea();
}

void DashboardPage::createMetricCards()
{
    auto *cards = new QGridLayout();
    cards->setHorizontalSpacing(14);
    cards->setVerticalSpacing(14);
    const struct { MetricCard **card; QString title; QString unit; QString help; } definitions[] = {
        {&m_windSpeedCard, QStringLiteral("风速"), "m/s",
         QStringLiteral("<b>标题所示 AGL 高度处的水平风速 U<sub>h</sub></b><br>"
                        "<b>定义：</b>该高度距离门的水平风速大小。AGL 表示相对雷达安装基准的高度。<br>"
                        "<b>计算：</b>U<sub>h</sub> = √(U<sub>E</sub>² + V<sub>N</sub>²)。<br>"
                        "<b>有效条件：</b>有效波束数≥3、CNR通过门限、WLS 残差≤1.5 m/s且质量标志有效。<br>"
                        "<b>数据来源：</b>0x8105 五波束径向速度，经加权最小二乘反演。")},
        {&m_windDirectionCard, QStringLiteral("风向"), QStringLiteral("°"),
         QStringLiteral("<b>标题所示 AGL 高度处的气象学风向 D<sub>met</sub></b><br>"
                        "<b>定义：</b>该高度距离门水平风的来向。<br>"
                        "<b>计算：</b>D<sub>met</sub> = mod[atan2(-U<sub>E</sub>, -V<sub>N</sub>)·180/π + 360, 360]。<br>"
                        "<b>方位基准：</b>雷达 0° 轴指向真北，方位角顺时针增大。<br>"
                        "<b>解释：</b>0°=北风，90°=东风，180°=南风，270°=西风。")},
        {&m_validGatesCard, QStringLiteral("有效反演层数"), QStringLiteral("层"),
         QStringLiteral("<b>有效反演层数 N<sub>valid</sub></b><br>"
                        "<b>定义：</b>当前一组风廓线中通过反演质量控制的高度层数。<br>"
                        "<b>统计：</b>N<sub>valid</sub> = Σ I(C<sub>i</sub>≥50% ∧ ε<sub>i</sub>≤1.5 m/s ∧ N<sub>beam,i</sub>≥3)。<br>"
                        "<b>用途：</b>表征当前可用风廓线的垂直覆盖能力；层数需结合距离分辨率解读。<br>"
                        "<b>质量详情：</b>设备状态 → 数据质量。")},
        {&m_alarmCountCard, QStringLiteral("未恢复告警数"), QStringLiteral("条"),
         QStringLiteral("<b>未恢复告警数 N<sub>alarm</sub></b><br>"
                        "<b>定义：</b>告警生命周期状态不为 Resolved 的告警事件数。<br>"
                        "<b>范围：</b>通信链路、设备遥测、波束质量、反演算法及数据质量。<br>"
                        "<b>状态：</b>0 表示无未恢复告警；>0 时应进入告警列表查看级别、来源和发生时间。")}
    };
    for (int index = 0; index < 4; ++index) {
        const auto &definition = definitions[index];
        *definition.card = new MetricCard(this);
        (*definition.card)->setData(definition.title, 0.0, definition.unit);
        (*definition.card)->setToolTip(definition.help);
        cards->addWidget(*definition.card, 0, index);
    }
    for (int column = 0; column < 4; ++column) cards->setColumnStretch(column, 1);
    static_cast<QVBoxLayout *>(layout())->addLayout(cards);
}

void DashboardPage::createChartArea()
{
    auto *row = new QHBoxLayout(); row->setSpacing(16);
    auto *trendPanel = panel(this); auto *trendLayout = new QVBoxLayout(trendPanel);
    trendLayout->setContentsMargins(18, 16, 18, 18);
    m_speedSeriesTitle = sectionTitle(QStringLiteral("风速时序"), trendPanel);
    auto *trendHeader = new QHBoxLayout();
    trendHeader->addWidget(m_speedSeriesTitle);
    trendHeader->addStretch();
    auto *windowLabel = new QLabel(QStringLiteral("时间窗口"), trendPanel);
    windowLabel->setStyleSheet("color:#344054; font-size:13px; font-weight:600;");
    trendHeader->addWidget(windowLabel);
    m_speedHistoryWindowCombo = new QComboBox(trendPanel);
    const struct { const char *label; int seconds; } windows[] = {
        {"1 分钟", 60}, {"5 分钟", 300}, {"10 分钟", 600}, {"30 分钟", 1800},
        {"1 小时", 3600}, {"2 小时", 7200}, {"6 小时", 21600},
        {"12 小时", 43200}, {"24 小时", 86400}
    };
    for (const auto &window : windows) {
        m_speedHistoryWindowCombo->addItem(QString::fromUtf8(window.label), window.seconds);
    }
    m_speedHistoryWindowCombo->setMinimumWidth(104);
    m_speedHistoryWindowCombo->setStyleSheet(
        "QComboBox { background:#fff; color:#172b3d; border:1px solid #b9c5d1;"
        " border-radius:3px; padding:6px 28px 6px 10px; font-size:13px; }"
        "QComboBox:hover { border-color:#176b87; }"
        "QComboBox QAbstractItemView { background:#fff; color:#172b3d; selection-background-color:#e7f2f6; }"
    );
    trendHeader->addWidget(m_speedHistoryWindowCombo);
    trendLayout->addLayout(trendHeader);
    trendPanel->setToolTip(QStringLiteral("<b>水平风速时序</b><br>横坐标为雷达观测时标，纵坐标为图题所示固定 AGL 高度处的 U<sub>h</sub>。当展示高度改变时，历史序列将清空，不混合不同高度的观测值。"));
    m_windTrendChart = new WindTrendChart(trendPanel); trendLayout->addWidget(m_windTrendChart);
    connect(m_windTrendChart, &WindTrendChart::timeWindowChanged, this, &DashboardPage::onTimeWindowChanged);
    row->addWidget(trendPanel, 2);

    connect(m_speedHistoryWindowCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_speedHistoryWindowSeconds = m_speedHistoryWindowCombo->itemData(index).toInt();
        emit speedHistoryWindowChanged(m_speedHistoryWindowSeconds);
    });

    auto *rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(16);
    auto *directionPanel = panel(this); auto *directionLayout = new QVBoxLayout(directionPanel);
    directionLayout->setContentsMargins(18, 16, 18, 18);
    m_directionIndicatorTitle = sectionTitle(QStringLiteral("风向"), directionPanel);
    directionLayout->addWidget(m_directionIndicatorTitle);
    directionPanel->setToolTip(QStringLiteral("<b>风向极坐标指示</b><br>"
        "蓝色指针指向气象学来向；N/E/S/W 分别对应真北/东/南/西。"
        "雷达安装航向偏差为 0°，本图未施加额外方位旋转补偿。"));
    m_windRoseWidget = new WindRoseWidget(directionPanel); directionLayout->addWidget(m_windRoseWidget, 1, Qt::AlignCenter);
    rightColumn->addWidget(directionPanel, 1);

    auto *directionSeriesPanel = panel(this);
    auto *directionSeriesLayout = new QVBoxLayout(directionSeriesPanel);
    directionSeriesLayout->setContentsMargins(18, 14, 18, 16);
    auto *directionHeader = new QHBoxLayout();
    auto *directionSeriesTitle = sectionTitle(QStringLiteral("风向时序"), directionSeriesPanel);
    directionSeriesTitle->setToolTip(QStringLiteral("气象学风向随观测时间的变化；0°/360°均表示北风。跨越正北方向时曲线断开，避免产生虚假的大角度跳变。"));
    directionHeader->addWidget(directionSeriesTitle);
    directionHeader->addStretch();
    auto *directionWindowLabel = new QLabel(QStringLiteral("时间窗口"), directionSeriesPanel);
    directionWindowLabel->setStyleSheet("color:#344054; font-size:13px; font-weight:600;");
    directionHeader->addWidget(directionWindowLabel);
    m_directionHistoryWindowCombo = new QComboBox(directionSeriesPanel);
    for (const auto &window : windows) {
        m_directionHistoryWindowCombo->addItem(QString::fromUtf8(window.label), window.seconds);
    }
    m_directionHistoryWindowCombo->setMinimumWidth(104);
    m_directionHistoryWindowCombo->setStyleSheet(m_speedHistoryWindowCombo->styleSheet());
    directionHeader->addWidget(m_directionHistoryWindowCombo);
    directionSeriesLayout->addLayout(directionHeader);
    m_windDirectionChart = new WindTrendChart(directionSeriesPanel);
    m_windDirectionChart->setSeriesType(WindTrendChart::SeriesType::WindDirection);
    directionSeriesLayout->addWidget(m_windDirectionChart);
    connect(m_directionHistoryWindowCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_directionHistoryWindowSeconds = m_directionHistoryWindowCombo->itemData(index).toInt();
        emit directionHistoryWindowChanged(m_directionHistoryWindowSeconds);
    });
    rightColumn->addWidget(directionSeriesPanel, 1);

    row->addLayout(rightColumn, 1);
    static_cast<QVBoxLayout *>(layout())->addLayout(row);
}

void DashboardPage::createDataTableArea()
{
    auto *row = new QHBoxLayout(); row->setSpacing(14);
    auto *gatePanel = panel(this); auto *gateLayout = new QVBoxLayout(gatePanel);
    gateLayout->setContentsMargins(14, 12, 14, 14); m_gateTable = new RangeGateTable(gatePanel); gateLayout->addWidget(m_gateTable);
    connect(m_gateTable, &RangeGateTable::gateClicked, this, &DashboardPage::gateClicked); row->addWidget(gatePanel, 2);
    gatePanel->hide();
    auto *alarmPanel = panel(this); auto *alarmLayout = new QVBoxLayout(alarmPanel);
    alarmLayout->setContentsMargins(14, 12, 14, 14); m_alarmList = new AlarmList(alarmPanel); alarmLayout->addWidget(m_alarmList);
    connect(m_alarmList, &AlarmList::alarmClicked, this, &DashboardPage::alarmClicked); row->addWidget(alarmPanel, 1);
    static_cast<QVBoxLayout *>(layout())->addLayout(row);
}

void DashboardPage::onRefreshTimer() {}
void DashboardPage::onTimeWindowChanged(const QString &window) { Q_UNUSED(window) }
