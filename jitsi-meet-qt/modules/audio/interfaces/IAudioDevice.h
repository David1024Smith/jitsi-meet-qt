#ifndef IAUDIODEVICE_H
#define IAUDIODEVICE_H

#include <QObject>
#include <QString>

/**
 * @brief 音频设备接口
 * 
 * IAudioDevice定义了音频设备的标准接口，包括设备控制、
 * 状态管理和音频参数设置等功能。
 */
class IAudioDevice : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 设备类型枚举
     */
    enum DeviceType {
        Input,      ///< 输入设备 (麦克风)
        Output      ///< 输出设备 (扬声器)
    };
    Q_ENUM(DeviceType)

    /**
     * @brief 设备状态枚举
     */
    enum Status {
        Inactive,   ///< 非活动状态
        Active,     ///< 活动状态
        Error       ///< 错误状态
    };
    Q_ENUM(Status)

    /**
     * @brief 音频质量预设枚举
     */
    enum QualityPreset {
        LowQuality,     ///< 低质量 (节省带宽)
        StandardQuality,///< 标准质量 (平衡)
        HighQuality     ///< 高质量 (最佳音质)
    };
    Q_ENUM(QualityPreset)

    /**
     * @brief 虚析构函数
     */
    virtual ~IAudioDevice() = default;

    /**
     * @brief 初始化设备
     * @return 初始化成功返回true
     */
    virtual bool initialize() = 0;

    /**
     * @brief 启动设备
     * @return 启动成功返回true
     */
    virtual bool start() = 0;

    /**
     * @brief 停止设备
     */
    virtual void stop() = 0;

    /**
     * @brief 检查设备是否活动
     * @return 设备活动返回true
     */
    virtual bool isActive() const = 0;

    /**
     * @brief 获取设备ID
     * @return 设备唯一标识符
     */
    virtual QString deviceId() const = 0;

    /**
     * @brief 获取设备名称
     * @return 设备显示名称
     */
    virtual QString deviceName() const = 0;

    /**
     * @brief 获取设备类型
     * @return 设备类型
     */
    virtual DeviceType deviceType() const = 0;

    /**
     * @brief 获取设备状态
     * @return 当前设备状态
     */
    virtual Status status() const = 0;

    /**
     * @brief 设置音量
     * @param volume 音量值 (0.0-1.0)
     */
    virtual void setVolume(qreal volume) = 0;

    /**
     * @brief 获取音量
     * @return 当前音量值 (0.0-1.0)
     */
    virtual qreal volume() const = 0;

    /**
     * @brief 设置静音状态
     * @param muted 是否静音
     */
    virtual void setMuted(bool muted) = 0;

    /**
     * @brief 获取静音状态
     * @return 是否静音
     */
    virtual bool isMuted() const = 0;

    /**
     * @brief 设置音频质量预设
     * @param preset 质量预设
     */
    virtual void setQualityPreset(QualityPreset preset) = 0;

    /**
     * @brief 获取音频质量预设
     * @return 当前质量预设
     */
    virtual QualityPreset qualityPreset() const = 0;

    /**
     * @brief 获取采样率
     * @return 采样率 (Hz)
     */
    virtual int sampleRate() const = 0;

    /**
     * @brief 设置采样率
     * @param sampleRate 采样率 (Hz)
     */
    virtual void setSampleRate(int sampleRate) = 0;

    /**
     * @brief 获取声道数
     * @return 声道数
     */
    virtual int channels() const = 0;

    /**
     * @brief 设置声道数
     * @param channels 声道数
     */
    virtual void setChannels(int channels) = 0;

    /**
     * @brief 获取缓冲区大小
     * @return 缓冲区大小 (样本数)
     */
    virtual int bufferSize() const = 0;

    /**
     * @brief 设置缓冲区大小
     * @param bufferSize 缓冲区大小 (样本数)
     */
    virtual void setBufferSize(int bufferSize) = 0;

    /**
     * @brief 获取设备延迟
     * @return 延迟时间 (毫秒)
     */
    virtual qreal latency() const = 0;

    /**
     * @brief 检查设备是否支持指定格式
     * @param sampleRate 采样率
     * @param channels 声道数
     * @return 支持返回true
     */
    virtual bool supportsFormat(int sampleRate, int channels) const = 0;

signals:
    /**
     * @brief 设备状态改变信号
     * @param status 新状态
     */
    void statusChanged(Status status);

    /**
     * @brief 音量改变信号
     * @param volume 新音量值
     */
    void volumeChanged(qreal volume);

    /**
     * @brief 静音状态改变信号
     * @param muted 新静音状态
     */
    void muteChanged(bool muted);

    /**
     * @brief 设备错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

    /**
     * @brief 音频格式改变信号
     * @param sampleRate 新采样率
     * @param channels 新声道数
     */
    void formatChanged(int sampleRate, int channels);

    /**
     * @brief 设备连接状态改变信号
     * @param connected 是否连接
     */
    void connectionChanged(bool connected);
};

Q_DECLARE_INTERFACE(IAudioDevice, "org.jitsi.qt.IAudioDevice/1.0")

#endif // IAUDIODEVICE_H