#ifndef DATAQUALITYPAGE_H
#define DATAQUALITYPAGE_H

#include <QWidget>

class QLabel;
class MetricCard;

class DataQualityPage : public QWidget
{
    Q_OBJECT

public:
    explicit DataQualityPage(QWidget *parent = nullptr);
    void updateQuality(double confidencePct, int validGates, int totalGates,
                       double blindRatio, double meanCnrDb, double residualMps,
                       const QString &source);

private:
    MetricCard *m_confidenceCard;
    MetricCard *m_validGateCard;
    MetricCard *m_blindRatioCard;
    MetricCard *m_cnrCard;
    MetricCard *m_residualCard;
    QLabel *m_sourceValue;
};

#endif // DATAQUALITYPAGE_H
