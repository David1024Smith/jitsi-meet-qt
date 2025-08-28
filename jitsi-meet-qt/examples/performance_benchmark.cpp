/**
 * @file performance_benchmark.cpp
 * @brief 性能基准测试工具
 * 
 * 这个工具用于测试和验证各种性能优化组件的效果，
 * 提供详细的性能指标和比较分析。
 */

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QtConcurrent>

#include "PerformanceManager.h"
#include "StartupOptimizer.h"
#include "MemoryLeakDetector.h"
#include "MemoryProfiler.h"
#include "OptimizedRecentManager.h"
#include "PerformanceConfig.h"

class PerformanceBenchmark : public QObject
{
    Q_OBJECT

public:
    struct BenchmarkResult {
        QString testName;
        qint64 duration;
        qint64 memoryBefore;
        qint64 memoryAfter;
        qint64 memoryPeak;
        bool success;
        QString details;
    };

    PerformanceBenchmark(QObject* parent = nullptr);
    ~PerformanceBenchmark();

    void runAllBenchmarks();
    void exportResults(const QString& filePath);

private:
    // 基准测试方法
    BenchmarkResult benchmarkStartupOptimization();
    BenchmarkResult benchmarkMemoryManagement();
    BenchmarkResult benchmarkRecentItemsPerformance();
    BenchmarkResult benchmarkMemoryLeakDetection();
    BenchmarkResult benchmarkMemoryProfiling();
    BenchmarkResult benchmarkConfigurationManagement();
    BenchmarkResult benchmarkLargeDatasetHandling();
    BenchmarkResult benchmarkConcurrentOperations();

    // 辅助方法
    qint64 getCurrentMemoryUsage();
    void simulateMemoryLoad(int sizeMB);
    void clearMemoryLoad();
    void logResult(const BenchmarkResult& result);

    QList<BenchmarkResult> m_results;
    QList<void*> m_testAllocations;
    
    // 性能组件
    PerformanceManager* m_performanceManager;
    StartupOptimizer* m_startupOptimizer;
    MemoryLeakDetector* m_memoryLeakDetector;
    MemoryProfiler* m_memoryProfiler;
    OptimizedRecentManager* m_recentManager;
    PerformanceConfig* m_performanceConfig;
};

PerformanceBenchmark::PerformanceBenchmark(QObject* parent)
    : QObject(parent)
{
    // 初始化性能组件
    m_performanceConfig = new PerformanceConfig(this);
    m_performanceManager = new PerformanceManager(this);
    m_startupOptimizer = new StartupOptimizer(this);
    m_memoryLeakDetector = new MemoryLeakDetector(this);
    m_memoryProfiler = new MemoryProfiler(this);
    m_recentManager = new OptimizedRecentManager(this);
    
    qDebug() << "Performance Benchmark initialized";
}

PerformanceBenchmark::~PerformanceBenchmark()
{
    clearMemoryLoad();
}

