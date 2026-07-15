#include "WindVectorFieldWidget.h"

#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QResizeEvent>
#include <QtMath>
#include <cmath>
#include <utility>

WindVectorFieldWidget::WindVectorFieldWidget(QWidget *parent)
    : QWidget(parent)
    , m_columns(0)
    , m_rows(0)
    , m_heightAglM(0.0)
    , m_emptyTitle(QStringLiteral("二维网格风场不可用"))
    , m_emptyDetail(QStringLiteral("当前数据产品不包含水平空间网格"))
    , m_speedScaleMaximumMps(60.0)
    , m_targetTimestampMs(0)
    , m_historyWindowSeconds(60)
    , m_profileCacheDirty(true)
    , m_transitionAnimation(new QVariantAnimation(this))
    , m_historyAppendCount(0)
{
    setMinimumHeight(190);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_transitionAnimation->setDuration(480);
    m_transitionAnimation->setStartValue(0.0);
    m_transitionAnimation->setEndValue(1.0);
    m_transitionAnimation->setEasingCurve(QEasingCurve::InOutSine);
    connect(m_transitionAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
        if (m_renderThrottle.isValid() && m_renderThrottle.elapsed() < 66
            && value.toReal() < 0.999) return;
        m_renderThrottle.restart();
        interpolateProfile(value.toReal());
        update();
    });
    connect(m_transitionAnimation, &QVariantAnimation::finished, this, [this] {
        m_profileLevels = m_profileTargetLevels;
        m_profileCacheDirty = true;
        update();
    });
}

void WindVectorFieldWidget::setSpeedScaleMaximum(double maximumMps)
{
    if (maximumMps <= 0.0) return;
    m_speedScaleMaximumMps = maximumMps;
    m_profileCacheDirty = true;
    update();
}

void WindVectorFieldWidget::setWindGrid(int columns, int rows, const QVector<WindCell> &cells,
                                        double heightAglM, const QString &source)
{
    if (columns <= 0 || rows <= 0 || cells.size() != columns * rows) return;
    m_columns = columns;
    m_rows = rows;
    m_cells = cells;
    m_transitionAnimation->stop();
    m_profileLevels.clear();
    m_profileStartLevels.clear();
    m_profileTargetLevels.clear();
    m_heightAglM = heightAglM;
    m_source = source;
    m_profileCacheDirty = true;
    setMinimumHeight(390);
    update();
}

void WindVectorFieldWidget::setWindProfile(const QVector<ProfileLevel> &levels, const QString &source)
{
    m_columns = 0;
    m_rows = 0;
    m_cells.clear();
    m_source = source;
    setMinimumHeight(470);

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    if (!m_profileTargetLevels.isEmpty() && m_targetTimestampMs > 0) {
        m_profileHistory.append({m_targetTimestampMs, m_profileTargetLevels});
        const qint64 oldestUsefulMs = nowMs - 24LL * 60 * 60 * 1000;
        while (!m_profileHistory.isEmpty()
               && m_profileHistory.first().timestampMs < oldestUsefulMs) {
            m_profileHistory.removeFirst();
        }
        if (++m_historyAppendCount >= 30) {
            compactProfileHistory(nowMs);
            m_historyAppendCount = 0;
        }
    }
    m_targetTimestampMs = nowMs;

    if (!isVisible() || m_profileLevels.size() != levels.size() || levels.isEmpty()) {
        m_transitionAnimation->stop();
        m_profileLevels = levels;
        m_profileStartLevels = levels;
        m_profileTargetLevels = levels;
        m_profileCacheDirty = true;
        update();
        return;
    }

    m_transitionAnimation->stop();
    m_profileStartLevels = m_profileLevels;
    m_profileTargetLevels = levels;
    m_renderThrottle.restart();
    m_transitionAnimation->start();
}

void WindVectorFieldWidget::setHistoryWindowSeconds(int seconds)
{
    m_historyWindowSeconds = qBound(60, seconds, 24 * 60 * 60);
    m_profileCacheDirty = true;
    update();
}

void WindVectorFieldWidget::compactProfileHistory(qint64 nowMs)
{
    QVector<ProfileSnapshot> compacted;
    compacted.reserve(qMin(m_profileHistory.size(), 3000));
    qint64 lastKeptMs = -1;
    for (const ProfileSnapshot &snapshot : std::as_const(m_profileHistory)) {
        const qint64 ageMs = qMax<qint64>(0, nowMs - snapshot.timestampMs);
        const qint64 intervalMs = ageMs <= 10 * 60 * 1000
            ? 1000
            : (ageMs <= 2 * 60 * 60 * 1000 ? 10000 : 60000);
        if (lastKeptMs < 0 || snapshot.timestampMs - lastKeptMs >= intervalMs) {
            compacted.append(snapshot);
            lastKeptMs = snapshot.timestampMs;
        }
    }
    m_profileHistory = std::move(compacted);
}

