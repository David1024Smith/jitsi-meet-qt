#include "FrameProcessor.h"
#include <QPainter>
#include <QDebug>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QVariant>
#include <QVariantMap>
#include <QThread>

class FrameProcessor::Private
{
public:
    Private()
        : initialized(false)
        , scalingEnabled(true)
        , filteringEnabled(true)
        , cropEnabled(false)
        , rotationEnabled(false)
        , scalingMode(Qt::SmoothTransformation)
        , rotationAngle(0)
        , brightness(0)
        , contrast(0)
        , saturation(0)
    {
    }

    bool initialized;
    bool scalingEnabled;
    bool filteringEnabled;
    bool cropEnabled;
    bool rotationEnabled;
    
    Qt::TransformationMode scalingMode;
    QSize targetSize;
    QRect cropRegion;
    int rotationAngle;
    
    // 图像调整参数
    int brightness;  // -100 to 100
    int contrast;    // -100 to 100
    int saturation;  // -100 to 100
    
    mutable QMutex mutex;
};

FrameProcessor::FrameProcessor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

FrameProcessor::~FrameProcessor()
{
    shutdown();
    delete d;
}

bool FrameProcessor::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        return true;
    }
    
    try {
        d->initialized = true;
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "FrameProcessor initialization failed:" << e.what();
        return false;
    }
}

void FrameProcessor::shutdown()
{
    QMutexLocker locker(&d->mutex);
    d->initialized = false;
}

bool FrameProcessor::isInitialized() const
{
    QMutexLocker locker(&d->mutex);
    return d->initialized;
}

QPixmap FrameProcessor::processFrame(const QPixmap& frame)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "FrameProcessor not initialized";
        return frame;
    }
    
    if (frame.isNull()) {
        return frame;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        QPixmap processedFrame = frame;
        
        // 1. 裁剪
        if (d->cropEnabled && !d->cropRegion.isEmpty()) {
            processedFrame = applyCrop(processedFrame);
        }
        
        // 2. 旋转
        if (d->rotationEnabled && d->rotationAngle != 0) {
            processedFrame = applyRotation(processedFrame);
        }
        
        // 3. 缩放
        if (d->scalingEnabled && d->targetSize.isValid()) {
            processedFrame = applyScaling(processedFrame);
        }
        
        // 4. 图像滤镜
        if (d->filteringEnabled) {
            processedFrame = applyFilters(processedFrame);
        }
        
        qint64 processingTime = timer.elapsed();
        emit frameProcessed(processedFrame, processingTime);
        
        return processedFrame;
        
    } catch (const std::exception& e) {
        qWarning() << "Frame processing failed:" << e.what();
        emit processingError(QString("Processing failed: %1").arg(e.what()));
        return frame;
    }
}

void FrameProcessor::setScalingEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    if (d->scalingEnabled != enabled) {
        d->scalingEnabled = enabled;
        emit scalingEnabledChanged(enabled);
    }
}

bool FrameProcessor::isScalingEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->scalingEnabled;
}

void FrameProcessor::setTargetSize(const QSize& size)
{
    QMutexLocker locker(&d->mutex);
    if (d->targetSize != size) {
        d->targetSize = size;
        emit targetSizeChanged(size);
    }
}

QSize FrameProcessor::targetSize() const
{
    QMutexLocker locker(&d->mutex);
    return d->targetSize;
}

void FrameProcessor::setScalingMode(ScalingMode mode)
{
    QMutexLocker locker(&d->mutex);
    if (d->scalingMode != static_cast<Qt::TransformationMode>(mode)) {
        d->scalingMode = static_cast<Qt::TransformationMode>(mode);
        emit scalingModeChanged(mode);
    }
}

Qt::TransformationMode FrameProcessor::scalingMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->scalingMode;
}

void FrameProcessor::setCropEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    if (d->cropEnabled != enabled) {
        d->cropEnabled = enabled;
        emit cropEnabledChanged(enabled);
    }
}

bool FrameProcessor::isCropEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->cropEnabled;
}

void FrameProcessor::setCropRegion(const QRect& region)
{
    QMutexLocker locker(&d->mutex);
    if (d->cropRegion != region) {
        d->cropRegion = region;
        emit cropRegionChanged(region);
    }
}

QRect FrameProcessor::cropRegion() const
{
    QMutexLocker locker(&d->mutex);
    return d->cropRegion;
}

