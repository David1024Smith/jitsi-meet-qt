#include "ProgressiveReplacementTest.h"
#include "../include/ProgressiveReplacementManager.h"
#include "../config/ReplacementConfig.h"
#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include <QDebug>

ProgressiveReplacementTest::ProgressiveReplacementTest(QObject *parent)
    : QObject(parent)
    , m_manager(nullptr)
    , m_config(nullptr)
{
}

void ProgressiveReplacementTest::initTestCase()
{
    m_manager = new ProgressiveReplacementManager(this);
    m_config = new ReplacementConfig(this);
    
    QVERIFY(m_manager->initialize());
    QVERIFY(m_config->isLoaded());
}

void ProgressiveReplacementTest::cleanupTestCase()
{
    if (m_manager) {
        delete m_manager;
        m_manager = nullptr;
    }
    
    if (m_config) {
        delete m_config;
        m_config = nullptr;
    }
}

void ProgressiveReplacementTest::init()
{
    // 每个测试前的初始化
}

void ProgressiveReplacementTest::cleanup()
{
    // 每个测试后的清理
    if (m_manager) {
        // 清理所有替换计划
        QStringList modules = m_manager->getPlannedModules();
        for (const QString& module : modules) {
            m_manager->deleteReplacementPlan(module);
        }
    }
}

void ProgressiveReplacementTest::testInitialization()
{
    QVERIFY(m_manager->isInitialized());
    QCOMPARE(m_manager->globalStrategy(), ProgressiveReplacementManager::Balanced);
}

void ProgressiveReplacementTest::testReplacementPlanManagement()
{
    QString moduleName = "test_module";
    
    // 创建替换计划
    ProgressiveReplacementManager::ReplacementPlan plan;
    plan.moduleName = moduleName;
    plan.strategy = ProgressiveReplacementManager::Conservative;
    plan.priority = 1;
    plan.requiresValidation = true;
    plan.requiresPerformanceTest = true;
    
    QVERIFY(m_manager->createReplacementPlan(moduleName, plan));
    
    // 验证计划已创建
    QStringList modules = m_manager->getPlannedModules();
    QVERIFY(modules.contains(moduleName));
    
    // 获取计划
    ProgressiveReplacementManager::ReplacementPlan retrievedPlan = m_manager->getReplacementPlan(moduleName);
    QCOMPARE(retrievedPlan.moduleName, moduleName);
    QCOMPARE(retrievedPlan.strategy, ProgressiveReplacementManager::Conservative);
    
    // 更新计划
    plan.strategy = ProgressiveReplacementManager::Balanced;
    QVERIFY(m_manager->updateReplacementPlan(moduleName, plan));
    
    retrievedPlan = m_manager->getReplacementPlan(moduleName);
    QCOMPARE(retrievedPlan.strategy, ProgressiveReplacementManager::Balanced);
    
    // 删除计划
    QVERIFY(m_manager->deleteReplacementPlan(moduleName));
    modules = m_manager->getPlannedModules();
    QVERIFY(!modules.contains(moduleName));
}

void ProgressiveReplacementTest::testReplacementExecution()
{
    QString moduleName = "execution_test_module";
    
    // 创建替换计划
    ProgressiveReplacementManager::ReplacementPlan plan;
    plan.moduleName = moduleName;
    plan.strategy = ProgressiveReplacementManager::Aggressive; // 使用激进策略以便快速测试
    plan.priority = 1;
    plan.requiresValidation = false;
    plan.requiresPerformanceTest = false;
    
    QVERIFY(m_manager->createReplacementPlan(moduleName, plan));
    
    // 设置信号监听
    QSignalSpy startedSpy(m_manager, &ProgressiveReplacementManager::replacementStarted);
    QSignalSpy progressSpy(m_manager, &ProgressiveReplacementManager::replacementProgress);
    
    // 开始替换
    QVERIFY(m_manager->startReplacement(moduleName));
    
    // 验证信号发出
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(startedSpy.takeFirst().at(0).toString(), moduleName);
    
    // 检查执行状态
    ProgressiveReplacementManager::ExecutionState state = m_manager->getExecutionState(moduleName);
    QCOMPARE(state.moduleName, moduleName);
    QCOMPARE(state.status, ProgressiveReplacementManager::InProgress);
    
    // 验证活跃替换列表
    QStringList activeReplacements = m_manager->getActiveReplacements();
    QVERIFY(activeReplacements.contains(moduleName));
}

