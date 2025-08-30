#ifndef COMPREHENSIVE_FUNCTIONAL_VALIDATOR_H
#define COMPREHENSIVE_FUNCTIONAL_VALIDATOR_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>
#include <QVariantMap>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QNetworkAccessManager>
#include <QNetworkReply>

/**
 * @brief 全面功能验证器
 * 
 * 负责验证所有原有功能在新架构下正常工作，包括：
 * - 原有功能完整性验证
 * - 性能对比测试和优化调整
 * - 压力测试和稳定性测试
 * - 跨平台兼容性验证
 * - 部署流程验证
 * 
 * Requirements: 11.5, 11.6
 */
class ComprehensiveFunctionalValidator : public QObject
{
    Q_OBJECT

public:
    explicit ComprehensiveFunctionalValidator(QObject *parent = nullptr);
    ~ComprehensiveFunctionalValidator();

    // 验证类型枚举
    enum ValidationType {
        FunctionalValidation,
        PerformanceComparison,
        StressTest,
        StabilityTest,
        CrossPlatformTest,
        DeploymentTest,
        RegressionTest,
        SecurityTest
    };

    // 测试结果状态
    enum TestStatus {
        NotStarted,
        Running,
        Passed,
        Failed,
        Skipped,
        Timeout
    };

    // 功能验证结果
    struct ValidationResult {
        QString testName;
        ValidationType type;
        TestStatus status;
        QDateTime startTime;
        QDateTime endTime;
        qint64 executionTimeMs;
        double performanceScore;
        QVariantMap metrics;
        QString errorMessage;
        QStringList warnings;
        QVariantMap additionalData;
    };

    // 性能对比结果
    struct PerformanceComparison {
        QString functionality;
        double oldArchitectureTime;
        double newArchitectureTime;
        double improvementPercentage;
        bool isImprovement;
        QString description;
        QVariantMap detailedMetrics;
    };

    // 压力测试配置
    struct StressTestConfig {
        int concurrentUsers;
        int testDurationMinutes;
        int rampUpTimeSeconds;
        QVariantMap loadPatterns;
        QStringList targetFunctions;
    };

public slots:
    // 主要验证方法
    void runComprehensiveValidation();
    void runFunctionalValidation();
    void runPerformanceComparison();
    void runStressTests();
    void runStabilityTests();
    void runCrossPlatformTests();
    void runDeploymentTests();
    
    // 配置方法
    void setValidationConfig(const QVariantMap& config);
    void setStressTestConfig(const StressTestConfig& config);
    void setPerformanceBaselines(const QVariantMap& baselines);
    void enableContinuousValidation(bool enabled);

signals:
    void validationStarted(ValidationType type);
    void validationCompleted(const ValidationResult& result);
    void performanceComparisonCompleted(const PerformanceComparison& comparison);
    void stressTestProgress(int percentage, const QString& status);
    void stabilityTestUpdate(const QString& status, const QVariantMap& metrics);
    void allValidationsCompleted(bool allPassed);
    void criticalIssueDetected(const QString& issue, const QVariantMap& details);

private slots:
    void onValidationTimer();
    void onStressTestUpdate();
    void onStabilityMonitorUpdate();

private:
    // 核心功能验证
    ValidationResult validateAudioFunctionality();
    ValidationResult validateVideoFunctionality();
    ValidationResult validateNetworkFunctionality();
    ValidationResult validateChatFunctionality();
    ValidationResult validateScreenSharingFunctionality();
    ValidationResult validateMeetingFunctionality();
    ValidationResult validateUIFunctionality();
    ValidationResult validateSettingsFunctionality();
    ValidationResult validatePerformanceFunctionality();
    ValidationResult validateUtilsFunctionality();
    
    // 原有功能对比验证
    ValidationResult compareWithLegacyImplementation(const QString& functionality);
    bool verifyFeatureParity(const QString& feature);
    bool validateAPICompatibility(const QString& module);
    bool checkDataMigration(const QString& dataType);
    
    // 性能对比测试
    PerformanceComparison compareStartupPerformance();
    PerformanceComparison compareMemoryUsage();
    PerformanceComparison compareCPUUsage();
    PerformanceComparison compareNetworkLatency();
    PerformanceComparison compareRenderingPerformance();
    PerformanceComparison compareAudioLatency();
    PerformanceComparison compareVideoQuality();
    
    // 压力测试方法
    ValidationResult runConcurrentUserStressTest();
    ValidationResult runMemoryStressTest();
    ValidationResult runCPUStressTest();
    ValidationResult runNetworkStressTest();
    ValidationResult runLongRunningStressTest();
    ValidationResult runResourceExhaustionTest();
    
