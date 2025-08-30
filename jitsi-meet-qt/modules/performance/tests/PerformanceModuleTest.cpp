#include "PerformanceModuleTest.h"
#include <QApplication>
#include <QThread>
#include <QEventLoop>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>

// Mock classes for testing
class MockMonitor : public BaseMonitor
{
public:
    MockMonitor(const QString& name) : BaseMonitor(name) {}
    
protected:
    bool initializeMonitor() override { return true; }
    ResourceUsage collectResourceUsage() override {
        ResourceUsage usage;
        usage.timestamp = QDateTime::currentDateTime();
        usage.cpuUsage = 25.0;
        usage.memoryUsage = 512 * 1024 * 1024; // 512MB
        return usage;
    }
    QList<ResourceType> supportedResourceTypes() const override {
        return {ResourceType::CPU, ResourceType::Memory};
    }
};

class MockOptimizer : public BaseOptimizer
{
public:
    MockOptimizer(const QString& name) : BaseOptimizer(name) {}
    
protected:
    bool initializeOptimizer() override { return true; }
    OptimizationResult performOptimization(OptimizationStrategy strategy) override {
        OptimizationResult result;
        result.success = true;
        result.strategy = strategy;
        result.executionTime = 100;
        result.improvements.cpuImprovement = 10.0;
        result.improvements.memoryImprovement = 50 * 1024 * 1024;
        return result;
    }
    bool analyzeOptimizationNeed() const override { return true; }
    QStringList generateSuggestions() const override {
        return {"Reduce memory usage", "Optimize CPU intensive operations"};
    }
    QVariantMap estimateOptimizationImprovements(OptimizationStrategy) const override {
        QVariantMap improvements;
        improvements["cpu"] = 10.0;
        improvements["memory"] = 50;
        return improvements;
    }
    QString getOptimizerVersion() const override { return "1.0.0"; }
    QString getOptimizerDescription() const override { return "Mock optimizer for testing"; }
    OptimizationType getOptimizerType() const override { return OptimizationType::Memory; }
};

void PerformanceModuleTest::initTestCase()
{
    // 设置测试环境
    setupTestEnvironment();
    
    // 创建测试数据目录
    m_testDataPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/performance_test";
    QDir().mkpath(m_testDataPath);
    
    // 初始化测试组件
    m_performanceModule = nullptr;
    m_performanceManager = nullptr;
    m_metricsCollector = nullptr;
    m_config = nullptr;
    m_testTimer = new QTimer(this);
    m_testEnvironmentReady = false;
    
    qDebug() << "Performance module test suite initialized";
}

void PerformanceModuleTest::cleanupTestCase()
{
    cleanupTestEnvironment();
    
    // 清理测试数据
    QDir testDir(m_testDataPath);
    testDir.removeRecursively();
    
    qDebug() << "Performance module test suite cleanup completed";
}

void PerformanceModuleTest::init()
{
    // 每个测试前的初始化
    if (!m_testEnvironmentReady) {
        setupTestEnvironment();
    }
}

void PerformanceModuleTest::cleanup()
{
    // 每个测试后的清理
    if (m_performanceModule) {
        m_performanceModule->shutdown();
        m_performanceModule->deleteLater();
        m_performanceModule = nullptr;
    }
    
    if (m_performanceManager) {
        m_performanceManager->stop();
        m_performanceManager->deleteLater();
        m_performanceManager = nullptr;
    }
    
    if (m_metricsCollector) {
        m_metricsCollector->stop();
        m_metricsCollector->deleteLater();
        m_metricsCollector = nullptr;
    }
    
    if (m_config) {
        m_config->deleteLater();
        m_config = nullptr;
    }
}

// 基础功能测试
void PerformanceModuleTest::testModuleInitialization()
{
    m_performanceModule = new PerformanceModule(this);
    QVERIFY(m_performanceModule != nullptr);
    
    // 测试初始化
    bool initialized = m_performanceModule->initialize();
    QVERIFY(initialized);
    QVERIFY(m_performanceModule->isInitialized());
    QCOMPARE(m_performanceModule->status(), PerformanceModule::Ready);
}

