#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <QString>

#include "ui/widgets/AlarmList.h"
#include "ui/widgets/RangeGateTable.h"

class MetricCard;
class WindTrendChart;
class WindRoseWidget;
class BeamHealthGrid;
class QLabel;
class QComboBox;

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;

    void updateWindData(double windSpeed, double windDirection, double heightAglM,
                        int validGates, double blindRatio, int alarmCount);
    void updateBeamStatus(int index, const QString &beamId, double azimuthDeg, double elevationDeg,
                          const QString &status, double cnr, int validGates);
    void updateAlarms(const QVector<AlarmList::AlarmItem> &alarms);
    void updateGateData(const QVector<RangeGateTable::GateData> &data);
    void setDataSource(const QString &source);
    void updateAlarmCount(int count);
    void setSpeedHistory(const QVector<QPointF> &speedMps, int windowSeconds);
    void setDirectionHistory(const QVector<QPointF> &directionDeg, int windowSeconds);
    int speedHistoryWindowSeconds() const { return m_speedHistoryWindowSeconds; }
    int directionHistoryWindowSeconds() const { return m_directionHistoryWindowSeconds; }

signals:
    void beamClicked(int index);
    void alarmClicked(const QString &alarmId);
    void gateClicked(int gateIndex);
    void speedHistoryWindowChanged(int seconds);
    void directionHistoryWindowChanged(int seconds);

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
    MetricCard *m_validGatesCard;
    MetricCard *m_blindRatioCard;
    MetricCard *m_alarmCountCard;

    WindTrendChart *m_windTrendChart;
    WindTrendChart *m_windDirectionChart;
    WindRoseWidget *m_windRoseWidget;
    BeamHealthGrid *m_beamHealthGrid;

    AlarmList *m_alarmList;
    RangeGateTable *m_gateTable;
    QTimer *m_refreshTimer;
    QLabel *m_dataSourceValue;
    QLabel *m_speedSeriesTitle;
    QLabel *m_directionIndicatorTitle;
    QComboBox *m_speedHistoryWindowCombo;
    QComboBox *m_directionHistoryWindowCombo;
    int m_speedHistoryWindowSeconds;
    int m_directionHistoryWindowSeconds;
    double m_displayHeightAglM;
};

#endif // DASHBOARDPAGE_H
