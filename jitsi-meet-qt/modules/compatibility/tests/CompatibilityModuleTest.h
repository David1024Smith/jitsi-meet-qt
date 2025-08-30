#ifndef COMPATIBILITYMODULETEST_H
#define COMPATIBILITYMODULETEST_H

#include <QObject>
#include <QTest>

class CompatibilityModule;
class LegacyCompatibilityAdapter;
class RollbackManager;
class CompatibilityValidator;

/**
 * @brief 兼容性模块测试类
 * 
 * 测试兼容性适配器系统的各个组件。
 */
class CompatibilityModuleTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 模块测试
    void testModuleInitialization();
    void testModuleInfo();
    void testComponentAccess();

    // 适配器测试
    void testAdapterCreation();
    void testAdapterValidation();
    void testAdapterConfiguration();

    // 回滚管理器测试
    void testCheckpointCreation();
    void testCheckpointValidation();
    void testRollbackOperation();
    void testCheckpointCleanup();

    // 验证器测试
    void testFunctionValidation();
    void testPerformanceValidation();
    void testCompatibilityTests();

    // 集成测试
    void testEndToEndWorkflow();
    void testErrorHandling();
    void testConfigurationPersistence();

private:
    CompatibilityModule* m_module;
    QString m_testDataDir;
};

#endif // COMPATIBILITYMODULETEST_H