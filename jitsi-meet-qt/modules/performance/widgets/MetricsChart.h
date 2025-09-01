#ifndef METRICSCHART_H
#define METRICSCHART_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLegend>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QTimer>
#include <QMutex>
#include "PerformanceManager.h"

// QT_CHARTS_USE_NAMESPACE - Charts module not available, using alternative implementation

/**
 * @brief 性能指标图表组件
 * 
 * MetricsChart提供性能数据的可视化显示：
 * - 实时数据图表
 * - 历史数据趋势
 * - 多种图表类型
 * - 交互式操作
 */
class MetricsChart : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 图表类型枚举
     */
    enum ChartType {
        LineChart,          ///< 折线图
        AreaChart,          ///< 面积图
        BarChart,           ///< 柱状图
        SplineChart,        ///< 样条图
        ScatterChart        ///< 散点图
    };
    Q_ENUM(ChartType)

    /**
     * @brief 时间范围枚举
     */
    enum TimeRange {
        Last1Minute,        ///< 最近1分钟
        Last5Minutes,       ///< 最近5分钟
        Last15Minutes,      ///< 最近15分钟
        Last30Minutes,      ///< 最近30分钟
        Last1Hour,          ///< 最近1小时
        Last6Hours,         ///< 最近6小时
        Last24Hours,        ///< 最近24小时
        CustomRange         ///< 自定义范围
    };
    Q_ENUM(TimeRange)

    /**
     * @brief 指标类型枚举
     */
    enum MetricType {
        CPUUsage,           ///< CPU使用率
        MemoryUsage,        ///< 内存使用率
        NetworkLatency,     ///< 网络延迟
        NetworkBandwidth,   ///< 网络带宽
        AudioLatency,       ///< 音频延迟
        VideoFrameRate,     ///< 视频帧率
        SystemLoad,         ///< 系统负载
        CustomMetric        ///< 自定义指标
    };
    Q_ENUM(MetricType)

    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit MetricsChart(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MetricsChart();

    /**
     * @brief 设置图表类型
     * @param type 图表类型
     */
    void setChartType(ChartType type);

    /**
     * @brief 获取图表类型
     * @return 图表类型
     */
    ChartType chartType() const;

    /**
     * @brief 设置指标类型
     * @param type 指标类型
     */
    void setMetricType(MetricType type);

    /**
     * @brief 获取指标类型
     * @return 指标类型
     */
    MetricType metricType() const;

    /**
     * @brief 设置时间范围
     * @param range 时间范围
     */
    void setTimeRange(TimeRange range);

    /**
     * @brief 获取时间范围
     * @return 时间范围
     */
    TimeRange timeRange() const;

    /**
     * @brief 设置自定义时间范围
     * @param from 开始时间
     * @param to 结束时间
     */
    void setCustomTimeRange(const QDateTime& from, const QDateTime& to);

    /**
     * @brief 获取自定义时间范围
     * @return 时间范围(开始时间, 结束时间)
     */
    QPair<QDateTime, QDateTime> customTimeRange() const;

    /**
     * @brief 设置图表标题
     * @param title 标题
     */
    void setChartTitle(const QString& title);

    /**
     * @brief 获取图表标题
     * @return 标题
     */
    QString chartTitle() const;

    /**
     * @brief 设置Y轴标签
     * @param label Y轴标签
     */
    void setYAxisLabel(const QString& label);

    /**
     * @brief 获取Y轴标签
     * @return Y轴标签
     */
    QString yAxisLabel() const;

    /**
     * @brief 设置最大数据点数
     * @param maxPoints 最大数据点数
     */
    void setMaxDataPoints(int maxPoints);

    /**
     * @brief 获取最大数据点数
     * @return 最大数据点数
     */
    int maxDataPoints() const;

    /**
     * @brief 设置更新间隔
     * @param interval 间隔时间(毫秒)
     */
    void setUpdateInterval(int interval);

    /**
     * @brief 获取更新间隔
     * @return 间隔时间(毫秒)
     */
    int updateInterval() const;

    /**
     * @brief 启动实时更新
     */
    void startRealTimeUpdate();

    /**
     * @brief 停止实时更新
     */
    void stopRealTimeUpdate();

    /**
     * @brief 检查是否正在实时更新
     * @return 是否正在更新
     */
    bool isRealTimeUpdateActive() const;

    /**
     * @brief 添加数据点
     * @param timestamp 时间戳
     * @param value 数值
     */
    void addDataPoint(const QDateTime& timestamp, double value);

    /**
     * @brief 添加多个数据点
     * @param dataPoints 数据点列表(时间戳, 数值)
     */
    void addDataPoints(const QList<QPair<QDateTime, double>>& dataPoints);

    /**
     * @brief 设置数据系列
     * @param dataPoints 数据点列表
     */
    void setDataSeries(const QList<QPair<QDateTime, double>>& dataPoints);

    /**
     * @brief 清除所有数据
     */
    void clearData();

    /**
     * @brief 刷新图表
     */
    void refreshChart();

    /**
     * @brief 导出图表
     * @param filePath 文件路径
     * @param format 导出格式
     * @return 导出是否成功
     */
    bool exportChart(const QString& filePath, const QString& format = "png");

    /**
     * @brief 获取统计信息
     * @return 统计信息
     */
    QVariantMap getStatistics() const;

    /**
     * @brief 设置阈值线
     * @param threshold 阈值
     * @param color 颜色
     */
    void setThresholdLine(double threshold, const QColor& color = Qt::red);

    /**
     * @brief 移除阈值线
     */
    void removeThresholdLine();

    /**
     * @brief 设置图表主题
     * @param theme 主题
     */
    void setChartTheme(QChart::ChartTheme theme);

    /**
     * @brief 获取图表主题
     * @return 主题
     */
    QChart::ChartTheme chartTheme() const;

public slots:
    /**
     * @brief 更新性能指标
     * @param metrics 性能指标
     */
    void updateMetrics(const PerformanceMetrics& metrics);

    /**
     * @brief 缩放到适合大小
     */
    void zoomToFit();

    /**
     * @brief 重置缩放
     */
    void resetZoom();

    /**
     * @brief 放大
     */
    void zoomIn();

    /**
     * @brief 缩小
     */
    void zoomOut();

signals:
    /**
     * @brief 数据点点击信号
     * @param timestamp 时间戳
     * @param value 数值
     */
    void dataPointClicked(const QDateTime& timestamp, double value);

    /**
     * @brief 时间范围改变信号
     * @param range 新的时间范围
     */
    void timeRangeChanged(TimeRange range);

    /**
     * @brief 图表类型改变信号
     * @param type 新的图表类型
     */
    void chartTypeChanged(ChartType type);

    /**
     * @brief 导出请求信号
     * @param filePath 文件路径
     * @param format 导出格式
     */
    void exportRequested(const QString& filePath, const QString& format);

protected:
    /**
     * @brief 重写大小改变事件
     * @param event 大小改变事件
     */
    void resizeEvent(QResizeEvent* event) override;

    /**
     * @brief 重写鼠标按下事件
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief 重写鼠标移动事件
     * @param event 鼠标事件
     */
    void mouseMoveEvent(QMouseEvent* event) override;

    /**
     * @brief 重写滚轮事件
     * @param event 滚轮事件
     */
    void wheelEvent(QWheelEvent* event) override;

private slots:
    /**
     * @brief 处理实时更新
     */
    void handleRealTimeUpdate();

    /**
     * @brief 处理时间范围改变
     * @param index 选择索引
     */
    void handleTimeRangeChanged(int index);

    /**
     * @brief 处理图表类型改变
     * @param index 选择索引
     */
    void handleChartTypeChanged(int index);

    /**
     * @brief 处理最大数据点数改变
     * @param maxPoints 最大数据点数
     */
    void handleMaxDataPointsChanged(int maxPoints);

private:
    /**
     * @brief 初始化界面
     */
    void initializeUI();

    /**
     * @brief 创建图表
     */
    void createChart();

    /**
     * @brief 创建控制面板
     * @return 控制面板组件
     */
    QWidget* createControlPanel();

    /**
     * @brief 更新图表数据
     */
    void updateChartData();

    /**
     * @brief 配置坐标轴
     */
    void configureAxes();

    /**
     * @brief 应用图表类型
     */
    void applyChartType();

    /**
     * @brief 应用时间范围
     */
    void applyTimeRange();

    /**
     * @brief 获取指标值
     * @param metrics 性能指标
     * @return 指标值
     */
    double getMetricValue(const PerformanceMetrics& metrics) const;

    /**
     * @brief 获取指标单位
     * @return 指标单位
     */
    QString getMetricUnit() const;

    /**
     * @brief 获取时间范围毫秒数
     * @param range 时间范围
     * @return 毫秒数
     */
    qint64 getTimeRangeMilliseconds(TimeRange range) const;

    /**
     * @brief 过滤数据点
     * @param from 开始时间
     * @param to 结束时间
     * @return 过滤后的数据点
     */
    QList<QPair<QDateTime, double>> filterDataPoints(const QDateTime& from, const QDateTime& to) const;

    /**
     * @brief 计算统计信息
     * @return 统计信息
     */
    QVariantMap calculateStatistics() const;

    ChartType m_chartType;                          ///< 图表类型
    MetricType m_metricType;                        ///< 指标类型
    TimeRange m_timeRange;                          ///< 时间范围
    QPair<QDateTime, QDateTime> m_customTimeRange;  ///< 自定义时间范围
    
    // UI组件
    QVBoxLayout* m_mainLayout;                      ///< 主布局
    QHBoxLayout* m_controlLayout;                   ///< 控制布局
    QChart* m_chart;                                ///< 图表对象
    QChartView* m_chartView;                        ///< 图表视图
    
    // 控制组件
    QComboBox* m_timeRangeCombo;                    ///< 时间范围选择
    QComboBox* m_chartTypeCombo;                    ///< 图表类型选择
    QSpinBox* m_maxPointsSpinBox;                   ///< 最大数据点数选择
    QPushButton* m_zoomFitButton;                   ///< 缩放适合按钮
    QPushButton* m_exportButton;                    ///< 导出按钮
    
    // 图表组件
    QLineSeries* m_lineSeries;                      ///< 折线系列
    QAreaSeries* m_areaSeries;                      ///< 面积系列
    QBarSeries* m_barSeries;                        ///< 柱状系列
    QLineSeries* m_thresholdSeries;                 ///< 阈值线系列
    
    // 坐标轴
    QDateTimeAxis* m_xAxis;                         ///< X轴(时间)
    QValueAxis* m_yAxis;                            ///< Y轴(数值)
    
    // 数据
    QList<QPair<QDateTime, double>> m_dataPoints;   ///< 数据点列表
    int m_maxDataPoints;                            ///< 最大数据点数
    double m_thresholdValue;                        ///< 阈值
    bool m_hasThreshold;                            ///< 是否有阈值
    
    // 定时器
    QTimer* m_updateTimer;                          ///< 更新定时器
    
    // 状态变量
    bool m_realTimeUpdateActive;                    ///< 实时更新是否活跃
    int m_updateInterval;                           ///< 更新间隔
    
    mutable QMutex m_mutex;                         ///< 线程安全互斥锁
};

#endif // METRICSCHART_H