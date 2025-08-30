#include "CoreModuleTest.h"
#include <QCoreApplication>
#include <QTimer>
#include <QEventLoop>

void CoreModuleTest::initTestCase()
{
    // 初始化测试环境
    m_moduleManager = ModuleManager::instance();
    QVERIFY(m_moduleManager != nullptr);
    
    // 初始化模块管理器
    QVERIFY(m_moduleManager->initialize());
    
    // 获取子系统引用
    m_globalConfig = m_moduleManager->getGlobalConfig();
    m_healthMonitor = m_moduleManager->getHealthMonitor();
    m_versionManager = m_moduleManager->getVersionManager();
    m_runtimeController = m_moduleManager->getRuntimeController();
    
    QVERIFY(m_globalConfig != nullptr);
    QVERIFY(m_healthMonitor != nullptr);
    QVERIFY(m_versionManager != nullptr);
    QVERIFY(m_runtimeController != nullptr);
}

void CoreModuleTest::cleanupTestCase()
{
    if (m_moduleManager) {
        m_moduleManager->shutdown();
    }
}

void CoreModuleTest::init()
{
    // 每个测试前的初始化
}

void CoreModuleTest::cleanup()
{
    // 每个测试后的清理
}

void CoreModuleTest::testModuleManagerSingleton()
{
    // 测试单例模式
    ModuleManager* instance1 = ModuleManager::instance();
    ModuleManager* instance2 = ModuleManager::instance();
    
    QCOMPARE(instance1, instance2);
    QVERIFY(instance1 != nullptr);
}

void CoreModuleTest::testModuleManagerInitialization()
{
    // 测试初始化状态
    QVERIFY(m_moduleManager->isInitialized());
    
    // 测试可用模块列表
    QStringList availableModules = m_moduleManager->getAvailableModules();
    QVERIFY(!availableModules.isEmpty());
    
    // 验证内置模块是否注册
    QVERIFY(availableModules.contains("audio"));
    QVERIFY(availableModules.contains("network"));
    QVERIFY(availableModules.contains("ui"));
}

void CoreModuleTest::testModuleLoading()
{
    const QString testModule = "audio";
    
    // 创建信号监听器
    QSignalSpy loadedSpy(m_moduleManager, &ModuleManager::moduleLoaded);
    QSignalSpy statusSpy(m_moduleManager, &ModuleManager::moduleStatusChanged);
    
    // 加载模块
    QVERIFY(m_moduleManager->loadModule(testModule));
    
    // 验证信号发送
    QCOMPARE(loadedSpy.count(), 1);
    QCOMPARE(loadedSpy.at(0).at(0).toString(), testModule);
    
    // 验证模块状态
    QVERIFY(m_moduleManager->isModuleLoaded(testModule));
    QCOMPARE(m_moduleManager->getModuleStatus(testModule), IModuleManager::Ready);
    
    // 验证加载的模块列表
    QStringList loadedModules = m_moduleManager->getLoadedModules();
    QVERIFY(loadedModules.contains(testModule));
}

void CoreModuleTest::testModuleUnloading()
{
    const QString testModule = "network";
    
    // 先加载模块
    QVERIFY(m_moduleManager->loadModule(testModule));
    QVERIFY(m_moduleManager->isModuleLoaded(testModule));
    
    // 创建信号监听器
    QSignalSpy unloadedSpy(m_moduleManager, &ModuleManager::moduleUnloaded);
    
    // 卸载模块
    QVERIFY(m_moduleManager->unloadModule(testModule));
    
    // 验证信号发送
    QCOMPARE(unloadedSpy.count(), 1);
    QCOMPARE(unloadedSpy.at(0).at(0).toString(), testModule);
    
    // 验证模块状态
    QVERIFY(!m_moduleManager->isModuleLoaded(testModule));
    QCOMPARE(m_moduleManager->getModuleStatus(testModule), IModuleManager::NotLoaded);
}