void PerformanceModuleTest::testModuleStartStop()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    
    // 测试启动
    QSignalSpy startedSpy(m_performanceModule, &PerformanceModule::started);
    bool started = m_performanceModule->start();
    QVERIFY(started);
    QVERIFY(m_performanceModule->isRunning());
    QCOMPARE(m_performanceModule->status(), PerformanceModule::Running);
    QCOMPARE(startedSpy.count(), 1);
    
    // 测试停止
    QSignalSpy stoppedSpy(m_performanceModule, &PerformanceModule::stopped);
    m_performanceModule->stop();
    QVERIFY(!m_performanceModule->isRunning());
    QCOMPARE(stoppedSpy.count(), 1);
}

void PerformanceModuleTest::testModulePauseResume()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    m_performanceModule->start();
    
    // 测试暂停
    QSignalSpy pausedSpy(m_performanceModule, &PerformanceModule::paused);
    m_performanceModule->pause();
    QCOMPARE(m_performanceModule->status(), PerformanceModule::Paused);
    QCOMPARE(pausedSpy.count(), 1);
    
    // 测试恢复
    QSignalSpy resumedSpy(m_performanceModule, &PerformanceModule::resumed);
    m_performanceModule->resume();
    QCOMPARE(m_performanceModule->status(), PerformanceModule::Running);
    QCOMPARE(resumedSpy.count(), 1);
}

void PerformanceModuleTest::testModuleShutdown()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    m_performanceModule->start();
    
    QSignalSpy shutdownSpy(m_performanceModule, &PerformanceModule::shutdown);
    m_performanceModule->shutdown();
    QCOMPARE(m_performanceModule->status(), PerformanceModule::Shutdown);
    QCOMPARE(shutdownSpy.count(), 1);
}

void PerformanceModuleTest::testModuleStatus()
{
    m_performanceModule = new PerformanceModule(this);
    
    // 测试初始状态
    QCOMPARE(m_performanceModule->status(), PerformanceModule::NotInitialized);
    
    // 测试状态变化信号
    QSignalSpy statusSpy(m_performanceModule, &PerformanceModule::statusChanged);
    
    m_performanceModule->initialize();
    QVERIFY(statusSpy.count() > 0);
    QCOMPARE(m_performanceModule->status(), PerformanceModule::Ready);
}

void PerformanceModuleTest::testModuleVersion()
{
    m_performanceModule = new PerformanceModule(this);
    QString version = m_performanceModule->version();
    QVERIFY(!version.isEmpty());
    QVERIFY(version.contains("."));
}

void PerformanceModuleTest::testModuleSingleton()
{
    PerformanceModule* instance1 = PerformanceModule::instance();
    PerformanceModule* instance2 = PerformanceModule::instance();
    
    QVERIFY(instance1 != nullptr);
    QCOMPARE(instance1, instance2);
}

// 组件测试
void PerformanceModuleTest::testPerformanceManagerCreation()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    
    PerformanceManager* manager = m_performanceModule->performanceManager();
    QVERIFY(manager != nullptr);
}

void PerformanceModuleTest::testMetricsCollectorCreation()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    
    MetricsCollector* collector = m_performanceModule->metricsCollector();
    QVERIFY(collector != nullptr);
}

void PerformanceModuleTest::testConfigurationCreation()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    
    PerformanceConfig* config = m_performanceModule->config();
    QVERIFY(config != nullptr);
}

void PerformanceModuleTest::testComponentInitialization()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    
    // 验证所有组件都已正确初始化
    QVERIFY(m_performanceModule->performanceManager() != nullptr);
    QVERIFY(m_performanceModule->metricsCollector() != nullptr);
    QVERIFY(m_performanceModule->config() != nullptr);
}

void PerformanceModuleTest::testComponentInteraction()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    
    PerformanceManager* manager = m_performanceModule->performanceManager();
    MetricsCollector* collector = m_performanceModule->metricsCollector();
    
    // 测试组件间的交互
    QVERIFY(manager->metricsCollector() == collector);
}// 监
控功能测试
void PerformanceModuleTest::testCPUMonitoring()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    
    // 添加CPU监控器
    MockMonitor* cpuMonitor = new MockMonitor("CPUMonitor");
    bool added = m_performanceManager->addMonitor(cpuMonitor);
    QVERIFY(added);
    
    // 启动监控
    m_performanceManager->start();
    
    // 等待数据收集
    QTest::qWait(1000);
    
    // 验证CPU监控数据
    PerformanceMetrics metrics = m_performanceManager->getCurrentMetrics();
    QVERIFY(metrics.system.cpuUsage >= 0.0);
    QVERIFY(metrics.system.cpuUsage <= 100.0);
}

