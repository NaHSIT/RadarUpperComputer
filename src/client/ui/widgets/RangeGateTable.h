#ifndef RANGEGATETABLE_H
#define RANGEGATETABLE_H

#include <QWidget>
#include <QTableWidget>
#include <QVector>
#include <QString>

class RangeGateTable : public QWidget
{
    Q_OBJECT

public:
    struct GateData {
        int gateIndex;
        double distanceM;
        double heightM;
        double windSpeedMps;
        double windDirectionDeg;
        double turbulenceIntensity;
        double verticalShear;
        double horizontalShear;
        double cnrAvg;
        int validBeams;
        double confidence;
        QString status;
    };

    explicit RangeGateTable(QWidget *parent = nullptr);
    ~RangeGateTable() override;

    void setGateData(const QVector<GateData> &data);
    void clear();

signals:
    void gateClicked(int gateIndex);

private slots:
    void onItemClicked(int row, int column);

private:
    void setupUI();
    void updateTable();

    QTableWidget *m_table;
    QVector<GateData> m_gateData;
};

#endif // RANGEGATETABLE_H
