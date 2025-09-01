#ifndef SCREENSHAREMANAGER_H
#define SCREENSHAREMANAGER_H

#include "../interfaces/IScreenShareManager.h"
#include <QTimer>
#include <QMutex>
#include <QVideoWidget>

class IScreenCapture;
class CaptureEngine;
class ScreenShareConfig;

/**
 * @brief 屏幕共享管理器实现类
 * 
 * 实现IScreenShareManager接口，提供完整的屏幕共享管理功能
 */
class ScreenShareManager : public IScreenShareManager
{
    Q_OBJECT

public:
    explicit ScreenShareManager(QObject *parent = nullptr);
    virtual ~ScreenShareManager();

    // IScreenShareManager接口实现
    bool initialize() override;
    void shutdown() override;
    ManagerStatus status() const override;
    bool isReady() const override;

    // 屏幕共享控制接口
    bool startScreenShare(const QVariantMap& config = QVariantMap()) override;
    void stopScreenShare() override;
    void pauseScreenShare() override;
    void resumeScreenShare() override;
    bool isSharing() const override;

    // 配置管理接口
    void setShareMode(ShareMode mode) override;
    ShareMode shareMode() const override;
    void setEncodingFormat(EncodingFormat format) override;
    EncodingFormat encodingFormat() const override;
    void setConfiguration(const QVariantMap& config) override;
    QVariantMap configuration() const override;

    // 捕获源管理接口
    QStringList availableScreens() const override;
    QStringList availableWindows() const override;
    bool selectScreen(const QString& screenId) override;
    bool selectWindow(const QString& windowId) override;
    QString currentSource() const override;

    // 质量控制接口
    void setQuality(IScreenCapture::CaptureQuality quality) override;
    IScreenCapture::CaptureQuality quality() const override;
    void setFrameRate(int fps) override;
    int frameRate() const override;
    void setBitrate(int kbps) override;
    int bitrate() const override;

    // 统计信息接口
    QVariantMap getStatistics() const override;
    double getCurrentFPS() const override;
    int getCurrentBitrate() const override;
    qint64 getTotalFrames() const override;

signals:
    /**
     * @brief 屏幕共享开始信号
     */
    void shareStarted();

    /**
     * @brief 屏幕共享停止信号
     */
    void shareStopped();

    /**
     * @brief 远程屏幕共享接收信号
     * @param participantId 参与者ID
     * @param widget 视频控件
     */
    void remoteScreenShareReceived(const QString& participantId, QVideoWidget* widget);

    /**
     * @brief 远程屏幕共享移除信号
     * @param participantId 参与者ID
     */
    void remoteScreenShareRemoved(const QString& participantId);

    // 扩展功能接口
    void setAutoQualityAdjustment(bool enabled);
    bool isAutoQualityAdjustmentEnabled() const;
    void setMaxFrameRate(int maxFps);
    int maxFrameRate() const;
    void setMaxBitrate(int maxKbps);
    int maxBitrate() const;

public slots:
    /**
     * @brief 刷新可用源列表
     */
    void refreshAvailableSources();

    /**
     * @brief 优化性能设置
     */
    void optimizePerformance();

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

    /**
     * @brief 显示屏幕选择对话框
     * @return 是否选择了屏幕
     */
    bool showScreenSelectionDialog();

    /**
     * @brief 获取本地屏幕共享控件
     * @return 屏幕共享控件
     */
    QVideoWidget* localScreenShareWidget() const;

private slots:
    void onCaptureStatusChanged(IScreenCapture::CaptureStatus status);
    void onFrameCaptured(const QPixmap& frame);
    void onCaptureError(const QString& error);
    void onStatisticsTimer();
    void onQualityAdjustmentTimer();

private:
    void initializeCapture();
    void cleanupCapture();
    void updateStatistics();
    void adjustQualityBasedOnPerformance();
    bool validateShareConfiguration(const QVariantMap& config) const;
    void updateStatus(ManagerStatus newStatus);
    void emitError(const QString& error);

    class Private;
    Private* d;
};

#endif // SCREENSHAREMANAGER_H