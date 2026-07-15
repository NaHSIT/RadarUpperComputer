#include "BeamPage.h"

#include <QFrame>
#include <QDateTime>
#include <QHeaderView>
#include <QLabel>
#include <QSplitter>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QtMath>

#include "domain/BeamState.h"

namespace {
QFrame *createSection(QWidget *parent)
{
    auto *section = new QFrame(parent);
    section->setObjectName("beamWorkspaceSection");
    section->setStyleSheet("QFrame#beamWorkspaceSection { background:#ffffff; border:1px solid #d9dee5; border-radius:3px; }");
    return section;
}

QLabel *createSectionTitle(const QString &text, QWidget *parent)
{
    auto *title = new QLabel(text, parent);
    title->setStyleSheet("color:#243447; font-size:14px; font-weight:600;");
    return title;
}

void applyTableStyle(QTableWidget *table)
{
    table->verticalHeader()->hide();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setStyleSheet(
        "QTableWidget { border:1px solid #d9dee5; color:#344054; font-size:12px; }"
        "QTableWidget::item { padding:8px 10px; border-bottom:1px solid #edf0f2; }"
        "QTableWidget::item:alternate { background:#f8fafb; }"
        "QTableWidget::item:selected { background:#eaf1f6; color:#182230; }"
        "QHeaderView::section { background:#f2f5f7; color:#52606d; border:0; border-bottom:1px solid #d9dee5; padding:8px 10px; font-size:12px; font-weight:600; }"
    );
}
}

BeamPage::BeamPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
    m_statusLine = findChild<QLabel *>("beamConnectionStatus");
    m_beamTable = findChild<QTableWidget *>("beamStatusTable");
    setConnectionState(false);
}

BeamPage::~BeamPage() = default;

void BeamPage::setConnectionState(bool connected)
{
    m_connected = connected;
    if (!m_statusLine || !m_beamTable) return;

    m_statusLine->setText(connected
        ? QStringLiteral("当前状态：已连接雷达，正在接收仿真波束数据。")
        : QStringLiteral("当前状态：未连接雷达，实时波束数据将在设备连接后显示。"));
    m_statusLine->setStyleSheet(connected
        ? "color:#16713b; background:#edf7f0; border:1px solid #b7dfc1; padding:7px 10px; font-size:12px;"
        : "color:#52606d; background:#edf3f7; border:1px solid #d7e3ec; padding:7px 10px; font-size:12px;");
    for (int row = 0; row < m_beamTable->rowCount(); ++row) {
        m_beamTable->item(row, 7)->setText(connected ? QStringLiteral("等待数据") : QStringLiteral("未连接"));
        m_beamTable->item(row, 7)->setForeground(connected ? QColor("#9a6700") : QColor("#667085"));
    }
}

void BeamPage::updateSimulationData(double cnrDb, int validGates)
{
    if (!m_connected || !m_beamTable) return;

    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    for (int row = 0; row < m_beamTable->rowCount(); ++row) {
        m_beamTable->item(row, 3)->setText(QString::number(cnrDb, 'f', 1));
        m_beamTable->item(row, 4)->setText(QString::number(validGates));
        m_beamTable->item(row, 5)->setText(QStringLiteral("—"));
        m_beamTable->item(row, 6)->setText(timestamp);
        m_beamTable->item(row, 7)->setText(QStringLiteral("兼容数据"));
        m_beamTable->item(row, 7)->setForeground(QColor("#9a6700"));
    }
}

void BeamPage::updateBeamData(const QVector<BeamState *> &beams)
{
    if (!m_connected || !m_beamTable) return;
    for (const BeamState *beam : beams) {
        if (!beam) continue;
        const int row = static_cast<int>(beam->beamId());
        if (row < 0 || row >= m_beamTable->rowCount()) continue;
        const QVector<double> cnr = beam->cnrDbByGate();
        const QVector<double> radialVelocity = beam->rwsByGate();
        double cnrSum = 0.0;
        int cnrCount = 0;
        int valid = 0;
        int complete = 0;
        const int gateCount = qMax(cnr.size(), radialVelocity.size());
        for (int gate = 0; gate < gateCount; ++gate) {
            if (gate < cnr.size() && qIsFinite(cnr.at(gate))) {
                cnrSum += cnr.at(gate);
                ++cnrCount;
            }
            const bool radialValid = gate < radialVelocity.size() && qIsFinite(radialVelocity.at(gate));
            if (radialValid) ++complete;
            if (radialValid && gate < cnr.size() && qIsFinite(cnr.at(gate))
                && cnr.at(gate) >= -22.0) ++valid;
        }
        const QString name = row == 0 ? QStringLiteral("法向波束") : QStringLiteral("斜波束 %1").arg(row);
        const QString geometry = row == 0 ? QStringLiteral("法向 / 90°")
            : QStringLiteral("%1° / %2°").arg(beam->azimuthDeg(), 0, 'f', 0)
                                           .arg(beam->elevationDeg(), 0, 'f', 0);
        m_beamTable->item(row, 0)->setText(name);
        m_beamTable->item(row, 1)->setText(geometry);
        m_beamTable->item(row, 2)->setText(QStringLiteral("%1 GHz")
            .arg(beam->carrierFrequencyHz() / 1.0e9, 0, 'f', 1));
        m_beamTable->item(row, 3)->setText(cnrCount > 0 ? QString::number(cnrSum / cnrCount, 'f', 1)
                                                        : QStringLiteral("—"));
        m_beamTable->item(row, 4)->setText(QString::number(valid));
        const double completeness = gateCount > 0 ? 100.0 * complete / gateCount : 0.0;
        m_beamTable->item(row, 5)->setText(gateCount > 0
            ? QStringLiteral("%1%").arg(completeness, 0, 'f', 0) : QStringLiteral("—"));
        m_beamTable->item(row, 6)->setText(beam->lastUpdateTime().toLocalTime().toString("HH:mm:ss"));
        const double meanCnr = cnrCount > 0 ? cnrSum / cnrCount : -100.0;
        const bool normal = meanCnr >= -22.0 && completeness >= 95.0;
        m_beamTable->item(row, 7)->setText(normal ? QStringLiteral("正常")
            : (meanCnr < -22.0 ? QStringLiteral("CNR 低") : QStringLiteral("数据不完整")));
        m_beamTable->item(row, 7)->setForeground(normal ? QColor("#16713b") : QColor("#9a6700"));
    }
}

