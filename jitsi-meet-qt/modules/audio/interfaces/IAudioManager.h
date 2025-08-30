#ifndef IAUDIOMANAGER_H
#define IAUDIOMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

/**
 * @brief 音频管理器接口
 * 
 * IAudioManager定义了音频管理器的标准接口，提供高级音频管理功能，
 * 包括设备管理、音频流控制和配置管理等。
 */
class IAudioManager : public QObject
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
     * @brief 音频流状态枚举
     */
    enum StreamStatus {
        Stopped,        ///< 已停止
        Starting,       ///< 启动中
        Running,        ///< 运行中
        Stopping,       ///< 停止中
        Paused          ///< 已暂停
    };
    Q_ENUM(StreamStatus)

    /**
     * @brief 虚析构函数
     */
    virtual ~IAudioManager() = default;

    /**
     * @brief 初始化管理器
     * @return 初始化成功返回true
     */
    virtual bool initialize() = 0;

    /**
     * @brief 获取管理器状态
     * @return 当前状态
     */
    virtual ManagerStatus status() const = 0;

    /**
     * @brief 获取音频流状态
     * @return 当前音频流状态
     */
    virtual StreamStatus streamStatus() const = 0;

    /**
     * @brief 获取可用输入设备列表
     * @return 输入设备ID列表
     */
    virtual QStringList availableInputDevices() const = 0;

    /**
     * @brief 获取可用输出设备列表
     * @return 输出设备ID列表
     */
    virtual QStringList availableOutputDevices() const = 0;

    /**
     * @brief 选择输入设备
     * @param deviceId 设备ID
     * @return 选择成功返回true
     */
    virtual bool selectInputDevice(const QString &deviceId) = 0;

    /**
     * @brief 选择输出设备
     * @param deviceId 设备ID
     * @return 选择成功返回true
     */
    virtual bool selectOutputDevice(const QString &deviceId) = 0;

    /**
     * @brief 获取当前输入设备ID
     * @return 当前输入设备ID
     */
    virtual QString currentInputDevice() const = 0;

    /**
     * @brief 获取当前输出设备ID
     * @return 当前输出设备ID
     */
    virtual QString currentOutputDevice() const = 0;

    /**
     * @brief 设置主音量
     * @param volume 音量值 (0.0-1.0)
     */
    virtual void setMasterVolume(qreal volume) = 0;

    /**
     * @brief 获取主音量
     * @return 当前主音量 (0.0-1.0)
     */
    virtual qreal masterVolume() const = 0;

    /**
     * @brief 设置麦克风增益
     * @param gain 增益值 (0.0-1.0)
     */
    virtual void setMicrophoneGain(qreal gain) = 0;

    /**
     * @brief 获取麦克风增益
     * @return 当前麦克风增益 (0.0-1.0)
     */
    virtual qreal microphoneGain() const = 0;

    /**
     * @brief 设置全局静音状态
     * @param muted 是否静音
     */
    virtual void setMuted(bool muted) = 0;

    /**
     * @brief 获取全局静音状态
     * @return 是否静音
     */
    virtual bool isMuted() const = 0;

    /**
     * @brief 开始音频流
     * @return 开始成功返回true
     */
    virtual bool startAudioStream() = 0;

    /**
     * @brief 停止音频流
     */
    virtual void stopAudioStream() = 0;

    /**
     * @brief 暂停音频流
     */
    virtual void pauseAudioStream() = 0;

    /**
     * @brief 恢复音频流
     */
    virtual void resumeAudioStream() = 0;

    /**
     * @brief 获取音频统计信息
     * @return 统计信息映射
     */
    virtual QVariantMap audioStatistics() const = 0;

    /**
     * @brief 设置音频配置
     * @param config 配置映射
     */
    virtual void setAudioConfiguration(const QVariantMap &config) = 0;

    /**
     * @brief 获取音频配置
     * @return 配置映射
     */
    virtual QVariantMap audioConfiguration() const = 0;

    /**
     * @brief 应用音频效果
     * @param effectName 效果名称
     * @param parameters 效果参数
     * @return 应用成功返回true
     */
    virtual bool applyAudioEffect(const QString &effectName, 
                                 const QVariantMap &parameters) = 0;

    /**
     * @brief 移除音频效果
     * @param effectName 效果名称
     */
    virtual void removeAudioEffect(const QString &effectName) = 0;

    /**
     * @brief 获取支持的音频效果列表
     * @return 效果名称列表
     */
    virtual QStringList supportedAudioEffects() const = 0;

    /**
     * @brief 测试音频设备
     * @param deviceId 设备ID
     * @param isInput 是否为输入设备
     * @return 测试成功返回true
     */
    virtual bool testAudioDevice(const QString &deviceId, bool isInput) = 0;

signals:
    /**
     * @brief 状态改变信号
     * @param status 新状态
     */
    void statusChanged(ManagerStatus status);

    /**
     * @brief 音频流状态改变信号
     * @param status 新的流状态
     */
    void streamStatusChanged(StreamStatus status);

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
     * @brief 音频暂停信号
     */
    void audioPaused();

    /**
     * @brief 音频恢复信号
     */
    void audioResumed();

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
     * @brief 音频统计更新信号
     * @param statistics 统计信息
     */
    void statisticsUpdated(const QVariantMap &statistics);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);
};

Q_DECLARE_INTERFACE(IAudioManager, "org.jitsi.qt.IAudioManager/1.0")

#endif // IAUDIOMANAGER_H