#include "ClientApp.h"
#include "ui/MainWindow.h"
#include "services/DeviceService.h"
#include "services/AlarmService.h"
#include "services/DataService.h"
#include "algorithms/PyArtWindService.h"

ClientApp::ClientApp(QObject *parent)
    : QObject(parent)
    , m_deviceService(nullptr)
    , m_alarmService(nullptr)
    , m_pyArtWindService(nullptr)
    , m_dataService(nullptr)
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
    m_pyArtWindService = new PyArtWindService(this);
    m_dataService = new DataService(this);
    m_dataService->initialize();

    connect(m_deviceService, &DeviceService::windProfileUpdated,
            m_dataService, &DataService::updateWindProfile);
    connect(m_deviceService, &DeviceService::deviceHealthUpdated,
            m_dataService, &DataService::updateDeviceHealth);
    connect(m_alarmService, &AlarmService::alarmRaised,
            m_dataService, &DataService::addAlarm);

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
    m_mainWindow->setPyArtWindService(m_pyArtWindService);
    m_mainWindow->setDataService(m_dataService);
    m_pyArtWindService->checkEnvironment();
}

void ClientApp::loadSettings()
{
    // TODO: Load application settings.
}