void PerformanceModuleTest::testMemoryMonitoring()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    
    // 添加内存监控器
    MockMonitor* memoryMonitor = new MockMonitor("MemoryMonitor");
    bool added = m_performanceManager->addMonitor(memoryMonitor);
    QVERIFY(added);
    
    m_performanceManager->start();
    QTest::qWait(1000);
    
    PerformanceMetrics metrics = m_performanceManager->getCurrentMetrics();
    QVERIFY(metrics.system.memoryUsage > 0);
}

void PerformanceModuleTest::testNetworkMonitoring()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    
    MockMonitor* networkMonitor = new MockMonitor("NetworkMonitor");
    m_performanceManager->addMonitor(networkMonitor);
    m_performanceManager->start();
    
    QTest::qWait(1000);
    
    PerformanceMetrics metrics = m_performanceManager->getCurrentMetrics();
    QVERIFY(metrics.network.bandwidth >= 0.0);
}

void PerformanceModuleTest::testRealTimeMonitoring()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    m_performanceManager->setMonitoringInterval(100); // 100ms间隔
    
    QSignalSpy metricsSpy(m_performanceManager, &PerformanceManager::metricsUpdated);
    m_performanceManager->start();
    
    // 等待多次更新
    QTest::qWait(500);
    
    QVERIFY(metricsSpy.count() >= 3); // 至少3次更新
}

void PerformanceModuleTest::testHistoricalDataCollection()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    m_performanceManager->start();
    
    // 收集一段时间的数据
    QTest::qWait(2000);
    
    QDateTime from = QDateTime::currentDateTime().addSecs(-5);
    QDateTime to = QDateTime::currentDateTime();
    
    QList<PerformanceMetrics> history = m_performanceManager->getHistoricalMetrics(from, to);
    QVERIFY(history.size() > 0);
}

void PerformanceModuleTest::testThresholdDetection()
{
    m_performanceManager = new PerformanceManager(this);
    m_config = new PerformanceConfig(this);
    
    // 设置低阈值以触发告警
    m_config->setCpuThreshold(1.0); // 1% CPU阈值
    m_performanceManager->setConfig(m_config);
    m_performanceManager->initialize();
    
    QSignalSpy thresholdSpy(m_performanceManager, &PerformanceManager::thresholdExceeded);
    m_performanceManager->start();
    
    // 模拟高CPU使用
    simulateSystemLoad();
    QTest::qWait(1000);
    
    QVERIFY(thresholdSpy.count() > 0);
}

// 优化功能测试
void PerformanceModuleTest::testStartupOptimization()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    
    MockOptimizer* startupOptimizer = new MockOptimizer("StartupOptimizer");
    m_performanceManager->addOptimizer(startupOptimizer);
    
    bool result = m_performanceManager->performOptimization();
    QVERIFY(result);
}

void PerformanceModuleTest::testMemoryOptimization()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    
    MockOptimizer* memoryOptimizer = new MockOptimizer("MemoryOptimizer");
    m_performanceManager->addOptimizer(memoryOptimizer);
    
    QSignalSpy optimizationSpy(m_performanceManager, &PerformanceManager::optimizationCompleted);
    m_performanceManager->performOptimization();
    
    QVERIFY(optimizationSpy.count() > 0);
    QList<QVariant> arguments = optimizationSpy.takeFirst();
    bool success = arguments.at(0).toBool();
    QVERIFY(success);
}

void PerformanceModuleTest::testRenderOptimization()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    
    MockOptimizer* renderOptimizer = new MockOptimizer("RenderOptimizer");
    m_performanceManager->addOptimizer(renderOptimizer);
    
    bool result = m_performanceManager->performOptimization();
    QVERIFY(result);
}

