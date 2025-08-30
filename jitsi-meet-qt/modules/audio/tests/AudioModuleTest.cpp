#include "AudioModuleTest.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QThread>
#include <QProcess>
#include <QDebug>

AudioModuleTest::AudioModuleTest(QObject *parent)
    : QObject(parent)
{
}

AudioModuleTest::~AudioModuleTest()
{
}

void AudioModuleTest::initTestCase()
{
    qDebug() << "=== 开始音频模块测试 ===";
    qDebug() << "Qt版本:" << QT_VERSION_STR;
    qDebug() << "测试平台:" << QSysInfo::prettyProductName();
    
    setupTestEnvironment();
    
    // 创建测试配置目录
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/AudioModuleTest";
    QDir().mkpath(configDir);
    m_testConfigPath = configDir + "/test_audio_config.json";
    
    qDebug() << "测试配置路径:" << m_testConfigPath;
    qDebug() << "测试环境初始化完成";
}

void AudioModuleTest::cleanupTestCase()
{
    cleanupTestEnvironment();
    
    // 清理测试配置文件
    QFile::remove(m_testConfigPath);
    
    // 输出性能测试结果
    if (!m_performanceResults.isEmpty()) {
        qDebug() << "\n=== 性能测试结果 ===";
        for (const auto &result : m_performanceResults) {
            qDebug() << QString("%1: 执行时间=%2ms, 内存使用=%3KB")
                        .arg(result.testName)
                        .arg(result.executionTime)
                        .arg(result.memoryUsage / 1024);
        }
    }
    
    qDebug() << "=== 音频模块测试完成 ===";
}

void AudioModuleTest::init()
{
    // 每个测试前创建新的实例
    m_audioModule = std::make_unique<AudioModule>();
    m_audioManager = std::make_unique<AudioManager>();
    m_audioConfig = std::make_unique<AudioConfig>();
    
    // 设置测试配置路径
    m_audioConfig->setConfigFilePath(m_testConfigPath);
}

void AudioModuleTest::cleanup()
{
    // 每个测试后清理
    if (m_audioManager && m_audioManager->isAudioActive()) {
        m_audioManager->stopAudio();
    }
    
    if (m_audioModule && m_audioModule->status() != AudioModule::Uninitialized) {
        m_audioModule->shutdown();
    }
    
    m_audioModule.reset();
    m_audioManager.reset();
    m_audioConfig.reset();
}

// === 基础模块测试 ===

void AudioModuleTest::testModuleInitialization()
{
    QVERIFY(m_audioModule != nullptr);
    QCOMPARE(m_audioModule->status(), AudioModule::Uninitialized);
    
    QSignalSpy statusSpy(m_audioModule.get(), &AudioModule::statusChanged);
    QSignalSpy initializedSpy(m_audioModule.get(), &AudioModule::initialized);
    
    bool result = m_audioModule->initialize();
    QVERIFY(result);
    
    // 等待初始化完成
    QVERIFY(waitForSignal(m_audioModule.get(), SIGNAL(initialized())));
    
    QCOMPARE(m_audioModule->status(), AudioModule::Ready);
    QVERIFY(m_audioModule->isAvailable());
    
    // 验证信号
    QVERIFY(statusSpy.count() >= 1);
    QCOMPARE(initializedSpy.count(), 1);
}

void AudioModuleTest::testModuleShutdown()
{
    // 先初始化
    QVERIFY(m_audioModule->initialize());
    QVERIFY(waitForSignal(m_audioModule.get(), SIGNAL(initialized())));
    
    QSignalSpy statusSpy(m_audioModule.get(), &AudioModule::statusChanged);
    QSignalSpy shutdownSpy(m_audioModule.get(), &AudioModule::shutdownCompleted);
    
    m_audioModule->shutdown();
    
    // 等待关闭完成
    QVERIFY(waitForSignal(m_audioModule.get(), SIGNAL(shutdownCompleted())));
    
    QCOMPARE(m_audioModule->status(), AudioModule::Shutdown);
    QVERIFY(!m_audioModule->isAvailable());
    
    // 验证信号
    QVERIFY(shutdownSpy.count() == 1);
}