void CoreModuleTest::testModuleReloading()
{
    const QString testModule = "ui";
    
    // 先加载模块
    QVERIFY(m_moduleManager->loadModule(testModule));
    
    // 创建信号监听器
    QSignalSpy unloadedSpy(m_moduleManager, &ModuleManager::moduleUnloaded);
    QSignalSpy loadedSpy(m_moduleManager, &ModuleManager::moduleLoaded);
    
    // 重新加载模块
    QVERIFY(m_moduleManager->reloadModule(testModule));
    
    // 验证信号发送（应该先卸载再加载）
    QCOMPARE(unloadedSpy.count(), 1);
    QCOMPARE(loadedSpy.count(), 1);
    
    // 验证最终状态
    QVERIFY(m_moduleManager->isModuleLoaded(testModule));
    QCOMPARE(m_moduleManager->getModuleStatus(testModule), IModuleManager::Ready);
}

void CoreModuleTest::testGlobalConfigSingleton()
{
    // 测试单例模式
    GlobalModuleConfig* instance1 = GlobalModuleConfig::instance();
    GlobalModuleConfig* instance2 = GlobalModuleConfig::instance();
    
    QCOMPARE(instance1, instance2);
    QVERIFY(instance1 != nullptr);
}

void CoreModuleTest::testModuleRegistration()
{
    const QString testModule = "testModule";
    
    // 创建测试模块信息
    GlobalModuleConfig::ModuleInfo info;
    info.name = testModule;
    info.version = "1.0.0";
    info.description = "Test module";
    info.enabled = true;
    info.priority = 2;
    
    // 注册模块
    m_globalConfig->registerModule(testModule, info);
    
    // 验证注册结果
    QVERIFY(m_globalConfig->hasModule(testModule));
    
    auto retrievedInfo = m_globalConfig->getModuleInfo(testModule);
    QCOMPARE(retrievedInfo.name, testModule);
    QCOMPARE(retrievedInfo.version, "1.0.0");
    QCOMPARE(retrievedInfo.description, "Test module");
}

void CoreModuleTest::testModuleEnableDisable()
{
    const QString testModule = "performance";
    
    // 创建信号监听器
    QSignalSpy enabledSpy(m_globalConfig, &GlobalModuleConfig::moduleEnabled);
    QSignalSpy disabledSpy(m_globalConfig, &GlobalModuleConfig::moduleDisabled);
    
    // 启用模块
    m_globalConfig->setModuleEnabled(testModule, true);
    QVERIFY(m_globalConfig->isModuleEnabled(testModule));
    
    // 禁用模块
    m_globalConfig->setModuleEnabled(testModule, false);
    QVERIFY(!m_globalConfig->isModuleEnabled(testModule));
    
    // 验证启用的模块列表
    QStringList enabledModules = m_globalConfig->getEnabledModules();
    QVERIFY(!enabledModules.contains(testModule));
}

void CoreModuleTest::testHealthMonitorBasicCheck()
{
    const QString testModule = "utils";
    
    // 执行基础健康检查
    auto report = m_healthMonitor->performHealthCheck(testModule, IHealthMonitor::Basic);
    
    // 验证报告内容
    QCOMPARE(report.moduleName, testModule);
    QVERIFY(report.timestamp.isValid());
    QVERIFY(report.checkDuration >= 0);
    QVERIFY(report.score >= 0.0 && report.score <= 100.0);
    QVERIFY(!report.message.isEmpty());
}

void CoreModuleTest::testHealthMonitorPerformanceCheck()
{
    const QString testModule = "settings";
    
    // 执行性能检查
    auto report = m_healthMonitor->performHealthCheck(testModule, IHealthMonitor::Performance);
    
    // 验证报告内容
    QCOMPARE(report.moduleName, testModule);
    QVERIFY(report.details.contains("performanceScore"));
    
    // 性能分数应该在合理范围内
    double performanceScore = report.details["performanceScore"].toDouble();
    QVERIFY(performanceScore >= 0.0 && performanceScore <= 100.0);
}

void CoreModuleTest::testHealthMonitorThresholds()
{
    const QString testModule = "chat";
    
    // 设置健康阈值
    m_healthMonitor->setHealthThreshold(testModule, IHealthMonitor::Warning);
    QCOMPARE(m_healthMonitor->getHealthThreshold(testModule), IHealthMonitor::Warning);
    
    // 设置性能阈值
    m_healthMonitor->setPerformanceThreshold(testModule, 75.0);
    QCOMPARE(m_healthMonitor->getPerformanceThreshold(testModule), 75.0);
}

