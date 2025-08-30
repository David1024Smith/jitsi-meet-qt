#include "CompatibilityModuleTest.h"
#include "CompatibilityModule.h"
#include "LegacyCompatibilityAdapter.h"
#include "RollbackManager.h"
#include "CompatibilityValidator.h"
#include "CompatibilityConfig.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QSignalSpy>

void CompatibilityModuleTest::initTestCase()
{
    qDebug() << "Initializing CompatibilityModuleTest...";
    
    // 设置测试数据目录
    m_testDataDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/compatibility_test";
    QDir().mkpath(m_testDataDir);
    
    m_module = nullptr;
}

void CompatibilityModuleTest::cleanupTestCase()
{
    qDebug() << "Cleaning up CompatibilityModuleTest...";
    
    if (m_module) {
        delete m_module;
        m_module = nullptr;
    }
    
    // 清理测试数据目录
    QDir testDir(m_testDataDir);
    if (testDir.exists()) {
        testDir.removeRecursively();
    }
}

void CompatibilityModuleTest::init()
{
    // 每个测试前创建新的模块实例
    m_module = new CompatibilityModule();
}

void CompatibilityModuleTest::cleanup()
{
    // 每个测试后清理
    if (m_module) {
        delete m_module;
        m_module = nullptr;
    }
}

void CompatibilityModuleTest::testModuleInitialization()
{
    QVERIFY(m_module != nullptr);
    QVERIFY(!m_module->isInitialized());
    
    QSignalSpy initSpy(m_module, &CompatibilityModule::moduleInitialized);
    
    bool result = m_module->initialize();
    QVERIFY(result);
    QVERIFY(m_module->isInitialized());
    QCOMPARE(initSpy.count(), 1);
}

void CompatibilityModuleTest::testModuleInfo()
{
    QVERIFY(m_module->initialize());
    
    QString name = m_module->getModuleName();
    QCOMPARE(name, QString("CompatibilityModule"));
    
    QString version = m_module->getModuleVersion();
    QVERIFY(!version.isEmpty());
    
    QVariantMap info = m_module->getModuleInfo();
    QVERIFY(info.contains("name"));
    QVERIFY(info.contains("version"));
    QVERIFY(info.contains("description"));
    QVERIFY(info.contains("initialized"));
    QCOMPARE(info["initialized"].toBool(), true);
}

void CompatibilityModuleTest::testComponentAccess()
{
    QVERIFY(m_module->initialize());
    
    LegacyCompatibilityAdapter* adapter = m_module->getAdapter();
    QVERIFY(adapter != nullptr);
    QVERIFY(adapter->isInitialized());
    
    RollbackManager* rollback = m_module->getRollbackManager();
    QVERIFY(rollback != nullptr);
    
    CompatibilityValidator* validator = m_module->getValidator();
    QVERIFY(validator != nullptr);
    
    CompatibilityConfig* config = m_module->getConfig();
    QVERIFY(config != nullptr);
}

void CompatibilityModuleTest::testAdapterCreation()
{
    QVERIFY(m_module->initialize());
    
    // 测试静态工厂方法
    auto mediaManager = LegacyCompatibilityAdapter::createLegacyMediaManager();
    QVERIFY(mediaManager != nullptr);
    delete mediaManager;
    
    auto chatManager = LegacyCompatibilityAdapter::createLegacyChatManager();
    QVERIFY(chatManager != nullptr);
    delete chatManager;
    
    auto screenShareManager = LegacyCompatibilityAdapter::createLegacyScreenShareManager();
    QVERIFY(screenShareManager != nullptr);
    delete screenShareManager;
    
    auto conferenceManager = LegacyCompatibilityAdapter::createLegacyConferenceManager();
    QVERIFY(conferenceManager != nullptr);
    delete conferenceManager;
}

void CompatibilityModuleTest::testAdapterValidation()
{
    QVERIFY(m_module->initialize());
    
    // 测试功能验证
    bool audioValid = LegacyCompatibilityAdapter::validateFunctionality("audio");
    QVERIFY(audioValid); // 应该通过，因为我们使用模拟实现
    
    bool networkValid = LegacyCompatibilityAdapter::validateFunctionality("network");
    QVERIFY(networkValid);
    
    bool unknownValid = LegacyCompatibilityAdapter::validateFunctionality("unknown_module");
    QVERIFY(!unknownValid); // 未知模块应该失败
}

