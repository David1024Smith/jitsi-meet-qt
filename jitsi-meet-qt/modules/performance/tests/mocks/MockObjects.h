#ifndef MOCKOBJECTS_H
#define MOCKOBJECTS_H

#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <QDateTime>
#include "BaseMonitor.h"
#include "BaseOptimizer.h"
#include "PerformanceManager.h"

/**
 * @brief Mock监控器类，用于测试
 */
class MockMonitor : public BaseMonitor
{
    Q_OBJECT

public:
    explicit MockMonitor(const QString& name, QObject* parent = nullptr);
    ~MockMonitor() override = default;

    // 设置模拟数据
    void setMockCpuUsage(double usage) { m_mockCpuUsage = usage; }
    void setMockMemoryUsage(size_t usage) { m_mockMemoryUsage = usage; }
    void setMockNetworkLatency(double latency) { m_mockNetworkLatency = latency; }
    void setMockBandwidth(double bandwidth) { m_mockBandwidth = bandwidth; }
    
    // 设置错误模拟
    void setSimulateError(bool simulate) { m_simulateError = simulate; }
    void setErrorMessage(const QString& message) { m_errorMessage = message; }
    
    // 设置延迟模拟
    void setSimulateDelay(int delayMs) { m_simulateDelayMs = delayMs; }

protected:
    bool initializeMonitor() override;
    ResourceUsage collectResourceUsage() override;
    QList<ResourceType> supportedResourceTypes() const override;

private:
    double m_mockCpuUsage = 25.0;
    size_t m_mockMemoryUsage = 512 * 1024 * 1024; // 512MB
    double m_mockNetworkLatency = 50.0;
    double m_mockBandwidth = 100.0;
    bool m_simulateError = false;
    QString m_errorMessage = "Mock error";
    int m_simulateDelayMs = 0;
    int m_callCount = 0;
};

/**
 * @brief Mock优化器类，用于测试
 */
class MockOptimizer : public BaseOptimizer
{
    Q_OBJECT

public:
    explicit MockOptimizer(const QString& name, QObject* parent = nullptr);
    ~MockOptimizer() override = default;

    // 设置模拟结果
    void setMockSuccess(bool success) { m_mockSuccess = success; }
    void setMockExecutionTime(qint64 time) { m_mockExecutionTime = time; }
    void setMockImprovements(const OptimizationResult::Improvements& improvements) { 
        m_mockImprovements = improvements; 
    }
    
    // 设置错误模拟
    void setSimulateError(bool simulate) { m_simulateError = simulate; }
    void setErrorMessage(const QString& message) { m_errorMessage = message; }
    
    // 设置延迟模拟
    void setSimulateDelay(int delayMs) { m_simulateDelayMs = delayMs; }
    
    // 获取调用统计
    int getOptimizationCallCount() const { return m_callCount; }
    OptimizationStrategy getLastStrategy() const { return m_lastStrategy; }

protected:
    bool initializeOptimizer() override;
    OptimizationResult performOptimization(OptimizationStrategy strategy) override;
    bool analyzeOptimizationNeed() const override;
    QStringList generateSuggestions() const override;
    QVariantMap estimateOptimizationImprovements(OptimizationStrategy strategy) const override;
    QString getOptimizerVersion() const override;
    QString getOptimizerDescription() const override;
    OptimizationType getOptimizerType() const override;

private:
    bool m_mockSuccess = true;
    qint64 m_mockExecutionTime = 100;
    OptimizationResult::Improvements m_mockImprovements;
    bool m_simulateError = false;
    QString m_errorMessage = "Mock optimization error";
    int m_simulateDelayMs = 0;
    int m_callCount = 0;
    OptimizationStrategy m_lastStrategy = OptimizationStrategy::Balanced;
};

/**
 * @brief Mock性能管理器类，用于测试
 */
class MockPerformanceManager : public PerformanceManager
{
    Q_OBJECT

public:
    explicit MockPerformanceManager(QObject* parent = nullptr);
    ~MockPerformanceManager() override = default;

    // 重写关键方法以提供可控的测试行为
    bool initialize() override;
    bool start() override;
    void stop() override;
    PerformanceMetrics getCurrentMetrics() const override;
    
    // 设置模拟数据
    void setMockMetrics(const PerformanceMetrics& metrics) { m_mockMetrics = metrics; }
    void setMockInitializeResult(bool result) { m_mockInitializeResult = result; }
    void setMockStartResult(bool result) { m_mockStartResult = result; }
    
