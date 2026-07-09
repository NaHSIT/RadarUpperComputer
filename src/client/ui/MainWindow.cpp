#include "MainWindow.h"
#include "navigation/NavigationBar.h"
#include "pages/DashboardPage.h"
#include "pages/WindFieldPage.h"
#include "pages/BeamPage.h"
#include "pages/SpectrumPage.h"
#include "pages/DeviceHealthPage.h"
#include "pages/SettingsPage.h"
#include "services/DeviceService.h"
#include "services/AlarmService.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_deviceService(nullptr)
    , m_alarmService(nullptr)
    , m_navigationBar(nullptr)
    , m_contentStack(nullptr)
    , m_connectionStatusLabel(nullptr)
    , m_alarmCountLabel(nullptr)
    , m_dashboardPage(nullptr)
    , m_windFieldPage(nullptr)
    , m_beamPage(nullptr)
    , m_spectrumPage(nullptr)
    , m_deviceHealthPage(nullptr)
    , m_settingsPage(nullptr)
{
    setupUI();
    setWindowTitle("测风雷达上位机 - 客户端");
    resize(1280, 800);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setDeviceService(DeviceService *service)
{
    m_deviceService = service;
    connect(m_deviceService, &DeviceService::connectionStateChanged,
            this, &MainWindow::onConnectionStateChanged);
}

void MainWindow::setAlarmService(AlarmService *service)
{
    m_alarmService = service;
    connect(m_alarmService, &AlarmService::alarmCountChanged,
            this, &MainWindow::onAlarmCountChanged);
}

void MainWindow::navigateTo(int pageIndex)
{
    if (m_contentStack && pageIndex >= 0 && pageIndex < m_contentStack->count()) {
        m_contentStack->setCurrentIndex(pageIndex);
        m_navigationBar->setCurrentIndex(pageIndex);
        emit pageChanged(pageIndex);
    }
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建导航栏
    setupNavigation();
    mainLayout->addWidget(m_navigationBar);

    // 创建内容区
    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    setupContentArea();
    contentLayout->addWidget(m_contentStack);

    mainLayout->addLayout(contentLayout);

    // 创建状态栏
    setupStatusBar();
}

void MainWindow::setupStatusBar()
{
    QStatusBar *statusBar = this->statusBar();

    // 连接状态
    m_connectionStatusLabel = new QLabel("离线", statusBar);
    m_connectionStatusLabel->setStyleSheet("color: gray; padding: 5px;");
    statusBar->addWidget(m_connectionStatusLabel);

    // 告警计数
    m_alarmCountLabel = new QLabel("告警: 0", statusBar);
    m_alarmCountLabel->setStyleSheet("color: green; padding: 5px;");
    statusBar->addWidget(m_alarmCountLabel);

    // 版本信息
    statusBar->addPermanentWidget(new QLabel("v1.0.0", statusBar));
}

void MainWindow::setupNavigation()
{
    m_navigationBar = new NavigationBar(this);

    // 添加导航项
    m_navigationBar->addItem("📊", "总览", PAGE_DASHBOARD);
    m_navigationBar->addItem("🌬️", "风场", PAGE_WIND_FIELD);
    m_navigationBar->addItem("📡", "波束", PAGE_BEAM);
    m_navigationBar->addItem("📈", "频谱", PAGE_SPECTRUM);
    m_navigationBar->addItem("💚", "设备健康", PAGE_DEVICE_HEALTH);
    m_navigationBar->addItem("⚙️", "设置", PAGE_SETTINGS);

    // 连接信号
    connect(m_navigationBar, &NavigationBar::itemClicked,
            this, &MainWindow::onNavigationClicked);

    // 默认选中总览页
    m_navigationBar->setCurrentIndex(0);
}

void MainWindow::setupContentArea()
{
    m_contentStack = new QStackedWidget(this);
    createPages();
}

void MainWindow::createPages()
{
    // 总览页
    m_dashboardPage = new DashboardPage(this);
    m_contentStack->addWidget(m_dashboardPage);

    // 风场页
    m_windFieldPage = new WindFieldPage(this);
    m_contentStack->addWidget(m_windFieldPage);

    // 波束页
    m_beamPage = new BeamPage(this);
    m_contentStack->addWidget(m_beamPage);

    // 频谱页
    m_spectrumPage = new SpectrumPage(this);
    m_contentStack->addWidget(m_spectrumPage);

    // 设备健康页
    m_deviceHealthPage = new DeviceHealthPage(this);
    m_contentStack->addWidget(m_deviceHealthPage);

    // 设置页
    m_settingsPage = new SettingsPage(this);
    m_contentStack->addWidget(m_settingsPage);

    // 连接设置页信号
    connect(m_settingsPage, &SettingsPage::connectRequested,
            this, [this](const QString &ip, int port) {
                if (m_deviceService) {
                    m_deviceService->connectDevice(ip, port);
                }
            });

    connect(m_settingsPage, &SettingsPage::disconnectRequested,
            this, [this]() {
                if (m_deviceService) {
                    m_deviceService->disconnectDevice();
                }
            });

    // 默认显示总览页
    m_contentStack->setCurrentIndex(0);
}

void MainWindow::onNavigationClicked(int pageIndex)
{
    navigateTo(pageIndex);
}

void MainWindow::onConnectionStateChanged(ConnectionState state)
{
    QString statusText;
    QString statusColor;

    switch (state) {
    case ConnectionState::Offline:
        statusText = "离线";
        statusColor = "gray";
        break;
    case ConnectionState::Connecting:
        statusText = "连接中...";
        statusColor = "orange";
        break;
    case ConnectionState::Online:
        statusText = "在线";
        statusColor = "green";
        break;
    case ConnectionState::DataTimeout:
        statusText = "数据超时";
        statusColor = "red";
        break;
    case ConnectionState::ProtocolError:
        statusText = "协议错误";
        statusColor = "red";
        break;
    default:
        statusText = "未知";
        statusColor = "gray";
    }

    m_connectionStatusLabel->setText(statusText);
    m_connectionStatusLabel->setStyleSheet(QString("color: %1; padding: 5px;").arg(statusColor));
}

void MainWindow::onAlarmCountChanged(int count)
{
    m_alarmCountLabel->setText(QString("告警: %1").arg(count));

    if (count > 0) {
        m_alarmCountLabel->setStyleSheet("color: red; padding: 5px;");
    } else {
        m_alarmCountLabel->setStyleSheet("color: green; padding: 5px;");
    }
}