void WindVectorFieldWidget::setUniformWind(double speedMps, double windFromDirectionDeg,
                                           double heightAglM, const QString &source)
{
    constexpr int columns = 24;
    constexpr int rows = 16;
    const double directionRad = qDegreesToRadians(windFromDirectionDeg);
    const double eastward = -speedMps * qSin(directionRad);
    const double northward = -speedMps * qCos(directionRad);
    QVector<WindCell> cells;
    cells.reserve(columns * rows);
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const double eastKm = -2.0 + 4.0 * column / (columns - 1);
            const double northKm = -1.5 + 3.0 * row / (rows - 1);
            cells.append({eastKm, northKm, eastward, northward, 100.0});
        }
    }
    setWindGrid(columns, rows, cells, heightAglM, source);
}

void WindVectorFieldWidget::clearField(const QString &title, const QString &detail)
{
    m_columns = 0;
    m_rows = 0;
    m_cells.clear();
    m_transitionAnimation->stop();
    m_profileLevels.clear();
    m_profileStartLevels.clear();
    m_profileTargetLevels.clear();
    m_profileHistory.clear();
    m_targetTimestampMs = 0;
    m_profileCacheDirty = true;
    setMinimumHeight(190);
    if (!title.isEmpty()) m_emptyTitle = title;
    if (!detail.isEmpty()) m_emptyDetail = detail;
    update();
}

void WindVectorFieldWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#ffffff"));

    if (!m_profileLevels.isEmpty()) {
        paintWindProfile(painter);
        m_profileCacheDirty = false;
        return;
    }

    const QRectF plot = QRectF(rect()).adjusted(64, 42, -100, -54);
    painter.setPen(QColor("#172b3d"));
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 10, QFont::DemiBold));
    const QString title = m_cells.isEmpty() ? QStringLiteral("二维网格风场")
        : QStringLiteral("二维网格风场  ·  高度 %1 m AGL").arg(m_heightAglM, 0, 'f', 0);
    painter.drawText(QRectF(plot.left(), 8, plot.width(), 24), Qt::AlignLeft, title);

    if (m_cells.isEmpty()) {
        painter.fillRect(plot, QColor("#f8fafc"));
        painter.setPen(QPen(QColor("#d0d5dd"), 1, Qt::DashLine));
        painter.drawRect(plot);
        painter.setPen(QColor("#344054"));
        painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 11, QFont::DemiBold));
        painter.drawText(QRectF(plot.left(), plot.center().y() - 30, plot.width(), 24), Qt::AlignCenter, m_emptyTitle);
        painter.setPen(QColor("#667085"));
        painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9));
        painter.drawText(QRectF(plot.left() + 40, plot.center().y(), plot.width() - 80, 44),
                         Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, m_emptyDetail);
        return;
    }

    const double maximumSpeed = m_speedScaleMaximumMps;
    const double cellWidth = plot.width() / m_columns;
    const double cellHeight = plot.height() / m_rows;

    painter.setPen(Qt::NoPen);
    for (int row = 0; row < m_rows; ++row) {
        for (int column = 0; column < m_columns; ++column) {
            const WindCell &cell = m_cells.at(row * m_columns + column);
            const double speed = qSqrt(cell.eastwardMps * cell.eastwardMps
                                       + cell.northwardMps * cell.northwardMps);
            const QRectF cellRect(plot.left() + column * cellWidth,
                                  plot.bottom() - (row + 1) * cellHeight,
                                  cellWidth + 0.8, cellHeight + 0.8);
            if (cell.confidencePct >= 50.0) {
                painter.fillRect(cellRect, speedColor(speed, maximumSpeed));
            } else {
                painter.fillRect(cellRect, QColor("#e4e7ec"));
                painter.setPen(QPen(QColor(152, 162, 179, 120), 1));
                painter.drawLine(cellRect.topLeft(), cellRect.bottomRight());
                painter.setPen(Qt::NoPen);
            }
        }
    }

    const int arrowStep = m_columns >= 20 ? 2 : 1;
    for (int row = 0; row < m_rows; row += arrowStep) {
        for (int column = 0; column < m_columns; column += arrowStep) {
            const WindCell &cell = m_cells.at(row * m_columns + column);
            if (cell.confidencePct < 50.0) continue;
            const QPointF center(plot.left() + (column + 0.5) * cellWidth,
                                 plot.bottom() - (row + 0.5) * cellHeight);
            drawArrow(painter, center, cell.eastwardMps, cell.northwardMps,
                      qMin(cellWidth, cellHeight) * arrowStep * 0.72);
        }
    }

    painter.setPen(QPen(QColor("#475467"), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(plot);
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 8));
    for (int tick = 0; tick <= 4; ++tick) {
        const double fraction = tick / 4.0;
        const double x = plot.left() + fraction * plot.width();
        const double y = plot.bottom() - fraction * plot.height();
        painter.drawLine(QPointF(x, plot.bottom()), QPointF(x, plot.bottom() + 5));
        painter.drawText(QRectF(x - 28, plot.bottom() + 8, 56, 18), Qt::AlignCenter,
                         QStringLiteral("%1").arg(-2.0 + fraction * 4.0, 0, 'f', 1));
        painter.drawLine(QPointF(plot.left() - 5, y), QPointF(plot.left(), y));
        painter.drawText(QRectF(2, y - 9, 56, 18), Qt::AlignRight | Qt::AlignVCenter,
                         QStringLiteral("%1").arg(-1.5 + fraction * 3.0, 0, 'f', 1));
    }
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9));
    painter.drawText(QRectF(plot.left(), plot.bottom() + 30, plot.width(), 20), Qt::AlignCenter,
                     QStringLiteral("向东距离 (km)"));
    painter.save();
    painter.translate(15, plot.center().y());
    painter.rotate(-90);
    painter.drawText(QRectF(-plot.height() / 2, 0, plot.height(), 20), Qt::AlignCenter,
                     QStringLiteral("向北距离 (km)"));
    painter.restore();

    const QRectF colorBar(plot.right() + 28, plot.top(), 18, plot.height());
    for (int pixel = 0; pixel < static_cast<int>(colorBar.height()); ++pixel) {
        const double fraction = 1.0 - pixel / colorBar.height();
        painter.fillRect(QRectF(colorBar.left(), colorBar.top() + pixel, colorBar.width(), 1.2),
                         speedColor(fraction * maximumSpeed, maximumSpeed));
    }
    painter.setPen(QColor("#475467"));
    painter.drawRect(colorBar);
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 8));
    for (int tick = 0; tick <= 5; ++tick) {
        const double fraction = tick / 5.0;
        const double y = colorBar.bottom() - fraction * colorBar.height();
        painter.drawText(QRectF(colorBar.right() + 5, y - 9, 40, 18), Qt::AlignLeft | Qt::AlignVCenter,
                         QString::number(fraction * maximumSpeed, 'f', 0));
    }
    painter.drawText(QRectF(colorBar.left() - 15, colorBar.top() - 25, 75, 20), Qt::AlignLeft,
                     QStringLiteral("风速 (m/s)"));
    painter.setPen(QColor("#667085"));
    painter.drawText(QRectF(colorBar.left() - 20, colorBar.bottom() + 8, 95, 18), Qt::AlignLeft,
                     QStringLiteral("固定量程 0–%1").arg(maximumSpeed, 0, 'f', 0));
    painter.setPen(QColor("#667085"));
    painter.drawText(QRectF(plot.left(), height() - 22, plot.width(), 18), Qt::AlignLeft,
                     QStringLiteral("数据来源：%1").arg(m_source));
}

