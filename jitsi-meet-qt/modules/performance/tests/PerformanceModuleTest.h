#ifndef PERFORMANCEMODULETEST_H
#define PERFORMANCEMODULETEST_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include "PerformanceModule.h"
#include "PerformanceManager.h"
#include "MetricsCollector.h"
#include "PerformanceConfig.h"

/**
 * @brief 性能模块测试类
 * 
 * PerformanceModuleTest提供性能模块的完整测试套件：
 * - 模块初始化和生命周期测试
 * - 性能监控功能测试
 * - 优化器功能测试
 * - 配置管理测试
 * - 集成测试和压力测试
 */
class PerformanceModuleTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化和清理
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基础功能测试
    void testModuleInitialization();
    void testModuleStartStop();
    void testModulePauseResume();
    void testModuleShutdown();
    void testModuleStatus();
    void testModuleVersion();
    void testModuleSingleton();

    // 组件测试
    void testPerformanceManagerCreation();
    void testMetricsCollectorCreation();
    void testConfigurationCreation();
    void testComponentInitialization();
    void testComponentInteraction();

    // 监控功能测试
    void testCPUMonitoring();
    void testMemoryMonitoring();
    void testNetworkMonitoring();
    void testRealTimeMonitoring();
    void testHistoricalDataCollection();
    void testThresholdDetection();

    // 优化功能测试
    void testStartupOptimization();
    void testMemoryOptimization();
    void testRenderOptimization();
    void testAutoOptimization();
    void testOptimizationStrategies();

    // 配置测试
    void testConfigurationLoad();
    void testConfigurationSave();
    void testConfigurationValidation();
    void testConfigurationDefaults();
    void testConfigurationUpdate();

    // 数据管理测试
    void testDataCollection();
    void testDataStorage();
    void testDataExport();
    void testDataImport();
    void testDataCleanup();

    // 性能测试
    void testPerformanceOverhead();
    void testMemoryUsage();
    void testCPUUsage();
    void testResponseTime();
    void testThroughput();

    // 错误处理测试
    void testErrorHandling();
    void testExceptionHandling();
    void testRecoveryMechanisms();
    void testFailureScenarios();

    // 集成测试
    void testModuleIntegration();
    void testUIIntegration();
    void testSystemIntegration();
    void testCrossModuleInteraction();

    // 压力测试
    void testHighLoadScenario();
    void testLongRunningTest();
    void testMemoryLeakTest();
    void testConcurrencyTest();

    // 兼容性测试
    void testPlatformCompatibility();
    void testVersionCompatibility();
    void testConfigurationCompatibility();

private:
    // 测试辅助方法
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    bool waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
    void generateTestData();
    void verifyPerformanceMetrics(const PerformanceMetrics& metrics);
    void simulateSystemLoad();
    void createTestConfiguration();

    // 测试数据
    PerformanceModule* m_performanceModule;
    PerformanceManager* m_performanceManager;
    MetricsCollector* m_metricsCollector;
    PerformanceConfig* m_config;
    
    QList<PerformanceMetrics> m_testMetrics;
    QVariantMap m_testConfig;
    QString m_testDataPath;
    
    // 测试状态
    bool m_testEnvironmentReady;
    QTimer* m_testTimer;
};

#endif // PERFORMANCEMODULETEST_H