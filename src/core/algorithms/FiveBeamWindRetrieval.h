#ifndef FIVEBEAMWINDRETRIEVAL_H
#define FIVEBEAMWINDRETRIEVAL_H

#include <QVector>

struct BeamObservation
{
    int beamId = 0;
    double azimuthDeg = 0.0;
    double elevationDeg = 0.0;
    double carrierFrequencyHz = 0.0;
    double startRangeM = 0.0;
    double gateSpacingM = 0.0;
    QVector<double> radialVelocityMps;
    QVector<double> cnrDb;
    QVector<double> confidencePct;
};

struct WindVectorLevel
{
    int gateIndex = 0;
    double distanceM = 0.0;
    double heightAglM = 0.0;
    double eastwardMps = 0.0;
    double northwardMps = 0.0;
    double upwardMps = 0.0;
    double horizontalSpeedMps = 0.0;
    double windFromDirectionDeg = 0.0;
    double cnrMeanDb = 0.0;
    double confidencePct = 0.0;
    double residualRmsMps = 0.0;
    int validBeamCount = 0;
    bool valid = false;
};

class FiveBeamWindRetrieval
{
public:
    static QVector<WindVectorLevel> retrieve(const QVector<BeamObservation> &beams);
};

#endif // FIVEBEAMWINDRETRIEVAL_H
