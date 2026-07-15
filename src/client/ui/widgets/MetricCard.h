#ifndef METRICCARD_H
#define METRICCARD_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QString>
#include <QVariantAnimation>

class MetricCard : public QWidget
{
    Q_OBJECT

public:
    explicit MetricCard(QWidget *parent = nullptr);
    ~MetricCard() override;

    void setData(const QString &title, double value, const QString &unit, const QString &status = "normal");
    void setValue(double value);
    void setTitle(const QString &title);
    void setToolTip(const QString &text);
    void setStatus(const QString &status);
    QString status() const { return m_status; }

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void updateDisplay();

    QString m_title;
    double m_value;
    QString m_unit;
    QString m_status;
    QString m_explanation;

    QLabel *m_titleLabel;
    QLabel *m_valueLabel;
    QLabel *m_unitLabel;
    QVariantAnimation *m_valueAnimation;
};

#endif // METRICCARD_H
