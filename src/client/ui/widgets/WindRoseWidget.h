#ifndef WINDROSEWIDGET_H
#define WINDROSEWIDGET_H

#include <QWidget>
#include <QVector>

class WindRoseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WindRoseWidget(QWidget *parent = nullptr);
    ~WindRoseWidget() override;

    void setWindDirection(double direction);
    void setDirectionHistory(const QVector<double> &directions);

signals:
    void directionHovered(double direction);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();

    double m_currentDirection;
    QVector<double> m_directionHistory;
};

#endif // WINDROSEWIDGET_H
