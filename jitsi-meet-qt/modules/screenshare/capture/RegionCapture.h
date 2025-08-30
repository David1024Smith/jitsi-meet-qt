#ifndef REGIONCAPTURE_H
#define REGIONCAPTURE_H

#include "../interfaces/IScreenCapture.h"
#include <QRect>
#include <QTimer>
#include <QMutex>

/**
 * @brief 区域捕获实现类
 * 
 * 实现IScreenCapture接口，提供自定义区域捕获功能
 */
class RegionCapture : public IScreenCapture
{
    Q_OBJECT
    Q_PROPERTY(QRect customRegion READ customRegion WRITE setCustomRegion NOTIFY customRegionChanged)
    Q_PROPERTY(bool regionLocked READ isRegionLocked WRITE setRegionLocked NOTIFY regionLockedChanged)

public:
    /**
     * @brief 区域选择模式枚举
     */
    enum SelectionMode {
        ManualSelection,    ///< 手动选择
        InteractiveSelection, ///< 交互式选择
        PresetRegion,       ///< 预设区域
        FollowMouse         ///< 跟随鼠标
    };
    Q_ENUM(SelectionMode)

    /**
     * @brief 边界处理模式枚举
     */
    enum BoundaryMode {
        Clip,               ///< 裁剪到边界
        Extend,             ///< 扩展到边界
        Wrap,               ///< 环绕处理
        Error               ///< 错误处理
    };
    Q_ENUM(BoundaryMode)

    explicit RegionCapture(QObject *parent = nullptr);
    virtual ~RegionCapture();

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

    // 区域特定接口
    QRect customRegion() const;
    void setCustomRegion(const QRect& region);
    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode mode);
    BoundaryMode boundaryMode() const;
    void setBoundaryMode(BoundaryMode mode);

    // 区域管理接口
    bool isRegionLocked() const;
    void setRegionLocked(bool locked);
    bool isRegionValid() const;
    QRect adjustedRegion() const;
    QRect normalizedRegion() const;

    // 交互式选择接口
    void startInteractiveSelection();
    void cancelInteractiveSelection();
    bool isInteractiveSelectionActive() const;

    // 预设区域接口
    void setPresetRegions(const QList<QRect>& regions);
    QList<QRect> presetRegions() const;
    bool selectPresetRegion(int index);
    int currentPresetIndex() const;

    // 跟随鼠标接口
    void setMouseFollowSize(const QSize& size);
    QSize mouseFollowSize() const;
    void setMouseFollowOffset(const QPoint& offset);
    QPoint mouseFollowOffset() const;

public slots:
    /**
     * @brief 重置区域到全屏
     */
    void resetToFullScreen();

    /**
     * @brief 居中区域
     */
    void centerRegion();

    /**
     * @brief 调整区域到屏幕边界
     */
    void adjustToScreenBounds();

    /**
     * @brief 开始区域选择
     */
    void startRegionSelection();

signals:
    /**
     * @brief 自定义区域改变信号
     * @param region 新的区域
     */
    void customRegionChanged(const QRect& region);

    /**
     * @brief 区域锁定状态改变信号
     * @param locked 是否锁定
     */
    void regionLockedChanged(bool locked);

    /**
     * @brief 交互式选择开始信号
     */
    void interactiveSelectionStarted();

    /**
     * @brief 交互式选择完成信号
     * @param region 选择的区域
     */
    void interactiveSelectionFinished(const QRect& region);

    /**
     * @brief 交互式选择取消信号
     */
    void interactiveSelectionCancelled();

    /**
     * @brief 区域验证失败信号
     * @param reason 失败原因
     */
    void regionValidationFailed(const QString& reason);

private slots:
    void onCaptureTimer();
    void onMousePositionTimer();
    void onInteractiveSelectionUpdate();

private:
    void initializeCapture();
    void cleanupCapture();
    void updateCaptureTimer();
    void updateStatus(CaptureStatus newStatus);
    QPixmap captureRegionInternal();
    QRect validateRegion(const QRect& region) const;
    QRect applyBoundaryMode(const QRect& region) const;
    void updateMouseFollowRegion();
    void emitError(const QString& error);

    class Private;
    Private* d;
};

#endif // REGIONCAPTURE_H