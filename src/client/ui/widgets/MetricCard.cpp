#include "MetricCard.h"

#include <QMouseEvent>
#include <QPainter>

MetricCard::MetricCard(QWidget *parent)
    : QWidget(parent)
    , m_value(0.0)
    , m_status("normal")
    , m_titleLabel(nullptr)
    , m_valueLabel(nullptr)
    , m_unitLabel(nullptr)
{
    setupUI();
    setMinimumSize(148, 92);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
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

void MetricCard::setValue(double value) { m_value = value; updateDisplay(); }
void MetricCard::setStatus(const QString &status) { m_status = status; update(); }

void MetricCard::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 12, 12, 10);
    mainLayout->setSpacing(5);
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("color:#667085; font-size:12px;");
    mainLayout->addWidget(m_titleLabel);
    auto *valueLayout = new QHBoxLayout();
    valueLayout->setSpacing(4);
    m_valueLabel = new QLabel(this);
    m_valueLabel->setStyleSheet("color:#182230; font-size:26px; font-weight:600;");
    m_unitLabel = new QLabel(this);
    m_unitLabel->setStyleSheet("color:#667085; font-size:12px; padding-top:7px;");
    valueLayout->addWidget(m_valueLabel);
    valueLayout->addWidget(m_unitLabel);
    valueLayout->addStretch();
    mainLayout->addLayout(valueLayout);
    setStyleSheet("MetricCard { background:#ffffff; border:1px solid #d9dee5; border-radius:4px; }"
                  "MetricCard:hover { border-color:#9eb4c7; }");
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
    painter.fillRect(QRect(0, 0, 4, height()), color);
}

void MetricCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) emit clicked();
    QWidget::mousePressEvent(event);
}