void PerformanceBenchmark::runAllBenchmarks()
{
    qDebug() << "Starting performance benchmarks...";
    
    m_results.clear();
    
    // 运行所有基准测试
    m_results.append(benchmarkStartupOptimization());
    m_results.append(benchmarkMemoryManagement());
    m_results.append(benchmarkRecentItemsPerformance());
    m_results.append(benchmarkMemoryLeakDetection());
    m_results.append(benchmarkMemoryProfiling());
    m_results.append(benchmarkConfigurationManagement());
    m_results.append(benchmarkLargeDatasetHandling());
    m_results.append(benchmarkConcurrentOperations());
    
    // 输出总结
    qDebug() << "\n=== Benchmark Summary ===";
    int passed = 0;
    qint64 totalTime = 0;
    
    for (const BenchmarkResult& result : m_results) {
        logResult(result);
        if (result.success) {
            passed++;
        }
        totalTime += result.duration;
    }
    
    qDebug() << QString("Total tests: %1, Passed: %2, Failed: %3")
                .arg(m_results.size()).arg(passed).arg(m_results.size() - passed);
    qDebug() << QString("Total time: %1 ms").arg(totalTime);
    qDebug() << "========================\n";
}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::benchmarkStartupOptimization()
{
    BenchmarkResult result;
    result.testName = "Startup Optimization";
    result.memoryBefore = getCurrentMemoryUsage();
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        // 测试不同优化级别的启动时间
        QList<qint64> startupTimes;
        
        for (int level = 0; level <= 2; ++level) {
            StartupOptimizer optimizer;
            optimizer.setOptimizationLevel(static_cast<StartupOptimizer::OptimizationLevel>(level));
            
            QElapsedTimer startupTimer;
            startupTimer.start();
            
            optimizer.enableFastStartup();
            optimizer.preloadCriticalResources();
            
            // 等待预加载完成
            QEventLoop loop;
            QTimer::singleShot(100, &loop, &QEventLoop::quit);
            loop.exec();
            
            qint64 startupTime = startupTimer.elapsed();
            startupTimes.append(startupTime);
        }
        
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.memoryPeak = result.memoryAfter;
        result.success = true;
        
        qint64 avgStartupTime = 0;
        for (qint64 time : startupTimes) {
            avgStartupTime += time;
        }
        avgStartupTime /= startupTimes.size();
        
        result.details = QString("Average startup time: %1 ms, Levels tested: %2")
                        .arg(avgStartupTime).arg(startupTimes.size());
        
    } catch (...) {
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.success = false;
        result.details = "Exception occurred during startup optimization test";
    }
    
    return result;
}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::benchmarkMemoryManagement()
{
    BenchmarkResult result;
    result.testName = "Memory Management";
    result.memoryBefore = getCurrentMemoryUsage();
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        // 启动内存监控
        m_performanceManager->startMemoryMonitoring();
        
        // 模拟内存负载
        simulateMemoryLoad(50); // 50MB
        
        qint64 memoryWithLoad = getCurrentMemoryUsage();
        result.memoryPeak = memoryWithLoad;
        
        // 执行内存清理
        m_performanceManager->performMemoryCleanup();
        
        // 等待清理完成
        QThread::msleep(100);
        
        qint64 memoryAfterCleanup = getCurrentMemoryUsage();
        
        // 清理测试负载
        clearMemoryLoad();
        
        m_performanceManager->stopMemoryMonitoring();
        
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.success = true;
        
        qint64 memoryFreed = memoryWithLoad - memoryAfterCleanup;
        result.details = QString("Memory freed by cleanup: %1 MB, Peak usage: %2 MB")
                        .arg(memoryFreed / (1024*1024))
                        .arg(result.memoryPeak / (1024*1024));
        
    } catch (...) {
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.success = false;
        result.details = "Exception occurred during memory management test";
        clearMemoryLoad();
    }
    
    return result;
}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::benchmarkRecentItemsPerformance()
{
    BenchmarkResult result;
    result.testName = "Recent Items Performance";
    result.memoryBefore = getCurrentMemoryUsage();
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        // 测试大量项目的添加性能
        const int itemCount = 1000;
        
        QElapsedTimer addTimer;
        addTimer.start();
        
        for (int i = 0; i < itemCount; ++i) {
            QString url = QString("https://meet.jit.si/perf-test-%1").arg(i);
            QString name = QString("Performance Test Room %1").arg(i);
            m_recentManager->addRecentItem(url, name);
        }
        
        qint64 addTime = addTimer.elapsed();
        
        // 测试搜索性能
        QElapsedTimer searchTimer;
        searchTimer.start();
        
        QList<RecentItem> searchResults = m_recentManager->searchRecentItems("test");
        
        qint64 searchTime = searchTimer.elapsed();
        
        // 测试优化性能
        QElapsedTimer optimizeTimer;
        optimizeTimer.start();
        
        m_recentManager->optimizeStorage();
        
        qint64 optimizeTime = optimizeTimer.elapsed();
        
        // 清理
        m_recentManager->clearRecentItems();
        
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.memoryPeak = result.memoryAfter;
        result.success = true;
        
        result.details = QString("Added %1 items in %2 ms, Search: %3 ms (%4 results), Optimize: %5 ms")
                        .arg(itemCount).arg(addTime).arg(searchTime)
                        .arg(searchResults.size()).arg(optimizeTime);
        
    } catch (...) {
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.success = false;
        result.details = "Exception occurred during recent items performance test";
        m_recentManager->clearRecentItems();
    }
    
    return result;
}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::benchmarkMemoryLeakDetection()
{
    BenchmarkResult result;
    result.testName = "Memory Leak Detection";
    result.memoryBefore = getCurrentMemoryUsage();
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        m_memoryLeakDetector->startLeakDetection();
        
        // 模拟一些分配和释放
        QList<void*> testPtrs;
        
        for (int i = 0; i < 100; ++i) {
            void* ptr = malloc(1024);
            testPtrs.append(ptr);
            m_memoryLeakDetector->trackAllocation(ptr, 1024, __FILE__, __LINE__);
        }
        
        // 释放一半
        for (int i = 0; i < 50; ++i) {
            void* ptr = testPtrs[i];
            m_memoryLeakDetector->trackDeallocation(ptr);
            free(ptr);
        }
        
        // 执行泄漏检查
        m_memoryLeakDetector->performLeakCheck();
        
        int allocationCount = m_memoryLeakDetector->getAllocationCount();
        qint64 totalMemory = m_memoryLeakDetector->getTotalAllocatedMemory();
        
        // 清理剩余分配
        for (int i = 50; i < testPtrs.size(); ++i) {
            void* ptr = testPtrs[i];
            m_memoryLeakDetector->trackDeallocation(ptr);
            free(ptr);
        }
        
        m_memoryLeakDetector->stopLeakDetection();
        
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.memoryPeak = result.memoryAfter;
        result.success = true;
        
        result.details = QString("Tracked allocations: %1, Total memory: %2 KB")
                        .arg(allocationCount).arg(totalMemory / 1024);
        
    } catch (...) {
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.success = false;
        result.details = "Exception occurred during memory leak detection test";
    }
    
    return result;
}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::benchmarkMemoryProfiling()
{
    BenchmarkResult result;
    result.testName = "Memory Profiling";
    result.memoryBefore = getCurrentMemoryUsage();
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        m_memoryProfiler->setSnapshotInterval(50); // 50ms
        m_memoryProfiler->startProfiling();
        
        // 生成一些内存活动
        simulateMemoryLoad(20); // 20MB
        QThread::msleep(200); // 等待快照
        
        clearMemoryLoad();
        QThread::msleep(200); // 等待更多快照
        
        // 分析趋势
        MemoryProfiler::MemoryTrend trend = m_memoryProfiler->analyzeTrend(1);
        
        // 生成优化建议
        QList<MemoryProfiler::OptimizationSuggestion> suggestions = 
            m_memoryProfiler->generateOptimizationSuggestions();
        
        // 生成报告
        QJsonObject report = m_memoryProfiler->generateDetailedReport();
        
        m_memoryProfiler->stopProfiling();
        
        int snapshotCount = m_memoryProfiler->getSnapshotCount();
        
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.memoryPeak = trend.peakUsage;
        result.success = true;
        
        result.details = QString("Snapshots: %1, Suggestions: %2, Peak: %3 MB, Growth: %4%")
                        .arg(snapshotCount).arg(suggestions.size())
                        .arg(trend.peakUsage / (1024*1024))
                        .arg(trend.growthRate * 100, 0, 'f', 2);
        
    } catch (...) {
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.success = false;
        result.details = "Exception occurred during memory profiling test";
        clearMemoryLoad();
    }
    
    return result;
}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::benchmarkConfigurationManagement()
{
    BenchmarkResult result;
    result.testName = "Configuration Management";
    result.memoryBefore = getCurrentMemoryUsage();
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        // 测试配置保存和加载
        QElapsedTimer saveTimer;
        saveTimer.start();
        
        // 修改配置
        PerformanceConfig::MemorySettings memSettings = m_performanceConfig->memorySettings();
        memSettings.warningThreshold = 1024 * 1024 * 1024; // 1GB
        m_performanceConfig->setMemorySettings(memSettings);
        
        m_performanceConfig->saveConfiguration();
        qint64 saveTime = saveTimer.elapsed();
        
        // 测试自动调整
        QElapsedTimer tuneTimer;
        tuneTimer.start();
        
        m_performanceConfig->autoTuneForSystem();
        
        qint64 tuneTime = tuneTimer.elapsed();
        
        // 测试重置
        QElapsedTimer resetTimer;
        resetTimer.start();
        
        m_performanceConfig->resetToDefaults();
        
        qint64 resetTime = resetTimer.elapsed();
        
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.memoryPeak = result.memoryAfter;
        result.success = true;
        
        result.details = QString("Save: %1 ms, Auto-tune: %2 ms, Reset: %3 ms")
                        .arg(saveTime).arg(tuneTime).arg(resetTime);
        
    } catch (...) {
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.success = false;
        result.details = "Exception occurred during configuration management test";
    }
    
    return result;
}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::benchmarkLargeDatasetHandling()
{
    BenchmarkResult result;
    result.testName = "Large Dataset Handling";
    result.memoryBefore = getCurrentMemoryUsage();
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        // 测试大量最近项目的处理
        const int largeItemCount = 5000;
        
        QElapsedTimer addTimer;
        addTimer.start();
        
        for (int i = 0; i < largeItemCount; ++i) {
            QString url = QString("https://meet.jit.si/large-test-%1").arg(i);
            QString name = QString("Large Test Room %1").arg(i);
            m_recentManager->addRecentItem(url, name);
            
            // 每1000个项目检查一次内存
            if (i % 1000 == 0) {
                qint64 currentMemory = getCurrentMemoryUsage();
                if (currentMemory > result.memoryPeak) {
                    result.memoryPeak = currentMemory;
                }
            }
        }
        
        qint64 addTime = addTimer.elapsed();
        
        // 测试搜索性能
        QElapsedTimer searchTimer;
        searchTimer.start();
        
        QList<RecentItem> results = m_recentManager->searchRecentItems("large");
        
        qint64 searchTime = searchTimer.elapsed();
        
        // 测试优化大数据集
        QElapsedTimer optimizeTimer;
        optimizeTimer.start();
        
        m_recentManager->optimizeStorage();
        
        qint64 optimizeTime = optimizeTimer.elapsed();
        
        int finalCount = m_recentManager->getItemCount();
        
        // 清理
        m_recentManager->clearRecentItems();
        
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.success = true;
        
        result.details = QString("Added %1 items in %2 ms, Search: %3 ms (%4 results), "
                               "Optimize: %5 ms, Final count: %6")
                        .arg(largeItemCount).arg(addTime).arg(searchTime)
                        .arg(results.size()).arg(optimizeTime).arg(finalCount);
        
    } catch (...) {
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.success = false;
        result.details = "Exception occurred during large dataset handling test";
        m_recentManager->clearRecentItems();
    }
    
    return result;
}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::benchmarkConcurrentOperations()
{
    BenchmarkResult result;
    result.testName = "Concurrent Operations";
    result.memoryBefore = getCurrentMemoryUsage();
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        // 测试并发操作
        const int threadCount = 4;
        const int operationsPerThread = 100;
        
        QList<QFuture<void>> futures;
        
        // 启动并发任务
        for (int t = 0; t < threadCount; ++t) {
            QFuture<void> future = QtConcurrent::run([this, t, operationsPerThread]() {
                for (int i = 0; i < operationsPerThread; ++i) {
                    QString url = QString("https://meet.jit.si/concurrent-%1-%2").arg(t).arg(i);
                    QString name = QString("Concurrent Room %1-%2").arg(t).arg(i);
                    m_recentManager->addRecentItem(url, name);
                    
                    // 模拟一些内存分配
                    void* ptr = malloc(1024);
                    m_memoryLeakDetector->trackAllocation(ptr, 1024, __FILE__, __LINE__);
                    
                    // 立即释放
                    m_memoryLeakDetector->trackDeallocation(ptr);
                    free(ptr);
                    
                    QThread::msleep(1); // 短暂延迟
                }
            });
            
            futures.append(future);
        }
        
        // 等待所有任务完成
        for (QFuture<void>& future : futures) {
            future.waitForFinished();
        }
        
        int totalItems = m_recentManager->getItemCount();
        
        // 清理
        m_recentManager->clearRecentItems();
        
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.memoryPeak = result.memoryAfter;
        result.success = true;
        
        result.details = QString("Threads: %1, Operations per thread: %2, Total items: %3")
                        .arg(threadCount).arg(operationsPerThread).arg(totalItems);
        
    } catch (...) {
        result.duration = timer.elapsed();
        result.memoryAfter = getCurrentMemoryUsage();
        result.success = false;
        result.details = "Exception occurred during concurrent operations test";
        m_recentManager->clearRecentItems();
    }
    
    return result;
}

