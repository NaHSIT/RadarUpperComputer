#ifndef WINDFIELDPAGE_H
#define WINDFIELDPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVector>
#include <QJsonObject>
#include <QDateTime>
#include <QPointF>

#include "ui/widgets/RangeGateTable.h"

class WindTrendChart;
class WindVectorFieldWidget;
class QLabel;
class QPushButton;
class QFrame;

class WindFieldPage : public QWidget
{
    Q_OBJECT

public:
    explicit WindFieldPage(QWidget *parent = nullptr);
    ~WindFieldPage() override;

    void updateWindData(double windSpeed, double windDirection, double heightAglM);
    void updateGateData(const QVector<RangeGateTable::GateData> &data);
    void showRadarDirectSource(const QDateTime &timestamp = QDateTime(),
                               int validGateCount = 0, int gateCount = 0,
                               double meanCnrDb = qQNaN());
    void showFiveBeamSource(const QDateTime &timestamp, int validGateCount,
                            int gateCount, double meanCnrDb, quint32 scanId);
    void setPyArtAvailability(bool available, const QString &version, const QString &message = QString());
    void setPyArtBusy(bool busy);
    void showPyArtResult(const QJsonObject &result);
    void showPyArtError(const QString &message);
    void setSpeedHistory(const QVector<QPointF> &speedMps, int windowSeconds);
    void setDirectionHistory(const QVector<QPointF> &directionDeg, int windowSeconds);
    int speedHistoryWindowSeconds() const { return m_speedHistoryWindowSeconds; }
    int directionHistoryWindowSeconds() const { return m_directionHistoryWindowSeconds; }

signals:
    void pyArtValidationRequested();
    void speedHistoryWindowChanged(int seconds);
    void directionHistoryWindowChanged(int seconds);

private slots:
    void onTimeWindowChanged(const QString &window);
    void onResolutionChanged(const QString &resolution);

private:
    void setupUI();
    void createHeader();
    void createCharts();
    void createTable();
    void createAlgorithmPanel();
    void createProductPreview();
    void createVectorField();

    QComboBox *m_timeWindowCombo;
    QComboBox *m_resolutionCombo;
    QComboBox *m_speedHistoryWindowCombo;
    QComboBox *m_directionHistoryWindowCombo;
    QHBoxLayout *m_headerLayout;
    WindTrendChart *m_windSpeedChart;
    WindTrendChart *m_windDirectionChart;
    WindVectorFieldWidget *m_vectorField;
    RangeGateTable *m_gateTable;
    QLabel *m_dataSourceLabel;
    QLabel *m_dataSourceDetailLabel;
    QLabel *m_productTypeValue;
    QLabel *m_observationTimeValue;
    QLabel *m_qualityValue;
    QLabel *m_algorithmValue;
    QLabel *m_pyArtStatusLabel;
    QPushButton *m_runPyArtButton;
    QPushButton *m_openOutputButton;
    QFrame *m_productPanel;
    QLabel *m_productPreview;
    QLabel *m_productPathLabel;
    QLabel *m_speedSeriesTitle;
    QLabel *m_directionSeriesTitle;
    double m_seriesHeightAglM;
    int m_speedHistoryWindowSeconds;
    int m_directionHistoryWindowSeconds;
    QString m_productDirectory;
};

#endif // WINDFIELDPAGE_H
