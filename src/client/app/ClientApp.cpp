#include "ClientApp.h"
#include "ui/MainWindow.h"
#include "services/DeviceService.h"
#include "services/AlarmService.h"

ClientApp::ClientApp(QObject *parent)
    : QObject(parent)
    , m_deviceService(nullptr)
    , m_alarmService(nullptr)
{
}

ClientApp::~ClientApp()
{
    shutdown();
}

bool ClientApp::initialize()
{
    // 设置服务
    setupServices();

    // 设置 UI
    setupUI();

    // 加载设置
    loadSettings();

    emit initialized();
    return true;
}

void ClientApp::showMainWindow()
{
    if (m_mainWindow) {
        m_mainWindow->show();
    }
}

void ClientApp::shutdown()
{
    emit aboutToQuit();

    if (m_mainWindow) {
        m_mainWindow->close();
    }
}

void ClientApp::setupServices()
{
    // 创建服务
    m_deviceService = new DeviceService(this);
    m_alarmService = new AlarmService(this);

    // 连接信号
    connect(m_deviceService, &DeviceService::connectionStateChanged,
            this, [this](ConnectionState state) {
                // 处理连接状态变化
            });
}

void ClientApp::setupUI()
{
    m_mainWindow = QScopedPointer<MainWindow>(new MainWindow());
    m_mainWindow->setDeviceService(m_deviceService);
    m_mainWindow->setAlarmService(m_alarmService);
}

void ClientApp::loadSettings()
{
    // 加载应用程序设置
    // TODO: 从配置文件加载设置
}
