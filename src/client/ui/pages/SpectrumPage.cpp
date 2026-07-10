#include "SpectrumPage.h"

#include <QFrame>
#include <QFormLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>

namespace { QFrame *panel(QWidget *p) { auto *f = new QFrame(p); f->setObjectName("spectrumPanel"); f->setStyleSheet("QFrame#spectrumPanel { background:#fff; border:1px solid #d9dee5; border-radius:4px; }"); return f; } }
SpectrumPage::SpectrumPage(QWidget *parent) : QWidget(parent) { setupUI(); }
SpectrumPage::~SpectrumPage() = default;
void SpectrumPage::setupUI()
{
    auto *layout = new QVBoxLayout(this); layout->setContentsMargins(24, 20, 24, 18); layout->setSpacing(14);
    auto *title = new QLabel(QStringLiteral("频谱诊断"), this); title->setStyleSheet("color:#182230; font-size:22px; font-weight:600;"); layout->addWidget(title);
    auto *subtitle = new QLabel(QStringLiteral("对当前测量链路的谱峰、信噪比与干扰特征进行诊断"), this);
    subtitle->setStyleSheet("color:#667085; font-size:12px; padding-top:2px;");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);
    auto *upper = new QHBoxLayout(); upper->setSpacing(14);
    auto *spectrum = panel(this); auto *spectrumLayout = new QVBoxLayout(spectrum); spectrumLayout->setContentsMargins(14, 12, 14, 14);
    auto *spectrumTitle = new QLabel(QStringLiteral("功率谱"), spectrum); spectrumTitle->setStyleSheet("color:#263442; font-size:14px; font-weight:600;"); spectrumLayout->addWidget(spectrumTitle);
    auto *canvas = new QLabel(QStringLiteral("等待频谱数据"), spectrum); canvas->setAlignment(Qt::AlignCenter); canvas->setMinimumHeight(290); canvas->setStyleSheet("color:#7b8794; background:#f8fafb; border:1px dashed #cbd5df; font-size:13px;"); spectrumLayout->addWidget(canvas);
    upper->addWidget(spectrum, 3);
    auto *diagnosis = panel(this); auto *form = new QFormLayout(diagnosis); form->setContentsMargins(14, 12, 14, 14); form->setSpacing(12);
    auto *diagTitle = new QLabel(QStringLiteral("诊断结论"), diagnosis); diagTitle->setStyleSheet("color:#263442; font-size:14px; font-weight:600;"); form->addRow(diagTitle);
    const struct { QString label; QString value; QString color; } rows[] = {{QStringLiteral("谱峰检测"), QStringLiteral("等待数据"), "#667085"}, {QStringLiteral("杂波抑制"), QStringLiteral("等待数据"), "#667085"}, {QStringLiteral("信噪比"), QStringLiteral("-- dB"), "#182230"}, {QStringLiteral("频谱泄漏"), QStringLiteral("--"), "#667085"}};
    for (const auto &row : rows) { auto *value = new QLabel(row.value, diagnosis); value->setStyleSheet(QStringLiteral("color:%1; font-size:13px; font-weight:600;").arg(row.color)); form->addRow(row.label + QStringLiteral("："), value); }
    upper->addWidget(diagnosis, 1); layout->addLayout(upper);
    auto *history = panel(this); auto *historyLayout = new QVBoxLayout(history); historyLayout->setContentsMargins(14, 12, 14, 14);
    auto *historyTitle = new QLabel(QStringLiteral("诊断记录"), history); historyTitle->setStyleSheet("color:#263442; font-size:14px; font-weight:600;"); historyLayout->addWidget(historyTitle);
    auto *table = new QTableWidget(0, 5, history); table->setHorizontalHeaderLabels({QStringLiteral("时间"), QStringLiteral("波束"), QStringLiteral("诊断项"), QStringLiteral("结果"), QStringLiteral("备注")}); table->verticalHeader()->hide(); table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); table->setMinimumHeight(170); table->setStyleSheet("QTableWidget { border:1px solid #d9dee5; font-size:12px; } QHeaderView::section { background:#f2f5f7; color:#52606d; padding:8px; border:0; border-bottom:1px solid #d9dee5; font-weight:600; }"); historyLayout->addWidget(table); layout->addWidget(history);
}
