#include "CameraUtils.h"
#include <QDebug>
#include <QRegularExpression>
#include <QtMath>

QSize CameraUtils::resolutionForPreset(ICameraDevice::QualityPreset preset)
{
    switch (preset) {
    case ICameraDevice::LowQuality:
        return QSize(320, 240);
    case ICameraDevice::StandardQuality:
        return QSize(640, 480);
    case ICameraDevice::HighQuality:
        return QSize(1280, 720);
    case ICameraDevice::UltraQuality:
        return QSize(1920, 1080);
    default:
        return QSize(640, 480);
    }
}

int CameraUtils::frameRateForPreset(ICameraDevice::QualityPreset preset)
{
    switch (preset) {
    case ICameraDevice::LowQuality:
        return 15;
    case ICameraDevice::StandardQuality:
        return 30;
    case ICameraDevice::HighQuality:
        return 30;
    case ICameraDevice::UltraQuality:
        return 30;
    default:
        return 30;
    }
}

QString CameraUtils::presetName(ICameraDevice::QualityPreset preset)
{
    switch (preset) {
    case ICameraDevice::LowQuality:
        return "Low Quality (320x240@15fps)";
    case ICameraDevice::StandardQuality:
        return "Standard Quality (640x480@30fps)";
    case ICameraDevice::HighQuality:
        return "High Quality (1280x720@30fps)";
    case ICameraDevice::UltraQuality:
        return "Ultra Quality (1920x1080@30fps)";
    default:
        return "Unknown Quality";
    }
}

QString CameraUtils::statusName(ICameraDevice::Status status)
{
    switch (status) {
    case ICameraDevice::Inactive:
        return "Inactive";
    case ICameraDevice::Loading:
        return "Loading";
    case ICameraDevice::Loaded:
        return "Loaded";
    case ICameraDevice::Starting:
        return "Starting";
    case ICameraDevice::Active:
        return "Active";
    case ICameraDevice::Stopping:
        return "Stopping";
    case ICameraDevice::Stopped:
        return "Stopped";
    case ICameraDevice::Error:
        return "Error";
    default:
        return "Unknown";
    }
}

bool CameraUtils::isValidResolution(const QSize& resolution)
{
    return resolution.width() > 0 && 
           resolution.height() > 0 && 
           resolution.width() <= 4096 && 
           resolution.height() <= 4096 &&
           resolution.width() % 2 == 0 &&  // 确保宽度为偶数
           resolution.height() % 2 == 0;   // 确保高度为偶数
}

bool CameraUtils::isValidFrameRate(int frameRate)
{
    return frameRate > 0 && frameRate <= 120;
}

QList<QSize> CameraUtils::recommendedResolutions()
{
    return {
        QSize(320, 240),    // QVGA
        QSize(640, 480),    // VGA
        QSize(800, 600),    // SVGA
        QSize(1024, 768),   // XGA
        QSize(1280, 720),   // HD 720p
        QSize(1280, 960),   // SXGA
        QSize(1600, 1200),  // UXGA
        QSize(1920, 1080),  // Full HD 1080p
        QSize(2560, 1440),  // QHD
        QSize(3840, 2160)   // 4K UHD
    };
}

QList<int> CameraUtils::recommendedFrameRates()
{
    return {5, 10, 15, 20, 24, 25, 30, 50, 60, 120};
}

int CameraUtils::calculateBitrate(const QSize& resolution, int frameRate, ICameraDevice::QualityPreset preset)
{
    // 基础比特率计算：像素数 * 帧率 * 比特率因子
    qint64 pixels = resolution.width() * resolution.height();
    double factor = 0.1; // 基础因子
    
    // 根据质量预设调整因子
    switch (preset) {
    case ICameraDevice::LowQuality:
        factor = 0.05;
        break;
    case ICameraDevice::StandardQuality:
        factor = 0.1;
        break;
    case ICameraDevice::HighQuality:
        factor = 0.15;
        break;
    case ICameraDevice::UltraQuality:
        factor = 0.2;
        break;
    }
    
    return static_cast<int>(pixels * frameRate * factor);
}

QString CameraUtils::formatResolution(const QSize& resolution)
{
    return QString("%1x%2").arg(resolution.width()).arg(resolution.height());
}

QSize CameraUtils::parseResolution(const QString& resolutionStr)
{
    QRegularExpression regex(R"((\d+)x(\d+))");
    QRegularExpressionMatch match = regex.match(resolutionStr);
    
    if (match.hasMatch()) {
        int width = match.captured(1).toInt();
        int height = match.captured(2).toInt();
        return QSize(width, height);
    }
    
    return QSize();
}

QPixmap CameraUtils::frameToPixmap(const QVideoFrame& frame)
{
    if (!frame.isValid()) {
        return QPixmap();
    }
    
    // 这里需要根据实际的QVideoFrame实现来转换
    // 这是一个简化的实现
    QVideoFrame clonedFrame(frame);
    if (clonedFrame.map(QVideoFrame::ReadOnly)) {
        // Qt 6.x compatibility fix
        const uchar* bits = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        bits = clonedFrame.bits(0);
#else
        bits = clonedFrame.bits();
#endif
        QImage image(bits, 
                    clonedFrame.width(), 
                    clonedFrame.height(), 
                    QImage::Format_RGB32);
        clonedFrame.unmap();
        return QPixmap::fromImage(image);
    }
    
    return QPixmap();
}

