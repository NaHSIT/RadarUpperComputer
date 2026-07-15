#include "SpectrumPage.h"

#include <QComboBox>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
QFrame *createSection(QWidget *parent)
{
    auto *section = new QFrame(parent);
    section->setObjectName("spectrumWorkspaceSection");
    section->setStyleSheet("QFrame#spectrumWorkspaceSection { background:#ffffff; border:1px solid #d9dee5; border-radius:3px; }");
    return section;
}

QLabel *createTitle(const QString &text, QWidget *parent)
{
    auto *title = new QLabel(text, parent);
    title->setStyleSheet("color:#243447; font-size:14px; font-weight:600;");
    return title;
}

void applyTableStyle(QTableWidget *table)
{
    table->verticalHeader()->hide();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setShowGrid(false);
    table->setStyleSheet(
        "QTableWidget { border:1px solid #d9dee5; color:#344054; font-size:12px; }"
        "QTableWidget::item { padding:8px 10px; border-bottom:1px solid #edf0f2; }"
        "QHeaderView::section { background:#f2f5f7; color:#52606d; border:0; border-bottom:1px solid #d9dee5; padding:8px 10px; font-size:12px; font-weight:600; }"
    );
}
}

SpectrumPage::SpectrumPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

SpectrumPage::~SpectrumPage() = default;

void SpectrumPage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 18);
    mainLayout->setSpacing(12);

    auto *title = new QLabel(QStringLiteral("频谱诊断"), this);
    title->setStyleSheet("color:#182230; font-size:22px; font-weight:600;");
    auto *subtitle = new QLabel(QStringLiteral("查看原始频谱和诊断结果，用于定位低信噪比、杂波和谱峰异常。"), this);
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("color:#667085; font-size:13px;");
    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    auto *toolbar = new QHBoxLayout();
    auto *beamLabel = new QLabel(QStringLiteral("波束："), this);
    beamLabel->setStyleSheet("color:#475467; font-size:12px;");
    auto *beamCombo = new QComboBox(this);
    beamCombo->addItems({QStringLiteral("LOS 1"), QStringLiteral("LOS 2"), QStringLiteral("LOS 3"), QStringLiteral("LOS 4"), QStringLiteral("LOS 5")});
    beamCombo->setToolTip(QStringLiteral("选择需要查看的波束。未连接时显示该波束的空状态。"));
    beamCombo->setMinimumWidth(100);
    beamCombo->setStyleSheet("QComboBox { min-height:28px; padding:0 8px; border:1px solid #bfc9d4; border-radius:3px; } QComboBox:disabled { color:#667085; background:#f2f5f7; }");
    toolbar->addWidget(beamLabel);
    toolbar->addWidget(beamCombo);
    toolbar->addStretch();
    auto *source = new QLabel(QStringLiteral("数据源：未连接 / LOS 1"), this);
    source->setStyleSheet("color:#667085; background:#f2f5f7; border:1px solid #d9dee5; padding:6px 10px; font-size:12px;");
    toolbar->addWidget(source);
    mainLayout->addLayout(toolbar);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);
    auto *plotSection = createSection(splitter);
    auto *plotLayout = new QVBoxLayout(plotSection);
    plotLayout->setContentsMargins(14, 12, 14, 14);
    plotLayout->setSpacing(10);
    plotLayout->addWidget(createTitle(QStringLiteral("功率谱"), plotSection));
    auto *emptyPlot = new QLabel(QStringLiteral("LOS 1 暂无频谱数据"), plotSection);
    emptyPlot->setAlignment(Qt::AlignCenter);
    emptyPlot->setMinimumHeight(300);
    emptyPlot->setStyleSheet("color:#667085; background:#f8fafb; border:1px solid #d9dee5; font-size:13px;");
    plotLayout->addWidget(emptyPlot, 1);
    splitter->addWidget(plotSection);

    auto *resultSection = createSection(splitter);
    auto *resultLayout = new QVBoxLayout(resultSection);
    resultLayout->setContentsMargins(14, 12, 14, 14);
    resultLayout->setSpacing(10);
    resultLayout->addWidget(createTitle(QStringLiteral("诊断结果"), resultSection));
    auto *resultTable = new QTableWidget(4, 2, resultSection);
    resultTable->setHorizontalHeaderLabels({QStringLiteral("诊断项"), QStringLiteral("当前结果")});
    applyTableStyle(resultTable);
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    const QStringList labels = {QStringLiteral("谱峰检测"), QStringLiteral("杂波抑制"), QStringLiteral("信噪比"), QStringLiteral("频谱泄漏")};
    const QStringList help = {
        QStringLiteral("是否从多普勒功率谱中找到可靠回波峰。"),
        QStringLiteral("直流、地物和固定目标杂波是否已经被识别和抑制。"),
        QStringLiteral("有效谱峰功率与噪声底的比值，用于判断径向速度可信性。"),
        QStringLiteral("FFT 能量向相邻频点扩散的程度，可由截断和窗函数引起。")
    };
    for (int row = 0; row < labels.size(); ++row) {
        resultTable->setItem(row, 0, new QTableWidgetItem(labels[row]));
        resultTable->item(row, 0)->setToolTip(help.at(row));
        resultTable->setItem(row, 1, new QTableWidgetItem(QStringLiteral("未连接")));
        resultTable->item(row, 1)->setForeground(QColor("#667085"));
    }
    resultLayout->addWidget(resultTable);
    resultLayout->addStretch();
    splitter->addWidget(resultSection);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter, 1);

    auto *historySection = createSection(this);
    auto *historyLayout = new QVBoxLayout(historySection);
    historyLayout->setContentsMargins(14, 12, 14, 14);
    historyLayout->setSpacing(10);
    historyLayout->addWidget(createTitle(QStringLiteral("诊断记录"), historySection));
    auto *historyTable = new QTableWidget(0, 5, historySection);
    historyTable->setHorizontalHeaderLabels({QStringLiteral("时间"), QStringLiteral("波束"), QStringLiteral("诊断项"), QStringLiteral("结果"), QStringLiteral("备注")});
    applyTableStyle(historyTable);
    historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyTable->setMinimumHeight(150);
    historyLayout->addWidget(historyTable);
    mainLayout->addWidget(historySection);

    connect(beamCombo, &QComboBox::currentTextChanged, this,
            [source, emptyPlot](const QString &beam) {
                source->setText(QStringLiteral("数据源：未连接 / %1").arg(beam));
                emptyPlot->setText(QStringLiteral("%1 暂无频谱数据").arg(beam));
            });
}