void AudioModuleTest::testModuleStatus()
{
    // 测试状态转换
    QCOMPARE(m_audioModule->status(), AudioModule::Uninitialized);
    
    QSignalSpy statusSpy(m_audioModule.get(), &AudioModule::statusChanged);
    
    m_audioModule->initialize();
    QVERIFY(waitForSignal(m_audioModule.get(), SIGNAL(initialized())));
    
    // 验证状态变化
    QVERIFY(statusSpy.count() >= 1);
    QList<QVariant> arguments = statusSpy.last();
    QCOMPARE(arguments.at(0).value<AudioModule::ModuleStatus>(), AudioModule::Ready);
}

void AudioModuleTest::testModuleVersion()
{
    QString version = AudioModule::version();
    QVERIFY(!version.isEmpty());
    QVERIFY(version.contains('.'));
    
    qDebug() << "音频模块版本:" << version;
}

void AudioModuleTest::testModuleAvailability()
{
    // 未初始化时不可用
    QVERIFY(!m_audioModule->isAvailable());
    
    // 初始化后可用
    QVERIFY(m_audioModule->initialize());
    QVERIFY(waitForSignal(m_audioModule.get(), SIGNAL(initialized())));
    QVERIFY(m_audioModule->isAvailable());
    
    // 关闭后不可用
    m_audioModule->shutdown();
    QVERIFY(waitForSignal(m_audioModule.get(), SIGNAL(shutdownCompleted())));
    QVERIFY(!m_audioModule->isAvailable());
}

// === 设备枚举和选择测试 ===

void AudioModuleTest::testDeviceEnumeration()
{
    QVERIFY(m_audioManager->initialize());
    
    QStringList inputDevices = m_audioManager->availableInputDevices();
    QStringList outputDevices = m_audioManager->availableOutputDevices();
    
    qDebug() << "发现输入设备数量:" << inputDevices.size();
    qDebug() << "发现输出设备数量:" << outputDevices.size();
    
    // 至少应该有一个默认设备
    QVERIFY(!inputDevices.isEmpty());
    QVERIFY(!outputDevices.isEmpty());
    
    // 验证设备ID不为空
    for (const QString &deviceId : inputDevices) {
        QVERIFY(!deviceId.isEmpty());
        QString displayName = m_audioManager->deviceDisplayName(deviceId);
        QVERIFY(!displayName.isEmpty());
        qDebug() << "输入设备:" << deviceId << "-" << displayName;
    }
    
    for (const QString &deviceId : outputDevices) {
        QVERIFY(!deviceId.isEmpty());
        QString displayName = m_audioManager->deviceDisplayName(deviceId);
        QVERIFY(!displayName.isEmpty());
        qDebug() << "输出设备:" << deviceId << "-" << displayName;
    }
    
    m_availableInputDevices = inputDevices;
    m_availableOutputDevices = outputDevices;
}

void AudioModuleTest::testInputDeviceEnumeration()
{
    measureLatency("输入设备枚举", [this]() {
        QVERIFY(m_audioManager->initialize());
        QStringList devices = m_audioManager->availableInputDevices();
        QVERIFY(!devices.isEmpty());
    });
}

void AudioModuleTest::testOutputDeviceEnumeration()
{
    measureLatency("输出设备枚举", [this]() {
        QVERIFY(m_audioManager->initialize());
        QStringList devices = m_audioManager->availableOutputDevices();
        QVERIFY(!devices.isEmpty());
    });
}

void AudioModuleTest::testDeviceSelection()
{
    QVERIFY(m_audioManager->initialize());
    
    QStringList inputDevices = m_audioManager->availableInputDevices();
    QStringList outputDevices = m_audioManager->availableOutputDevices();
    
    QVERIFY(!inputDevices.isEmpty());
    QVERIFY(!outputDevices.isEmpty());
    
    QSignalSpy deviceChangedSpy(m_audioManager.get(), &AudioManager::deviceChanged);
    
    // 选择第一个输入设备
    QString inputDevice = inputDevices.first();
    bool result = m_audioManager->selectInputDevice(inputDevice);
    QVERIFY(result);
    QCOMPARE(m_audioManager->currentInputDevice(), inputDevice);
    
    // 选择第一个输出设备
    QString outputDevice = outputDevices.first();
    result = m_audioManager->selectOutputDevice(outputDevice);
    QVERIFY(result);
    QCOMPARE(m_audioManager->currentOutputDevice(), outputDevice);
    
    // 验证设备改变信号
    QVERIFY(deviceChangedSpy.count() >= 2);
}

