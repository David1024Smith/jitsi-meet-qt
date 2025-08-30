#include "AudioUtils.h"
#include <QtMath>
#include <QDebug>
#include <QAudioFormat>
#include <QRegularExpression>

// 静态常量定义
const QList<int> SUPPORTED_SAMPLE_RATES = {8000, 16000, 22050, 44100, 48000, 96000};
const QList<int> SUPPORTED_CHANNEL_COUNTS = {1, 2, 4, 6, 8};
const QList<int> SUPPORTED_SAMPLE_SIZES = {8, 16, 24, 32};

QByteArray AudioUtils::convertAudioFormat(const QByteArray &inputData,
                                         const AudioFormat &inputFormat,
                                         const AudioFormat &outputFormat)
{
    if (inputData.isEmpty() || !isValidAudioFormat(inputFormat) || !isValidAudioFormat(outputFormat)) {
        return QByteArray();
    }

    // 如果格式相同，直接返回
    if (inputFormat == outputFormat) {
        return inputData;
    }

    QByteArray result = inputData;

    // 1. 首先转换采样率
    if (inputFormat.sampleRate != outputFormat.sampleRate) {
        result = resampleAudio(result, inputFormat.sampleRate, outputFormat.sampleRate, inputFormat.channels);
    }

    // 2. 转换声道数
    if (inputFormat.channels != outputFormat.channels) {
        result = convertChannels(result, inputFormat.channels, outputFormat.channels, inputFormat.sampleSize);
    }

    // 3. 转换样本格式
    if (inputFormat.sampleSize != outputFormat.sampleSize ||
        inputFormat.isSigned != outputFormat.isSigned ||
        inputFormat.isFloat != outputFormat.isFloat) {
        
        // 这里实现样本格式转换的详细逻辑
        // 为了简化，这里只处理基本的16位有符号整数转换
        if (inputFormat.sampleSize == 16 && outputFormat.sampleSize == 16 &&
            inputFormat.isSigned && outputFormat.isSigned) {
            // 相同格式，无需转换
        } else {
            qWarning() << "AudioUtils: Complex sample format conversion not implemented";
        }
    }

    return result;
}

QByteArray AudioUtils::resampleAudio(const QByteArray &inputData,
                                    int inputSampleRate,
                                    int outputSampleRate,
                                    int channels)
{
    if (inputData.isEmpty() || inputSampleRate <= 0 || outputSampleRate <= 0 || channels <= 0) {
        return QByteArray();
    }

    if (inputSampleRate == outputSampleRate) {
        return inputData;
    }

    // 简单的线性插值重采样
    const qint16 *inputSamples = reinterpret_cast<const qint16*>(inputData.constData());
    int inputSampleCount = inputData.size() / (sizeof(qint16) * channels);
    
    double ratio = static_cast<double>(outputSampleRate) / inputSampleRate;
    int outputSampleCount = static_cast<int>(inputSampleCount * ratio);
    
    QByteArray outputData(outputSampleCount * sizeof(qint16) * channels, 0);
    qint16 *outputSamples = reinterpret_cast<qint16*>(outputData.data());

    for (int i = 0; i < outputSampleCount; ++i) {
        double sourceIndex = i / ratio;
        int index1 = static_cast<int>(sourceIndex);
        int index2 = qMin(index1 + 1, inputSampleCount - 1);
        double fraction = sourceIndex - index1;

        for (int ch = 0; ch < channels; ++ch) {
            qint16 sample1 = inputSamples[index1 * channels + ch];
            qint16 sample2 = inputSamples[index2 * channels + ch];
            qint16 interpolated = static_cast<qint16>(sample1 + fraction * (sample2 - sample1));
            outputSamples[i * channels + ch] = interpolated;
        }
    }

    return outputData;
}

