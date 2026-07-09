#include "RangeGateTable.h"
#include <QVBoxLayout>
#include <QHeaderView>

RangeGateTable::RangeGateTable(QWidget *parent)
    : QWidget(parent)
    , m_table(nullptr)
{
    setupUI();
}

RangeGateTable::~RangeGateTable()
{
}

void RangeGateTable::setGateData(const QVector<GateData> &data)
{
    m_gateData = data;
    updateTable();
}

void RangeGateTable::clear()
{
    m_gateData.clear();
    updateTable();
}

void RangeGateTable::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLabel = new QLabel("分层风场数据", this);
    titleLabel->setStyleSheet("color: #333; font-size: 12px; font-weight: bold; padding: 5px;");
    layout->addWidget(titleLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({
        "层号", "距离(m)", "高度(m)", "风速(m/s)", "风向(°)",
        "CNR(dB)", "置信度", "状态"
    });
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
    m_table->verticalHeader()->hide();
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);
    m_table->setSelectionBehavior(QTableWidget::SelectRows);
    m_table->setSortingEnabled(true);

    connect(m_table, &QTableWidget::cellClicked, this, &RangeGateTable::onItemClicked);

    layout->addWidget(m_table);
}

void RangeGateTable::updateTable()
{
    m_table->setRowCount(0);

    for (const auto &gate : m_gateData) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(gate.gateIndex)));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::number(gate.distanceM, 'f', 0)));
        m_table->setItem(row, 2, new QTableWidgetItem(QString::number(gate.heightM, 'f', 0)));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::number(gate.windSpeedMps, 'f', 1)));
        m_table->setItem(row, 4, new QTableWidgetItem(QString::number(gate.windDirectionDeg, 'f', 1)));
        m_table->setItem(row, 5, new QTableWidgetItem(QString::number(gate.cnrAvg, 'f', 1)));
        m_table->setItem(row, 6, new QTableWidgetItem(QString::number(gate.confidence, 'f', 0)));
        m_table->setItem(row, 7, new QTableWidgetItem(gate.status));
    }
}

void RangeGateTable::onItemClicked(int row, int column)
{
    Q_UNUSED(column)

    if (row >= 0 && row < m_gateData.size()) {
        emit gateClicked(m_gateData[row].gateIndex);
    }
}
