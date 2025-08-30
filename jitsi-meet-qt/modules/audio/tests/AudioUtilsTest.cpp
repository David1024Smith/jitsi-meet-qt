#include <QtTest/QtTest>
#include "../utils/AudioUtils.h"

/**
 * @brief AudioUtils工具类的单元测试
 */
class AudioUtilsTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 格式验证测试
    void testIsValidAudioFormat();
    void testSupportedFormats();
    
    // 音频数据计算测试
    void testCalculateAudioDataSize();
    void testCalculateAudioDuration();
    
    // 音频格式转换测试
    void testResampleAudio();
    void testConvertChannels();
    void testConvertAudioFormat();
    
    // 音量处理测试
    void testCalculateRMSVolume();
    void testCalculatePeakVolume();
    void testApplyVolumeGain();
    
    // 音频混合测试
    void testMixAudioStreams();
    
    // 质量预设测试
    void testQualityPresets();
    
    // 设备信息处理测试
    void testDeviceInfoFormatting();
    void testDeviceIdParsing();
    void testFriendlyDeviceName();
    
    // 测试音频生成测试
    void testGenerateTestTone();
    
    // 数据验证测试
    void testValidateAudioData();
    void testFormatCompatibility();

private:
    AudioUtils::AudioFormat createStandardFormat();
    QByteArray createTestAudioData(const AudioUtils::AudioFormat &format, int durationMs);
};

void AudioUtilsTest::initTestCase()
{
    qDebug() << "开始AudioUtils测试";
}

void AudioUtilsTest::cleanupTestCase()
{
    qDebug() << "AudioUtils测试完成";
}

void AudioUtilsTest::testIsValidAudioFormat()
{
    // 测试有效格式
    AudioUtils::AudioFormat validFormat;
    validFormat.sampleRate = 44100;
    validFormat.channels = 2;
    validFormat.sampleSize = 16;
    validFormat.isSigned = true;
    validFormat.isFloat = false;
    
    QVERIFY(AudioUtils::isValidAudioFormat(validFormat));
    
    // 测试无效格式
    AudioUtils::AudioFormat invalidFormat1;
    invalidFormat1.sampleRate = 0; // 无效采样率
    invalidFormat1.channels = 2;
    invalidFormat1.sampleSize = 16;
    
    QVERIFY(!AudioUtils::isValidAudioFormat(invalidFormat1));
    
    AudioUtils::AudioFormat invalidFormat2;
    invalidFormat2.sampleRate = 44100;
    invalidFormat2.channels = 0; // 无效声道数
    invalidFormat2.sampleSize = 16;
    
    QVERIFY(!AudioUtils::isValidAudioFormat(invalidFormat2));
    
    AudioUtils::AudioFormat invalidFormat3;
    invalidFormat3.sampleRate = 99999; // 不支持的采样率
    invalidFormat3.channels = 2;
    invalidFormat3.sampleSize = 16;
    
    QVERIFY(!AudioUtils::isValidAudioFormat(invalidFormat3));
}

void AudioUtilsTest::testSupportedFormats()
{
    QList<int> sampleRates = AudioUtils::supportedSampleRates();
    QVERIFY(!sampleRates.isEmpty());
    QVERIFY(sampleRates.contains(44100));
    QVERIFY(sampleRates.contains(48000));
    
    QList<int> channels = AudioUtils::supportedChannelCounts();
    QVERIFY(!channels.isEmpty());
    QVERIFY(channels.contains(1));
    QVERIFY(channels.contains(2));
    
    QList<int> sampleSizes = AudioUtils::supportedSampleSizes();
    QVERIFY(!sampleSizes.isEmpty());
    QVERIFY(sampleSizes.contains(16));
}

