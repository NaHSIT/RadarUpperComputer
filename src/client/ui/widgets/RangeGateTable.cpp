#include "RangeGateTable.h"

#include <QHeaderView>
#include <QLabel>
#include <QTableWidgetItem>
#include <QVBoxLayout>

RangeGateTable::RangeGateTable(QWidget *parent) : QWidget(parent), m_table(nullptr) { setupUI(); }
RangeGateTable::~RangeGateTable() = default;
void RangeGateTable::setGateData(const QVector<GateData> &data) { m_gateData = data; updateTable(); }
void RangeGateTable::clear() { m_gateData.clear(); updateTable(); }

void RangeGateTable::setupUI()
{
    auto *layout = new QVBoxLayout(this); layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(0);
    auto *title = new QLabel(QStringLiteral("分层风场数据"), this);
    title->setStyleSheet("color:#263442; font-size:14px; font-weight:600; padding:0 0 8px;"); layout->addWidget(title);
    m_table = new QTableWidget(this); m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({QStringLiteral("层号"), QStringLiteral("距离 (m)"), QStringLiteral("高度 (m)"), QStringLiteral("风速 (m/s)"), QStringLiteral("风向 (°)"), QStringLiteral("CNR (dB)"), QStringLiteral("置信度 (%)"), QStringLiteral("状态")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); m_table->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
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
    m_table->setSortingEnabled(false); m_table->setRowCount(0);
    for (const auto &gate : m_gateData) {
        const int row = m_table->rowCount(); m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(gate.gateIndex)));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::number(gate.distanceM, 'f', 0)));
        m_table->setItem(row, 2, new QTableWidgetItem(QString::number(gate.heightM, 'f', 0)));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::number(gate.windSpeedMps, 'f', 1)));
        m_table->setItem(row, 4, new QTableWidgetItem(QString::number(gate.windDirectionDeg, 'f', 1)));
        m_table->setItem(row, 5, new QTableWidgetItem(QString::number(gate.cnrAvg, 'f', 1)));
        m_table->setItem(row, 6, new QTableWidgetItem(QString::number(gate.confidence, 'f', 0)));
        auto *status = new QTableWidgetItem(gate.status);
        if (gate.status == QStringLiteral("正常") || gate.status == "Valid") status->setForeground(QColor("#16713b"));
        else if (gate.status == QStringLiteral("低信噪比") || gate.status == "Low SNR") status->setForeground(QColor("#9a6700"));
        else status->setForeground(QColor("#b42318"));
        m_table->setItem(row, 7, status);
    }
    m_table->setSortingEnabled(true);
}

void RangeGateTable::onItemClicked(int row, int column)
{
    Q_UNUSED(column)
    if (row >= 0 && row < m_gateData.size()) emit gateClicked(m_gateData[row].gateIndex);
}