void FrameProcessor::setRotationEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    if (d->rotationEnabled != enabled) {
        d->rotationEnabled = enabled;
        emit rotationEnabledChanged(enabled);
    }
}

bool FrameProcessor::isRotationEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->rotationEnabled;
}

void FrameProcessor::setRotationAngle(int angle)
{
    QMutexLocker locker(&d->mutex);
    // 标准化角度到0-359范围
    angle = angle % 360;
    if (angle < 0) angle += 360;
    
    if (d->rotationAngle != angle) {
        d->rotationAngle = angle;
        emit rotationAngleChanged(angle);
    }
}

int FrameProcessor::rotationAngle() const
{
    QMutexLocker locker(&d->mutex);
    return d->rotationAngle;
}

void FrameProcessor::setFilteringEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    if (d->filteringEnabled != enabled) {
        d->filteringEnabled = enabled;
        emit filteringEnabledChanged(enabled);
    }
}

bool FrameProcessor::isFilteringEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->filteringEnabled;
}

void FrameProcessor::setBrightness(int brightness)
{
    QMutexLocker locker(&d->mutex);
    brightness = qBound(-100, brightness, 100);
    if (d->brightness != brightness) {
        d->brightness = brightness;
        emit brightnessChanged(brightness);
    }
}

int FrameProcessor::brightness() const
{
    QMutexLocker locker(&d->mutex);
    return d->brightness;
}

void FrameProcessor::setContrast(int contrast)
{
    QMutexLocker locker(&d->mutex);
    contrast = qBound(-100, contrast, 100);
    if (d->contrast != contrast) {
        d->contrast = contrast;
        emit contrastChanged(contrast);
    }
}

int FrameProcessor::contrast() const
{
    QMutexLocker locker(&d->mutex);
    return d->contrast;
}

void FrameProcessor::setSaturation(int saturation)
{
    QMutexLocker locker(&d->mutex);
    saturation = qBound(-100, saturation, 100);
    if (d->saturation != saturation) {
        d->saturation = saturation;
        emit saturationChanged(saturation);
    }
}

int FrameProcessor::saturation() const
{
    QMutexLocker locker(&d->mutex);
    return d->saturation;
}

QVariantMap FrameProcessor::getProcessingStatistics() const
{
    QMutexLocker locker(&d->mutex);
    
    QVariantMap stats;
    stats["initialized"] = d->initialized;
    stats["scalingEnabled"] = d->scalingEnabled;
    stats["filteringEnabled"] = d->filteringEnabled;
    stats["cropEnabled"] = d->cropEnabled;
    stats["rotationEnabled"] = d->rotationEnabled;
    stats["targetSize"] = d->targetSize;
    stats["cropRegion"] = d->cropRegion;
    stats["rotationAngle"] = d->rotationAngle;
    stats["brightness"] = d->brightness;
    stats["contrast"] = d->contrast;
    stats["saturation"] = d->saturation;
    
    return stats;
}

void FrameProcessor::resetProcessor()
{
    QMutexLocker locker(&d->mutex);
    
    d->brightness = 0;
    d->contrast = 0;
    d->saturation = 0;
    d->rotationAngle = 0;
    d->cropRegion = QRect();
    d->targetSize = QSize();
    
    emit brightnessChanged(0);
    emit contrastChanged(0);
    emit saturationChanged(0);
    emit rotationAngleChanged(0);
    emit cropRegionChanged(QRect());
    emit targetSizeChanged(QSize());
}

QPixmap FrameProcessor::applyCrop(const QPixmap& frame)
{
    if (d->cropRegion.isEmpty() || !d->cropRegion.intersects(frame.rect())) {
        return frame;
    }
    
    QRect validCrop = d->cropRegion.intersected(frame.rect());
    return frame.copy(validCrop);
}

QPixmap FrameProcessor::applyRotation(const QPixmap& frame)
{
    if (d->rotationAngle == 0) {
        return frame;
    }
    
    QTransform transform;
    transform.rotate(d->rotationAngle);
    
    return frame.transformed(transform, Qt::SmoothTransformation);
}

QPixmap FrameProcessor::applyScaling(const QPixmap& frame)
{
    if (!d->targetSize.isValid() || frame.size() == d->targetSize) {
        return frame;
    }
    
    return frame.scaled(d->targetSize, Qt::KeepAspectRatio, d->scalingMode);
}

