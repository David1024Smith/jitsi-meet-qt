#ifndef PERFORMANCEMANAGER_H
#define PERFORMANCEMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QDateTime>
#include <QVariantMap>
#include <QSharedPointer>
#include <QSize>
#include "IPerformanceMonitor.h"
#include "IResourceTracker.h"

class MetricsCollector;
class PerformanceConfig;
class BaseMonitor;
class BaseOptimizer;

#include "PerformanceMetrics.h"
#include "OptimizationType.h"

/**
 * @brief 性能管理器类
 * 
 * PerformanceManager负责高级性能管理功能：
 * - 协调各种性能监控器
 * - 管理性能优化器
 * - 提供性能数据分析和报告
 * - 处理性能阈值和告警
 */
class PerformanceManager : public QObject, public IPerformanceMonitor
{
    Q_OBJECT
    Q_INTERFACES(IPerformanceMonitor)

public:
    /**
     * @brief 性能等级枚举
     */
    enum PerformanceLevel {
        Excellent = 5,      ///< 优秀
        Good = 4,           ///< 良好
        Fair = 3,           ///< 一般
        Poor = 2,           ///< 较差
        Critical = 1        ///< 严重
    };
    Q_ENUM(PerformanceLevel)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PerformanceManager(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~PerformanceManager();

    // IPerformanceMonitor接口实现
    bool initialize() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;
    PerformanceMetrics getCurrentMetrics() const override;
    QList<PerformanceMetrics> getHistoricalMetrics(const QDateTime& from, const QDateTime& to) const override;

    /**
     * @brief 设置配置
     * @param config 配置对象
     */
    void setConfig(PerformanceConfig* config);
    
    /**
     * @brief 设置配置（QObject*重载版本）
     * @param config 配置对象
     */
    void setConfig(QObject* config);

    /**
     * @brief 获取配置
     * @return 配置对象
     */
    PerformanceConfig* config() const;

    /**
     * @brief 设置指标收集器
     * @param collector 指标收集器
     */
    void setMetricsCollector(MetricsCollector* collector);

    /**
     * @brief 获取指标收集器
     * @return 指标收集器
     */
    MetricsCollector* metricsCollector() const;

    /**
     * @brief 添加监控器
     * @param monitor 监控器对象
     * @return 添加是否成功
     */
    bool addMonitor(BaseMonitor* monitor);

    /**
     * @brief 移除监控器
     * @param monitorName 监控器名称
     * @return 移除是否成功
     */
    bool removeMonitor(const QString& monitorName);

    /**
     * @brief 获取监控器
     * @param monitorName 监控器名称
     * @return 监控器对象
     */
    BaseMonitor* getMonitor(const QString& monitorName) const;

    /**
     * @brief 获取监控器名称
     * @return 监控器名称
     */
    QString getMonitorName() const override;

    /**
     * @brief 获取版本信息
     * @return 版本字符串
     */
    QString getVersion() const override;

    /**
     * @brief 获取状态信息
     * @return 状态映射
     */
    QVariantMap getStatus() const override;

    /**
     * @brief 重置监控器
     */
    void reset() override;

    /**
     * @brief 获取所有监控器
     * @return 监控器列表
     */
    QList<BaseMonitor*> getAllMonitors() const;

    /**
     * @brief 添加优化器
     * @param optimizer 优化器对象
     * @return 添加是否成功
     */
    bool addOptimizer(BaseOptimizer* optimizer);

    /**
     * @brief 移除优化器
     * @param optimizerName 优化器名称
     * @return 移除是否成功
     */
    bool removeOptimizer(const QString& optimizerName);

    /**
     * @brief 获取优化器
     * @param optimizerName 优化器名称
     * @return 优化器对象
     */
    BaseOptimizer* getOptimizer(const QString& optimizerName) const;

    /**
     * @brief 获取所有优化器
     * @return 优化器列表
     */
    QList<BaseOptimizer*> getAllOptimizers() const;

    /**
     * @brief 设置监控间隔
     * @param interval 间隔时间(毫秒)
     */
    void setMonitoringInterval(int interval);

    /**
     * @brief 获取监控间隔
     * @return 间隔时间(毫秒)
     */
    int monitoringInterval() const;

    /**
     * @brief 检查监控是否活跃
     * @return 是否活跃
     */
    bool isMonitoringActive() const;

    /**
     * @brief 开始监控
     * @return 启动是否成功
     */
    bool startMonitoring();

    /**
     * @brief 停止监控
     */
    void stopMonitoring();

    /**
     * @brief 启用自动优化
     * @param enabled 是否启用
     */
    void setAutoOptimizationEnabled(bool enabled);

    /**
     * @brief 检查是否启用自动优化
     * @return 是否启用
     */
    bool isAutoOptimizationEnabled() const;

    /**
     * @brief 设置优化策略
     * @param strategy 优化策略
     */
    void setOptimizationStrategy(OptimizationStrategy strategy);

    /**
     * @brief 获取优化策略
     * @return 优化策略
     */
    OptimizationStrategy optimizationStrategy() const;

    /**
     * @brief 获取当前性能等级
     * @return 性能等级
     */
    PerformanceLevel getCurrentPerformanceLevel() const;

    /**
     * @brief 获取性能评分
     * @return 性能评分(0-100)
     */
    int getPerformanceScore() const;

    /**
     * @brief 执行手动优化
     * @return 优化是否成功
     */
    bool performOptimization();

    /**
     * @brief 生成性能报告
     * @return 性能报告
     */
    QVariantMap generatePerformanceReport() const;

    /**
     * @brief 导出性能数据
     * @param filePath 文件路径
     * @param format 导出格式
     * @return 导出是否成功
     */
    bool exportPerformanceData(const QString& filePath, const QString& format = "json") const;

    /**
     * @brief 清除历史数据
     * @param olderThan 清除指定时间之前的数据
     */
    void clearHistoricalData(const QDateTime& olderThan = QDateTime());

    /**
     * @brief 获取系统信息
     * @return 系统信息
     */
    QVariantMap getSystemInfo() const;

signals:
    /**
     * @brief 性能指标更新信号
     * @param metrics 性能指标
     */
    void metricsUpdated(const PerformanceMetrics& metrics);

    /**
     * @brief 性能等级改变信号
     * @param level 新的性能等级
     */
    void performanceLevelChanged(PerformanceLevel level);

    /**
     * @brief 性能阈值超出信号
     * @param metricName 指标名称
     * @param value 当前值
     * @param threshold 阈值
     */
    void thresholdExceeded(const QString& metricName, double value, double threshold);

    /**
     * @brief 优化完成信号
     * @param success 优化是否成功
     * @param improvements 改进信息
     */
    void optimizationCompleted(bool success, const QVariantMap& improvements);

    /**
     * @brief 监控器状态改变信号
     * @param monitorName 监控器名称
     * @param status 新状态
     */
    void monitorStatusChanged(const QString& monitorName, const QString& status);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief 更新性能指标
     */
    void updateMetrics();

    /**
     * @brief 检查性能阈值
     */
    void checkThresholds();

    /**
     * @brief 执行自动优化
     */
    void performAutoOptimization();

    /**
     * @brief 处理监控器错误
     * @param error 错误信息
     */
    void handleMonitorError(const QString& error);

private:
    /**
     * @brief 初始化默认监控器
     */
    void initializeDefaultMonitors();

    /**
     * @brief 初始化默认优化器
     */
    void initializeDefaultOptimizers();

    /**
     * @brief 计算性能等级
     * @param metrics 性能指标
     * @return 性能等级
     */
    PerformanceLevel calculatePerformanceLevel(const PerformanceMetrics& metrics) const;

    /**
     * @brief 计算性能评分
     * @param metrics 性能指标
     * @return 性能评分
     */
    int calculatePerformanceScore(const PerformanceMetrics& metrics) const;

    /**
     * @brief 检查是否需要优化
     * @param metrics 性能指标
     * @return 是否需要优化
     */
    bool shouldOptimize(const PerformanceMetrics& metrics) const;

    PerformanceConfig* m_config;                    ///< 配置对象
    MetricsCollector* m_metricsCollector;          ///< 指标收集器
    QTimer* m_monitoringTimer;                     ///< 监控定时器
    QTimer* m_optimizationTimer;                   ///< 优化定时器
    
    QMap<QString, BaseMonitor*> m_monitors;        ///< 监控器映射
    QMap<QString, BaseOptimizer*> m_optimizers;    ///< 优化器映射
    
    bool m_isRunning;                              ///< 是否正在运行
    bool m_autoOptimizationEnabled;                ///< 是否启用自动优化
    OptimizationStrategy m_optimizationStrategy;   ///< 优化策略
    PerformanceLevel m_currentLevel;               ///< 当前性能等级
    
    mutable QMutex m_mutex;                        ///< 线程安全互斥锁
};

#endif // PERFORMANCEMANAGER_H