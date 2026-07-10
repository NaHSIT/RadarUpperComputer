#ifndef WINDTRENDCHART_H
#define WINDTRENDCHART_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QString>

class WindTrendChart : public QWidget
{
    Q_OBJECT

public:
    explicit WindTrendChart(QWidget *parent = nullptr);
    ~WindTrendChart() override;

    void setTimeWindow(const QString &window);
    void addDataPoint(double value);
    void clear();

signals:
    void timeWindowChanged(const QString &window);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();
    void updateChart();

    QString m_timeWindow;
    QVector<QPointF> m_data;
    QTimer *m_refreshTimer;
};

#endif // WINDTRENDCHART_H
