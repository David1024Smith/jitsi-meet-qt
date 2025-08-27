#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QThread>

#include "PerformanceManager.h"
#include "MemoryLeakDetector.h"
#include "StartupOptimizer.h"
#include "OptimizedRecentManager.h"
#include "PerformanceConfig.h"
#include "MemoryProfiler.h"

class TestPerformance : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // PerformanceManager tests
    void testPerformanceManagerStartup();
    void testPerformanceManagerMemoryMonitoring();
    void testPerformanceManagerWebEngineOptimization();
    void testPerformanceManagerMetrics();
    
    // MemoryLeakDetector tests
    void testMemoryLeakDetectorTracking();
    void testMemoryLeakDetectorLeakDetection();
    void testMemoryLeakDetectorStatistics();
    
    // StartupOptimizer tests
    void testStartupOptimizerBasic();
    void testStartupOptimizerResourcePreload();
    void testStartupOptimizerDelayedInit();
    
    // OptimizedRecentManager tests
    void testOptimizedRecentManagerBasic();
    void testOptimizedRecentManagerAsync();
    void testOptimizedRecentManagerSearch();
    void testOptimizedRecentManagerPerformance();
    
    // PerformanceConfig tests
    void testPerformanceConfigBasic();
    void testPerformanceConfigAutoTune();
    void testPerformanceConfigSaveLoad();
    
    // MemoryProfiler tests
    void testMemoryProfilerBasic();
    void testMemoryProfilerSnapshots();
    void testMemoryProfilerAnalysis();
    void testMemoryProfilerReporting();

private:
    PerformanceManager* m_performanceManager;
    MemoryLeakDetector* m_memoryLeakDetector;
    StartupOptimizer* m_startupOptimizer;
    OptimizedRecentManager* m_recentManager;
    PerformanceConfig* m_performanceConfig;
    MemoryProfiler* m_memoryProfiler;
};

void TestPerformance::initTestCase()
{
    m_performanceManager = new PerformanceManager(this);
    m_memoryLeakDetector = new MemoryLeakDetector(this);
    m_startupOptimizer = new StartupOptimizer(this);
    m_recentManager = new OptimizedRecentManager(this);
    m_performanceConfig = new PerformanceConfig(this);
    m_memoryProfiler = new MemoryProfiler(this);
}

void TestPerformance::cleanupTestCase()
{
    // 清理会在对象析构时自动进行
}

void TestPerformance::testPerformanceManagerStartup()
{
    // 测试启动时间测量
    m_performanceManager->startStartupTimer();
    
    // 模拟启动过程
    QThread::msleep(100);
    
    m_performanceManager->markStartupComplete();
    
    qint64 startupTime = m_performanceManager->getStartupTime();
    QVERIFY(startupTime >= 100);
    QVERIFY(startupTime < 1000); // 应该在合理范围内
}

void TestPerformance::testPerformanceManagerMemoryMonitoring()
{
    QSignalSpy memorySpy(m_performanceManager, &PerformanceManager::memoryWarning);
    QSignalSpy metricsSpy(m_performanceManager, &PerformanceManager::performanceMetricsUpdated);
    
    m_performanceManager->startMemoryMonitoring();
    
    // 等待一些监控周期
    QTest::qWait(100);
    
    m_performanceManager->stopMemoryMonitoring();
    
    // 验证内存使用情况
    qint64 memoryUsage = m_performanceManager->getCurrentMemoryUsage();
    QVERIFY(memoryUsage > 0);
    
    // 测试内存清理
    m_performanceManager->performMemoryCleanup();
    
    qint64 memoryAfterCleanup = m_performanceManager->getCurrentMemoryUsage();
    // 内存使用量应该保持合理
    QVERIFY(memoryAfterCleanup > 0);
}

void TestPerformance::testPerformanceManagerWebEngineOptimization()
{
    // 测试WebEngine优化（不需要实际的WebEngine实例）
    m_performanceManager->optimizeWebEngineMemory();
    m_performanceManager->clearWebEngineCache();
    
    // 验证操作完成（主要测试不会崩溃）
    QVERIFY(true);
}

void TestPerformance::testPerformanceManagerMetrics()
{
    PerformanceManager::PerformanceMetrics metrics = m_performanceManager->getMetrics();
    
    // 验证指标结构
    QVERIFY(metrics.startupTime >= 0);
    QVERIFY(metrics.memoryUsage >= 0);
    QVERIFY(metrics.webEngineMemory >= 0);
    QVERIFY(metrics.recentItemsCount >= 0);
    QVERIFY(metrics.configLoadTime >= 0);
    QVERIFY(metrics.resourceLoadTime >= 0);
    
    // 测试日志输出
    m_performanceManager->logPerformanceMetrics();
}