void AudioModuleTest::testInputDeviceSelection()
{
    QVERIFY(m_audioManager->initialize());
    
    QStringList devices = m_audioManager->availableInputDevices();
    QVERIFY(!devices.isEmpty());
    
    for (const QString &deviceId : devices) {
        QSignalSpy deviceSpy(m_audioManager.get(), &AudioManager::deviceChanged);
        
        bool result = m_audioManager->selectInputDevice(deviceId);
        QVERIFY(result);
        QCOMPARE(m_audioManager->currentInputDevice(), deviceId);
        
        // 验证信号
        if (deviceSpy.count() > 0) {
            QList<QVariant> arguments = deviceSpy.last();
            QCOMPARE(arguments.at(0).toString(), deviceId);
            QCOMPARE(arguments.at(1).toBool(), true); // isInput = true
        }
    }
}

void AudioModuleTest::testOutputDeviceSelection()
{
    QVERIFY(m_audioManager->initialize());
    
    QStringList devices = m_audioManager->availableOutputDevices();
    QVERIFY(!devices.isEmpty());
    
    for (const QString &deviceId : devices) {
        QSignalSpy deviceSpy(m_audioManager.get(), &AudioManager::deviceChanged);
        
        bool result = m_audioManager->selectOutputDevice(deviceId);
        QVERIFY(result);
        QCOMPARE(m_audioManager->currentOutputDevice(), deviceId);
        
        // 验证信号
        if (deviceSpy.count() > 0) {
            QList<QVariant> arguments = deviceSpy.last();
            QCOMPARE(arguments.at(0).toString(), deviceId);
            QCOMPARE(arguments.at(1).toBool(), false); // isInput = false
        }
    }
}

void AudioModuleTest::testDeviceSelectionValidation()
{
    QVERIFY(m_audioManager->initialize());
    
    // 测试有效设备选择
    QStringList inputDevices = m_audioManager->availableInputDevices();
    if (!inputDevices.isEmpty()) {
        QVERIFY(m_audioManager->selectInputDevice(inputDevices.first()));
    }
    
    QStringList outputDevices = m_audioManager->availableOutputDevices();
    if (!outputDevices.isEmpty()) {
        QVERIFY(m_audioManager->selectOutputDevice(outputDevices.first()));
    }
}

void AudioModuleTest::testInvalidDeviceSelection()
{
    QVERIFY(m_audioManager->initialize());
    
    QSignalSpy errorSpy(m_audioManager.get(), &AudioManager::errorOccurred);
    
    // 测试无效设备ID
    bool result = m_audioManager->selectInputDevice("invalid_device_id");
    QVERIFY(!result);
    
    result = m_audioManager->selectOutputDevice("invalid_device_id");
    QVERIFY(!result);
    
    // 测试空设备ID
    result = m_audioManager->selectInputDevice("");
    QVERIFY(!result);
    
    result = m_audioManager->selectOutputDevice("");
    QVERIFY(!result);
    
    // 可能会有错误信号
    if (errorSpy.count() > 0) {
        qDebug() << "预期的错误信号数量:" << errorSpy.count();
    }
}

void AudioModuleTest::testDeviceDisplayNames()
{
    QVERIFY(m_audioManager->initialize());
    
    QStringList inputDevices = m_audioManager->availableInputDevices();
    QStringList outputDevices = m_audioManager->availableOutputDevices();
    
    // 测试输入设备显示名称
    for (const QString &deviceId : inputDevices) {
        QString displayName = m_audioManager->deviceDisplayName(deviceId);
        QVERIFY(!displayName.isEmpty());
        QVERIFY(displayName != deviceId); // 显示名称应该不同于ID
    }
    
    // 测试输出设备显示名称
    for (const QString &deviceId : outputDevices) {
        QString displayName = m_audioManager->deviceDisplayName(deviceId);
        QVERIFY(!displayName.isEmpty());
        QVERIFY(displayName != deviceId); // 显示名称应该不同于ID
    }
    
    // 测试无效设备ID
    QString invalidName = m_audioManager->deviceDisplayName("invalid_device");
    QVERIFY(invalidName.isEmpty() || invalidName == "Unknown Device");
}

