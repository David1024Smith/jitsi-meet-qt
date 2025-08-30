#ifndef MODULEPERFORMANCEINTEGRATOR_H
#define MODULEPERFORMANCEINTEGRATOR_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include "ModuleCommunicationBus.h"
#include "ModuleResourceManager.h"
#include "ModuleStartupOptimizer.h"

/**
 * @brief 模块性能集成器 - 统一管理所有性能优化组件
 * 
 * 整合并协调：
 * - 通信总线优化
 * - 资源管理优化
 * - 启动优化
 * - 性能监控和调优
 */
class ModulePerformanceIntegrator : public QObject
{
    Q_OBJECT

public:
    struct SystemPerformanceMetrics {
        // 通信性能
        ModuleCommunicationBus::PerformanceMetrics communicationMetrics;
        
        // 资源管理性能
        ModuleResourceManager::CacheStatistics resourceMetrics;
        
        // 启动性能
        ModuleStartupOptimizer::StartupMetrics startupMetrics;
        
        // 系统整体性能
        qint64 totalMemoryUsage;
        qint64 peakMemoryUsage;
        double cpuUsage;
        qint64 totalMessages;
        qint64 totalResources;
        int activeModules;
        
        // 性能评分 (0-100)
        int performanceScore;
        QString performanceLevel;  // "Excellent", "Good", "Fair", "Poor"
        
        SystemPerformanceMetrics() : totalMemoryUsage(0), peakMemoryUsage(0), 
                                   cpuUsage(0.0), totalMessages(0), totalResources(0),
                                   activeModules(0), performanceScore(0) {}
    };

    struct OptimizationRecommendation {
        QString category;
        QString issue;
        QString recommendation;
        QString action;
        int priority;  // 1-5, 5 is highest
        bool autoApplicable;
        
        OptimizationRecommendation() : priority(1), autoApplicable(false) {}
    };

    static ModulePerformanceIntegrator* instance();
    ~ModulePerformanceIntegrator();

    // 系统初始化和控制
    void initialize();
    void shutdown();
    void startOptimization();
    void stopOptimization();
    void pauseOptimization();
    void resumeOptimization();

    // 性能监控
    SystemPerformanceMetrics getSystemMetrics() const;
    QList<OptimizationRecommendation> getOptimizationRecommendations() const;
    void updatePerformanceMetrics();

    // 自动优化控制
    void enableAutoOptimization(bool enabled);
    bool isAutoOptimizationEnabled() const;
    void setOptimizationInterval(int seconds);
    void setPerformanceThresholds(int memoryThreshold, double cpuThreshold);

    // 手动优化操作
    void optimizeMemoryUsage();
    void optimizeCommunication();
    void optimizeStartupPerformance();
    void optimizeResourceUsage();
    void performFullOptimization();

    // 性能配置
    void applyPerformanceProfile(const QString& profileName);
    void saveCurrentProfile(const QString& profileName);
    QStringList getAvailableProfiles() const;

    // 组件访问器
    ModuleCommunicationBus* getCommunicationBus() const;
    ModuleResourceManager* getResourceManager() const;
    ModuleStartupOptimizer* getStartupOptimizer() const;

signals:
    void performanceMetricsUpdated(const ModulePerformanceIntegrator::SystemPerformanceMetrics& metrics);
    void optimizationRecommendationAvailable(const ModulePerformanceIntegrator::OptimizationRecommendation& recommendation);
    void optimizationCompleted(const QString& category, const QString& result);
    void performanceAlert(const QString& alert, int severity);
    void autoOptimizationTriggered(const QString& reason);

private slots:
    void performPeriodicOptimization();
    void onCommunicationPerformanceAlert(const QString& alert);
    void onResourceMemoryWarning(qint64 currentUsage, qint64 maxUsage);
    void checkPerformanceThresholds();

private:
    explicit ModulePerformanceIntegrator(QObject* parent = nullptr);
    
    void initializeComponents();
    void connectSignals();
    void loadPerformanceProfiles();
    void savePerformanceProfiles();
    
    int calculatePerformanceScore(const SystemPerformanceMetrics& metrics) const;
    QString getPerformanceLevel(int score) const;
    void generateOptimizationRecommendations();
    
    void applyMemoryOptimizations();
    void applyCommunicationOptimizations();
    void applyStartupOptimizations();
    void applyResourceOptimizations();
    
    bool shouldTriggerAutoOptimization() const;
    void executeAutoOptimization();

    static ModulePerformanceIntegrator* s_instance;
    static QMutex s_mutex;

    // 核心组件
    ModuleCommunicationBus* m_communicationBus;
    ModuleResourceManager* m_resourceManager;
    ModuleStartupOptimizer* m_startupOptimizer;

    // 性能监控
    SystemPerformanceMetrics m_currentMetrics;
    QList<OptimizationRecommendation> m_recommendations;
    mutable QMutex m_metricsLock;

    // 自动优化配置
    bool m_autoOptimizationEnabled;
    int m_optimizationInterval;
    int m_memoryThreshold;
    double m_cpuThreshold;
    
    // 定时器和控制
    QTimer* m_optimizationTimer;
    QTimer* m_metricsTimer;
    bool m_optimizationActive;
    bool m_optimizationPaused;

    // 性能配置文件
    QHash<QString, QVariantMap> m_performanceProfiles;
    QString m_currentProfileName;
};

#endif // MODULEPERFORMANCEINTEGRATOR_H