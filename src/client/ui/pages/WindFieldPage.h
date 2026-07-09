#ifndef WINDFIELDPAGE_H
#define WINDFIELDPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QTableWidget>
#include <QVector>

// 包含头文件（因为使用了嵌套类型）
#include "widgets/RangeGateTable.h"

// 前向声明
class WindTrendChart;

/**
 * @brief 风场页面
 *
 * 显示 30-1000m 分层风廓线、切变、湍流和空间分布
 */
class WindFieldPage : public QWidget
{
    Q_OBJECT

public:
    explicit WindFieldPage(QWidget *parent = nullptr);
    ~WindFieldPage() override;

    // 数据更新
    void updateWindData(double windSpeed, double windDirection, double confidence);
    void updateGateData(const QVector<RangeGateTable::GateData> &data);

private slots:
    void onTimeWindowChanged(const QString &window);
    void onResolutionChanged(const QString &resolution);

private:
    void setupUI();
    void createHeader();
    void createCharts();
    void createTable();

    // 筛选控件
    QComboBox *m_timeWindowCombo;
    QComboBox *m_resolutionCombo;

    // 图表
    WindTrendChart *m_windSpeedChart;
    WindTrendChart *m_windDirectionChart;

    // 表格
    RangeGateTable *m_gateTable;
};

#endif // WINDFIELDPAGE_H