void AudioUtilsTest::testCalculateAudioDataSize()
{
    AudioUtils::AudioFormat format = createStandardFormat();
    
    // 测试1秒音频的数据大小
    int size1s = AudioUtils::calculateAudioDataSize(1000, format);
    int expected1s = 44100 * 2 * 2; // 44.1kHz * 2声道 * 2字节/样本
    QCOMPARE(size1s, expected1s);
    
    // 测试500毫秒音频的数据大小
    int size500ms = AudioUtils::calculateAudioDataSize(500, format);
    int expected500ms = expected1s / 2;
    QCOMPARE(size500ms, expected500ms);
    
    // 测试无效输入
    QCOMPARE(AudioUtils::calculateAudioDataSize(0, format), 0);
    QCOMPARE(AudioUtils::calculateAudioDataSize(-1, format), 0);
}

void AudioUtilsTest::testCalculateAudioDuration()
{
    AudioUtils::AudioFormat format = createStandardFormat();
    
    // 测试1秒音频数据的持续时间
    int dataSize1s = 44100 * 2 * 2;
    int duration = AudioUtils::calculateAudioDuration(dataSize1s, format);
    QCOMPARE(duration, 1000);
    
    // 测试500毫秒音频数据的持续时间
    int dataSize500ms = dataSize1s / 2;
    int duration500ms = AudioUtils::calculateAudioDuration(dataSize500ms, format);
    QCOMPARE(duration500ms, 500);
    
    // 测试无效输入
    QCOMPARE(AudioUtils::calculateAudioDuration(0, format), 0);
    QCOMPARE(AudioUtils::calculateAudioDuration(-1, format), 0);
}

void AudioUtilsTest::testResampleAudio()
{
    // 创建44.1kHz的测试数据
    AudioUtils::AudioFormat format44k = createStandardFormat();
    QByteArray testData = createTestAudioData(format44k, 100); // 100ms
    
    // 重采样到48kHz
    QByteArray resampled = AudioUtils::resampleAudio(testData, 44100, 48000, 2);
    
    // 验证输出数据大小合理
    QVERIFY(!resampled.isEmpty());
    
    // 计算期望的大小比例
    double ratio = 48000.0 / 44100.0;
    int expectedSize = static_cast<int>(testData.size() * ratio);
    
    // 允许一定的误差
    QVERIFY(qAbs(resampled.size() - expectedSize) <= 8); // 允许4个样本的误差
    
    // 测试相同采样率（应该返回原数据）
    QByteArray unchanged = AudioUtils::resampleAudio(testData, 44100, 44100, 2);
    QCOMPARE(unchanged, testData);
    
    // 测试无效输入
    QByteArray empty = AudioUtils::resampleAudio(QByteArray(), 44100, 48000, 2);
    QVERIFY(empty.isEmpty());
}

void AudioUtilsTest::testConvertChannels()
{
    // 创建立体声测试数据
    AudioUtils::AudioFormat stereoFormat = createStandardFormat();
    QByteArray stereoData = createTestAudioData(stereoFormat, 100);
    
    // 转换为单声道
    QByteArray monoData = AudioUtils::convertChannels(stereoData, 2, 1, 16);
    
    // 验证输出数据大小
    QCOMPARE(monoData.size(), stereoData.size() / 2);
    
    // 转换回立体声
    QByteArray backToStereo = AudioUtils::convertChannels(monoData, 1, 2, 16);
    QCOMPARE(backToStereo.size(), stereoData.size());
    
    // 测试相同声道数（应该返回原数据）
    QByteArray unchanged = AudioUtils::convertChannels(stereoData, 2, 2, 16);
    QCOMPARE(unchanged, stereoData);
    
    // 测试无效输入
    QByteArray empty = AudioUtils::convertChannels(QByteArray(), 2, 1, 16);
    QVERIFY(empty.isEmpty());
}

