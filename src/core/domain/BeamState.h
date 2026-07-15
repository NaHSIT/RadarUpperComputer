#ifndef BEAMSTATE_H
#define BEAMSTATE_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include "RadarTypes.h"

/**
 * @brief 波束状态模型
 *
 * 描述单个波束的状态信息
 */
class BeamState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(BeamId beamId READ beamId WRITE setBeamId)
    Q_PROPERTY(double azimuthDeg READ azimuthDeg WRITE setAzimuthDeg)
    Q_PROPERTY(double elevationDeg READ elevationDeg WRITE setElevationDeg)
    Q_PROPERTY(double carrierFrequencyHz READ carrierFrequencyHz WRITE setCarrierFrequencyHz)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(BeamStatus status READ status WRITE setStatus)
    Q_PROPERTY(double phaseErrorDeg READ phaseErrorDeg WRITE setPhaseErrorDeg)

public:
    explicit BeamState(QObject *parent = nullptr);
    ~BeamState() override;

    // 属性访问器
    BeamId beamId() const { return m_beamId; }
    double azimuthDeg() const { return m_azimuthDeg; }
    double elevationDeg() const { return m_elevationDeg; }
    double carrierFrequencyHz() const { return m_carrierFrequencyHz; }
    bool enabled() const { return m_enabled; }
    BeamStatus status() const { return m_status; }
    QVector<double> cnrDbByGate() const { return m_cnrDbByGate; }
    QVector<double> rwsByGate() const { return m_rwsByGate; }
    QVector<double> confidenceByGate() const { return m_confidenceByGate; }
    double phaseErrorDeg() const { return m_phaseErrorDeg; }
    QDateTime lastUpdateTime() const { return m_lastUpdateTime; }

    // 设置方法
    void setBeamId(BeamId id);
    void setAzimuthDeg(double azimuth);
    void setElevationDeg(double elevation);
    void setCarrierFrequencyHz(double frequency);
    void setEnabled(bool enabled);
    void setStatus(BeamStatus status);
    void setCnrDbByGate(const QVector<double> &cnr);
    void setRwsByGate(const QVector<double> &rws);
    void setConfidenceByGate(const QVector<double> &confidence);
    void setPhaseErrorDeg(double error);
    void setLastUpdateTime(const QDateTime &time);

    // 数据操作
    void appendCnr(double cnr);
    void appendRws(double rws);
    void appendConfidence(double confidence);
    void clearData();

    // 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

signals:
    void statusChanged(BeamStatus status);
    void dataUpdated();

private:
    BeamId m_beamId;
    double m_azimuthDeg;
    double m_elevationDeg;
    double m_carrierFrequencyHz;
    bool m_enabled;
    BeamStatus m_status;
    QVector<double> m_cnrDbByGate;
    QVector<double> m_rwsByGate;
    QVector<double> m_confidenceByGate;
    double m_phaseErrorDeg;
    QDateTime m_lastUpdateTime;
};

#endif // BEAMSTATE_H
