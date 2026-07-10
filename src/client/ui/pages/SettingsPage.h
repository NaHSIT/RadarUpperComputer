#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QString>

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage() override;

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
    void onBrowseExportPathClicked();

private:
    void setupUI();
    void createConnectionSection();
    void createDisplaySection();
    void createExportSection();
    void createButtons();

    QLineEdit *m_ipEdit;
    QSpinBox *m_portSpinBox;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;

    QComboBox *m_themeCombo;
    QComboBox *m_languageCombo;
    QSpinBox *m_refreshRateSpinBox;

    QLineEdit *m_exportPathEdit;
    QPushButton *m_browseExportPathBtn;
    QComboBox *m_exportFormatCombo;
    QCheckBox *m_autoExportCheckBox;

    QPushButton *m_saveBtn;
    QPushButton *m_resetBtn;
};

#endif // SETTINGSPAGE_H
