#include "VideoEncoder.h"
#include <QBuffer>
#include <QDebug>
#include <QMutexLocker>
#include <QThread>

class VideoEncoder::Private
{
public:
    Private()
        : initialized(false)
        , format(static_cast<VideoEncoder::EncodingFormat>(0)) // 使用默认编码格式
        , quality(VideoEncoder::Medium) // 使用中等质量
        , bitrate(2000)
        , frameRate(30)
        , keyFrameInterval(30)
        , compressionLevel(6)
    {
    }

    bool initialized;
    VideoEncoder::EncodingFormat format;
    VideoEncoder::EncodingQuality quality;
    int bitrate;
    int frameRate;
    int keyFrameInterval;
    int compressionLevel;
    QSize frameSize;
    
    mutable QMutex mutex;
};

VideoEncoder::VideoEncoder(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

VideoEncoder::~VideoEncoder()
{
    shutdown();
    delete d;
}

bool VideoEncoder::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        return true;
    }
    
    try {
        // 初始化编码器设置
        d->initialized = true;
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "VideoEncoder initialization failed:" << e.what();
        return false;
    }
}

void VideoEncoder::shutdown()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        return;
    }
    
    d->initialized = false;
}

bool VideoEncoder::isInitialized() const
{
    QMutexLocker locker(&d->mutex);
    return d->initialized;
}

void VideoEncoder::setEncodingFormat(EncodingFormat format)
{
    QMutexLocker locker(&d->mutex);
    if (d->format != format) {
        d->format = format;
        emit formatChanged(format);
    }
}

VideoEncoder::EncodingFormat VideoEncoder::encodingFormat() const
{
    QMutexLocker locker(&d->mutex);
    return d->format;
}

void VideoEncoder::setEncodingQuality(EncodingQuality quality)
{
    QMutexLocker locker(&d->mutex);
    if (d->quality != quality) {
        d->quality = quality;
        
        // 根据质量调整比特率
        switch (quality) {
        case Low:
            d->bitrate = 500;
            d->compressionLevel = 9;
            break;
        case Medium:
            d->bitrate = 2000;
            d->compressionLevel = 6;
            break;
        case High:
            d->bitrate = 5000;
            d->compressionLevel = 3;
            break;
        case VeryHigh:
            d->bitrate = 10000;
            d->compressionLevel = 1;
            break;
        }
        
        emit qualityChanged(quality);
    }
}

VideoEncoder::EncodingQuality VideoEncoder::encodingQuality() const
{
    QMutexLocker locker(&d->mutex);
    return d->quality;
}

void VideoEncoder::setBitrate(int kbps)
{
    QMutexLocker locker(&d->mutex);
    if (kbps > 0 && d->bitrate != kbps) {
        d->bitrate = kbps;
        emit bitrateChanged(kbps);
    }
}

int VideoEncoder::bitrate() const
{
    QMutexLocker locker(&d->mutex);
    return d->bitrate;
}

void VideoEncoder::setFrameRate(int fps)
{
    QMutexLocker locker(&d->mutex);
    if (fps > 0 && fps <= 120 && d->frameRate != fps) {
        d->frameRate = fps;
        emit frameRateChanged(fps);
    }
}

int VideoEncoder::frameRate() const
{
    QMutexLocker locker(&d->mutex);
    return d->frameRate;
}

void VideoEncoder::setFrameSize(const QSize& size)
{
    QMutexLocker locker(&d->mutex);
    if (size.isValid() && d->frameSize != size) {
        d->frameSize = size;
        emit frameSizeChanged(size);
    }
}

QSize VideoEncoder::frameSize() const
{
    QMutexLocker locker(&d->mutex);
    return d->frameSize;
}

void VideoEncoder::setKeyFrameInterval(int frames)
{
    QMutexLocker locker(&d->mutex);
    if (frames > 0 && d->keyFrameInterval != frames) {
        d->keyFrameInterval = frames;
    }
}

int VideoEncoder::keyFrameInterval() const
{
    QMutexLocker locker(&d->mutex);
    return d->keyFrameInterval;
}

QByteArray VideoEncoder::encodeFrameRaw(const QPixmap& frame)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "VideoEncoder not initialized";
        return QByteArray();
    }
    
    if (frame.isNull()) {
        return QByteArray();
    }
    
    try {
        // 简化的编码实现 - 实际项目中应该使用真正的视频编码库
        QByteArray encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QIODevice::WriteOnly);
        
        // 根据格式选择编码方式
        QString format;
        int quality = 75;
        
        switch (d->format) {
        case EncodingFormat::H264:
        case EncodingFormat::VP8:
        case EncodingFormat::VP9:
        case EncodingFormat::AV1:
            format = "JPEG"; // 简化实现，实际应该使用对应的视频编码
            break;
        }
        
        // 根据质量调整压缩参数
        switch (d->quality) {
        case EncodingQuality::Low:
            quality = 50;
            break;
        case EncodingQuality::Medium:
            quality = 75;
            break;
        case EncodingQuality::High:
            quality = 90;
            break;
        case EncodingQuality::VeryHigh:
            quality = 100;
            format = "PNG";
            break;
        }
        
        // 调整帧大小
        QPixmap scaledFrame = frame;
        if (d->frameSize.isValid() && frame.size() != d->frameSize) {
            scaledFrame = frame.scaled(d->frameSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        
        // 保存编码数据
        scaledFrame.save(&buffer, format.toUtf8().constData(), quality);
        
        emit frameEncoded(encodedData);
        return encodedData;
        
    } catch (const std::exception& e) {
        qWarning() << "Frame encoding failed:" << e.what();
        emit encodingError(QString("Encoding failed: %1").arg(e.what()));
        return QByteArray();
    }
}