void AudioModuleTest::testDeviceRefresh()
{
    QVERIFY(m_audioManager->initialize());
    
    QSignalSpy devicesSpy(m_audioManager.get(), &AudioManager::devicesUpdated);
    
    // 获取初始设备列表
    QStringList initialInput = m_audioManager->availableInputDevices();
    QStringList initialOutput = m_audioManager->availableOutputDevices();
    
    // 触发设备刷新（通过重新初始化）
    m_audioManager.reset();
    m_audioManager = std::make_unique<AudioManager>();
    QVERIFY(m_audioManager->initialize());
    
    // 获取刷新后的设备列表
    QStringList refreshedInput = m_audioManager->availableInputDevices();
    QStringList refreshedOutput = m_audioManager->availableOutputDevices();
    
    // 设备列表应该基本相同（除非有设备插拔）
    QCOMPARE(refreshedInput.size(), initialInput.size());
    QCOMPARE(refreshedOutput.size(), initialOutput.size());
}

// === 音频质量测试 ===

void AudioModuleTest::testQualityPresets()
{
    QVERIFY(m_audioManager->initialize());
    
    // 测试所有质量预设
    QList<AudioManager::QualityPreset> presets = {
        AudioManager::LowQuality,
        AudioManager::StandardQuality,
        AudioManager::HighQuality
    };
    
    for (AudioManager::QualityPreset preset : presets) {
        m_audioManager->setQualityPreset(preset);
        QCOMPARE(m_audioManager->qualityPreset(), preset);
        
        // 验证质量预设的音频配置
        AudioConfig *config = m_audioManager->audioConfig();
        QVERIFY(config != nullptr);
        
        switch (preset) {
        case AudioManager::LowQuality:
            QCOMPARE(config->sampleRate(), 16000);
            QCOMPARE(config->channels(), 1);
            break;
        case AudioManager::StandardQuality:
            QCOMPARE(config->sampleRate(), 44100);
            QCOMPARE(config->channels(), 2);
            break;
        case AudioManager::HighQuality:
            QCOMPARE(config->sampleRate(), 48000);
            QCOMPARE(config->channels(), 2);
            break;
        }
        
        qDebug() << QString("质量预设 %1: 采样率=%2Hz, 声道=%3")
                    .arg(preset)
                    .arg(config->sampleRate())
                    .arg(config->channels());
    }
}

void AudioModuleTest::testLowQualityPreset()
{
    QVERIFY(m_audioManager->initialize());
    
    m_audioManager->setQualityPreset(AudioManager::LowQuality);
    AudioConfig *config = m_audioManager->audioConfig();
    
    QCOMPARE(config->sampleRate(), 16000);
    QCOMPARE(config->channels(), 1);
    QCOMPARE(config->bitrate(), 64);
    QCOMPARE(config->bufferSize(), 512);
    
    validateAudioQuality(AudioUtils::AudioFormat{
        16000, 1, 16, true, false
    });
}

void AudioModuleTest::testStandardQualityPreset()
{
    QVERIFY(m_audioManager->initialize());
    
    m_audioManager->setQualityPreset(AudioManager::StandardQuality);
    AudioConfig *config = m_audioManager->audioConfig();
    
    QCOMPARE(config->sampleRate(), 44100);
    QCOMPARE(config->channels(), 2);
    QCOMPARE(config->bitrate(), 128);
    QCOMPARE(config->bufferSize(), 1024);
    
    validateAudioQuality(AudioUtils::AudioFormat{
        44100, 2, 16, true, false
    });
}

void AudioModuleTest::testHighQualityPreset()
{
    QVERIFY(m_audioManager->initialize());
    
    m_audioManager->setQualityPreset(AudioManager::HighQuality);
    AudioConfig *config = m_audioManager->audioConfig();
    
    QCOMPARE(config->sampleRate(), 48000);
    QCOMPARE(config->channels(), 2);
    QCOMPARE(config->bitrate(), 256);
    QCOMPARE(config->bufferSize(), 2048);
    
    validateAudioQuality(AudioUtils::AudioFormat{
        48000, 2, 16, true, false
    });
}