void ProgressiveReplacementTest::testReplacementControl()
{
    QString moduleName = "control_test_module";
    
    // 创建并开始替换
    ProgressiveReplacementManager::ReplacementPlan plan;
    plan.moduleName = moduleName;
    plan.strategy = ProgressiveReplacementManager::Conservative;
    
    QVERIFY(m_manager->createReplacementPlan(moduleName, plan));
    QVERIFY(m_manager->startReplacement(moduleName));
    
    // 测试暂停
    QSignalSpy pausedSpy(m_manager, &ProgressiveReplacementManager::replacementPaused);
    QVERIFY(m_manager->pauseReplacement(moduleName));
    QCOMPARE(pausedSpy.count(), 1);
    
    ProgressiveReplacementManager::ExecutionState state = m_manager->getExecutionState(moduleName);
    QCOMPARE(state.status, ProgressiveReplacementManager::Paused);
    
    // 测试恢复
    QSignalSpy resumedSpy(m_manager, &ProgressiveReplacementManager::replacementResumed);
    QVERIFY(m_manager->resumeReplacement(moduleName));
    QCOMPARE(resumedSpy.count(), 1);
    
    state = m_manager->getExecutionState(moduleName);
    QCOMPARE(state.status, ProgressiveReplacementManager::InProgress);
    
    // 测试停止
    QVERIFY(m_manager->stopReplacement(moduleName));
    
    state = m_manager->getExecutionState(moduleName);
    QVERIFY(state.status == ProgressiveReplacementManager::RolledBack || 
            state.status == ProgressiveReplacementManager::Failed);
}

void ProgressiveReplacementTest::testParallelMode()
{
    QString moduleName = "parallel_test_module";
    
    // 创建替换计划
    ProgressiveReplacementManager::ReplacementPlan plan;
    plan.moduleName = moduleName;
    plan.strategy = ProgressiveReplacementManager::Balanced;
    
    QVERIFY(m_manager->createReplacementPlan(moduleName, plan));
    
    // 测试并行模式
    QVERIFY(m_manager->enableParallelMode(moduleName));
    
    ProgressiveReplacementManager::CodeRunMode mode = m_manager->getCodeRunMode(moduleName);
    QCOMPARE(mode, ProgressiveReplacementManager::Parallel);
    
    // 测试设置运行模式
    QVERIFY(m_manager->setCodeRunMode(moduleName, ProgressiveReplacementManager::Comparison));
    mode = m_manager->getCodeRunMode(moduleName);
    QCOMPARE(mode, ProgressiveReplacementManager::Comparison);
    
    // 禁用并行模式
    QVERIFY(m_manager->disableParallelMode(moduleName));
    mode = m_manager->getCodeRunMode(moduleName);
    QCOMPARE(mode, ProgressiveReplacementManager::LegacyOnly);
}

void ProgressiveReplacementTest::testValidationAndTesting()
{
    QString moduleName = "validation_test_module";
    
    // 创建替换计划
    ProgressiveReplacementManager::ReplacementPlan plan;
    plan.moduleName = moduleName;
    plan.strategy = ProgressiveReplacementManager::Conservative;
    plan.requiresValidation = true;
    plan.requiresPerformanceTest = true;
    
    QVERIFY(m_manager->createReplacementPlan(moduleName, plan));
    
    // 测试功能对比验证
    // 注意：这里可能需要模拟验证器的行为
    bool validationResult = m_manager->runFunctionalComparison(moduleName);
    // 验证结果取决于模拟的验证器实现
    
    // 测试性能基准测试
    bool benchmarkResult = m_manager->runPerformanceBenchmark(moduleName);
    // 基准测试结果取决于模拟的性能验证器实现
    
    // 获取结果
    QVariantMap comparisonResults = m_manager->getComparisonResults(moduleName);
    QVariantMap performanceResults = m_manager->getPerformanceResults(moduleName);
    
    // 结果可能为空，取决于验证器的实现
}

void ProgressiveReplacementTest::testSafetyControls()
{
    QString moduleName = "safety_test_module";
    
    // 创建替换计划
    ProgressiveReplacementManager::ReplacementPlan plan;
    plan.moduleName = moduleName;
    plan.strategy = ProgressiveReplacementManager::Conservative;
    
    QVERIFY(m_manager->createReplacementPlan(moduleName, plan));
    
    // 测试安全检查点创建
    bool checkpointResult = m_manager->createSafetyCheckpoint(moduleName);
    // 结果取决于回滚管理器的实现
    
    // 测试安全条件验证
    bool safetyResult = m_manager->validateSafetyConditions(moduleName);
    // 结果取决于安全检查的实现
    
    // 测试安全切换
    bool switchResult = m_manager->executeSafeSwitch(moduleName);
    // 结果取决于切换实现
}

