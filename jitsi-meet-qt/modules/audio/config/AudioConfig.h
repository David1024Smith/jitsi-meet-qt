#ifndef AUDIOCONFIG_H
#define AUDIOCONFIG_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QSettings>

/**
 * @brief 音频配置管理类
 * 
 * AudioConfig负责管理音频模块的所有配置参数，包括设备设置、
 * 音频质量参数、音效设置等，并提供配置的持久化存储功能。
 */
class AudioConfig : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 音频质量预设枚举
     */
    enum QualityPreset {
        LowQuality,     ///< 低质量 (16kHz, 单声道, 64kbps)
        StandardQuality,///< 标准质量 (44.1kHz, 立体声, 128kbps)
        HighQuality     ///< 高质量 (48kHz, 立体声, 256kbps)
    };
    Q_ENUM(QualityPreset)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AudioConfig(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~AudioConfig();

    /**
     * @brief 加载配置
     * @return 加载成功返回true
     */
    bool load();

    /**
     * @brief 保存配置
     * @return 保存成功返回true
     */
    bool save();

    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults();

    /**
     * @brief 验证配置有效性
     * @return 配置有效返回true
     */
    bool validate() const;

    // 设备配置
    /**
     * @brief 设置首选输入设备ID
     * @param deviceId 设备ID
     */
    void setPreferredInputDevice(const QString &deviceId);

    /**
     * @brief 获取首选输入设备ID
     * @return 设备ID
     */
    QString preferredInputDevice() const;

    /**
     * @brief 设置首选输出设备ID
     * @param deviceId 设备ID
     */
    void setPreferredOutputDevice(const QString &deviceId);

    /**
     * @brief 获取首选输出设备ID
     * @return 设备ID
     */
    QString preferredOutputDevice() const;

    // 音频质量配置
    /**
     * @brief 设置采样率
     * @param sampleRate 采样率 (Hz)
     */
    void setSampleRate(int sampleRate);

    /**
     * @brief 获取采样率
     * @return 采样率 (Hz)
     */
    int sampleRate() const;

    /**
     * @brief 设置声道数
     * @param channels 声道数
     */
    void setChannels(int channels);

    /**
     * @brief 获取声道数
     * @return 声道数
     */
    int channels() const;

    /**
     * @brief 设置缓冲区大小
     * @param bufferSize 缓冲区大小 (样本数)
     */
    void setBufferSize(int bufferSize);

    /**
     * @brief 获取缓冲区大小
     * @return 缓冲区大小 (样本数)
     */
    int bufferSize() const;

    /**
     * @brief 设置比特率
     * @param bitrate 比特率 (kbps)
     */
    void setBitrate(int bitrate);

    /**
     * @brief 获取比特率
     * @return 比特率 (kbps)
     */
    int bitrate() const;

    /**
     * @brief 设置质量预设
     * @param preset 质量预设
     */
    void setQualityPreset(QualityPreset preset);

    /**
     * @brief 获取质量预设
     * @return 质量预设
     */
    QualityPreset qualityPreset() const;

    // 音量配置
    /**
     * @brief 设置主音量
     * @param volume 音量 (0.0-1.0)
     */
    void setMasterVolume(qreal volume);

    /**
     * @brief 获取主音量
     * @return 音量 (0.0-1.0)
     */
    qreal masterVolume() const;

    /**
     * @brief 设置麦克风增益
     * @param gain 增益 (0.0-1.0)
     */
    void setMicrophoneGain(qreal gain);

    /**
     * @brief 获取麦克风增益
     * @return 增益 (0.0-1.0)
     */
    qreal microphoneGain() const;

    /**
     * @brief 设置静音状态
     * @param muted 是否静音
     */
    void setMuted(bool muted);

    /**
     * @brief 获取静音状态
     * @return 是否静音
     */
    bool isMuted() const;

    // 音频处理配置
    /**
     * @brief 启用噪声抑制
     * @param enabled 是否启用
     */
    void setNoiseSuppressionEnabled(bool enabled);

    /**
     * @brief 检查噪声抑制是否启用
     * @return 启用返回true
     */
    bool isNoiseSuppressionEnabled() const;

    /**
     * @brief 启用回声消除
     * @param enabled 是否启用
     */
    void setEchoCancellationEnabled(bool enabled);

    /**
     * @brief 检查回声消除是否启用
     * @return 启用返回true
     */
    bool isEchoCancellationEnabled() const;

    /**
     * @brief 启用自动增益控制
     * @param enabled 是否启用
     */
    void setAutoGainControlEnabled(bool enabled);

    /**
     * @brief 检查自动增益控制是否启用
     * @return 启用返回true
     */
    bool isAutoGainControlEnabled() const;

    // 高级配置
    /**
     * @brief 设置自定义参数
     * @param key 参数键
     * @param value 参数值
     */
    void setCustomParameter(const QString &key, const QVariant &value);

    /**
     * @brief 获取自定义参数
     * @param key 参数键
     * @param defaultValue 默认值
     * @return 参数值
     */
    QVariant customParameter(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /**
     * @brief 获取所有自定义参数
     * @return 参数映射
     */
    QVariantMap customParameters() const;

    /**
     * @brief 转换为变体映射
     * @return 配置的变体映射表示
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 从变体映射加载配置
     * @param map 变体映射
     */
    void fromVariantMap(const QVariantMap &map);

    /**
     * @brief 获取配置文件路径
     * @return 配置文件路径
     */
    QString configFilePath() const;

    /**
     * @brief 设置配置文件路径
     * @param filePath 配置文件路径
     */
    void setConfigFilePath(const QString &filePath);

signals:
    /**
     * @brief 配置改变信号
     * @param key 改变的配置键
     * @param value 新值
     */
    void configChanged(const QString &key, const QVariant &value);

    /**
     * @brief 设备配置改变信号
     */
    void deviceConfigChanged();

    /**
     * @brief 音频质量配置改变信号
     */
    void qualityConfigChanged();

    /**
     * @brief 音量配置改变信号
     */
    void volumeConfigChanged();

    /**
     * @brief 音频处理配置改变信号
     */
    void processingConfigChanged();

private slots:
    /**
     * @brief 应用质量预设
     * @param preset 质量预设
     */
    void applyQualityPreset(QualityPreset preset);

private:
    /**
     * @brief 初始化默认值
     */
    void initializeDefaults();

    /**
     * @brief 发出配置改变信号
     * @param key 配置键
     * @param value 新值
     */
    void emitConfigChanged(const QString &key, const QVariant &value);

private:
    // 设备配置
    QString m_preferredInputDevice;
    QString m_preferredOutputDevice;

    // 音频质量配置
    int m_sampleRate;
    int m_channels;
    int m_bufferSize;
    int m_bitrate;
    QualityPreset m_qualityPreset;

    // 音量配置
    qreal m_masterVolume;
    qreal m_microphoneGain;
    bool m_muted;

    // 音频处理配置
    bool m_noiseSuppressionEnabled;
    bool m_echoCancellationEnabled;
    bool m_autoGainControlEnabled;

    // 自定义参数
    QVariantMap m_customParameters;

    // 配置文件
    QString m_configFilePath;
    QSettings *m_settings;
};

#endif // AUDIOCONFIG_H