QByteArray AudioUtils::convertChannels(const QByteArray &inputData,
                                      int inputChannels,
                                      int outputChannels,
                                      int sampleSize)
{
    if (inputData.isEmpty() || inputChannels <= 0 || outputChannels <= 0 || sampleSize != 16) {
        return QByteArray();
    }

    if (inputChannels == outputChannels) {
        return inputData;
    }

    const qint16 *inputSamples = reinterpret_cast<const qint16*>(inputData.constData());
    int frameCount = inputData.size() / (sizeof(qint16) * inputChannels);
    
    QByteArray outputData(frameCount * sizeof(qint16) * outputChannels, 0);
    qint16 *outputSamples = reinterpret_cast<qint16*>(outputData.data());

    for (int frame = 0; frame < frameCount; ++frame) {
        if (inputChannels == 1 && outputChannels == 2) {
            // 单声道转立体声
            qint16 monoSample = inputSamples[frame];
            outputSamples[frame * 2] = monoSample;
            outputSamples[frame * 2 + 1] = monoSample;
        } else if (inputChannels == 2 && outputChannels == 1) {
            // 立体声转单声道
            qint16 leftSample = inputSamples[frame * 2];
            qint16 rightSample = inputSamples[frame * 2 + 1];
            outputSamples[frame] = static_cast<qint16>((leftSample + rightSample) / 2);
        } else {
            // 其他转换情况的简化处理
            for (int ch = 0; ch < outputChannels; ++ch) {
                int sourceChannel = qMin(ch, inputChannels - 1);
                outputSamples[frame * outputChannels + ch] = inputSamples[frame * inputChannels + sourceChannel];
            }
        }
    }

    return outputData;
}

qreal AudioUtils::calculateRMSVolume(const QByteArray &audioData, const AudioFormat &format)
{
    if (audioData.isEmpty() || !isValidAudioFormat(format)) {
        return 0.0;
    }

    if (format.sampleSize != 16 || !format.isSigned) {
        qWarning() << "AudioUtils: RMS calculation only supports 16-bit signed samples";
        return 0.0;
    }

    const qint16 *samples = reinterpret_cast<const qint16*>(audioData.constData());
    int sampleCount = audioData.size() / sizeof(qint16);
    
    qint64 sumSquares = 0;
    for (int i = 0; i < sampleCount; ++i) {
        qint64 sample = samples[i];
        sumSquares += sample * sample;
    }

    double rms = qSqrt(static_cast<double>(sumSquares) / sampleCount);
    return qMin(1.0, rms / 32767.0); // 归一化到0-1范围
}

qreal AudioUtils::calculatePeakVolume(const QByteArray &audioData, const AudioFormat &format)
{
    if (audioData.isEmpty() || !isValidAudioFormat(format)) {
        return 0.0;
    }

    if (format.sampleSize != 16 || !format.isSigned) {
        qWarning() << "AudioUtils: Peak calculation only supports 16-bit signed samples";
        return 0.0;
    }

    const qint16 *samples = reinterpret_cast<const qint16*>(audioData.constData());
    int sampleCount = audioData.size() / sizeof(qint16);
    
    qint16 peak = 0;
    for (int i = 0; i < sampleCount; ++i) {
        peak = qMax(peak, static_cast<qint16>(qAbs(samples[i])));
    }

    return qMin(1.0, static_cast<double>(peak) / 32767.0);
}

QByteArray AudioUtils::applyVolumeGain(const QByteArray &audioData, qreal gain, const AudioFormat &format)
{
    if (audioData.isEmpty() || !isValidAudioFormat(format) || gain < 0.0) {
        return audioData;
    }

    if (format.sampleSize != 16 || !format.isSigned) {
        qWarning() << "AudioUtils: Volume gain only supports 16-bit signed samples";
        return audioData;
    }

    QByteArray result = audioData;
    qint16 *samples = reinterpret_cast<qint16*>(result.data());
    int sampleCount = result.size() / sizeof(qint16);

    for (int i = 0; i < sampleCount; ++i) {
        qint32 amplified = static_cast<qint32>(samples[i] * gain);
        samples[i] = static_cast<qint16>(qBound(-32768, amplified, 32767));
    }

    return result;
}

QByteArray AudioUtils::mixAudioStreams(const QByteArray &audio1,
                                      const QByteArray &audio2,
                                      const AudioFormat &format,
                                      qreal mixRatio)
{
    if (audio1.isEmpty() || audio2.isEmpty() || !isValidAudioFormat(format)) {
        return audio1.isEmpty() ? audio2 : audio1;
    }

    if (format.sampleSize != 16 || !format.isSigned) {
        qWarning() << "AudioUtils: Audio mixing only supports 16-bit signed samples";
        return audio1;
    }

    int minSize = qMin(audio1.size(), audio2.size());
    QByteArray result(minSize, 0);

    const qint16 *samples1 = reinterpret_cast<const qint16*>(audio1.constData());
    const qint16 *samples2 = reinterpret_cast<const qint16*>(audio2.constData());
    qint16 *resultSamples = reinterpret_cast<qint16*>(result.data());

    int sampleCount = minSize / sizeof(qint16);
    qreal ratio1 = 1.0 - mixRatio;
    qreal ratio2 = mixRatio;

    for (int i = 0; i < sampleCount; ++i) {
        qint32 mixed = static_cast<qint32>(samples1[i] * ratio1 + samples2[i] * ratio2);
        resultSamples[i] = static_cast<qint16>(qBound(-32768, mixed, 32767));
    }

    return result;
}

