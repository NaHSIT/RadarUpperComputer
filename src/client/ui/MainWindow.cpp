#include "MainWindow.h"

#include "navigation/NavigationBar.h"
#include "pages/BeamPage.h"
#include "pages/DashboardPage.h"
#include "pages/DeviceHealthPage.h"
#include "pages/DeviceStatusPage.h"
#include "pages/DataQualityPage.h"
#include "pages/SettingsPage.h"
#include "pages/SpectrumPage.h"
#include "pages/WindFieldPage.h"
#include "services/AlarmService.h"
#include "services/DeviceService.h"
#include "services/DataService.h"
#include "algorithms/PyArtWindService.h"

#include <QDateTime>
#include <QApplication>
#include <QCoreApplication>
#include <QDockWidget>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolTip>
#include <QTimer>
#include <QMouseEvent>
#include <QPalette>
#include <QVBoxLayout>
#include <QtMath>

namespace {
constexpr int kHistoryDisplayPointBudget = 480;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_deviceService(nullptr)
    , m_alarmService(nullptr)
    , m_pyArtWindService(nullptr)
    , m_dataService(nullptr)
    , m_navigationBar(nullptr)
    , m_contentStack(nullptr)
    , m_connectionStatusLabel(nullptr)
    , m_alarmCountLabel(nullptr)
    , m_eventLog(nullptr)
    , m_historyRefreshTimer(new QTimer(this))
    , m_dashboardPage(nullptr)
    , m_windFieldPage(nullptr)
    , m_beamPage(nullptr)
    , m_spectrumPage(nullptr)
    , m_deviceHealthPage(nullptr)
    , m_deviceStatusPage(nullptr)
    , m_dataQualityPage(nullptr)
    , m_settingsPage(nullptr)
{
    m_historyRefreshTimer->setSingleShot(true);
    m_historyRefreshTimer->setInterval(30000);
    setupUI();
    qApp->installEventFilter(this);
    QPalette toolTipPalette;
    toolTipPalette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
    toolTipPalette.setColor(QPalette::ToolTipText, QColor("#101828"));
    QToolTip::setPalette(toolTipPalette);
    QToolTip::setFont(QFont(QStringLiteral("Microsoft YaHei"), 11));
    setWindowTitle(QStringLiteral("测风雷达运行监控系统"));
    resize(1440, 900);
    setMinimumSize(1180, 720);
}

MainWindow::~MainWindow() = default;

void MainWindow::setDataService(DataService *service)
{
    m_dataService = service;
    if (!m_dataService || !m_dashboardPage) return;

    connect(m_dashboardPage, &DashboardPage::speedHistoryWindowChanged,
            this, &MainWindow::refreshDashboardSpeedHistory);
    connect(m_dashboardPage, &DashboardPage::directionHistoryWindowChanged,
            this, &MainWindow::refreshDashboardDirectionHistory);
    connect(m_windFieldPage, &WindFieldPage::speedHistoryWindowChanged,
            this, &MainWindow::refreshWindFieldSpeedHistory);
    connect(m_windFieldPage, &WindFieldPage::directionHistoryWindowChanged,
            this, &MainWindow::refreshWindFieldDirectionHistory);
    connect(m_dataService, &DataService::historyStored, this, [this](qint64) {
        if (!m_historyRefreshTimer->isActive()) m_historyRefreshTimer->start();
    });
    connect(m_historyRefreshTimer, &QTimer::timeout, this, [this] {
        refreshAllWindHistory();
    });
    connect(m_dataService, &DataService::storageError, this,
            [this](const QString &message) {
        appendLog(QStringLiteral("历史数据库错误：%1").arg(message));
    });
    appendLog(QStringLiteral("历史数据库：%1")
        .arg(QDir::toNativeSeparators(m_dataService->databasePath())));
    refreshAllWindHistory();
}

void MainWindow::refreshDashboardSpeedHistory(int seconds)
{
    if (!m_dataService || !m_dashboardPage) return;
    const DataService::WindSeries series = m_dataService->queryWindSeries(seconds, kHistoryDisplayPointBudget);
    m_dashboardPage->setSpeedHistory(series.speedMps, seconds);
}

void MainWindow::refreshDashboardDirectionHistory(int seconds)
{
    if (!m_dataService || !m_dashboardPage) return;
    const DataService::WindSeries series = m_dataService->queryWindSeries(seconds, kHistoryDisplayPointBudget);
    m_dashboardPage->setDirectionHistory(series.directionDeg, seconds);
}

void MainWindow::refreshWindFieldSpeedHistory(int seconds)
{
    if (!m_dataService || !m_windFieldPage) return;
    const DataService::WindSeries series = m_dataService->queryWindSeries(seconds, kHistoryDisplayPointBudget);
    m_windFieldPage->setSpeedHistory(series.speedMps, seconds);
}

void MainWindow::refreshWindFieldDirectionHistory(int seconds)
{
    if (!m_dataService || !m_windFieldPage) return;
    const DataService::WindSeries series = m_dataService->queryWindSeries(seconds, kHistoryDisplayPointBudget);
    m_windFieldPage->setDirectionHistory(series.directionDeg, seconds);
}

void MainWindow::refreshAllWindHistory()
{
    if (!m_dataService || !m_dashboardPage || !m_windFieldPage) return;

    QHash<int, DataService::WindSeries> cache;
    const auto seriesFor = [this, &cache](int seconds) {
        if (!cache.contains(seconds)) {
            cache.insert(seconds, m_dataService->queryWindSeries(seconds, kHistoryDisplayPointBudget));
        }
        return cache.value(seconds);
    };

    const int currentPage = m_contentStack ? m_contentStack->currentIndex() : PAGE_DASHBOARD;
    if (currentPage == PAGE_DASHBOARD) {
        const int speedWindow = m_dashboardPage->speedHistoryWindowSeconds();
        const int directionWindow = m_dashboardPage->directionHistoryWindowSeconds();
        m_dashboardPage->setSpeedHistory(seriesFor(speedWindow).speedMps, speedWindow);
        m_dashboardPage->setDirectionHistory(seriesFor(directionWindow).directionDeg, directionWindow);
    } else if (currentPage == PAGE_WIND_FIELD) {
        const int speedWindow = m_windFieldPage->speedHistoryWindowSeconds();
        const int directionWindow = m_windFieldPage->directionHistoryWindowSeconds();
        m_windFieldPage->setSpeedHistory(seriesFor(speedWindow).speedMps, speedWindow);
        m_windFieldPage->setDirectionHistory(seriesFor(directionWindow).directionDeg, directionWindow);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        auto *label = qobject_cast<QLabel *>(watched);
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (label && mouseEvent->button() == Qt::LeftButton && !label->toolTip().isEmpty()) {
            const QPoint position = label->mapToGlobal(QPoint(0, label->height() + 6));
            QToolTip::showText(position, label->toolTip(), label, label->rect(), 20000);
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setDeviceService(DeviceService *service)
{
    m_deviceService = service;
    if (!m_deviceService) return;
    connect(m_deviceService, &DeviceService::connectionStateChanged,
            this, &MainWindow::onConnectionStateChanged);
    connect(m_deviceService, &DeviceService::errorOccurred, this,
            [this](const QString &error) { appendLog(QStringLiteral("连接错误：%1").arg(error)); });
    connect(m_deviceService, &DeviceService::windProfileUpdated, this,
            [this](WindProfile *profile) {
        if (!profile || !m_dashboardPage || !m_windFieldPage) return;

        QVector<RangeGateTable::GateData> gateData;
        gateData.reserve(profile->rangeGates().size());
        for (const RangeGate *gate : profile->rangeGates()) {
            gateData.append({
                gate->gateIndex(), gate->distanceM(), gate->heightM(),
                gate->windSpeedMps(), gate->windDirectionDeg(),
                gate->eastwardWindMps(), gate->northwardWindMps(), gate->upwardWindMps(),
                gate->turbulenceIntensity(),
                gate->verticalShear(), gate->horizontalShear(),
                gate->cnrDb().isEmpty() ? 0.0 : gate->cnrDb().first(),
                gate->validBeamCount(), gate->retrievalResidualMps(), gate->confidence(),
                gate->radialWindSpeedMps()
            });
        }

        m_dashboardPage->updateWindData(profile->hubWindSpeedMps(), profile->hubWindDirectionDeg(),
                                         profile->hubHeightM(),
                                         profile->validGateCount(),
                                         profile->blindRatio(),
                                         m_alarmService ? m_alarmService->activeAlarmCount() : 0);
        m_windFieldPage->updateWindData(profile->hubWindSpeedMps(), profile->hubWindDirectionDeg(),
                                        profile->hubHeightM());
        m_windFieldPage->updateGateData(gateData);
        double cnrSum = 0.0;
        double residualSum = 0.0;
        int cnrCount = 0;
        int residualCount = 0;
        for (const RangeGateTable::GateData &gate : gateData) {
            if (qIsFinite(gate.cnrAvg)) { cnrSum += gate.cnrAvg; ++cnrCount; }
            if (qIsFinite(gate.retrievalResidualMps)) {
                residualSum += gate.retrievalResidualMps;
                ++residualCount;
            }
        }
        const QString qualitySource = profile->retrievalMethod() == WindRetrievalMethod::FiveBeamLeastSquares
            ? QStringLiteral("0x8105 五波束径向速度 + 加权最小二乘")
            : QStringLiteral("0x8100 雷达端风廓线产品");
        m_dashboardPage->setDataSource(QStringLiteral("数据来源  ·  %1").arg(qualitySource));
        if (m_dataQualityPage) {
            m_dataQualityPage->updateQuality(
                profile->confidence(), profile->validGateCount(), profile->gateCount(),
                profile->blindRatio(), cnrCount > 0 ? cnrSum / cnrCount : 0.0,
                residualCount > 0 ? residualSum / residualCount : 0.0, qualitySource);
        }
        const double meanCnrDb = cnrCount > 0 ? cnrSum / cnrCount : qQNaN();
        if (profile->retrievalMethod() == WindRetrievalMethod::FiveBeamLeastSquares) {
            m_windFieldPage->showFiveBeamSource(profile->timestampUtc(), profile->validGateCount(),
                                                profile->gateCount(), meanCnrDb,
                                                profile->sourceScanId());
        } else {
            m_windFieldPage->showRadarDirectSource(profile->timestampUtc(), profile->validGateCount(),
                                                   profile->gateCount(), meanCnrDb);
        }
        const double cnrDb = gateData.isEmpty() ? 0.0 : gateData.first().cnrAvg;
        if (m_beamPage && !profile->beamStates().isEmpty()) {
            m_beamPage->updateBeamData(profile->beamStates());
        } else if (m_beamPage) {
            m_beamPage->updateSimulationData(cnrDb, profile->validGateCount());
        }
        if (!profile->beamStates().isEmpty()) {
            for (const BeamState *beam : profile->beamStates()) {
                if (!beam) continue;
                double cnrSum = 0.0;
                int cnrCount = 0;
                int validGates = 0;
                const QVector<double> cnr = beam->cnrDbByGate();
                const QVector<double> radialVelocity = beam->rwsByGate();
                const int beamGateCount = qMax(cnr.size(), radialVelocity.size());
                for (int gate = 0; gate < beamGateCount; ++gate) {
                    if (gate < cnr.size() && qIsFinite(cnr.at(gate))) {
                        cnrSum += cnr.at(gate);
                        ++cnrCount;
                    }
                    if (gate < radialVelocity.size() && qIsFinite(radialVelocity.at(gate))
                        && gate < cnr.size() && qIsFinite(cnr.at(gate))
                        && cnr.at(gate) >= -22.0) ++validGates;
                }
                const int index = static_cast<int>(beam->beamId());
                const QString name = index == 0 ? QStringLiteral("法向") : QStringLiteral("斜束%1").arg(index);
                m_dashboardPage->updateBeamStatus(index, name, beam->azimuthDeg(), beam->elevationDeg(),
                    beam->status() == BeamStatus::Normal ? QStringLiteral("normal") : QStringLiteral("warning"),
                    cnrCount > 0 ? cnrSum / cnrCount : 0.0, validGates);
            }
        } else {
            const double azimuths[] = {0.0, 45.0, 135.0, 225.0, 315.0};
            for (int beam = 0; beam < 5; ++beam) {
                m_dashboardPage->updateBeamStatus(beam,
                    beam == 0 ? QStringLiteral("法向") : QStringLiteral("斜束%1").arg(beam),
                    azimuths[beam], beam == 0 ? 90.0 : 75.0,
                    QStringLiteral("warning"), cnrDb, profile->validGateCount());
            }
        }
    });
}

void MainWindow::setPyArtWindService(PyArtWindService *service)
{
    m_pyArtWindService = service;
    if (!m_pyArtWindService || !m_windFieldPage) return;

    connect(m_pyArtWindService, &PyArtWindService::availabilityChanged, this,
            [this](bool available, const QJsonObject &details) {
        const QString version = details.value(QStringLiteral("pyartVersion")).toString();
        const QString error = details.value(QStringLiteral("error")).toString();
        m_windFieldPage->setPyArtAvailability(available, version, error);
        appendLog(available
            ? QStringLiteral("Py-ART %1 算法环境已就绪。").arg(version)
            : QStringLiteral("Py-ART 算法环境不可用：%1").arg(error));
    });
    connect(m_pyArtWindService, &PyArtWindService::busyChanged,
            m_windFieldPage, &WindFieldPage::setPyArtBusy);
    connect(m_pyArtWindService, &PyArtWindService::resultReady, this,
            [this](const QJsonObject &result) {
        m_windFieldPage->showPyArtResult(result);
        navigateTo(PAGE_WIND_FIELD);
        const QJsonObject summary = result.value(QStringLiteral("summary")).toObject();
        appendLog(QStringLiteral("Py-ART 风场反演完成：有效层 %1/%2，耗时 %3 ms。")
            .arg(summary.value(QStringLiteral("validLevelCount")).toInt())
            .arg(summary.value(QStringLiteral("levelCount")).toInt())
            .arg(result.value(QStringLiteral("elapsedMs")).toDouble(), 0, 'f', 1));
    });
    connect(m_pyArtWindService, &PyArtWindService::taskFailed, this,
            [this](const QString &code, const QString &message) {
        m_windFieldPage->showPyArtError(QStringLiteral("%1 · %2").arg(code, message));
        appendLog(QStringLiteral("Py-ART 算法失败：%1 · %2").arg(code, message));
    });
    connect(m_pyArtWindService, &PyArtWindService::processLog,
            this, [this](const QString &message) { appendLog(QStringLiteral("Py-ART：%1").arg(message)); });
    connect(m_windFieldPage, &WindFieldPage::pyArtValidationRequested, this, [this] {
        const QString baseDirectory = QDir(QCoreApplication::applicationDirPath())
            .absoluteFilePath(QStringLiteral("data/pyart-output"));
        const QJsonObject request = PyArtWindService::createSyntheticValidationRequest(baseDirectory);
        appendLog(QStringLiteral("开始 Py-ART 标准验证。输入为 36 方位合成径向速度，结果目录：%1")
            .arg(QDir::toNativeSeparators(baseDirectory)));
        m_pyArtWindService->submit(request);
    });
}

void MainWindow::setAlarmService(AlarmService *service)
{
    m_alarmService = service;
    if (m_alarmService) {
        connect(m_alarmService, &AlarmService::alarmCountChanged,
                this, &MainWindow::onAlarmCountChanged);
    }
}

void MainWindow::navigateTo(int pageIndex)
{
    if (!m_contentStack || pageIndex < 0 || pageIndex >= m_contentStack->count()) return;
    m_contentStack->setCurrentIndex(pageIndex);
    m_navigationBar->setCurrentIndex(pageIndex);
    emit pageChanged(pageIndex);
    if (m_dataService && (pageIndex == PAGE_DASHBOARD || pageIndex == PAGE_WIND_FIELD)) {
        QTimer::singleShot(0, this, [this] { refreshAllWindHistory(); });
    }
}

void MainWindow::setupUI()
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setupNavigation();
    mainLayout->addWidget(m_navigationBar);
    setupContentArea();
    mainLayout->addWidget(m_contentStack, 1);
    setupStatusBar();
    setupEventLog();
    setStyleSheet(
        "QMainWindow { background:#f4f7fb; }"
        "QStackedWidget { background:#f4f7fb; }"
        "QScrollArea { border:0; }"
        "QScrollBar:vertical { width:10px; background:transparent; margin:8px 2px; }"
        "QScrollBar::handle:vertical { min-height:42px; background:#c5d0dc; border-radius:4px; }"
        "QScrollBar::handle:vertical:hover { background:#9dacbb; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }"
        "QStatusBar { background:#ffffff; border-top:1px solid #dce4ec; color:#5b6573; font-size:12px; }"
        "QStatusBar QLabel { padding:0 14px; }"
        "QDockWidget { color:#1d2939; font-size:13px; font-weight:600; }"
        "QDockWidget::title { background:#ffffff; border-top:1px solid #dce4ec; padding:9px 14px; }"
        "QToolTip { color:#101828; background-color:#ffffff; border:1px solid #98a2b3;"
        " padding:10px 12px; font-family:'Microsoft YaHei'; font-size:14px; }"
    );
}

void MainWindow::setupStatusBar()
{
    auto *bar = statusBar();
    bar->setFixedHeight(32);
    m_connectionStatusLabel = new QLabel(QStringLiteral("连接状态：未连接"), bar);
    m_alarmCountLabel = new QLabel(QStringLiteral("未恢复告警：0"), bar);
    m_connectionStatusLabel->setStyleSheet("color:#667085; font-weight:600;");
    m_alarmCountLabel->setStyleSheet("color:#667085;");
    bar->addWidget(m_connectionStatusLabel);
    bar->addWidget(m_alarmCountLabel);
    auto *version = new QLabel(QStringLiteral("测风雷达客户端 1.0.0"), bar);
    version->setStyleSheet("color:#98a2b3; font-size:11px;");
    bar->addPermanentWidget(version);
}

void MainWindow::setupEventLog()
{
    auto *logDock = new QDockWidget(QStringLiteral("运行日志"), this);
    logDock->setObjectName("eventLogDock");
    logDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    logDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    m_eventLog = new QPlainTextEdit(logDock);
    m_eventLog->setReadOnly(true);
    m_eventLog->setMaximumBlockCount(500);
    m_eventLog->setStyleSheet(
        "QPlainTextEdit { background:#17212b; color:#d9e2ec; border:0; padding:8px;"
        " font-family:Consolas, 'Microsoft YaHei'; font-size:12px; }"
    );
    logDock->setWidget(m_eventLog);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
    resizeDocks({logDock}, {150}, Qt::Vertical);
    appendLog(QStringLiteral("客户端已启动，等待设备连接。"));
}

void MainWindow::appendLog(const QString &message)
{
    if (!m_eventLog) return;
    const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    m_eventLog->appendPlainText(QStringLiteral("[%1] %2").arg(timestamp, message));
}

void MainWindow::setupNavigation()
{
    m_navigationBar = new NavigationBar(this);
    m_navigationBar->addItem("01", QStringLiteral("运行总览"), PAGE_DASHBOARD);
    m_navigationBar->addItem("02", QStringLiteral("风场数据"), PAGE_WIND_FIELD);
    m_navigationBar->addItem("03", QStringLiteral("设备状态"), PAGE_DEVICE_HEALTH);
    m_navigationBar->addItem("04", QStringLiteral("系统配置"), PAGE_SETTINGS);
    connect(m_navigationBar, &NavigationBar::itemClicked, this, &MainWindow::onNavigationClicked);
    m_navigationBar->setCurrentIndex(0);
}

void MainWindow::setupContentArea()
{
    m_contentStack = new QStackedWidget(this);
    createPages();
}

void MainWindow::createPages()
{
    const auto addScrollablePage = [this](QWidget *page) {
        auto *scrollArea = new QScrollArea(m_contentStack);
        scrollArea->setWidget(page);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setStyleSheet("QScrollArea { background:#f3f5f7; }");
        m_contentStack->addWidget(scrollArea);
    };

    m_dashboardPage = new DashboardPage(); addScrollablePage(m_dashboardPage);
    m_windFieldPage = new WindFieldPage(); addScrollablePage(m_windFieldPage);
    m_deviceStatusPage = new DeviceStatusPage();
    m_deviceHealthPage = m_deviceStatusPage->healthPage();
    m_dataQualityPage = m_deviceStatusPage->qualityPage();
    m_beamPage = m_deviceStatusPage->beamPage();
    m_spectrumPage = m_deviceStatusPage->spectrumPage();
    addScrollablePage(m_deviceStatusPage);
    m_settingsPage = new SettingsPage(); addScrollablePage(m_settingsPage);
    connect(m_settingsPage, &SettingsPage::connectRequested, this, [this](const QString &ip, int port) {
        appendLog(QStringLiteral("请求连接设备：%1:%2").arg(ip).arg(port));
        if (m_deviceService && !m_deviceService->connectDevice(ip, port)) {
            appendLog(QStringLiteral("连接请求未被接受。"));
        }
    });
    connect(m_settingsPage, &SettingsPage::disconnectRequested, this, [this] {
        appendLog(QStringLiteral("请求断开设备连接。"));
        if (m_deviceService) m_deviceService->disconnectDevice();
    });
}

void MainWindow::onNavigationClicked(int pageIndex) { navigateTo(pageIndex); }

void MainWindow::onConnectionStateChanged(ConnectionState state)
{
    if (m_beamPage) {
        m_beamPage->setConnectionState(state == ConnectionState::Online);
    }
    QString text = QStringLiteral("连接状态：未连接");
    QString color = "#687384";
    switch (state) {
    case ConnectionState::Connecting: text = QStringLiteral("连接状态：正在连接"); color = "#9a6700"; break;
    case ConnectionState::Online: text = QStringLiteral("连接状态：已连接"); color = "#16713b"; break;
    case ConnectionState::DataTimeout: text = QStringLiteral("连接状态：数据超时"); color = "#b42318"; break;
    case ConnectionState::ProtocolError: text = QStringLiteral("连接状态：协议异常"); color = "#b42318"; break;
    case ConnectionState::Offline: break;
    }
    m_connectionStatusLabel->setText(text);
    m_connectionStatusLabel->setStyleSheet(QStringLiteral("color:%1;").arg(color));
    appendLog(text);
}

void MainWindow::onAlarmCountChanged(int count)
{
    const int activeCount = m_alarmService ? m_alarmService->activeAlarmCount() : count;
    m_alarmCountLabel->setText(QStringLiteral("未恢复告警：%1").arg(activeCount));
    m_alarmCountLabel->setStyleSheet(activeCount > 0 ? "color:#b42318; font-weight:600;" : "color:#16713b;");
    if (m_dashboardPage) m_dashboardPage->updateAlarmCount(activeCount);
}