void BeamPage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 18);
    mainLayout->setSpacing(12);

    auto *title = new QLabel(QStringLiteral("波束监视"), this);
    title->setStyleSheet("color:#182230; font-size:22px; font-weight:600;");
    auto *subtitle = new QLabel(QStringLiteral("按 LOS 波束查看方位、信噪比、有效测量层和数据完整性。"), this);
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("color:#667085; font-size:13px;");
    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    auto *statusLine = new QLabel(QStringLiteral("当前状态：未连接雷达，实时波束数据将在设备连接后显示。"), this);
    statusLine->setObjectName("beamConnectionStatus");
    statusLine->setStyleSheet("color:#52606d; background:#edf3f7; border:1px solid #d7e3ec; padding:7px 10px; font-size:12px;");
    mainLayout->addWidget(statusLine);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);

    auto *beamSection = createSection(splitter);
    auto *beamLayout = new QVBoxLayout(beamSection);
    beamLayout->setContentsMargins(14, 12, 14, 14);
    beamLayout->setSpacing(10);
    beamLayout->addWidget(createSectionTitle(QStringLiteral("波束状态"), beamSection));
    auto *beamTable = new QTableWidget(5, 8, beamSection);
    beamTable->setHorizontalHeaderLabels({
        QStringLiteral("波束"), QStringLiteral("方位/仰角"), QStringLiteral("载频"), QStringLiteral("平均 CNR"),
        QStringLiteral("有效层"), QStringLiteral("完整率"), QStringLiteral("更新时间"), QStringLiteral("状态")
    });
    const QStringList headerHelp = {
        QStringLiteral("固定五波束的视线编号。"),
        QStringLiteral("方位角从真北顺时针计算；仰角 90° 为垂直向上。"),
        QStringLiteral("射频前端工作载波频率，应来自设备参数，当前初始值为界面预置。"),
        QStringLiteral("该波束所有距离门的平均载噪比。"),
        QStringLiteral("该波束中径向速度有效且 CNR 不低于门限的高度层数。"),
        QStringLiteral("该波束有限径向速度数据门数占总距离门数的比例。"),
        QStringLiteral("最后一次收到该波束 0x8105 数据的时间。"),
        QStringLiteral("根据平均 CNR、数据完整率和有效层数得出的具体波束健康状态。")
    };
    for (int column = 0; column < headerHelp.size(); ++column) {
        beamTable->horizontalHeaderItem(column)->setToolTip(headerHelp.at(column));
    }
    beamTable->setObjectName("beamStatusTable");
    applyTableStyle(beamTable);
    beamTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    const double azimuths[] = {0.0, 45.0, 135.0, 225.0, 315.0};
    for (int row = 0; row < beamTable->rowCount(); ++row) {
        const QStringList values = {
            row == 0 ? QStringLiteral("法向波束") : QStringLiteral("斜波束 %1").arg(row),
            row == 0 ? QStringLiteral("法向 / 90°")
                     : QStringLiteral("%1° / 75°").arg(azimuths[row], 0, 'f', 0),
            QStringLiteral("%1 GHz").arg(23.8 + row * 0.1, 0, 'f', 1), QStringLiteral("--"),
            QStringLiteral("--"), QStringLiteral("--"), QStringLiteral("--"), QStringLiteral("未连接")
        };
        for (int column = 0; column < values.size(); ++column) {
            beamTable->setItem(row, column, new QTableWidgetItem(values[column]));
        }
        beamTable->item(row, 7)->setForeground(QColor("#667085"));
    }
    beamLayout->addWidget(beamTable, 1);
    splitter->addWidget(beamSection);

    auto *qualitySection = createSection(splitter);
    auto *qualityLayout = new QVBoxLayout(qualitySection);
    qualityLayout->setContentsMargins(14, 12, 14, 14);
    qualityLayout->setSpacing(10);
    qualityLayout->addWidget(createSectionTitle(QStringLiteral("质量诊断"), qualitySection));
    auto *qualityHint = new QLabel(QStringLiteral("该区域用于汇总波束遮挡、低信噪比、相位异常和有效层不足等诊断结论。\n\n连接设备后，选择左侧波束可查看对应诊断详情。"), qualitySection);
    qualityHint->setWordWrap(true);
    qualityHint->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    qualityHint->setStyleSheet("color:#667085; background:#f8fafb; border:1px solid #e4e9ed; padding:12px; font-size:12px; line-height:1.5;");
    qualityLayout->addWidget(qualityHint);
    qualityLayout->addStretch();
    splitter->addWidget(qualitySection);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter, 1);
}
