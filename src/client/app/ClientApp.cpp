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
    setupServices();
    setupUI();
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
    m_deviceService = new DeviceService(this);
    m_alarmService = new AlarmService(this);

    connect(m_deviceService, &DeviceService::connectionStateChanged,
            this, [this](ConnectionState state) {
                Q_UNUSED(state);
            });
}

void ClientApp::setupUI()
{
    m_mainWindow.reset(new MainWindow());
    m_mainWindow->setDeviceService(m_deviceService);
    m_mainWindow->setAlarmService(m_alarmService);
}

void ClientApp::loadSettings()
{
    // TODO: Load application settings.
}
