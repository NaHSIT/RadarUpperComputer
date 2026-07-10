#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QVector>
#include <QString>

class QListWidgetItem;

class NavigationBar : public QWidget
{
    Q_OBJECT

public:
    explicit NavigationBar(QWidget *parent = nullptr);
    ~NavigationBar() override;

    void addItem(const QString &icon, const QString &text, int pageIndex);
    void setCurrentIndex(int index);
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

    struct NavItem {
        QString icon;
        QString text;
        int pageIndex;
    };

    QListWidget *m_listWidget;
    QLabel *m_logoLabel;
    QVBoxLayout *m_layout;
    int m_currentIndex;
    QVector<NavItem> m_items;
};

#endif // NAVIGATIONBAR_H
