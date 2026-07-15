#ifndef BEAMPAGE_H
#define BEAMPAGE_H

#include <QWidget>
#include <QVector>

class QLabel;
class QTableWidget;
class BeamState;

/**
 * @brief 波束页面
 *
 * 诊断五波束是否一致、是否遮挡、是否弱信号、是否相位异常
 */
class BeamPage : public QWidget
{
    Q_OBJECT

public:
    explicit BeamPage(QWidget *parent = nullptr);
    ~BeamPage() override;

    void setConnectionState(bool connected);
    void updateSimulationData(double cnrDb, int validGates);
    void updateBeamData(const QVector<BeamState *> &beams);

private:
    void setupUI();

    QLabel *m_statusLine = nullptr;
    QTableWidget *m_beamTable = nullptr;
    bool m_connected = false;
};

#endif // BEAMPAGE_H