void WindVectorFieldWidget::resizeEvent(QResizeEvent *event)
{
    m_profileCacheDirty = true;
    QWidget::resizeEvent(event);
}

void WindVectorFieldWidget::interpolateProfile(qreal progress)
{
    if (m_profileStartLevels.size() != m_profileTargetLevels.size()) return;
    const auto interpolate = [progress](double start, double target) {
        if (!qIsFinite(start)) return target;
        if (!qIsFinite(target)) return start;
        return start + (target - start) * progress;
    };

    m_profileLevels.resize(m_profileTargetLevels.size());
    for (int index = 0; index < m_profileTargetLevels.size(); ++index) {
        const ProfileLevel &start = m_profileStartLevels.at(index);
        const ProfileLevel &target = m_profileTargetLevels.at(index);
        ProfileLevel level{
            interpolate(start.heightAglM, target.heightAglM),
            interpolate(start.eastwardMps, target.eastwardMps),
            interpolate(start.northwardMps, target.northwardMps),
            interpolate(start.upwardMps, target.upwardMps),
            interpolate(start.confidencePct, target.confidencePct),
            {}
        };
        const int radialCount = qMin(start.radialVelocityMps.size(), target.radialVelocityMps.size());
        level.radialVelocityMps.reserve(radialCount);
        for (int beam = 0; beam < radialCount; ++beam) {
            level.radialVelocityMps.append(interpolate(start.radialVelocityMps.at(beam),
                                                       target.radialVelocityMps.at(beam)));
        }
        m_profileLevels[index] = level;
    }
    m_profileCacheDirty = true;
}

void WindVectorFieldWidget::rebuildProfileCache()
{
    if (size().isEmpty()) return;
    m_profileCache = QPixmap(size());
    m_profileCache.fill(QColor("#f5f7fa"));
    QPainter cachePainter(&m_profileCache);
    paintWindProfile(cachePainter);
    m_profileCacheDirty = false;
}

