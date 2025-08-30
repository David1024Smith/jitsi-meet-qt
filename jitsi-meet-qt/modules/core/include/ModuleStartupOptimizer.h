#ifndef MODULESTARTUPOPTIMIZER_H
#define MODULESTARTUPOPTIMIZER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QReadWriteLock>
#include <QHash>
#include <QQueue>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QElapsedTimer>
#include <QFuture>
#include <QFutureWatcher>

/**
 * @brief 模块启动优化器 - 优化模块启动时间和内存使用
 * 
 * 提供智能的模块启动优化，支持：
 * - 并行模块加载
 * - 延迟初始化
 * - 预加载策略
 * - 启动时间分析
 * - 内存使用优化
 * - 依赖关系优化
 */
class ModuleStartupOptimizer : public QObject
{
    Q_OBJECT

public:
    enum LoadStrategy {
        Immediate,      // 立即加载
        Lazy,          // 延迟加载
        Preload,       // 预加载
        OnDemand,      // 按需加载
        Background,    // 后台加载
        Parallel       // 并行加载
    };

    enum OptimizationLevel {
        None,          // 无优化
        Basic,         // 基础优化
        Aggressive,    // 激进优化
        Adaptive       // 自适应优化
    };

    struct ModuleLoadInfo {
        QString moduleName;
        LoadStrategy strategy;
        int priority;
        QStringList dependencies;
        QStringList optionalDependencies;
        qint64 estimatedLoadTime;
        qint64 estimatedMemoryUsage;
        bool criticalModule;
        bool preloadEnabled;
        QVariantMap metadata;
        
        ModuleLoadInfo() : strategy(Immediate), priority(0), estimatedLoadTime(0), 
                          estimatedMemoryUsage(0), criticalModule(false), preloadEnabled(false) {}
    };

    struct LoadSession {
        QString sessionId;
        QStringList modulesToLoad;
        qint64 startTime;
        qint64 endTime;
        qint64 totalLoadTime;
        qint64 parallelLoadTime;
        int successCount;
        int failureCount;
        QHash<QString, qint64> moduleLoadTimes;
        QHash<QString, qint64> moduleMemoryUsage;
        QString errorMessage;
        
        LoadSession() : startTime(0), endTime(0), totalLoadTime(0), parallelLoadTime(0),
                       successCount(0), failureCount(0) {}
    };

    struct PerformanceProfile {
        QString profileName;
        QHash<QString, ModuleLoadInfo> moduleConfigs;
        OptimizationLevel optimizationLevel;
        int maxParallelLoads;
        qint64 preloadDelay;
        qint64 lazyLoadTimeout;
        bool memoryOptimizationEnabled;
        bool dependencyOptimizationEnabled;
        
        PerformanceProfile() : optimizationLevel(Basic), maxParallelLoads(4), 
                              preloadDelay(1000), lazyLoadTimeout(30000),
                              memoryOptimizationEnabled(true), dependencyOptimizationEnabled(true) {}
    };

    struct StartupMetrics {
        qint64 totalStartupTime;
        qint64 moduleLoadTime;
        qint64 dependencyResolutionTime;
        qint64 initializationTime;
        qint64 peakMemoryUsage;
        qint64 finalMemoryUsage;
        int totalModules;
        int loadedModules;
        int failedModules;
        int parallelLoadCount;
        double averageLoadTime;
        double loadTimeVariance;
        
        StartupMetrics() : totalStartupTime(0), moduleLoadTime(0), dependencyResolutionTime(0),
                          initializationTime(0), peakMemoryUsage(0), finalMemoryUsage(0),
                          totalModules(0), loadedModules(0), failedModules(0), parallelLoadCount(0),
                          averageLoadTime(0.0), loadTimeVariance(0.0) {}
    };

    static ModuleStartupOptimizer* instance();
    ~ModuleStartupOptimizer();

    // 配置管理
    void setPerformanceProfile(const PerformanceProfile& profile);
    PerformanceProfile getPerformanceProfile() const;
    void setOptimizationLevel(OptimizationLevel level);
    OptimizationLevel getOptimizationLevel() const;

    // 模块配置
    void setModuleLoadInfo(const QString& moduleName, const ModuleLoadInfo& info);
    ModuleLoadInfo getModuleLoadInfo(const QString& moduleName) const;
    void setModuleLoadStrategy(const QString& moduleName, LoadStrategy strategy);
    void setModulePriority(const QString& moduleName, int priority);
    void setModuleDependencies(const QString& moduleName, const QStringList& dependencies);

    // 启动控制
    QString startLoadSession(const QStringList& modules);
    bool stopLoadSession(const QString& sessionId);
    bool pauseLoadSession(const QString& sessionId);
    bool resumeLoadSession(const QString& sessionId);
    LoadSession getLoadSession(const QString& sessionId) const;

    // 优化策略
    void enableParallelLoading(bool enabled, int maxParallel = 4);
    void enableLazyLoading(bool enabled, qint64 timeoutMs = 30000);
    void enablePreloading(bool enabled, qint64 delayMs = 1000);
    void enableMemoryOptimization(bool enabled);
    void enableDependencyOptimization(bool enabled);

    // 预加载管理
    void schedulePreload(const QString& moduleName, qint64 delayMs = 0);
    void cancelPreload(const QString& moduleName);
    void preloadCriticalModules();
    QStringList getPreloadQueue() const;