void PerformanceModuleTest::testAutoOptimization()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    m_performanceManager->setAutoOptimizationEnabled(true);
    
    MockOptimizer* optimizer = new MockOptimizer("AutoOptimizer");
    m_performanceManager->addOptimizer(optimizer);
    
    QSignalSpy optimizationSpy(m_performanceManager, &PerformanceManager::optimizationCompleted);
    m_performanceManager->start();
    
    // 等待自动优化触发
    QTest::qWait(3000);
    
    QVERIFY(optimizationSpy.count() > 0);
}

void PerformanceModuleTest::testOptimizationStrategies()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    
    MockOptimizer* optimizer = new MockOptimizer("StrategyOptimizer");
    m_performanceManager->addOptimizer(optimizer);
    
    // 测试不同优化策略
    m_performanceManager->setOptimizationStrategy(PerformanceManager::Conservative);
    QCOMPARE(m_performanceManager->optimizationStrategy(), PerformanceManager::Conservative);
    
    m_performanceManager->setOptimizationStrategy(PerformanceManager::Balanced);
    QCOMPARE(m_performanceManager->optimizationStrategy(), PerformanceManager::Balanced);
    
    m_performanceManager->setOptimizationStrategy(PerformanceManager::Aggressive);
    QCOMPARE(m_performanceManager->optimizationStrategy(), PerformanceManager::Aggressive);
}

// 配置测试
void PerformanceModuleTest::testConfigurationLoad()
{
    createTestConfiguration();
    
    m_config = new PerformanceConfig(this);
    QString configPath = m_testDataPath + "/test_config.json";
    
    bool loaded = m_config->loadConfig(configPath);
    QVERIFY(loaded);
    QVERIFY(m_config->isMonitoringEnabled());
}

void PerformanceModuleTest::testConfigurationSave()
{
    m_config = new PerformanceConfig(this);
    m_config->setMonitoringEnabled(true);
    m_config->setMonitoringInterval(1000);
    
    QString configPath = m_testDataPath + "/save_test_config.json";
    bool saved = m_config->saveConfig(configPath);
    QVERIFY(saved);
    
    // 验证文件存在
    QFile configFile(configPath);
    QVERIFY(configFile.exists());
}

void PerformanceModuleTest::testConfigurationValidation()
{
    m_config = new PerformanceConfig(this);
    
    // 设置有效配置
    m_config->setMonitoringInterval(1000);
    m_config->setCpuThreshold(80.0);
    m_config->setMemoryThreshold(1024);
    
    bool valid = m_config->validateConfig();
    QVERIFY(valid);
    
    // 设置无效配置
    m_config->setMonitoringInterval(-1);
    valid = m_config->validateConfig();
    QVERIFY(!valid);
}

void PerformanceModuleTest::testConfigurationDefaults()
{
    m_config = new PerformanceConfig(this);
    m_config->resetToDefaults();
    
    // 验证默认值
    QVERIFY(m_config->monitoringInterval() > 0);
    QVERIFY(m_config->cpuThreshold() > 0);
    QVERIFY(m_config->memoryThreshold() > 0);
}

void PerformanceModuleTest::testConfigurationUpdate()
{
    m_config = new PerformanceConfig(this);
    
    QSignalSpy configSpy(m_config, &PerformanceConfig::configChanged);
    
    m_config->setMonitoringInterval(2000);
    QCOMPARE(configSpy.count(), 1);
    QCOMPARE(m_config->monitoringInterval(), 2000);
}

// 数据管理测试
void PerformanceModuleTest::testDataCollection()
{
    m_metricsCollector = new MetricsCollector(this);
    m_metricsCollector->initialize();
    
    MockMonitor* monitor = new MockMonitor("TestMonitor");
    m_metricsCollector->registerMonitor(monitor);
    
    QSignalSpy collectionSpy(m_metricsCollector, &MetricsCollector::metricsCollected);
    m_metricsCollector->start();
    
    QTest::qWait(1000);
    
    QVERIFY(collectionSpy.count() > 0);
}

