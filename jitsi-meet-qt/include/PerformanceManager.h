#ifndef PERFORMANCEMANAGER_H
#define PERFORMANCEMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantMap>
#include <QList>
#include <QMutex>
#include "modules/performance/include/PerformanceMetrics.h"
#include "modules/performance/include/OptimizationType.h"
#include "modules/performance/include/IOptimizer.h"

// 前向声明
class MetricsCollector;
class IOptimizer;

/**
 * @brief 性能管理器
 * 
 * 负责监控和优化应用程序性能
 */
class PerformanceManager : public QObject
{
    Q_OBJECT

public:
    explicit PerformanceManager(QObject* parent = nullptr);
    virtual ~PerformanceManager();

    // 初始化和控制方法
    bool initialize();
    bool start();
    void stop();
    bool isRunning() const;

    // 配置方法
    void setConfig(QObject* config);
    void setMetricsCollector(MetricsCollector* collector);

    // 性能监控方法
    bool startMonitoring();
    void stopMonitoring();
    
    // 系统信息获取
    QVariantMap getSystemInfo() const;
    
    // 性能数据获取
    virtual double getCpuUsage() const;
    virtual double getMemoryUsage() const;
    virtual PerformanceMetrics getCurrentMetrics() const;
    virtual PerformanceLevel getCurrentPerformanceLevel() const;

    // 优化策略设置
    void setOptimizationStrategy(OptimizationStrategy strategy);
    OptimizationStrategy getOptimizationStrategy() const;
    
    // 自动优化控制
    void setAutoOptimizationEnabled(bool enabled);
    bool isAutoOptimizationEnabled() const;
    
    // 监控控制
    virtual bool isMonitoringActive() const;
    
    // 优化操作
    virtual bool performOptimization();

signals:
    /**
     * @brief 性能数据更新信号
     * @param cpuUsage CPU使用率
     * @param memoryUsage 内存使用量
     */
    void performanceDataUpdated(double cpuUsage, double memoryUsage);
    
    /**
     * @brief 指标更新信号
     * @param metrics 性能指标
     */
    void metricsUpdated(const PerformanceMetrics& metrics);
    
    /**
     * @brief 性能级别变化信号
     * @param level 新的性能级别
     */
    void performanceLevelChanged(PerformanceLevel level);
    
    /**
     * @brief 阈值超出信号
     * @param metric 超出阈值的指标名称
     * @param value 当前值
     * @param threshold 阈值
     */
    void thresholdExceeded(const QString& metric, double value, double threshold);
    
    /**
     * @brief 优化完成信号
     * @param result 优化结果
     */
    void optimizationCompleted(const OptimizationResult& result);

    /**
     * @brief 优化完成信号（兼容版本）
     * @param success 是否成功
     * @param improvements 改进信息
     */
    void optimizationCompleted(bool success, const QVariantMap& improvements);
    
    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    void updatePerformanceData();
    void updateMetrics();
    void performAutoOptimization();

private:
    // 基础成员
    QTimer* m_updateTimer;
    bool m_monitoring;
    
    // 扩展成员 (与源文件中的构造函数匹配)
    QObject* m_config;
    MetricsCollector* m_metricsCollector;
    QTimer* m_monitoringTimer;
    QTimer* m_optimizationTimer;
    bool m_isRunning;
    bool m_autoOptimizationEnabled;
    OptimizationStrategy m_optimizationStrategy;
    PerformanceLevel m_currentLevel;
    
    // 监控器和优化器列表
    QList<QObject*> m_monitors;
    QList<IOptimizer*> m_optimizers;
    
    // 互斥锁
    mutable QMutex m_mutex;
};

#endif // PERFORMANCEMANAGER_H