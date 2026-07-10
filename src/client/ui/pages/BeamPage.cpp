#include "BeamPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>

namespace {
QFrame *makePanel(QWidget *parent) {
    auto *frame = new QFrame(parent);
    frame->setObjectName("beamPanel");
    frame->setStyleSheet("QFrame#beamPanel { background:#ffffff; border:1px solid #d9dee5; border-radius:4px; }");
    return frame;
}
}

BeamPage::BeamPage(QWidget *parent) : QWidget(parent) { setupUI(); }
BeamPage::~BeamPage() = default;

void BeamPage::setupUI()
{
    auto *layout = new QVBoxLayout(this); layout->setContentsMargins(24, 20, 24, 18); layout->setSpacing(14);
    auto *title = new QLabel(QStringLiteral("波束监视"), this);
    title->setStyleSheet("color:#182230; font-size:22px; font-weight:600;"); layout->addWidget(title);
    auto *subtitle = new QLabel(QStringLiteral("查看五路 LOS 波束的方位、信噪比、有效层数及数据质量"), this);
    subtitle->setStyleSheet("color:#667085; font-size:12px; padding-top:2px;");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);

    auto *summary = new QHBoxLayout(); summary->setSpacing(10);
    const struct { QString name; QString value; QString hint; QString color; } beams[] = {
        {"LOS 1", "0°", QStringLiteral("运行正常"), "#16713b"}, {"LOS 2", "72°", QStringLiteral("运行正常"), "#16713b"},
        {"LOS 3", "144°", QStringLiteral("待数据"), "#9a6700"}, {"LOS 4", "216°", QStringLiteral("运行正常"), "#16713b"},
        {"LOS 5", "288°", QStringLiteral("运行正常"), "#16713b"}
    };
    for (const auto &beam : beams) {
        auto *card = makePanel(this); auto *cardLayout = new QVBoxLayout(card); cardLayout->setContentsMargins(14, 12, 14, 12); cardLayout->setSpacing(4);
        auto *name = new QLabel(beam.name, card); name->setStyleSheet("color:#52606d; font-size:12px;"); cardLayout->addWidget(name);
        auto *angle = new QLabel(beam.value, card); angle->setStyleSheet("color:#182230; font-size:24px; font-weight:600;"); cardLayout->addWidget(angle);
        auto *state = new QLabel(beam.hint, card); state->setStyleSheet(QStringLiteral("color:%1; font-size:12px; font-weight:600;").arg(beam.color)); cardLayout->addWidget(state);
        summary->addWidget(card);
    }
    layout->addLayout(summary);

    auto *detail = makePanel(this); auto *detailLayout = new QVBoxLayout(detail); detailLayout->setContentsMargins(14, 12, 14, 14); detailLayout->setSpacing(8);
    auto *detailTitle = new QLabel(QStringLiteral("波束质量明细"), detail); detailTitle->setStyleSheet("color:#263442; font-size:14px; font-weight:600;"); detailLayout->addWidget(detailTitle);
    auto *table = new QTableWidget(5, 7, detail);
    table->setHorizontalHeaderLabels({QStringLiteral("波束"), QStringLiteral("方位角"), QStringLiteral("平均 CNR (dB)"), QStringLiteral("有效层数"), QStringLiteral("数据完整率"), QStringLiteral("最近更新时间"), QStringLiteral("状态")});
    table->verticalHeader()->hide(); table->setEditTriggers(QAbstractItemView::NoEditTriggers); table->setSelectionBehavior(QAbstractItemView::SelectRows); table->setShowGrid(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setStyleSheet("QTableWidget { border:1px solid #d9dee5; font-size:12px; } QTableWidget::item { padding:7px; border-bottom:1px solid #edf0f2; } QHeaderView::section { background:#f2f5f7; color:#52606d; padding:8px; border:0; border-bottom:1px solid #d9dee5; font-weight:600; }");
    for (int row = 0; row < 5; ++row) {
        const QStringList values = {QStringLiteral("LOS %1").arg(row + 1), QStringLiteral("%1°").arg(row * 72), "--", "--", "--", "--", row == 2 ? QStringLiteral("待数据") : QStringLiteral("正常")};
        for (int column = 0; column < values.size(); ++column) table->setItem(row, column, new QTableWidgetItem(values[column]));
        table->item(row, 6)->setForeground(row == 2 ? QColor("#9a6700") : QColor("#16713b"));
    }
    detailLayout->addWidget(table); layout->addWidget(detail, 1);
}