    // 稳定性测试方法
    ValidationResult runLongTermStabilityTest();
    ValidationResult runMemoryLeakTest();
    ValidationResult runResourceCleanupTest();
    ValidationResult runErrorRecoveryTest();
    ValidationResult runFailoverTest();
    
    // 跨平台兼容性测试
    ValidationResult validateWindowsCompatibility();
    ValidationResult validateLinuxCompatibility();
    ValidationResult validateMacOSCompatibility();
    ValidationResult validateDifferentQtVersions();
    ValidationResult validateDifferentCompilers();
    
    // 部署流程验证
    ValidationResult validateBuildProcess();
    ValidationResult validatePackaging();
    ValidationResult validateInstallation();
    ValidationResult validateUpgrade();
    ValidationResult validateUninstallation();
    ValidationResult validateConfiguration();
    
    // 工作流验证
    ValidationResult validateCompleteWorkflow();
    ValidationResult validateMeetingJoinWorkflow();
    ValidationResult validateAudioVideoWorkflow();
    ValidationResult validateChatWorkflow();
    ValidationResult validateScreenShareWorkflow();
    ValidationResult validateSettingsWorkflow();
    
    // 性能监控和分析
    void startPerformanceMonitoring();
    void stopPerformanceMonitoring();
    QVariantMap collectPerformanceMetrics();
    void analyzePerformanceTrends();
    void detectPerformanceRegressions();
    
    // 资源监控
    double getCurrentCPUUsage();
    qint64 getCurrentMemoryUsage();
    double getCurrentNetworkUsage();
    qint64 getCurrentDiskUsage();
    int getCurrentThreadCount();
    int getCurrentHandleCount();
    
    // 测试环境管理
    bool setupTestEnvironment();
    void cleanupTestEnvironment();
    bool prepareTestData();
    void resetSystemState();
    bool validateTestPreconditions();
    
    // 模拟用户操作
    bool simulateUserLogin();
    bool simulateMeetingJoin(const QString& meetingUrl);
    bool simulateAudioToggle();
    bool simulateVideoToggle();
    bool simulateChatMessage(const QString& message);
    bool simulateScreenShare();
    bool simulateSettingsChange();
    
    // 数据验证
    bool validateDataIntegrity();
    bool validateConfigurationPersistence();
    bool validateLogOutput();
    bool validateErrorHandling();
    
    // 报告生成
    void generateValidationReport();
    void generatePerformanceReport();
    void generateStressTestReport();
    void generateStabilityReport();
    void generateComparisonReport();
    
    // 工具方法
    QString generateTestId();
    void logValidationResult(const ValidationResult& result);
    void updateValidationProgress(int percentage);
    bool isTestEnvironmentReady();
    void notifyStakeholders(const QString& message);

private:
    // 验证状态
    QList<ValidationResult> m_validationResults;
    QList<PerformanceComparison> m_performanceComparisons;
    QVariantMap m_performanceBaselines;
    QVariantMap m_validationConfig;
    StressTestConfig m_stressTestConfig;
    
    // 监控组件
    QTimer* m_validationTimer;
    QTimer* m_stressTestTimer;
    QTimer* m_stabilityTimer;
    QElapsedTimer m_testTimer;
    
    // 性能监控数据
    QList<QVariantMap> m_performanceHistory;
    QVariantMap m_currentMetrics;
    QVariantMap m_baselineMetrics;
    
    // 测试进程管理
    QList<QProcess*> m_testProcesses;
    QNetworkAccessManager* m_networkManager;
    
    // 配置
    QString m_configFilePath;
    QString m_reportsDirectory;
    QString m_testDataDirectory;
    QString m_logFilePath;
    
    // 验证阈值
    double m_performanceRegressionThreshold; // percentage
    double m_memoryLeakThreshold; // MB
    int m_maxResponseTimeMs;
    double m_minSuccessRate; // percentage
    int m_maxCrashCount;
    
    // 当前状态
    bool m_validationRunning;
    bool m_continuousValidationEnabled;
    QString m_currentTest;
    int m_totalTests;
    int m_completedTests;
    int m_passedTests;
    int m_failedTests;
    
    // 同步对象
    QMutex m_validationMutex;
    QWaitCondition m_validationCondition;
    
    // 平台特定配置
    QString m_currentPlatform;
    QStringList m_supportedPlatforms;
    QVariantMap m_platformSpecificConfig;
};

#endif // COMPREHENSIVE_FUNCTIONAL_VALIDATOR_H