QPixmap FrameProcessor::applyFilters(const QPixmap& frame)
{
    // 如果所有滤镜参数都是默认值，直接返回
    if (d->brightness == 0 && d->contrast == 0 && d->saturation == 0) {
        return frame;
    }
    
    QImage image = frame.toImage();
    if (image.isNull()) {
        return frame;
    }
    
    // 应用亮度、对比度和饱和度调整
    // 这是一个简化的实现，实际项目中可能需要更复杂的图像处理算法
    
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixel = image.pixel(x, y);
            
            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);
            int a = qAlpha(pixel);
            
            // 应用亮度调整
            if (d->brightness != 0) {
                int brightnessAdjust = (d->brightness * 255) / 100;
                r = qBound(0, r + brightnessAdjust, 255);
                g = qBound(0, g + brightnessAdjust, 255);
                b = qBound(0, b + brightnessAdjust, 255);
            }
            
            // 应用对比度调整
            if (d->contrast != 0) {
                double contrastFactor = (100.0 + d->contrast) / 100.0;
                r = qBound(0, static_cast<int>((r - 128) * contrastFactor + 128), 255);
                g = qBound(0, static_cast<int>((g - 128) * contrastFactor + 128), 255);
                b = qBound(0, static_cast<int>((b - 128) * contrastFactor + 128), 255);
            }
            
            // 简化的饱和度调整
            if (d->saturation != 0) {
                double saturationFactor = (100.0 + d->saturation) / 100.0;
                int gray = (r + g + b) / 3;
                r = qBound(0, static_cast<int>(gray + (r - gray) * saturationFactor), 255);
                g = qBound(0, static_cast<int>(gray + (g - gray) * saturationFactor), 255);
                b = qBound(0, static_cast<int>(gray + (b - gray) * saturationFactor), 255);
            }
            
            image.setPixel(x, y, qRgba(r, g, b, a));
        }
    }
    
    return QPixmap::fromImage(image);
}

// 处理器控制接口实现
FrameProcessor::ProcessorStatus FrameProcessor::status() const
{
    QMutexLocker locker(&d->mutex);
    return d->initialized ? Ready : Inactive;
}

bool FrameProcessor::isActive() const
{
    QMutexLocker locker(&d->mutex);
    return d->initialized;
}

// 尺寸和缩放接口实现
QSize FrameProcessor::outputSize() const
{
    return targetSize();
}

void FrameProcessor::setOutputSize(const QSize& size)
{
    setTargetSize(size);
    emit outputSizeChanged(size);
}
bool FrameProcessor::maintainAspectRatio() const
{
    return true; // 简化实现
}

void FrameProcessor::setMaintainAspectRatio(bool maintain)
{
    Q_UNUSED(maintain)
    // 简化实现
}

// 旋转和翻转接口实现
FrameProcessor::RotationAngle FrameProcessor::rotation() const
{
    int angle = rotationAngle();
    return static_cast<RotationAngle>(angle);
}

void FrameProcessor::setRotation(RotationAngle angle)
{
    setRotationAngle(static_cast<int>(angle));
}

bool FrameProcessor::isHorizontalFlip() const
{
    return false; // 简化实现
}

void FrameProcessor::setHorizontalFlip(bool flip)
{
    Q_UNUSED(flip)
    // 简化实现
}

bool FrameProcessor::isVerticalFlip() const
{
    return false; // 简化实现
}

void FrameProcessor::setVerticalFlip(bool flip)
{
    Q_UNUSED(flip)
    // 简化实现
}

// 质量和压缩接口实现
int FrameProcessor::quality() const
{
    return 75; // 简化实现
}

void FrameProcessor::setQuality(int quality)
{
    Q_UNUSED(quality)
    emit qualityChanged(quality);
}

int FrameProcessor::compressionLevel() const
{
    return 6; // 简化实现
}

void FrameProcessor::setCompressionLevel(int level)
{
    Q_UNUSED(level)
    // 简化实现
}

// 滤镜接口实现
QList<FrameProcessor::FilterType> FrameProcessor::activeFilters() const
{
    QList<FilterType> filters;
    if (brightness() != 0) filters.append(Brightness);
    if (contrast() != 0) filters.append(Contrast);
    if (saturation() != 0) filters.append(Saturation);
    return filters;
}

