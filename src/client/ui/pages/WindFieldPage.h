#ifndef WINDFIELDPAGE_H
#define WINDFIELDPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVector>

#include "ui/widgets/RangeGateTable.h"

class WindTrendChart;

class WindFieldPage : public QWidget
{
    Q_OBJECT

public:
    explicit WindFieldPage(QWidget *parent = nullptr);
    ~WindFieldPage() override;

    void updateWindData(double windSpeed, double windDirection, double confidence);
    void updateGateData(const QVector<RangeGateTable::GateData> &data);

private slots:
    void onTimeWindowChanged(const QString &window);
    void onResolutionChanged(const QString &resolution);

private:
    void setupUI();
    void createHeader();
    void createCharts();
    void createTable();

    QComboBox *m_timeWindowCombo;
    QComboBox *m_resolutionCombo;
    QHBoxLayout *m_headerLayout;
    WindTrendChart *m_windSpeedChart;
    WindTrendChart *m_windDirectionChart;
    RangeGateTable *m_gateTable;
};

#endif // WINDFIELDPAGE_H
