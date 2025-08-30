#include "ScreenCapture.h"
#include <QApplication>
#include <QScreen>
#include <QPixmap>
#include <QTimer>
#include <QMutexLocker>
#include <QDebug>
#include <QBuffer>

class ScreenCapture::Private
{
public:
    Private()
        : status(IScreenCapture::Inactive)
        , initialized(false)
        , captureMode(IScreenCapture::FullScreen)
        , quality(IScreenCapture::MediumQuality)
        , frameRate(30)
        , captureCursor(true)
        , captureDelay(0)
        , compressionQuality(75)
        , targetScreen(nullptr)
        , captureTimer(nullptr)
    {
    }

    IScreenCapture::CaptureStatus status;
    bool initialized;
    IScreenCapture::CaptureMode captureMode;
    IScreenCapture::CaptureQuality quality;
    int frameRate;
    bool captureCursor;
    int captureDelay;
    int compressionQuality;
    
    QScreen* targetScreen;
    QRect captureRegion;
    QTimer* captureTimer;
    
    mutable QMutex mutex;
};

ScreenCapture::ScreenCapture(QObject *parent)
    : IScreenCapture(parent)
    , d(new Private)
{
    d->captureTimer = new QTimer(this);
    connect(d->captureTimer, &QTimer::timeout,
            this, &ScreenCapture::onCaptureTimer);
    
    // 监听屏幕变化
    connect(QApplication::instance(), &QApplication::screenAdded,
            this, &ScreenCapture::onScreenChanged);
    connect(QApplication::instance(), &QApplication::screenRemoved,
            this, &ScreenCapture::onScreenChanged);
}

ScreenCapture::~ScreenCapture()
{
    stopCapture();
    delete d;
}

bool ScreenCapture::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        return true;
    }
    
    try {
        // 初始化捕获设置
        initializeCapture();
        
        d->initialized = true;
        updateStatus(Inactive);
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "ScreenCapture initialization failed:" << e.what();
        return false;
    }
}

bool ScreenCapture::startCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        emitError("ScreenCapture not initialized");
        return false;
    }
    
    if (d->status == Active) {
        return true;
    }
    
    updateStatus(Initializing);
    
    try {
        // 确保有目标屏幕
        if (!d->targetScreen) {
            autoSelectScreen();
        }
        
        if (!d->targetScreen) {
            throw std::runtime_error("No screen available for capture");
        }
        
        // 启动捕获定时器
        updateCaptureTimer();
        d->captureTimer->start();
        
        updateStatus(Active);
        emit captureStarted();
        return true;
        
    } catch (const std::exception& e) {
        emitError(QString("Failed to start capture: %1").arg(e.what()));
        updateStatus(Error);
        return false;
    }
}

void ScreenCapture::stopCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status == Inactive) {
        return;
    }
    
    // 停止定时器
    if (d->captureTimer) {
        d->captureTimer->stop();
    }
    
    updateStatus(Inactive);
    emit captureStopped();
}

void ScreenCapture::pauseCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Active) {
        return;
    }
    
    if (d->captureTimer) {
        d->captureTimer->stop();
    }
    
    updateStatus(Paused);
    emit capturePaused();
}

void ScreenCapture::resumeCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Paused) {
        return;
    }
    
    if (d->captureTimer) {
        d->captureTimer->start();
    }
    
    updateStatus(Active);
    emit captureResumed();
}

IScreenCapture::CaptureStatus ScreenCapture::status() const
{
    QMutexLocker locker(&d->mutex);
    return d->status;
}

bool ScreenCapture::isCapturing() const
{
    QMutexLocker locker(&d->mutex);
    return d->status == Active;
}

bool ScreenCapture::isInitialized() const
{
    QMutexLocker locker(&d->mutex);
    return d->initialized;
}

void ScreenCapture::setCaptureMode(CaptureMode mode)
{
    QMutexLocker locker(&d->mutex);
    if (d->captureMode != mode) {
        d->captureMode = mode;
        
        // 根据模式调整捕获区域
        if (mode == FullScreen && d->targetScreen) {
            d->captureRegion = d->targetScreen->geometry();
        }
    }
}

IScreenCapture::CaptureMode ScreenCapture::captureMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureMode;
}

