#include "NavigationBar.h"

#include <QSize>

NavigationBar::NavigationBar(QWidget *parent)
    : QWidget(parent), m_listWidget(nullptr), m_logoLabel(nullptr), m_layout(nullptr), m_currentIndex(-1)
{
    setupUI();
    setFixedWidth(224);
}

NavigationBar::~NavigationBar() = default;

void NavigationBar::addItem(const QString &icon, const QString &text, int pageIndex)
{
    m_items.append({icon, text, pageIndex});
    auto *item = new QListWidgetItem(QStringLiteral("%1    %2").arg(icon, text), m_listWidget);
    item->setData(Qt::UserRole, pageIndex);
    item->setSizeHint(QSize(0, 46));
}

void NavigationBar::setCurrentIndex(int index)
{
    if (index >= 0 && index < m_listWidget->count()) updateSelection(index);
}

void NavigationBar::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_logoLabel = new QLabel(QStringLiteral("测风雷达\n运行监控系统"), this);
    m_logoLabel->setStyleSheet("background:#17324d; color:#ffffff; font-size:16px; font-weight:600; padding:18px 20px; line-height:1.4;");
    m_logoLabel->setFixedHeight(78);
    m_layout->addWidget(m_logoLabel);

    auto *section = new QLabel(QStringLiteral("功能导航"), this);
    section->setStyleSheet("background:#17324d; color:#9fb4c8; font-size:11px; padding:16px 20px 8px;");
    m_layout->addWidget(section);
    m_listWidget = new QListWidget(this);
    m_listWidget->setStyleSheet(
        "QListWidget { background:#17324d; border:0; outline:0; color:#d8e3ed; font-size:14px; }"
        "QListWidget::item { padding:0 20px; border-left:3px solid transparent; }"
        "QListWidget::item:hover { background:#20415f; }"
        "QListWidget::item:selected { background:#244a6a; border-left-color:#54a3d8; color:#ffffff; font-weight:600; }"
    );
    connect(m_listWidget, &QListWidget::currentRowChanged, this, &NavigationBar::onCurrentRowChanged);
    m_layout->addWidget(m_listWidget, 1);
    auto *footer = new QLabel(QStringLiteral("工业现场版"), this);
    footer->setStyleSheet("background:#17324d; color:#7f99af; padding:12px 20px; font-size:11px;");
    m_layout->addWidget(footer);
}

void NavigationBar::onItemClicked(QListWidgetItem *item)
{
    if (!item) return;
    const int oldIndex = m_currentIndex;
    m_currentIndex = m_listWidget->row(item);
    emit itemClicked(item->data(Qt::UserRole).toInt());
    if (oldIndex != m_currentIndex) emit itemChanged(oldIndex, m_currentIndex);
}

void NavigationBar::onCurrentRowChanged(int row)
{
    if (row >= 0 && row < m_listWidget->count()) onItemClicked(m_listWidget->item(row));
}

void NavigationBar::updateSelection(int index) { m_listWidget->setCurrentRow(index); }