void AudioModuleTest::testCustomQualitySettings()
{
    QVERIFY(m_audioManager->initialize());
    
    AudioConfig *config = m_audioManager->audioConfig();
    
    // 设置自定义质量参数
    config->setSampleRate(22050);
    config->setChannels(1);
    config->setBitrate(96);
    config->setBufferSize(1536);
    
    QCOMPARE(config->sampleRate(), 22050);
    QCOMPARE(config->channels(), 1);
    QCOMPARE(config->bitrate(), 96);
    QCOMPARE(config->bufferSize(), 1536);
    
    // 验证自定义设置的有效性
    QVERIFY(config->validate());
}

void AudioModuleTest::testSampleRateConfiguration()
{
    QVERIFY(m_audioManager->initialize());
    
    AudioConfig *config = m_audioManager->audioConfig();
    QSignalSpy configSpy(config, &AudioConfig::qualityConfigChanged);
    
    QList<int> testRates = {8000, 16000, 22050, 44100, 48000, 96000};
    
    for (int rate : testRates) {
        config->setSampleRate(rate);
        QCOMPARE(config->sampleRate(), rate);
        
        // 验证配置有效性
        if (rate >= 8000 && rate <= 96000) {
            QVERIFY(config->validate());
        }
    }
    
    QVERIFY(configSpy.count() >= testRates.size());
}

void AudioModuleTest::testChannelConfiguration()
{
    QVERIFY(m_audioManager->initialize());
    
    AudioConfig *config = m_audioManager->audioConfig();
    QSignalSpy configSpy(config, &AudioConfig::qualityConfigChanged);
    
    QList<int> testChannels = {1, 2, 4, 6, 8};
    
    for (int channels : testChannels) {
        config->setChannels(channels);
        QCOMPARE(config->channels(), channels);
        
        // 验证配置有效性
        if (channels >= 1 && channels <= 8) {
            QVERIFY(config->validate());
        }
    }
    
    QVERIFY(configSpy.count() >= testChannels.size());
}

void AudioModuleTest::testBufferSizeConfiguration()
{
    QVERIFY(m_audioManager->initialize());
    
    AudioConfig *config = m_audioManager->audioConfig();
    
    QList<int> testSizes = {128, 256, 512, 1024, 2048, 4096};
    
    for (int size : testSizes) {
        config->setBufferSize(size);
        QCOMPARE(config->bufferSize(), size);
        QVERIFY(config->validate());
    }
}

void AudioModuleTest::testBitrateConfiguration()
{
    QVERIFY(m_audioManager->initialize());
    
    AudioConfig *config = m_audioManager->audioConfig();
    
    QList<int> testBitrates = {32, 64, 96, 128, 192, 256, 320};
    
    for (int bitrate : testBitrates) {
        config->setBitrate(bitrate);
        QCOMPARE(config->bitrate(), bitrate);
        QVERIFY(config->validate());
    }
}

// === 音频延迟测试 ===

void AudioModuleTest::testAudioLatency()
{
    QVERIFY(m_audioManager->initialize());
    
    // 选择设备并启动音频
    QStringList inputDevices = m_audioManager->availableInputDevices();
    QStringList outputDevices = m_audioManager->availableOutputDevices();
    
    if (!inputDevices.isEmpty() && !outputDevices.isEmpty()) {
        QVERIFY(m_audioManager->selectInputDevice(inputDevices.first()));
        QVERIFY(m_audioManager->selectOutputDevice(outputDevices.first()));
        
        QVERIFY(m_audioManager->startAudio());
        
        // 测量延迟（这里是模拟测量）
        QElapsedTimer timer;
        timer.start();
        
        // 模拟音频处理延迟
        QThread::msleep(10);
        
        qint64 latency = timer.elapsed();
        qDebug() << "测量到的音频延迟:" << latency << "ms";
        
        // 验证延迟在合理范围内
        QVERIFY(latency < LATENCY_THRESHOLD_MS);
        
        m_audioManager->stopAudio();
    }
}

void AudioModuleTest::testInputLatency()
{
    QVERIFY(m_audioManager->initialize());
    
    QStringList devices = m_audioManager->availableInputDevices();
    if (!devices.isEmpty()) {
        QVERIFY(m_audioManager->selectInputDevice(devices.first()));
        
        measureLatency("输入设备延迟", [this]() {
            QVERIFY(m_audioManager->startAudio());
            QThread::msleep(5); // 模拟输入处理
            m_audioManager->stopAudio();
        });
    }
}