void CompatibilityModuleTest::testAdapterConfiguration()
{
    QVERIFY(m_module->initialize());
    
    LegacyCompatibilityAdapter* adapter = m_module->getAdapter();
    QVERIFY(adapter != nullptr);
    
    // 测试全局配置
    QVariantMap globalConfig;
    globalConfig["test_setting"] = "test_value";
    globalConfig["validation_enabled"] = true;
    
    adapter->setGlobalConfig(globalConfig);
    QVariantMap retrievedConfig = adapter->getGlobalConfig();
    
    QCOMPARE(retrievedConfig["test_setting"].toString(), QString("test_value"));
    QCOMPARE(retrievedConfig["validation_enabled"].toBool(), true);
}

void CompatibilityModuleTest::testCheckpointCreation()
{
    QVERIFY(m_module->initialize());
    
    RollbackManager* rollback = m_module->getRollbackManager();
    QVERIFY(rollback != nullptr);
    
    // 设置测试目录
    rollback->setCheckpointDirectory(m_testDataDir + "/checkpoints");
    
    QSignalSpy createdSpy(rollback, &RollbackManager::checkpointCreated);
    
    QString checkpointName = "test_checkpoint_1";
    bool result = rollback->createCheckpoint(checkpointName, "Test checkpoint");
    
    QVERIFY(result);
    QCOMPARE(createdSpy.count(), 1);
    
    QStringList checkpoints = rollback->availableCheckpoints();
    QVERIFY(checkpoints.contains(checkpointName));
}

void CompatibilityModuleTest::testCheckpointValidation()
{
    QVERIFY(m_module->initialize());
    
    RollbackManager* rollback = m_module->getRollbackManager();
    rollback->setCheckpointDirectory(m_testDataDir + "/checkpoints");
    
    QString checkpointName = "test_checkpoint_validation";
    QVERIFY(rollback->createCheckpoint(checkpointName));
    
    bool isValid = rollback->validateCheckpoint(checkpointName);
    QVERIFY(isValid);
    
    bool invalidResult = rollback->validateCheckpoint("non_existent_checkpoint");
    QVERIFY(!invalidResult);
}

void CompatibilityModuleTest::testRollbackOperation()
{
    QVERIFY(m_module->initialize());
    
    RollbackManager* rollback = m_module->getRollbackManager();
    rollback->setCheckpointDirectory(m_testDataDir + "/checkpoints");
    
    QString checkpointName = "test_rollback_checkpoint";
    QVERIFY(rollback->createCheckpoint(checkpointName));
    
    QSignalSpy rollbackSpy(rollback, &RollbackManager::rollbackCompleted);
    
    bool result = rollback->rollbackToCheckpoint(checkpointName);
    QVERIFY(result);
    QCOMPARE(rollbackSpy.count(), 1);
    
    QList<QVariant> arguments = rollbackSpy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), checkpointName);
    QCOMPARE(arguments.at(1).toBool(), true);
}

void CompatibilityModuleTest::testCheckpointCleanup()
{
    QVERIFY(m_module->initialize());
    
    RollbackManager* rollback = m_module->getRollbackManager();
    rollback->setCheckpointDirectory(m_testDataDir + "/checkpoints");
    
    // 创建多个检查点
    QStringList checkpointNames;
    for (int i = 0; i < 5; ++i) {
        QString name = QString("cleanup_test_%1").arg(i);
        QVERIFY(rollback->createCheckpoint(name));
        checkpointNames.append(name);
    }
    
    QStringList beforeCleanup = rollback->availableCheckpoints();
    QVERIFY(beforeCleanup.size() >= 5);
    
    // 清理过期检查点（保留0天，应该删除所有）
    int cleaned = rollback->cleanupExpiredCheckpoints(0);
    QVERIFY(cleaned > 0);
    
    QStringList afterCleanup = rollback->availableCheckpoints();
    QVERIFY(afterCleanup.size() < beforeCleanup.size());
}

void CompatibilityModuleTest::testFunctionValidation()
{
    QVERIFY(m_module->initialize());
    
    CompatibilityValidator* validator = m_module->getValidator();
    QVERIFY(validator != nullptr);
    
    QSignalSpy validationSpy(validator, &CompatibilityValidator::validationCompleted);
    
    auto result = validator->validateFunctionality("audio");
    QVERIFY(result == ICompatibilityValidator::Passed || 
            result == ICompatibilityValidator::Warning);
    
    QCOMPARE(validationSpy.count(), 1);
}

