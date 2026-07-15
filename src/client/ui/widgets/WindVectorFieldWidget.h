#ifndef WINDVECTORFIELDWIDGET_H
#define WINDVECTORFIELDWIDGET_H

#include <QVector>
#include <QWidget>
#include <QPixmap>
#include <QVariantAnimation>
#include <QElapsedTimer>

class WindVectorFieldWidget : public QWidget
{
    Q_OBJECT

public:
    struct WindCell {
        double eastKm;
        double northKm;
        double eastwardMps;
        double northwardMps;
        double confidencePct;
    };

    struct ProfileLevel {
        double heightAglM;
        double eastwardMps;
        double northwardMps;
        double upwardMps;
        double confidencePct;
        QVector<double> radialVelocityMps;
    };

    explicit WindVectorFieldWidget(QWidget *parent = nullptr);

    void setWindGrid(int columns, int rows, const QVector<WindCell> &cells,
                     double heightAglM, const QString &source);
    void setUniformWind(double speedMps, double windFromDirectionDeg,
                        double heightAglM, const QString &source);
    void setWindProfile(const QVector<ProfileLevel> &levels, const QString &source);
    void setHistoryWindowSeconds(int seconds);
    void setSpeedScaleMaximum(double maximumMps);
    void clearField(const QString &title = QString(), const QString &detail = QString());

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct ProfileSnapshot {
        qint64 timestampMs;
        QVector<ProfileLevel> levels;
    };

    QColor speedColor(double speed, double maximum) const;
    void drawArrow(QPainter &painter, const QPointF &center, double eastward,
                   double northward, double length) const;
    void paintWindProfile(QPainter &painter) const;
    void paintPolarDiagnostic(QPainter &painter) const;
    void rebuildProfileCache();
    void interpolateProfile(qreal progress);
    void compactProfileHistory(qint64 nowMs);

    int m_columns;
    int m_rows;
    QVector<WindCell> m_cells;
    QVector<ProfileLevel> m_profileLevels;
    QVector<ProfileLevel> m_profileStartLevels;
    QVector<ProfileLevel> m_profileTargetLevels;
    QVector<ProfileSnapshot> m_profileHistory;
    qint64 m_targetTimestampMs;
    int m_historyWindowSeconds;
    double m_heightAglM;
    QString m_source;
    QString m_emptyTitle;
    QString m_emptyDetail;
    double m_speedScaleMaximumMps;
    QPixmap m_profileCache;
    bool m_profileCacheDirty;
    QVariantAnimation *m_transitionAnimation;
    QElapsedTimer m_renderThrottle;
    int m_historyAppendCount;
};

#endif // WINDVECTORFIELDWIDGET_H