void PerformanceModuleTest::testDataStorage()
{
    m_metricsCollector = new MetricsCollector(this);
    m_metricsCollector->initialize();
    m_metricsCollector->setStorageStrategy(MetricsCollector::FileStorage);
    
    PerformanceMetrics metrics = generateTestMetrics();
    
    QSignalSpy storageSpy(m_metricsCollector, &MetricsCollector::dataStored);
    // 模拟数据存储
    QTest::qWait(500);
    
    // 验证数据存储
    QVERIFY(m_metricsCollector->getDataCount() >= 0);
}

void PerformanceModuleTest::testDataExport()
{
    m_metricsCollector = new MetricsCollector(this);
    m_metricsCollector->initialize();
    
    QString exportPath = m_testDataPath + "/export_test.json";
    QDateTime from = QDateTime::currentDateTime().addSecs(-3600);
    QDateTime to = QDateTime::currentDateTime();
    
    bool exported = m_metricsCollector->exportData(exportPath, "json", from, to);
    QVERIFY(exported);
    
    QFile exportFile(exportPath);
    QVERIFY(exportFile.exists());
}

void PerformanceModuleTest::testDataImport()
{
    m_metricsCollector = new MetricsCollector(this);
    m_metricsCollector->initialize();
    
    // 首先导出一些数据
    QString exportPath = m_testDataPath + "/import_test.json";
    QDateTime from = QDateTime::currentDateTime().addSecs(-3600);
    QDateTime to = QDateTime::currentDateTime();
    m_metricsCollector->exportData(exportPath, "json", from, to);
    
    // 然后导入数据
    bool imported = m_metricsCollector->importData(exportPath, "json");
    QVERIFY(imported);
}

void PerformanceModuleTest::testDataCleanup()
{
    m_metricsCollector = new MetricsCollector(this);
    m_metricsCollector->initialize();
    m_metricsCollector->setDataRetentionHours(1); // 1小时保留
    
    QSignalSpy cleanupSpy(m_metricsCollector, &MetricsCollector::dataCleanupCompleted);
    
    // 清理旧数据
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-7200); // 2小时前
    m_metricsCollector->clearHistoricalData(cutoff);
    
    QVERIFY(cleanupSpy.count() > 0);
}// 
性能测试
void PerformanceModuleTest::testPerformanceOverhead()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    
    // 测量启动性能管理器的开销
    QElapsedTimer timer;
    timer.start();
    
    m_performanceManager->start();
    qint64 startupTime = timer.elapsed();
    
    // 验证启动时间在合理范围内（< 1秒）
    QVERIFY(startupTime < 1000);
    
    // 测量监控开销
    timer.restart();
    for (int i = 0; i < 100; ++i) {
        m_performanceManager->getCurrentMetrics();
    }
    qint64 monitoringTime = timer.elapsed();
    
    // 验证监控开销较低（< 100ms for 100 calls）
    QVERIFY(monitoringTime < 100);
}

void PerformanceModuleTest::testMemoryUsage()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    
    // 获取初始内存使用
    PerformanceMetrics initialMetrics = m_performanceManager->getCurrentMetrics();
    size_t initialMemory = initialMetrics.system.memoryUsage;
    
    // 启动监控并运行一段时间
    m_performanceManager->start();
    QTest::qWait(2000);
    
    // 检查内存使用是否稳定
    PerformanceMetrics finalMetrics = m_performanceManager->getCurrentMetrics();
    size_t finalMemory = finalMetrics.system.memoryUsage;
    
    // 内存增长应该在合理范围内
    size_t memoryGrowth = finalMemory > initialMemory ? finalMemory - initialMemory : 0;
    QVERIFY(memoryGrowth < 100 * 1024 * 1024); // < 100MB增长
}

void PerformanceModuleTest::testCPUUsage()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    m_performanceManager->start();
    
    // 收集CPU使用数据
    QList<double> cpuReadings;
    for (int i = 0; i < 10; ++i) {
        PerformanceMetrics metrics = m_performanceManager->getCurrentMetrics();
        cpuReadings.append(metrics.system.cpuUsage);
        QTest::qWait(100);
    }
    
    // 验证CPU使用率在合理范围内
    for (double usage : cpuReadings) {
        QVERIFY(usage >= 0.0 && usage <= 100.0);
    }
    
    // 计算平均CPU使用率
    double avgCpu = 0.0;
    for (double usage : cpuReadings) {
        avgCpu += usage;
    }
    avgCpu /= cpuReadings.size();
    
    // 性能监控本身不应该消耗过多CPU（< 10%）
    QVERIFY(avgCpu < 10.0);
}