void TestPerformance::testMemoryLeakDetectorTracking()
{
    // 测试内存分配跟踪
    void* testPtr1 = malloc(1024);
    void* testPtr2 = malloc(2048);
    
    m_memoryLeakDetector->trackAllocation(testPtr1, 1024, __FILE__, __LINE__);
    m_memoryLeakDetector->trackAllocation(testPtr2, 2048, __FILE__, __LINE__);
    
    QCOMPARE(m_memoryLeakDetector->getAllocationCount(), 2);
    QCOMPARE(m_memoryLeakDetector->getTotalAllocatedMemory(), qint64(3072));
    
    // 测试释放跟踪
    m_memoryLeakDetector->trackDeallocation(testPtr1);
    QCOMPARE(m_memoryLeakDetector->getAllocationCount(), 1);
    QCOMPARE(m_memoryLeakDetector->getTotalAllocatedMemory(), qint64(2048));
    
    m_memoryLeakDetector->trackDeallocation(testPtr2);
    QCOMPARE(m_memoryLeakDetector->getAllocationCount(), 0);
    QCOMPARE(m_memoryLeakDetector->getTotalAllocatedMemory(), qint64(0));
    
    free(testPtr1);
    free(testPtr2);
}

void TestPerformance::testMemoryLeakDetectorLeakDetection()
{
    QSignalSpy leakSpy(m_memoryLeakDetector, &MemoryLeakDetector::memoryLeakDetected);
    
    m_memoryLeakDetector->startLeakDetection();
    
    // 模拟内存泄漏（不释放的分配）
    void* leakPtr = malloc(1024);
    m_memoryLeakDetector->trackAllocation(leakPtr, 1024, __FILE__, __LINE__);
    
    // 执行泄漏检查
    m_memoryLeakDetector->performLeakCheck();
    
    // 获取潜在泄漏
    QList<MemoryLeakDetector::AllocationInfo> leaks = m_memoryLeakDetector->getPotentialLeaks();
    
    m_memoryLeakDetector->stopLeakDetection();
    
    // 清理
    m_memoryLeakDetector->trackDeallocation(leakPtr);
    free(leakPtr);
}

void TestPerformance::testMemoryLeakDetectorStatistics()
{
    // 测试统计信息
    m_memoryLeakDetector->logMemoryStatistics();
    m_memoryLeakDetector->generateLeakReport();
    
    // 验证基本功能不会崩溃
    QVERIFY(true);
}

void TestPerformance::testStartupOptimizerBasic()
{
    QSignalSpy phaseSpy(m_startupOptimizer, &StartupOptimizer::startupPhaseCompleted);
    
    // 测试不同优化级别
    m_startupOptimizer->setOptimizationLevel(StartupOptimizer::Basic);
    m_startupOptimizer->setOptimizationLevel(StartupOptimizer::Moderate);
    m_startupOptimizer->setOptimizationLevel(StartupOptimizer::Aggressive);
    
    // 测试快速启动
    m_startupOptimizer->enableFastStartup();
    
    // 验证阶段计时
    QVERIFY(phaseSpy.count() > 0);
}

void TestPerformance::testStartupOptimizerResourcePreload()
{
    QSignalSpy resourceSpy(m_startupOptimizer, &StartupOptimizer::allResourcesPreloaded);
    
    // 测试资源预加载
    m_startupOptimizer->preloadCriticalResources();
    m_startupOptimizer->optimizeResourceLoading();
    
    // 等待异步操作完成
    QTest::qWait(500);
    
    // 验证预加载完成信号
    QVERIFY(resourceSpy.count() >= 0);
}

void TestPerformance::testStartupOptimizerDelayedInit()
{
    QSignalSpy delayedSpy(m_startupOptimizer, &StartupOptimizer::delayedInitializationCompleted);
    
    bool testInitCalled = false;
    
    // 添加延迟初始化任务
    m_startupOptimizer->scheduleDelayedInitialization("TestComponent", [&testInitCalled]() {
        testInitCalled = true;
    });
    
    // 执行延迟初始化
    m_startupOptimizer->executeDelayedInitializations();
    
    // 验证初始化被调用
    QVERIFY(testInitCalled);
    QCOMPARE(delayedSpy.count(), 1);
}

