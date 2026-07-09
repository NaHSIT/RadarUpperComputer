#include "AlarmList.h"
#include <QVBoxLayout>
#include <QHeaderView>

AlarmList::AlarmList(QWidget *parent)
    : QWidget(parent)
    , m_table(nullptr)
{
    setupUI();
}

AlarmList::~AlarmList()
{
}

void AlarmList::setAlarms(const QVector<AlarmItem> &alarms)
{
    m_alarms = alarms;
    updateTable();
}

void AlarmList::addAlarm(const AlarmItem &alarm)
{
    m_alarms.prepend(alarm);
    updateTable();
}

void AlarmList::clear()
{
    m_alarms.clear();
    updateTable();
}

void AlarmList::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLabel = new QLabel("当前告警", this);
    titleLabel->setStyleSheet("color: #333; font-size: 12px; font-weight: bold; padding: 5px;");
    layout->addWidget(titleLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"等级", "来源", "标题", "时间"});
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->verticalHeader()->hide();
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);
    m_table->setSelectionBehavior(QTableWidget::SelectRows);

    connect(m_table, &QTableWidget::cellClicked, this, &AlarmList::onItemClicked);

    layout->addWidget(m_table);
}

void AlarmList::updateTable()
{
    m_table->setRowCount(0);

    for (const auto &alarm : m_alarms) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, 0, new QTableWidgetItem(alarm.severity));
        m_table->setItem(row, 1, new QTableWidgetItem(alarm.source));
        m_table->setItem(row, 2, new QTableWidgetItem(alarm.title));
        m_table->setItem(row, 3, new QTableWidgetItem(alarm.firstSeen.toString("HH:mm:ss")));
    }
}

void AlarmList::onItemClicked(int row, int column)
{
    Q_UNUSED(column)

    if (row >= 0 && row < m_alarms.size()) {
        emit alarmClicked(m_alarms[row].alarmId);
    }
}
