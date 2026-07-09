#ifndef WINDPROFILE_H
#define WINDPROFILE_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include "RadarTypes.h"
#include "BeamState.h"
#include "RangeGate.h"

/**
 * @brief 风廓线模型
 *
 * 描述完整的风场数据
 */
class WindProfile : public QObject
{
    Q_OBJECT

public:
    explicit WindProfile(QObject *parent = nullptr);
    ~WindProfile() override;

    // 属性访问器
    QDateTime timestampUtc() const { return m_timestampUtc; }
    TimeQuality timeQuality() const { return m_timeQuality; }
    int rangeResolutionM() const { return m_rangeResolutionM; }
    int gateCount() const { return m_gateCount; }
    double hubHeightM() const { return m_hubHeightM; }
    double lidarHeightM() const { return m_lidarHeightM; }
    double rollDeg() const { return m_rollDeg; }
    double tiltDeg() const { return m_tiltDeg; }
    double hubWindSpeedMps() const { return m_hubWindSpeedMps; }
    double hubWindDirectionDeg() const { return m_hubWindDirectionDeg; }
    double rawsMps() const { return m_rawsMps; }
    QVector<RangeGate*> rangeGates() const { return m_rangeGates; }
    QVector<BeamState*> beamStates() const { return m_beamStates; }
    int validGateCount() const;
    double blindRatio() const;
    double confidence() const;

    // 设置方法
    void setTimestampUtc(const QDateTime &time);
    void setTimeQuality(TimeQuality quality);
    void setRangeResolutionM(int resolution);
    void setGateCount(int count);
    void setHubHeightM(double height);
    void setLidarHeightM(double height);
    void setRollDeg(double roll);
    void setTiltDeg(double tilt);
    void setHubWindSpeedMps(double speed);
    void setHubWindDirectionDeg(double direction);
    void setRawsMps(double raws);

    // 数据操作
    void addRangeGate(RangeGate *gate);
    void addBeamState(BeamState *beam);
    void clearRangeGates();
    void clearBeamStates();

    // 查找
    RangeGate* findGateByHeight(double height) const;
    BeamState* findBeamById(BeamId id) const;

    // 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

signals:
    void dataUpdated();
    void rangeGatesChanged();
    void beamStatesChanged();

private:
    QDateTime m_timestampUtc;
    TimeQuality m_timeQuality;
    int m_rangeResolutionM;
    int m_gateCount;
    double m_hubHeightM;
    double m_lidarHeightM;
    double m_rollDeg;
    double m_tiltDeg;
    double m_hubWindSpeedMps;
    double m_hubWindDirectionDeg;
    double m_rawsMps;
    QVector<RangeGate*> m_rangeGates;
    QVector<BeamState*> m_beamStates;
};

#endif // WINDPROFILE_H
