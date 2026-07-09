#include "WindFieldPage.h"
#include "widgets/WindTrendChart.h"
#include "widgets/RangeGateTable.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

WindFieldPage::WindFieldPage(QWidget *parent)
    : QWidget(parent)
    , m_timeWindowCombo(nullptr)
    , m_resolutionCombo(nullptr)
    , m_windSpeedChart(nullptr)
    , m_windDirectionChart(nullptr)
    , m_gateTable(nullptr)
{
    setupUI();
}

WindFieldPage::~WindFieldPage()
{
}

void WindFieldPage::updateWindData(double windSpeed, double windDirection, double confidence)
{
    if (m_windSpeedChart) {
        m_windSpeedChart->addDataPoint(windSpeed);
    }
    if (m_windDirectionChart) {
        m_windDirectionChart->addDataPoint(windDirection);
    }
}

void WindFieldPage::updateGateData(const QVector<RangeGateTable::GateData> &data)
{
    if (m_gateTable) {
        m_gateTable->setGateData(data);
    }
}

void WindFieldPage::onTimeWindowChanged(const QString &window)
{
    Q_UNUSED(window)
    // TODO: 加载对应时间窗口的数据
}

void WindFieldPage::onResolutionChanged(const QString &resolution)
{
    Q_UNUSED(resolution)
    // TODO: 切换分辨率
}

void WindFieldPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // 页面标题
    QLabel *titleLabel = new QLabel("风场", this);
    titleLabel->setStyleSheet("color: #333; font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // 创建筛选控件
    createHeader();

    // 创建图表
    createCharts();

    // 创建表格
    createTable();

    mainLayout->addStretch();
}

void WindFieldPage::createHeader()
{
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(16);

    // 时间窗口选择
    QLabel *timeLabel = new QLabel("时间窗口:", this);
    timeLabel->setStyleSheet("color: #666;");
    headerLayout->addWidget(timeLabel);

    m_timeWindowCombo = new QComboBox(this);
    m_timeWindowCombo->addItems({"实时", "1分钟", "10分钟", "1小时"});
    m_timeWindowCombo->setStyleSheet("padding: 5px; border: 1px solid #ddd; border-radius: 4px;");
    connect(m_timeWindowCombo, &QComboBox::currentTextChanged,
            this, &WindFieldPage::onTimeWindowChanged);
    headerLayout->addWidget(m_timeWindowCombo);

    // 分辨率选择
    QLabel *resLabel = new QLabel("分辨率:", this);
    resLabel->setStyleSheet("color: #666;");
    headerLayout->addWidget(resLabel);

    m_resolutionCombo = new QComboBox(this);
    m_resolutionCombo->addItems({"10m", "20m"});
    m_resolutionCombo->setStyleSheet("padding: 5px; border: 1px solid #ddd; border-radius: 4px;");
    connect(m_resolutionCombo, &QComboBox::currentTextChanged,
            this, &WindFieldPage::onResolutionChanged);
    headerLayout->addWidget(m_resolutionCombo);

    headerLayout->addStretch();

    // 添加到主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addLayout(headerLayout);
    }
}

void WindFieldPage::createCharts()
{
    QHBoxLayout *chartLayout = new QHBoxLayout();
    chartLayout->setSpacing(16);

    // 风速廓线图
    QVBoxLayout *speedLayout = new QVBoxLayout();
    QLabel *speedLabel = new QLabel("风速廓线", this);
    speedLabel->setStyleSheet("color: #333; font-size: 12px; font-weight: bold;");
    speedLayout->addWidget(speedLabel);

    m_windSpeedChart = new WindTrendChart(this);
    speedLayout->addWidget(m_windSpeedChart);

    chartLayout->addLayout(speedLayout);

    // 风向廓线图
    QVBoxLayout *dirLayout = new QVBoxLayout();
    QLabel *dirLabel = new QLabel("风向廓线", this);
    dirLabel->setStyleSheet("color: #333; font-size: 12px; font-weight: bold;");
    dirLayout->addWidget(dirLabel);

    m_windDirectionChart = new WindTrendChart(this);
    dirLayout->addWidget(m_windDirectionChart);

    chartLayout->addLayout(dirLayout);

    // 添加到主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addLayout(chartLayout);
    }
}

void WindFieldPage::createTable()
{
    m_gateTable = new RangeGateTable(this);

    // 添加到主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addWidget(m_gateTable);
    }
}
