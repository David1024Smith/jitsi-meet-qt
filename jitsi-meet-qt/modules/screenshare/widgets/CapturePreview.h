#ifndef CAPTUREPREVIEW_H
#define CAPTUREPREVIEW_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QCheckBox>

/**
 * @brief 捕获预览组件
 * 
 * 提供屏幕捕获的实时预览功能
 */
class CapturePreview : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool previewEnabled READ isPreviewEnabled WRITE setPreviewEnabled NOTIFY previewEnabledChanged)
    Q_PROPERTY(QSize previewSize READ previewSize WRITE setPreviewSize NOTIFY previewSizeChanged)
    Q_PROPERTY(int refreshRate READ refreshRate WRITE setRefreshRate NOTIFY refreshRateChanged)

public:
    /**
     * @brief 预览模式枚举
     */
    enum PreviewMode {
        StaticPreview,      ///< 静态预览
        LivePreview,        ///< 实时预览
        OnDemandPreview     ///< 按需预览
    };
    Q_ENUM(PreviewMode)

    /**
     * @brief 缩放模式枚举
     */
    enum ScaleMode {
        FitToWidget,        ///< 适应组件
        KeepAspectRatio,    ///< 保持宽高比
        OriginalSize,       ///< 原始尺寸
        CustomScale         ///< 自定义缩放
    };
    Q_ENUM(ScaleMode)

    explicit CapturePreview(QWidget *parent = nullptr);
    virtual ~CapturePreview();

    // 预览控制接口
    bool isPreviewEnabled() const;
    void setPreviewEnabled(bool enabled);
    PreviewMode previewMode() const;
    void setPreviewMode(PreviewMode mode);

    // 预览配置接口
    QSize previewSize() const;
    void setPreviewSize(const QSize& size);
    ScaleMode scaleMode() const;
    void setScaleMode(ScaleMode mode);
    int refreshRate() const;
    void setRefreshRate(int fps);

    // 显示配置接口
    bool isShowControls() const;
    void setShowControls(bool show);
    bool isShowStatistics() const;
    void setShowStatistics(bool show);
    bool isShowCrosshair() const;
    void setShowCrosshair(bool show);

    // 预览内容接口
    QPixmap currentFrame() const;
    void setCurrentFrame(const QPixmap& frame);
    QSize originalSize() const;
    double scaleFactor() const;

    // 统计信息接口
    int frameCount() const;
    double currentFPS() const;
    qint64 lastUpdateTime() const;

public slots:
    /**
     * @brief 开始预览
     */
    void startPreview();

    /**
     * @brief 停止预览
     */
    void stopPreview();

    /**
     * @brief 暂停预览
     */
    void pausePreview();

    /**
     * @brief 恢复预览
     */
    void resumePreview();

    /**
     * @brief 刷新预览
     */
    void refreshPreview();

    /**
     * @brief 捕获当前帧
     */
    void captureCurrentFrame();

    /**
     * @brief 保存当前帧
     */
    void saveCurrentFrame();

    /**
     * @brief 重置预览
     */
    void resetPreview();

signals:
    /**
     * @brief 预览启用状态改变信号
     * @param enabled 是否启用
     */
    void previewEnabledChanged(bool enabled);

    /**
     * @brief 预览尺寸改变信号
     * @param size 新的预览尺寸
     */
    void previewSizeChanged(const QSize& size);

    /**
     * @brief 刷新率改变信号
     * @param rate 新的刷新率
     */
    void refreshRateChanged(int rate);

    /**
     * @brief 帧更新信号
     * @param frame 新的帧数据
     */
    void frameUpdated(const QPixmap& frame);

    /**
     * @brief 预览开始信号
     */
    void previewStarted();

    /**
     * @brief 预览停止信号
     */
    void previewStopped();

    /**
     * @brief 预览点击信号
     * @param position 点击位置
     */
    void previewClicked(const QPoint& position);

    /**
     * @brief 预览双击信号
     * @param position 双击位置
     */
    void previewDoubleClicked(const QPoint& position);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onRefreshTimer();
    void onPlayPauseClicked();
    void onRefreshClicked();
    void onSaveClicked();
    void onScaleSliderChanged(int value);
    void onFitToWidgetClicked();
    void onOriginalSizeClicked();

private:
    void setupUI();
    void setupControls();
    void connectSignals();
    void updatePreview();
    void updateControls();
    void updateStatistics();
    void calculateScaledSize();
    void drawFrame(QPainter& painter);
    void drawCrosshair(QPainter& painter);
    void drawStatistics(QPainter& painter);
    QPoint mapToOriginal(const QPoint& widgetPos) const;
    QPoint mapFromOriginal(const QPoint& originalPos) const;

    class Private;
    Private* d;
};

#endif // CAPTUREPREVIEW_H