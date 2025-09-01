#ifndef PERFORMANCEWIDGET_H
#define PERFORMANCEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTabWidget>
#include <QGroupBox>
#include <QTimer>
#include <QMutex>
#include "PerformanceManager.h"

class MetricsChart;
class MonitorWidget;
class PerformanceConfig;

/**
 * @brief 性能监控主界面组件
 * 
 * PerformanceWidget提供完整的性能监控用户界面：
 * - 实时性能指标显示
 * - 性能图表展示
 * - 优化控制面板
 * - 配置设置界面
 */
class PerformanceWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 显示模式枚举
     */
    enum DisplayMode {
        CompactMode,        ///< 紧凑模式
        DetailedMode,       ///< 详细模式
        DashboardMode,      ///< 仪表板模式
        MinimalMode         ///< 最小模式
    };
    Q_ENUM(DisplayMode)

    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit PerformanceWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~PerformanceWidget();

    /**
     * @brief 设置性能管理器
     * @param manager 性能管理器
     */
    void setPerformanceManager(PerformanceManager* manager);

    /**
     * @brief 获取性能管理器
     * @return 性能管理器
     */
    PerformanceManager* performanceManager() const;

    /**
     * @brief 设置配置对象
     * @param config 配置对象
     */
    void setConfig(PerformanceConfig* config);

    /**
     * @brief 获取配置对象
     * @return 配置对象
     */
    PerformanceConfig* config() const;

    /**
     * @brief 设置显示模式
     * @param mode 显示模式
     */
    void setDisplayMode(DisplayMode mode);

    /**
     * @brief 获取显示模式
     * @return 显示模式
     */
    DisplayMode displayMode() const;

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
     * @brief 刷新显示
     */
    void refreshDisplay();

    /**
     * @brief 重置界面
     */
    void resetInterface();

    /**
     * @brief 导出性能数据
     * @param filePath 文件路径
     * @param format 导出格式
     * @return 导出是否成功
     */
    bool exportPerformanceData(const QString& filePath, const QString& format = "json");

    /**
     * @brief 获取界面状态
     * @return 界面状态
     */
    QVariantMap getInterfaceState() const;

    /**
     * @brief 设置界面状态
     * @param state 界面状态
     */
    void setInterfaceState(const QVariantMap& state);

public slots:
    /**
     * @brief 更新性能指标显示
     * @param metrics 性能指标
     */
    void updateMetrics(const PerformanceMetrics& metrics);

    /**
     * @brief 更新性能等级显示
     * @param level 性能等级
     */
    void updatePerformanceLevel(PerformanceLevel level);

    /**
     * @brief 显示阈值超出警告
     * @param metricName 指标名称
     * @param value 当前值
     * @param threshold 阈值
     */
    void showThresholdWarning(const QString& metricName, double value, double threshold);

    /**
     * @brief 显示优化完成结果
     * @param success 优化是否成功
     * @param improvements 改进信息
     */
    void showOptimizationResult(bool success, const QVariantMap& improvements);

    /**
     * @brief 执行手动优化
     */
    void performManualOptimization();

    /**
     * @brief 打开配置对话框
     */
    void openConfigurationDialog();

    /**
     * @brief 切换监控状态
     */
    void toggleMonitoring();

    /**
     * @brief 切换自动优化
     */
    void toggleAutoOptimization();

signals:
    /**
     * @brief 监控状态改变信号
     * @param enabled 是否启用监控
     */
    void monitoringToggled(bool enabled);

    /**
     * @brief 自动优化状态改变信号
     * @param enabled 是否启用自动优化
     */
    void autoOptimizationToggled(bool enabled);

    /**
     * @brief 配置改变信号
     * @param config 新配置
     */
    void configurationChanged(const QVariantMap& config);

    /**
     * @brief 导出请求信号
     * @param filePath 文件路径
     * @param format 导出格式
     */
    void exportRequested(const QString& filePath, const QString& format);

protected:
    /**
     * @brief 重写绘制事件
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief 重写大小改变事件
     * @param event 大小改变事件
     */
    void resizeEvent(QResizeEvent* event) override;

    /**
     * @brief 重写显示事件
     * @param event 显示事件
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief 重写隐藏事件
     * @param event 隐藏事件
     */
    void hideEvent(QHideEvent* event) override;

private slots:
    /**
     * @brief 处理实时更新
     */
    void handleRealTimeUpdate();

    /**
     * @brief 处理标签页切换
     * @param index 标签页索引
     */
    void handleTabChanged(int index);

    /**
     * @brief 处理监控器状态改变
     * @param monitorName 监控器名称
     * @param status 新状态
     */
    void handleMonitorStatusChanged(const QString& monitorName, const QString& status);

