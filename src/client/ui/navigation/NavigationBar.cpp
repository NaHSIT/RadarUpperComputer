#include "NavigationBar.h"
#include <QPainter>
#include <QPainterPath>
#include <QSize>

NavigationBar::NavigationBar(QWidget *parent)
    : QWidget(parent)
    , m_listWidget(nullptr)
    , m_logoLabel(nullptr)
    , m_layout(nullptr)
    , m_currentIndex(-1)
{
    setupUI();
    setFixedWidth(200);
}

NavigationBar::~NavigationBar()
{
}

void NavigationBar::addItem(const QString &icon, const QString &text, int pageIndex)
{
    NavItem item;
    item.icon = icon;
    item.text = text;
    item.pageIndex = pageIndex;
    m_items.append(item);

    QListWidgetItem *listItem = new QListWidgetItem(m_listWidget);
    listItem->setText(QString("  %1  %2").arg(icon, text));
    listItem->setData(Qt::UserRole, pageIndex);
    listItem->setSizeHint(QSize(0, 45));
}

void NavigationBar::setCurrentIndex(int index)
{
    if (index >= 0 && index < m_listWidget->count()) {
        updateSelection(index);
    }
}

void NavigationBar::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // Logo 区域
    m_logoLabel = new QLabel("测风雷达", this);
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold; padding: 20px;");
    m_logoLabel->setFixedHeight(60);
    m_layout->addWidget(m_logoLabel);

    // 导航列表
    m_listWidget = new QListWidget(this);
    m_listWidget->setStyleSheet(
        "QListWidget {"
        "  background: #2C3E50;"
        "  border: none;"
        "  outline: none;"
        "}"
        "QListWidget::item {"
        "  color: #BDC3C7;"
        "  padding: 12px 15px;"
        "  border-left: 3px solid transparent;"
        "}"
        "QListWidget::item:hover {"
        "  background: #34495E;"
        "  color: white;"
        "}"
        "QListWidget::item:selected {"
        "  background: #3498DB;"
        "  color: white;"
        "  border-left: 3px solid #2980B9;"
        "}"
    );

    connect(m_listWidget, &QListWidget::currentRowChanged,
            this, &NavigationBar::onCurrentRowChanged);

    m_layout->addWidget(m_listWidget);

    // 底部弹性空间
    m_layout->addStretch();

    // 设置背景色
    setStyleSheet("background: #2C3E50;");
}

void NavigationBar::onItemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int pageIndex = item->data(Qt::UserRole).toInt();
    int oldIndex = m_currentIndex;
    m_currentIndex = m_listWidget->row(item);

    emit itemClicked(pageIndex);
    if (oldIndex != m_currentIndex) {
        emit itemChanged(oldIndex, m_currentIndex);
    }
}

void NavigationBar::onCurrentRowChanged(int row)
{
    if (row >= 0 && row < m_listWidget->count()) {
        QListWidgetItem *item = m_listWidget->item(row);
        if (item) {
            onItemClicked(item);
        }
    }
}

void NavigationBar::updateSelection(int index)
{
    if (index >= 0 && index < m_listWidget->count()) {
        m_listWidget->setCurrentRow(index);
    }
}