void WindVectorFieldWidget::paintWindProfile(QPainter &painter) const
{
    painter.fillRect(rect(), QColor("#ffffff"));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.save();
    painter.setOpacity(0.0);
    painter.setPen(QColor("#1d2939"));
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 11, QFont::DemiBold));
    painter.drawText(QRectF(18, 8, width() - 36, 25), Qt::AlignLeft,
                     QStringLiteral("时间-高度风矢量剖面"));
    painter.setPen(QColor("#667085"));
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 8));
    painter.drawText(QRectF(18, 31, width() - 36, 18), Qt::AlignLeft,
                     QStringLiteral("箭头表示水平风向与大小 · 颜色表示水平风速 · 向上为高度 AGL"));

    painter.restore();

    const QRectF content = QRectF(rect()).adjusted(0, 0, 0, -26);
    const double gap = 14.0;
    const double profileWidth = qMax(245.0, content.width() * 0.28);
    const QRectF curtainPanel(content.left(), content.top(),
                              content.width() - profileWidth - gap, content.height());
    const QRectF profilePanel(curtainPanel.right() + gap, content.top(),
                              profileWidth, content.height());
    painter.setPen(QPen(QColor("#e4e7ec"), 1));
    const double separatorX = curtainPanel.right() + gap / 2.0;
    painter.drawLine(QPointF(separatorX, content.top() + 4),
                     QPointF(separatorX, content.bottom() - 4));

    double maximumHeight = 100.0;
    double maximumSpeed = 10.0;
    const auto inspectLevels = [&](const QVector<ProfileLevel> &levels) {
        for (const ProfileLevel &level : levels) {
            maximumHeight = qMax(maximumHeight, level.heightAglM);
            maximumSpeed = qMax(maximumSpeed, qSqrt(level.eastwardMps * level.eastwardMps
                                                    + level.northwardMps * level.northwardMps));
        }
    };
    inspectLevels(m_profileLevels);
    for (const ProfileSnapshot &snapshot : m_profileHistory) inspectLevels(snapshot.levels);
    maximumSpeed = qMax(m_speedScaleMaximumMps, qCeil(maximumSpeed / 5.0) * 5.0);

    const QRectF curtain = curtainPanel.adjusted(54, 33, -20, -55);
    const QRectF profile = profilePanel.adjusted(45, 33, -18, -55);
    painter.setPen(QColor("#344054"));
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9, QFont::DemiBold));
    const auto windowText = [](int seconds) {
        if (seconds >= 86400) return QStringLiteral("24 小时");
        if (seconds >= 43200) return QStringLiteral("12 小时");
        if (seconds >= 21600) return QStringLiteral("6 小时");
        if (seconds >= 7200) return QStringLiteral("2 小时");
        if (seconds >= 3600) return QStringLiteral("1 小时");
        if (seconds >= 1800) return QStringLiteral("30 分钟");
        if (seconds >= 600) return QStringLiteral("10 分钟");
        if (seconds >= 300) return QStringLiteral("5 分钟");
        return QStringLiteral("1 分钟");
    };
    const QString windowLabel = QStringLiteral("最近 %1 风矢量剖面").arg(windowText(m_historyWindowSeconds));
    painter.drawText(QRectF(curtain.left(), curtainPanel.top() + 7, curtain.width(), 20),
                     Qt::AlignLeft, windowLabel);
    painter.drawText(QRectF(profile.left(), profilePanel.top() + 7, profile.width(), 20),
                     Qt::AlignLeft, QStringLiteral("当前风速廓线"));

    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 8));
    const double heightStep = maximumHeight <= 500.0 ? 50.0
        : (maximumHeight <= 1000.0 ? 100.0 : 500.0);
    const int heightTickCount = qMax(1, qFloor(maximumHeight / heightStep));
    for (int tick = 0; tick <= heightTickCount; ++tick) {
        const double referenceHeight = tick * heightStep;
        const double fraction = referenceHeight / maximumHeight;
        const double yCurtain = curtain.bottom() - fraction * curtain.height();
        const double yProfile = profile.bottom() - fraction * profile.height();
        painter.setPen(QPen(QColor("#e4e9ef"), 1, Qt::DashLine));
        painter.drawLine(QPointF(curtain.left(), yCurtain), QPointF(curtain.right(), yCurtain));
        painter.drawLine(QPointF(profile.left(), yProfile), QPointF(profile.right(), yProfile));
        painter.setPen(QColor("#667085"));
        const QString heightLabel = QString::number(referenceHeight, 'f', 0);
        painter.drawText(QRectF(curtainPanel.left() + 3, yCurtain - 8, 45, 16),
                         Qt::AlignRight | Qt::AlignVCenter, heightLabel);
        painter.drawText(QRectF(profilePanel.left() + 2, yProfile - 8, 37, 16),
                         Qt::AlignRight | Qt::AlignVCenter, heightLabel);
    }

    const qint64 nowMs = m_targetTimestampMs > 0 ? m_targetTimestampMs : QDateTime::currentMSecsSinceEpoch();
    const qint64 windowDurationMs = static_cast<qint64>(m_historyWindowSeconds) * 1000;
    const qint64 windowStartMs = nowMs - windowDurationMs;
    for (int tick = 0; tick <= 4; ++tick) {
        const double fraction = tick / 4.0;
        const double x = curtain.left() + fraction * curtain.width();
        painter.setPen(QPen(QColor("#d7e0e8"), 1, Qt::DashLine));
        painter.drawLine(QPointF(x, curtain.top()), QPointF(x, curtain.bottom()));
        const qint64 timestamp = windowStartMs
            + static_cast<qint64>(fraction * static_cast<double>(windowDurationMs));
        painter.setPen(QColor("#667085"));
        const QString timeFormat = m_historyWindowSeconds <= 3600
            ? QStringLiteral("HH:mm:ss")
            : (m_historyWindowSeconds <= 21600 ? QStringLiteral("HH:mm")
                                                : QStringLiteral("MM-dd HH:mm"));
        painter.drawText(QRectF(x - 48, curtain.bottom() + 7, 96, 28),
                         Qt::AlignHCenter | Qt::AlignTop,
                         QDateTime::fromMSecsSinceEpoch(timestamp).toString(timeFormat));
    }

    const auto drawWindBarb = [&](const QPointF &position, const ProfileLevel &level, double length) {
        const double speed = qSqrt(level.eastwardMps * level.eastwardMps
                                   + level.northwardMps * level.northwardMps);
        if (!qIsFinite(speed) || speed < 0.05 || level.confidencePct < 20.0) return;
        const QPointF direction(level.eastwardMps / speed, -level.northwardMps / speed);
        const QPointF normal(-direction.y(), direction.x());
        const QPointF start = position - direction * (length * 0.42);
        const QPointF end = position + direction * (length * 0.42);
        QColor color = speedColor(speed, maximumSpeed);
        if (level.confidencePct < 50.0) color = QColor("#98a2b3");
        painter.setPen(QPen(color, 1.35, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(start, end);
        const int feathers = qBound(1, qRound(speed / 5.0), 4);
        for (int feather = 0; feather < feathers; ++feather) {
            const QPointF base = end - direction * (2.2 + feather * 2.4);
            painter.drawLine(base, base - direction * 3.0 + normal * 3.2);
        }
    };

    const auto drawSnapshot = [&](qint64 timestampMs, const QVector<ProfileLevel> &levels, bool current) {
        if (timestampMs < windowStartMs || levels.isEmpty()) return;
        const double fraction = qBound(0.0,
            static_cast<double>(timestampMs - windowStartMs) / windowDurationMs, 1.0);
        const double x = curtain.left() + fraction * curtain.width();
        const int gateStep = qMax(1, qCeil(levels.size() / 12.0));
        const double arrowLength = current ? 12.0 : 11.0;
        for (int gate = 0; gate < levels.size(); gate += gateStep) {
            const ProfileLevel &level = levels.at(gate);
            const double y = curtain.bottom() - level.heightAglM / maximumHeight * curtain.height();
            drawWindBarb(QPointF(x, y), level, arrowLength);
        }
    };
    int visibleHistoryCount = 0;
    for (const ProfileSnapshot &snapshot : std::as_const(m_profileHistory)) {
        if (snapshot.timestampMs >= windowStartMs) ++visibleHistoryCount;
    }
    const int historyStep = qMax(1, qCeil(visibleHistoryCount / 24.0));
    int visibleIndex = 0;
    for (int index = 0; index < m_profileHistory.size(); ++index) {
        const ProfileSnapshot &snapshot = m_profileHistory.at(index);
        if (snapshot.timestampMs < windowStartMs) continue;
        if ((visibleIndex++ % historyStep) == 0) {
            drawSnapshot(snapshot.timestampMs, snapshot.levels, false);
        }
    }
    drawSnapshot(nowMs, m_profileLevels, true);

    QPainterPath speedPath;
    bool pathStarted = false;
    const int currentGateStep = qMax(1, qCeil(m_profileLevels.size() / 12.0));
    for (int gate = 0; gate < m_profileLevels.size(); ++gate) {
        const ProfileLevel &level = m_profileLevels.at(gate);
        const double speed = qSqrt(level.eastwardMps * level.eastwardMps
                                   + level.northwardMps * level.northwardMps);
        if (!qIsFinite(speed)) continue;
        const QPointF point(profile.left() + speed / maximumSpeed * profile.width(),
                            profile.bottom() - level.heightAglM / maximumHeight * profile.height());
        if (!pathStarted) speedPath.moveTo(point);
        else speedPath.lineTo(point);
        pathStarted = true;
        painter.setPen(Qt::NoPen);
        painter.setBrush(level.confidencePct >= 50.0 ? speedColor(speed, maximumSpeed)
                                                     : QColor("#98a2b3"));
        painter.drawEllipse(point, 2.4, 2.4);
        if (gate % currentGateStep == 0) drawWindBarb(point + QPointF(12, 0), level, 11.0);
    }
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor("#246b8f"), 1.8, Qt::SolidLine, Qt::RoundCap));
    painter.drawPath(speedPath);

    for (int tick = 0; tick <= 3; ++tick) {
        const double fraction = tick / 3.0;
        const double x = profile.left() + fraction * profile.width();
        painter.setPen(QPen(QColor("#edf1f5"), 1));
        painter.drawLine(QPointF(x, profile.top()), QPointF(x, profile.bottom()));
        painter.setPen(QColor("#667085"));
        painter.drawText(QRectF(x - 22, profile.bottom() + 7, 44, 16), Qt::AlignCenter,
                         QString::number(maximumSpeed * fraction, 'f', 0));
    }

    painter.setPen(QColor("#475467"));
    painter.drawRect(curtain);
    painter.drawRect(profile);
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 8));
    painter.drawText(QRectF(curtain.left(), curtain.bottom() + 27, curtain.width(), 16),
                     Qt::AlignCenter, QStringLiteral("时间"));
    painter.drawText(QRectF(profile.left(), profile.bottom() + 27, profile.width(), 16),
                     Qt::AlignCenter, QStringLiteral("水平风速 (m/s)"));

    const QRectF colorBar(curtain.left(), curtainPanel.bottom() - 17, qMin(260.0, curtain.width() * 0.42), 7);
    painter.setPen(Qt::NoPen);
    constexpr int colorSteps = 80;
    for (int step = 0; step < colorSteps; ++step) {
        const double fraction = step / static_cast<double>(colorSteps - 1);
        painter.setBrush(speedColor(maximumSpeed * fraction, maximumSpeed));
        painter.drawRect(QRectF(colorBar.left() + fraction * colorBar.width(), colorBar.top(),
                                colorBar.width() / colorSteps + 1.0, colorBar.height()));
    }
    painter.setPen(QColor("#667085"));
    painter.drawText(QRectF(colorBar.right() + 7, colorBar.top() - 5, 70, 16), Qt::AlignLeft,
                     QStringLiteral("0-%1 m/s").arg(maximumSpeed, 0, 'f', 0));
    painter.drawText(QRectF(profilePanel.left(), height() - 25, profilePanel.width(), 17),
                     Qt::AlignRight, QStringLiteral("%1 · 连续插值显示").arg(m_source));
}