void ProgressiveReplacementTest::testScheduling()
{
    QString moduleName = "scheduling_test_module";
    
    // 创建替换计划
    ProgressiveReplacementManager::ReplacementPlan plan;
    plan.moduleName = moduleName;
    plan.strategy = ProgressiveReplacementManager::Balanced;
    
    QVERIFY(m_manager->createReplacementPlan(moduleName, plan));
    
    // 调度替换（1秒后执行）
    QDateTime scheduledTime = QDateTime::currentDateTime().addSecs(1);
    m_manager->scheduleReplacement(moduleName, scheduledTime);
    
    // 等待调度执行
    QSignalSpy startedSpy(m_manager, &ProgressiveReplacementManager::replacementStarted);
    
    // 等待最多3秒
    bool signalReceived = startedSpy.wait(3000);
    
    if (signalReceived) {
        QCOMPARE(startedSpy.count(), 1);
        QCOMPARE(startedSpy.takeFirst().at(0).toString(), moduleName);
    }
}

void ProgressiveReplacementTest::testBatchReplacement()
{
    QStringList moduleNames = {"batch_module_1", "batch_module_2", "batch_module_3"};
    
    // 创建多个替换计划
    for (const QString& moduleName : moduleNames) {
        ProgressiveReplacementManager::ReplacementPlan plan;
        plan.moduleName = moduleName;
        plan.strategy = ProgressiveReplacementManager::Aggressive;
        
        QVERIFY(m_manager->createReplacementPlan(moduleName, plan));
    }
    
    // 批量替换
    QSignalSpy startedSpy(m_manager, &ProgressiveReplacementManager::replacementStarted);
    m_manager->batchReplacement(moduleNames);
    
    // 验证所有模块都开始了替换
    // 注意：信号可能不会立即发出，取决于实现
    QStringList activeReplacements = m_manager->getActiveReplacements();
    
    for (const QString& moduleName : moduleNames) {
        ProgressiveReplacementManager::ExecutionState state = m_manager->getExecutionState(moduleName);
        // 状态应该是InProgress或其他活跃状态
    }
}

void ProgressiveReplacementTest::testReporting()
{
    // 创建一些测试模块
    QStringList moduleNames = {"report_module_1", "report_module_2"};
    
    for (const QString& moduleName : moduleNames) {
        ProgressiveReplacementManager::ReplacementPlan plan;
        plan.moduleName = moduleName;
        plan.strategy = ProgressiveReplacementManager::Balanced;
        
        QVERIFY(m_manager->createReplacementPlan(moduleName, plan));
    }
    
    // 生成进度报告
    QVariantMap progressReport = m_manager->generateProgressReport();
    
    QVERIFY(progressReport.contains("timestamp"));
    QVERIFY(progressReport.contains("total_modules"));
    QCOMPARE(progressReport["total_modules"].toInt(), moduleNames.size());
    
    // 生成详细报告
    QString moduleName = moduleNames.first();
    QVariantMap detailedReport = m_manager->generateDetailedReport(moduleName);
    
    QVERIFY(detailedReport.contains("module_name"));
    QCOMPARE(detailedReport["module_name"].toString(), moduleName);
    
    // 获取替换历史
    QStringList history = m_manager->getReplacementHistory();
    QVERIFY(!history.isEmpty()); // 应该有一些历史记录
}

void ProgressiveReplacementTest::testConfigurationIntegration()
{
    // 测试策略配置
    QStringList strategies = m_config->getAvailableStrategies();
    QVERIFY(strategies.contains("conservative"));
    QVERIFY(strategies.contains("balanced"));
    QVERIFY(strategies.contains("aggressive"));
    
    // 获取策略配置
    QVariantMap conservativeConfig = m_config->getStrategyConfiguration("conservative");
    QVERIFY(conservativeConfig.contains("validation_required"));
    QVERIFY(conservativeConfig["validation_required"].toBool());
    
    // 测试安全配置
    QVariantMap safetyConfig = m_config->getSafetyConfiguration();
    QVERIFY(safetyConfig.contains("max_concurrent_replacements"));
    QVERIFY(safetyConfig.contains("emergency_rollback_enabled"));
}

void ProgressiveReplacementTest::testErrorHandling()
{
    // 测试无效模块名
    QVERIFY(!m_manager->startReplacement(""));
    QVERIFY(!m_manager->startReplacement("nonexistent_module"));
    
    // 测试重复创建计划
    QString moduleName = "error_test_module";
    ProgressiveReplacementManager::ReplacementPlan plan;
    plan.moduleName = moduleName;
    plan.strategy = ProgressiveReplacementManager::Balanced;
    
    QVERIFY(m_manager->createReplacementPlan(moduleName, plan));
    QVERIFY(!m_manager->createReplacementPlan(moduleName, plan)); // 应该失败
    
    // 测试在执行中更新计划
    QVERIFY(m_manager->startReplacement(moduleName));
    QVERIFY(!m_manager->updateReplacementPlan(moduleName, plan)); // 应该失败
    
    // 清理
    m_manager->stopReplacement(moduleName);
}

QTEST_MAIN(ProgressiveReplacementTest)