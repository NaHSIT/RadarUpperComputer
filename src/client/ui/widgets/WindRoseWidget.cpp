#include "WindRoseWidget.h"
#include <QPainter>

WindRoseWidget::WindRoseWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentDirection(0)
{
    setupUI();
    setMinimumSize(200, 200);
    setMaximumSize(300, 300);
}

WindRoseWidget::~WindRoseWidget()
{
}

void WindRoseWidget::setWindDirection(double direction)
{
    m_currentDirection = direction;
    update();
}

void WindRoseWidget::setDirectionHistory(const QVector<double> &directions)
{
    m_directionHistory = directions;
    update();
}

void WindRoseWidget::setupUI()
{
    setStyleSheet("background: white; border-radius: 8px;");
}

void WindRoseWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    painter.fillRect(rect(), QColor(255, 255, 255));

    // 绘制罗盘
    // TODO: 完整实现
}
