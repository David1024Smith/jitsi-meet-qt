#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QRect>
#include <QMutex>
#include <QVariantMap>

/**
 * @brief 帧处理器类
 * 
 * 负责对捕获的帧进行各种处理操作，如缩放、裁剪、滤镜等
 */
class FrameProcessor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(QSize outputSize READ outputSize WRITE setOutputSize NOTIFY outputSizeChanged)
    Q_PROPERTY(int quality READ quality WRITE setQuality NOTIFY qualityChanged)

public:
    /**
     * @brief 处理器状态枚举
     */
    enum ProcessorStatus {
        Inactive,       ///< 未激活
        Ready,          ///< 就绪状态
        Processing,     ///< 处理中
        Error           ///< 错误状态
    };
    Q_ENUM(ProcessorStatus)

    /**
     * @brief 缩放模式枚举
     */
    enum ScalingMode {
        KeepAspectRatio,        ///< 保持宽高比
        IgnoreAspectRatio,      ///< 忽略宽高比
        KeepAspectRatioByExpanding, ///< 保持宽高比(扩展)
        FitToSize               ///< 适应尺寸
    };
    Q_ENUM(ScalingMode)

    /**
     * @brief 滤镜类型枚举
     */
    enum FilterType {
        NoFilter,           ///< 无滤镜
        Blur,               ///< 模糊
        Sharpen,            ///< 锐化
        Brightness,         ///< 亮度调整
        Contrast,           ///< 对比度调整
        Saturation,         ///< 饱和度调整
        Grayscale,          ///< 灰度
        Sepia,              ///< 棕褐色
        Invert,             ///< 反色
        EdgeDetection       ///< 边缘检测
    };
    Q_ENUM(FilterType)

    /**
     * @brief 旋转角度枚举
     */
    enum RotationAngle {
        NoRotation = 0,     ///< 不旋转
        Rotate90 = 90,      ///< 顺时针90度
        Rotate180 = 180,    ///< 180度
        Rotate270 = 270     ///< 顺时针270度
    };
    Q_ENUM(RotationAngle)

    explicit FrameProcessor(QObject *parent = nullptr);
    virtual ~FrameProcessor();

    // 处理器控制接口
    bool initialize();
    void shutdown();
    ProcessorStatus status() const;
    bool isActive() const;

    // 基础处理接口
    QPixmap processFrame(const QPixmap& frame);
    QByteArray processFrameData(const QByteArray& data, const QSize& size);
    bool processFrameAsync(const QPixmap& frame);
    bool isInitialized() const;

    // 尺寸和缩放接口
    QSize outputSize() const;
    void setOutputSize(const QSize& size);
    QSize targetSize() const;
    void setTargetSize(const QSize& size);
    Qt::TransformationMode scalingMode() const;
    void setScalingMode(ScalingMode mode);
    bool maintainAspectRatio() const;
    void setMaintainAspectRatio(bool maintain);
    bool isScalingEnabled() const;
    void setScalingEnabled(bool enabled);

    // 裁剪接口
    QRect cropRegion() const;
    void setCropRegion(const QRect& region);
    bool isCropEnabled() const;
    void setCropEnabled(bool enabled);

    // 旋转和翻转接口
    RotationAngle rotation() const;
    void setRotation(RotationAngle angle);
    bool isRotationEnabled() const;
    void setRotationEnabled(bool enabled);
    int rotationAngle() const;
    void setRotationAngle(int angle);
    bool isHorizontalFlip() const;
    void setHorizontalFlip(bool flip);
    bool isVerticalFlip() const;
    void setVerticalFlip(bool flip);

    // 质量和压缩接口
    int quality() const;
    void setQuality(int quality);
    int compressionLevel() const;
    void setCompressionLevel(int level);

    // 滤镜接口
    QList<FilterType> activeFilters() const;
    void addFilter(FilterType filter, const QVariantMap& parameters = QVariantMap{});
    void removeFilter(FilterType filter);
    void clearFilters();
    void setFilterParameter(FilterType filter, const QString& parameter, const QVariant& value);
    QVariant getFilterParameter(FilterType filter, const QString& parameter) const;
    bool isFilteringEnabled() const;
    void setFilteringEnabled(bool enabled);
    int brightness() const;
    void setBrightness(int brightness);
    int contrast() const;
    void setContrast(int contrast);
    int saturation() const;
    void setSaturation(int saturation);

    // 水印接口
    bool isWatermarkEnabled() const;
    void setWatermarkEnabled(bool enabled);
    QPixmap watermark() const;
    void setWatermark(const QPixmap& watermark);
    QPoint watermarkPosition() const;
    void setWatermarkPosition(const QPoint& position);
    qreal watermarkOpacity() const;
    void setWatermarkOpacity(qreal opacity);

    // 性能接口
    bool isMultithreadingEnabled() const;
    void setMultithreadingEnabled(bool enabled);
    int threadCount() const;
    void setThreadCount(int count);
    bool isHardwareAcceleration() const;
    void setHardwareAcceleration(bool enabled);

    // 统计信息接口
    qint64 totalFramesProcessed() const;
    double averageProcessingTime() const;
    QVariantMap getProcessingStatistics() const;