void FrameProcessor::addFilter(FilterType filter, const QVariantMap& parameters)
{
    Q_UNUSED(parameters)
    
    switch (filter) {
    case Brightness:
        setBrightness(parameters.value("value", 0).toInt());
        break;
    case Contrast:
        setContrast(parameters.value("value", 0).toInt());
        break;
    case Saturation:
        setSaturation(parameters.value("value", 0).toInt());
        break;
    default:
        // 其他滤镜的简化实现
        break;
    }
}

void FrameProcessor::removeFilter(FilterType filter)
{
    switch (filter) {
    case Brightness:
        setBrightness(0);
        break;
    case Contrast:
        setContrast(0);
        break;
    case Saturation:
        setSaturation(0);
        break;
    default:
        // 其他滤镜的简化实现
        break;
    }
}

void FrameProcessor::clearFilters()
{
    setBrightness(0);
    setContrast(0);
    setSaturation(0);
}

void FrameProcessor::setFilterParameter(FilterType filter, const QString& parameter, const QVariant& value)
{
    Q_UNUSED(filter)
    Q_UNUSED(parameter)
    Q_UNUSED(value)
    // 简化实现
}

QVariant FrameProcessor::getFilterParameter(FilterType filter, const QString& parameter) const
{
    Q_UNUSED(filter)
    Q_UNUSED(parameter)
    return QVariant(); // 简化实现
}

// 水印接口实现
bool FrameProcessor::isWatermarkEnabled() const
{
    return false; // 简化实现
}

void FrameProcessor::setWatermarkEnabled(bool enabled)
{
    Q_UNUSED(enabled)
    // 简化实现
}

QPixmap FrameProcessor::watermark() const
{
    return QPixmap(); // 简化实现
}

void FrameProcessor::setWatermark(const QPixmap& watermark)
{
    Q_UNUSED(watermark)
    // 简化实现
}

QPoint FrameProcessor::watermarkPosition() const
{
    return QPoint(); // 简化实现
}

void FrameProcessor::setWatermarkPosition(const QPoint& position)
{
    Q_UNUSED(position)
    // 简化实现
}

qreal FrameProcessor::watermarkOpacity() const
{
    return 1.0; // 简化实现
}

void FrameProcessor::setWatermarkOpacity(qreal opacity)
{
    Q_UNUSED(opacity)
    // 简化实现
}

// 性能接口实现
bool FrameProcessor::isMultithreadingEnabled() const
{
    return true; // 简化实现
}

void FrameProcessor::setMultithreadingEnabled(bool enabled)
{
    Q_UNUSED(enabled)
    // 简化实现
}

int FrameProcessor::threadCount() const
{
    return QThread::idealThreadCount();
}

void FrameProcessor::setThreadCount(int count)
{
    Q_UNUSED(count)
    // 简化实现
}

bool FrameProcessor::isHardwareAcceleration() const
{
    return false; // 简化实现
}

void FrameProcessor::setHardwareAcceleration(bool enabled)
{
    Q_UNUSED(enabled)
    // 简化实现
}

// 统计信息接口实现
qint64 FrameProcessor::totalFramesProcessed() const
{
    return 0; // 简化实现
}

double FrameProcessor::averageProcessingTime() const
{
    return 0.0; // 简化实现
}

// 异步处理接口实现
bool FrameProcessor::processFrameAsync(const QPixmap& frame)
{
    // 简化实现 - 直接同步处理
    QPixmap result = processFrame(frame);
    emit asyncProcessingFinished(result);
    return !result.isNull();
}

QByteArray FrameProcessor::processFrameData(const QByteArray& data, const QSize& size)
{
    Q_UNUSED(data)
    Q_UNUSED(size)
    return QByteArray(); // 简化实现
}

// 槽函数实现
void FrameProcessor::reset()
{
    resetProcessor();
}

void FrameProcessor::resetStatistics()
{
    // 简化实现
}

void FrameProcessor::optimizeSettings()
{
    // 简化实现 - 根据系统性能优化设置
}

// 私有槽函数实现
void FrameProcessor::onAsyncProcessingFinished()
{
    // 简化实现
}

void FrameProcessor::onStatisticsTimer()
{
    QVariantMap stats = getProcessingStatistics();
    emit statisticsUpdated(stats);
}

// 私有方法实现
void FrameProcessor::updateStatus(ProcessorStatus newStatus)
{
    emit statusChanged(newStatus);
}