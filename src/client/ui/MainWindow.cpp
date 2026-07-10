#include "MainWindow.h"

#include "navigation/NavigationBar.h"
#include "pages/BeamPage.h"
#include "pages/DashboardPage.h"
#include "pages/DeviceHealthPage.h"
#include "pages/SettingsPage.h"
#include "pages/SpectrumPage.h"
#include "pages/WindFieldPage.h"
#include "services/AlarmService.h"
#include "services/DeviceService.h"

#include <QDateTime>
#include <QDockWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QStatusBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_deviceService(nullptr)
    , m_alarmService(nullptr)
    , m_navigationBar(nullptr)
    , m_contentStack(nullptr)
    , m_connectionStatusLabel(nullptr)
    , m_alarmCountLabel(nullptr)
    , m_eventLog(nullptr)
    , m_dashboardPage(nullptr)
    , m_windFieldPage(nullptr)
    , m_beamPage(nullptr)
    , m_spectrumPage(nullptr)
    , m_deviceHealthPage(nullptr)
    , m_settingsPage(nullptr)
{
    setupUI();
    setWindowTitle(QStringLiteral("测风雷达运行监控系统"));
    resize(1440, 900);
    setMinimumSize(1180, 720);
}

MainWindow::~MainWindow() = default;

void MainWindow::setDeviceService(DeviceService *service)
{
    m_deviceService = service;
    if (!m_deviceService) return;
    connect(m_deviceService, &DeviceService::connectionStateChanged,
            this, &MainWindow::onConnectionStateChanged);
    connect(m_deviceService, &DeviceService::errorOccurred, this,
            [this](const QString &error) { appendLog(QStringLiteral("连接错误：%1").arg(error)); });
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
        "QMainWindow { background:#f3f5f7; }"
        "QStatusBar { background:#ffffff; border-top:1px solid #d9dee5; color:#5b6573; font-size:12px; }"
        "QStatusBar QLabel { padding:0 12px; }"
    );
}

void MainWindow::setupStatusBar()
{
    auto *bar = statusBar();
    bar->setFixedHeight(28);
    m_connectionStatusLabel = new QLabel(QStringLiteral("连接状态：未连接"), bar);
    m_alarmCountLabel = new QLabel(QStringLiteral("活动告警：0"), bar);
    m_connectionStatusLabel->setStyleSheet("color:#687384;");
    m_alarmCountLabel->setStyleSheet("color:#687384;");
    bar->addWidget(m_connectionStatusLabel);
    bar->addWidget(m_alarmCountLabel);
    auto *version = new QLabel(QStringLiteral("测风雷达客户端 1.0.0"), bar);
    version->setStyleSheet("color:#8a94a3;");
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
    m_navigationBar->addItem("03", QStringLiteral("波束监视"), PAGE_BEAM);
    m_navigationBar->addItem("04", QStringLiteral("频谱诊断"), PAGE_SPECTRUM);
    m_navigationBar->addItem("05", QStringLiteral("设备状态"), PAGE_DEVICE_HEALTH);
    m_navigationBar->addItem("06", QStringLiteral("系统配置"), PAGE_SETTINGS);
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
    m_beamPage = new BeamPage(); addScrollablePage(m_beamPage);
    m_spectrumPage = new SpectrumPage(); addScrollablePage(m_spectrumPage);
    m_deviceHealthPage = new DeviceHealthPage(); addScrollablePage(m_deviceHealthPage);
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
    m_alarmCountLabel->setText(QStringLiteral("活动告警：%1").arg(count));
    m_alarmCountLabel->setStyleSheet(count > 0 ? "color:#b42318; font-weight:600;" : "color:#16713b;");
}