public slots:
    /**
     * @brief 重置处理器
     */
    void reset();

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

    /**
     * @brief 优化处理设置
     */
    void optimizeSettings();

signals:
    /**
     * @brief 处理器激活状态改变信号
     * @param active 是否激活
     */
    void activeChanged(bool active);

    /**
     * @brief 处理器状态改变信号
     * @param status 新的处理器状态
     */
    void statusChanged(ProcessorStatus status);

    /**
     * @brief 输出尺寸改变信号
     * @param size 新的输出尺寸
     */
    void outputSizeChanged(const QSize& size);
    
    /**
     * @brief 目标尺寸改变信号
     * @param size 新的目标尺寸
     */
    void targetSizeChanged(const QSize& size);
    
    /**
     * @brief 缩放模式改变信号
     * @param mode 新的缩放模式
     */
    void scalingModeChanged(ScalingMode mode);

    /**
     * @brief 质量改变信号
     * @param quality 新的质量设置
     */
    void qualityChanged(int quality);
    
    /**
     * @brief 缩放启用状态改变信号
     * @param enabled 是否启用缩放
     */
    void scalingEnabledChanged(bool enabled);
    
    /**
     * @brief 裁剪启用状态改变信号
     * @param enabled 是否启用裁剪
     */
    void cropEnabledChanged(bool enabled);
    
    /**
     * @brief 裁剪区域改变信号
     * @param region 新的裁剪区域
     */
    void cropRegionChanged(const QRect& region);
    
    /**
     * @brief 旋转启用状态改变信号
     * @param enabled 是否启用旋转
     */
    void rotationEnabledChanged(bool enabled);
    
    /**
     * @brief 旋转角度改变信号
     * @param angle 新的旋转角度
     */
    void rotationAngleChanged(int angle);
    
    /**
     * @brief 滤镜启用状态改变信号
     * @param enabled 是否启用滤镜
     */
    void filteringEnabledChanged(bool enabled);
    
    /**
     * @brief 亮度改变信号
     * @param brightness 新的亮度值
     */
    void brightnessChanged(int brightness);
    
    /**
     * @brief 对比度改变信号
     * @param contrast 新的对比度值
     */
    void contrastChanged(int contrast);
    
    /**
     * @brief 饱和度改变信号
     * @param saturation 新的饱和度值
     */
    void saturationChanged(int saturation);

    /**
     * @brief 帧处理完成信号
     * @param frame 处理后的帧
     * @param processingTime 处理时间(毫秒)
     */
    void frameProcessed(const QPixmap& frame, qint64 processingTime);

    /**
     * @brief 异步处理完成信号
     * @param frame 处理后的帧
     */
    void asyncProcessingFinished(const QPixmap& frame);

    /**
     * @brief 处理错误信号
     * @param error 错误信息
     */
    void processingError(const QString& error);

    /**
     * @brief 统计信息更新信号
     * @param statistics 统计信息
     */
    void statisticsUpdated(const QVariantMap& statistics);

private slots:
    void onAsyncProcessingFinished();
    void onStatisticsTimer();

private:
    void updateStatus(ProcessorStatus newStatus);
    QPixmap applyScaling(const QPixmap& frame);
    QPixmap applyCrop(const QPixmap& frame);
    QPixmap applyRotation(const QPixmap& frame);
    QPixmap applyFlipping(const QPixmap& frame);
    QPixmap applyFilters(const QPixmap& frame);
    QPixmap applyWatermark(const QPixmap& frame);
    void updateStatistics(qint64 processingTime);
    void emitError(const QString& error);
    void resetProcessor();

    // 滤镜实现
    QPixmap applyBlurFilter(const QPixmap& frame, const QVariantMap& params);
    QPixmap applySharpenFilter(const QPixmap& frame, const QVariantMap& params);
    QPixmap applyBrightnessFilter(const QPixmap& frame, const QVariantMap& params);
    QPixmap applyContrastFilter(const QPixmap& frame, const QVariantMap& params);
    QPixmap applySaturationFilter(const QPixmap& frame, const QVariantMap& params);
    QPixmap applyGrayscaleFilter(const QPixmap& frame, const QVariantMap& params);
    QPixmap applySepiaFilter(const QPixmap& frame, const QVariantMap& params);
    QPixmap applyInvertFilter(const QPixmap& frame, const QVariantMap& params);
    QPixmap applyEdgeDetectionFilter(const QPixmap& frame, const QVariantMap& params);

    class Private;
    Private* d;
};

#endif // FRAMEPROCESSOR_H