#include "algorithms/FiveBeamWindRetrieval.h"

#include <QCoreApplication>
#include <QtMath>

#include <iostream>

namespace {

double project(double azimuthDeg, double elevationDeg, double eastward,
               double northward, double upward)
{
    const double azimuth = qDegreesToRadians(azimuthDeg);
    const double elevation = qDegreesToRadians(elevationDeg);
    return eastward * qCos(elevation) * qSin(azimuth)
        + northward * qCos(elevation) * qCos(azimuth)
        + upward * qSin(elevation);
}

bool closeTo(double actual, double expected, double tolerance = 1.0e-6)
{
    if (qAbs(actual - expected) <= tolerance) return true;
    std::cerr << "expected " << expected << ", got " << actual << '\n';
    return false;
}

}

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    Q_UNUSED(application)

    const double azimuths[] = {0.0, 45.0, 135.0, 225.0, 315.0};
    const double elevations[] = {90.0, 75.0, 75.0, 75.0, 75.0};
    QVector<BeamObservation> beams;
    for (int beamIndex = 0; beamIndex < 5; ++beamIndex) {
        BeamObservation beam;
        beam.beamId = beamIndex;
        beam.azimuthDeg = azimuths[beamIndex];
        beam.elevationDeg = elevations[beamIndex];
        beam.carrierFrequencyHz = (23.8 + beamIndex * 0.1) * 1.0e9;
        beam.startRangeM = 10.0 / qSin(qDegreesToRadians(beam.elevationDeg));
        beam.gateSpacingM = 10.0 / qSin(qDegreesToRadians(beam.elevationDeg));
        for (int gate = 0; gate < 30; ++gate) {
            const double eastward = 4.0 + gate * 0.08;
            const double northward = -2.0 + gate * 0.03;
            const double upward = 0.25 * qSin(gate / 6.0);
            beam.radialVelocityMps.append(project(beam.azimuthDeg, beam.elevationDeg,
                                                  eastward, northward, upward));
            beam.cnrDb.append(15.0);
            beam.confidencePct.append(96.0);
        }
        beams.append(beam);
    }

    const QVector<WindVectorLevel> levels = FiveBeamWindRetrieval::retrieve(beams);
    if (levels.size() != 30) return 1;
    for (int gate = 0; gate < levels.size(); ++gate) {
        const WindVectorLevel &level = levels.at(gate);
        if (!level.valid || level.validBeamCount != 5
            || !closeTo(level.heightAglM, (gate + 1) * 10.0)
            || !closeTo(level.eastwardMps, 4.0 + gate * 0.08)
            || !closeTo(level.northwardMps, -2.0 + gate * 0.03)
            || !closeTo(level.upwardMps, 0.25 * qSin(gate / 6.0))) return 2;
    }

    std::cout << "five-beam retrieval: 30/30 levels recovered\n";
    return 0;
}
