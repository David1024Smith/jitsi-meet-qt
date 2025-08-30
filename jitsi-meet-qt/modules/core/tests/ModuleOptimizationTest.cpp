#include "ModuleOptimizationTest.h"
#include "../include/ModuleCommunicationBus.h"
#include "../include/ModuleResourceManager.h"
#include "../include/ModuleStartupOptimizer.h"
#include "../include/ModulePerformanceIntegrator.h"
#include <QTest>
#include <QSignalSpy>
#include <QThread>

void ModuleOptimizationTest::initTestCase()
{
    // 初始化测试环境
    m_communicationBus = ModuleCommunicationBus::instance();
    m_resourceManager = ModuleResourceManager::instance();
    m_startupOptimizer = ModuleStartupOptimizer::instance();
    m_performanceIntegrator = ModulePerformanceIntegrator::instance();
    
    // 启动系统
    m_communicationBus->start();
    m_resourceManager->initialize();
    m_startupOptimizer->initialize();
    m_performanceIntegrator->initialize();
}

void ModuleOptimizationTest::cleanupTestCase()
{
    // 清理测试环境
    m_performanceIntegrator->shutdown();
    m_startupOptimizer->shutdown();
    m_resourceManager->shutdown();
    m_communicationBus->stop();
}

void ModuleOptimizationTest::testCommunicationBusPerformance()
{
    // 测试通信总线性能优化
    
    // 1. 测试消息发送性能
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < 1000; ++i) {
        m_communicationBus->sendEvent(QString("test_event_%1").arg(i), QString("test_data_%1").arg(i));
    }
    
    qint64 sendTime = timer.elapsed();
    QVERIFY(sendTime < 5000);  // 应该在5秒内完成
    
    // 2. 测试批量发送
    QList<ModuleCommunicationBus::Message> messages;
    for (int i = 0; i < 100; ++i) {
        ModuleCommunicationBus::Message msg;
        msg.type = ModuleCommunicationBus::Event;
        msg.payload = QString("batch_data_%1").arg(i);
        messages.append(msg);
    }
    
    timer.restart();
    bool batchResult = m_communicationBus->sendBatch(messages);
    qint64 batchTime = timer.elapsed();
    
    QVERIFY(batchResult);
    QVERIFY(batchTime < 1000);  // 批量发送应该更快
    
    // 3. 测试性能指标
    auto metrics = m_communicationBus->getPerformanceMetrics();
    QVERIFY(metrics.totalMessages > 0);
    QVERIFY(metrics.processedMessages > 0);
    QVERIFY(metrics.throughput > 0);
}

void ModuleOptimizationTest::testResourceManagerOptimization()
{
    // 测试资源管理器优化
    
    // 1. 测试资源存储和获取
    QString testResourceId = "test_resource_1";
    QVariant testData = "This is test data for resource optimization";
    
    bool storeResult = m_resourceManager->storeResource(testResourceId, testData);
    QVERIFY(storeResult);
    
    QVariant retrievedData = m_resourceManager->getResource(testResourceId);
    QCOMPARE(retrievedData.toString(), testData.toString());
    
    // 2. 测试缓存性能
    QElapsedTimer timer;
    timer.start();
    
    // 存储多个资源
    for (int i = 0; i < 100; ++i) {
        QString resourceId = QString("perf_test_%1").arg(i);
        QVariant data = QString("Performance test data %1").arg(i);
        m_resourceManager->storeResource(resourceId, data);
    }
    
    qint64 storeTime = timer.elapsed();
    
    // 获取资源（应该从缓存获取）
    timer.restart();
    for (int i = 0; i < 100; ++i) {
        QString resourceId = QString("perf_test_%1").arg(i);
        m_resourceManager->getResource(resourceId);
    }
    
    qint64 retrieveTime = timer.elapsed();
    
    QVERIFY(retrieveTime < storeTime);  // 从缓存获取应该更快
    
    // 3. 测试缓存统计
    auto cacheStats = m_resourceManager->getCacheStatistics();
    QVERIFY(cacheStats.hitCount > 0);
    QVERIFY(cacheStats.hitRatio > 0.5);  // 命中率应该大于50%
    
    // 4. 测试内存优化
    qint64 memoryBefore = m_resourceManager->getMemoryUsage();
    m_resourceManager->compactMemory();
    qint64 memoryAfter = m_resourceManager->getMemoryUsage();
    
    QVERIFY(memoryAfter <= memoryBefore);  // 内存使用应该不增加
}

