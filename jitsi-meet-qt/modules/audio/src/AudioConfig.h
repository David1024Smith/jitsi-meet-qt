#ifndef AUDIOCONFIG_H
#define AUDIOCONFIG_H

#include <QString>
#include <QVariant>
#include <QMap>

/**
 * @brief 音频配置类
 * 
 * 提供音频模块的配置参数
 */
class AudioConfig
{
public:
    /**
     * @brief 音频设备类型枚举
     */
    enum DeviceType {
        InputDevice,    ///< 输入设备（麦克风）
        OutputDevice    ///< 输出设备（扬声器）
    };
    
    /**
     * @brief 音频格式枚举
     */
    enum AudioFormat {
        PCM_16,         ///< 16位PCM
        PCM_24,         ///< 24位PCM
        PCM_32,         ///< 32位PCM
        Float           ///< 浮点格式
    };
    
    /**
     * @brief 回声消除模式枚举
     */
    enum EchoMode {
        NoEchoCancellation,      ///< 无回声消除
        SoftwareEchoCancellation, ///< 软件回声消除
        HardwareEchoCancellation  ///< 硬件回声消除
    };
    
    /**
     * @brief 噪声抑制级别枚举
     */
    enum NoiseSuppressionLevel {
        NoNoiseSuppression,      ///< 无噪声抑制
        LowNoiseSuppression,     ///< 低噪声抑制
        MediumNoiseSuppression,  ///< 中噪声抑制
        HighNoiseSuppression     ///< 高噪声抑制
    };
    
    /**
     * @brief 构造函数
     */
    AudioConfig() {}
    
    /**
     * @brief 获取默认配置
     * @return 默认配置
     */
    static AudioConfig getDefaultConfig();
    
    /**
     * @brief 从QVariantMap加载配置
     * @param map 配置映射
     * @return 加载的配置
     */
    static AudioConfig fromVariantMap(const QVariantMap& map);
    
    /**
     * @brief 转换为QVariantMap
     * @return 配置映射
     */
    QVariantMap toVariantMap() const;
    
    // 配置属性
    QString inputDeviceId;           ///< 输入设备ID
    QString outputDeviceId;          ///< 输出设备ID
    int sampleRate = 48000;          ///< 采样率
    int channelCount = 2;            ///< 通道数
    AudioFormat format = PCM_16;     ///< 音频格式
    int bufferSize = 1024;           ///< 缓冲区大小
    int latency = 20;                ///< 延迟(毫秒)
    
    // 音频处理参数
    bool echoCancellation = true;    ///< 是否启用回声消除
    EchoMode echoMode = SoftwareEchoCancellation;  ///< 回声消除模式
    bool noiseSuppression = true;    ///< 是否启用噪声抑制
    NoiseSuppressionLevel noiseSuppressionLevel = MediumNoiseSuppression;  ///< 噪声抑制级别
    bool autoGainControl = true;     ///< 是否启用自动增益控制
    double gainLevel = 1.0;          ///< 增益级别
    
    // 高级参数
    QMap<QString, QVariant> advancedParams;  ///< 高级参数
};

#endif // AUDIOCONFIG_H
