#ifndef QTAUDIODEVICE_H
#define QTAUDIODEVICE_H

#include "IAudioDevice.h"
#include "AudioFactory.h"
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioOutput>

/**
 * @brief Qt音频设备实现类
 * 
 * QtAudioDevice是基于Qt音频API的IAudioDevice接口实现，
 * 提供跨平台的音频设备访问功能。
 */
class QtAudioDevice : public IAudioDevice
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param deviceId 设备ID
     * @param deviceType 设备类型
     * @param parent 父对象
     */
    explicit QtAudioDevice(const QString &deviceId, 
                          AudioFactory::DeviceType deviceType, 
                          QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~QtAudioDevice() override;

    // IAudioDevice interface implementation
    bool initialize() override;
    bool start() override;
    void stop() override;
    bool isActive() const override;
    QString deviceId() const override;
    QString deviceName() const override;
    DeviceType deviceType() const override;
    Status status() const override;
    void setVolume(qreal volume) override;
    qreal volume() const override;
    void setMuted(bool muted) override;
    bool isMuted() const override;
    void setQualityPreset(QualityPreset preset) override;
    QualityPreset qualityPreset() const override;
    int sampleRate() const override;
    void setSampleRate(int sampleRate) override;
    int channels() const override;
    void setChannels(int channels) override;
    int bufferSize() const override;
    void setBufferSize(int bufferSize) override;
    qreal latency() const override;
    bool supportsFormat(int sampleRate, int channels) const override;

private slots:
    /**
     * @brief 处理音频输入状态变化
     * @param state 新状态
     */
    void onInputStateChanged(QAudio::State state);

    /**
     * @brief 处理音频输出状态变化
     * @param state 新状态
     */
    void onOutputStateChanged(QAudio::State state);

private:
    /**
     * @brief 设置设备状态
     * @param status 新状态
     */
    void setStatus(Status status);

    /**
     * @brief 更新音频格式
     */
    void updateAudioFormat();

    /**
     * @brief 查找设备信息
     * @return 是否找到设备
     */
    bool findDeviceInfo();

private:
    QString m_deviceId;                 ///< 设备ID
    DeviceType m_deviceType;            ///< 设备类型
    Status m_status;                    ///< 设备状态
    qreal m_volume;                     ///< 音量
    bool m_muted;                       ///< 静音状态
    QualityPreset m_qualityPreset;      ///< 质量预设
    int m_sampleRate;                   ///< 采样率
    int m_channels;                     ///< 声道数
    int m_bufferSize;                   ///< 缓冲区大小
    
    QAudioDeviceInfo m_deviceInfo;     ///< Qt设备信息
    QAudioFormat m_format;              ///< 音频格式
    QAudioInput *m_audioInput;          ///< 音频输入
    QAudioOutput *m_audioOutput;        ///< 音频输出
    
    QIODevice *m_inputIODevice;         ///< 输入IO设备
    QIODevice *m_outputIODevice;        ///< 输出IO设备
};

#endif // QTAUDIODEVICE_H