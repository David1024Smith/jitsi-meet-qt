#ifndef COREMODULETEST_H
#define COREMODULETEST_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include "ModuleManager.h"
#include "GlobalModuleConfig.h"
#include "ModuleHealthMonitor.h"
#include "ModuleVersionManager.h"
#include "management/RuntimeController.h"

/**
 * @brief Core模块管理系统测试类
 */
class CoreModuleTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // ModuleManager测试
    void testModuleManagerSingleton();
    void testModuleManagerInitialization();
    void testModuleLoading();
    void testModuleUnloading();
    void testModuleReloading();
    void testModuleStatus();
    void testModuleDependencies();
    void testBatchOperations();

    // GlobalModuleConfig测试
    void testGlobalConfigSingleton();
    void testModuleRegistration();
    void testModuleEnableDisable();
    void testConfigurationPersistence();
    void testConfigurationValidation();
    void testDependencyManagement();

    // ModuleHealthMonitor测试
    void testHealthMonitorBasicCheck();
    void testHealthMonitorPerformanceCheck();
    void testHealthMonitorResourceCheck();
    void testHealthMonitorConnectivityCheck();
    void testHealthMonitorFunctionalCheck();
    void testHealthMonitorThresholds();
    void testHealthMonitorAutoRecovery();
    void testHealthMonitorHistory();

    // ModuleVersionManager测试
    void testVersionManagerVersionInfo();
    void testVersionManagerCompatibility();
    void testVersionManagerUpgrade();
    void testVersionManagerRollback();
    void testVersionManagerAutoUpgrade();
    void testVersionManagerDependencyVersions();

    // RuntimeController测试
    void testRuntimeControllerBasicOperations();
    void testRuntimeControllerAsyncOperations();
    void testRuntimeControllerBatchOperations();
    void testRuntimeControllerSafeMode();
    void testRuntimeControllerTimeout();
    void testRuntimeControllerQueue();

    // 集成测试
    void testSystemIntegration();
    void testErrorHandling();
    void testPerformanceUnderLoad();

private:
    void createTestModule(const QString& moduleName, const QString& version = "1.0.0");
    void verifyModuleState(const QString& moduleName, IModuleManager::ModuleStatus expectedStatus);
    void waitForSignal(QObject* sender, const char* signal, int timeout = 5000);

    ModuleManager* m_moduleManager;
    GlobalModuleConfig* m_globalConfig;
    ModuleHealthMonitor* m_healthMonitor;
    ModuleVersionManager* m_versionManager;
    RuntimeController* m_runtimeController;
};

#endif // COREMODULETEST_H