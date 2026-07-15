#include "WindFieldPage.h"

#include "ui/widgets/RangeGateTable.h"
#include "ui/widgets/WindTrendChart.h"
#include "ui/widgets/WindVectorFieldWidget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QGridLayout>
#include <QPixmap>
#include <QUrl>
#include <QVBoxLayout>
#include <QtMath>

namespace {
QFrame *createPanel(QWidget *parent)
{
    auto *panel = new QFrame(parent);
    panel->setObjectName("windFieldPanel");
    panel->setStyleSheet("QFrame#windFieldPanel { background:#ffffff; border:1px solid #d9dee5; border-radius:4px; }");
    return panel;
}

void populateHistoryWindows(QComboBox *combo)
{
    const struct { const char *label; int seconds; } windows[] = {
        {"1 分钟", 60}, {"5 分钟", 300}, {"10 分钟", 600}, {"30 分钟", 1800},
        {"1 小时", 3600}, {"2 小时", 7200}, {"6 小时", 21600},
        {"12 小时", 43200}, {"24 小时", 86400}
    };
    for (const auto &window : windows) {
        combo->addItem(QString::fromUtf8(window.label), window.seconds);
    }
}
}

WindFieldPage::WindFieldPage(QWidget *parent)
    : QWidget(parent)
    , m_timeWindowCombo(nullptr)
    , m_resolutionCombo(nullptr)
    , m_speedHistoryWindowCombo(nullptr)
    , m_directionHistoryWindowCombo(nullptr)
    , m_headerLayout(nullptr)
    , m_windSpeedChart(nullptr)
    , m_windDirectionChart(nullptr)
    , m_vectorField(nullptr)
    , m_gateTable(nullptr)
    , m_dataSourceLabel(nullptr)
    , m_dataSourceDetailLabel(nullptr)
    , m_productTypeValue(nullptr)
    , m_observationTimeValue(nullptr)
    , m_qualityValue(nullptr)
    , m_algorithmValue(nullptr)
    , m_pyArtStatusLabel(nullptr)
    , m_runPyArtButton(nullptr)
    , m_openOutputButton(nullptr)
    , m_productPanel(nullptr)
    , m_productPreview(nullptr)
    , m_productPathLabel(nullptr)
    , m_speedSeriesTitle(nullptr)
    , m_directionSeriesTitle(nullptr)
    , m_seriesHeightAglM(-1.0)
    , m_speedHistoryWindowSeconds(60)
    , m_directionHistoryWindowSeconds(60)
{
    setupUI();
}

WindFieldPage::~WindFieldPage() = default;

void WindFieldPage::updateWindData(double speed, double direction, double heightAglM)
{
    if (heightAglM >= 0.0 && qAbs(heightAglM - m_seriesHeightAglM) > 0.1) {
        m_seriesHeightAglM = heightAglM;
        const QString height = QString::number(heightAglM, 'f', 0);
        m_speedSeriesTitle->setText(QStringLiteral("风速时序"));
        m_speedSeriesTitle->setToolTip(QStringLiteral("<b>风速时序</b><br>当前显示 %1 m AGL 高度处的水平风速，单位 m/s；横轴为观测时间。").arg(height));
        m_directionSeriesTitle->setText(QStringLiteral("风向时序"));
        m_directionSeriesTitle->setToolTip(QStringLiteral("<b>风向时序</b><br>当前显示 %1 m AGL 高度处的气象学来向，单位为度。").arg(height));
    }
    m_windSpeedChart->addDataPoint(speed);
    m_windDirectionChart->addDataPoint(direction);
}

void WindFieldPage::setSpeedHistory(const QVector<QPointF> &speedMps, int windowSeconds)
{
    m_speedHistoryWindowSeconds = qBound(60, windowSeconds, 24 * 60 * 60);
    if (!m_windSpeedChart) return;
    m_windSpeedChart->setWindowSeconds(m_speedHistoryWindowSeconds);
    if (!speedMps.isEmpty()) m_windSpeedChart->setData(speedMps);
}