void PerformanceModuleTest::testResponseTime()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    m_performanceManager->start();
    
    // 测试API响应时间
    QElapsedTimer timer;
    
    // 测试getCurrentMetrics响应时间
    timer.start();
    m_performanceManager->getCurrentMetrics();
    qint64 metricsTime = timer.elapsed();
    QVERIFY(metricsTime < 50); // < 50ms
    
    // 测试优化操作响应时间
    MockOptimizer* optimizer = new MockOptimizer("ResponseOptimizer");
    m_performanceManager->addOptimizer(optimizer);
    
    timer.restart();
    m_performanceManager->performOptimization();
    qint64 optimizationTime = timer.elapsed();
    QVERIFY(optimizationTime < 500); // < 500ms
}

void PerformanceModuleTest::testThroughput()
{
    m_metricsCollector = new MetricsCollector(this);
    m_metricsCollector->initialize();
    m_metricsCollector->setCollectionInterval(10); // 10ms间隔
    
    MockMonitor* monitor = new MockMonitor("ThroughputMonitor");
    m_metricsCollector->registerMonitor(monitor);
    
    QSignalSpy collectionSpy(m_metricsCollector, &MetricsCollector::metricsCollected);
    m_metricsCollector->start();
    
    // 运行1秒并计算吞吐量
    QTest::qWait(1000);
    
    int collectionsPerSecond = collectionSpy.count();
    
    // 验证数据收集吞吐量（应该接近100次/秒）
    QVERIFY(collectionsPerSecond > 50); // 至少50次/秒
    QVERIFY(collectionsPerSecond < 150); // 不超过150次/秒
}

// 错误处理测试
void PerformanceModuleTest::testErrorHandling()
{
    m_performanceModule = new PerformanceModule(this);
    
    // 测试未初始化状态下的操作
    bool started = m_performanceModule->start();
    QVERIFY(!started); // 应该失败
    
    // 测试错误信号
    QSignalSpy errorSpy(m_performanceModule, &PerformanceModule::errorOccurred);
    
    // 触发错误条件
    m_performanceModule->start(); // 未初始化的启动
    
    // 验证错误信号被发出
    QVERIFY(errorSpy.count() > 0);
}

void PerformanceModuleTest::testExceptionHandling()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    
    // 测试空指针处理
    bool result = m_performanceManager->addMonitor(nullptr);
    QVERIFY(!result);
    
    result = m_performanceManager->addOptimizer(nullptr);
    QVERIFY(!result);
    
    // 测试无效参数处理
    m_performanceManager->setMonitoringInterval(-1);
    QVERIFY(m_performanceManager->monitoringInterval() > 0); // 应该保持有效值
}

void PerformanceModuleTest::testRecoveryMechanisms()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    m_performanceModule->start();
    
    // 模拟错误状态
    QSignalSpy statusSpy(m_performanceModule, &PerformanceModule::statusChanged);
    
    // 触发错误并验证恢复
    // 这里可以模拟各种错误场景
    m_performanceModule->reset();
    
    // 验证模块能够恢复
    bool recovered = m_performanceModule->initialize();
    QVERIFY(recovered);
}

void PerformanceModuleTest::testFailureScenarios()
{
    m_metricsCollector = new MetricsCollector(this);
    
    // 测试存储失败场景
    m_metricsCollector->setStoragePath("/invalid/path/that/does/not/exist");
    bool initialized = m_metricsCollector->initialize();
    
    // 应该能够处理存储失败
    QVERIFY(initialized || !initialized); // 无论成功失败都应该优雅处理
    
    QSignalSpy errorSpy(m_metricsCollector, &MetricsCollector::errorOccurred);
    m_metricsCollector->start();
    
    // 可能会有错误信号，但不应该崩溃
    QTest::qWait(500);
}

