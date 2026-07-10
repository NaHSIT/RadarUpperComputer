#ifndef ALARMLIST_H
#define ALARMLIST_H

#include <QWidget>
#include <QTableWidget>
#include <QVector>
#include <QDateTime>
#include <QString>

class AlarmList : public QWidget
{
    Q_OBJECT

public:
    struct AlarmItem {
        QString alarmId;
        QString severity;
        QString source;
        QString title;
        QString description;
        QDateTime firstSeen;
        QDateTime lastSeen;
        bool acknowledged;
    };

    explicit AlarmList(QWidget *parent = nullptr);
    ~AlarmList() override;

    void setAlarms(const QVector<AlarmItem> &alarms);
    void addAlarm(const AlarmItem &alarm);
    void clear();

signals:
    void alarmClicked(const QString &alarmId);

private slots:
    void onItemClicked(int row, int column);

private:
    void setupUI();
    void updateTable();

    QTableWidget *m_table;
    QVector<AlarmItem> m_alarms;
};

#endif // ALARMLIST_H
