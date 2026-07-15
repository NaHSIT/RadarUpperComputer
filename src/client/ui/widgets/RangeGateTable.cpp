#include "RangeGateTable.h"

#include <QHeaderView>
#include <QLabel>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <QShowEvent>
#include <QCursor>
#include <QToolTip>
#include <QtMath>

namespace {
QString numberOrDash(double value, char format = 'f', int precision = 1)
{
    return qIsFinite(value) ? QString::number(value, format, precision) : QStringLiteral("—");
}
}

RangeGateTable::RangeGateTable(QWidget *parent) : QWidget(parent), m_table(nullptr) { setupUI(); }
RangeGateTable::~RangeGateTable() = default;
void RangeGateTable::setGateData(const QVector<GateData> &data)
{
    m_gateData = data;
    if (isVisible()) updateTable();
}
void RangeGateTable::clear() { m_gateData.clear(); updateTable(); }

void RangeGateTable::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateTable();
}

void RangeGateTable::setupUI()
{
    auto *layout = new QVBoxLayout(this); layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(0);
    auto *title = new QLabel(QStringLiteral("分层风场数据"), this);
    title->setStyleSheet("color:#263442; font-size:14px; font-weight:600; padding:0 0 8px;");
    title->setToolTip(QStringLiteral("<b>分层风场数据</b><br>每一行对应一个距离门或高度层，展示该层的风场测量与反演结果。硬件运行状态及数据质量诊断统一在“设备状态”中查看。点击任意单元格可查看字段定义。"));
    layout->addWidget(title);
    m_table = new QTableWidget(this); m_table->setColumnCount(11);
    m_table->setHorizontalHeaderLabels({
        QStringLiteral("层号"), QStringLiteral("斜距 (m)"), QStringLiteral("高度 (m)"),
        QStringLiteral("水平风速"), QStringLiteral("风向 (°)"), QStringLiteral("U 东向"),
        QStringLiteral("V 北向"), QStringLiteral("W 垂直"), QStringLiteral("平均 CNR"),
        QStringLiteral("有效波束"), QStringLiteral("拟合残差")
    });
    m_headerHelp = {
        QStringLiteral("距离门编号，从 0 开始。"),
        QStringLiteral("沿当前波束方向的测量距离。"),
        QStringLiteral("相对雷达安装点的离地高度 AGL。"),
        QStringLiteral("由 U 东向和 V 北向分量合成的水平风速，单位 m/s。"),
        QStringLiteral("气象学来向：0°=北，90°=东。"),
        QStringLiteral("东西向风分量，向东为正，单位 m/s。"),
        QStringLiteral("南北向风分量，向北为正，单位 m/s。"),
        QStringLiteral("垂直风分量，向上为正，单位 m/s。"),
        QStringLiteral("各有效波束载噪比的平均值，来源为 0x8105。"),
        QStringLiteral("该高度层参与反演的有效波束数，三分量求解至少需要 3 束。"),
        QStringLiteral("径向速度观测与 WLS 拟合值的均方根误差，单位 m/s，越小越好。")
    };
    for (int column = 0; column < m_headerHelp.size(); ++column) {
        m_table->horizontalHeaderItem(column)->setToolTip(m_headerHelp.at(column));
    }
    for (int column = 0; column < 10; ++column) {
        m_table->horizontalHeader()->setSectionResizeMode(column, QHeaderView::ResizeToContents);
    }
    m_table->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Stretch);
    m_table->verticalHeader()->hide(); m_table->setEditTriggers(QTableWidget::NoEditTriggers); m_table->setSelectionBehavior(QTableWidget::SelectRows);
    m_table->setSortingEnabled(true); m_table->setShowGrid(false); m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet(
        "QTableWidget { background:#fff; border:1px solid #dce4ec; border-radius:4px; font-size:12px; }"
        "QTableWidget::item { padding:8px 10px; border-bottom:1px solid #edf1f5; }"
        "QTableWidget::item:selected { background:#eaf5f7; color:#172b3d; }"
        "QTableWidget::item:alternate { background:#f8fafc; }"
        "QHeaderView::section { background:#f1f5f8; color:#52606d; font-weight:600; font-size:12px; padding:9px 10px; border:0; border-bottom:1px solid #dce4ec; }"
    );
    connect(m_table, &QTableWidget::cellClicked, this, &RangeGateTable::onItemClicked); layout->addWidget(m_table);
}

void RangeGateTable::updateTable()
{
    const QSignalBlocker blocker(m_table);
    const bool sorting = m_table->isSortingEnabled();
    m_table->setUpdatesEnabled(false);
    m_table->setSortingEnabled(false);
    m_table->setRowCount(m_gateData.size());

    const auto setCell = [this](int row, int column, const QString &text) {
        QTableWidgetItem *item = m_table->item(row, column);
        if (!item) {
            item = new QTableWidgetItem();
            m_table->setItem(row, column, item);
        }
        if (item->text() != text) item->setText(text);
        if (column < m_headerHelp.size()) item->setToolTip(m_headerHelp.at(column));
    };

    for (int row = 0; row < m_gateData.size(); ++row) {
        const auto &gate = m_gateData.at(row);
        setCell(row, 0, QString::number(gate.gateIndex));
        setCell(row, 1, numberOrDash(gate.distanceM, 'f', 0));
        setCell(row, 2, numberOrDash(gate.heightM, 'f', 0));
        setCell(row, 3, numberOrDash(gate.windSpeedMps));
        setCell(row, 4, numberOrDash(gate.windDirectionDeg));
        setCell(row, 5, numberOrDash(gate.eastwardMps));
        setCell(row, 6, numberOrDash(gate.northwardMps));
        setCell(row, 7, numberOrDash(gate.upwardMps));
        setCell(row, 8, numberOrDash(gate.cnrAvg));
        setCell(row, 9, gate.validBeams > 0 ? QString::number(gate.validBeams) : QStringLiteral("—"));
        setCell(row, 10, gate.validBeams > 0 ? numberOrDash(gate.retrievalResidualMps, 'f', 2) : QStringLiteral("—"));
        m_table->item(row, 0)->setData(Qt::UserRole, gate.gateIndex);
    }
    m_table->setSortingEnabled(sorting);
    m_table->setUpdatesEnabled(true);
    m_table->viewport()->update();
}

void RangeGateTable::onItemClicked(int row, int column)
{
    if (row < 0 || column < 0 || !m_table->item(row, column)) return;
    const QString fieldName = m_table->horizontalHeaderItem(column)->text();
    const QString value = m_table->item(row, column)->text();
    const QString explanation = column < m_headerHelp.size() ? m_headerHelp.at(column) : QString();
    QToolTip::showText(QCursor::pos(),
        QStringLiteral("<b>%1：%2</b><br>%3").arg(fieldName, value, explanation),
        m_table, m_table->visualItemRect(m_table->item(row, column)), 20000);
    if (m_table->item(row, 0)) emit gateClicked(m_table->item(row, 0)->data(Qt::UserRole).toInt());
}
