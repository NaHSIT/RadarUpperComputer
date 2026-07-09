#include "MetricCard.h"
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>

MetricCard::MetricCard(QWidget *parent)
    : QWidget(parent)
    , m_value(0.0)
    , m_status("normal")
    , m_titleLabel(nullptr)
    , m_valueLabel(nullptr)
    , m_unitLabel(nullptr)
{
    setupUI();
    setFixedSize(180, 100);
}

MetricCard::~MetricCard()
{
}

void MetricCard::setData(const QString &title, double value, const QString &unit, const QString &status)
{
    m_title = title;
    m_value = value;
    m_unit = unit;
    m_status = status;

    if (m_titleLabel) m_titleLabel->setText(title);
    updateDisplay();
}

void MetricCard::setValue(double value)
{
    m_value = value;
    updateDisplay();
}

void MetricCard::setStatus(const QString &status)
{
    m_status = status;
    update();
}

void MetricCard::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(4);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("color: #666; font-size: 12px;");
    mainLayout->addWidget(m_titleLabel);

    QHBoxLayout *valueLayout = new QHBoxLayout();
    valueLayout->setSpacing(4);

    m_valueLabel = new QLabel(this);
    m_valueLabel->setStyleSheet("color: #333; font-size: 24px; font-weight: bold;");
    valueLayout->addWidget(m_valueLabel);

    m_unitLabel = new QLabel(this);
    m_unitLabel->setStyleSheet("color: #999; font-size: 12px;");
    valueLayout->addWidget(m_unitLabel);
    valueLayout->addStretch();

    mainLayout->addLayout(valueLayout);
    mainLayout->addStretch();

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(10);
    shadow->setXOffset(0);
    shadow->setYOffset(2);
    shadow->setColor(QColor(0, 0, 0, 30));
    setGraphicsEffect(shadow);
}

void MetricCard::updateDisplay()
{
    if (m_valueLabel) {
        m_valueLabel->setText(QString::number(m_value, 'f', 2));
    }
}

void MetricCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}