bool AudioUtils::isValidAudioFormat(const AudioFormat &format)
{
    return format.sampleRate > 0 &&
           format.channels > 0 &&
           format.sampleSize > 0 &&
           SUPPORTED_SAMPLE_RATES.contains(format.sampleRate) &&
           SUPPORTED_CHANNEL_COUNTS.contains(format.channels) &&
           SUPPORTED_SAMPLE_SIZES.contains(format.sampleSize);
}

QList<int> AudioUtils::supportedSampleRates()
{
    return SUPPORTED_SAMPLE_RATES;
}

QList<int> AudioUtils::supportedChannelCounts()
{
    return SUPPORTED_CHANNEL_COUNTS;
}

QList<int> AudioUtils::supportedSampleSizes()
{
    return SUPPORTED_SAMPLE_SIZES;
}

int AudioUtils::calculateAudioDataSize(int durationMs, const AudioFormat &format)
{
    if (durationMs <= 0 || !isValidAudioFormat(format)) {
        return 0;
    }

    int bytesPerSample = format.sampleSize / 8;
    int bytesPerSecond = format.sampleRate * format.channels * bytesPerSample;
    return (bytesPerSecond * durationMs) / 1000;
}

int AudioUtils::calculateAudioDuration(int dataSize, const AudioFormat &format)
{
    if (dataSize <= 0 || !isValidAudioFormat(format)) {
        return 0;
    }

    int bytesPerSample = format.sampleSize / 8;
    int bytesPerSecond = format.sampleRate * format.channels * bytesPerSample;
    return (dataSize * 1000) / bytesPerSecond;
}

AudioUtils::AudioFormat AudioUtils::getFormatForQualityPreset(QualityPreset preset)
{
    AudioFormat format;
    
    switch (preset) {
    case LowQuality:
        format.sampleRate = 16000;
        format.channels = 1;
        format.sampleSize = 16;
        break;
    case StandardQuality:
        format.sampleRate = 44100;
        format.channels = 2;
        format.sampleSize = 16;
        break;
    case HighQuality:
        format.sampleRate = 48000;
        format.channels = 2;
        format.sampleSize = 24;
        break;
    }
    
    format.isSigned = true;
    format.isFloat = false;
    return format;
}

QString AudioUtils::getQualityPresetDescription(QualityPreset preset)
{
    switch (preset) {
    case LowQuality:
        return QStringLiteral("低质量 (16kHz, 单声道)");
    case StandardQuality:
        return QStringLiteral("标准质量 (44.1kHz, 立体声)");
    case HighQuality:
        return QStringLiteral("高质量 (48kHz, 立体声, 24位)");
    }
    return QString();
}

int AudioUtils::getBitrateForQualityPreset(QualityPreset preset)
{
    AudioFormat format = getFormatForQualityPreset(preset);
    return (format.sampleRate * format.channels * format.sampleSize) / 1000; // kbps
}

QString AudioUtils::formatDeviceInfo(const QVariantMap &deviceInfo)
{
    QString result;
    
    if (deviceInfo.contains("name")) {
        result += QStringLiteral("设备名称: %1\n").arg(deviceInfo["name"].toString());
    }
    
    if (deviceInfo.contains("id")) {
        result += QStringLiteral("设备ID: %1\n").arg(deviceInfo["id"].toString());
    }
    
    if (deviceInfo.contains("driver")) {
        result += QStringLiteral("驱动: %1\n").arg(deviceInfo["driver"].toString());
    }
    
    if (deviceInfo.contains("channels")) {
        result += QStringLiteral("声道数: %1\n").arg(deviceInfo["channels"].toInt());
    }
    
    if (deviceInfo.contains("sampleRate")) {
        result += QStringLiteral("采样率: %1 Hz\n").arg(deviceInfo["sampleRate"].toInt());
    }
    
    return result.trimmed();
}

