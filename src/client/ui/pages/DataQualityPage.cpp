#include "DataQualityPage.h"

#include "ui/widgets/MetricCard.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace {
QFrame *section(QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName("qualitySection");
    frame->setStyleSheet("QFrame#qualitySection { background:#ffffff; border:1px solid #d9dee5; border-radius:4px; }");
    return frame;
}
}

DataQualityPage::DataQualityPage(QWidget *parent)
    : QWidget(parent)
    , m_confidenceCard(new MetricCard(this))
    , m_validGateCard(new MetricCard(this))
    , m_blindRatioCard(new MetricCard(this))
    , m_cnrCard(new MetricCard(this))
    , m_residualCard(new MetricCard(this))
    , m_sourceValue(nullptr)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 18);
    layout->setSpacing(14);

    auto *title = new QLabel(QStringLiteral("数据质量"), this);
    title->setStyleSheet("color:#182230; font-size:22px; font-weight:600;");
    layout->addWidget(title);
    auto *subtitle = new QLabel(QStringLiteral("查看风场反演是否可信，以及质量判定所依据的观测量。"), this);
    subtitle->setStyleSheet("color:#667085; font-size:13px;");
    layout->addWidget(subtitle);

    auto *sourcePanel = section(this);
    auto *sourceLayout = new QHBoxLayout(sourcePanel);
    sourceLayout->setContentsMargins(16, 11, 16, 11);
    auto *sourceLabel = new QLabel(QStringLiteral("数据来源"), sourcePanel);
    sourceLabel->setStyleSheet("color:#667085; font-size:12px; font-weight:600;");
    sourceLayout->addWidget(sourceLabel);
    m_sourceValue = new QLabel(QStringLiteral("等待雷达数据"), sourcePanel);
    m_sourceValue->setStyleSheet("color:#1d2939; font-size:13px; font-weight:600;");
    sourceLayout->addWidget(m_sourceValue);
    sourceLayout->addStretch();
    layout->addWidget(sourcePanel);

    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);
    m_confidenceCard->setData(QStringLiteral("综合置信度"), 0.0, "%");
    m_confidenceCard->setToolTip(QStringLiteral("<b>风廓线平均反演置信度 C̅</b><br>"
        "C̅ = (1/N<sub>valid</sub>)ΣC<sub>i</sub>。五波束 WLS 模式下，单层残差修正采用 exp[-0.5(ε<sub>RMS</sub>/1.5)²]。"
        "该量为算法质量指标，无量纲，非雷达直接观测量。"));
    m_validGateCard->setData(QStringLiteral("有效反演层数"), 0.0, QStringLiteral("层"));
    m_validGateCard->setToolTip(QStringLiteral("<b>有效反演层数 N<sub>valid</sub></b><br>"
        "统计同时满足 N<sub>beam</sub>≥3、C<sub>i</sub>≥50% 且 ε<sub>RMS</sub>≤1.5 m/s 的高度层。"));
    m_blindRatioCard->setData(QStringLiteral("无效层比例"), 0.0, "%");
    m_blindRatioCard->setToolTip(QStringLiteral("<b>无效反演层比例 R<sub>invalid</sub></b><br>"
        "R<sub>invalid</sub> = 1 - N<sub>valid</sub>/N<sub>total</sub>。用于表征当前风廓线的垂直数据缺失程度。"));
    m_cnrCard->setData(QStringLiteral("平均 CNR"), 0.0, "dB");
    m_cnrCard->setToolTip(QStringLiteral("<b>平均载噪比 CNR̅</b><br>"
        "CNR = 10log<sub>10</sub>(P<sub>carrier</sub>/P<sub>noise</sub>)，单位 dB。当前值为 0x8105 中参与反演之波束/距离门 CNR 的算术平均。"));
    m_residualCard->setData(QStringLiteral("反演残差"), 0.0, "m/s");
    m_residualCard->setToolTip(QStringLiteral("<b>平均 WLS 反演残差 ε̅<sub>RMS</sub></b><br>"
        "单层 ε<sub>RMS</sub> = √[Σw<sub>j</sub>(v<sub>r,j</sub>-a<sub>j</sub>x)²/Σw<sub>j</sub>]，单位 m/s。"
        "表征径向速度观测与反演三分量风场的一致性。"));
    MetricCard *cards[] = {m_confidenceCard, m_validGateCard, m_blindRatioCard, m_cnrCard, m_residualCard};
    for (int index = 0; index < 5; ++index) grid->addWidget(cards[index], index / 3, index % 3);
    for (int column = 0; column < 3; ++column) grid->setColumnStretch(column, 1);
    layout->addLayout(grid);

    auto *explanation = section(this);
    auto *explanationLayout = new QVBoxLayout(explanation);
    explanationLayout->setContentsMargins(16, 13, 16, 14);
    auto *heading = new QLabel(QStringLiteral("判定说明"), explanation);
    heading->setStyleSheet("color:#263442; font-size:14px; font-weight:600;");
    explanationLayout->addWidget(heading);
    auto *text = new QLabel(QStringLiteral("置信度 < 50% 判为低可信；50%–80% 需关注；≥80% 表示数据质量良好。"
        "  该阈值是当前软件规则，量产前应根据实测标定。"), explanation);
    text->setWordWrap(true);
    text->setStyleSheet("color:#475467; font-size:12px; line-height:1.5;");
    explanationLayout->addWidget(text);
    layout->addWidget(explanation);
    layout->addStretch();
}

void DataQualityPage::updateQuality(double confidencePct, int validGates, int totalGates,
                                    double blindRatio, double meanCnrDb, double residualMps,
                                    const QString &source)
{
    m_confidenceCard->setValue(confidencePct);
    m_confidenceCard->setStatus(confidencePct < 50.0 ? "danger" : confidencePct < 80.0 ? "warning" : "normal");
    m_validGateCard->setValue(validGates);
    m_validGateCard->setStatus(validGates == 0 ? "danger" : "normal");
    m_validGateCard->setToolTip(QStringLiteral("<b>有效反演层数 N<sub>valid</sub></b><br>"
        "当前 N<sub>valid</sub> = %1，N<sub>total</sub> = %2。有效层需满足 N<sub>beam</sub>≥3、C<sub>i</sub>≥50% 且 ε<sub>RMS</sub>≤1.5 m/s。")
        .arg(validGates).arg(totalGates));
    m_blindRatioCard->setValue(blindRatio * 100.0);
    m_blindRatioCard->setStatus(blindRatio > 0.1 ? "danger" : blindRatio > 0.05 ? "warning" : "normal");
    m_cnrCard->setValue(meanCnrDb);
    m_cnrCard->setStatus(meanCnrDb < -22.0 ? "danger" : "normal");
    m_residualCard->setValue(residualMps);
    m_residualCard->setStatus(residualMps > 2.0 ? "danger" : residualMps > 1.0 ? "warning" : "normal");
    m_sourceValue->setText(source);
}
