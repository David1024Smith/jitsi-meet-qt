#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTemporaryFile>
#include "../config/AudioConfig.h"

/**
 * @brief AudioConfig类的单元测试
 */
class AudioConfigTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基础功能测试
    void testDefaultValues();
    void testDeviceConfiguration();
    void testQualityConfiguration();
    void testVolumeConfiguration();
    void testProcessingConfiguration();
    void testCustomParameters();

    // 质量预设测试
    void testQualityPresets();
    void testQualityPresetApplication();

    // 配置持久化测试
    void testConfigurationSaveLoad();
    void testConfigurationValidation();
    void testConfigurationReset();

    // 信号测试
    void testConfigChangeSignals();
    void testSpecificConfigSignals();

    // 边界值测试
    void testBoundaryValues();
    void testInvalidValues();

    // 序列化测试
    void testVariantMapSerialization();

private:
    AudioConfig *m_audioConfig;
    QString m_tempConfigPath;
};

void AudioConfigTest::initTestCase()
{
    // 测试开始前的初始化
}

void AudioConfigTest::cleanupTestCase()
{
    // 测试结束后的清理
}

void AudioConfigTest::init()
{
    // 每个测试前创建新的AudioConfig实例
    m_audioConfig = new AudioConfig(this);
    
    // 创建临时配置文件
    QTemporaryFile tempFile;
    tempFile.open();
    m_tempConfigPath = tempFile.fileName();
    tempFile.close();
    
    m_audioConfig->setConfigFilePath(m_tempConfigPath);
}

void AudioConfigTest::cleanup()
{
    // 每个测试后清理
    delete m_audioConfig;
    m_audioConfig = nullptr;
    
    // 删除临时配置文件
    QFile::remove(m_tempConfigPath);
}

void AudioConfigTest::testDefaultValues()
{
    // 测试默认值
    QCOMPARE(m_audioConfig->sampleRate(), 48000);
    QCOMPARE(m_audioConfig->channels(), 2);
    QCOMPARE(m_audioConfig->bufferSize(), 1024);
    QCOMPARE(m_audioConfig->bitrate(), 128);
    QCOMPARE(m_audioConfig->qualityPreset(), AudioConfig::StandardQuality);
    
    QCOMPARE(m_audioConfig->masterVolume(), 0.8);
    QCOMPARE(m_audioConfig->microphoneGain(), 0.6);
    QCOMPARE(m_audioConfig->isMuted(), false);
    
    QCOMPARE(m_audioConfig->isNoiseSuppressionEnabled(), true);
    QCOMPARE(m_audioConfig->isEchoCancellationEnabled(), true);
    QCOMPARE(m_audioConfig->isAutoGainControlEnabled(), false);
    
    QVERIFY(m_audioConfig->preferredInputDevice().isEmpty());
    QVERIFY(m_audioConfig->preferredOutputDevice().isEmpty());
}

void AudioConfigTest::testDeviceConfiguration()
{
    // 测试设备配置
    QString inputDevice = "test_input_device";
    QString outputDevice = "test_output_device";
    
    m_audioConfig->setPreferredInputDevice(inputDevice);
    QCOMPARE(m_audioConfig->preferredInputDevice(), inputDevice);
    
    m_audioConfig->setPreferredOutputDevice(outputDevice);
    QCOMPARE(m_audioConfig->preferredOutputDevice(), outputDevice);
}

void AudioConfigTest::testQualityConfiguration()
{
    // 测试音频质量配置
    m_audioConfig->setSampleRate(44100);
    QCOMPARE(m_audioConfig->sampleRate(), 44100);
    
    m_audioConfig->setChannels(1);
    QCOMPARE(m_audioConfig->channels(), 1);
    
    m_audioConfig->setBufferSize(2048);
    QCOMPARE(m_audioConfig->bufferSize(), 2048);
    
    m_audioConfig->setBitrate(256);
    QCOMPARE(m_audioConfig->bitrate(), 256);
}

void AudioConfigTest::testVolumeConfiguration()
{
    // 测试音量配置
    m_audioConfig->setMasterVolume(0.5);
    QCOMPARE(m_audioConfig->masterVolume(), 0.5);
    
    m_audioConfig->setMicrophoneGain(0.8);
    QCOMPARE(m_audioConfig->microphoneGain(), 0.8);
    
    m_audioConfig->setMuted(true);
    QCOMPARE(m_audioConfig->isMuted(), true);
}

void AudioConfigTest::testProcessingConfiguration()
{
    // 测试音频处理配置
    m_audioConfig->setNoiseSuppressionEnabled(false);
    QCOMPARE(m_audioConfig->isNoiseSuppressionEnabled(), false);
    
    m_audioConfig->setEchoCancellationEnabled(false);
    QCOMPARE(m_audioConfig->isEchoCancellationEnabled(), false);
    
    m_audioConfig->setAutoGainControlEnabled(true);
    QCOMPARE(m_audioConfig->isAutoGainControlEnabled(), true);
}