QVariantMap AudioUtils::parseDeviceId(const QString &deviceId)
{
    QVariantMap result;
    
    // 简单的设备ID解析，实际实现可能更复杂
    QRegularExpression re(R"(^([^:]+):(.+)$)");
    QRegularExpressionMatch match = re.match(deviceId);
    
    if (match.hasMatch()) {
        result["driver"] = match.captured(1);
        result["device"] = match.captured(2);
    } else {
        result["device"] = deviceId;
    }
    
    result["id"] = deviceId;
    return result;
}

QString AudioUtils::generateFriendlyDeviceName(const QString &deviceName, const QString &deviceId)
{
    QString friendlyName = deviceName;
    
    // 移除常见的技术前缀
    friendlyName.remove(QRegularExpression(R"(^(ALSA|DirectSound|WASAPI|CoreAudio):\s*)"));
    
    // 如果名称为空或太短，使用设备ID的一部分
    if (friendlyName.length() < 3) {
        QVariantMap deviceInfo = parseDeviceId(deviceId);
        friendlyName = deviceInfo["device"].toString();
    }
    
    // 限制长度
    if (friendlyName.length() > 50) {
        friendlyName = friendlyName.left(47) + "...";
    }
    
    return friendlyName;
}

QString AudioUtils::getAudioErrorDescription(int errorCode)
{
    switch (errorCode) {
    case 0:
        return QStringLiteral("无错误");
    case 1:
        return QStringLiteral("设备未找到");
    case 2:
        return QStringLiteral("设备忙碌");
    case 3:
        return QStringLiteral("权限不足");
    case 4:
        return QStringLiteral("格式不支持");
    case 5:
        return QStringLiteral("缓冲区溢出");
    case 6:
        return QStringLiteral("缓冲区不足");
    default:
        return QStringLiteral("未知错误 (%1)").arg(errorCode);
    }
}

bool AudioUtils::areFormatsCompatible(const AudioFormat &format1, const AudioFormat &format2)
{
    // 检查基本兼容性
    return isValidAudioFormat(format1) && 
           isValidAudioFormat(format2) &&
           format1.channels == format2.channels &&
           qAbs(format1.sampleRate - format2.sampleRate) <= format1.sampleRate * 0.1; // 10%容差
}

QString AudioUtils::formatToDebugString(const AudioFormat &format)
{
    return QStringLiteral("AudioFormat{sampleRate=%1, channels=%2, sampleSize=%3, signed=%4, float=%5}")
           .arg(format.sampleRate)
           .arg(format.channels)
           .arg(format.sampleSize)
           .arg(format.isSigned ? "true" : "false")
           .arg(format.isFloat ? "true" : "false");
}

bool AudioUtils::validateAudioData(const QByteArray &audioData, const AudioFormat &format)
{
    if (audioData.isEmpty() || !isValidAudioFormat(format)) {
        return false;
    }

    int bytesPerSample = format.sampleSize / 8;
    int expectedAlignment = format.channels * bytesPerSample;
    
    return (audioData.size() % expectedAlignment) == 0;
}

QByteArray AudioUtils::generateTestTone(int frequency, int durationMs, const AudioFormat &format, qreal amplitude)
{
    if (frequency <= 0 || durationMs <= 0 || !isValidAudioFormat(format) || amplitude <= 0.0) {
        return QByteArray();
    }

    if (format.sampleSize != 16 || !format.isSigned) {
        qWarning() << "AudioUtils: Test tone generation only supports 16-bit signed samples";
        return QByteArray();
    }

    int sampleCount = (format.sampleRate * durationMs) / 1000;
    QByteArray result(sampleCount * format.channels * sizeof(qint16), 0);
    qint16 *samples = reinterpret_cast<qint16*>(result.data());

    double angleIncrement = 2.0 * M_PI * frequency / format.sampleRate;
    qint16 maxAmplitude = static_cast<qint16>(32767 * amplitude);

    for (int i = 0; i < sampleCount; ++i) {
        qint16 sample = static_cast<qint16>(maxAmplitude * qSin(i * angleIncrement));
        for (int ch = 0; ch < format.channels; ++ch) {
            samples[i * format.channels + ch] = sample;
        }
    }

    return result;
}

qint32 AudioUtils::convertSample(qint32 sample, int inputSize, int outputSize, bool inputSigned, bool outputSigned)
{
    // 简化的样本格式转换
    if (inputSize == outputSize && inputSigned == outputSigned) {
        return sample;
    }

    // 这里应该实现完整的样本格式转换逻辑
    // 为了简化，只返回原样本
    Q_UNUSED(inputSigned)
    Q_UNUSED(outputSigned)
    
    return sample;
}