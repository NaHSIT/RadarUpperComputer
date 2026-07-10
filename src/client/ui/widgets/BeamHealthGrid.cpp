#include "BeamHealthGrid.h"

#include <QMouseEvent>
#include <QPainter>

namespace {
constexpr int kMargin = 14;
constexpr int kGap = 7;
constexpr int kTitleHeight = 30;
constexpr int kCardHeight = 100;
}

BeamHealthGrid::BeamHealthGrid(QWidget *parent) : QWidget(parent)
{
    m_beamStatuses.resize(5);
    for (int index = 0; index < 5; ++index) {
        m_beamStatuses[index] = {QStringLiteral("LOS%1").arg(index + 1), index * 72.0, "normal", 0.0, 0};
    }
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

BeamHealthGrid::~BeamHealthGrid() = default;

void BeamHealthGrid::setBeamStatus(int index, const BeamStatus &status)
{
    if (index < 0 || index >= m_beamStatuses.size()) return;
    m_beamStatuses[index] = status;
    update();
}

void BeamHealthGrid::setAllBeamStatus(const QVector<BeamStatus> &statuses)
{
    m_beamStatuses = statuses;
    update();
}

void BeamHealthGrid::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#ffffff"));
    painter.setPen(QColor("#263442"));
    painter.setFont(QFont("Microsoft YaHei", 12, QFont::DemiBold));
    painter.drawText(QRect(kMargin, 0, width() - kMargin * 2, kTitleHeight), Qt::AlignVCenter | Qt::AlignLeft, QStringLiteral("波束健康状态"));

    const int availableWidth = qMax(0, width() - kMargin * 2 - kGap * (m_beamStatuses.size() - 1));
    const int cardWidth = m_beamStatuses.isEmpty() ? 0 : availableWidth / m_beamStatuses.size();
    for (int index = 0; index < m_beamStatuses.size(); ++index) {
        const QRect cardRect(kMargin + index * (cardWidth + kGap), kTitleHeight + 6, cardWidth, kCardHeight);
        drawBeamCard(painter, index, m_beamStatuses[index], cardRect);
    }
}

void BeamHealthGrid::drawBeamCard(QPainter &painter, int index, const BeamStatus &status, const QRect &rect)
{
    Q_UNUSED(index)
    QColor statusColor("#16713b");
    if (status.status == "warning") statusColor = QColor("#9a6700");
    else if (status.status != "normal") statusColor = QColor("#b42318");

    painter.fillRect(rect, QColor("#f8fafb"));
    painter.setPen(QPen(QColor("#d9dee5"), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect);
    painter.fillRect(QRect(rect.left(), rect.top(), 3, rect.height()), statusColor);

    const QRect content = rect.adjusted(10, 7, -6, -5);
    painter.setPen(QColor("#263442"));
    painter.setFont(QFont("Microsoft YaHei", 10, QFont::DemiBold));
    painter.drawText(QRect(content.left(), content.top(), content.width(), 18), Qt::AlignLeft | Qt::AlignVCenter, status.beamId);
    painter.setPen(QColor("#52606d"));
    painter.setFont(QFont("Microsoft YaHei", 9));
    painter.drawText(QRect(content.left(), content.top() + 22, content.width(), 16), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("%1°").arg(status.azimuthDeg, 0, 'f', 0));
    painter.drawText(QRect(content.left(), content.top() + 41, content.width(), 16), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("CNR %1").arg(status.cnrAvg, 0, 'f', 1));
    painter.drawText(QRect(content.left(), content.top() + 60, content.width(), 16), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("层 %1").arg(status.validGates));
}

void BeamHealthGrid::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        const int availableWidth = qMax(0, width() - kMargin * 2 - kGap * (m_beamStatuses.size() - 1));
        const int cardWidth = m_beamStatuses.isEmpty() ? 0 : availableWidth / m_beamStatuses.size();
        const int relativeX = event->pos().x() - kMargin;
        const int relativeY = event->pos().y() - kTitleHeight - 6;
        if (cardWidth > 0 && relativeX >= 0 && relativeY >= 0 && relativeY <= kCardHeight) {
            const int index = relativeX / (cardWidth + kGap);
            if (index >= 0 && index < m_beamStatuses.size() && relativeX - index * (cardWidth + kGap) <= cardWidth) {
                emit beamClicked(index, m_beamStatuses[index].beamId);
            }
        }
    }
    QWidget::mousePressEvent(event);
}
