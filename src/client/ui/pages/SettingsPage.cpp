#include "SettingsPage.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>

namespace {
QFrame *createSection(const QString &title, QWidget *parent)
{
    auto *section = new QFrame(parent);
    section->setObjectName("settingsSection");
    section->setStyleSheet("QFrame#settingsSection { background:#ffffff; border:1px solid #d9dee5; border-radius:4px; }");
    auto *layout = new QVBoxLayout(section);
    layout->setContentsMargins(18, 14, 18, 16);
    layout->setSpacing(12);
    auto *heading = new QLabel(title, section);
    heading->setStyleSheet("color:#263442; font-size:14px; font-weight:600;");
    layout->addWidget(heading);
    return section;
}

const char *kInputStyle =
    "QLineEdit, QSpinBox, QComboBox { min-height:30px; padding:0 8px; background:#ffffff; border:1px solid #bfc9d4; border-radius:3px; font-size:13px; }"
    "QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border-color:#2f6f9f; }";

const char *kPrimaryButtonStyle =
    "QPushButton { background:#2f6f9f; color:#ffffff; border:0; border-radius:3px; padding:7px 18px; font-weight:600; }"
    "QPushButton:hover { background:#255c85; }"
    "QPushButton:disabled { background:#a9bac8; }";

const char *kSecondaryButtonStyle =
    "QPushButton { background:#ffffff; color:#344054; border:1px solid #bfc9d4; border-radius:3px; padding:7px 18px; }"
    "QPushButton:hover { background:#f2f5f7; }";
}

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
    , m_browseExportPathBtn(nullptr)
    , m_exportFormatCombo(nullptr)
    , m_autoExportCheckBox(nullptr)
    , m_saveBtn(nullptr)
    , m_resetBtn(nullptr)
{
    setupUI();
    loadSettings();
}

SettingsPage::~SettingsPage() = default;

void SettingsPage::loadSettings()
{
    QSettings settings;
    m_ipEdit->setText(settings.value("connection/ip", "192.168.100.2").toString());
    m_portSpinBox->setValue(settings.value("connection/port", 1000).toInt());
    m_themeCombo->setCurrentText(settings.value("display/theme", QStringLiteral("标准浅色")).toString());
    m_languageCombo->setCurrentText(settings.value("display/language", QStringLiteral("中文")).toString());
    m_refreshRateSpinBox->setValue(settings.value("display/refreshRate", 1).toInt());
    m_exportPathEdit->setText(settings.value("export/path", "").toString());
    m_exportFormatCombo->setCurrentText(settings.value("export/format", "CSV").toString());
    m_autoExportCheckBox->setChecked(settings.value("export/autoExport", false).toBool());
}

void SettingsPage::saveSettings()
{
    QSettings settings;
    settings.setValue("connection/ip", m_ipEdit->text().trimmed());
    settings.setValue("connection/port", m_portSpinBox->value());
    settings.setValue("display/theme", m_themeCombo->currentText());
    settings.setValue("display/language", m_languageCombo->currentText());
    settings.setValue("display/refreshRate", m_refreshRateSpinBox->value());
    settings.setValue("export/path", m_exportPathEdit->text().trimmed());
    settings.setValue("export/format", m_exportFormatCombo->currentText());
    settings.setValue("export/autoExport", m_autoExportCheckBox->isChecked());
    settings.sync();
    emit settingsChanged();
}

void SettingsPage::onConnectClicked()
{
    const QString ip = m_ipEdit->text().trimmed();
    if (ip.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("连接设备"), QStringLiteral("请输入雷达设备 IP 地址。"));
        m_ipEdit->setFocus();
        return;
    }
    saveSettings();
    emit connectRequested(ip, m_portSpinBox->value());
}

void SettingsPage::onDisconnectClicked() { emit disconnectRequested(); }

void SettingsPage::onSaveClicked()
{
    saveSettings();
    QMessageBox::information(this, QStringLiteral("系统配置"), QStringLiteral("配置已保存。"));
}

void SettingsPage::onResetClicked()
{
    m_ipEdit->setText("192.168.100.2");
    m_portSpinBox->setValue(1000);
    m_themeCombo->setCurrentText(QStringLiteral("标准浅色"));
    m_languageCombo->setCurrentText(QStringLiteral("中文"));
    m_refreshRateSpinBox->setValue(1);
    m_exportPathEdit->clear();
    m_exportFormatCombo->setCurrentText("CSV");
    m_autoExportCheckBox->setChecked(false);
}

void SettingsPage::onBrowseExportPathClicked()
{
    const QString selectedPath = QFileDialog::getExistingDirectory(
        this, QStringLiteral("选择数据导出目录"), m_exportPathEdit->text());
    if (!selectedPath.isEmpty()) m_exportPathEdit->setText(selectedPath);
}

void SettingsPage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 18);
    mainLayout->setSpacing(14);
    auto *title = new QLabel(QStringLiteral("系统配置"), this);
    title->setStyleSheet("color:#182230; font-size:22px; font-weight:600;");
    auto *subtitle = new QLabel(QStringLiteral("配置雷达连接、数据刷新与导出规则。连接参数在保存后立即用于下一次连接。"), this);
    subtitle->setStyleSheet("color:#667085; font-size:12px;");
    subtitle->setWordWrap(true);
    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);
    createConnectionSection();
    createDisplaySection();
    createExportSection();
    createButtons();
    mainLayout->addStretch();
}