void ScreenCapture::setCaptureQuality(CaptureQuality quality)
{
    QMutexLocker locker(&d->mutex);
    if (d->quality != quality) {
        d->quality = quality;
        
        // 根据质量调整压缩设置
        switch (quality) {
        case LowQuality:
            d->compressionQuality = 50;
            break;
        case MediumQuality:
            d->compressionQuality = 75;
            break;
        case HighQuality:
            d->compressionQuality = 90;
            break;
        case UltraQuality:
            d->compressionQuality = 100;
            break;
        }
    }
}

IScreenCapture::CaptureQuality ScreenCapture::captureQuality() const
{
    QMutexLocker locker(&d->mutex);
    return d->quality;
}

void ScreenCapture::setFrameRate(int fps)
{
    QMutexLocker locker(&d->mutex);
    if (fps > 0 && fps <= 120 && d->frameRate != fps) {
        d->frameRate = fps;
        updateCaptureTimer();
    }
}

int ScreenCapture::frameRate() const
{
    QMutexLocker locker(&d->mutex);
    return d->frameRate;
}

void ScreenCapture::setCaptureRegion(const QRect& region)
{
    QMutexLocker locker(&d->mutex);
    if (d->captureRegion != region) {
        d->captureRegion = region;
        if (!region.isEmpty()) {
            d->captureMode = Region;
        }
    }
}

QRect ScreenCapture::captureRegion() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureRegion;
}

void ScreenCapture::setTargetScreen(QScreen* screen)
{
    QMutexLocker locker(&d->mutex);
    if (d->targetScreen != screen) {
        d->targetScreen = screen;
        
        if (screen && d->captureMode == FullScreen) {
            d->captureRegion = screen->geometry();
        }
        
        // 连接屏幕信号
        if (screen) {
            connect(screen, &QScreen::geometryChanged,
                    this, &ScreenCapture::refreshScreenInfo, Qt::UniqueConnection);
        }
    }
}

QScreen* ScreenCapture::targetScreen() const
{
    QMutexLocker locker(&d->mutex);
    return d->targetScreen;
}

QPixmap ScreenCapture::captureFrame()
{
    QMutexLocker locker(&d->mutex);
    
    if (!isCapturing()) {
        return QPixmap();
    }
    
    try {
        QPixmap frame = captureScreenInternal();
        if (!frame.isNull()) {
            frame = applyCaptureQuality(frame);
            emit frameCaptured(frame);
        }
        return frame;
        
    } catch (const std::exception& e) {
        emitError(QString("Frame capture failed: %1").arg(e.what()));
        return QPixmap();
    }
}

QByteArray ScreenCapture::captureFrameData()
{
    QPixmap frame = captureFrame();
    if (frame.isNull()) {
        return QByteArray();
    }
    
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    
    QString format = (d->quality == UltraQuality) ? "PNG" : "JPEG";
    frame.save(&buffer, format.toUtf8().constData(), d->compressionQuality);
    
    return data;
}

QSize ScreenCapture::captureSize() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureRegion.size();
}

void ScreenCapture::setCaptureCursor(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    d->captureCursor = enabled;
}

bool ScreenCapture::isCaptureCursorEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureCursor;
}

void ScreenCapture::setCaptureDelay(int ms)
{
    QMutexLocker locker(&d->mutex);
    if (ms >= 0) {
        d->captureDelay = ms;
    }
}

int ScreenCapture::captureDelay() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureDelay;
}

void ScreenCapture::setCompressionQuality(int quality)
{
    QMutexLocker locker(&d->mutex);
    if (quality >= 0 && quality <= 100) {
        d->compressionQuality = quality;
    }
}

int ScreenCapture::compressionQuality() const
{
    QMutexLocker locker(&d->mutex);
    return d->compressionQuality;
}

void ScreenCapture::refreshScreenInfo()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->targetScreen && d->captureMode == FullScreen) {
        d->captureRegion = d->targetScreen->geometry();
    }
}

void ScreenCapture::autoSelectScreen()
{
    const auto screens = QApplication::screens();
    if (!screens.isEmpty()) {
        // 选择主屏幕
        setTargetScreen(QApplication::primaryScreen());
    }
}

void ScreenCapture::onCaptureTimer()
{
    if (d->captureDelay > 0) {
        QTimer::singleShot(d->captureDelay, this, [this]() {
            captureFrame();
        });
    } else {
        captureFrame();
    }
}

void ScreenCapture::onScreenChanged()
{
    // 屏幕配置改变时刷新信息
    refreshScreenInfo();
}

