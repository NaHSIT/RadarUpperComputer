#include "AlarmList.h"

#include <QHeaderView>
#include <QLabel>
#include <QTableWidgetItem>
#include <QVBoxLayout>

AlarmList::AlarmList(QWidget *parent) : QWidget(parent), m_table(nullptr) { setupUI(); }
AlarmList::~AlarmList() = default;
void AlarmList::setAlarms(const QVector<AlarmItem> &alarms) { m_alarms = alarms; updateTable(); }
void AlarmList::addAlarm(const AlarmItem &alarm) { m_alarms.prepend(alarm); updateTable(); }
void AlarmList::clear() { m_alarms.clear(); updateTable(); }

void AlarmList::setupUI()
{
    auto *layout = new QVBoxLayout(this); layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(0);
    auto *title = new QLabel(QStringLiteral("活动告警"), this);
    title->setStyleSheet("color:#263442; font-size:14px; font-weight:600; padding:0 0 8px;"); layout->addWidget(title);
    m_table = new QTableWidget(this); m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({QStringLiteral("等级"), QStringLiteral("来源"), QStringLiteral("告警内容"), QStringLiteral("首次时间")});
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch); m_table->verticalHeader()->hide();
    m_table->setEditTriggers(QTableWidget::NoEditTriggers); m_table->setSelectionBehavior(QTableWidget::SelectRows);
    m_table->setShowGrid(false); m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet(
        "QTableWidget { background:#fff; border:1px solid #d9dee5; border-radius:2px; font-size:12px; }"
        "QTableWidget::item { padding:7px 9px; border-bottom:1px solid #edf0f2; }"
        "QTableWidget::item:selected { background:#eaf1f6; color:#182230; }"
        "QTableWidget::item:alternate { background:#f8fafb; }"
        "QHeaderView::section { background:#f2f5f7; color:#52606d; font-weight:600; font-size:12px; padding:7px 9px; border:0; border-bottom:1px solid #d9dee5; }"
    );
    connect(m_table, &QTableWidget::cellClicked, this, &AlarmList::onItemClicked); layout->addWidget(m_table);
}

void AlarmList::updateTable()
{
    m_table->setRowCount(0);
    for (const auto &alarm : m_alarms) {
        const int row = m_table->rowCount(); m_table->insertRow(row);
        auto *severity = new QTableWidgetItem(alarm.severity);
        if (alarm.severity.contains(QStringLiteral("严重")) || alarm.severity.contains("Critical")) severity->setForeground(QColor("#b42318"));
        else if (alarm.severity.contains(QStringLiteral("警告")) || alarm.severity.contains("Warning")) severity->setForeground(QColor("#9a6700"));
        else severity->setForeground(QColor("#2f6f9f"));
        m_table->setItem(row, 0, severity); m_table->setItem(row, 1, new QTableWidgetItem(alarm.source));
        m_table->setItem(row, 2, new QTableWidgetItem(alarm.title));
        m_table->setItem(row, 3, new QTableWidgetItem(alarm.firstSeen.toString("HH:mm:ss")));
    }
}

void AlarmList::onItemClicked(int row, int column)
{
    Q_UNUSED(column)
    if (row >= 0 && row < m_alarms.size()) emit alarmClicked(m_alarms[row].alarmId);
}
