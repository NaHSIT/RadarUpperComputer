#ifndef RANGEGATE_H
#define RANGEGATE_H

#include <QObject>
#include <QVector>
#include "RadarTypes.h"

/**
 * @brief 距离层模型
 *
 * 描述单个距离层的风场数据
 */
class RangeGate : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int gateIndex READ gateIndex WRITE setGateIndex)
    Q_PROPERTY(double distanceM READ distanceM WRITE setDistanceM)
    Q_PROPERTY(double heightM READ heightM WRITE setHeightM)
    Q_PROPERTY(double windSpeedMps READ windSpeedMps WRITE setWindSpeedMps)
    Q_PROPERTY(double windDirectionDeg READ windDirectionDeg WRITE setWindDirectionDeg)
    Q_PROPERTY(double turbulenceIntensity READ turbulenceIntensity WRITE setTurbulenceIntensity)
    Q_PROPERTY(double verticalShear READ verticalShear WRITE setVerticalShear)
    Q_PROPERTY(double horizontalShear READ horizontalShear WRITE setHorizontalShear)
    Q_PROPERTY(double confidence READ confidence WRITE setConfidence)

public:
    explicit RangeGate(QObject *parent = nullptr);
    ~RangeGate() override;

    // 属性访问器
    int gateIndex() const { return m_gateIndex; }
    double distanceM() const { return m_distanceM; }
    double heightM() const { return m_heightM; }
    double windSpeedMps() const { return m_windSpeedMps; }
    double windDirectionDeg() const { return m_windDirectionDeg; }
    QVector<double> radialWindSpeedMps() const { return m_radialWindSpeedMps; }
    QVector<double> cnrDb() const { return m_cnrDb; }
    double turbulenceIntensity() const { return m_turbulenceIntensity; }
    double verticalShear() const { return m_verticalShear; }
    double horizontalShear() const { return m_horizontalShear; }
    double veerDegPerM() const { return m_veerDegPerM; }
    double confidence() const { return m_confidence; }
    QVector<StatusCode> statusFlags() const { return m_statusFlags; }

    // 设置方法
    void setGateIndex(int index);
    void setDistanceM(double distance);
    void setHeightM(double height);
    void setWindSpeedMps(double speed);
    void setWindDirectionDeg(double direction);
    void setRadialWindSpeedMps(const QVector<double> &rws);
    void setCnrDb(const QVector<double> &cnr);
    void setTurbulenceIntensity(double ti);
    void setVerticalShear(double shear);
    void setHorizontalShear(double shear);
    void setVeerDegPerM(double veer);
    void setConfidence(double confidence);
    void setStatusFlags(const QVector<StatusCode> &flags);

    // 数据操作
    void appendStatusCode(StatusCode code);
    void clearStatusFlags();

    // 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

signals:
    void dataUpdated();

private:
    int m_gateIndex;
    double m_distanceM;
    double m_heightM;
    double m_windSpeedMps;
    double m_windDirectionDeg;
    QVector<double> m_radialWindSpeedMps;
    QVector<double> m_cnrDb;
    double m_turbulenceIntensity;
    double m_verticalShear;
    double m_horizontalShear;
    double m_veerDegPerM;
    double m_confidence;
    QVector<StatusCode> m_statusFlags;
};

#endif // RANGEGATE_H