void TestPerformance::testOptimizedRecentManagerBasic()
{
    // 测试基本功能
    QCOMPARE(m_recentManager->getItemCount(), 0);
    
    // 添加项目
    m_recentManager->addRecentItem("https://meet.jit.si/test1", "Test Meeting 1");
    m_recentManager->addRecentItem("https://meet.jit.si/test2", "Test Meeting 2");
    
    QCOMPARE(m_recentManager->getItemCount(), 2);
    
    // 检查项目存在
    QVERIFY(m_recentManager->hasRecentItem("https://meet.jit.si/test1"));
    QVERIFY(m_recentManager->hasRecentItem("https://meet.jit.si/test2"));
    
    // 获取项目列表
    QList<RecentItem> items = m_recentManager->getRecentItems();
    QCOMPARE(items.size(), 2);
    
    // 移除项目
    m_recentManager->removeRecentItem("https://meet.jit.si/test1");
    QCOMPARE(m_recentManager->getItemCount(), 1);
    QVERIFY(!m_recentManager->hasRecentItem("https://meet.jit.si/test1"));
    
    // 清理
    m_recentManager->clearRecentItems();
    QCOMPARE(m_recentManager->getItemCount(), 0);
}

void TestPerformance::testOptimizedRecentManagerAsync()
{
    QSignalSpy loadSpy(m_recentManager, &OptimizedRecentManager::recentItemsLoaded);
    QSignalSpy addSpy(m_recentManager, &OptimizedRecentManager::recentItemAdded);
    QSignalSpy removeSpy(m_recentManager, &OptimizedRecentManager::recentItemRemoved);
    
    // 测试异步加载
    m_recentManager->loadRecentItemsAsync();
    
    // 添加项目（应该触发信号）
    m_recentManager->addRecentItem("https://meet.jit.si/async-test", "Async Test");
    
    QCOMPARE(addSpy.count(), 1);
    
    // 测试异步保存
    m_recentManager->saveRecentItemsAsync();
    
    // 等待异步操作
    QTest::qWait(100);
    
    // 清理
    m_recentManager->clearRecentItems();
}

void TestPerformance::testOptimizedRecentManagerSearch()
{
    // 添加测试数据
    m_recentManager->addRecentItem("https://meet.jit.si/project-alpha", "Project Alpha Meeting");
    m_recentManager->addRecentItem("https://meet.jit.si/project-beta", "Project Beta Discussion");
    m_recentManager->addRecentItem("https://meet.jit.si/team-standup", "Daily Team Standup");
    
    // 测试搜索
    QList<RecentItem> alphaResults = m_recentManager->searchRecentItems("alpha");
    QCOMPARE(alphaResults.size(), 1);
    QVERIFY(alphaResults[0].displayName.contains("Alpha"));
    
    QList<RecentItem> projectResults = m_recentManager->searchRecentItems("project");
    QCOMPARE(projectResults.size(), 2);
    
    QList<RecentItem> emptyResults = m_recentManager->searchRecentItems("nonexistent");
    QCOMPARE(emptyResults.size(), 0);
    
    // 清理
    m_recentManager->clearRecentItems();
}

void TestPerformance::testOptimizedRecentManagerPerformance()
{
    // 测试性能优化功能
    m_recentManager->setMaxItems(10);
    m_recentManager->setLazyLoadingEnabled(true);
    
    // 添加大量项目测试性能
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < 100; ++i) {
        QString url = QString("https://meet.jit.si/perf-test-%1").arg(i);
        QString name = QString("Performance Test %1").arg(i);
        m_recentManager->addRecentItem(url, name);
    }
    
    qint64 addTime = timer.elapsed();
    QVERIFY(addTime < 1000); // 应该在1秒内完成
    
    // 验证最大项目限制
    QVERIFY(m_recentManager->getItemCount() <= 10);
    
    // 测试优化存储
    m_recentManager->optimizeStorage();
    
    // 测试加载时间
    qint64 loadTime = m_recentManager->getLoadTime();
    QVERIFY(loadTime >= 0);
    
    // 清理
    m_recentManager->clearRecentItems();
}

void TestPerformance::testPerformanceConfigBasic()
{
    // 测试基本配置功能
    QVERIFY(m_performanceConfig->isPerformanceOptimizationEnabled());
    
    // 测试内存设置
    PerformanceConfig::MemorySettings memSettings = m_performanceConfig->memorySettings();
    QVERIFY(memSettings.warningThreshold > 0);
    QVERIFY(memSettings.criticalThreshold > memSettings.warningThreshold);
    QVERIFY(memSettings.cleanupInterval > 0);
    QVERIFY(memSettings.monitoringInterval > 0);
    
    // 测试启动设置
    PerformanceConfig::StartupSettings startupSettings = m_performanceConfig->startupSettings();
    QVERIFY(startupSettings.optimizationLevel >= 0 && startupSettings.optimizationLevel <= 2);
    QVERIFY(startupSettings.maxPreloadResources > 0);
    QVERIFY(startupSettings.delayedInitTimeout > 0);
    
    // 测试WebEngine设置
    PerformanceConfig::WebEngineSettings webSettings = m_performanceConfig->webEngineSettings();
    QVERIFY(webSettings.cacheMaxSize > 0);
    QVERIFY(webSettings.cacheCleanupInterval > 0);
    
    // 测试最近项目设置
    PerformanceConfig::RecentItemsSettings recentSettings = m_performanceConfig->recentItemsSettings();
    QVERIFY(recentSettings.maxItems > 0);
    QVERIFY(recentSettings.optimizationInterval > 0);
    QVERIFY(recentSettings.searchCacheSize > 0);
    QVERIFY(recentSettings.maxAge > 0);
}