void AudioUtilsTest::testConvertAudioFormat()
{
    // 创建输入格式和数据
    AudioUtils::AudioFormat inputFormat;
    inputFormat.sampleRate = 44100;
    inputFormat.channels = 2;
    inputFormat.sampleSize = 16;
    inputFormat.isSigned = true;
    inputFormat.isFloat = false;
    
    QByteArray inputData = createTestAudioData(inputFormat, 100);
    
    // 创建输出格式
    AudioUtils::AudioFormat outputFormat;
    outputFormat.sampleRate = 48000;
    outputFormat.channels = 1;
    outputFormat.sampleSize = 16;
    outputFormat.isSigned = true;
    outputFormat.isFloat = false;
    
    // 执行格式转换
    QByteArray outputData = AudioUtils::convertAudioFormat(inputData, inputFormat, outputFormat);
    
    // 验证输出数据不为空
    QVERIFY(!outputData.isEmpty());
    
    // 测试相同格式（应该返回原数据）
    QByteArray unchanged = AudioUtils::convertAudioFormat(inputData, inputFormat, inputFormat);
    QCOMPARE(unchanged, inputData);
    
    // 测试无效输入
    QByteArray empty = AudioUtils::convertAudioFormat(QByteArray(), inputFormat, outputFormat);
    QVERIFY(empty.isEmpty());
}

void AudioUtilsTest::testCalculateRMSVolume()
{
    AudioUtils::AudioFormat format = createStandardFormat();
    
    // 测试静音数据
    QByteArray silentData(1000, 0);
    qreal silentRMS = AudioUtils::calculateRMSVolume(silentData, format);
    QCOMPARE(silentRMS, 0.0);
    
    // 测试满幅度数据
    QByteArray fullScaleData;
    fullScaleData.resize(1000);
    qint16 *samples = reinterpret_cast<qint16*>(fullScaleData.data());
    for (int i = 0; i < 500; ++i) {
        samples[i] = 32767; // 最大正值
    }
    
    qreal fullScaleRMS = AudioUtils::calculateRMSVolume(fullScaleData, format);
    QVERIFY(fullScaleRMS > 0.9); // 应该接近1.0
    
    // 测试无效输入
    qreal invalidRMS = AudioUtils::calculateRMSVolume(QByteArray(), format);
    QCOMPARE(invalidRMS, 0.0);
}

void AudioUtilsTest::testCalculatePeakVolume()
{
    AudioUtils::AudioFormat format = createStandardFormat();
    
    // 测试静音数据
    QByteArray silentData(1000, 0);
    qreal silentPeak = AudioUtils::calculatePeakVolume(silentData, format);
    QCOMPARE(silentPeak, 0.0);
    
    // 测试包含峰值的数据
    QByteArray peakData(1000, 0);
    qint16 *samples = reinterpret_cast<qint16*>(peakData.data());
    samples[100] = 16383; // 50%峰值
    
    qreal peak = AudioUtils::calculatePeakVolume(peakData, format);
    QVERIFY(qAbs(peak - 0.5) < 0.01); // 应该接近0.5
    
    // 测试无效输入
    qreal invalidPeak = AudioUtils::calculatePeakVolume(QByteArray(), format);
    QCOMPARE(invalidPeak, 0.0);
}

void AudioUtilsTest::testApplyVolumeGain()
{
    AudioUtils::AudioFormat format = createStandardFormat();
    QByteArray testData = createTestAudioData(format, 100);
    
    // 测试50%音量
    QByteArray halfVolume = AudioUtils::applyVolumeGain(testData, 0.5, format);
    QCOMPARE(halfVolume.size(), testData.size());
    
    qreal originalRMS = AudioUtils::calculateRMSVolume(testData, format);
    qreal halfRMS = AudioUtils::calculateRMSVolume(halfVolume, format);
    
    // 50%增益应该大约减半RMS值
    QVERIFY(qAbs(halfRMS - originalRMS * 0.5) < 0.1);
    
    // 测试100%音量（应该不变）
    QByteArray unchanged = AudioUtils::applyVolumeGain(testData, 1.0, format);
    QCOMPARE(unchanged, testData);
    
    // 测试无效增益
    QByteArray invalid = AudioUtils::applyVolumeGain(testData, -1.0, format);
    QCOMPARE(invalid, testData); // 应该返回原数据
}

