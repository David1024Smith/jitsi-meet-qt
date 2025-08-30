#ifndef SCREENSHAREMODULETEST_H
#define SCREENSHAREMODULETEST_H

#include <QObject>
#include <QTest>

class ScreenShareModule;
class ScreenShareManager;
class CaptureEngine;

/**
 * @brief 屏幕共享模块测试类
 * 
 * 提供屏幕共享模块的完整测试覆盖
 */
class ScreenShareModuleTest : public QObject
{
    Q_OBJECT

public:
    explicit ScreenShareModuleTest(QObject *parent = nullptr);
    ~ScreenShareModuleTest();

private slots:
    // 测试套件初始化和清理
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 模块基础测试
    void testModuleInitialization();
    void testModuleConfiguration();
    void testManagerAccess();
    void testModuleInfo();
    void testSelfTest();

    // 屏幕共享管理器测试
    void testScreenShareManagerInitialization();
    void testScreenShareManagerConfiguration();
    void testAvailableSources();
    void testSourceSelection();
    void testStatistics();

    // 捕获引擎测试
    void testCaptureEngineInitialization();
    void testCaptureEngineConfiguration();

    // 屏幕捕获测试
    void testScreenCaptureBasic();
    void testScreenCaptureStart();

    // 捕获系统测试 (Task 41)
    void testCaptureSystem();
    void testScreenCapture();
    void testWindowCapture();
    void testRegionCapture();

    // 编码处理测试 (Task 41)
    void testEncodingProcessing();
    void testVideoEncoder();
    void testFrameProcessor();

    // 质量自适应和性能优化测试 (Task 41)
    void testQualityAdaptiveAndPerformanceOptimization();
    void testIntegrationFeatures();

    // 错误处理测试
    void testErrorHandling();

    // Task 43: 屏幕共享模块测试框架
    // 捕获质量和性能测试
    void testCaptureQualityMetrics();
    void testPerformanceBenchmarks();
    void testMemoryUsageOptimization();
    void testCPUUsageMonitoring();
    void testFrameRateStability();
    void testLatencyMeasurement();
    
    // 屏幕共享UI组件测试
    void testScreenShareWidget();
    void testScreenSelector();
    void testCapturePreview();
    void testUIComponentInteractions();
    void testUIResponsiveness();
    void testUIStateManagement();
    
    // 与现有ScreenShareManager的兼容性测试
    void testLegacyCompatibility();
    void testAPICompatibility();
    void testConfigurationMigration();
    void testFeatureParity();
    void testBackwardCompatibility();
    void testIntegrationWithExistingCode();

private:
    ScreenShareModule* m_module;
    ScreenShareManager* m_manager;
    CaptureEngine* m_captureEngine;
};

#endif // SCREENSHAREMODULETEST_H