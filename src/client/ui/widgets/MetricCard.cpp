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
    setMinimumSize(172, 112);
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
    mainLayout->setContentsMargins(18, 15, 14, 12);
    mainLayout->setSpacing(7);
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("color:#667085; font-size:12px; font-weight:600;");
    mainLayout->addWidget(m_titleLabel);
    auto *valueLayout = new QHBoxLayout();
    valueLayout->setSpacing(4);
    m_valueLabel = new QLabel(this);
    m_valueLabel->setStyleSheet("color:#172b3d; font-size:30px; font-weight:600;");
    m_unitLabel = new QLabel(this);
    m_unitLabel->setStyleSheet("color:#667085; font-size:12px; padding-top:10px;");
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
