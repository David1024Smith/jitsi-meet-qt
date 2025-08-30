#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

#include "../interfaces/IScreenCapture.h"
#include <QScreen>
#include <QTimer>
#include <QMutex>

/**
 * @brief 屏幕捕获实现类
 * 
 * 实现IScreenCapture接口，提供全屏捕获功能
 */
class ScreenCapture : public IScreenCapture
{
    Q_OBJECT

public:
    explicit ScreenCapture(QObject *parent = nullptr);
    virtual ~ScreenCapture();

    // IScreenCapture接口实现
    bool initialize() override;
    bool startCapture() override;
    void stopCapture() override;
    void pauseCapture() override;
    void resumeCapture() override;

    // 状态查询接口
    CaptureStatus status() const override;
    bool isCapturing() const override;
    bool isInitialized() const override;

    // 捕获配置接口
    void setCaptureMode(CaptureMode mode) override;
    CaptureMode captureMode() const override;
    void setCaptureQuality(CaptureQuality quality) override;
    CaptureQuality captureQuality() const override;
    void setFrameRate(int fps) override;
    int frameRate() const override;

    // 捕获区域接口
    void setCaptureRegion(const QRect& region) override;
    QRect captureRegion() const override;
    void setTargetScreen(QScreen* screen) override;
    QScreen* targetScreen() const override;

    // 捕获数据接口
    QPixmap captureFrame() override;
    QByteArray captureFrameData() override;
    QSize captureSize() const override;

    // 扩展功能接口
    void setCaptureCursor(bool enabled);
    bool isCaptureCursorEnabled() const;
    void setCaptureDelay(int ms);
    int captureDelay() const;
    void setCompressionQuality(int quality);
    int compressionQuality() const;
    
    // 性能优化接口
    void enableAdaptiveQuality(bool enabled);
    void optimizeCaptureQuality();

public slots:
    /**
     * @brief 刷新屏幕信息
     */
    void refreshScreenInfo();

    /**
     * @brief 自动选择最佳屏幕
     */
    void autoSelectScreen();

private slots:
    void onCaptureTimer();
    void onScreenChanged();

private:
    void initializeCapture();
    void cleanupCapture();
    void updateCaptureTimer();
    void updateStatus(CaptureStatus newStatus);
    QPixmap captureScreenInternal();
    QPixmap applyCaptureQuality(const QPixmap& source);
    void emitError(const QString& error);
    
    // 性能监控方法
    double getCurrentCPUUsage() const;
    qint64 getCurrentMemoryUsage() const;

    class Private;
    Private* d;
};

#endif // SCREENCAPTURE_H