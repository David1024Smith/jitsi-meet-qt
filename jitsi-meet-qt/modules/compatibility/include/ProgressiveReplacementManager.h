#ifndef PROGRESSIVEREPLACEMENTMANAGER_H
#define PROGRESSIVEREPLACEMENTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QHash>
#include <QMutex>
#include <QTimer>
#include <QDateTime>

class LegacyCompatibilityAdapter;
class RollbackManager;
class PerformanceValidator;
class FunctionValidator;

/**
 * @brief 渐进式代码替换管理器
 * 
 * 提供安全的代码替换策略和执行计划，支持新旧代码并行运行、
 * 功能对比验证和性能基准测试。
 */
class ProgressiveReplacementManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 替换策略枚举
     */
    enum ReplacementStrategy {
        Conservative,    // 保守策略：逐步替换，充分验证
        Balanced,        // 平衡策略：适度并行，定期验证
        Aggressive,      // 激进策略：快速替换，最小验证
        Custom          // 自定义策略
    };
    Q_ENUM(ReplacementStrategy)

    /**
     * @brief 替换阶段枚举
     */
    enum ReplacementPhase {
        Planning,       // 规划阶段
        Preparation,    // 准备阶段
        Execution,      // 执行阶段
        Validation,     // 验证阶段
        Completion,     // 完成阶段
        Rollback        // 回滚阶段
    };
    Q_ENUM(ReplacementPhase)

    /**
     * @brief 替换状态枚举
     */
    enum ReplacementStatus {
        NotStarted,     // 未开始
        InProgress,     // 进行中
        Paused,         // 已暂停
        Completed,      // 已完成
        Failed,         // 失败
        RolledBack      // 已回滚
    };
    Q_ENUM(ReplacementStatus)

    /**
     * @brief 代码运行模式枚举
     */
    enum CodeRunMode {
        LegacyOnly,     // 仅运行旧代码
        NewOnly,        // 仅运行新代码
        Parallel,       // 并行运行
        Comparison      // 对比运行
    };
    Q_ENUM(CodeRunMode)

    /**
     * @brief 替换计划结构
     */
    struct ReplacementPlan {
        QString moduleName;
        ReplacementStrategy strategy;
        QStringList dependencies;
        QVariantMap configuration;
        QDateTime scheduledStart;
        QDateTime estimatedCompletion;
        int priority;
        bool requiresValidation;
        bool requiresPerformanceTest;
    };

    /**
     * @brief 替换执行状态结构
     */
    struct ExecutionState {
        QString moduleName;
        ReplacementPhase currentPhase;
        ReplacementStatus status;
        CodeRunMode runMode;
        int progressPercentage;
        QDateTime startTime;
        QDateTime lastUpdate;
        QStringList completedSteps;
        QStringList pendingSteps;
        QVariantMap metrics;
    };

    explicit ProgressiveReplacementManager(QObject *parent = nullptr);
    ~ProgressiveReplacementManager();

    // 初始化和配置
    bool initialize();
    bool isInitialized() const;
    void setGlobalStrategy(ReplacementStrategy strategy);
    ReplacementStrategy globalStrategy() const;

    // 替换计划管理
    bool createReplacementPlan(const QString& moduleName, const ReplacementPlan& plan);
    bool updateReplacementPlan(const QString& moduleName, const ReplacementPlan& plan);
    bool deleteReplacementPlan(const QString& moduleName);
    ReplacementPlan getReplacementPlan(const QString& moduleName) const;
    QStringList getPlannedModules() const;

    // 替换执行控制
    bool startReplacement(const QString& moduleName);
    bool pauseReplacement(const QString& moduleName);
    bool resumeReplacement(const QString& moduleName);
    bool stopReplacement(const QString& moduleName);
    bool rollbackReplacement(const QString& moduleName);

    // 并行运行管理
    bool enableParallelMode(const QString& moduleName);
    bool disableParallelMode(const QString& moduleName);
    bool setCodeRunMode(const QString& moduleName, CodeRunMode mode);
    CodeRunMode getCodeRunMode(const QString& moduleName) const;

    // 状态查询
    ExecutionState getExecutionState(const QString& moduleName) const;
    QStringList getActiveReplacements() const;
    QStringList getCompletedReplacements() const;
    QStringList getFailedReplacements() const;

    // 验证和测试
    bool runFunctionalComparison(const QString& moduleName);
    bool runPerformanceBenchmark(const QString& moduleName);
    QVariantMap getComparisonResults(const QString& moduleName) const;
    QVariantMap getPerformanceResults(const QString& moduleName) const;

    // 安全控制
    bool createSafetyCheckpoint(const QString& moduleName);
    bool validateSafetyConditions(const QString& moduleName);
    bool executeSafeSwitch(const QString& moduleName);
    bool emergencyRollback(const QString& moduleName);

    // 报告和监控
    QVariantMap generateProgressReport() const;
    QVariantMap generateDetailedReport(const QString& moduleName) const;
    QStringList getReplacementHistory() const;