QVariantMap VideoEncoder::getEncodingStatistics() const
{
    QMutexLocker locker(&d->mutex);
    
    QVariantMap stats;
    stats["format"] = static_cast<int>(d->format);
    stats["quality"] = static_cast<int>(d->quality);
    stats["bitrate"] = d->bitrate;
    stats["frameRate"] = d->frameRate;
    stats["frameSize"] = d->frameSize;
    stats["keyFrameInterval"] = d->keyFrameInterval;
    stats["initialized"] = d->initialized;
    
    return stats;
}

// 编码器控制接口实现
bool VideoEncoder::start()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        if (!initialize()) {
            return false;
        }
    }
    
    updateStatus(Ready);
    emit activeChanged(true);
    return true;
}

void VideoEncoder::stop()
{
    QMutexLocker locker(&d->mutex);
    updateStatus(Ready);
    emit activeChanged(false);
}

void VideoEncoder::pause()
{
    QMutexLocker locker(&d->mutex);
    updateStatus(Paused);
}

void VideoEncoder::resume()
{
    QMutexLocker locker(&d->mutex);
    updateStatus(Ready);
}

VideoEncoder::EncoderStatus VideoEncoder::status() const
{
    QMutexLocker locker(&d->mutex);
    return static_cast<EncoderStatus>(d->quality); // 简化实现
}

bool VideoEncoder::isActive() const
{
    QMutexLocker locker(&d->mutex);
    return d->initialized;
}

bool VideoEncoder::isPaused() const
{
    return status() == Paused;
}

// 视频参数接口实现
QSize VideoEncoder::resolution() const
{
    return frameSize();
}

void VideoEncoder::setResolution(const QSize& size)
{
    setFrameSize(size);
    emit resolutionChanged(size);
}

// 高级配置接口实现
bool VideoEncoder::hardwareAcceleration() const
{
    return true; // 简化实现
}

void VideoEncoder::setHardwareAcceleration(bool enabled)
{
    Q_UNUSED(enabled)
    // 简化实现 - 实际项目中应该配置硬件加速
}

int VideoEncoder::threadCount() const
{
    return QThread::idealThreadCount();
}

void VideoEncoder::setThreadCount(int count)
{
    Q_UNUSED(count)
    // 简化实现
}

int VideoEncoder::bufferSize() const
{
    return 1024 * 1024; // 1MB
}

void VideoEncoder::setBufferSize(int size)
{
    Q_UNUSED(size)
    // 简化实现
}

VideoEncoder::EncodingPreset VideoEncoder::encodingPreset() const
{
    return Fast; // 简化实现
}

void VideoEncoder::setEncodingPreset(EncodingPreset preset)
{
    Q_UNUSED(preset)
    // 简化实现
}

// 编码接口实现
bool VideoEncoder::encodeFrame(const QPixmap& frame)
{
    QByteArray data = encodeFrameRaw(frame);
    return !data.isEmpty();
}

bool VideoEncoder::encodeFrameData(const QByteArray& data, const QSize& size)
{
    Q_UNUSED(data)
    Q_UNUSED(size)
    // 简化实现 - 实际项目中应该处理原始帧数据
    return false;
}

void VideoEncoder::flush()
{
    // 简化实现 - 实际项目中应该刷新编码器缓冲区
}

// 统计信息接口实现
qint64 VideoEncoder::totalFramesEncoded() const
{
    return 0; // 简化实现
}

double VideoEncoder::averageEncodingTime() const
{
    return 0.0; // 简化实现
}

qint64 VideoEncoder::totalEncodedBytes() const
{
    return 0; // 简化实现
}

double VideoEncoder::compressionRatio() const
{
    return 1.0; // 简化实现
}

// 槽函数实现
void VideoEncoder::reset()
{
    resetEncoder();
}

void VideoEncoder::resetStatistics()
{
    // 简化实现
}

void VideoEncoder::optimizeSettings()
{
    // 简化实现 - 实际项目中应该根据系统性能优化设置
}

// 私有方法实现
void VideoEncoder::updateStatus(EncoderStatus newStatus)
{
    // 简化实现
    emit statusChanged(newStatus);
}

void VideoEncoder::onEncodingTimer()
{
    // 简化实现
}

void VideoEncoder::onStatisticsTimer()
{
    QVariantMap stats = getEncodingStatistics();
    emit statisticsUpdated(stats);
}

void VideoEncoder::resetEncoder()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        shutdown();
        initialize();
    }
}