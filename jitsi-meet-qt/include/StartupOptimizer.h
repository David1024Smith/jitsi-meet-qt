#ifndef STARTUPOPTIMIZER_H
#define STARTUPOPTIMIZER_H

#include <QObject>
#include <QElapsedTimer>
#include <QHash>
#include <QStringList>

/**
 * @brief 启动优化器 - 优化应用程序启动时间和资源加载
 */
class StartupOptimizer : public QObject
{
    Q_OBJECT

public:
    enum OptimizationLevel {
        Basic,      // 基本优化
        Moderate,   // 中等优化
        Aggressive  // 激进优化
    };

    explicit StartupOptimizer(QObject *parent = nullptr);
    ~StartupOptimizer();

    // 启动优化
    void setOptimizationLevel(OptimizationLevel level);
    void enableFastStartup();
    void preloadCriticalResources();
    void deferNonCriticalInitialization();

    // 资源管理
    void preloadStyleSheets();
    void preloadTranslations();
    void preloadIcons();
    void optimizeResourceLoading();

    // 延迟初始化
    void scheduleDelayedInitialization(const QString& component, std::function<void()> initFunc);
    void executeDelayedInitializations();

    // 性能监控
    void startPhaseTimer(const QString& phase);
    void endPhaseTimer(const QString& phase);
    qint64 getPhaseTime(const QString& phase) const;
    void logStartupMetrics();

    // 配置
    void setPreloadEnabled(bool enabled);
    void setDeferredInitEnabled(bool enabled);
    void setCriticalResourcePaths(const QStringList& paths);

signals:
    void startupPhaseCompleted(const QString& phase, qint64 duration);
    void allResourcesPreloaded();
    void delayedInitializationCompleted();

private:
    void initializeBasicOptimizations();
    void initializeModerateOptimizations();
    void initializeAggressiveOptimizations();
    void loadResourceInBackground(const QString& path);

    OptimizationLevel m_optimizationLevel;
    bool m_preloadEnabled;
    bool m_deferredInitEnabled;
    
    // 计时器
    QHash<QString, QElapsedTimer> m_phaseTimers;
    QHash<QString, qint64> m_phaseTimes;
    
    // 延迟初始化
    QHash<QString, std::function<void()>> m_delayedInitializations;
    
    // 资源管理
    QStringList m_criticalResourcePaths;
    QHash<QString, QByteArray> m_preloadedResources;
    
    // 状态跟踪
    bool m_fastStartupEnabled;
    bool m_resourcesPreloaded;
    int m_preloadProgress;
};

#endif // STARTUPOPTIMIZER_H