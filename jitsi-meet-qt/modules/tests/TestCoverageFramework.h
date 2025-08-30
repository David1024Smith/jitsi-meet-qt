#ifndef TEST_COVERAGE_FRAMEWORK_H
#define TEST_COVERAGE_FRAMEWORK_H

#include <QObject>
#include <QTest>
#include <QTimer>
#include <QEventLoop>
#include <QVariantMap>
#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include <QSignalSpy>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QElapsedTimer>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

/**
 * @brief 测试覆盖和质量保证框架
 * 
 * 这个框架负责：
 * - 确保所有模块都有完整的单元测试覆盖
 * - 添加集成测试和端到端测试
 * - 实现自动化测试和持续集成
 * - 创建性能基准测试和回归测试
 * 
 * Requirements: 11.5, 11.6, 12.6
 */
class TestCoverageFramework : public QObject
{
    Q_OBJECT

public:
    explicit TestCoverageFramework(QObject *parent = nullptr);
    ~TestCoverageFramework();

    // 测试类型枚举
    enum TestType {
        UnitTest,
        IntegrationTest,
        EndToEndTest,
        PerformanceTest,
        RegressionTest,
        StressTest,
        SecurityTest
    };

    // 测试状态枚举
    enum TestStatus {
        NotRun,
        Running,
        Passed,
        Failed,
        Skipped,
        Error
    };

    // 覆盖率级别枚举
    enum CoverageLevel {
        None = 0,
        Low = 25,
        Medium = 50,
        Good = 75,
        Excellent = 90,
        Complete = 100
    };

    // 测试结果结构
    struct TestResult {
        QString testName;
        QString moduleName;
        TestType testType;
        TestStatus status;
        QString errorMessage;
        QDateTime startTime;
        QDateTime endTime;
        qint64 executionTime; // milliseconds
        QVariantMap metrics;
        double coveragePercentage;
    };

    // 模块测试覆盖信息
    struct ModuleCoverage {
        QString moduleName;
        int totalLines;
        int coveredLines;
        int totalFunctions;
        int coveredFunctions;
        int totalBranches;
        int coveredBranches;
        double linesCoverage;
        double functionsCoverage;
        double branchesCoverage;
        double overallCoverage;
        QStringList uncoveredFiles;
        QStringList uncoveredFunctions;
    };

    // 性能基准数据
    struct PerformanceBenchmark {
        QString testName;
        QString moduleName;
        QDateTime timestamp;
        double cpuUsage;
        qint64 memoryUsage;
        qint64 executionTime;
        double throughput;
        QVariantMap customMetrics;
    };

    // 回归测试数据
    struct RegressionTestData {
        QString testName;
        QString version;
        QDateTime timestamp;
        QVariantMap baselineMetrics;
        QVariantMap currentMetrics;
        bool hasRegression;
        QStringList regressionDetails;
    };

public slots:
    // 主要测试方法
    void runAllTests();
    void runModuleTests(const QString& moduleName);
    void runTestsByType(TestType testType);
    void generateCoverageReport();
    void generatePerformanceReport();
    void generateRegressionReport();
    
    // 覆盖率分析
    void analyzeCoverage();
    void generateCoverageData();
    void identifyUncoveredCode();
    void suggestAdditionalTests();
    
    // 性能基准测试
    void runPerformanceBenchmarks();
    void comparePerformanceMetrics();
    void detectPerformanceRegressions();
    
    // 自动化测试
    void setupContinuousIntegration();
    void runAutomatedTestSuite();
    void scheduleRegularTests();
    
    // 质量保证
    void validateTestQuality();
    void checkTestMaintainability();
    void analyzeTestEffectiveness();

signals:
    void testStarted(const QString& testName);
    void testCompleted(const TestResult& result);
    void coverageAnalysisCompleted(const ModuleCoverage& coverage);
    void performanceBenchmarkCompleted(const PerformanceBenchmark& benchmark);
    void regressionDetected(const RegressionTestData& regression);
    void allTestsCompleted();

private slots:
    // 内部测试方法
    void runUnitTests();
    void runIntegrationTests();
    void runEndToEndTests();
    void runPerformanceTests();
    void runRegressionTests();
    void runStressTests();
    void runSecurityTests();

private:
    // 测试发现和执行
    QStringList discoverTestFiles();
    QStringList discoverTestClasses();
    bool executeTest(const QString& testClass, const QString& testMethod);
    void executeTestSuite(const QStringList& tests);
    
    // 覆盖率分析方法
    ModuleCoverage analyzeCoverageForModule(const QString& moduleName);
    void generateCoverageDataForFile(const QString& filePath);
    double calculateOverallCoverage(const QList<ModuleCoverage>& coverages);
    void identifyMissingTests(const QString& moduleName);
    
    // 性能测试方法
    PerformanceBenchmark runPerformanceBenchmark(const QString& testName);
    void measureStartupPerformance();
    void measureRuntimePerformance();
    void measureMemoryUsage();
    void measureCPUUsage();
    void measureNetworkPerformance();
    
    // 回归测试方法
    void loadBaselineMetrics();
    void saveCurrentMetrics();
    bool compareMetrics(const QVariantMap& baseline, const QVariantMap& current);
    void detectRegressions();
    
    // 自动化和CI方法
    void setupTestEnvironment();
    void configureTestRunner();
    void setupTestReporting();
    void integrateWithCI();
    
    // 报告生成
    void generateHtmlReport();
    void generateJsonReport();
    void generateXmlReport();
    void generateCoverageHtml();
    void generatePerformanceCharts();
    
    // 工具方法
    QString getModuleTestDirectory(const QString& moduleName);
    QStringList getAvailableModules();
    bool isModuleTestable(const QString& moduleName);
    void logTestResult(const TestResult& result);
    void updateTestStatistics();
    
    // 验证方法
    bool validateTestSetup();
    bool validateCoverageTools();
    bool validatePerformanceTools();
    void checkTestDependencies();

private:
    // 测试状态
    QList<TestResult> m_testResults;
    QList<ModuleCoverage> m_moduleCoverages;
    QList<PerformanceBenchmark> m_performanceBenchmarks;
    QList<RegressionTestData> m_regressionData;
    
    // 配置
    QString m_testOutputDirectory;
    QString m_coverageOutputDirectory;
    QString m_performanceOutputDirectory;
    bool m_enableCoverageAnalysis;
    bool m_enablePerformanceTesting;
    bool m_enableRegressionTesting;
    bool m_enableStressTesting;
    bool m_enableSecurityTesting;
    
    // 阈值设置
    double m_minimumCoverageThreshold;
    double m_performanceRegressionThreshold;
    int m_testTimeoutMs;
    int m_maxConcurrentTests;
    
    // 工具和进程
    QProcess* m_coverageProcess;
    QProcess* m_testProcess;
    QNetworkAccessManager* m_networkManager;
    
    // 同步对象
    QMutex m_testMutex;
    QWaitCondition m_testCondition;
    QEventLoop m_eventLoop;
    
    // 统计数据
    int m_totalTests;
    int m_passedTests;
    int m_failedTests;
    int m_skippedTests;
    double m_overallCoverage;
    QDateTime m_testStartTime;
    QDateTime m_testEndTime;
};

#endif // TEST_COVERAGE_FRAMEWORK_H