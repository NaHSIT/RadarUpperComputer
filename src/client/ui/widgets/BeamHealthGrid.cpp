#include "BeamHealthGrid.h"

#include <QMouseEvent>
#include <QPainter>

BeamHealthGrid::BeamHealthGrid(QWidget *parent) : QWidget(parent)
{
    m_beamStatuses.resize(5);
    for (int i = 0; i < 5; ++i) {
        m_beamStatuses[i] = {QStringLiteral("LOS%1").arg(i + 1), i * 72.0, "normal", 0.0, 0};
    }
    setMinimumSize(400, 150); setMaximumHeight(170); setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}
BeamHealthGrid::~BeamHealthGrid() = default;
void BeamHealthGrid::setBeamStatus(int index, const BeamStatus &status) { if (index >= 0 && index < m_beamStatuses.size()) { m_beamStatuses[index] = status; update(); } }
void BeamHealthGrid::setAllBeamStatus(const QVector<BeamStatus> &statuses) { m_beamStatuses = statuses; update(); }
void BeamHealthGrid::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.fillRect(rect(), QColor("#ffffff"));
    p.setPen(QColor("#263442")); p.setFont(QFont("Microsoft YaHei", 12, QFont::DemiBold)); p.drawText(14, 23, QStringLiteral("波束健康状态"));
    const int left = 14, top = 36, gap = 7, total = width() - left * 2; const int cardWidth = qMax(58, (total - gap * 4) / 5); const int cardHeight = 100;
    for (int i = 0; i < m_beamStatuses.size(); ++i) drawBeamCard(p, i, m_beamStatuses[i], QRect(left + i * (cardWidth + gap), top, cardWidth, cardHeight));
}
void BeamHealthGrid::drawBeamCard(QPainter &p, int index, const BeamStatus &status, const QRect &rect)
{
    Q_UNUSED(index)
    QColor color("#16713b"); if (status.status == "warning") color = QColor("#9a6700"); else if (status.status != "normal") color = QColor("#b42318");
    p.fillRect(rect, QColor("#f8fafb")); p.setPen(QPen(QColor("#d9dee5"), 1)); p.setBrush(Qt::NoBrush); p.drawRect(rect);
    p.fillRect(QRect(rect.left(), rect.top(), 3, rect.height()), color);
    p.setPen(QColor("#263442")); p.setFont(QFont("Microsoft YaHei", 10, QFont::DemiBold)); p.drawText(rect.adjusted(8, 10, -5, 0), Qt::AlignLeft, status.beamId);
    p.setPen(QColor("#667085")); p.setFont(QFont("Microsoft YaHei", 9));
    p.drawText(rect.adjusted(8, 34, -5, 0), Qt::AlignLeft, QStringLiteral("方位：%1°").arg(status.azimuthDeg, 0, 'f', 0));
    p.drawText(rect.adjusted(8, 53, -5, 0), Qt::AlignLeft, QStringLiteral("CNR：%1 dB").arg(status.cnrAvg, 0, 'f', 1));
    p.drawText(rect.adjusted(8, 72, -5, 0), Qt::AlignLeft, QStringLiteral("有效层：%1").arg(status.validGates));
}
void BeamHealthGrid::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) { const int left=14,gap=7,total=width()-left*2,cardWidth=qMax(58,(total-gap*4)/5); const int relative=event->pos().x()-left; const int index=relative/(cardWidth+gap); if (relative>=0 && index>=0 && index<m_beamStatuses.size() && relative-index*(cardWidth+gap)<=cardWidth) emit beamClicked(index,m_beamStatuses[index].beamId); }
    QWidget::mousePressEvent(event);
}
