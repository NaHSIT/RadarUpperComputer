#ifndef BEAMHEALTHGRID_H
#define BEAMHEALTHGRID_H

#include <QWidget>
#include <QVector>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QString>

class BeamHealthGrid : public QWidget
{
    Q_OBJECT

public:
    struct BeamStatus {
        QString beamId;
        double azimuthDeg;
        double elevationDeg;
        QString status;
        double cnrAvg;
        int validGates;
    };

    explicit BeamHealthGrid(QWidget *parent = nullptr);
    ~BeamHealthGrid() override;

    void setBeamStatus(int index, const BeamStatus &status);
    void setAllBeamStatus(const QVector<BeamStatus> &statuses);

signals:
    void beamClicked(int index, const QString &beamId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void drawBeamCard(QPainter &painter, int index, const BeamStatus &status, const QRect &rect);

    QVector<BeamStatus> m_beamStatuses;
};

#endif // BEAMHEALTHGRID_H