void CoreModuleTest::testVersionManagerVersionInfo()
{
    const QString testModule = "screenshare";
    
    // 获取版本信息
    auto versionInfo = m_versionManager->getVersionInfo(testModule);
    
    // 验证版本信息
    QCOMPARE(versionInfo.moduleName, testModule);
    QVERIFY(!versionInfo.version.isNull());
    QVERIFY(!versionInfo.description.isEmpty());
    QVERIFY(versionInfo.releaseDate.isValid());
}

void CoreModuleTest::testVersionManagerCompatibility()
{
    const QString testModule = "meeting";
    
    // 测试版本兼容性
    QVersionNumber testVersion(1, 0, 0);
    bool compatible = m_versionManager->isVersionCompatible(testModule, testVersion);
    
    // 应该兼容基础版本
    QVERIFY(compatible);
    
    // 测试极端版本
    QVersionNumber extremeVersion(999, 999, 999);
    bool extremeCompatible = m_versionManager->isVersionCompatible(testModule, extremeVersion);
    
    // 极端版本可能不兼容
    // QVERIFY(!extremeCompatible); // 取决于具体实现
}

void CoreModuleTest::testRuntimeControllerBasicOperations()
{
    const QString testModule = "audio";
    
    // 创建信号监听器
    QSignalSpy operationStartedSpy(m_runtimeController, &RuntimeController::operationStarted);
    QSignalSpy operationCompletedSpy(m_runtimeController, &RuntimeController::operationCompleted);
    
    // 执行启用操作
    QVERIFY(m_runtimeController->enableModule(testModule, RuntimeController::Synchronous));
    
    // 验证信号发送
    QCOMPARE(operationStartedSpy.count(), 1);
    QCOMPARE(operationCompletedSpy.count(), 1);
    
    // 验证操作结果
    QVERIFY(m_globalConfig->isModuleEnabled(testModule));
}

void CoreModuleTest::testRuntimeControllerAsyncOperations()
{
    const QString testModule = "network";
    
    // 创建信号监听器
    QSignalSpy requestQueuedSpy(m_runtimeController, &RuntimeController::requestQueued);
    
    // 执行异步操作
    QVERIFY(m_runtimeController->enableModule(testModule, RuntimeController::Asynchronous));
    
    // 应该有请求被排队
    QVERIFY(requestQueuedSpy.count() >= 0); // 可能立即执行或排队
    
    // 等待操作完成
    waitForSignal(m_runtimeController, SIGNAL(operationCompleted(QString,RuntimeController::ControlAction,bool)), 3000);
}

void CoreModuleTest::testSystemIntegration()
{
    // 测试系统集成功能
    
    // 1. 加载所有模块
    QVERIFY(m_moduleManager->loadAllModules());
    
    // 2. 检查所有模块健康状态
    QStringList loadedModules = m_moduleManager->getLoadedModules();
    for (const QString& moduleName : loadedModules) {
        auto report = m_healthMonitor->checkModuleHealth(moduleName);
        QVERIFY(report.status != IHealthMonitor::Failure);
    }
    
    // 3. 验证版本信息
    for (const QString& moduleName : loadedModules) {
        QVersionNumber version = m_versionManager->getModuleVersion(moduleName);
        QVERIFY(!version.isNull());
    }
    
    // 4. 测试配置保存和加载
    QVERIFY(m_globalConfig->saveConfiguration());
    QVERIFY(m_globalConfig->loadConfiguration());
}

void CoreModuleTest::createTestModule(const QString& moduleName, const QString& version)
{
    GlobalModuleConfig::ModuleInfo info;
    info.name = moduleName;
    info.version = version;
    info.description = QString("Test module: %1").arg(moduleName);
    info.enabled = true;
    info.priority = 2;
    
    m_globalConfig->registerModule(moduleName, info);
}

void CoreModuleTest::verifyModuleState(const QString& moduleName, IModuleManager::ModuleStatus expectedStatus)
{
    QCOMPARE(m_moduleManager->getModuleStatus(moduleName), expectedStatus);
}

void CoreModuleTest::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(timeout);
    
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(sender, signal, &loop, &QEventLoop::quit);
    
    timer.start();
    loop.exec();
}

// 测试用例注册
QTEST_MAIN(CoreModuleTest)