private:
    /**
     * @brief 初始化界面
     */
    void initializeUI();

    /**
     * @brief 创建概览标签页
     * @return 概览组件
     */
    QWidget* createOverviewTab();

    /**
     * @brief 创建详细信息标签页
     * @return 详细信息组件
     */
    QWidget* createDetailsTab();

    /**
     * @brief 创建图表标签页
     * @return 图表组件
     */
    QWidget* createChartsTab();

    /**
     * @brief 创建优化标签页
     * @return 优化组件
     */
    QWidget* createOptimizationTab();

    /**
     * @brief 创建设置标签页
     * @return 设置组件
     */
    QWidget* createSettingsTab();

    /**
     * @brief 创建系统信息组
     * @return 系统信息组件
     */
    QGroupBox* createSystemInfoGroup();

    /**
     * @brief 创建性能指标组
     * @return 性能指标组件
     */
    QGroupBox* createMetricsGroup();

    /**
     * @brief 创建控制面板组
     * @return 控制面板组件
     */
    QGroupBox* createControlPanelGroup();

    /**
     * @brief 创建状态栏
     * @return 状态栏组件
     */
    QWidget* createStatusBar();

    /**
     * @brief 更新系统信息显示
     */
    void updateSystemInfo();

    /**
     * @brief 更新指标显示
     * @param metrics 性能指标
     */
    void updateMetricsDisplay(const PerformanceMetrics& metrics);

    /**
     * @brief 更新图表显示
     * @param metrics 性能指标
     */
    void updateChartsDisplay(const PerformanceMetrics& metrics);

    /**
     * @brief 更新状态显示
     */
    void updateStatusDisplay();

    /**
     * @brief 应用显示模式
     */
    void applyDisplayMode();

    /**
     * @brief 格式化字节大小
     * @param bytes 字节数
     * @return 格式化字符串
     */
    QString formatBytes(qint64 bytes) const;

    /**
     * @brief 格式化百分比
     * @param percentage 百分比
     * @return 格式化字符串
     */
    QString formatPercentage(double percentage) const;

    /**
     * @brief 格式化时间
     * @param milliseconds 毫秒数
     * @return 格式化字符串
     */
    QString formatTime(double milliseconds) const;

    /**
     * @brief 获取性能等级颜色
     * @param level 性能等级
     * @return 颜色
     */
    QColor getPerformanceLevelColor(PerformanceLevel level) const;

    PerformanceManager* m_performanceManager;       ///< 性能管理器
    PerformanceConfig* m_config;                   ///< 配置对象
    DisplayMode m_displayMode;                     ///< 显示模式
    
    // UI组件
    QTabWidget* m_tabWidget;                       ///< 标签页组件
    QVBoxLayout* m_mainLayout;                     ///< 主布局
    
    // 概览标签页组件
    QLabel* m_cpuUsageLabel;                       ///< CPU使用率标签
    QLabel* m_memoryUsageLabel;                    ///< 内存使用率标签
    QLabel* m_networkLatencyLabel;                 ///< 网络延迟标签
    QLabel* m_performanceLevelLabel;               ///< 性能等级标签
    QProgressBar* m_cpuProgressBar;                ///< CPU进度条
    QProgressBar* m_memoryProgressBar;             ///< 内存进度条
    QProgressBar* m_networkProgressBar;            ///< 网络进度条
    
    // 详细信息标签页组件
    QLabel* m_systemInfoLabel;                     ///< 系统信息标签
    QLabel* m_audioMetricsLabel;                   ///< 音频指标标签
    QLabel* m_videoMetricsLabel;                   ///< 视频指标标签
    QLabel* m_networkMetricsLabel;                 ///< 网络指标标签
    
    // 图表标签页组件
    MetricsChart* m_cpuChart;                      ///< CPU图表
    MetricsChart* m_memoryChart;                   ///< 内存图表
    MetricsChart* m_networkChart;                  ///< 网络图表
    
    // 优化标签页组件
    QPushButton* m_optimizeButton;                 ///< 优化按钮
    QPushButton* m_autoOptimizeButton;             ///< 自动优化按钮
    QLabel* m_optimizationStatusLabel;             ///< 优化状态标签
    QLabel* m_lastOptimizationLabel;               ///< 上次优化标签
    
    // 设置标签页组件
    QPushButton* m_configButton;                   ///< 配置按钮
    QPushButton* m_exportButton;                   ///< 导出按钮
    QPushButton* m_resetButton;                    ///< 重置按钮
    
    // 控制组件
    QPushButton* m_startStopButton;                ///< 启动/停止按钮
    QLabel* m_statusLabel;                         ///< 状态标签
    
    // 定时器
    QTimer* m_updateTimer;                         ///< 更新定时器
    
    // 状态变量
    bool m_realTimeUpdateActive;                   ///< 实时更新是否活跃
    int m_updateInterval;                          ///< 更新间隔
    PerformanceMetrics m_lastMetrics;              ///< 上次指标
    
    mutable QMutex m_mutex;                        ///< 线程安全互斥锁
};

#endif // PERFORMANCEWIDGET_H