    // 设置错误模拟
    void setSimulateError(bool simulate) { m_simulateError = simulate; }
    
    // 获取调用统计
    int getInitializeCallCount() const { return m_initializeCallCount; }
    int getStartCallCount() const { return m_startCallCount; }
    int getStopCallCount() const { return m_stopCallCount; }
    int getGetMetricsCallCount() const { return m_getMetricsCallCount; }

private:
    PerformanceMetrics m_mockMetrics;
    bool m_mockInitializeResult = true;
    bool m_mockStartResult = true;
    bool m_simulateError = false;
    
    mutable int m_initializeCallCount = 0;
    mutable int m_startCallCount = 0;
    mutable int m_stopCallCount = 0;
    mutable int m_getMetricsCallCount = 0;
};

/**
 * @brief Mock指标收集器类，用于测试
 */
class MockMetricsCollector : public MetricsCollector
{
    Q_OBJECT

public:
    explicit MockMetricsCollector(QObject* parent = nullptr);
    ~MockMetricsCollector() override = default;

    // 重写关键方法
    bool initialize() override;
    bool start() override;
    void stop() override;
    PerformanceMetrics collectCurrentMetrics() override;
    
    // 设置模拟数据
    void setMockMetrics(const PerformanceMetrics& metrics) { m_mockMetrics = metrics; }
    void setMockHistoricalData(const QList<PerformanceMetrics>& data) { m_mockHistoricalData = data; }
    
    // 模拟数据收集
    void simulateDataCollection();
    
    // 获取调用统计
    int getCollectionCallCount() const { return m_collectionCallCount; }

private:
    PerformanceMetrics m_mockMetrics;
    QList<PerformanceMetrics> m_mockHistoricalData;
    mutable int m_collectionCallCount = 0;
};

/**
 * @brief 测试工具类
 */
class TestUtils : public QObject
{
    Q_OBJECT

public:
    static TestUtils* instance();

    // 生成测试数据
    static PerformanceMetrics generateRandomMetrics();
    static QList<PerformanceMetrics> generateMetricsHistory(int count, int intervalSeconds = 60);
    
    // 验证工具
    static bool validateMetrics(const PerformanceMetrics& metrics);
    static bool compareMetrics(const PerformanceMetrics& m1, const PerformanceMetrics& m2, double tolerance = 0.01);
    
    // 时间工具
    static void waitForSignal(QObject* sender, const char* signal, int timeoutMs = 5000);
    static void simulateSystemLoad(int durationMs = 100);
    
    // 文件工具
    static QString createTempFile(const QString& content = QString());
    static bool createTempDirectory(const QString& path);
    static void cleanupTempFiles();
    
    // 内存工具
    static size_t getCurrentMemoryUsage();
    static void forceGarbageCollection();

private:
    explicit TestUtils(QObject* parent = nullptr);
    static TestUtils* s_instance;
    static QStringList s_tempFiles;
    static QStringList s_tempDirectories;
};

/**
 * @brief 性能基准测试类
 */
class PerformanceBenchmark : public QObject
{
    Q_OBJECT

public:
    struct BenchmarkResult {
        QString testName;
        qint64 executionTime;
        size_t memoryUsage;
        double cpuUsage;
        bool success;
        QString errorMessage;
    };

    explicit PerformanceBenchmark(QObject* parent = nullptr);
    ~PerformanceBenchmark() = default;

    // 基准测试方法
    BenchmarkResult benchmarkModuleInitialization();
    BenchmarkResult benchmarkDataCollection(int iterations = 1000);
    BenchmarkResult benchmarkOptimization(int iterations = 100);
    BenchmarkResult benchmarkMemoryUsage(int iterations = 1000);
    
    // 压力测试
    BenchmarkResult stressTestConcurrentAccess(int threadCount = 10, int operationsPerThread = 100);
    BenchmarkResult stressTestLongRunning(int durationSeconds = 60);
    
    // 报告生成
    QString generateBenchmarkReport(const QList<BenchmarkResult>& results);
    bool saveBenchmarkReport(const QList<BenchmarkResult>& results, const QString& filePath);

private:
    QElapsedTimer m_timer;
    size_t m_initialMemory;
};

#endif // MOCKOBJECTS_H