#include "SettingsPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QSettings>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
    , m_ipEdit(nullptr)
    , m_portSpinBox(nullptr)
    , m_connectBtn(nullptr)
    , m_disconnectBtn(nullptr)
    , m_themeCombo(nullptr)
    , m_languageCombo(nullptr)
    , m_refreshRateSpinBox(nullptr)
    , m_exportPathEdit(nullptr)
    , m_exportFormatCombo(nullptr)
    , m_autoExportCheckBox(nullptr)
    , m_saveBtn(nullptr)
    , m_resetBtn(nullptr)
{
    setupUI();
    loadSettings();
}

SettingsPage::~SettingsPage()
{
}

void SettingsPage::loadSettings()
{
    QSettings settings;

    // 连接配置
    m_ipEdit->setText(settings.value("connection/ip", "192.168.100.2").toString());
    m_portSpinBox->setValue(settings.value("connection/port", 1000).toInt());

    // 显示设置
    m_themeCombo->setCurrentText(settings.value("display/theme", "浅色").toString());
    m_languageCombo->setCurrentText(settings.value("display/language", "中文").toString());
    m_refreshRateSpinBox->setValue(settings.value("display/refreshRate", 1).toInt());

    // 导出设置
    m_exportPathEdit->setText(settings.value("export/path", "").toString());
    m_exportFormatCombo->setCurrentText(settings.value("export/format", "CSV").toString());
    m_autoExportCheckBox->setChecked(settings.value("export/autoExport", false).toBool());
}

void SettingsPage::saveSettings()
{
    QSettings settings;

    // 连接配置
    settings.setValue("connection/ip", m_ipEdit->text());
    settings.setValue("connection/port", m_portSpinBox->value());

    // 显示设置
    settings.setValue("display/theme", m_themeCombo->currentText());
    settings.setValue("display/language", m_languageCombo->currentText());
    settings.setValue("display/refreshRate", m_refreshRateSpinBox->value());

    // 导出设置
    settings.setValue("export/path", m_exportPathEdit->text());
    settings.setValue("export/format", m_exportFormatCombo->currentText());
    settings.setValue("export/autoExport", m_autoExportCheckBox->isChecked());

    settings.sync();
    emit settingsChanged();
}

void SettingsPage::onConnectClicked()
{
    QString ip = m_ipEdit->text();
    int port = m_portSpinBox->value();
    emit connectRequested(ip, port);
}

void SettingsPage::onDisconnectClicked()
{
    emit disconnectRequested();
}

void SettingsPage::onSaveClicked()
{
    saveSettings();
}

void SettingsPage::onResetClicked()
{
    // 重置为默认值
    m_ipEdit->setText("192.168.100.2");
    m_portSpinBox->setValue(1000);
    m_themeCombo->setCurrentText("浅色");
    m_languageCombo->setCurrentText("中文");
    m_refreshRateSpinBox->setValue(1);
    m_exportPathEdit->setText("");
    m_exportFormatCombo->setCurrentText("CSV");
    m_autoExportCheckBox->setChecked(false);
}

void SettingsPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // 页面标题
    QLabel *titleLabel = new QLabel("设置", this);
    titleLabel->setStyleSheet("color: #333; font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // 连接配置
    createConnectionSection();

    // 显示设置
    createDisplaySection();

    // 导出设置
    createExportSection();

    // 按钮
    createButtons();

    mainLayout->addStretch();
}