void WindVectorFieldWidget::paintPolarDiagnostic(QPainter &painter) const
{
    painter.fillRect(rect(), QColor("#f5f7fa"));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QColor("#1d2939"));
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 11, QFont::DemiBold));
    painter.drawText(QRectF(18, 10, width() - 36, 26), Qt::AlignLeft,
                     QStringLiteral("五波束风场诊断产品"));
    painter.setPen(QColor("#667085"));
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 8));
    painter.drawText(QRectF(18, 32, width() - 36, 18), Qt::AlignLeft,
                     QStringLiteral("真北向上 · 顺时针方位 · 高度环带由内向外"));

    const QRectF content = QRectF(rect()).adjusted(14, 54, -14, -35);
    const double gap = 14.0;
    const double panelWidth = (content.width() - gap) / 2.0;
    const QRectF panels[] = {
        QRectF(content.left(), content.top(), panelWidth, content.height()),
        QRectF(content.left() + panelWidth + gap, content.top(), panelWidth, content.height())
    };

    double maximumRadial = 2.0;
    double maximumSpeed = 5.0;
    for (const ProfileLevel &level : m_profileLevels) {
        maximumSpeed = qMax(maximumSpeed, qSqrt(level.eastwardMps * level.eastwardMps
                                                + level.northwardMps * level.northwardMps));
        for (double radial : level.radialVelocityMps) {
            if (qIsFinite(radial)) maximumRadial = qMax(maximumRadial, qAbs(radial));
        }
    }
    maximumRadial = qCeil(maximumRadial / 2.0) * 2.0;
    maximumSpeed = qMax(m_speedScaleMaximumMps, qCeil(maximumSpeed / 5.0) * 5.0);

    const auto blend = [](const QColor &a, const QColor &b, double ratio) {
        return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * ratio,
                                a.greenF() + (b.greenF() - a.greenF()) * ratio,
                                a.blueF() + (b.blueF() - a.blueF()) * ratio);
    };
    const auto radialColor = [&](double value) {
        const double normalized = qBound(-1.0, value / maximumRadial, 1.0);
        if (normalized < 0.0) return blend(QColor("#f4f7f9"), QColor("#1764ab"), -normalized);
        return blend(QColor("#f4f7f9"), QColor("#d73027"), normalized);
    };
    const auto polarPoint = [](const QPointF &center, double radius, double azimuthDeg) {
        const double angle = qDegreesToRadians(azimuthDeg);
        return QPointF(center.x() + radius * qSin(angle), center.y() - radius * qCos(angle));
    };
    const auto interpolatedRadial = [](const ProfileLevel &level, double azimuthDeg) {
        if (level.radialVelocityMps.size() < 5) return qQNaN();
        double shifted = std::fmod(azimuthDeg - 45.0 + 360.0, 360.0);
        const int sector = static_cast<int>(shifted / 90.0);
        const double ratio = (shifted - sector * 90.0) / 90.0;
        const double first = level.radialVelocityMps.at(1 + sector);
        const double second = level.radialVelocityMps.at(1 + ((sector + 1) % 4));
        return qIsFinite(first) && qIsFinite(second) ? first + (second - first) * ratio : qQNaN();
    };

    for (int panelIndex = 0; panelIndex < 2; ++panelIndex) {
        const QRectF panel = panels[panelIndex];
        painter.setPen(QPen(QColor("#d6dee7"), 1));
        painter.setBrush(QColor("#ffffff"));
        painter.drawRect(panel);
        painter.setPen(QColor("#344054"));
        painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9, QFont::DemiBold));
        painter.drawText(QRectF(panel.left() + 12, panel.top() + 7, panel.width() - 24, 22),
                         Qt::AlignLeft, panelIndex == 0
                         ? QStringLiteral("径向速度 · 四斜束方位插值")
                         : QStringLiteral("水平风速 · 高度环带产品"));

        const double radius = qMax(40.0, qMin((panel.width() - 76.0) / 2.0,
                                              (panel.height() - 52.0) / 2.0));
        const QPointF center(panel.left() + (panel.width() - 38.0) / 2.0,
                             panel.top() + 31.0 + (panel.height() - 43.0) / 2.0);
        const double ringWidth = radius / qMax(1, m_profileLevels.size());

        painter.setRenderHint(QPainter::Antialiasing, false);
        if (panelIndex == 0) {
            constexpr int azimuthStep = 10;
            for (int gate = 0; gate < m_profileLevels.size(); ++gate) {
                const ProfileLevel &level = m_profileLevels.at(gate);
                if (level.confidencePct < 20.0) continue;
                const double inner = gate * ringWidth;
                const double outer = (gate + 1) * ringWidth + 0.8;
                for (int azimuth = 0; azimuth < 360; azimuth += azimuthStep) {
                    const double value = interpolatedRadial(level, azimuth + azimuthStep / 2.0);
                    if (!qIsFinite(value)) continue;
                    QPolygonF sector;
                    sector << polarPoint(center, inner, azimuth)
                           << polarPoint(center, outer, azimuth)
                           << polarPoint(center, outer, azimuth + azimuthStep)
                           << polarPoint(center, inner, azimuth + azimuthStep);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(radialColor(value));
                    painter.drawPolygon(sector);
                }
            }
            if (!m_profileLevels.first().radialVelocityMps.isEmpty()) {
                const double vertical = m_profileLevels.first().radialVelocityMps.first();
                painter.setBrush(qIsFinite(vertical) ? radialColor(vertical) : QColor("#e4e7ec"));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(center, qMax(3.0, ringWidth * 1.4), qMax(3.0, ringWidth * 1.4));
            }
        } else {
            for (int gate = 0; gate < m_profileLevels.size(); ++gate) {
                const ProfileLevel &level = m_profileLevels.at(gate);
                const double speed = qSqrt(level.eastwardMps * level.eastwardMps
                                           + level.northwardMps * level.northwardMps);
                QColor color = speedColor(speed, maximumSpeed);
                if (level.confidencePct < 50.0) color.setAlpha(75);
                const double midRadius = (gate + 0.5) * ringWidth;
                painter.setPen(QPen(color, ringWidth + 1.2));
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(center, midRadius, midRadius);
            }
        }

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(Qt::NoBrush);
        for (int ring = 1; ring <= 4; ++ring) {
            const double gridRadius = radius * ring / 4.0;
            painter.setPen(QPen(QColor(102, 112, 133, 85), 0.8, Qt::DashLine));
            painter.drawEllipse(center, gridRadius, gridRadius);
        }
        for (int azimuth = 0; azimuth < 360; azimuth += 45) {
            painter.setPen(QPen(QColor(102, 112, 133, azimuth % 90 == 0 ? 125 : 60), 0.8));
            painter.drawLine(center, polarPoint(center, radius, azimuth));
        }
        painter.setPen(QColor("#475467"));
        painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 8, QFont::DemiBold));
        painter.drawText(QRectF(center.x() - 10, center.y() - radius - 17, 20, 16), Qt::AlignCenter, "N");
        painter.drawText(QRectF(center.x() + radius + 3, center.y() - 8, 20, 16), Qt::AlignCenter, "E");
        painter.drawText(QRectF(center.x() - 10, center.y() + radius + 1, 20, 16), Qt::AlignCenter, "S");
        painter.drawText(QRectF(center.x() - radius - 23, center.y() - 8, 20, 16), Qt::AlignCenter, "W");

        if (panelIndex == 1) {
            for (double fraction : {0.28, 0.52, 0.76}) {
                const int gate = qBound(0, qRound(fraction * (m_profileLevels.size() - 1)),
                                        m_profileLevels.size() - 1);
                const ProfileLevel &level = m_profileLevels.at(gate);
                const double magnitude = qSqrt(level.eastwardMps * level.eastwardMps
                                                + level.northwardMps * level.northwardMps);
                if (magnitude < 0.01 || level.confidencePct < 50.0) continue;
                const QPointF direction(level.eastwardMps / magnitude, -level.northwardMps / magnitude);
                for (int azimuth = 0; azimuth < 360; azimuth += 60) {
                    const QPointF position = polarPoint(center, radius * fraction, azimuth);
                    const QPointF start = position - direction * 6.0;
                    const QPointF end = position + direction * 6.0;
                    painter.setPen(QPen(QColor(8, 15, 22, 210), 1.2));
                    painter.drawLine(start, end);
                    const QPointF normal(-direction.y(), direction.x());
                    QPolygonF head;
                    head << end << end - direction * 4.0 + normal * 2.0
                         << end - direction * 4.0 - normal * 2.0;
                    painter.setBrush(QColor(8, 15, 22, 210));
                    painter.drawPolygon(head);
                }
            }
        }

        const QRectF scale(panel.right() - 25, center.y() - radius, 9, radius * 2.0);
        constexpr int scaleSteps = 60;
        painter.setPen(Qt::NoPen);
        for (int step = 0; step < scaleSteps; ++step) {
            const double fraction = step / static_cast<double>(scaleSteps - 1);
            const QColor color = panelIndex == 0
                ? radialColor(maximumRadial * (1.0 - 2.0 * fraction))
                : speedColor(maximumSpeed * (1.0 - fraction), maximumSpeed);
            painter.setBrush(color);
            painter.drawRect(QRectF(scale.left(), scale.top() + step * scale.height() / scaleSteps,
                                    scale.width(), scale.height() / scaleSteps + 1.0));
        }
        painter.setPen(QColor("#667085"));
        painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 7));
        painter.drawText(QRectF(scale.left() - 15, scale.top() - 17, 40, 14), Qt::AlignCenter,
                         panelIndex == 0 ? QStringLiteral("m/s") : QStringLiteral("m/s"));
        painter.drawText(QRectF(scale.left() - 18, scale.top(), 16, 14), Qt::AlignRight,
                         QString::number(panelIndex == 0 ? maximumRadial : maximumSpeed, 'f', 0));
        painter.drawText(QRectF(scale.left() - 18, scale.bottom() - 14, 16, 14), Qt::AlignRight,
                         panelIndex == 0 ? QString::number(-maximumRadial, 'f', 0) : QStringLiteral("0"));
    }

    painter.setPen(QColor("#667085"));
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei"), 8));
    painter.drawText(QRectF(18, height() - 27, width() - 36, 18), Qt::AlignLeft,
                     QStringLiteral("%1  |  方位间区域为四斜束几何插值，仅用于诊断，不等同于扫描雷达 PPI")
                         .arg(m_source));
}