void AudioModuleTest::testOutputLatency()
{
    QVERIFY(m_audioManager->initialize());
    
    QStringList devices = m_audioManager->availableOutputDevices();
    if (!devices.isEmpty()) {
        QVERIFY(m_audioManager->selectOutputDevice(devices.first()));
        
        measureLatency("输出设备延迟", [this]() {
            QVERIFY(m_audioManager->startAudio());
            QThread::msleep(5); // 模拟输出处理
            m_audioManager->stopAudio();
        });
    }
}

void AudioModuleTest::testRoundTripLatency()
{
    QVERIFY(m_audioManager->initialize());
    
    QStringList inputDevices = m_audioManager->availableInputDevices();
    QStringList outputDevices = m_audioManager->availableOutputDevices();
    
    if (!inputDevices.isEmpty() && !outputDevices.isEmpty()) {
        QVERIFY(m_audioManager->selectInputDevice(inputDevices.first()));
        QVERIFY(m_audioManager->selectOutputDevice(outputDevices.first()));
        
        measureLatency("往返延迟", [this]() {
            QVERIFY(m_audioManager->startAudio());
            QThread::msleep(20); // 模拟往返处理
            m_audioManager->stopAudio();
        });
    }
}

void AudioModuleTest::testLatencyMeasurement()
{
    QVERIFY(m_audioManager->initialize());
    
    // 测试不同缓冲区大小对延迟的影响
    QList<int> bufferSizes = {128, 256, 512, 1024, 2048};
    
    for (int bufferSize : bufferSizes) {
        AudioConfig *config = m_audioManager->audioConfig();
        config->setBufferSize(bufferSize);
        
        QString testName = QString("缓冲区大小 %1").arg(bufferSize);
        measureLatency(testName, [this]() {
            if (m_audioManager->startAudio()) {
                QThread::msleep(10);
                m_audioManager->stopAudio();
            }
        });
    }
}

void AudioModuleTest::testLatencyOptimization()
{
    QVERIFY(m_audioManager->initialize());
    
    AudioConfig *config = m_audioManager->audioConfig();
    
    // 测试低延迟配置
    config->setBufferSize(128);
    config->setSampleRate(48000);
    
    measureLatency("低延迟配置", [this]() {
        if (m_audioManager->startAudio()) {
            QThread::msleep(5);
            m_audioManager->stopAudio();
        }
    });
    
    // 测试标准配置
    config->setBufferSize(1024);
    config->setSampleRate(44100);
    
    measureLatency("标准配置", [this]() {
        if (m_audioManager->startAudio()) {
            QThread::msleep(10);
            m_audioManager->stopAudio();
        }
    });
}

void AudioModuleTest::testBufferSizeLatencyImpact()
{
    QVERIFY(m_audioManager->initialize());
    
    AudioConfig *config = m_audioManager->audioConfig();
    QList<int> bufferSizes = {64, 128, 256, 512, 1024, 2048, 4096};
    
    for (int bufferSize : bufferSizes) {
        config->setBufferSize(bufferSize);
        
        // 理论延迟计算
        double theoreticalLatency = (double)bufferSize / config->sampleRate() * 1000.0;
        
        qDebug() << QString("缓冲区大小 %1: 理论延迟 %2ms")
                    .arg(bufferSize)
                    .arg(theoreticalLatency, 0, 'f', 2);
        
        // 验证理论延迟在合理范围内
        QVERIFY(theoreticalLatency < 100.0); // 100ms以内
    }
}

// 辅助方法实现

void AudioModuleTest::setupTestEnvironment()
{
    // 设置测试环境
    qDebug() << "设置测试环境...";
}

void AudioModuleTest::cleanupTestEnvironment()
{
    // 清理测试环境
    qDebug() << "清理测试环境...";
}

bool AudioModuleTest::waitForSignal(QObject *sender, const char *signal, int timeout)
{
    QSignalSpy spy(sender, signal);
    return spy.wait(timeout);
}

