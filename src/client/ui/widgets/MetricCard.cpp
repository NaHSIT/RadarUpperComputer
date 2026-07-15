#include "MetricCard.h"

#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

MetricCard::MetricCard(QWidget *parent)
    : QWidget(parent)
    , m_value(0.0)
    , m_status("normal")
    , m_titleLabel(nullptr)
    , m_valueLabel(nullptr)
    , m_unitLabel(nullptr)
    , m_valueAnimation(new QVariantAnimation(this))
{
    setupUI();
    setMinimumSize(172, 120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_valueAnimation->setDuration(900);
    m_valueAnimation->setEasingCurve(QEasingCurve::InOutSine);
    connect(m_valueAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
        m_value = value.toDouble();
        updateDisplay();
    });
}

MetricCard::~MetricCard() = default;

void MetricCard::setData(const QString &title, double value, const QString &unit, const QString &status)
{
    m_title = title;
    m_value = value;
    m_unit = unit;
    m_status = status;
    m_titleLabel->setText(title);
    m_unitLabel->setText(unit);
    updateDisplay();
}

void MetricCard::setValue(double value)
{
    if (qFuzzyCompare(m_value + 1.0, value + 1.0)) return;
    if (!isVisible()) {
        m_valueAnimation->stop();
        m_value = value;
        updateDisplay();
        return;
    }
    m_valueAnimation->stop();
    m_valueAnimation->setStartValue(m_value);
    m_valueAnimation->setEndValue(value);
    m_valueAnimation->start();
}
void MetricCard::setTitle(const QString &title)
{
    m_title = title;
    m_titleLabel->setText(title);
}
void MetricCard::setToolTip(const QString &text)
{
    m_explanation = text;
    QWidget::setToolTip(text);
    m_titleLabel->setToolTip(text);
    m_valueLabel->setToolTip(text);
    m_unitLabel->setToolTip(text);
}
void MetricCard::setStatus(const QString &status) { m_status = status; update(); }

void MetricCard::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 15, 14, 12);
    mainLayout->setSpacing(7);
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("color:#172b3d; font-size:17px; font-weight:600;");
    m_titleLabel->setCursor(Qt::PointingHandCursor);
    m_titleLabel->installEventFilter(this);
    mainLayout->addWidget(m_titleLabel);
    auto *valueLayout = new QHBoxLayout();
    valueLayout->setSpacing(4);
    m_valueLabel = new QLabel(this);
    m_valueLabel->setStyleSheet("color:#101828; font-size:32px; font-weight:600;");
    m_unitLabel = new QLabel(this);
    m_unitLabel->setStyleSheet("color:#344054; font-size:15px; font-weight:600; padding-top:10px;");
    valueLayout->addWidget(m_valueLabel);
    valueLayout->addWidget(m_unitLabel);
    valueLayout->addStretch();
    mainLayout->addLayout(valueLayout);
    setStyleSheet("MetricCard { background:#ffffff; border:1px solid #dce4ec; border-radius:6px; }"
                  "MetricCard:hover { border-color:#8ab6c9; background:#fbfdff; }");
}

void MetricCard::updateDisplay()
{
    m_valueLabel->setText(QString::number(m_value, 'f', 1));
}

void MetricCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QColor color("#2f6f9f");
    if (m_status == "warning") color = QColor("#b7791f");
    if (m_status == "danger") color = QColor("#c93d32");
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#ffffff"));
    painter.setPen(QPen(QColor("#dce4ec"), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 6, 6);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(18, 8, 24, 3), 1.5, 1.5);
}

void MetricCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) emit clicked();
    QWidget::mousePressEvent(event);
}

bool MetricCard::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_titleLabel && event->type() == QEvent::MouseButtonRelease
        && !m_explanation.isEmpty()) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            const QPoint globalPosition = m_titleLabel->mapToGlobal(
                QPoint(0, m_titleLabel->height() + 6));
            QToolTip::showText(globalPosition, m_explanation, m_titleLabel,
                               m_titleLabel->rect(), 20000);
            emit clicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