void ModuleOptimizationTest::testStartupOptimization()
{
    // 测试启动优化
    
    // 1. 配置测试模块
    QStringList testModules = {"test_module_1", "test_module_2", "test_module_3", "test_module_4"};
    
    for (int i = 0; i < testModules.size(); ++i) {
        ModuleStartupOptimizer::ModuleLoadInfo info;
        info.moduleName = testModules[i];
        info.strategy = ModuleStartupOptimizer::Parallel;
        info.priority = i + 1;
        info.estimatedLoadTime = 100 + (i * 50);
        
        m_startupOptimizer->setModuleLoadInfo(testModules[i], info);
    }
    
    // 2. 测试并行加载
    m_startupOptimizer->enableParallelLoading(true, 4);
    
    QSignalSpy sessionStartedSpy(m_startupOptimizer, &ModuleStartupOptimizer::loadSessionStarted);
    QSignalSpy sessionCompletedSpy(m_startupOptimizer, &ModuleStartupOptimizer::loadSessionCompleted);
    
    QElapsedTimer timer;
    timer.start();
    
    QString sessionId = m_startupOptimizer->startLoadSession(testModules);
    QVERIFY(!sessionId.isEmpty());
    
    // 等待加载完成
    QTest::qWait(2000);
    
    qint64 loadTime = timer.elapsed();
    
    QVERIFY(sessionStartedSpy.count() == 1);
    QVERIFY(loadTime < 1000);  // 并行加载应该很快
    
    // 3. 测试加载顺序优化
    QStringList optimizedOrder = m_startupOptimizer->optimizeLoadOrder(testModules);
    QCOMPARE(optimizedOrder.size(), testModules.size());
    
    // 验证优先级排序（高优先级在前）
    for (int i = 0; i < optimizedOrder.size() - 1; ++i) {
        auto info1 = m_startupOptimizer->getModuleLoadInfo(optimizedOrder[i]);
        auto info2 = m_startupOptimizer->getModuleLoadInfo(optimizedOrder[i + 1]);
        QVERIFY(info1.priority >= info2.priority);
    }
    
    // 4. 测试预加载
    m_startupOptimizer->enablePreloading(true, 100);
    m_startupOptimizer->schedulePreload("preload_test_module", 50);
    
    QSignalSpy preloadSpy(m_startupOptimizer, &ModuleStartupOptimizer::preloadCompleted);
    QTest::qWait(200);
    
    QVERIFY(preloadSpy.count() > 0);
}

void ModuleOptimizationTest::testPerformanceIntegration()
{
    // 测试性能集成器
    
    // 1. 测试系统指标收集
    m_performanceIntegrator->updatePerformanceMetrics();
    
    auto metrics = m_performanceIntegrator->getSystemMetrics();
    QVERIFY(metrics.performanceScore >= 0 && metrics.performanceScore <= 100);
    QVERIFY(!metrics.performanceLevel.isEmpty());
    
    // 2. 测试优化建议生成
    auto recommendations = m_performanceIntegrator->getOptimizationRecommendations();
    // 建议数量可能为0（如果系统性能良好）
    
    // 3. 测试自动优化
    QSignalSpy optimizationSpy(m_performanceIntegrator, &ModulePerformanceIntegrator::optimizationCompleted);
    
    m_performanceIntegrator->optimizeMemoryUsage();
    m_performanceIntegrator->optimizeCommunication();
    m_performanceIntegrator->optimizeStartupPerformance();
    m_performanceIntegrator->optimizeResourceUsage();
    
    QVERIFY(optimizationSpy.count() == 4);
    
    // 4. 测试性能监控
    QSignalSpy metricsSpy(m_performanceIntegrator, &ModulePerformanceIntegrator::performanceMetricsUpdated);
    
    m_performanceIntegrator->updatePerformanceMetrics();
    
    QVERIFY(metricsSpy.count() > 0);
}

