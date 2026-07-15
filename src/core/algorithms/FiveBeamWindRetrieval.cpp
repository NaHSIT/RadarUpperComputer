#include "FiveBeamWindRetrieval.h"

#include <QtMath>

namespace {

bool solveThreeByThree(double matrix[3][3], double right[3], double result[3])
{
    double augmented[3][4] = {};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            augmented[row][column] = matrix[row][column];
        }
        augmented[row][3] = right[row];
    }

    for (int pivot = 0; pivot < 3; ++pivot) {
        int bestRow = pivot;
        for (int row = pivot + 1; row < 3; ++row) {
            if (qAbs(augmented[row][pivot]) > qAbs(augmented[bestRow][pivot])) bestRow = row;
        }
        if (qAbs(augmented[bestRow][pivot]) < 1.0e-8) return false;
        if (bestRow != pivot) {
            for (int column = pivot; column < 4; ++column) {
                qSwap(augmented[pivot][column], augmented[bestRow][column]);
            }
        }

        const double divisor = augmented[pivot][pivot];
        for (int column = pivot; column < 4; ++column) augmented[pivot][column] /= divisor;
        for (int row = 0; row < 3; ++row) {
            if (row == pivot) continue;
            const double factor = augmented[row][pivot];
            for (int column = pivot; column < 4; ++column) {
                augmented[row][column] -= factor * augmented[pivot][column];
            }
        }
    }

    for (int index = 0; index < 3; ++index) result[index] = augmented[index][3];
    return true;
}

double normalizedWindFromDirection(double eastward, double northward)
{
    double direction = qRadiansToDegrees(qAtan2(-eastward, -northward));
    if (direction < 0.0) direction += 360.0;
    return direction;
}

}

QVector<WindVectorLevel> FiveBeamWindRetrieval::retrieve(const QVector<BeamObservation> &beams)
{
    int gateCount = 0;
    for (const BeamObservation &beam : beams) {
        gateCount = qMax(gateCount, beam.radialVelocityMps.size());
    }

    QVector<WindVectorLevel> levels;
    levels.reserve(gateCount);
    for (int gate = 0; gate < gateCount; ++gate) {
        WindVectorLevel level;
        level.gateIndex = gate;
        double normal[5][3] = {};
        double velocities[5] = {};
        double weights[5] = {};
        double matrix[3][3] = {};
        double right[3] = {};
        double heightSum = 0.0;
        double distanceSum = 0.0;
        double cnrSum = 0.0;
        double confidenceSum = 0.0;

        for (const BeamObservation &beam : beams) {
            if (gate >= beam.radialVelocityMps.size()) continue;
            const double radial = beam.radialVelocityMps.at(gate);
            const double confidence = gate < beam.confidencePct.size()
                ? beam.confidencePct.at(gate) : 0.0;
            const double cnr = gate < beam.cnrDb.size() ? beam.cnrDb.at(gate) : -100.0;
            if (!qIsFinite(radial) || confidence < 20.0 || cnr < -30.0 || level.validBeamCount >= 5) continue;

            const double azimuth = qDegreesToRadians(beam.azimuthDeg);
            const double elevation = qDegreesToRadians(beam.elevationDeg);
            const double direction[3] = {
                qCos(elevation) * qSin(azimuth),
                qCos(elevation) * qCos(azimuth),
                qSin(elevation)
            };
            const double cnrWeight = qBound(0.1, (cnr + 30.0) / 40.0, 1.0);
            const double weight = qMax(0.01, confidence / 100.0 * cnrWeight);
            const int observation = level.validBeamCount++;
            for (int component = 0; component < 3; ++component) normal[observation][component] = direction[component];
            velocities[observation] = radial;
            weights[observation] = weight;

            for (int row = 0; row < 3; ++row) {
                right[row] += weight * direction[row] * radial;
                for (int column = 0; column < 3; ++column) {
                    matrix[row][column] += weight * direction[row] * direction[column];
                }
            }

            const double slantRange = beam.startRangeM + gate * beam.gateSpacingM;
            heightSum += slantRange * qSin(elevation);
            distanceSum += slantRange;
            cnrSum += cnr;
            confidenceSum += confidence;
        }

        level.distanceM = level.validBeamCount > 0 ? distanceSum / level.validBeamCount : 0.0;
        level.heightAglM = level.validBeamCount > 0 ? heightSum / level.validBeamCount : 0.0;
        level.cnrMeanDb = level.validBeamCount > 0 ? cnrSum / level.validBeamCount : -100.0;
        level.confidencePct = level.validBeamCount > 0 ? confidenceSum / level.validBeamCount : 0.0;

        double vector[3] = {};
        if (level.validBeamCount >= 3 && solveThreeByThree(matrix, right, vector)) {
            level.eastwardMps = vector[0];
            level.northwardMps = vector[1];
            level.upwardMps = vector[2];
            level.horizontalSpeedMps = qSqrt(vector[0] * vector[0] + vector[1] * vector[1]);
            level.windFromDirectionDeg = normalizedWindFromDirection(vector[0], vector[1]);

            double weightedSquaredError = 0.0;
            double weightSum = 0.0;
            for (int observation = 0; observation < level.validBeamCount; ++observation) {
                const double predicted = normal[observation][0] * vector[0]
                    + normal[observation][1] * vector[1]
                    + normal[observation][2] * vector[2];
                const double error = velocities[observation] - predicted;
                weightedSquaredError += weights[observation] * error * error;
                weightSum += weights[observation];
            }
            level.residualRmsMps = qSqrt(weightedSquaredError / qMax(0.001, weightSum));
            // Residual quality follows the same 1.5 m/s acceptance scale used
            // below. The previous exp(-r/0.8) curve over-penalized ordinary
            // simulation noise and effectively introduced a much lower limit.
            const double normalizedResidual = level.residualRmsMps / 1.5;
            const double residualQuality = qExp(-0.5 * normalizedResidual * normalizedResidual);
            level.confidencePct = qBound(0.0,
                level.confidencePct * residualQuality, 100.0);
            level.valid = level.confidencePct >= 50.0 && level.residualRmsMps <= 1.5;
        }
        levels.append(level);
    }
    return levels;
}