QColor WindVectorFieldWidget::speedColor(double speed, double maximum) const
{
    const double value = qBound(0.0, speed / qMax(0.001, maximum), 1.0);
    struct Stop { double position; QColor color; };
    const Stop stops[] = {
        {0.0, QColor("#243B75")}, {0.2, QColor("#2F80C1")},
        {0.4, QColor("#49B6A8")}, {0.6, QColor("#E4D354")},
        {0.8, QColor("#E88732")}, {1.0, QColor("#B6202A")}
    };
    for (int index = 0; index < 5; ++index) {
        if (value <= stops[index + 1].position) {
            const double ratio = (value - stops[index].position)
                / (stops[index + 1].position - stops[index].position);
            return QColor::fromRgbF(
                stops[index].color.redF() + ratio * (stops[index + 1].color.redF() - stops[index].color.redF()),
                stops[index].color.greenF() + ratio * (stops[index + 1].color.greenF() - stops[index].color.greenF()),
                stops[index].color.blueF() + ratio * (stops[index + 1].color.blueF() - stops[index].color.blueF()));
        }
    }
    return stops[5].color;
}

void WindVectorFieldWidget::drawArrow(QPainter &painter, const QPointF &center, double eastward,
                                      double northward, double length) const
{
    const double magnitude = qSqrt(eastward * eastward + northward * northward);
    if (magnitude < 0.01) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(20, 28, 38, 190));
        painter.drawEllipse(center, 1.5, 1.5);
        return;
    }
    const QPointF direction(eastward / magnitude, -northward / magnitude);
    const QPointF start = center - direction * (length * 0.45);
    const QPointF end = center + direction * (length * 0.45);
    painter.setPen(QPen(QColor(15, 23, 32, 205), 1.15, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(start, end);
    const QPointF normal(-direction.y(), direction.x());
    QPainterPath head;
    head.moveTo(end);
    head.lineTo(end - direction * 5.0 + normal * 2.5);
    head.lineTo(end - direction * 5.0 - normal * 2.5);
    head.closeSubpath();
    painter.fillPath(head, QColor(15, 23, 32, 205));
}