void AudioUtilsTest::testMixAudioStreams()
{
    AudioUtils::AudioFormat format = createStandardFormat();
    QByteArray audio1 = createTestAudioData(format, 100);
    QByteArray audio2 = createTestAudioData(format, 100);
    
    // 测试等比例混合
    QByteArray mixed = AudioUtils::mixAudioStreams(audio1, audio2, format, 0.5);
    QCOMPARE(mixed.size(), qMin(audio1.size(), audio2.size()));
    
    // 测试不同比例混合
    QByteArray mixed75 = AudioUtils::mixAudioStreams(audio1, audio2, format, 0.75);
    QCOMPARE(mixed75.size(), qMin(audio1.size(), audio2.size()));
    
    // 测试空数据
    QByteArray emptyMix1 = AudioUtils::mixAudioStreams(QByteArray(), audio2, format, 0.5);
    QCOMPARE(emptyMix1, audio2);
    
    QByteArray emptyMix2 = AudioUtils::mixAudioStreams(audio1, QByteArray(), format, 0.5);
    QCOMPARE(emptyMix2, audio1);
}

void AudioUtilsTest::testQualityPresets()
{
    // 测试所有质量预设
    for (int i = 0; i < 3; ++i) {
        AudioUtils::QualityPreset preset = static_cast<AudioUtils::QualityPreset>(i);
        
        AudioUtils::AudioFormat format = AudioUtils::getFormatForQualityPreset(preset);
        QVERIFY(AudioUtils::isValidAudioFormat(format));
        
        QString description = AudioUtils::getQualityPresetDescription(preset);
        QVERIFY(!description.isEmpty());
        
        int bitrate = AudioUtils::getBitrateForQualityPreset(preset);
        QVERIFY(bitrate > 0);
        
        // 验证比特率计算
        int expectedBitrate = (format.sampleRate * format.channels * format.sampleSize) / 1000;
        QCOMPARE(bitrate, expectedBitrate);
    }
}

void AudioUtilsTest::testDeviceInfoFormatting()
{
    QVariantMap deviceInfo;
    deviceInfo["name"] = "测试设备";
    deviceInfo["id"] = "test_device_001";
    deviceInfo["driver"] = "ALSA";
    deviceInfo["channels"] = 2;
    deviceInfo["sampleRate"] = 44100;
    
    QString formatted = AudioUtils::formatDeviceInfo(deviceInfo);
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("测试设备"));
    QVERIFY(formatted.contains("test_device_001"));
    QVERIFY(formatted.contains("ALSA"));
    QVERIFY(formatted.contains("2"));
    QVERIFY(formatted.contains("44100"));
}

void AudioUtilsTest::testDeviceIdParsing()
{
    // 测试带驱动前缀的设备ID
    QString deviceId = "ALSA:hw:0,0";
    QVariantMap parsed = AudioUtils::parseDeviceId(deviceId);
    
    QCOMPARE(parsed["id"].toString(), deviceId);
    QCOMPARE(parsed["driver"].toString(), QString("ALSA"));
    QCOMPARE(parsed["device"].toString(), QString("hw:0,0"));
    
    // 测试简单设备ID
    QString simpleId = "default";
    QVariantMap simpleParsed = AudioUtils::parseDeviceId(simpleId);
    
    QCOMPARE(simpleParsed["id"].toString(), simpleId);
    QCOMPARE(simpleParsed["device"].toString(), simpleId);
    QVERIFY(!simpleParsed.contains("driver"));
}

