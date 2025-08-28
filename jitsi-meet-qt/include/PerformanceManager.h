#ifndef PERFORMANCEMANAGER_H
#define PERFORMANCEMANAGER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QHash>
#include <QMutex>
#include <QThread>
#include <memory>
class ConfigurationManager;
class PerformanceConfig;

/**
 * @brief 性能管理器 - 负责应用程序性能优化和监控
 */
class PerformanceManager : public QObject
{
    Q_OBJECT

public:
    struct PerformanceMetrics {
        qint64 startupTime = 0;
        qint64 memoryUsage = 0;
        qint64 networkMemory = 0;
        int recentItemsCount = 0;
        qint64 configLoadTime = 0;
        qint64 resourceLoadTime = 0;
    };

    explicit PerformanceManager(QObject *parent = nullptr);
    ~PerformanceManager();

    static PerformanceManager* instance();

    // 启动时间优化
    void startStartupTimer();
    void markStartupComplete();
    qint64 getStartupTime() const;

    // 资源加载优化
    void preloadResources();
    void optimizeResourceLoading();

    // 网络内存管理
    void optimizeNetworkMemory();

    // 历史记录性能优化
    void optimizeRecentItemsLoading();
    void setMaxRecentItems(int maxItems);

    // 内存监控和清理
    void startMemoryMonitoring();
    void stopMemoryMonitoring();
    void performMemoryCleanup();
    qint64 getCurrentMemoryUsage();

    // 性能指标
    PerformanceMetrics getMetrics() const;
    void logPerformanceMetrics();

    // 配置管理
    PerformanceConfig* performanceConfig() const;
    void applyPerformanceConfiguration();

signals:
    void memoryWarning(qint64 memoryUsage);
    void performanceMetricsUpdated(const PerformanceMetrics& metrics);

private slots:
    void onMemoryCheckTimer();
    void onCleanupTimer();
    void onConfigurationChanged();

private:
    void initializeOptimizations();
    void setupMemoryThresholds();
    void cleanupUnusedResources();
    qint64 getProcessMemoryUsage();
    qint64 getNetworkMemoryUsage();

    static PerformanceManager* s_instance;
    
    QElapsedTimer m_startupTimer;
    QTimer* m_memoryCheckTimer;
    QTimer* m_cleanupTimer;
    PerformanceConfig* m_performanceConfig;
    
    PerformanceMetrics m_metrics;
    QMutex m_metricsMutex;
    
    // 内存管理配置
    qint64 m_memoryWarningThreshold;
    qint64 m_memoryCriticalThreshold;
    int m_maxRecentItems;
    
    // 网络优化
    bool m_networkOptimized;
    
    // 资源预加载
    QHash<QString, QByteArray> m_preloadedResources;
    bool m_resourcesPreloaded;
};

#endif // PERFORMANCEMANAGER_H