    // 延迟加载管理
    void scheduleLazyLoad(const QString& moduleName, qint64 timeoutMs = 0);
    void triggerLazyLoad(const QString& moduleName);
    QStringList getLazyLoadQueue() const;

    // 依赖关系优化
    QStringList optimizeLoadOrder(const QStringList& modules) const;
    QList<QStringList> createLoadBatches(const QStringList& modules) const;
    bool validateDependencies(const QStringList& modules) const;
    QStringList resolveDependencies(const QString& moduleName) const;

    // 性能分析
    StartupMetrics getStartupMetrics() const;
    QHash<QString, qint64> getModuleLoadTimes() const;
    QHash<QString, qint64> getModuleMemoryUsage() const;
    void resetMetrics();

    // 系统控制
    void initialize();
    void shutdown();
    void optimizeForNextStartup();
    void saveOptimizationData();
    void loadOptimizationData();

signals:
    void moduleLoadStarted(const QString& moduleName, const QString& sessionId);
    void moduleLoadCompleted(const QString& moduleName, const QString& sessionId, qint64 loadTime);
    void moduleLoadFailed(const QString& moduleName, const QString& sessionId, const QString& error);
    void loadSessionStarted(const QString& sessionId, const QStringList& modules);
    void loadSessionCompleted(const QString& sessionId, const ModuleStartupOptimizer::LoadSession& session);
    void loadSessionFailed(const QString& sessionId, const QString& error);
    void preloadScheduled(const QString& moduleName, qint64 delay);
    void preloadCompleted(const QString& moduleName);
    void lazyLoadTriggered(const QString& moduleName);
    void optimizationCompleted(const QString& summary);

private slots:
    void processPreloadQueue();
    void processLazyLoadQueue();
    void onModuleLoadFinished();
    void updateMetrics();

private:
    explicit ModuleStartupOptimizer(QObject* parent = nullptr);
    
    void initializeSystem();
    void shutdownSystem();
    
    void loadModuleAsync(const QString& moduleName, const QString& sessionId);
    void loadModuleBatch(const QStringList& modules, const QString& sessionId);
    void processLoadQueue();
    
    bool canLoadInParallel(const QString& moduleName, const QStringList& currentlyLoading) const;
    QStringList getReadyToLoadModules(const QStringList& remaining, const QStringList& loaded) const;
    void updateLoadSession(const QString& sessionId, const QString& moduleName, bool success, qint64 loadTime);
    
    void analyzeLoadPerformance();
    void optimizeLoadStrategies();
    void updateEstimatedTimes();
    
    qint64 estimateLoadTime(const QString& moduleName) const;
    qint64 estimateMemoryUsage(const QString& moduleName) const;
    void recordActualMetrics(const QString& moduleName, qint64 loadTime, qint64 memoryUsage);

    static ModuleStartupOptimizer* s_instance;
    static QMutex s_mutex;

    // 配置数据
    PerformanceProfile m_currentProfile;
    QHash<QString, ModuleLoadInfo> m_moduleConfigs;
    mutable QReadWriteLock m_configLock;

    // 加载会话管理
    QHash<QString, LoadSession> m_loadSessions;
    mutable QReadWriteLock m_sessionLock;

    // 队列管理
    QQueue<QString> m_preloadQueue;
    QQueue<QString> m_lazyLoadQueue;
    QHash<QString, qint64> m_preloadSchedule;  // moduleName -> scheduledTime
    QHash<QString, qint64> m_lazyLoadSchedule;
    mutable QMutex m_queueLock;

    // 性能数据
    StartupMetrics m_metrics;
    QHash<QString, qint64> m_moduleLoadTimes;
    QHash<QString, qint64> m_moduleMemoryUsage;
    QHash<QString, QList<qint64>> m_loadTimeHistory;  // 历史加载时间
    mutable QMutex m_metricsLock;

    // 系统组件
    QTimer* m_preloadTimer;
    QTimer* m_lazyLoadTimer;
    QTimer* m_metricsTimer;
    QThreadPool* m_threadPool;
    
    // 状态管理
    bool m_parallelLoadingEnabled;
    bool m_lazyLoadingEnabled;
    bool m_preloadingEnabled;
    bool m_memoryOptimizationEnabled;
    bool m_dependencyOptimizationEnabled;
    
    int m_maxParallelLoads;
    qint64 m_preloadDelay;
    qint64 m_lazyLoadTimeout;
    
    // 运行时状态
    QStringList m_currentlyLoading;
    QElapsedTimer m_startupTimer;
    bool m_optimizationInProgress;
};

/**
 * @brief 异步模块加载任务
 */
class AsyncModuleLoadTask : public QRunnable
{
public:
    AsyncModuleLoadTask(const QString& moduleName, const QString& sessionId, 
                       ModuleStartupOptimizer* optimizer);
    void run() override;

private:
    QString m_moduleName;
    QString m_sessionId;
    QPointer<ModuleStartupOptimizer> m_optimizer;
};

/**
 * @brief 批量模块加载任务
 */
class BatchModuleLoadTask : public QRunnable
{
public:
    BatchModuleLoadTask(const QStringList& modules, const QString& sessionId,
                       ModuleStartupOptimizer* optimizer);
    void run() override;

private:
    QStringList m_modules;
    QString m_sessionId;
    QPointer<ModuleStartupOptimizer> m_optimizer;
};

#endif // MODULESTARTUPOPTIMIZER_H