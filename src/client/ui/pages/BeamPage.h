#ifndef BEAMPAGE_H
#define BEAMPAGE_H

#include <QWidget>

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

private:
    void setupUI();
};

#endif // BEAMPAGE_H
