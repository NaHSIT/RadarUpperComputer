#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include <QTimer>
#include <QVector>

// 包含头文件（因为使用了嵌套类型）
#include "widgets/AlarmList.h"
#include "widgets/RangeGateTable.h"

// 前向声明
class MetricCard;
class WindTrendChart;
class WindRoseWidget;
class BeamHealthGrid;

/**
 * @brief 总览页面
 *
 * 目标：让用户 10 秒内判断"雷达是否在线、数据是否可信、是否有风险"
 *
 * 布局：
 * - 第一行：6 个关键指标卡
 * - 第二行：风速趋势图 + 风向罗盘 + 五波束健康矩阵
 * - 第三行：分层风场表 + 告警列表
 */
class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;

    // 更新数据
    void updateWindData(double windSpeed, double windDirection, double confidence,
                        int validGates, double blindRatio, int alarmCount);
    void updateBeamStatus(int index, const QString &status, double cnr, int validGates);
    void updateAlarms(const QVector<AlarmList::AlarmItem> &alarms);
    void updateGateData(const QVector<RangeGateTable::GateData> &data);

signals:
    void beamClicked(int index);
    void alarmClicked(const QString &alarmId);
    void gateClicked(int gateIndex);

private slots:
    void onRefreshTimer();
    void onTimeWindowChanged(const QString &window);

private:
    void setupUI();
    void createMetricCards();
    void createChartArea();
    void createDataTableArea();

    // 指标卡
    MetricCard *m_windSpeedCard;
    MetricCard *m_windDirectionCard;
    MetricCard *m_confidenceCard;
    MetricCard *m_validGatesCard;
    MetricCard *m_blindRatioCard;
    MetricCard *m_alarmCountCard;

    // 图表
    WindTrendChart *m_windTrendChart;
    WindRoseWidget *m_windRoseWidget;
    BeamHealthGrid *m_beamHealthGrid;

    // 数据表格
    AlarmList *m_alarmList;
    RangeGateTable *m_gateTable;

    QTimer *m_refreshTimer;
};

#endif // DASHBOARDPAGE_H
