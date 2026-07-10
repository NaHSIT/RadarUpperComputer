#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QString>

#include "ui/widgets/AlarmList.h"
#include "ui/widgets/RangeGateTable.h"

class MetricCard;
class WindTrendChart;
class WindRoseWidget;
class BeamHealthGrid;

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;

    void updateWindData(double windSpeed, double windDirection, double confidence,
                        int validGates, double blindRatio, int alarmCount);
    void updateBeamStatus(int index, const QString &status, double cnr, int validGates);
    void updateAlarms(const QVector<AlarmList::AlarmItem> &alarms);
    void updateGateData(const QVector<RangeGateTable::GateData> &data);

signals:
    void beamClicked(int index);
    void alarmClicked(const QString &alarmId);
    void gateClicked(int gateIndex);

private slots:
    void onRefreshTimer();
    void onTimeWindowChanged(const QString &window);

private:
    void setupUI();
    void createMetricCards();
    void createChartArea();
    void createDataTableArea();

    MetricCard *m_windSpeedCard;
    MetricCard *m_windDirectionCard;
    MetricCard *m_confidenceCard;
    MetricCard *m_validGatesCard;
    MetricCard *m_blindRatioCard;
    MetricCard *m_alarmCountCard;

    WindTrendChart *m_windTrendChart;
    WindRoseWidget *m_windRoseWidget;
    BeamHealthGrid *m_beamHealthGrid;

    AlarmList *m_alarmList;
    RangeGateTable *m_gateTable;
    QTimer *m_refreshTimer;
};

#endif // DASHBOARDPAGE_H
