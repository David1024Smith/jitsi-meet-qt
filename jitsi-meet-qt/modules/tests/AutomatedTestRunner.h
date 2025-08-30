#ifndef AUTOMATED_TEST_RUNNER_H
#define AUTOMATED_TEST_RUNNER_H

#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QFileSystemWatcher>
#include <QDateTime>
#include <QStringList>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "TestCoverageFramework.h"

/**
 * @brief 自动化测试运行器
 * 
 * 负责：
 * - 自动化测试执行和调度
 * - 持续集成支持
 * - 测试结果通知和报告
 * - 回归检测和警报
 * 
 * Requirements: 11.5, 11.6, 12.6
 */
class AutomatedTestRunner : public QObject
{
    Q_OBJECT

public:
    explicit AutomatedTestRunner(QObject *parent = nullptr);
    ~AutomatedTestRunner();

    // 调度模式枚举
    enum ScheduleMode {
        Manual,
        OnFileChange,
        Periodic,
        OnCommit,
        OnBuild
    };

    // 通知类型枚举
    enum NotificationType {
        TestStarted,
        TestCompleted,
        TestFailed,
        CoverageAlert,
        RegressionDetected,
        BuildBroken
    };

    // CI集成类型
    enum CIProvider {
        None,
        Jenkins,
        GitHubActions,
        GitLabCI,
        AzureDevOps,
        TeamCity
    };

public slots:
    // 主要控制方法
    void startAutomatedTesting();
    void stopAutomatedTesting();
    void runTestsNow();
    void scheduleTests(ScheduleMode mode, int intervalMinutes = 60);
    
    // 配置方法
    void setTestSchedule(const QStringList& testTypes, const QString& cronExpression);
    void enableFileWatching(const QStringList& watchPaths);
    void configureCIIntegration(CIProvider provider, const QVariantMap& config);
    void setNotificationSettings(const QVariantMap& settings);
    
    // 报告和通知
    void generateScheduledReports();
    void sendNotification(NotificationType type, const QString& message);
    void publishTestResults();
    void updateDashboard();

signals:
    void automatedTestStarted();
    void automatedTestCompleted(bool success);
    void testScheduleUpdated();
    void notificationSent(NotificationType type, const QString& message);
    void ciIntegrationConfigured();

private slots:
    // 内部调度方法
    void onScheduledTestTrigger();
    void onFileChanged(const QString& path);
    void onTestCompleted();
    void onCoverageThresholdExceeded();
    void onRegressionDetected();
    
    // CI集成回调
    void onCIWebhookReceived();
    void onBuildStatusChanged();
    void onCommitReceived();

private:
    // 测试执行方法
    void executeScheduledTests();
    void runTestSuite(const QStringList& testTypes);
    void validateTestEnvironment();
    void prepareTestData();
    void cleanupTestArtifacts();
    
    // 调度管理
    void setupTestSchedule();
    void updateSchedule();
    void checkScheduleTriggers();
    bool shouldRunTests();
    
    // 文件监控
    void setupFileWatching();
    void addWatchPath(const QString& path);
    void removeWatchPath(const QString& path);
    void handleFileChange(const QString& filePath);
    
    // CI集成方法
    void setupJenkinsIntegration();
    void setupGitHubActionsIntegration();
    void setupGitLabCIIntegration();
    void setupAzureDevOpsIntegration();
    void setupTeamCityIntegration();
    void sendCIStatus(const QString& status, const QString& description);
    
    // 通知系统
    void setupNotifications();
    void sendEmailNotification(const QString& subject, const QString& body);
    void sendSlackNotification(const QString& message);
    void sendWebhookNotification(const QVariantMap& data);
    void logNotification(const QString& message);
    
    // 报告生成
    void generateDailyReport();
    void generateWeeklyReport();
    void generateTrendAnalysis();
    void updateMetricsDashboard();
    
    // 配置管理
    void loadConfiguration();
    void saveConfiguration();
    void validateConfiguration();
    QVariantMap getDefaultConfiguration();
    
    // 工具方法
    QString formatTestResults(const QList<TestCoverageFramework::TestResult>& results);
    QString generateSummaryReport();
    bool isBusinessHours();
    QString getCurrentBranch();
    QString getLastCommitHash();

private:
    // 核心组件
    TestCoverageFramework* m_testFramework;
    QTimer* m_scheduleTimer;
    QFileSystemWatcher* m_fileWatcher;
    QNetworkAccessManager* m_networkManager;
    
    // 调度配置
    ScheduleMode m_scheduleMode;
    int m_scheduleInterval; // minutes
    QString m_cronExpression;
    QDateTime m_lastRunTime;
    QDateTime m_nextRunTime;
    QStringList m_scheduledTestTypes;
    
    // 文件监控
    QStringList m_watchPaths;
    QStringList m_watchedFiles;
    bool m_fileWatchingEnabled;
    int m_fileChangeDebounceMs;
    
    // CI集成
    CIProvider m_ciProvider;
    QVariantMap m_ciConfig;
    QString m_ciWebhookUrl;
    QString m_ciApiToken;
    bool m_ciIntegrationEnabled;
    
    // 通知配置
    QVariantMap m_notificationConfig;
    bool m_emailNotificationsEnabled;
    bool m_slackNotificationsEnabled;
    bool m_webhookNotificationsEnabled;
    QString m_emailRecipients;
    QString m_slackWebhookUrl;
    QString m_notificationWebhookUrl;
    
    // 状态跟踪
    bool m_isRunning;
    bool m_testsInProgress;
    QDateTime m_currentRunStartTime;
    int m_consecutiveFailures;
    double m_lastCoveragePercentage;
    
    // 配置文件路径
    QString m_configFilePath;
    QString m_logFilePath;
    QString m_reportsDirectory;
    
    // 阈值和限制
    double m_coverageThreshold;
    int m_maxConsecutiveFailures;
    int m_testTimeoutMinutes;
    bool m_runOnlyOnBusinessHours;
};

#endif // AUTOMATED_TEST_RUNNER_H