#ifndef ISCREENCAPTURE_H
#define ISCREENCAPTURE_H

#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QScreen>

/**
 * @brief 屏幕捕获接口
 * 
 * 定义屏幕捕获的标准接口，支持全屏、窗口和区域捕获
 */
class IScreenCapture : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 捕获模式枚举
     */
    enum CaptureMode {
        FullScreen,     ///< 全屏捕获
        Window,         ///< 窗口捕获
        Region,         ///< 区域捕获
        Custom          ///< 自定义捕获
    };
    Q_ENUM(CaptureMode)

    /**
     * @brief 捕获状态枚举
     */
    enum CaptureStatus {
        Inactive,       ///< 未激活
        Initializing,   ///< 初始化中
        Active,         ///< 激活状态
        Paused,         ///< 暂停状态
        Error           ///< 错误状态
    };
    Q_ENUM(CaptureStatus)

    /**
     * @brief 捕获质量枚举
     */
    enum CaptureQuality {
        LowQuality,     ///< 低质量 (快速)
        MediumQuality,  ///< 中等质量
        HighQuality,    ///< 高质量
        UltraQuality    ///< 超高质量 (慢速)
    };
    Q_ENUM(CaptureQuality)

    explicit IScreenCapture(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IScreenCapture() = default;

    // 基础控制接口
    virtual bool initialize() = 0;
    virtual bool startCapture() = 0;
    virtual void stopCapture() = 0;
    virtual void pauseCapture() = 0;
    virtual void resumeCapture() = 0;

    // 状态查询接口
    virtual CaptureStatus status() const = 0;
    virtual bool isCapturing() const = 0;
    virtual bool isInitialized() const = 0;

    // 捕获配置接口
    virtual void setCaptureMode(CaptureMode mode) = 0;
    virtual CaptureMode captureMode() const = 0;
    virtual void setCaptureQuality(CaptureQuality quality) = 0;
    virtual CaptureQuality captureQuality() const = 0;
    virtual void setFrameRate(int fps) = 0;
    virtual int frameRate() const = 0;

    // 捕获区域接口
    virtual void setCaptureRegion(const QRect& region) = 0;
    virtual QRect captureRegion() const = 0;
    virtual void setTargetScreen(QScreen* screen) = 0;
    virtual QScreen* targetScreen() const = 0;

    // 捕获数据接口
    virtual QPixmap captureFrame() = 0;
    virtual QByteArray captureFrameData() = 0;
    virtual QSize captureSize() const = 0;

signals:
    /**
     * @brief 捕获状态改变信号
     * @param status 新的捕获状态
     */
    void statusChanged(CaptureStatus status);

    /**
     * @brief 新帧捕获信号
     * @param frame 捕获的帧数据
     */
    void frameCaptured(const QPixmap& frame);

    /**
     * @brief 捕获错误信号
     * @param error 错误信息
     */
    void captureError(const QString& error);

    /**
     * @brief 捕获开始信号
     */
    void captureStarted();

    /**
     * @brief 捕获停止信号
     */
    void captureStopped();

    /**
     * @brief 捕获暂停信号
     */
    void capturePaused();

    /**
     * @brief 捕获恢复信号
     */
    void captureResumed();
};

#endif // ISCREENCAPTURE_H