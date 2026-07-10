#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include <QObject>
#include <QScopedPointer>

class MainWindow;
class DeviceService;
class AlarmService;

class ClientApp : public QObject
{
    Q_OBJECT

public:
    explicit ClientApp(QObject *parent = nullptr);
    ~ClientApp() override;

    bool initialize();
    void showMainWindow();
    void shutdown();

signals:
    void initialized();
    void aboutToQuit();

private:
    void setupServices();
    void setupUI();
    void loadSettings();

    QScopedPointer<MainWindow> m_mainWindow;
    DeviceService *m_deviceService;
    AlarmService *m_alarmService;
};

#endif // CLIENTAPP_H