void SettingsPage::createConnectionSection()
{
    QGroupBox *group = new QGroupBox("连接配置", this);
    group->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #ddd; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }");

    QGridLayout *layout = new QGridLayout(group);

    // IP 地址
    layout->addWidget(new QLabel("IP 地址:", group), 0, 0);
    m_ipEdit = new QLineEdit(group);
    m_ipEdit->setPlaceholderText("192.168.100.2");
    layout->addWidget(m_ipEdit, 0, 1);

    // 端口
    layout->addWidget(new QLabel("端口:", group), 0, 2);
    m_portSpinBox = new QSpinBox(group);
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(1000);
    layout->addWidget(m_portSpinBox, 0, 3);

    // 连接按钮
    m_connectBtn = new QPushButton("连接", group);
    m_connectBtn->setStyleSheet("QPushButton { background: #4CAF50; color: white; border: none; padding: 8px 16px; border-radius: 4px; } QPushButton:hover { background: #45a049; }");
    connect(m_connectBtn, &QPushButton::clicked, this, &SettingsPage::onConnectClicked);
    layout->addWidget(m_connectBtn, 1, 0, 1, 2);

    // 断开按钮
    m_disconnectBtn = new QPushButton("断开", group);
    m_disconnectBtn->setStyleSheet("QPushButton { background: #f44336; color: white; border: none; padding: 8px 16px; border-radius: 4px; } QPushButton:hover { background: #da190b; }");
    connect(m_disconnectBtn, &QPushButton::clicked, this, &SettingsPage::onDisconnectClicked);
    layout->addWidget(m_disconnectBtn, 1, 2, 1, 2);

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(group);
    }
}

void SettingsPage::createDisplaySection()
{
    QGroupBox *group = new QGroupBox("显示设置", this);
    group->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #ddd; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }");

    QGridLayout *layout = new QGridLayout(group);

    // 主题
    layout->addWidget(new QLabel("主题:", group), 0, 0);
    m_themeCombo = new QComboBox(group);
    m_themeCombo->addItems({"浅色", "深色", "高对比"});
    layout->addWidget(m_themeCombo, 0, 1);

    // 语言
    layout->addWidget(new QLabel("语言:", group), 0, 2);
    m_languageCombo = new QComboBox(group);
    m_languageCombo->addItems({"中文", "English"});
    layout->addWidget(m_languageCombo, 0, 3);

    // 刷新率
    layout->addWidget(new QLabel("刷新率:", group), 1, 0);
    m_refreshRateSpinBox = new QSpinBox(group);
    m_refreshRateSpinBox->setRange(1, 10);
    m_refreshRateSpinBox->setValue(1);
    m_refreshRateSpinBox->setSuffix(" 秒");
    layout->addWidget(m_refreshRateSpinBox, 1, 1);

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(group);
    }
}

void SettingsPage::createExportSection()
{
    QGroupBox *group = new QGroupBox("导出设置", this);
    group->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #ddd; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }");

    QGridLayout *layout = new QGridLayout(group);

    // 导出路径
    layout->addWidget(new QLabel("导出路径:", group), 0, 0);
    m_exportPathEdit = new QLineEdit(group);
    m_exportPathEdit->setPlaceholderText("选择导出路径...");
    layout->addWidget(m_exportPathEdit, 0, 1, 1, 3);

    // 导出格式
    layout->addWidget(new QLabel("导出格式:", group), 1, 0);
    m_exportFormatCombo = new QComboBox(group);
    m_exportFormatCombo->addItems({"CSV", "JSON", "NetCDF"});
    layout->addWidget(m_exportFormatCombo, 1, 1);

    // 自动导出
    m_autoExportCheckBox = new QCheckBox("自动导出", group);
    layout->addWidget(m_autoExportCheckBox, 1, 2);

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(group);
    }
}

void SettingsPage::createButtons()
{
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(16);

    m_saveBtn = new QPushButton("保存设置", this);
    m_saveBtn->setStyleSheet("QPushButton { background: #2196F3; color: white; border: none; padding: 10px 20px; border-radius: 4px; } QPushButton:hover { background: #0b7dda; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsPage::onSaveClicked);
    buttonLayout->addWidget(m_saveBtn);

    m_resetBtn = new QPushButton("恢复默认", this);
    m_resetBtn->setStyleSheet("QPushButton { background: #9e9e9e; color: white; border: none; padding: 10px 20px; border-radius: 4px; } QPushButton:hover { background: #757575; }");
    connect(m_resetBtn, &QPushButton::clicked, this, &SettingsPage::onResetClicked);
    buttonLayout->addWidget(m_resetBtn);

    buttonLayout->addStretch();

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addLayout(buttonLayout);
    }
}
