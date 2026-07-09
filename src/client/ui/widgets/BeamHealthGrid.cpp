#include "BeamHealthGrid.h"
#include <QPainter>
#include <QMouseEvent>

BeamHealthGrid::BeamHealthGrid(QWidget *parent)
    : QWidget(parent)
{
    m_beamStatuses.resize(5);
    for (int i = 0; i < 5; ++i) {
        m_beamStatuses[i].beamId = QString("LOS%1").arg(i + 1);
        m_beamStatuses[i].azimuthDeg = i * 72.0;
        m_beamStatuses[i].status = "normal";
        m_beamStatuses[i].cnrAvg = 0;
        m_beamStatuses[i].validGates = 0;
    }

    setMinimumSize(400, 120);
    setMaximumHeight(150);
}

BeamHealthGrid::~BeamHealthGrid()
{
}

void BeamHealthGrid::setBeamStatus(int index, const BeamStatus &status)
{
    if (index >= 0 && index < m_beamStatuses.size()) {
        m_beamStatuses[index] = status;
        update();
    }
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

    // 绘制背景
    painter.fillRect(rect(), QColor(255, 255, 255));

    // 绘制波束卡片
    int cardWidth = (width() - 40) / 5;
    int cardHeight = 80;
    int startY = 35;

    for (int i = 0; i < m_beamStatuses.size(); ++i) {
        int x = 10 + i * (cardWidth + 5);
        QRect cardRect(x, startY, cardWidth, cardHeight);
        drawBeamCard(painter, i, m_beamStatuses[i]);
    }
}

void BeamHealthGrid::drawBeamCard(QPainter &painter, int index, const BeamStatus &status)
{
    Q_UNUSED(painter)
    Q_UNUSED(index)
    Q_UNUSED(status)
    // TODO: 完整实现
}

void BeamHealthGrid::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int cardWidth = (width() - 40) / 5;
        int index = (event->pos().x() - 10) / (cardWidth + 5);

        if (index >= 0 && index < m_beamStatuses.size()) {
            emit beamClicked(index, m_beamStatuses[index].beamId);
        }
    }
    QWidget::mousePressEvent(event);
}
