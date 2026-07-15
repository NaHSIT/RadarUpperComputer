#include "WindTrendChart.h"

#include <QDateTime>
#include <QPainter>
#include <QtMath>
#include <cmath>

WindTrendChart::WindTrendChart(QWidget *parent)
    : QWidget(parent)
    , m_timeWindow(QStringLiteral("1min"))
    , m_refreshTimer(new QTimer(this))
    , m_valueAnimation(new QVariantAnimation(this))
    , m_seriesType(SeriesType::WindSpeed)
    , m_windowSeconds(60)
    , m_lastAnimationPaintMs(0)
{
    setupUI();
    setMinimumSize(320, 230);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_valueAnimation->setDuration(260);
    m_valueAnimation->setEasingCurve(QEasingCurve::InOutSine);
    connect(m_valueAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &animated) {
        if (m_data.isEmpty()) return;
        double value = animated.toDouble();
        if (m_seriesType == SeriesType::WindDirection) value = std::fmod(value + 360.0, 360.0);
        m_data.last().setY(value);
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        if (nowMs - m_lastAnimationPaintMs < 40
            && m_valueAnimation->currentTime() < m_valueAnimation->duration()) return;
        m_lastAnimationPaintMs = nowMs;
        update();
    });
    connect(m_valueAnimation, &QVariantAnimation::finished, this, [this] { update(); });
}

WindTrendChart::~WindTrendChart() = default;

void WindTrendChart::setTimeWindow(const QString &window)
{
    if (m_timeWindow == window) return;
    m_timeWindow = window;
    emit timeWindowChanged(window);
}

void WindTrendChart::addDataPoint(double value)
{
    if (!qIsFinite(value)) return;
    if (m_seriesType == SeriesType::WindDirection) value = std::fmod(value + 360.0, 360.0);
    const qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    if (m_data.isEmpty()) {
        m_data.append(QPointF(timestamp, value));
        update();
        return;
    }

    const double start = m_data.last().y();
    double end = value;
    if (m_seriesType == SeriesType::WindDirection) {
        const double delta = std::fmod(value - start + 540.0, 360.0) - 180.0;
        end = start + delta;
    }
    m_data.append(QPointF(timestamp, start));
    const qint64 cutoff = timestamp - static_cast<qint64>(m_windowSeconds) * 1000;
    while (!m_data.isEmpty() && m_data.first().x() < cutoff) m_data.removeFirst();
    while (m_data.size() > 1200) m_data.removeFirst();
    const bool animateLatestValue = m_windowSeconds <= 300 && m_data.size() <= 300;
    if (!isVisible() || !animateLatestValue) {
        m_valueAnimation->stop();
        m_data.last().setY(value);
        if (isVisible()) update();
        return;
    }
    m_valueAnimation->stop();
    m_valueAnimation->setStartValue(start);
    m_valueAnimation->setEndValue(end);
    m_valueAnimation->start();
}

void WindTrendChart::setWindowSeconds(int seconds)
{
    m_windowSeconds = qBound(60, seconds, 24 * 60 * 60);
    if (m_windowSeconds > 300) m_valueAnimation->stop();
    update();
}

void WindTrendChart::setData(const QVector<QPointF> &data)
{
    m_valueAnimation->stop();
    constexpr int kMaximumDisplayPoints = 480;
    if (data.size() <= kMaximumDisplayPoints) {
        m_data = data;
    } else {
        m_data.clear();
        m_data.reserve(kMaximumDisplayPoints);
        const double step = static_cast<double>(data.size() - 1) / (kMaximumDisplayPoints - 1);
        for (int index = 0; index < kMaximumDisplayPoints; ++index) {
            m_data.append(data.at(qRound(index * step)));
        }
    }
    if (m_seriesType == SeriesType::WindDirection) {
        for (QPointF &point : m_data) {
            point.setY(std::fmod(point.y() + 360.0, 360.0));
        }
    }
    update();
}

void WindTrendChart::clear()
{
    m_valueAnimation->stop();
    m_data.clear();
    update();
}

void WindTrendChart::setSeriesType(SeriesType type)
{
    m_seriesType = type;
    clear();
}

void WindTrendChart::setupUI()
{
    setStyleSheet("background:#ffffff; border:0;");
}

void WindTrendChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#ffffff"));
    const QRectF plot = QRectF(rect()).adjusted(58, 28, -18, -50);

    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 10));
    if (m_data.isEmpty()) {
        painter.setPen(QColor("#98a2b3"));
        painter.drawText(plot, Qt::AlignCenter, QStringLiteral("等待有效时序数据"));
        return;
    }

    double minimum = 0.0;
    double maximum = 360.0;
    if (m_seriesType == SeriesType::WindSpeed) {
        maximum = 0.0;
        for (const QPointF &point : m_data) maximum = qMax(maximum, point.y());
        maximum = qMax(5.0, qCeil(maximum / 5.0) * 5.0);
    }
    const double range = maximum - minimum;

    for (int tick = 0; tick <= 4; ++tick) {
        const double fraction = tick / 4.0;
        const double y = plot.bottom() - fraction * plot.height();
        painter.setPen(QPen(QColor("#e4e9ef"), 1));
        painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        painter.setPen(QColor("#667085"));
        const double value = minimum + fraction * range;
        const QString label = m_seriesType == SeriesType::WindDirection
            ? QStringLiteral("%1°").arg(value, 0, 'f', 0)
            : QString::number(value, 'f', 1);
        painter.drawText(QRectF(0, y - 9, plot.left() - 8, 18), Qt::AlignRight | Qt::AlignVCenter, label);
    }

    const qint64 newestSample = static_cast<qint64>(m_data.last().x());
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 lastTime = qMax(newestSample, now);
    const qint64 timeSpan = static_cast<qint64>(m_windowSeconds) * 1000;
    const qint64 firstTime = lastTime - timeSpan;
    const QString timeFormat = m_windowSeconds <= 3600
        ? QStringLiteral("HH:mm:ss")
        : (m_windowSeconds <= 21600 ? QStringLiteral("HH:mm") : QStringLiteral("MM-dd HH:mm"));
    for (int tick = 0; tick <= 4; ++tick) {
        const double fraction = tick / 4.0;
        const double x = plot.left() + fraction * plot.width();
        painter.setPen(QPen(QColor("#eef1f5"), 1));
        painter.drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom()));
        painter.setPen(QColor("#667085"));
        const qint64 timestamp = firstTime + static_cast<qint64>(fraction * timeSpan);
        painter.drawText(QRectF(x - 58, plot.bottom() + 8, 116, 30), Qt::AlignHCenter | Qt::AlignTop,
                         QDateTime::fromMSecsSinceEpoch(timestamp).toString(timeFormat));
    }

    QPainterPath path;
    QPainterPath fillPath;
    bool pathStarted = false;
    QPointF firstPoint;
    QPointF lastPoint;
    double previousValue = 0.0;
    for (int index = 0; index < m_data.size(); ++index) {
        const QPointF &sample = m_data.at(index);
        if (sample.x() < firstTime || sample.x() > lastTime) continue;
        const double x = plot.left() + (sample.x() - firstTime) * plot.width() / timeSpan;
        const double y = plot.bottom() - (sample.y() - minimum) * plot.height() / range;
        const QPointF point(x, y);
        const bool circularBreak = m_seriesType == SeriesType::WindDirection && index > 0
            && qAbs(sample.y() - previousValue) > 180.0;
        if (!pathStarted || circularBreak) {
            path.moveTo(point);
            pathStarted = true;
            if (index == 0) firstPoint = point;
        } else {
            path.lineTo(point);
        }
        lastPoint = point;
        previousValue = sample.y();
    }

    if (m_seriesType == SeriesType::WindSpeed && m_data.size() > 1) {
        fillPath = path;
        fillPath.lineTo(lastPoint.x(), plot.bottom());
        fillPath.lineTo(firstPoint.x(), plot.bottom());
        fillPath.closeSubpath();
        painter.fillPath(fillPath, QColor(34, 116, 165, 18));
    }
    painter.setPen(QPen(QColor("#176b87"), 2.0));
    painter.drawPath(path);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#176b87"));
    painter.drawEllipse(lastPoint, 3.0, 3.0);

    painter.setPen(QColor("#344054"));
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 11, QFont::DemiBold));
    const QString currentValue = m_seriesType == SeriesType::WindDirection
        ? QStringLiteral("当前 %1°").arg(m_data.last().y(), 0, 'f', 1)
        : QStringLiteral("当前 %1 m/s").arg(m_data.last().y(), 0, 'f', 1);
    painter.drawText(QRectF(plot.left(), 2, plot.width(), 20), Qt::AlignRight, currentValue);
}

void WindTrendChart::updateChart()
{
    update();
}
