#ifndef ICAMERADEVICE_H
#define ICAMERADEVICE_H

#include <QObject>
#include <QString>
#include <QSize>
#include <QVideoFrame>

/**
 * @brief 摄像头设备接口
 * 
 * 定义了摄像头设备的基本操作接口，所有摄像头设备实现都应该继承此接口
 */
class ICameraDevice : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 摄像头状态枚举
     */
    enum Status {
        Inactive,       ///< 未激活
        Loading,        ///< 加载中
        Loaded,         ///< 已加载
        Starting,       ///< 启动中
        Active,         ///< 活跃状态
        Stopping,       ///< 停止中
        Stopped,        ///< 已停止
        Error           ///< 错误状态
    };
    Q_ENUM(Status)

    /**
     * @brief 摄像头质量预设
     */
    enum QualityPreset {
        LowQuality,     ///< 低质量 (320x240)
        StandardQuality, ///< 标准质量 (640x480)
        HighQuality,    ///< 高质量 (1280x720)
        UltraQuality    ///< 超高质量 (1920x1080)
    };
    Q_ENUM(QualityPreset)

    explicit ICameraDevice(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ICameraDevice() = default;

    // 基本控制接口
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isActive() const = 0;
    virtual Status status() const = 0;

    // 设备信息接口
    virtual QString deviceId() const = 0;
    virtual QString deviceName() const = 0;
    virtual QString description() const = 0;
    virtual bool isAvailable() const = 0;

    // 配置接口
    virtual void setResolution(const QSize& resolution) = 0;
    virtual QSize resolution() const = 0;
    virtual void setFrameRate(int frameRate) = 0;
    virtual int frameRate() const = 0;
    virtual void setQualityPreset(QualityPreset preset) = 0;
    virtual QualityPreset qualityPreset() const = 0;

    // 支持的格式查询
    virtual QList<QSize> supportedResolutions() const = 0;
    virtual QList<int> supportedFrameRates() const = 0;

signals:
    /**
     * @brief 状态改变信号
     * @param status 新状态
     */
    void statusChanged(Status status);

    /**
     * @brief 错误发生信号
     * @param error 错误描述
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 新帧可用信号
     * @param frame 视频帧
     */
    void frameAvailable(const QVideoFrame& frame);

    /**
     * @brief 设备连接状态改变
     * @param connected 是否连接
     */
    void deviceConnected(bool connected);
};

#endif // ICAMERADEVICE_H