public slots:
    void scheduleReplacement(const QString& moduleName, const QDateTime& scheduledTime);
    void batchReplacement(const QStringList& moduleNames);
    void cleanupCompletedReplacements();

private slots:
    void onScheduledReplacementTriggered();
    void onValidationCompleted(const QString& moduleName, bool success);
    void onPerformanceTestCompleted(const QString& moduleName, const QVariantMap& results);
    void onRollbackCompleted(const QString& moduleName, bool success);
    void onParallelModeStatusChanged(const QString& moduleName, bool enabled);

signals:
    void replacementStarted(const QString& moduleName);
    void replacementCompleted(const QString& moduleName, bool success);
    void replacementProgress(const QString& moduleName, int percentage);
    void replacementPaused(const QString& moduleName);
    void replacementResumed(const QString& moduleName);
    void replacementFailed(const QString& moduleName, const QString& error);
    void rollbackInitiated(const QString& moduleName);
    void rollbackCompleted(const QString& moduleName, bool success);
    void validationFailed(const QString& moduleName, const QString& reason);
    void performanceIssueDetected(const QString& moduleName, const QVariantMap& metrics);
    void safetyCheckFailed(const QString& moduleName, const QString& issue);

private:
    // 内部执行方法
    bool executeReplacementStep(const QString& moduleName, const QString& step);
    bool validateReplacementStep(const QString& moduleName, const QString& step);
    bool prepareParallelExecution(const QString& moduleName);
    bool switchToNewImplementation(const QString& moduleName);
    bool cleanupLegacyCode(const QString& moduleName);

    // 策略实现
    QStringList getStrategySteps(ReplacementStrategy strategy) const;
    QVariantMap getStrategyConfiguration(ReplacementStrategy strategy) const;
    bool validateStrategyCompatibility(const QString& moduleName, ReplacementStrategy strategy) const;

    // 安全检查
    bool performSafetyChecks(const QString& moduleName) const;
    bool checkDependencies(const QString& moduleName) const;
    bool checkResourceAvailability(const QString& moduleName) const;
    bool checkSystemStability() const;

    // 工具方法
    QString generateExecutionId(const QString& moduleName) const;
    void updateExecutionState(const QString& moduleName, const ExecutionState& state);
    void logReplacementEvent(const QString& moduleName, const QString& event, const QVariantMap& data = QVariantMap());

    bool m_initialized;
    ReplacementStrategy m_globalStrategy;
    
    QHash<QString, ReplacementPlan> m_replacementPlans;
    QHash<QString, ExecutionState> m_executionStates;
    QHash<QString, QVariantMap> m_comparisonResults;
    QHash<QString, QVariantMap> m_performanceResults;
    QStringList m_replacementHistory;
    
    LegacyCompatibilityAdapter* m_compatibilityAdapter;
    RollbackManager* m_rollbackManager;
    PerformanceValidator* m_performanceValidator;
    FunctionValidator* m_functionValidator;
    
    QTimer* m_schedulerTimer;
    QHash<QString, QDateTime> m_scheduledReplacements;
    
    mutable QMutex m_mutex;
};

#endif // PROGRESSIVEREPLACEMENTMANAGER_H