void ScreenCapture::initializeCapture()
{
    // 自动选择屏幕
    if (!d->targetScreen) {
        autoSelectScreen();
    }
    
    // 设置默认捕获区域
    if (d->targetScreen && d->captureRegion.isEmpty()) {
        d->captureRegion = d->targetScreen->geometry();
    }
}

void ScreenCapture::cleanupCapture()
{
    if (d->captureTimer) {
        d->captureTimer->stop();
    }
}

void ScreenCapture::updateCaptureTimer()
{
    if (d->captureTimer && d->frameRate > 0) {
        int interval = 1000 / d->frameRate;
        d->captureTimer->setInterval(interval);
    }
}

void ScreenCapture::updateStatus(CaptureStatus newStatus)
{
    if (d->status != newStatus) {
        d->status = newStatus;
        emit statusChanged(newStatus);
    }
}

QPixmap ScreenCapture::captureScreenInternal()
{
    if (!d->targetScreen) {
        return QPixmap();
    }
    
    QPixmap screenshot;
    
    if (d->captureMode == FullScreen) {
        screenshot = d->targetScreen->grabWindow(0);
    } else if (d->captureMode == Region && !d->captureRegion.isEmpty()) {
        screenshot = d->targetScreen->grabWindow(0, 
                                                d->captureRegion.x(),
                                                d->captureRegion.y(),
                                                d->captureRegion.width(),
                                                d->captureRegion.height());
    }
    
    return screenshot;
}

QPixmap ScreenCapture::applyCaptureQuality(const QPixmap& source)
{
    if (source.isNull()) {
        return source;
    }
    
    QPixmap result = source;
    
    // 根据质量设置调整图像
    switch (d->quality) {
    case LowQuality:
        // 降低分辨率
        result = source.scaled(source.size() * 0.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        break;
    case MediumQuality:
        // 保持原始分辨率
        break;
    case HighQuality:
        // 保持原始分辨率，高质量
        break;
    case UltraQuality:
        // 最高质量，可能进行锐化等处理
        break;
    }
    
    return result;
}

// 捕获质量自适应和性能优化实现
void ScreenCapture::optimizeCaptureQuality()
{
    QMutexLocker locker(&d->mutex);
    
    // 基于系统性能动态调整捕获质量
    double cpuUsage = getCurrentCPUUsage();
    qint64 memoryUsage = getCurrentMemoryUsage();
    
    if (cpuUsage > 80.0 || memoryUsage > 80) {
        // 高负载时降低质量
        if (d->quality > LowQuality) {
            setCaptureQuality(static_cast<CaptureQuality>(d->quality - 1));
            qDebug() << "Reduced capture quality due to high system load";
        }
        
        // 降低帧率
        if (d->frameRate > 15) {
            setFrameRate(qMax(15, d->frameRate - 5));
        }
    } else if (cpuUsage < 50.0 && memoryUsage < 50) {
        // 低负载时提高质量
        if (d->quality < UltraQuality) {
            setCaptureQuality(static_cast<CaptureQuality>(d->quality + 1));
            qDebug() << "Increased capture quality due to low system load";
        }
        
        // 提高帧率
        if (d->frameRate < 30) {
            setFrameRate(qMin(30, d->frameRate + 5));
        }
    }
}

double ScreenCapture::getCurrentCPUUsage() const
{
    // 简化实现 - 实际项目中应该使用系统API获取CPU使用率
    static double simulatedCPU = 50.0;
    simulatedCPU += (qrand() % 21 - 10); // 随机变化 -10 到 +10
    return qBound(0.0, simulatedCPU, 100.0);
}

qint64 ScreenCapture::getCurrentMemoryUsage() const
{
    // 简化实现 - 实际项目中应该使用系统API获取内存使用率
    static qint64 simulatedMemory = 50;
    simulatedMemory += (qrand() % 21 - 10); // 随机变化 -10 到 +10
    return qBound(0LL, simulatedMemory, 100LL);
}

void ScreenCapture::enableAdaptiveQuality(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    
    if (enabled) {
        // 启动性能监控定时器
        QTimer* adaptiveTimer = new QTimer(this);
        adaptiveTimer->setInterval(5000); // 每5秒检查一次
        connect(adaptiveTimer, &QTimer::timeout, this, &ScreenCapture::optimizeCaptureQuality);
        adaptiveTimer->start();
    }
}

void ScreenCapture::emitError(const QString& error)
{
    qWarning() << "ScreenCapture error:" << error;
    emit captureError(error);
}