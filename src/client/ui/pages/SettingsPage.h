#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>

/**
 * @brief 设置页面
 *
 * 管理连接配置、显示设置、导出设置等
 */
class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage() override;

    // 配置读写
    void loadSettings();
    void saveSettings();

signals:
    void settingsChanged();
    void connectRequested(const QString &ip, int port);
    void disconnectRequested();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onSaveClicked();
    void onResetClicked();

private:
    void setupUI();
    void createConnectionSection();
    void createDisplaySection();
    void createExportSection();
    void createButtons();

    // 连接配置
    QLineEdit *m_ipEdit;
    QSpinBox *m_portSpinBox;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;

    // 显示设置
    QComboBox *m_themeCombo;
    QComboBox *m_languageCombo;
    QSpinBox *m_refreshRateSpinBox;

    // 导出设置
    QLineEdit *m_exportPathEdit;
    QComboBox *m_exportFormatCombo;
    QCheckBox *m_autoExportCheckBox;

    // 按钮
    QPushButton *m_saveBtn;
    QPushButton *m_resetBtn;
};

#endif // SETTINGSPAGE_H
