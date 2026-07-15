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
#include <QVariantAnimation>

class WindTrendChart : public QWidget
{
    Q_OBJECT

public:
    enum class SeriesType { WindSpeed, WindDirection };

    explicit WindTrendChart(QWidget *parent = nullptr);
    ~WindTrendChart() override;

    void setTimeWindow(const QString &window);
    void addDataPoint(double value);
    void clear();
    void setSeriesType(SeriesType type);
    void setWindowSeconds(int seconds);
    void setData(const QVector<QPointF> &data);

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
    QVariantAnimation *m_valueAnimation;
    SeriesType m_seriesType;
    int m_windowSeconds;
    qint64 m_lastAnimationPaintMs;
};

#endif // WINDTRENDCHART_H
