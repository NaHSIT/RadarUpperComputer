#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPlainTextEdit>
#include "domain/RadarTypes.h"

class DeviceService;
class AlarmService;
class NavigationBar;
class DashboardPage;
class WindFieldPage;
class BeamPage;
class SpectrumPage;
class DeviceHealthPage;
class SettingsPage;

/**
 * @brief 主窗口
 *
 * 客户端主窗口，包含导航、状态栏和内容区
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // 服务设置
    void setDeviceService(DeviceService *service);
    void setAlarmService(AlarmService *service);

    // 页面导航
    void navigateTo(int pageIndex);

signals:
    void pageChanged(int pageIndex);

private slots:
    void onNavigationClicked(int pageIndex);
    void onConnectionStateChanged(ConnectionState state);
    void onAlarmCountChanged(int count);

private:
    void setupUI();
    void setupStatusBar();
    void setupEventLog();
    void setupNavigation();
    void setupContentArea();
    void createPages();
    void appendLog(const QString &message);

    // 服务
    DeviceService *m_deviceService;
    AlarmService *m_alarmService;

    // UI 组件
    NavigationBar *m_navigationBar;
    QStackedWidget *m_contentStack;
    QLabel *m_connectionStatusLabel;
    QLabel *m_alarmCountLabel;
    QPlainTextEdit *m_eventLog;

    // 页面
    DashboardPage *m_dashboardPage;
    WindFieldPage *m_windFieldPage;
    BeamPage *m_beamPage;
    SpectrumPage *m_spectrumPage;
    DeviceHealthPage *m_deviceHealthPage;
    SettingsPage *m_settingsPage;

    // 页面索引
    enum PageIndex {
        PAGE_DASHBOARD = 0,
        PAGE_WIND_FIELD,
        PAGE_BEAM,
        PAGE_SPECTRUM,
        PAGE_DEVICE_HEALTH,
        PAGE_SETTINGS
    };
};

#endif // MAINWINDOW_H