qint64 CameraUtils::calculateFrameSize(const QSize& resolution, const QString& format)
{
    qint64 pixels = resolution.width() * resolution.height();
    
    if (format == "RGB32" || format == "ARGB32") {
        return pixels * 4; // 4 bytes per pixel
    } else if (format == "RGB24") {
        return pixels * 3; // 3 bytes per pixel
    } else if (format == "RGB16" || format == "RGB565") {
        return pixels * 2; // 2 bytes per pixel
    } else if (format == "YUV420P") {
        return pixels * 3 / 2; // 1.5 bytes per pixel
    }
    
    return pixels * 4; // 默认为RGB32
}

bool CameraUtils::isValidDeviceId(const QString& deviceId)
{
    return !deviceId.isEmpty() && deviceId.length() <= 256;
}

QString CameraUtils::generateFriendlyName(const QString& deviceId, const QString& deviceName)
{
    if (!deviceName.isEmpty()) {
        return deviceName;
    }
    
    // 从设备ID生成友好名称
    if (deviceId.contains("usb", Qt::CaseInsensitive)) {
        return "USB Camera";
    } else if (deviceId.contains("integrated", Qt::CaseInsensitive) || 
               deviceId.contains("built-in", Qt::CaseInsensitive)) {
        return "Built-in Camera";
    } else if (deviceId.contains("virtual", Qt::CaseInsensitive)) {
        return "Virtual Camera";
    }
    
    return QString("Camera (%1)").arg(deviceId.left(8));
}

int CameraUtils::compareResolutions(const QSize& res1, const QSize& res2)
{
    qint64 pixels1 = res1.width() * res1.height();
    qint64 pixels2 = res2.width() * res2.height();
    
    if (pixels1 < pixels2) return -1;
    if (pixels1 > pixels2) return 1;
    return 0;
}

QSize CameraUtils::findClosestResolution(const QSize& target, const QList<QSize>& supported)
{
    if (supported.isEmpty()) {
        return QSize();
    }
    
    QSize closest = supported.first();
    qint64 targetPixels = target.width() * target.height();
    qint64 minDiff = qAbs(closest.width() * closest.height() - targetPixels);
    
    for (const QSize& res : supported) {
        qint64 pixels = res.width() * res.height();
        qint64 diff = qAbs(pixels - targetPixels);
        
        if (diff < minDiff) {
            minDiff = diff;
            closest = res;
        }
    }
    
    return closest;
}

int CameraUtils::findClosestFrameRate(int target, const QList<int>& supported)
{
    if (supported.isEmpty()) {
        return 0;
    }
    
    int closest = supported.first();
    int minDiff = qAbs(closest - target);
    
    for (int rate : supported) {
        int diff = qAbs(rate - target);
        if (diff < minDiff) {
            minDiff = diff;
            closest = rate;
        }
    }
    
    return closest;
}

double CameraUtils::aspectRatio(const QSize& resolution)
{
    if (resolution.height() == 0) {
        return 0.0;
    }
    return static_cast<double>(resolution.width()) / resolution.height();
}

bool CameraUtils::isStandardAspectRatio(const QSize& resolution)
{
    double ratio = aspectRatio(resolution);
    
    // 常见的宽高比
    const double tolerance = 0.01;
    const QList<double> standardRatios = {
        4.0/3.0,    // 4:3
        16.0/9.0,   // 16:9
        16.0/10.0,  // 16:10
        3.0/2.0,    // 3:2
        5.0/4.0,    // 5:4
        1.0         // 1:1
    };
    
    for (double stdRatio : standardRatios) {
        if (qAbs(ratio - stdRatio) < tolerance) {
            return true;
        }
    }
    
    return false;
}

QString CameraUtils::aspectRatioName(const QSize& resolution)
{
    double ratio = aspectRatio(resolution);
    const double tolerance = 0.01;
    
    if (qAbs(ratio - 4.0/3.0) < tolerance) {
        return "4:3";
    } else if (qAbs(ratio - 16.0/9.0) < tolerance) {
        return "16:9";
    } else if (qAbs(ratio - 16.0/10.0) < tolerance) {
        return "16:10";
    } else if (qAbs(ratio - 3.0/2.0) < tolerance) {
        return "3:2";
    } else if (qAbs(ratio - 5.0/4.0) < tolerance) {
        return "5:4";
    } else if (qAbs(ratio - 1.0) < tolerance) {
        return "1:1";
    }
    
    return QString("%1:%2").arg(resolution.width()).arg(resolution.height());
}

QString CameraUtils::performanceLevel(const QSize& resolution, int frameRate)
{
    qint64 pixels = resolution.width() * resolution.height();
    qint64 pixelsPerSecond = pixels * frameRate;
    
    if (pixelsPerSecond < 5000000) {        // < 5M pixels/sec
        return "Low";
    } else if (pixelsPerSecond < 20000000) { // < 20M pixels/sec
        return "Medium";
    } else if (pixelsPerSecond < 60000000) { // < 60M pixels/sec
        return "High";
    } else {
        return "Ultra";
    }
}

qint64 CameraUtils::estimateMemoryUsage(const QSize& resolution, int frameRate, int bufferCount)
{
    qint64 frameSize = calculateFrameSize(resolution);
    return frameSize * bufferCount;
}