#ifndef SPECTRUMPAGE_H
#define SPECTRUMPAGE_H

#include <QWidget>

/**
 * @brief 频谱页面
 *
 * 显示功率谱、谱峰和杂波诊断
 */
class SpectrumPage : public QWidget
{
    Q_OBJECT

public:
    explicit SpectrumPage(QWidget *parent = nullptr);
    ~SpectrumPage() override;

private:
    void setupUI();
};

#endif // SPECTRUMPAGE_H