void CompatibilityModuleTest::testPerformanceValidation()
{
    QVERIFY(m_module->initialize());
    
    CompatibilityValidator* validator = m_module->getValidator();
    QVERIFY(validator != nullptr);
    
    bool result = validator->validatePerformance("audio");
    QVERIFY(result); // 模拟实现应该通过
}

void CompatibilityModuleTest::testCompatibilityTests()
{
    QVERIFY(m_module->initialize());
    
    CompatibilityValidator* validator = m_module->getValidator();
    QVERIFY(validator != nullptr);
    
    QSignalSpy progressSpy(validator, &CompatibilityValidator::progressUpdated);
    
    QStringList results = validator->runCompatibilityTests();
    QVERIFY(!results.isEmpty());
    
    // 检查是否有进度更新
    QVERIFY(progressSpy.count() > 0);
    
    // 检查详细报告
    auto detailedReport = validator->getDetailedReport();
    QVERIFY(!detailedReport.isEmpty());
}

void CompatibilityModuleTest::testEndToEndWorkflow()
{
    QVERIFY(m_module->initialize());
    
    // 1. 创建检查点
    RollbackManager* rollback = m_module->getRollbackManager();
    rollback->setCheckpointDirectory(m_testDataDir + "/e2e_checkpoints");
    
    QString checkpointName = "e2e_test_checkpoint";
    QVERIFY(rollback->createCheckpoint(checkpointName));
    
    // 2. 运行兼容性测试
    CompatibilityValidator* validator = m_module->getValidator();
    QStringList testResults = validator->runCompatibilityTests();
    QVERIFY(!testResults.isEmpty());
    
    // 3. 创建适配器
    auto mediaManager = LegacyCompatibilityAdapter::createLegacyMediaManager();
    QVERIFY(mediaManager != nullptr);
    
    // 4. 验证功能
    bool validationResult = LegacyCompatibilityAdapter::validateFunctionality("audio");
    QVERIFY(validationResult);
    
    // 5. 如果需要，执行回滚
    if (!validationResult) {
        QVERIFY(rollback->rollbackToCheckpoint(checkpointName));
    }
    
    delete mediaManager;
}

void CompatibilityModuleTest::testErrorHandling()
{
    QVERIFY(m_module->initialize());
    
    QSignalSpy errorSpy(m_module, &CompatibilityModule::moduleError);
    
    // 测试无效的模块验证
    bool result = LegacyCompatibilityAdapter::validateFunctionality("");
    QVERIFY(!result);
    
    // 测试无效的检查点操作
    RollbackManager* rollback = m_module->getRollbackManager();
    bool rollbackResult = rollback->rollbackToCheckpoint("non_existent");
    QVERIFY(!rollbackResult);
    
    // 测试无效的配置
    CompatibilityValidator* validator = m_module->getValidator();
    QVariantMap invalidConfig;
    invalidConfig["invalid_key"] = "invalid_value";
    validator->setValidationConfig(invalidConfig); // 应该不会崩溃
}

void CompatibilityModuleTest::testConfigurationPersistence()
{
    QVERIFY(m_module->initialize());
    
    CompatibilityConfig* config = m_module->getConfig();
    QVERIFY(config != nullptr);
    
    // 设置测试配置文件路径
    QString configPath = m_testDataDir + "/test_config.json";
    config->setConfigFilePath(configPath);
    
    // 修改配置
    config->setValidationEnabled(false);
    config->setPerformanceCheckEnabled(true);
    config->setCheckpointRetentionDays(15);
    
    // 保存配置
    QVERIFY(config->saveConfiguration());
    
    // 创建新的配置实例并加载
    CompatibilityConfig newConfig;
    newConfig.setConfigFilePath(configPath);
    QVERIFY(newConfig.loadConfiguration());
    
    // 验证配置是否正确加载
    QCOMPARE(newConfig.isValidationEnabled(), false);
    QCOMPARE(newConfig.isPerformanceCheckEnabled(), true);
    QCOMPARE(newConfig.getCheckpointRetentionDays(), 15);
}

QTEST_MAIN(CompatibilityModuleTest)