void PerformanceBenchmark::exportResults(const QString& filePath)
{
    QJsonObject root;
    root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["totalTests"] = m_results.size();
    
    QJsonArray resultsArray;
    
    for (const BenchmarkResult& result : m_results) {
        QJsonObject resultObj;
        resultObj["testName"] = result.testName;
        resultObj["duration"] = result.duration;
        resultObj["memoryBefore"] = result.memoryBefore;
        resultObj["memoryAfter"] = result.memoryAfter;
        resultObj["memoryPeak"] = result.memoryPeak;
        resultObj["success"] = result.success;
        resultObj["details"] = result.details;
        
        resultsArray.append(resultObj);
    }
    
    root["results"] = resultsArray;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        qDebug() << "Benchmark results exported to:" << filePath;
    } else {
        qWarning() << "Failed to export benchmark results to:" << filePath;
    }
}

qint64 PerformanceBenchmark::getCurrentMemoryUsage()
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
#endif
    return 0;
}

void PerformanceBenchmark::simulateMemoryLoad(int sizeMB)
{
    for (int i = 0; i < sizeMB; ++i) {
        void* ptr = malloc(1024 * 1024); // 1MB
        if (ptr) {
            m_testAllocations.append(ptr);
            // 写入一些数据以确保内存被实际使用
            memset(ptr, i % 256, 1024 * 1024);
        }
    }
}

void PerformanceBenchmark::clearMemoryLoad()
{
    for (void* ptr : m_testAllocations) {
        free(ptr);
    }
    m_testAllocations.clear();
}

void PerformanceBenchmark::logResult(const BenchmarkResult& result)
{
    QString status = result.success ? "PASS" : "FAIL";
    qDebug() << QString("[%1] %2: %3 ms (Memory: %4 -> %5 MB, Peak: %6 MB)")
                .arg(status)
                .arg(result.testName)
                .arg(result.duration)
                .arg(result.memoryBefore / (1024*1024))
                .arg(result.memoryAfter / (1024*1024))
                .arg(result.memoryPeak / (1024*1024));
    
    if (!result.details.isEmpty()) {
        qDebug() << "    Details:" << result.details;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    PerformanceBenchmark benchmark;
    
    qDebug() << "Starting Jitsi Meet Qt Performance Benchmark";
    qDebug() << "============================================\n";
    
    benchmark.runAllBenchmarks();
    
    // 导出结果
    QString resultsPath = "benchmark_results.json";
    benchmark.exportResults(resultsPath);
    
    qDebug() << "Benchmark completed. Results saved to:" << resultsPath;
    
    return 0;
}

#include "performance_benchmark.moc"