void ModuleOptimizationTest::testMemoryOptimization()
{
    // 测试内存优化
    
    // 1. 创建大量资源以测试内存管理
    QStringList resourceIds;
    for (int i = 0; i < 1000; ++i) {
        QString resourceId = QString("memory_test_%1").arg(i);
        QVariant data = QString("Large data block %1").arg(i).repeated(100);  // 创建较大的数据
        
        m_resourceManager->storeResource(resourceId, data, ModuleResourceManager::TempResource);
        resourceIds.append(resourceId);
    }
    
    qint64 memoryBefore = m_resourceManager->getMemoryUsage();
    
    // 2. 执行内存优化
    m_performanceIntegrator->optimizeMemoryUsage();
    
    qint64 memoryAfter = m_resourceManager->getMemoryUsage();
    
    // 内存使用应该减少或至少不增加
    QVERIFY(memoryAfter <= memoryBefore);
    
    // 3. 测试资源清理
    m_resourceManager->freeUnusedResources();
    
    qint64 memoryAfterCleanup = m_resourceManager->getMemoryUsage();
    QVERIFY(memoryAfterCleanup <= memoryAfter);
}

void ModuleOptimizationTest::testCommunicationOptimization()
{
    // 测试通信优化
    
    // 1. 发送大量消息以测试通信性能
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < 5000; ++i) {
        m_communicationBus->sendEventAsync(QString("perf_test_%1").arg(i), QString("data_%1").arg(i));
    }
    
    // 等待消息处理
    QTest::qWait(1000);
    
    qint64 sendTime = timer.elapsed();
    
    // 2. 获取性能指标
    auto metricsBefore = m_communicationBus->getPerformanceMetrics();
    
    // 3. 执行通信优化
    m_performanceIntegrator->optimizeCommunication();
    
    // 4. 再次发送消息测试优化效果
    timer.restart();
    
    for (int i = 0; i < 5000; ++i) {
        m_communicationBus->sendEventAsync(QString("optimized_test_%1").arg(i), QString("data_%1").arg(i));
    }
    
    QTest::qWait(1000);
    
    qint64 optimizedSendTime = timer.elapsed();
    
    auto metricsAfter = m_communicationBus->getPerformanceMetrics();
    
    // 验证优化效果
    QVERIFY(metricsAfter.throughput >= metricsBefore.throughput);  // 吞吐量应该不降低
}

void ModuleOptimizationTest::testFullSystemOptimization()
{
    // 测试完整系统优化
    
    // 1. 记录优化前的系统状态
    auto metricsBefore = m_performanceIntegrator->getSystemMetrics();
    
    // 2. 执行完整系统优化
    QSignalSpy optimizationSpy(m_performanceIntegrator, &ModulePerformanceIntegrator::optimizationCompleted);
    
    m_performanceIntegrator->performFullOptimization();
    
    // 应该有4个优化完成信号（内存、通信、启动、资源）+ 1个完整优化信号
    QVERIFY(optimizationSpy.count() == 5);
    
    // 3. 等待优化完成并更新指标
    QTest::qWait(1000);
    m_performanceIntegrator->updatePerformanceMetrics();
    
    auto metricsAfter = m_performanceIntegrator->getSystemMetrics();
    
    // 4. 验证优化效果
    QVERIFY(metricsAfter.performanceScore >= metricsBefore.performanceScore);  // 性能评分应该不降低
    
    // 5. 测试自动优化
    m_performanceIntegrator->enableAutoOptimization(true);
    QVERIFY(m_performanceIntegrator->isAutoOptimizationEnabled());
    
    m_performanceIntegrator->enableAutoOptimization(false);
    QVERIFY(!m_performanceIntegrator->isAutoOptimizationEnabled());
}

QTEST_MAIN(ModuleOptimizationTest)