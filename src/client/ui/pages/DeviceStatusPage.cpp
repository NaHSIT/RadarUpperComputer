#include "DeviceStatusPage.h"

#include "BeamPage.h"
#include "DataQualityPage.h"
#include "DeviceHealthPage.h"
#include "SpectrumPage.h"

#include <QTabWidget>
#include <QVBoxLayout>

DeviceStatusPage::DeviceStatusPage(QWidget *parent)
    : QWidget(parent)
    , m_healthPage(new DeviceHealthPage())
    , m_qualityPage(new DataQualityPage())
    , m_beamPage(new BeamPage())
    , m_spectrumPage(new SpectrumPage())
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto *tabs = new QTabWidget(this);
    tabs->setDocumentMode(true);
    tabs->setStyleSheet(
        "QTabWidget::pane { border:0; background:#f4f7fb; }"
        "QTabBar { background:#ffffff; }"
        "QTabBar::tab { min-width:112px; padding:12px 18px; color:#52606d; background:#ffffff; border:0; border-bottom:2px solid transparent; font-size:13px; }"
        "QTabBar::tab:selected { color:#1f6689; border-bottom-color:#2f8cad; font-weight:600; }"
        "QTabBar::tab:hover:!selected { color:#263442; background:#f8fafc; }");
    tabs->addTab(m_healthPage, QStringLiteral("运行状态"));
    tabs->addTab(m_qualityPage, QStringLiteral("数据质量"));
    tabs->addTab(m_beamPage, QStringLiteral("波束监视"));
    tabs->addTab(m_spectrumPage, QStringLiteral("频谱诊断"));
    layout->addWidget(tabs);
}