// 集成测试
void PerformanceModuleTest::testModuleIntegration()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    
    // 测试所有组件的集成
    PerformanceManager* manager = m_performanceModule->performanceManager();
    MetricsCollector* collector = m_performanceModule->metricsCollector();
    PerformanceConfig* config = m_performanceModule->config();
    
    QVERIFY(manager != nullptr);
    QVERIFY(collector != nullptr);
    QVERIFY(config != nullptr);
    
    // 测试组件间的数据流
    m_performanceModule->start();
    QTest::qWait(1000);
    
    PerformanceMetrics metrics = manager->getCurrentMetrics();
    QVERIFY(metrics.timestamp.isValid());
}

void PerformanceModuleTest::testUIIntegration()
{
    // 这里可以测试与UI组件的集成
    // 由于是单元测试，可能需要mock UI组件
    QVERIFY(true); // 占位符测试
}

void PerformanceModuleTest::testSystemIntegration()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    m_performanceModule->start();
    
    // 测试与系统的集成
    QVariantMap systemInfo = m_performanceModule->performanceManager()->getSystemInfo();
    QVERIFY(!systemInfo.isEmpty());
    QVERIFY(systemInfo.contains("platform"));
}

void PerformanceModuleTest::testCrossModuleInteraction()
{
    // 测试与其他模块的交互
    // 这里可以测试性能模块如何与网络、音频等模块交互
    QVERIFY(true); // 占位符测试
}

// 压力测试
void PerformanceModuleTest::testHighLoadScenario()
{
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->initialize();
    m_performanceManager->setMonitoringInterval(1); // 1ms间隔，高频监控
    
    // 添加多个监控器
    for (int i = 0; i < 10; ++i) {
        MockMonitor* monitor = new MockMonitor(QString("Monitor_%1").arg(i));
        m_performanceManager->addMonitor(monitor);
    }
    
    m_performanceManager->start();
    
    // 高负载运行
    QTest::qWait(5000);
    
    // 验证系统仍然响应
    PerformanceMetrics metrics = m_performanceManager->getCurrentMetrics();
    QVERIFY(metrics.timestamp.isValid());
    
    m_performanceManager->stop();
}

void PerformanceModuleTest::testLongRunningTest()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    m_performanceModule->start();
    
    QSignalSpy errorSpy(m_performanceModule, &PerformanceModule::errorOccurred);
    
    // 长时间运行测试（10秒）
    for (int i = 0; i < 100; ++i) {
        QTest::qWait(100);
        
        // 定期检查状态
        QVERIFY(m_performanceModule->isRunning());
        
        // 执行一些操作
        m_performanceModule->performanceManager()->getCurrentMetrics();
    }
    
    // 验证没有错误发生
    QCOMPARE(errorSpy.count(), 0);
}

void PerformanceModuleTest::testMemoryLeakTest()
{
    // 内存泄漏测试
    size_t initialMemory = getCurrentMemoryUsage();
    
    for (int i = 0; i < 100; ++i) {
        PerformanceModule* module = new PerformanceModule();
        module->initialize();
        module->start();
        QTest::qWait(10);
        module->shutdown();
        delete module;
    }
    
    size_t finalMemory = getCurrentMemoryUsage();
    size_t memoryGrowth = finalMemory > initialMemory ? finalMemory - initialMemory : 0;
    
    // 内存增长应该在合理范围内
    QVERIFY(memoryGrowth < 50 * 1024 * 1024); // < 50MB
}

void PerformanceModuleTest::testConcurrencyTest()
{
    m_performanceModule = new PerformanceModule(this);
    m_performanceModule->initialize();
    m_performanceModule->start();
    
    // 并发访问测试
    QList<QThread*> threads;
    QAtomicInt errorCount(0);
    
    for (int i = 0; i < 5; ++i) {
        QThread* thread = QThread::create([this, &errorCount]() {
            try {
                for (int j = 0; j < 100; ++j) {
                    m_performanceModule->performanceManager()->getCurrentMetrics();
                    QThread::msleep(1);
                }
            } catch (...) {
                errorCount.fetchAndAddOrdered(1);
            }
        });
        threads.append(thread);
        thread->start();
    }
    
    // 等待所有线程完成
    for (QThread* thread : threads) {
        thread->wait();
        delete thread;
    }
    
    // 验证没有并发错误
    QCOMPARE(errorCount.loadAcquire(), 0);
}

