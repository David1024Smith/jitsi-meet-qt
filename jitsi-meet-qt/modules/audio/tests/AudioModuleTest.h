#ifndef AUDIOMODULETEST_H
#define AUDIOMODULETEST_H

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>

// 音频模块相关头文件
#include "../include/AudioModule.h"
#include "../include/AudioManager.h"
#include "../include/AudioFactory.h"
#include "../interfaces/IAudioDevice.h"
#include "../interfaces/IAudioManager.h"
#include "../config/AudioConfig.h"
#include "../utils/AudioUtils.h"

/**
 * @brief 音频模块综合测试类
 * 
 * AudioModuleTest提供音频模块的全面测试，包括：
 * - 设备枚举和选择测试
 * - 音频质量和延迟测试
 * - 配置管理测试
 * - 与现有MediaManager的兼容性测试
 * - 性能和压力测试
 */
class AudioModuleTest : public QObject
{
    Q_OBJECT

public:
    explicit AudioModuleTest(QObject *parent = nullptr);
    ~AudioModuleTest();

private slots:
    // 测试框架生命周期
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // === 基础模块测试 ===
    void testModuleInitialization();
    void testModuleShutdown();
    void testModuleStatus();
    void testModuleVersion();
    void testModuleAvailability();

    // === 设备枚举和选择测试 ===
    void testDeviceEnumeration();
    void testInputDeviceEnumeration();
    void testOutputDeviceEnumeration();
    void testDeviceSelection();
    void testInputDeviceSelection();
    void testOutputDeviceSelection();
    void testDeviceSelectionValidation();
    void testInvalidDeviceSelection();
    void testDeviceDisplayNames();
    void testDeviceRefresh();

    // === 音频质量测试 ===
    void testQualityPresets();
    void testLowQualityPreset();
    void testStandardQualityPreset();
    void testHighQualityPreset();
    void testCustomQualitySettings();
    void testSampleRateConfiguration();
    void testChannelConfiguration();
    void testBufferSizeConfiguration();
    void testBitrateConfiguration();

    // === 音频延迟测试 ===
    void testAudioLatency();
    void testInputLatency();
    void testOutputLatency();
    void testRoundTripLatency();
    void testLatencyMeasurement();
    void testLatencyOptimization();
    void testBufferSizeLatencyImpact();

    // === 音频控制测试 ===
    void testVolumeControl();
    void testMasterVolumeControl();
    void testMicrophoneGainControl();
    void testMuteControl();
    void testVolumeRangeValidation();
    void testVolumeSignals();

    // === 音频流测试 ===
    void testAudioStreamStart();
    void testAudioStreamStop();
    void testAudioStreamPause();
    void testAudioStreamResume();
    void testAudioStreamStatus();
    void testMultipleStreamOperations();

    // === 配置管理测试 ===
    void testConfigurationLoad();
    void testConfigurationSave();
    void testConfigurationValidation();
    void testConfigurationReset();
    void testConfigurationSignals();
    void testCustomConfigurationParameters();

    // === 错误处理测试 ===
    void testDeviceErrors();
    void testInitializationErrors();
    void testConfigurationErrors();
    void testStreamErrors();
    void testErrorRecovery();
    void testErrorSignals();

    // === 性能测试 ===
    void testMemoryUsage();
    void testCPUUsage();
    void testStartupPerformance();
    void testDeviceEnumerationPerformance();
    void testConfigurationPerformance();

    // === 压力测试 ===
    void testMultipleInitializations();
    void testRapidDeviceSwitching();
    void testContinuousVolumeChanges();
    void testLongRunningAudioStream();
    void testResourceLeakage();

    // === 兼容性测试 ===
    void testMediaManagerCompatibility();
    void testLegacyAPICompatibility();
    void testConfigurationMigration();
    void testBackwardCompatibility();

    // === 集成测试 ===
    void testAudioManagerIntegration();
    void testAudioConfigIntegration();
    void testAudioUtilsIntegration();
    void testUIComponentIntegration();

    // === 平台特定测试 ===
    void testPlatformSpecificFeatures();
    void testWindowsAudioAPI();
    void testLinuxAudioAPI();
    void testMacOSAudioAPI();

private:
    // 测试辅助方法
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    bool waitForSignal(QObject *sender, const char *signal, int timeout = 5000);
    void measureLatency(const QString &testName, std::function<void()> operation);
    void measureMemoryUsage(const QString &testName, std::function<void()> operation);
    QStringList getTestAudioDevices();
    void simulateDeviceError(const QString &deviceId);
    void validateAudioQuality(const AudioUtils::AudioFormat &format);
    void performStressTest(const QString &testName, std::function<void()> operation, int iterations = 100);

    // 测试数据和状态
    std::unique_ptr<AudioModule> m_audioModule;
    std::unique_ptr<AudioManager> m_audioManager;
    std::unique_ptr<AudioConfig> m_audioConfig;
    
    QStringList m_availableInputDevices;
    QStringList m_availableOutputDevices;
    QString m_testConfigPath;
    
    // 性能测试数据
    struct PerformanceMetrics {
        qint64 executionTime;
        size_t memoryUsage;
        double cpuUsage;
        QString testName;
    };
    
    QList<PerformanceMetrics> m_performanceResults;
    
    // 测试配置
    static const int DEFAULT_TIMEOUT = 5000;
    static const int STRESS_TEST_ITERATIONS = 100;
    static const double LATENCY_THRESHOLD_MS = 50.0;
    static const double MEMORY_LEAK_THRESHOLD_MB = 10.0;
};

#endif // AUDIOMODULETEST_H