#ifndef PERFORMANCE_BENCHMARK_SUITE_H
#define PERFORMANCE_BENCHMARK_SUITE_H

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

/**
 * @brief 性能基准测试套件
 * 
 * 负责：
 * - 创建性能基准测试和回归测试
 * - 监控系统资源使用情况
 * - 检测性能回归和优化机会
 * - 生成性能趋势报告
 * 
 * Requirements: 11.5, 11.6, 12.6
 */
class PerformanceBenchmarkSuite : public QObject
{
    Q_OBJECT

public:
    explicit PerformanceBenchmarkSuite(QObject *parent = nullptr);
    ~PerformanceBenchmarkSuite();

    // 基准测试类型
    enum BenchmarkType {
        StartupTime,
        ModuleLoadTime,
        MemoryUsage,
        CPUUsage,
        NetworkLatency,
        RenderingPerformance,
        AudioProcessing,
        VideoProcessing,
        DatabaseOperations,
        FileIO
    };

    // 性能指标
    struct PerformanceMetric {
        QString name;
        QString unit;
        double value;
        double baseline;
        double threshold;
        QDateTime timestamp;
        QVariantMap metadata;
    };

    // 基准测试结果
    struct BenchmarkResult {
        QString testName;
        BenchmarkType type;
        QDateTime startTime;
        QDateTime endTime;
        qint64 executionTime;
        QList<PerformanceMetric> metrics;
        bool passed;
        QString errorMessage;
        QVariantMap additionalData;
    };

    // 回归检测结果
    struct RegressionResult {
        QString testName;
        QString version;
        QDateTime timestamp;
        double regressionPercentage;
        bool isRegression;
        QString description;
        QList<PerformanceMetric> affectedMetrics;
    };

public slots:
    // 主要测试方法
    void runAllBenchmarks();
    void runBenchmarksByType(BenchmarkType type);
    void runSpecificBenchmark(const QString& benchmarkName);
    void runRegressionTests();
    
    // 基准管理
    void setBaseline(const QString& testName, const QVariantMap& baselineMetrics);
    void updateBaselines();
    void loadBaselines();
    void saveBaselines();
    
    // 配置方法
    void setPerformanceThresholds(const QVariantMap& thresholds);
    void enableContinuousMonitoring(bool enabled);
    void setMonitoringInterval(int seconds);
    void configureBenchmarkSettings(const QVariantMap& settings);

signals:
    void benchmarkStarted(const QString& testName);
    void benchmarkCompleted(const BenchmarkResult& result);
    void regressionDetected(const RegressionResult& regression);
    void performanceImprovement(const QString& testName, double improvementPercentage);
    void allBenchmarksCompleted();

private slots:
    void onMonitoringTimer();
    void onBenchmarkProcessFinished();
    void onResourceMonitoringUpdate();

private:
    // 核心基准测试
    BenchmarkResult runStartupBenchmark();
    BenchmarkResult runModuleLoadBenchmark();
    BenchmarkResult runMemoryUsageBenchmark();
    BenchmarkResult runCPUUsageBenchmark();
    BenchmarkResult runNetworkLatencyBenchmark();
    BenchmarkResult runRenderingBenchmark();
    BenchmarkResult runAudioProcessingBenchmark();
    BenchmarkResult runVideoProcessingBenchmark();
    BenchmarkResult runDatabaseBenchmark();
    BenchmarkResult runFileIOBenchmark();
    
    // 系统监控方法
    void startResourceMonitoring();
    void stopResourceMonitoring();
    double getCurrentCPUUsage();
    qint64 getCurrentMemoryUsage();
    double getCurrentNetworkUsage();
    double getCurrentDiskUsage();
    
    // 模块特定基准测试
    BenchmarkResult benchmarkAudioModule();
    BenchmarkResult benchmarkVideoModule();
    BenchmarkResult benchmarkNetworkModule();
    BenchmarkResult benchmarkUIModule();
    BenchmarkResult benchmarkChatModule();
    BenchmarkResult benchmarkScreenShareModule();
    BenchmarkResult benchmarkMeetingModule();
    BenchmarkResult benchmarkSettingsModule();
    BenchmarkResult benchmarkUtilsModule();
    BenchmarkResult benchmarkPerformanceModule();
    
    // 回归检测方法
    void detectRegressions();
    bool isRegression(const PerformanceMetric& current, const PerformanceMetric& baseline);
    double calculateRegressionPercentage(double current, double baseline);
    void analyzePerformanceTrends();
    
    // 数据分析方法
    void calculateStatistics();
    void generatePerformanceReport();
    void updatePerformanceTrends();
    void identifyBottlenecks();
    void suggestOptimizations();
    
    // 工具方法
    void warmupSystem();
    void cleanupBenchmarkEnvironment();
    void prepareTestData();
    QString generateBenchmarkId();
    void logBenchmarkResult(const BenchmarkResult& result);
    
    // 配置和持久化
    void loadConfiguration();
    void saveConfiguration();
    void loadHistoricalData();
    void saveHistoricalData();
    
    // 报告生成
    void generateHtmlReport();
    void generateJsonReport();
    void generateCsvReport();
    void generateTrendCharts();
    void exportMetrics();

private:
    // 基准测试状态
    QList<BenchmarkResult> m_benchmarkResults;
    QList<RegressionResult> m_regressionResults;
    QVariantMap m_baselineMetrics;
    QVariantMap m_performanceThresholds;
    QVariantMap m_historicalData;
    
    // 监控组件
    QTimer* m_monitoringTimer;
    QProcess* m_benchmarkProcess;
    bool m_continuousMonitoringEnabled;
    int m_monitoringInterval;
    
    // 资源监控数据
    QList<double> m_cpuUsageHistory;
    QList<qint64> m_memoryUsageHistory;
    QList<double> m_networkUsageHistory;
    QList<double> m_diskUsageHistory;
    
    // 配置
    QString m_configFilePath;
    QString m_baselinesFilePath;
    QString m_historicalDataPath;
    QString m_reportsDirectory;
    
    // 阈值设置
    double m_regressionThreshold; // percentage
    double m_improvementThreshold; // percentage
    int m_warmupIterations;
    int m_benchmarkIterations;
    int m_maxBenchmarkDuration; // seconds
    
    // 同步对象
    QMutex m_benchmarkMutex;
    QWaitCondition m_benchmarkCondition;
    
    // 统计数据
    int m_totalBenchmarks;
    int m_passedBenchmarks;
    int m_failedBenchmarks;
    int m_regressionsDetected;
    int m_improvementsDetected;
    
    // 当前运行状态
    bool m_benchmarksRunning;
    QString m_currentBenchmark;
    QDateTime m_benchmarkStartTime;
    QElapsedTimer m_benchmarkTimer;
};

#endif // PERFORMANCE_BENCHMARK_SUITE_H