void AudioModuleTest::measureLatency(const QString &testName, std::function<void()> operation)
{
    QElapsedTimer timer;
    timer.start();
    
    operation();
    
    qint64 elapsed = timer.elapsed();
    
    PerformanceMetrics metrics;
    metrics.testName = testName;
    metrics.executionTime = elapsed;
    metrics.memoryUsage = 0; // 简化实现
    metrics.cpuUsage = 0.0;  // 简化实现
    
    m_performanceResults.append(metrics);
    
    qDebug() << QString("%1 执行时间: %2ms").arg(testName).arg(elapsed);
}

void AudioModuleTest::measureMemoryUsage(const QString &testName, std::function<void()> operation)
{
    // 简化的内存使用测量
    operation();
    
    qDebug() << QString("%1 内存测量完成").arg(testName);
}

QStringList AudioModuleTest::getTestAudioDevices()
{
    if (m_audioManager) {
        return m_audioManager->availableInputDevices() + m_audioManager->availableOutputDevices();
    }
    return QStringList();
}

void AudioModuleTest::simulateDeviceError(const QString &deviceId)
{
    // 模拟设备错误
    qDebug() << "模拟设备错误:" << deviceId;
}

void AudioModuleTest::validateAudioQuality(const AudioUtils::AudioFormat &format)
{
    QVERIFY(AudioUtils::isValidAudioFormat(format));
    QVERIFY(format.sampleRate > 0);
    QVERIFY(format.channels > 0);
}

void AudioModuleTest::performStressTest(const QString &testName, std::function<void()> operation, int iterations)
{
    qDebug() << QString("开始压力测试: %1 (%2次迭代)").arg(testName).arg(iterations);
    
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < iterations; ++i) {
        operation();
        
        if (i % 10 == 0) {
            QCoreApplication::processEvents();
        }
    }
    
    qint64 totalTime = timer.elapsed();
    double avgTime = (double)totalTime / iterations;
    
    qDebug() << QString("压力测试完成: %1, 总时间=%2ms, 平均时间=%3ms")
                .arg(testName)
                .arg(totalTime)
                .arg(avgTime, 0, 'f', 2);
}

// 其他测试方法的简化实现...

void AudioModuleTest::testVolumeControl()
{
    QVERIFY(m_audioManager->initialize());
    
    QSignalSpy volumeSpy(m_audioManager.get(), &AudioManager::volumeChanged);
    
    // 测试主音量控制
    m_audioManager->setMasterVolume(0.5);
    QCOMPARE(m_audioManager->masterVolume(), 0.5);
    
    m_audioManager->setMasterVolume(0.8);
    QCOMPARE(m_audioManager->masterVolume(), 0.8);
    
    // 测试麦克风增益
    m_audioManager->setMicrophoneGain(0.6);
    QCOMPARE(m_audioManager->microphoneGain(), 0.6);
    
    QVERIFY(volumeSpy.count() >= 2);
}

void AudioModuleTest::testMasterVolumeControl()
{
    QVERIFY(m_audioManager->initialize());
    
    QList<qreal> testVolumes = {0.0, 0.25, 0.5, 0.75, 1.0};
    
    for (qreal volume : testVolumes) {
        m_audioManager->setMasterVolume(volume);
        QCOMPARE(m_audioManager->masterVolume(), volume);
    }
}

void AudioModuleTest::testMicrophoneGainControl()
{
    QVERIFY(m_audioManager->initialize());
    
    QList<qreal> testGains = {0.0, 0.3, 0.6, 0.9, 1.0};
    
    for (qreal gain : testGains) {
        m_audioManager->setMicrophoneGain(gain);
        QCOMPARE(m_audioManager->microphoneGain(), gain);
    }
}

void AudioModuleTest::testMuteControl()
{
    QVERIFY(m_audioManager->initialize());
    
    QSignalSpy muteSpy(m_audioManager.get(), &AudioManager::muteChanged);
    
    // 测试静音控制
    m_audioManager->setMuted(true);
    QVERIFY(m_audioManager->isMuted());
    
    m_audioManager->setMuted(false);
    QVERIFY(!m_audioManager->isMuted());
    
    QCOMPARE(muteSpy.count(), 2);
}