void AudioUtilsTest::testFriendlyDeviceName()
{
    // 测试带技术前缀的设备名
    QString technicalName = "ALSA: USB Audio Device";
    QString deviceId = "alsa:usb:001";
    
    QString friendlyName = AudioUtils::generateFriendlyDeviceName(technicalName, deviceId);
    QVERIFY(!friendlyName.contains("ALSA:"));
    QVERIFY(friendlyName.contains("USB Audio Device"));
    
    // 测试空名称
    QString emptyName = "";
    QString friendlyFromEmpty = AudioUtils::generateFriendlyDeviceName(emptyName, deviceId);
    QVERIFY(!friendlyFromEmpty.isEmpty());
    
    // 测试长名称截断
    QString longName = QString("Very Long Device Name That Should Be Truncated Because It Exceeds The Maximum Length Limit");
    QString truncated = AudioUtils::generateFriendlyDeviceName(longName, deviceId);
    QVERIFY(truncated.length() <= 50);
    QVERIFY(truncated.endsWith("..."));
}

void AudioUtilsTest::testGenerateTestTone()
{
    AudioUtils::AudioFormat format = createStandardFormat();
    
    // 生成1秒440Hz测试音
    QByteArray testTone = AudioUtils::generateTestTone(440, 1000, format, 0.5);
    
    // 验证数据大小
    int expectedSize = AudioUtils::calculateAudioDataSize(1000, format);
    QCOMPARE(testTone.size(), expectedSize);
    
    // 验证数据不为空且不是静音
    QVERIFY(!testTone.isEmpty());
    qreal rms = AudioUtils::calculateRMSVolume(testTone, format);
    QVERIFY(rms > 0.1); // 应该有明显的音量
    
    // 测试无效参数
    QByteArray invalid1 = AudioUtils::generateTestTone(0, 1000, format, 0.5);
    QVERIFY(invalid1.isEmpty());
    
    QByteArray invalid2 = AudioUtils::generateTestTone(440, 0, format, 0.5);
    QVERIFY(invalid2.isEmpty());
    
    QByteArray invalid3 = AudioUtils::generateTestTone(440, 1000, format, 0.0);
    QVERIFY(invalid3.isEmpty());
}

void AudioUtilsTest::testValidateAudioData()
{
    AudioUtils::AudioFormat format = createStandardFormat();
    
    // 测试有效数据
    QByteArray validData = createTestAudioData(format, 100);
    QVERIFY(AudioUtils::validateAudioData(validData, format));
    
    // 测试无效数据大小（不对齐）
    QByteArray invalidData = validData.left(validData.size() - 1);
    QVERIFY(!AudioUtils::validateAudioData(invalidData, format));
    
    // 测试空数据
    QVERIFY(!AudioUtils::validateAudioData(QByteArray(), format));
    
    // 测试无效格式
    AudioUtils::AudioFormat invalidFormat;
    QVERIFY(!AudioUtils::validateAudioData(validData, invalidFormat));
}

void AudioUtilsTest::testFormatCompatibility()
{
    AudioUtils::AudioFormat format1 = createStandardFormat();
    AudioUtils::AudioFormat format2 = createStandardFormat();
    
    // 相同格式应该兼容
    QVERIFY(AudioUtils::areFormatsCompatible(format1, format2));
    
    // 不同声道数不兼容
    format2.channels = 1;
    QVERIFY(!AudioUtils::areFormatsCompatible(format1, format2));
    
    // 恢复声道数，测试采样率差异
    format2.channels = format1.channels;
    format2.sampleRate = 48000; // 在10%容差内
    QVERIFY(AudioUtils::areFormatsCompatible(format1, format2));
    
    // 采样率差异过大
    format2.sampleRate = 96000; // 超过10%容差
    QVERIFY(!AudioUtils::areFormatsCompatible(format1, format2));
}

AudioUtils::AudioFormat AudioUtilsTest::createStandardFormat()
{
    AudioUtils::AudioFormat format;
    format.sampleRate = 44100;
    format.channels = 2;
    format.sampleSize = 16;
    format.isSigned = true;
    format.isFloat = false;
    return format;
}

QByteArray AudioUtilsTest::createTestAudioData(const AudioUtils::AudioFormat &format, int durationMs)
{
    // 生成简单的测试音频数据（正弦波）
    return AudioUtils::generateTestTone(1000, durationMs, format, 0.3);
}

QTEST_MAIN(AudioUtilsTest)
#include "AudioUtilsTest.moc"