void AudioConfigTest::testCustomParameters()
{
    // 测试自定义参数
    QString key = "test_parameter";
    QVariant value = "test_value";
    
    m_audioConfig->setCustomParameter(key, value);
    QCOMPARE(m_audioConfig->customParameter(key), value);
    
    // 测试默认值
    QString nonExistentKey = "non_existent";
    QVariant defaultValue = "default";
    QCOMPARE(m_audioConfig->customParameter(nonExistentKey, defaultValue), defaultValue);
    
    // 测试获取所有自定义参数
    QVariantMap customParams = m_audioConfig->customParameters();
    QVERIFY(customParams.contains(key));
    QCOMPARE(customParams[key], value);
}

void AudioConfigTest::testQualityPresets()
{
    // 测试低质量预设
    m_audioConfig->setQualityPreset(AudioConfig::LowQuality);
    QCOMPARE(m_audioConfig->qualityPreset(), AudioConfig::LowQuality);
    QCOMPARE(m_audioConfig->sampleRate(), 16000);
    QCOMPARE(m_audioConfig->channels(), 1);
    QCOMPARE(m_audioConfig->bitrate(), 64);
    QCOMPARE(m_audioConfig->bufferSize(), 512);
    
    // 测试标准质量预设
    m_audioConfig->setQualityPreset(AudioConfig::StandardQuality);
    QCOMPARE(m_audioConfig->qualityPreset(), AudioConfig::StandardQuality);
    QCOMPARE(m_audioConfig->sampleRate(), 44100);
    QCOMPARE(m_audioConfig->channels(), 2);
    QCOMPARE(m_audioConfig->bitrate(), 128);
    QCOMPARE(m_audioConfig->bufferSize(), 1024);
    
    // 测试高质量预设
    m_audioConfig->setQualityPreset(AudioConfig::HighQuality);
    QCOMPARE(m_audioConfig->qualityPreset(), AudioConfig::HighQuality);
    QCOMPARE(m_audioConfig->sampleRate(), 48000);
    QCOMPARE(m_audioConfig->channels(), 2);
    QCOMPARE(m_audioConfig->bitrate(), 256);
    QCOMPARE(m_audioConfig->bufferSize(), 2048);
}

void AudioConfigTest::testQualityPresetApplication()
{
    // 测试质量预设的自动应用
    m_audioConfig->setSampleRate(22050); // 设置一个自定义值
    m_audioConfig->setQualityPreset(AudioConfig::LowQuality);
    
    // 验证预设覆盖了自定义值
    QCOMPARE(m_audioConfig->sampleRate(), 16000);
}

void AudioConfigTest::testConfigurationSaveLoad()
{
    // 设置一些配置值
    m_audioConfig->setPreferredInputDevice("input_device");
    m_audioConfig->setPreferredOutputDevice("output_device");
    m_audioConfig->setSampleRate(44100);
    m_audioConfig->setMasterVolume(0.7);
    m_audioConfig->setMuted(true);
    m_audioConfig->setCustomParameter("test_key", "test_value");
    
    // 保存配置
    QVERIFY(m_audioConfig->save());
    
    // 创建新的AudioConfig实例并加载配置
    AudioConfig newConfig;
    newConfig.setConfigFilePath(m_tempConfigPath);
    QVERIFY(newConfig.load());
    
    // 验证配置被正确加载
    QCOMPARE(newConfig.preferredInputDevice(), QString("input_device"));
    QCOMPARE(newConfig.preferredOutputDevice(), QString("output_device"));
    QCOMPARE(newConfig.sampleRate(), 44100);
    QCOMPARE(newConfig.masterVolume(), 0.7);
    QCOMPARE(newConfig.isMuted(), true);
    QCOMPARE(newConfig.customParameter("test_key"), QVariant("test_value"));
}

void AudioConfigTest::testConfigurationValidation()
{
    // 测试有效配置
    QVERIFY(m_audioConfig->validate());
    
    // 测试无效采样率
    m_audioConfig->setSampleRate(1000); // 太低
    QVERIFY(!m_audioConfig->validate());
    
    m_audioConfig->setSampleRate(300000); // 太高
    QVERIFY(!m_audioConfig->validate());
    
    // 恢复有效值
    m_audioConfig->setSampleRate(48000);
    QVERIFY(m_audioConfig->validate());
    
    // 测试无效声道数
    m_audioConfig->setChannels(0);
    QVERIFY(!m_audioConfig->validate());
    
    m_audioConfig->setChannels(10); // 太多
    QVERIFY(!m_audioConfig->validate());
    
    // 恢复有效值
    m_audioConfig->setChannels(2);
    QVERIFY(m_audioConfig->validate());
}