void SettingsPage::createConnectionSection()
{
    auto *section = createSection(QStringLiteral("雷达连接"), this);
    auto *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(10);
    m_ipEdit = new QLineEdit(section);
    m_ipEdit->setPlaceholderText("192.168.100.2");
    m_ipEdit->setStyleSheet(kInputStyle);
    m_portSpinBox = new QSpinBox(section);
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setStyleSheet(kInputStyle);
    form->addRow(QStringLiteral("设备 IP 地址："), m_ipEdit);
    form->addRow(QStringLiteral("TCP 端口："), m_portSpinBox);
    static_cast<QVBoxLayout *>(section->layout())->addLayout(form);
    auto *buttons = new QHBoxLayout();
    m_connectBtn = new QPushButton(QStringLiteral("连接设备"), section);
    m_connectBtn->setStyleSheet(kPrimaryButtonStyle);
    m_disconnectBtn = new QPushButton(QStringLiteral("断开连接"), section);
    m_disconnectBtn->setStyleSheet(kSecondaryButtonStyle);
    connect(m_connectBtn, &QPushButton::clicked, this, &SettingsPage::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &SettingsPage::onDisconnectClicked);
    buttons->addWidget(m_connectBtn); buttons->addWidget(m_disconnectBtn); buttons->addStretch();
    static_cast<QVBoxLayout *>(section->layout())->addLayout(buttons);
    static_cast<QVBoxLayout *>(layout())->addWidget(section);
}

void SettingsPage::createDisplaySection()
{
    auto *section = createSection(QStringLiteral("显示与刷新"), this);
    auto *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(18); form->setVerticalSpacing(10);
    m_themeCombo = new QComboBox(section); m_themeCombo->addItems({QStringLiteral("标准浅色"), QStringLiteral("高对比度")}); m_themeCombo->setStyleSheet(kInputStyle);
    m_languageCombo = new QComboBox(section); m_languageCombo->addItems({QStringLiteral("中文"), "English"}); m_languageCombo->setStyleSheet(kInputStyle);
    m_refreshRateSpinBox = new QSpinBox(section); m_refreshRateSpinBox->setRange(1, 10); m_refreshRateSpinBox->setSuffix(QStringLiteral(" 秒")); m_refreshRateSpinBox->setStyleSheet(kInputStyle);
    form->addRow(QStringLiteral("界面主题："), m_themeCombo);
    form->addRow(QStringLiteral("界面语言："), m_languageCombo);
    form->addRow(QStringLiteral("数据刷新周期："), m_refreshRateSpinBox);
    static_cast<QVBoxLayout *>(section->layout())->addLayout(form);
    static_cast<QVBoxLayout *>(layout())->addWidget(section);
}

void SettingsPage::createExportSection()
{
    auto *section = createSection(QStringLiteral("数据导出"), this);
    auto *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(18); form->setVerticalSpacing(10);
    auto *pathLayout = new QHBoxLayout();
    m_exportPathEdit = new QLineEdit(section); m_exportPathEdit->setStyleSheet(kInputStyle);
    m_browseExportPathBtn = new QPushButton(QStringLiteral("浏览..."), section); m_browseExportPathBtn->setStyleSheet(kSecondaryButtonStyle);
    connect(m_browseExportPathBtn, &QPushButton::clicked, this, &SettingsPage::onBrowseExportPathClicked);
    pathLayout->addWidget(m_exportPathEdit, 1); pathLayout->addWidget(m_browseExportPathBtn);
    m_exportFormatCombo = new QComboBox(section); m_exportFormatCombo->addItems({"CSV", "JSON", "NetCDF"}); m_exportFormatCombo->setStyleSheet(kInputStyle);
    m_autoExportCheckBox = new QCheckBox(QStringLiteral("按设定周期自动导出"), section);
    form->addRow(QStringLiteral("保存路径："), pathLayout);
    form->addRow(QStringLiteral("文件格式："), m_exportFormatCombo);
    form->addRow(QStringLiteral("自动导出："), m_autoExportCheckBox);
    static_cast<QVBoxLayout *>(section->layout())->addLayout(form);
    static_cast<QVBoxLayout *>(layout())->addWidget(section);
}

void SettingsPage::createButtons()
{
    auto *row = new QHBoxLayout();
    m_saveBtn = new QPushButton(QStringLiteral("保存配置"), this); m_saveBtn->setStyleSheet(kPrimaryButtonStyle);
    m_resetBtn = new QPushButton(QStringLiteral("恢复默认"), this); m_resetBtn->setStyleSheet(kSecondaryButtonStyle);
    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsPage::onSaveClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &SettingsPage::onResetClicked);
    row->addWidget(m_saveBtn); row->addWidget(m_resetBtn); row->addStretch();
    static_cast<QVBoxLayout *>(layout())->addLayout(row);
}