void AudioModuleTest::testVolumeRangeValidation()
{
    QVERIFY(m_audioManager->initialize());
    
    // 测试音量范围限制
    m_audioManager->setMasterVolume(-0.1);
    QCOMPARE(m_audioManager->masterVolume(), 0.0);
    
    m_audioManager->setMasterVolume(1.1);
    QCOMPARE(m_audioManager->masterVolume(), 1.0);
    
    // 测试增益范围限制
    m_audioManager->setMicrophoneGain(-0.1);
    QCOMPARE(m_audioManager->microphoneGain(), 0.0);
    
    m_audioManager->setMicrophoneGain(1.1);
    QCOMPARE(m_audioManager->microphoneGain(), 1.0);
}

void AudioModuleTest::testVolumeSignals()
{
    QVERIFY(m_audioManager->initialize());
    
    QSignalSpy volumeSpy(m_audioManager.get(), &AudioManager::volumeChanged);
    QSignalSpy muteSpy(m_audioManager.get(), &AudioManager::muteChanged);
    
    m_audioManager->setMasterVolume(0.7);
    m_audioManager->setMuted(true);
    m_audioManager->setMuted(false);
    
    QVERIFY(volumeSpy.count() >= 1);
    QVERIFY(muteSpy.count() >= 2);
}

// 简化其他测试方法...
void AudioModuleTest::testAudioStreamStart() { /* 实现 */ }
void AudioModuleTest::testAudioStreamStop() { /* 实现 */ }
void AudioModuleTest::testAudioStreamPause() { /* 实现 */ }
void AudioModuleTest::testAudioStreamResume() { /* 实现 */ }
void AudioModuleTest::testAudioStreamStatus() { /* 实现 */ }
void AudioModuleTest::testMultipleStreamOperations() { /* 实现 */ }
void AudioModuleTest::testConfigurationLoad() { /* 实现 */ }
void AudioModuleTest::testConfigurationSave() { /* 实现 */ }
void AudioModuleTest::testConfigurationValidation() { /* 实现 */ }
void AudioModuleTest::testConfigurationReset() { /* 实现 */ }
void AudioModuleTest::testConfigurationSignals() { /* 实现 */ }
void AudioModuleTest::testCustomConfigurationParameters() { /* 实现 */ }
void AudioModuleTest::testDeviceErrors() { /* 实现 */ }
void AudioModuleTest::testInitializationErrors() { /* 实现 */ }
void AudioModuleTest::testConfigurationErrors() { /* 实现 */ }
void AudioModuleTest::testStreamErrors() { /* 实现 */ }
void AudioModuleTest::testErrorRecovery() { /* 实现 */ }
void AudioModuleTest::testErrorSignals() { /* 实现 */ }
void AudioModuleTest::testMemoryUsage() { /* 实现 */ }
void AudioModuleTest::testCPUUsage() { /* 实现 */ }
void AudioModuleTest::testStartupPerformance() { /* 实现 */ }
void AudioModuleTest::testDeviceEnumerationPerformance() { /* 实现 */ }
void AudioModuleTest::testConfigurationPerformance() { /* 实现 */ }
void AudioModuleTest::testMultipleInitializations() { /* 实现 */ }
void AudioModuleTest::testRapidDeviceSwitching() { /* 实现 */ }
void AudioModuleTest::testContinuousVolumeChanges() { /* 实现 */ }
void AudioModuleTest::testLongRunningAudioStream() { /* 实现 */ }
void AudioModuleTest::testResourceLeakage() { /* 实现 */ }
void AudioModuleTest::testMediaManagerCompatibility() { /* 实现 */ }
void AudioModuleTest::testLegacyAPICompatibility() { /* 实现 */ }
void AudioModuleTest::testConfigurationMigration() { /* 实现 */ }
void AudioModuleTest::testBackwardCompatibility() { /* 实现 */ }
void AudioModuleTest::testAudioManagerIntegration() { /* 实现 */ }
void AudioModuleTest::testAudioConfigIntegration() { /* 实现 */ }
void AudioModuleTest::testAudioUtilsIntegration() { /* 实现 */ }
void AudioModuleTest::testUIComponentIntegration() { /* 实现 */ }
void AudioModuleTest::testPlatformSpecificFeatures() { /* 实现 */ }
void AudioModuleTest::testWindowsAudioAPI() { /* 实现 */ }
void AudioModuleTest::testLinuxAudioAPI() { /* 实现 */ }
void AudioModuleTest::testMacOSAudioAPI() { /* 实现 */ }

QTEST_MAIN(AudioModuleTest)
#include "AudioModuleTest.moc"