#include "WindRoseWidget.h"
#include <QPainter>
#include <QtMath>
#include <cmath>

WindRoseWidget::WindRoseWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentDirection(0)
    , m_directionAnimation(new QVariantAnimation(this))
{
    setupUI();
    setMinimumSize(200, 200);
    setMaximumSize(300, 300);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_directionAnimation->setDuration(900);
    m_directionAnimation->setEasingCurve(QEasingCurve::InOutSine);
    connect(m_directionAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
        m_currentDirection = std::fmod(value.toDouble() + 360.0, 360.0);
        update();
    });
}

WindRoseWidget::~WindRoseWidget()
{
}

void WindRoseWidget::setWindDirection(double direction)
{
    const double target = std::fmod(direction + 360.0, 360.0);
    if (!isVisible()) {
        m_directionAnimation->stop();
        m_currentDirection = target;
        return;
    }
    const double delta = std::fmod(target - m_currentDirection + 540.0, 360.0) - 180.0;
    m_directionAnimation->stop();
    m_directionAnimation->setStartValue(m_currentDirection);
    m_directionAnimation->setEndValue(m_currentDirection + delta);
    m_directionAnimation->start();
}

void WindRoseWidget::setDirectionHistory(const QVector<double> &directions)
{
    m_directionHistory = directions;
    update();
}

void WindRoseWidget::setupUI()
{
    setStyleSheet("background: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 8px;");
}

void WindRoseWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int side = qMin(width(), height());

    // 绘制圆角背景
    QPainterPath bgPath;
    bgPath.addRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
    painter.fillPath(bgPath, QColor("#FFFFFF"));

    // 中心点和半径
    QPointF center(width() / 2.0, height() / 2.0);
    double outerRadius = side / 2.0 - 30;
    double innerRadius = outerRadius * 0.15;

    // 绘制外圈
    painter.setPen(QPen(QColor("#E5E7EB"), 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, outerRadius, outerRadius);

    // 绘制内圈（同心圆）
    painter.setPen(QPen(QColor("#F3F4F6"), 1, Qt::DashLine));
    for (int i = 1; i <= 4; ++i) {
        double r = outerRadius * i / 4.0;
        painter.drawEllipse(center, r, r);
    }

    // 绘制方向线和标签
    struct DirLabel {
        double angle;
        QString label;
    };
    QVector<DirLabel> labels = {
        {0, QStringLiteral("N")},
        {45, QStringLiteral("NE")},
        {90, QStringLiteral("E")},
        {135, QStringLiteral("SE")},
        {180, QStringLiteral("S")},
        {225, QStringLiteral("SW")},
        {270, QStringLiteral("W")},
        {315, QStringLiteral("NW")}
    };

    painter.setFont(QFont("Microsoft YaHei", 11, QFont::Bold));
    for (const auto &dl : labels) {
        double rad = qDegreesToRadians(dl.angle - 90); // 0度=北，顺时针
        double cosA = qCos(rad);
        double sinA = qSin(rad);

        // 方向线
        QPen linePen(QColor("#D1D5DB"), 1);
        if (static_cast<int>(dl.angle) % 90 == 0) {
            linePen.setColor("#9CA3AF");
            linePen.setWidth(1.5);
        }
        painter.setPen(linePen);
        QPointF inner(center.x() + innerRadius * cosA, center.y() + innerRadius * sinA);
        QPointF outer(center.x() + outerRadius * cosA, center.y() + outerRadius * sinA);
        painter.drawLine(inner, outer);

        // 标签
        double labelRadius = outerRadius + 16;
        QPointF labelPos(center.x() + labelRadius * cosA, center.y() + labelRadius * sinA);
        painter.setPen(QColor("#6B7280"));
        QRectF labelRect(labelPos.x() - 15, labelPos.y() - 10, 30, 20);
        painter.drawText(labelRect, Qt::AlignCenter, dl.label);
    }

    // 绘制风向箭头
    double windRad = qDegreesToRadians(m_currentDirection - 90);
    double arrowLen = outerRadius * 0.75;
    QPointF arrowEnd(center.x() + arrowLen * qCos(windRad),
                     center.y() + arrowLen * qSin(windRad));
    QPointF arrowStart(center.x() - arrowLen * 0.3 * qCos(windRad),
                       center.y() - arrowLen * 0.3 * qSin(windRad));

    // 箭头主体
    QPen arrowPen(QColor("#3B82F6"), 3);
    arrowPen.setCapStyle(Qt::RoundCap);
    painter.setPen(arrowPen);
    painter.drawLine(arrowStart, arrowEnd);

    // 箭头头部
    double headLen = 12;
    double headAngle = 0.4;
    QPointF head1(arrowEnd.x() - headLen * qCos(windRad - headAngle),
                  arrowEnd.y() - headLen * qSin(windRad - headAngle));
    QPointF head2(arrowEnd.x() - headLen * qCos(windRad + headAngle),
                  arrowEnd.y() - headLen * qSin(windRad + headAngle));
    painter.setBrush(QColor("#3B82F6"));
    painter.setPen(Qt::NoPen);
    QPolygonF arrowHead;
    arrowHead << arrowEnd << head1 << head2;
    painter.drawPolygon(arrowHead);

    // 中心圆点
    painter.setBrush(QColor("#1F2937"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center, 5, 5);

    // 度数显示
    painter.setPen(QColor("#374151"));
    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    QRectF degreeRect(center.x() - 30, center.y() + outerRadius * 0.35, 60, 24);
    painter.drawText(degreeRect, Qt::AlignCenter,
                     QStringLiteral("%1°").arg(QString::number(m_currentDirection, 'f', 0)));
}
