#ifndef ISCREENSHAREMANAGER_H
#define ISCREENSHAREMANAGER_H

#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include "IScreenCapture.h"

/**
 * @brief 屏幕共享管理器接口
 * 
 * 定义屏幕共享管理的标准接口，负责协调捕获、编码和传输
 */
class IScreenShareManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 管理器状态枚举
     */
    enum ManagerStatus {
        Uninitialized,  ///< 未初始化
        Ready,          ///< 就绪状态
        Sharing,        ///< 共享中
        Paused,         ///< 暂停状态
        Error           ///< 错误状态
    };
    Q_ENUM(ManagerStatus)

    /**
     * @brief 共享模式枚举
     */
    enum ShareMode {
        LocalPreview,   ///< 本地预览模式
        NetworkShare,   ///< 网络共享模式
        Recording,      ///< 录制模式
        Broadcast       ///< 广播模式
    };
    Q_ENUM(ShareMode)

    /**
     * @brief 编码格式枚举
     */
    enum EncodingFormat {
        H264,           ///< H.264编码
        VP8,            ///< VP8编码
        VP9,            ///< VP9编码
        AV1             ///< AV1编码
    };
    Q_ENUM(EncodingFormat)

    explicit IScreenShareManager(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IScreenShareManager() = default;

    // 基础管理接口
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual ManagerStatus status() const = 0;
    virtual bool isReady() const = 0;

    // 屏幕共享控制接口
    virtual bool startScreenShare(const QVariantMap& config = QVariantMap()) = 0;
    virtual void stopScreenShare() = 0;
    virtual void pauseScreenShare() = 0;
    virtual void resumeScreenShare() = 0;
    virtual bool isSharing() const = 0;

    // 配置管理接口
    virtual void setShareMode(ShareMode mode) = 0;
    virtual ShareMode shareMode() const = 0;
    virtual void setEncodingFormat(EncodingFormat format) = 0;
    virtual EncodingFormat encodingFormat() const = 0;
    virtual void setConfiguration(const QVariantMap& config) = 0;
    virtual QVariantMap configuration() const = 0;

    // 捕获源管理接口
    virtual QStringList availableScreens() const = 0;
    virtual QStringList availableWindows() const = 0;
    virtual bool selectScreen(const QString& screenId) = 0;
    virtual bool selectWindow(const QString& windowId) = 0;
    virtual QString currentSource() const = 0;

    // 质量控制接口
    virtual void setQuality(IScreenCapture::CaptureQuality quality) = 0;
    virtual IScreenCapture::CaptureQuality quality() const = 0;
    virtual void setFrameRate(int fps) = 0;
    virtual int frameRate() const = 0;
    virtual void setBitrate(int kbps) = 0;
    virtual int bitrate() const = 0;

    // 统计信息接口
    virtual QVariantMap getStatistics() const = 0;
    virtual double getCurrentFPS() const = 0;
    virtual int getCurrentBitrate() const = 0;
    virtual qint64 getTotalFrames() const = 0;

signals:
    /**
     * @brief 管理器状态改变信号
     * @param status 新的管理器状态
     */
    void statusChanged(ManagerStatus status);

    /**
     * @brief 屏幕共享开始信号
     */
    void shareStarted();

    /**
     * @brief 屏幕共享停止信号
     */
    void shareStopped();

    /**
     * @brief 屏幕共享暂停信号
     */
    void sharePaused();

    /**
     * @brief 屏幕共享恢复信号
     */
    void shareResumed();

    /**
     * @brief 共享错误信号
     * @param error 错误信息
     */
    void shareError(const QString& error);

    /**
     * @brief 质量改变信号
     * @param quality 新的质量设置
     */
    void qualityChanged(IScreenCapture::CaptureQuality quality);

    /**
     * @brief 统计信息更新信号
     * @param statistics 统计信息
     */
    void statisticsUpdated(const QVariantMap& statistics);

    /**
     * @brief 可用源更新信号
     */
    void availableSourcesUpdated();
};

#endif // ISCREENSHAREMANAGER_H