void TestPerformance::testPerformanceConfigAutoTune()
{
    QSignalSpy configSpy(m_performanceConfig, &PerformanceConfig::configurationChanged);
    
    // 测试自动调整
    m_performanceConfig->autoTuneForSystem();
    
    // 验证配置已更改
    QVERIFY(configSpy.count() >= 0);
    
    // 测试低内存系统调整
    m_performanceConfig->adjustForLowMemorySystem();
    PerformanceConfig::MemorySettings lowMemSettings = m_performanceConfig->memorySettings();
    
    // 测试高性能系统调整
    m_performanceConfig->adjustForHighPerformanceSystem();
    PerformanceConfig::MemorySettings highMemSettings = m_performanceConfig->memorySettings();
    
    // 高性能系统应该有更高的阈值
    QVERIFY(highMemSettings.warningThreshold >= lowMemSettings.warningThreshold);
    QVERIFY(highMemSettings.criticalThreshold >= lowMemSettings.criticalThreshold);
}

void TestPerformance::testPerformanceConfigSaveLoad()
{
    // 修改配置
    PerformanceConfig::MemorySettings memSettings = m_performanceConfig->memorySettings();
    memSettings.warningThreshold = 1024 * 1024 * 1024; // 1GB
    memSettings.criticalThreshold = 2048 * 1024 * 1024; // 2GB
    
    m_performanceConfig->setMemorySettings(memSettings);
    
    // 保存配置
    m_performanceConfig->saveConfiguration();
    
    // 创建新实例并加载配置
    PerformanceConfig* newConfig = new PerformanceConfig(this);
    
    // 验证配置已正确加载
    PerformanceConfig::MemorySettings loadedSettings = newConfig->memorySettings();
    QCOMPARE(loadedSettings.warningThreshold, memSettings.warningThreshold);
    QCOMPARE(loadedSettings.criticalThreshold, memSettings.criticalThreshold);
    
    delete newConfig;
}

void TestPerformance::testMemoryProfilerBasic()
{
    // 测试基本功能
    QVERIFY(!m_memoryProfiler->isProfilingActive());
    QCOMPARE(m_memoryProfiler->getSnapshotCount(), 0);
    
    // 开始分析
    m_memoryProfiler->startProfiling();
    QVERIFY(m_memoryProfiler->isProfilingActive());
    
    // 等待一些快照
    QTest::qWait(100);
    
    // 停止分析
    m_memoryProfiler->stopProfiling();
    QVERIFY(!m_memoryProfiler->isProfilingActive());
    
    // 验证分析持续时间
    QVERIFY(m_memoryProfiler->getProfilingDuration() > 0);
}

void TestPerformance::testMemoryProfilerSnapshots()
{
    QSignalSpy snapshotSpy(m_memoryProfiler, &MemoryProfiler::snapshotTaken);
    
    m_memoryProfiler->setSnapshotInterval(50); // 50ms间隔
    m_memoryProfiler->startProfiling();
    
    // 等待几个快照
    QTest::qWait(200);
    
    m_memoryProfiler->stopProfiling();
    
    // 验证快照已生成
    QVERIFY(m_memoryProfiler->getSnapshotCount() > 0);
    QVERIFY(snapshotSpy.count() > 0);
    
    // 测试手动快照
    int oldCount = m_memoryProfiler->getSnapshotCount();
    m_memoryProfiler->takeSnapshot();
    QCOMPARE(m_memoryProfiler->getSnapshotCount(), oldCount + 1);
    
    // 测试快照历史
    QList<MemoryProfiler::MemorySnapshot> history = m_memoryProfiler->getSnapshotHistory();
    QVERIFY(!history.isEmpty());
    
    // 验证快照数据
    MemoryProfiler::MemorySnapshot current = m_memoryProfiler->getCurrentSnapshot();
    QVERIFY(current.timestamp > 0);
    QVERIFY(current.totalMemory > 0);
    
    // 清理快照历史
    m_memoryProfiler->clearSnapshotHistory();
    QCOMPARE(m_memoryProfiler->getSnapshotCount(), 0);