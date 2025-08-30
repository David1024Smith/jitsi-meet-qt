#ifndef AUDIOUTILS_H
#define AUDIOUTILS_H

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QByteArray>

/**
 * @brief 音频工具类
 * 
 * AudioUtils提供音频模块中常用的工具函数，包括音频格式转换、
 * 音频数据处理、设备信息获取等实用功能。
 */
class AudioUtils
{
public:
    /**
     * @brief 音频格式结构
     */
    struct AudioFormat {
        int sampleRate;     ///< 采样率 (Hz)
        int channels;       ///< 声道数
        int sampleSize;     ///< 样本大小 (bits)
        bool isFloat;       ///< 是否为浮点格式
        bool isSigned;      ///< 是否为有符号格式
        
        AudioFormat() : sampleRate(48000), channels(2), sampleSize(16), 
                       isFloat(false), isSigned(true) {}
        
        bool operator==(const AudioFormat &other) const {
            return sampleRate == other.sampleRate &&
                   channels == other.channels &&
                   sampleSize == other.sampleSize &&
                   isFloat == other.isFloat &&
                   isSigned == other.isSigned;
        }
    };

    /**
     * @brief 音频质量预设枚举
     */
    enum QualityPreset {
        LowQuality,     ///< 低质量
        StandardQuality,///< 标准质量
        HighQuality     ///< 高质量
    };

    // 格式转换函数
    /**
     * @brief 转换音频格式
     * @param inputData 输入音频数据
     * @param inputFormat 输入格式
     * @param outputFormat 输出格式
     * @return 转换后的音频数据
     */
    static QByteArray convertAudioFormat(const QByteArray &inputData,
                                        const AudioFormat &inputFormat,
                                        const AudioFormat &outputFormat);

    /**
     * @brief 重采样音频数据
     * @param inputData 输入音频数据
     * @param inputSampleRate 输入采样率
     * @param outputSampleRate 输出采样率
     * @param channels 声道数
     * @return 重采样后的音频数据
     */
    static QByteArray resampleAudio(const QByteArray &inputData,
                                   int inputSampleRate,
                                   int outputSampleRate,
                                   int channels);

    /**
     * @brief 转换声道数
     * @param inputData 输入音频数据
     * @param inputChannels 输入声道数
     * @param outputChannels 输出声道数
     * @param sampleSize 样本大小 (bits)
     * @return 转换后的音频数据
     */
    static QByteArray convertChannels(const QByteArray &inputData,
                                     int inputChannels,
                                     int outputChannels,
                                     int sampleSize);

    // 音频数据处理函数
    /**
     * @brief 计算音频数据的RMS音量
     * @param audioData 音频数据
     * @param format 音频格式
     * @return RMS音量值 (0.0-1.0)
     */
    static qreal calculateRMSVolume(const QByteArray &audioData,
                                   const AudioFormat &format);

    /**
     * @brief 计算音频数据的峰值音量
     * @param audioData 音频数据
     * @param format 音频格式
     * @return 峰值音量 (0.0-1.0)
     */
    static qreal calculatePeakVolume(const QByteArray &audioData,
                                    const AudioFormat &format);

    /**
     * @brief 应用音量增益
     * @param audioData 音频数据
     * @param gain 增益值 (0.0-2.0)
     * @param format 音频格式
     * @return 应用增益后的音频数据
     */
    static QByteArray applyVolumeGain(const QByteArray &audioData,
                                     qreal gain,
                                     const AudioFormat &format);

    /**
     * @brief 混合两个音频流
     * @param audio1 第一个音频流
     * @param audio2 第二个音频流
     * @param format 音频格式
     * @param mixRatio 混合比例 (0.0-1.0, 0.5为等比例混合)
     * @return 混合后的音频数据
     */
    static QByteArray mixAudioStreams(const QByteArray &audio1,
                                     const QByteArray &audio2,
                                     const AudioFormat &format,
                                     qreal mixRatio = 0.5);

    // 格式验证和信息函数
    /**
     * @brief 验证音频格式是否有效
     * @param format 音频格式
     * @return 格式有效返回true
     */
    static bool isValidAudioFormat(const AudioFormat &format);