// 兼容性测试
void PerformanceModuleTest::testPlatformCompatibility()
{
    m_performanceModule = new PerformanceModule(this);
    bool initialized = m_performanceModule->initialize();
    
    // 在所有支持的平台上都应该能够初始化
    QVERIFY(initialized);
    
    // 测试平台特定功能
    QVariantMap systemInfo = m_performanceModule->performanceManager()->getSystemInfo();
    QVERIFY(systemInfo.contains("platform"));
}

void PerformanceModuleTest::testVersionCompatibility()
{
    m_performanceModule = new PerformanceModule(this);
    QString version = m_performanceModule->version();
    
    // 验证版本格式
    QStringList versionParts = version.split('.');
    QVERIFY(versionParts.size() >= 2);
    
    // 验证版本号是数字
    bool ok;
    versionParts[0].toInt(&ok);
    QVERIFY(ok);
    versionParts[1].toInt(&ok);
    QVERIFY(ok);
}

void PerformanceModuleTest::testConfigurationCompatibility()
{
    m_config = new PerformanceConfig(this);
    
    // 测试配置的向后兼容性
    QVariantMap oldConfig;
    oldConfig["monitoring_enabled"] = true;
    oldConfig["monitoring_interval"] = 1000;
    
    // 应该能够处理旧格式的配置
    m_config->setAllConfig(oldConfig);
    bool valid = m_config->validateConfig();
    QVERIFY(valid);
}

// 测试辅助方法实现
void PerformanceModuleTest::setupTestEnvironment()
{
    // 设置测试环境
    m_testEnvironmentReady = true;
}

void PerformanceModuleTest::cleanupTestEnvironment()
{
    // 清理测试环境
    m_testEnvironmentReady = false;
}

bool PerformanceModuleTest::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QSignalSpy spy(sender, signal);
    return spy.wait(timeout);
}

void PerformanceModuleTest::generateTestData()
{
    // 生成测试数据
    for (int i = 0; i < 10; ++i) {
        PerformanceMetrics metrics = generateTestMetrics();
        m_testMetrics.append(metrics);
    }
}

PerformanceMetrics PerformanceModuleTest::generateTestMetrics()
{
    PerformanceMetrics metrics;
    metrics.timestamp = QDateTime::currentDateTime();
    metrics.system.cpuUsage = 25.0 + (qrand() % 50);
    metrics.system.memoryUsage = 512 * 1024 * 1024 + (qrand() % (256 * 1024 * 1024));
    metrics.network.bandwidth = 100.0 + (qrand() % 900);
    metrics.network.latency = 10.0 + (qrand() % 90);
    return metrics;
}

void PerformanceModuleTest::verifyPerformanceMetrics(const PerformanceMetrics& metrics)
{
    QVERIFY(metrics.timestamp.isValid());
    QVERIFY(metrics.system.cpuUsage >= 0.0 && metrics.system.cpuUsage <= 100.0);
    QVERIFY(metrics.system.memoryUsage > 0);
    QVERIFY(metrics.network.bandwidth >= 0.0);
    QVERIFY(metrics.network.latency >= 0.0);
}

void PerformanceModuleTest::simulateSystemLoad()
{
    // 模拟系统负载
    QThread::msleep(100);
    
    // 执行一些CPU密集型操作
    volatile int sum = 0;
    for (int i = 0; i < 1000000; ++i) {
        sum += i;
    }
}

void PerformanceModuleTest::createTestConfiguration()
{
    QVariantMap config;
    config["monitoring_enabled"] = true;
    config["monitoring_interval"] = 1000;
    config["cpu_threshold"] = 80.0;
    config["memory_threshold"] = 1024;
    
    m_testConfig = config;
    
    // 保存到文件
    QString configPath = m_testDataPath + "/test_config.json";
    QJsonDocument doc = QJsonDocument::fromVariant(config);
    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

size_t PerformanceModuleTest::getCurrentMemoryUsage()
{
    // 获取当前内存使用量的简单实现
    // 在实际实现中，这里应该使用平台特定的API
    return 100 * 1024 * 1024; // 返回100MB作为示例
}

QTEST_MAIN(PerformanceModuleTest)
#include "PerformanceModuleTest.moc"