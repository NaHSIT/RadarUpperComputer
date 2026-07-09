#include "WindTrendChart.h"
#include <QPainter>

WindTrendChart::WindTrendChart(QWidget *parent)
    : QWidget(parent)
    , m_timeWindow("1min")
    , m_refreshTimer(new QTimer(this))
{
    setupUI();
    setMinimumSize(300, 200);
}

WindTrendChart::~WindTrendChart()
{
}

void WindTrendChart::setTimeWindow(const QString &window)
{
    if (m_timeWindow != window) {
        m_timeWindow = window;
        emit timeWindowChanged(window);
    }
}

void WindTrendChart::addDataPoint(double value)
{
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    m_data.append(QPointF(timestamp, value));

    // 保留最近 1000 个点
    while (m_data.size() > 1000) {
        m_data.removeFirst();
    }

    update();
}

void WindTrendChart::clear()
{
    m_data.clear();
    update();
}

void WindTrendChart::setupUI()
{
    setStyleSheet("background: white; border-radius: 8px;");
}

void WindTrendChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    painter.fillRect(rect(), QColor(255, 255, 255));

    // 绘制网格和数据
    // TODO: 完整实现
}

void WindTrendChart::updateChart()
{
    update();
}
