#ifndef TEST_CAMERA_MODULE_H
#define TEST_CAMERA_MODULE_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include "../include/CameraModule.h"
#include "../include/CameraManager.h"
#include "../include/CameraFactory.h"
#include "../config/CameraConfig.h"
#include "../utils/CameraUtils.h"

/**
 * @brief 摄像头模块单元测试
 */
class TestCameraModule : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化和清理
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // CameraModule 测试
    void testCameraModuleInitialization();
    void testCameraModuleDeviceScanning();
    void testCameraModuleStartStop();
    void testCameraModuleConfiguration();
    void testCameraModuleErrorHandling();

    // CameraManager 测试
    void testCameraManagerInitialization();
    void testCameraManagerDeviceManagement();
    void testCameraManagerPreviewControl();
    void testCameraManagerQualityPresets();
    void testCameraManagerConfiguration();

    // CameraFactory 测试
    void testCameraFactorySingleton();
    void testCameraFactoryCreation();
    void testCameraFactoryDestruction();

    // CameraConfig 测试
    void testCameraConfigDefaults();
    void testCameraConfigPersistence();
    void testCameraConfigValidation();
    void testCameraConfigSignals();

    // CameraUtils 测试
    void testCameraUtilsResolutionMapping();
    void testCameraUtilsValidation();
    void testCameraUtilsFormatting();
    void testCameraUtilsCalculations();

    // 集成测试
    void testFullWorkflow();
    void testErrorRecovery();
    void testPerformanceMetrics();

private:
    void waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
    void verifyDeviceState(ICameraDevice* device, ICameraDevice::Status expectedStatus);
    void verifyManagerState(ICameraManager* manager, ICameraManager::ManagerStatus expectedStatus);

    CameraModule* m_cameraModule;
    CameraManager* m_cameraManager;
    CameraFactory* m_cameraFactory;
    CameraConfig* m_cameraConfig;
};

/**
 * @brief 摄像头性能测试
 */
class TestCameraPerformance : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 性能测试
    void testInitializationTime();
    void testStartupTime();
    void testFrameRateStability();
    void testMemoryUsage();
    void testCPUUsage();
    void testMultipleDevices();

private:
    void measureTime(std::function<void()> func, const QString& testName);
    void measureMemory(std::function<void()> func, const QString& testName);

    CameraManager* m_cameraManager;
};

/**
 * @brief 摄像头压力测试
 */
class TestCameraStress : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 压力测试
    void testRepeatedStartStop();
    void testDeviceSwitching();
    void testConfigurationChanges();
    void testLongRunning();
    void testConcurrentAccess();

private:
    CameraManager* m_cameraManager;
    QList<CameraManager*> m_managers;
};

#endif // TEST_CAMERA_MODULE_H