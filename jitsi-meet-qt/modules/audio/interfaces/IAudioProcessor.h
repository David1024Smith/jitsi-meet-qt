#ifndef IAUDIOPROCESSOR_H
#define IAUDIOPROCESSOR_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QByteArray>

/**
 * @brief 音频处理器接口
 * 
 * IAudioProcessor定义了音频处理器的标准接口，提供音频数据处理、
 * 音效应用、噪声抑制等功能。
 */
class IAudioProcessor : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 处理器状态枚举
     */
    enum ProcessorStatus {
        Idle,           ///< 空闲状态
        Processing,     ///< 处理中
        Error           ///< 错误状态
    };
    Q_ENUM(ProcessorStatus)

    /**
     * @brief 音频格式结构
     */
    struct AudioFormat {
        int sampleRate;     ///< 采样率 (Hz)
        int channels;       ///< 声道数
        int sampleSize;     ///< 样本大小 (bits)
        bool isFloat;       ///< 是否为浮点格式
        
        AudioFormat() : sampleRate(48000), channels(2), sampleSize(16), isFloat(false) {}
        AudioFormat(int rate, int ch, int size, bool floatFormat = false)
            : sampleRate(rate), channels(ch), sampleSize(size), isFloat(floatFormat) {}
    };

    /**
     * @brief 虚析构函数
     */
    virtual ~IAudioProcessor() = default;

    /**
     * @brief 初始化处理器
     * @param inputFormat 输入音频格式
     * @param outputFormat 输出音频格式
     * @return 初始化成功返回true
     */
    virtual bool initialize(const AudioFormat &inputFormat, 
                           const AudioFormat &outputFormat) = 0;

    /**
     * @brief 获取处理器状态
     * @return 当前状态
     */
    virtual ProcessorStatus status() const = 0;

    /**
     * @brief 处理音频数据
     * @param inputData 输入音频数据
     * @param outputData 输出音频数据
     * @return 处理成功返回true
     */
    virtual bool processAudio(const QByteArray &inputData, 
                             QByteArray &outputData) = 0;

    /**
     * @brief 设置处理参数
     * @param parameters 参数映射
     */
    virtual void setParameters(const QVariantMap &parameters) = 0;

    /**
     * @brief 获取处理参数
     * @return 参数映射
     */
    virtual QVariantMap parameters() const = 0;

    /**
     * @brief 启用噪声抑制
     * @param enabled 是否启用
     */
    virtual void setNoiseSuppressionEnabled(bool enabled) = 0;

    /**
     * @brief 检查噪声抑制是否启用
     * @return 启用返回true
     */
    virtual bool isNoiseSuppressionEnabled() const = 0;

    /**
     * @brief 启用回声消除
     * @param enabled 是否启用
     */
    virtual void setEchoCancellationEnabled(bool enabled) = 0;

    /**
     * @brief 检查回声消除是否启用
     * @return 启用返回true
     */
    virtual bool isEchoCancellationEnabled() const = 0;

    /**
     * @brief 启用自动增益控制
     * @param enabled 是否启用
     */
    virtual void setAutoGainControlEnabled(bool enabled) = 0;

    /**
     * @brief 检查自动增益控制是否启用
     * @return 启用返回true
     */
    virtual bool isAutoGainControlEnabled() const = 0;

    /**
     * @brief 设置音量增益
     * @param gain 增益值 (0.0-2.0, 1.0为原始音量)
     */
    virtual void setVolumeGain(qreal gain) = 0;

    /**
     * @brief 获取音量增益
     * @return 当前增益值
     */
    virtual qreal volumeGain() const = 0;

    /**
     * @brief 应用音频滤波器
     * @param filterType 滤波器类型
     * @param parameters 滤波器参数
     * @return 应用成功返回true
     */
    virtual bool applyFilter(const QString &filterType, 
                            const QVariantMap &parameters) = 0;

    /**
     * @brief 移除音频滤波器
     * @param filterType 滤波器类型
     */
    virtual void removeFilter(const QString &filterType) = 0;

    /**
     * @brief 获取支持的滤波器列表
     * @return 滤波器类型列表
     */
    virtual QStringList supportedFilters() const = 0;

    /**
     * @brief 重置处理器
     */
    virtual void reset() = 0;

    /**
     * @brief 获取处理延迟
     * @return 延迟时间 (毫秒)
     */
    virtual qreal processingLatency() const = 0;

    /**
     * @brief 获取音频级别
     * @return 音频级别 (0.0-1.0)
     */
    virtual qreal audioLevel() const = 0;

    /**
     * @brief 检查是否支持指定格式
     * @param format 音频格式
     * @return 支持返回true
     */
    virtual bool supportsFormat(const AudioFormat &format) const = 0;

signals:
    /**
     * @brief 状态改变信号
     * @param status 新状态
     */
    void statusChanged(ProcessorStatus status);

    /**
     * @brief 音频级别改变信号
     * @param level 新的音频级别
     */
    void audioLevelChanged(qreal level);

    /**
     * @brief 处理完成信号
     * @param inputSize 输入数据大小
     * @param outputSize 输出数据大小
     */
    void processingCompleted(int inputSize, int outputSize);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

    /**
     * @brief 参数改变信号
     * @param parameters 新参数
     */
    void parametersChanged(const QVariantMap &parameters);
};

Q_DECLARE_INTERFACE(IAudioProcessor, "org.jitsi.qt.IAudioProcessor/1.0")

#endif // IAUDIOPROCESSOR_H