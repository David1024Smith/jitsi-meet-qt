#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <memory>

// 前向声明
class IAudioDevice;
class IAudioManager;
class AudioConfig;

/**
 * @brief 音频管理器类
 * 
 * AudioManager是音频模块的高级管理类，提供音频设备管理、
 * 音频流控制和配置管理等高级功能。
 */
class AudioManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 管理器状态枚举
     */
    enum ManagerStatus {
        Uninitialized,  ///< 未初始化
        Ready,          ///< 就绪
        Busy,           ///< 忙碌
        Error           ///< 错误
    };
    Q_ENUM(ManagerStatus)

    /**
     * @brief 音频质量预设枚举
     */
    enum QualityPreset {
        LowQuality,     ///< 低质量 (16kHz, 单声道)
        StandardQuality,///< 标准质量 (44.1kHz, 立体声)
        HighQuality     ///< 高质量 (48kHz, 立体声)
    };
    Q_ENUM(QualityPreset)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AudioManager(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~AudioManager();

    /**
     * @brief 初始化音频管理器
     * @return 初始化成功返回true
     */
    bool initialize();

    /**
     * @brief 获取管理器状态
     * @return 当前状态
     */
    ManagerStatus status() const;

    /**
     * @brief 获取可用输入设备列表
     * @return 输入设备ID列表
     */
    QStringList availableInputDevices() const;

    /**
     * @brief 获取可用输出设备列表
     * @return 输出设备ID列表
     */
    QStringList availableOutputDevices() const;

    /**
     * @brief 获取设备显示名称
     * @param deviceId 设备ID
     * @return 设备显示名称
     */
    QString deviceDisplayName(const QString &deviceId) const;

    /**
     * @brief 选择输入设备
     * @param deviceId 设备ID
     * @return 选择成功返回true
     */
    bool selectInputDevice(const QString &deviceId);

    /**
     * @brief 选择输出设备
     * @param deviceId 设备ID
     * @return 选择成功返回true
     */
    bool selectOutputDevice(const QString &deviceId);

    /**
     * @brief 获取当前输入设备ID
     * @return 当前输入设备ID
     */
    QString currentInputDevice() const;

    /**
     * @brief 获取当前输出设备ID
     * @return 当前输出设备ID
     */
    QString currentOutputDevice() const;

    /**
     * @brief 设置主音量
     * @param volume 音量值 (0.0-1.0)
     */
    void setMasterVolume(qreal volume);

    /**
     * @brief 获取主音量
     * @return 当前主音量 (0.0-1.0)
     */
    qreal masterVolume() const;

    /**
     * @brief 设置麦克风增益
     * @param gain 增益值 (0.0-1.0)
     */
    void setMicrophoneGain(qreal gain);

    /**
     * @brief 获取麦克风增益
     * @return 当前麦克风增益 (0.0-1.0)
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

    /**
     * @brief 设置音频质量预设
     * @param preset 质量预设
     */
    void setQualityPreset(QualityPreset preset);

    /**
     * @brief 获取当前音频质量预设
     * @return 当前质量预设
     */
    QualityPreset qualityPreset() const;

    /**
     * @brief 开始音频处理
     * @return 开始成功返回true
     */
    bool startAudio();

    /**
     * @brief 停止音频处理
     */
    void stopAudio();

    /**
     * @brief 检查音频是否正在运行
     * @return 音频运行中返回true
     */
    bool isAudioActive() const;

    /**
     * @brief 获取音频配置
     * @return 音频配置对象
     */
    AudioConfig* audioConfig() const;

signals:
    /**
     * @brief 状态改变信号
     * @param status 新状态
     */
    void statusChanged(ManagerStatus status);

    /**
     * @brief 设备列表更新信号
     */
    void devicesUpdated();

    /**
     * @brief 音频开始信号
     */
    void audioStarted();

    /**
     * @brief 音频停止信号
     */
    void audioStopped();

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
     * @brief 设备改变信号
     * @param deviceId 新设备ID
     * @param isInput 是否为输入设备
     */
    void deviceChanged(const QString &deviceId, bool isInput);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

private slots:
    /**
     * @brief 处理设备状态改变
     */
    void onDeviceStatusChanged();

    /**
     * @brief 处理设备错误
     * @param error 错误信息
     */
    void onDeviceError(const QString &error);

private:
    class AudioManagerPrivate;
    std::unique_ptr<AudioManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(AudioManager)

    /**
     * @brief 刷新设备列表
     */
    void refreshDevices();

    /**
     * @brief 应用质量预设
     * @param preset 质量预设
     */
    void applyQualityPreset(QualityPreset preset);
};

#endif // AUDIOMANAGER_H