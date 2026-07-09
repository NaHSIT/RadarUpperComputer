#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief 导航栏组件
 *
 * 左侧纵向导航栏，显示页面入口
 */
class NavigationBar : public QWidget
{
    Q_OBJECT

public:
    explicit NavigationBar(QWidget *parent = nullptr);
    ~NavigationBar() override;

    // 添加导航项
    void addItem(const QString &icon, const QString &text, int pageIndex);

    // 设置当前选中项
    void setCurrentIndex(int index);

    // 获取当前索引
    int currentIndex() const { return m_currentIndex; }

signals:
    void itemClicked(int pageIndex);
    void itemChanged(int oldIndex, int newIndex);

private slots:
    void onItemClicked(QListWidgetItem *item);
    void onCurrentRowChanged(int row);

private:
    void setupUI();
    void updateSelection(int index);

    QListWidget *m_listWidget;
    QLabel *m_logoLabel;
    QVBoxLayout *m_layout;
    int m_currentIndex;
    struct NavItem {
        QString icon;
        QString text;
        int pageIndex;
    };
    QVector<NavItem> m_items;
};

#endif // NAVIGATIONBAR_H