    /**
     * @brief 获取支持的采样率列表
     * @return 支持的采样率列表
     */
    static QList<int> supportedSampleRates();

    /**
     * @brief 获取支持的声道数列表
     * @return 支持的声道数列表
     */
    static QList<int> supportedChannelCounts();

    /**
     * @brief 获取支持的样本大小列表
     * @return 支持的样本大小列表 (bits)
     */
    static QList<int> supportedSampleSizes();

    /**
     * @brief 计算音频数据的字节大小
     * @param durationMs 持续时间 (毫秒)
     * @param format 音频格式
     * @return 字节大小
     */
    static int calculateAudioDataSize(int durationMs, const AudioFormat &format);

    /**
     * @brief 计算音频数据的持续时间
     * @param dataSize 数据大小 (字节)
     * @param format 音频格式
     * @return 持续时间 (毫秒)
     */
    static int calculateAudioDuration(int dataSize, const AudioFormat &format);

    // 质量预设函数
    /**
     * @brief 获取质量预设对应的音频格式
     * @param preset 质量预设
     * @return 音频格式
     */
    static AudioFormat getFormatForQualityPreset(QualityPreset preset);

    /**
     * @brief 获取质量预设的描述
     * @param preset 质量预设
     * @return 描述字符串
     */
    static QString getQualityPresetDescription(QualityPreset preset);

    /**
     * @brief 获取质量预设的比特率
     * @param preset 质量预设
     * @return 比特率 (kbps)
     */
    static int getBitrateForQualityPreset(QualityPreset preset);

    // 设备信息函数
    /**
     * @brief 格式化设备信息为可读字符串
     * @param deviceInfo 设备信息映射
     * @return 格式化的设备信息字符串
     */
    static QString formatDeviceInfo(const QVariantMap &deviceInfo);

    /**
     * @brief 解析设备ID
     * @param deviceId 设备ID
     * @return 解析后的设备信息
     */
    static QVariantMap parseDeviceId(const QString &deviceId);

    /**
     * @brief 生成设备友好名称
     * @param deviceName 原始设备名称
     * @param deviceId 设备ID
     * @return 友好名称
     */
    static QString generateFriendlyDeviceName(const QString &deviceName,
                                             const QString &deviceId);

    // 错误处理函数
    /**
     * @brief 获取音频错误的描述
     * @param errorCode 错误代码
     * @return 错误描述
     */
    static QString getAudioErrorDescription(int errorCode);

    /**
     * @brief 检查音频格式兼容性
     * @param format1 第一个格式
     * @param format2 第二个格式
     * @return 兼容返回true
     */
    static bool areFormatsCompatible(const AudioFormat &format1,
                                    const AudioFormat &format2);

    // 调试和诊断函数
    /**
     * @brief 生成音频格式的调试字符串
     * @param format 音频格式
     * @return 调试字符串
     */
    static QString formatToDebugString(const AudioFormat &format);

    /**
     * @brief 验证音频数据完整性
     * @param audioData 音频数据
     * @param format 音频格式
     * @return 数据完整返回true
     */
    static bool validateAudioData(const QByteArray &audioData,
                                 const AudioFormat &format);

    /**
     * @brief 生成测试音频数据 (正弦波)
     * @param frequency 频率 (Hz)
     * @param durationMs 持续时间 (毫秒)
     * @param format 音频格式
     * @param amplitude 振幅 (0.0-1.0)
     * @return 测试音频数据
     */
    static QByteArray generateTestTone(int frequency,
                                      int durationMs,
                                      const AudioFormat &format,
                                      qreal amplitude = 0.5);

private:
    /**
     * @brief 私有构造函数 (工具类不允许实例化)
     */
    AudioUtils() = delete;

    /**
     * @brief 转换样本格式
     * @param sample 输入样本
     * @param inputSize 输入样本大小
     * @param outputSize 输出样本大小
     * @param inputSigned 输入是否有符号
     * @param outputSigned 输出是否有符号
     * @return 转换后的样本
     */
    static qint32 convertSample(qint32 sample,
                               int inputSize,
                               int outputSize,
                               bool inputSigned,
                               bool outputSigned);
};

#endif // AUDIOUTILS_H