void AudioConfigTest::testConfigurationReset()
{
    // 修改一些配置
    m_audioConfig->setPreferredInputDevice("test_device");
    m_audioConfig->setSampleRate(22050);
    m_audioConfig->setMasterVolume(0.3);
    m_audioConfig->setMuted(true);
    
    // 重置配置
    m_audioConfig->resetToDefaults();
    
    // 验证配置被重置为默认值
    QVERIFY(m_audioConfig->preferredInputDevice().isEmpty());
    QCOMPARE(m_audioConfig->sampleRate(), 48000);
    QCOMPARE(m_audioConfig->masterVolume(), 0.8);
    QCOMPARE(m_audioConfig->isMuted(), false);
}

void AudioConfigTest::testConfigChangeSignals()
{
    // 测试通用配置改变信号
    QSignalSpy configChangedSpy(m_audioConfig, &AudioConfig::configChanged);
    
    m_audioConfig->setSampleRate(44100);
    QCOMPARE(configChangedSpy.count(), 1);
    
    QList<QVariant> arguments = configChangedSpy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QString("SampleRate"));
    QCOMPARE(arguments.at(1).toInt(), 44100);
}

void AudioConfigTest::testSpecificConfigSignals()
{
    // 测试特定配置改变信号
    QSignalSpy deviceConfigSpy(m_audioConfig, &AudioConfig::deviceConfigChanged);
    QSignalSpy qualityConfigSpy(m_audioConfig, &AudioConfig::qualityConfigChanged);
    QSignalSpy volumeConfigSpy(m_audioConfig, &AudioConfig::volumeConfigChanged);
    QSignalSpy processingConfigSpy(m_audioConfig, &AudioConfig::processingConfigChanged);
    
    // 测试设备配置信号
    m_audioConfig->setPreferredInputDevice("test_device");
    QCOMPARE(deviceConfigSpy.count(), 1);
    
    // 测试质量配置信号
    m_audioConfig->setSampleRate(44100);
    QCOMPARE(qualityConfigSpy.count(), 1);
    
    // 测试音量配置信号
    m_audioConfig->setMasterVolume(0.5);
    QCOMPARE(volumeConfigSpy.count(), 1);
    
    // 测试处理配置信号
    m_audioConfig->setNoiseSuppressionEnabled(false);
    QCOMPARE(processingConfigSpy.count(), 1);
}

void AudioConfigTest::testBoundaryValues()
{
    // 测试音量边界值
    m_audioConfig->setMasterVolume(-0.1); // 应该被限制为0.0
    QCOMPARE(m_audioConfig->masterVolume(), 0.0);
    
    m_audioConfig->setMasterVolume(1.1); // 应该被限制为1.0
    QCOMPARE(m_audioConfig->masterVolume(), 1.0);
    
    m_audioConfig->setMicrophoneGain(-0.1); // 应该被限制为0.0
    QCOMPARE(m_audioConfig->microphoneGain(), 0.0);
    
    m_audioConfig->setMicrophoneGain(1.1); // 应该被限制为1.0
    QCOMPARE(m_audioConfig->microphoneGain(), 1.0);
}

void AudioConfigTest::testInvalidValues()
{
    // 测试无效值的处理
    int originalSampleRate = m_audioConfig->sampleRate();
    
    // 设置相同的值不应该触发信号
    QSignalSpy configChangedSpy(m_audioConfig, &AudioConfig::configChanged);
    m_audioConfig->setSampleRate(originalSampleRate);
    QCOMPARE(configChangedSpy.count(), 0);
}

void AudioConfigTest::testVariantMapSerialization()
{
    // 设置一些配置值
    m_audioConfig->setPreferredInputDevice("input_device");
    m_audioConfig->setSampleRate(44100);
    m_audioConfig->setMasterVolume(0.7);
    m_audioConfig->setCustomParameter("test_key", "test_value");
    
    // 转换为VariantMap
    QVariantMap map = m_audioConfig->toVariantMap();
    
    // 验证序列化结果
    QCOMPARE(map["PreferredInputDevice"].toString(), QString("input_device"));
    QCOMPARE(map["SampleRate"].toInt(), 44100);
    QCOMPARE(map["MasterVolume"].toReal(), 0.7);
    
    QVariantMap customParams = map["CustomParameters"].toMap();
    QCOMPARE(customParams["test_key"].toString(), QString("test_value"));
    
    // 创建新的AudioConfig并从VariantMap加载
    AudioConfig newConfig;
    newConfig.fromVariantMap(map);
    
    // 验证反序列化结果
    QCOMPARE(newConfig.preferredInputDevice(), QString("input_device"));
    QCOMPARE(newConfig.sampleRate(), 44100);
    QCOMPARE(newConfig.masterVolume(), 0.7);
    QCOMPARE(newConfig.customParameter("test_key"), QVariant("test_value"));
}

QTEST_MAIN(AudioConfigTest)
#include "AudioConfigTest.moc"