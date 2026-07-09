#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include <QObject>
#include <QScopedPointer>

class MainWindow;
class DeviceService;
class AlarmService;

/**
 * @brief 客户端应用程序类
 *
 * 负责初始化和管理应用程序的生命周期
 */
class ClientApp : public QObject
{
    Q_OBJECT

public:
    explicit ClientApp(QObject *parent = nullptr);
    ~ClientApp() override;

    // 初始化
    bool initialize();

    // 显示主窗口
    void showMainWindow();

    // 关闭应用程序
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