void WindFieldPage::setDirectionHistory(const QVector<QPointF> &directionDeg, int windowSeconds)
{
    m_directionHistoryWindowSeconds = qBound(60, windowSeconds, 24 * 60 * 60);
    if (!m_windDirectionChart) return;
    m_windDirectionChart->setWindowSeconds(m_directionHistoryWindowSeconds);
    if (!directionDeg.isEmpty()) m_windDirectionChart->setData(directionDeg);
}

void WindFieldPage::updateGateData(const QVector<RangeGateTable::GateData> &data)
{
    m_gateTable->setGateData(data);
    QVector<WindVectorFieldWidget::ProfileLevel> levels;
    for (const RangeGateTable::GateData &gate : data) {
        if (gate.validBeams < 3 || gate.validBeams > 5) continue;
        levels.append({gate.heightM, gate.eastwardMps, gate.northwardMps,
                       gate.upwardMps, gate.confidence, gate.radialVelocityMps});
    }
    if (!levels.isEmpty()) {
        m_vectorField->setWindProfile(levels, QStringLiteral("0x8105 五固定波束 / 加权最小二乘"));
    }
}

void WindFieldPage::showRadarDirectSource(const QDateTime &timestamp, int validGateCount,
                                          int gateCount, double meanCnrDb)
{
    m_dataSourceLabel->setText(QStringLiteral("雷达 TCP / 0x8100"));
    m_productTypeValue->setText(QStringLiteral("垂直风廓线"));
    m_observationTimeValue->setText(timestamp.isValid()
        ? timestamp.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")) : QStringLiteral("等待数据"));
    m_qualityValue->setText(gateCount > 0
        ? QStringLiteral("有效层 %1/%2%3").arg(validGateCount).arg(gateCount)
            .arg(qIsFinite(meanCnrDb)
                ? QStringLiteral(" · CNR %1 dB").arg(meanCnrDb, 0, 'f', 1) : QString())
        : QStringLiteral("尚未评估"));
    m_algorithmValue->setText(QStringLiteral("雷达端产品 · 未经过 Py-ART"));
    m_dataSourceDetailLabel->setText(QStringLiteral("安装基准：正北  |  风向：气象学来向  |  二维网格：不可用  |  原始径向扫描：未接入"));
    m_vectorField->clearField(QStringLiteral("当前产品不支持二维风场"),
        QStringLiteral("0x8100 仅包含分层风速和风向，不包含水平空间网格。等待 0x8105 原始径向速度扫描或标准网格产品。"));
}

void WindFieldPage::showFiveBeamSource(const QDateTime &timestamp, int validGateCount,
                                       int gateCount, double meanCnrDb, quint32 scanId)
{
    m_dataSourceLabel->setText(QStringLiteral("五波束原始观测 / 0x8105"));
    m_productTypeValue->setText(QStringLiteral("三分量风廓线"));
    m_observationTimeValue->setText(timestamp.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    m_qualityValue->setText(QStringLiteral("有效层 %1/%2 · CNR %3 dB")
        .arg(validGateCount).arg(gateCount).arg(meanCnrDb, 0, 'f', 1));
    m_algorithmValue->setText(QStringLiteral("固定五波束 WLS · 扫描 %1").arg(scanId));
    m_dataSourceDetailLabel->setText(
        QStringLiteral("安装基准：正北  |  法向90° + 四斜束75°  |  方位45/135/225/315°  |  ENU三分量  |  气象学来向"));
}

void WindFieldPage::setPyArtAvailability(bool available, const QString &version, const QString &message)
{
    m_runPyArtButton->setEnabled(available);
    if (available) {
        m_pyArtStatusLabel->setText(QStringLiteral("算法环境：Py-ART %1 就绪").arg(version));
        m_pyArtStatusLabel->setStyleSheet("color:#475467; font-size:12px;");
    } else {
        m_pyArtStatusLabel->setText(message.isEmpty() ? QStringLiteral("Py-ART 不可用") : message);
        m_pyArtStatusLabel->setStyleSheet("color:#b42318; font-size:12px;");
    }
}

void WindFieldPage::setPyArtBusy(bool busy)
{
    m_runPyArtButton->setEnabled(!busy);
    m_runPyArtButton->setText(busy ? QStringLiteral("正在执行...") : QStringLiteral("算法自检"));
    if (busy) {
        m_pyArtStatusLabel->setText(QStringLiteral("算法环境：正在执行 VAD 自检"));
        m_pyArtStatusLabel->setStyleSheet("color:#175cd3; font-size:12px;");
    }
}

void WindFieldPage::showPyArtResult(const QJsonObject &result)
{
    m_productDirectory.clear();
    const QJsonObject algorithm = result.value(QStringLiteral("algorithm")).toObject();
    const QJsonObject summary = result.value(QStringLiteral("summary")).toObject();
    m_dataSourceLabel->setText(QStringLiteral("算法自检 / 合成扫描"));
    m_productTypeValue->setText(QStringLiteral("VAD 垂直风廓线"));
    m_observationTimeValue->setText(QDateTime::fromString(
        result.value(QStringLiteral("observationTimeUtc")).toString(), Qt::ISODate)
        .toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    m_qualityValue->setText(QStringLiteral("有效 %1/%2 层")
        .arg(summary.value(QStringLiteral("validLevelCount")).toInt())
        .arg(summary.value(QStringLiteral("levelCount")).toInt()));
    m_algorithmValue->setText(QStringLiteral("%1 / Py-ART %2")
        .arg(algorithm.value(QStringLiteral("name")).toString(),
             algorithm.value(QStringLiteral("pyartVersion")).toString()));
    m_dataSourceDetailLabel->setText(QStringLiteral("验证数据，不代表现场实测  |  二维网格：不可用  |  风向约定：气象学来向"));
    m_pyArtStatusLabel->setText(QStringLiteral("算法环境：自检通过 · %1 ms")
        .arg(result.value(QStringLiteral("elapsedMs")).toDouble(), 0, 'f', 1));
    m_pyArtStatusLabel->setStyleSheet("color:#13795b; font-size:12px;");
    m_runPyArtButton->setEnabled(true);
    m_runPyArtButton->setText(QStringLiteral("重新自检"));

    QVector<RangeGateTable::GateData> gates;
    const QJsonArray levels = result.value(QStringLiteral("levels")).toArray();
    for (const QJsonValue &value : levels) {
        const QJsonObject level = value.toObject();
        const double speed = level.value(QStringLiteral("windSpeedMps")).toDouble();
        const double direction = level.value(QStringLiteral("windFromDirectionDeg")).toDouble();
        const bool valid = level.value(QStringLiteral("qualityFlag")).toInt(1) == 0;
        gates.append({level.value(QStringLiteral("levelIndex")).toInt(),
                      level.value(QStringLiteral("heightAglM")).toDouble(),
                      level.value(QStringLiteral("heightAglM")).toDouble(),
                      speed, direction,
                      level.value(QStringLiteral("eastwardWindMps")).toDouble(qQNaN()),
                      level.value(QStringLiteral("northwardWindMps")).toDouble(qQNaN()), qQNaN(),
                      0.0, 0.0, 0.0, qQNaN(), valid ? 36 : 0, qQNaN(),
                      level.value(QStringLiteral("confidencePct")).toDouble(), {}});
    }
    updateGateData(gates);
    m_vectorField->clearField(QStringLiteral("VAD 风廓线不是二维网格风场"),
        QStringLiteral("本次自检只反演随高度变化的水平风。系统不会将单点廓线复制到整个平面制造伪二维产品。"));

    QString imagePath;
    const QJsonArray products = result.value(QStringLiteral("products")).toArray();
    for (const QJsonValue &value : products) {
        const QJsonObject product = value.toObject();
        const QString path = product.value(QStringLiteral("path")).toString();
        if (m_productDirectory.isEmpty()) m_productDirectory = QFileInfo(path).absolutePath();
        if (product.value(QStringLiteral("type")).toString() == QStringLiteral("wind-profile-plot")) imagePath = path;
    }
    m_productPathLabel->setText(QStringLiteral("标准产品目录：%1").arg(QDir::toNativeSeparators(m_productDirectory)));
    m_openOutputButton->setEnabled(!m_productDirectory.isEmpty());
    const QPixmap image(imagePath);
    if (!image.isNull()) m_productPreview->setPixmap(image.scaledToWidth(760, Qt::SmoothTransformation));
    m_productPanel->show();
}

void WindFieldPage::showPyArtError(const QString &message)
{
    m_pyArtStatusLabel->setText(QStringLiteral("Py-ART 失败：%1").arg(message));
    m_pyArtStatusLabel->setStyleSheet("color:#b42318; font-size:12px;");
    m_runPyArtButton->setEnabled(true);
    m_runPyArtButton->setText(QStringLiteral("重新自检"));
}

void WindFieldPage::onTimeWindowChanged(const QString &window)
{
    Q_UNUSED(window)
    if (!m_vectorField) return;
    m_vectorField->setHistoryWindowSeconds(m_timeWindowCombo->currentData().toInt());
}
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
    createAlgorithmPanel();
    createVectorField();
    showRadarDirectSource();
    createCharts();
    createTable();
    createProductPreview();
}

void WindFieldPage::createVectorField()
{
    auto *panel = createPanel(this);
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(18, 14, 18, 12);
    panelLayout->setSpacing(4);

    auto *title = new QLabel(QStringLiteral("时间-高度风矢量剖面"), panel);
    title->setStyleSheet("color:#1d2939; font-size:15px; font-weight:600;");
    title->setToolTip(QStringLiteral("<b>时间-高度风矢量剖面</b><br>横轴为上位机接收时间，纵轴为雷达安装基准以上高度（AGL）。风羽方向表示水平风矢量方向，颜色表示水平风速；水平虚线为高度标定线，竖直虚线为时间标定线。"));
    panelLayout->addWidget(title);
    auto *subtitle = new QLabel(
        QStringLiteral("风羽表示水平风向与大小  ·  颜色表示水平风速  ·  高度为 AGL"), panel);
    subtitle->setStyleSheet("color:#667085; font-size:11px;");
    subtitle->setToolTip(title->toolTip());
    panelLayout->addWidget(subtitle);

    m_vectorField = new WindVectorFieldWidget(panel);
    panelLayout->addWidget(m_vectorField, 1);
    static_cast<QVBoxLayout *>(layout())->addWidget(panel);
}

void WindFieldPage::createAlgorithmPanel()
{
    auto *panel = createPanel(this);
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(18, 14, 18, 14);
    panelLayout->setSpacing(12);
    auto *metadata = new QGridLayout();
    metadata->setHorizontalSpacing(24);
    metadata->setVerticalSpacing(4);
    const auto addField = [panel, metadata](int column, const QString &name,
                                            const QString &help, QLabel **value) {
        auto *label = new QLabel(name, panel);
        label->setStyleSheet("color:#667085; font-size:11px;");
        label->setToolTip(help);
        *value = new QLabel(QStringLiteral("—"), panel);
        (*value)->setStyleSheet("color:#1d2939; font-size:13px; font-weight:600;");
        (*value)->setToolTip(help);
        metadata->addWidget(label, 0, column);
        metadata->addWidget(*value, 1, column);
        metadata->setColumnStretch(column, 1);
    };
    addField(0, QStringLiteral("数据源"),
             QStringLiteral("<b>数据源</b><br>标识当前风场产品对应的通信消息类型及原始观测量，例如 0x8105 五波束径向速度扫描。"),
             &m_dataSourceLabel);
    addField(1, QStringLiteral("产品类型"),
             QStringLiteral("<b>产品类型</b><br>描述当前数据产品的空间维度和物理分量；三分量风场包含东向 U、北向 V 和垂直 W。"),
             &m_productTypeValue);
    addField(2, QStringLiteral("观测时间"),
             QStringLiteral("<b>观测时间</b><br>雷达观测帧携带的采集时间，用于数据追溯；数据库历史窗口采用上位机接收时间建立索引。"),
             &m_observationTimeValue);
    addField(3, QStringLiteral("质量摘要"),
             QStringLiteral("<b>质量摘要</b><br>显示当前有效反演层数/总层数以及平均载噪比 CNR。综合置信度仅在“设备状态 > 数据质量”中展示。"),
             &m_qualityValue);
    addField(4, QStringLiteral("处理算法"),
             QStringLiteral("<b>处理算法</b><br>标识由径向速度获得风矢量所使用的反演方法、波束几何及扫描编号。"),
             &m_algorithmValue);
    panelLayout->addLayout(metadata);
    auto *footer = new QHBoxLayout();
    m_dataSourceDetailLabel = new QLabel(panel);
    m_dataSourceDetailLabel->setStyleSheet("color:#667085; font-size:11px;");
    footer->addWidget(m_dataSourceDetailLabel, 1);
    m_pyArtStatusLabel = new QLabel(QStringLiteral("正在检查 Py-ART 环境..."), panel);
    m_pyArtStatusLabel->setStyleSheet("color:#52606d; font-size:12px;");
    footer->addWidget(m_pyArtStatusLabel);
    m_runPyArtButton = new QPushButton(QStringLiteral("算法自检"), panel);
    m_runPyArtButton->setEnabled(false);
    m_runPyArtButton->setStyleSheet("QPushButton { min-height:28px; padding:0 12px; color:#344054; background:#ffffff; border:1px solid #bfc9d4; border-radius:3px; font-size:12px; } QPushButton:hover { background:#f2f4f7; } QPushButton:disabled { color:#98a2b3; background:#f2f4f7; }");
    connect(m_runPyArtButton, &QPushButton::clicked, this, &WindFieldPage::pyArtValidationRequested);
    footer->addWidget(m_runPyArtButton);
    panelLayout->addLayout(footer);
    static_cast<QVBoxLayout *>(layout())->addWidget(panel);
}

void WindFieldPage::createHeader()
{
    auto *timeLabel = new QLabel(QStringLiteral("风矢量剖面窗口"), this);
    timeLabel->setStyleSheet("color:#52606d; font-size:12px;");
    timeLabel->setToolTip(QStringLiteral("控制时间-高度风矢量剖面的独立历史窗口，不影响风速时序和风向时序。"));
    m_timeWindowCombo = new QComboBox(this);
    populateHistoryWindows(m_timeWindowCombo);
    m_timeWindowCombo->setMinimumWidth(96);
    m_timeWindowCombo->setStyleSheet("QComboBox { min-height:28px; padding:0 8px; border:1px solid #bfc9d4; border-radius:3px; }");
    connect(m_timeWindowCombo, &QComboBox::currentTextChanged, this, &WindFieldPage::onTimeWindowChanged);

    auto *resolutionLabel = new QLabel(QStringLiteral("距离分辨率"), this);
    resolutionLabel->setStyleSheet("color:#52606d; font-size:12px; margin-left:8px;");
    resolutionLabel->setToolTip(QStringLiteral("相邻距离门中心之间的斜距间隔；实际可选值必须与雷达当前探测配置一致。"));
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
    auto *speedHeader = new QHBoxLayout();
    m_speedSeriesTitle = new QLabel(QStringLiteral("风速时序"), speedPanel);
    m_speedSeriesTitle->setStyleSheet("color:#172b3d; font-size:16px; font-weight:600;");
    m_speedSeriesTitle->setToolTip(QStringLiteral("<b>风速时序</b><br>表示选定时间窗口内水平风速随上位机接收时间的变化，单位为 m/s。历史数据来自 SQLite 风场记录，长窗口按时间桶聚合。"));
    speedHeader->addWidget(m_speedSeriesTitle);
    speedHeader->addStretch();
    auto *speedWindowLabel = new QLabel(QStringLiteral("时间窗口"), speedPanel);
    speedWindowLabel->setStyleSheet("color:#52606d; font-size:12px;");
    speedWindowLabel->setToolTip(QStringLiteral("仅控制风速时序图的历史范围。"));
    speedHeader->addWidget(speedWindowLabel);
    m_speedHistoryWindowCombo = new QComboBox(speedPanel);
    populateHistoryWindows(m_speedHistoryWindowCombo);
    m_speedHistoryWindowCombo->setMinimumWidth(104);
    m_speedHistoryWindowCombo->setStyleSheet("QComboBox { min-height:28px; padding:0 8px; border:1px solid #bfc9d4; border-radius:3px; }");
    speedHeader->addWidget(m_speedHistoryWindowCombo);
    speedLayout->addLayout(speedHeader);
    m_windSpeedChart = new WindTrendChart(speedPanel);
    m_windSpeedChart->setSeriesType(WindTrendChart::SeriesType::WindSpeed);
    speedLayout->addWidget(m_windSpeedChart);
    connect(m_speedHistoryWindowCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_speedHistoryWindowSeconds = m_speedHistoryWindowCombo->itemData(index).toInt();
        emit speedHistoryWindowChanged(m_speedHistoryWindowSeconds);
    });
    row->addWidget(speedPanel, 1);

    auto *directionPanel = createPanel(this);
    auto *directionLayout = new QVBoxLayout(directionPanel);
    directionLayout->setContentsMargins(14, 12, 14, 14);
    auto *directionHeader = new QHBoxLayout();
    m_directionSeriesTitle = new QLabel(QStringLiteral("风向时序"), directionPanel);
    m_directionSeriesTitle->setStyleSheet("color:#172b3d; font-size:16px; font-weight:600;");
    m_directionSeriesTitle->setToolTip(QStringLiteral("<b>风向时序</b><br>表示选定时间窗口内气象学风向随时间的变化，单位为度。0°/360°表示北风；跨越正北时曲线断开，避免产生虚假的方向跳变。"));
    directionHeader->addWidget(m_directionSeriesTitle);
    directionHeader->addStretch();
    auto *directionWindowLabel = new QLabel(QStringLiteral("时间窗口"), directionPanel);
    directionWindowLabel->setStyleSheet("color:#52606d; font-size:12px;");
    directionWindowLabel->setToolTip(QStringLiteral("仅控制风向时序图的历史范围。"));
    directionHeader->addWidget(directionWindowLabel);
    m_directionHistoryWindowCombo = new QComboBox(directionPanel);
    populateHistoryWindows(m_directionHistoryWindowCombo);
    m_directionHistoryWindowCombo->setMinimumWidth(104);
    m_directionHistoryWindowCombo->setStyleSheet(m_speedHistoryWindowCombo->styleSheet());
    directionHeader->addWidget(m_directionHistoryWindowCombo);
    directionLayout->addLayout(directionHeader);
    m_windDirectionChart = new WindTrendChart(directionPanel);
    m_windDirectionChart->setSeriesType(WindTrendChart::SeriesType::WindDirection);
    directionLayout->addWidget(m_windDirectionChart);
    connect(m_directionHistoryWindowCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_directionHistoryWindowSeconds = m_directionHistoryWindowCombo->itemData(index).toInt();
        emit directionHistoryWindowChanged(m_directionHistoryWindowSeconds);
    });
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

void WindFieldPage::createProductPreview()
{
    m_productPanel = createPanel(this);
    auto *panelLayout = new QVBoxLayout(m_productPanel);
    panelLayout->setContentsMargins(16, 14, 16, 16);
    panelLayout->setSpacing(10);
    auto *header = new QHBoxLayout();
    auto *title = new QLabel(QStringLiteral("Py-ART 标准产品预览"), m_productPanel);
    title->setStyleSheet("color:#263442; font-size:14px; font-weight:600;");
    header->addWidget(title);
    header->addStretch();
    m_openOutputButton = new QPushButton(QStringLiteral("打开结果目录"), m_productPanel);
    m_openOutputButton->setEnabled(false);
    m_openOutputButton->setStyleSheet("QPushButton { min-height:30px; padding:0 12px; color:#344054; background:white; border:1px solid #bfc9d4; border-radius:4px; } QPushButton:hover { background:#f2f5f7; }");
    connect(m_openOutputButton, &QPushButton::clicked, this, [this] {
        if (!m_productDirectory.isEmpty()) QDesktopServices::openUrl(QUrl::fromLocalFile(m_productDirectory));
    });
    header->addWidget(m_openOutputButton);
    panelLayout->addLayout(header);
    m_productPathLabel = new QLabel(m_productPanel);
    m_productPathLabel->setStyleSheet("color:#667085; font-size:12px;");
    m_productPathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    panelLayout->addWidget(m_productPathLabel);
    m_productPreview = new QLabel(m_productPanel);
    m_productPreview->setAlignment(Qt::AlignCenter);
    m_productPreview->setMinimumHeight(260);
    m_productPreview->setStyleSheet("background:#f8fafc; border:1px solid #e4e7ec;");
    panelLayout->addWidget(m_productPreview);
    m_productPanel->hide();
    static_cast<QVBoxLayout *>(layout())->addWidget(m_productPanel);
}
