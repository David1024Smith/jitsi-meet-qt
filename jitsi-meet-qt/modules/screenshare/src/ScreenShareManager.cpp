#include "../include/ScreenShareManager.h"
#include "../include/CaptureEngine.h"
#include "../interfaces/IScreenCapture.h"
#include "../config/ScreenShareConfig.h"
#include "../capture/ScreenCapture.h"
#include "../capture/WindowCapture.h"
#include "../capture/RegionCapture.h"
#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <QDebug>
#include <QMutexLocker>
#include <QTimer>

class ScreenShareManager::Private
{
public:
    Private()
        : status(IScreenShareManager::Uninitialized)
        , shareMode(IScreenShareManager::LocalPreview)
        , encodingFormat(IScreenShareManager::H264)
        , quality(IScreenCapture::MediumQuality)
        , frameRate(30)
        , bitrate(2000)
        , maxFrameRate(60)
        , maxBitrate(10000)
        , autoQualityAdjustment(true)
        , captureEngine(nullptr)
        , currentCapture(nullptr)
        , config(nullptr)
        , statisticsTimer(nullptr)
        , qualityAdjustmentTimer(nullptr)
        , totalFrames(0)
        , currentFPS(0.0)
        , currentBitrate(0)
    {
    }

    IScreenShareManager::ManagerStatus status;
    IScreenShareManager::ShareMode shareMode;
    IScreenShareManager::EncodingFormat encodingFormat;
    IScreenCapture::CaptureQuality quality;
    int frameRate;
    int bitrate;
    int maxFrameRate;
    int maxBitrate;
    bool autoQualityAdjustment;
    
    CaptureEngine* captureEngine;
    IScreenCapture* currentCapture;
    ScreenShareConfig* config;
    
    QTimer* statisticsTimer;
    QTimer* qualityAdjustmentTimer;
    
    QString currentSourceId;
    QVariantMap configuration;
    QStringList availableScreenIds;
    QStringList availableWindowIds;
    
    // 统计信息
    qint64 totalFrames;
    double currentFPS;
    int currentBitrate;
    QVariantMap statistics;
    
    mutable QMutex mutex;
};

ScreenShareManager::ScreenShareManager(QObject *parent)
    : IScreenShareManager(parent)
    , d(new Private)
{
    d->statisticsTimer = new QTimer(this);
    d->qualityAdjustmentTimer = new QTimer(this);
    
    connect(d->statisticsTimer, &QTimer::timeout,
            this, &ScreenShareManager::onStatisticsTimer);
    connect(d->qualityAdjustmentTimer, &QTimer::timeout,
            this, &ScreenShareManager::onQualityAdjustmentTimer);
}

ScreenShareManager::~ScreenShareManager()
{
    shutdown();
    delete d;
}

bool ScreenShareManager::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Uninitialized) {
        return d->status == Ready;
    }
    
    try {
        // 创建配置对象
        if (!d->config) {
            d->config = new ScreenShareConfig(this);
        }
        
        // 创建捕获引擎
        if (!d->captureEngine) {
            d->captureEngine = new CaptureEngine(this);
            if (!d->captureEngine->initialize()) {
                throw std::runtime_error("Failed to initialize capture engine");
            }
        }
        
        // 刷新可用源
        refreshAvailableSources();
        
        // 启动定时器
        d->statisticsTimer->start(1000); // 每秒更新统计信息
        if (d->autoQualityAdjustment) {
            d->qualityAdjustmentTimer->start(5000); // 每5秒调整质量
        }
        
        updateStatus(Ready);
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "ScreenShareManager initialization failed:" << e.what();
        updateStatus(Error);
        return false;
    }
}

void ScreenShareManager::shutdown()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status == Uninitialized) {
        return;
    }
    
    // 停止共享
    if (isSharing()) {
        stopScreenShare();
    }
    
    // 停止定时器
    if (d->statisticsTimer) {
        d->statisticsTimer->stop();
    }
    if (d->qualityAdjustmentTimer) {
        d->qualityAdjustmentTimer->stop();
    }
    
    // 清理捕获引擎
    if (d->captureEngine) {
        d->captureEngine->shutdown();
    }
    
    // 清理捕获对象
    cleanupCapture();
    
    updateStatus(Uninitialized);
}

IScreenShareManager::ManagerStatus ScreenShareManager::status() const
{
    QMutexLocker locker(&d->mutex);
    return d->status;
}

bool ScreenShareManager::isReady() const
{
    return status() == Ready;
}

bool ScreenShareManager::startScreenShare(const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Ready) {
        emitError("Manager not ready for screen sharing");
        return false;
    }
    
    if (isSharing()) {
        qWarning() << "Screen sharing already active";
        return true;
    }
    
    try {
        // 验证配置
        if (!config.isEmpty() && !validateShareConfiguration(config)) {
            throw std::runtime_error("Invalid share configuration");
        }
        
        // 应用配置
        if (!config.isEmpty()) {
            setConfiguration(config);
        }
        
        // 初始化捕获
        initializeCapture();
        
        if (!d->currentCapture) {
            throw std::runtime_error("No capture source available");
        }
        
        // 配置捕获引擎
        d->captureEngine->setCaptureSource(d->currentCapture);
        d->captureEngine->setTargetFrameRate(d->frameRate);
        
        // 启动捕获
        if (!d->captureEngine->start()) {
            throw std::runtime_error("Failed to start capture engine");
        }
        
        updateStatus(Sharing);
        emit shareStarted();
        return true;
        
    } catch (const std::exception& e) {
        emitError(QString("Failed to start screen share: %1").arg(e.what()));
        return false;
    }
}

void ScreenShareManager::stopScreenShare()
{
    QMutexLocker locker(&d->mutex);
    
    if (!isSharing()) {
        return;
    }
    
    // 停止捕获引擎
    if (d->captureEngine) {
        d->captureEngine->stop();
    }
    
    // 清理捕获
    cleanupCapture();
    
    updateStatus(Ready);
    emit shareStopped();
}

void ScreenShareManager::pauseScreenShare()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Sharing) {
        return;
    }
    
    if (d->captureEngine) {
        d->captureEngine->pause();
    }
    
    updateStatus(Paused);
    emit sharePaused();
}

void ScreenShareManager::resumeScreenShare()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Paused) {
        return;
    }
    
    if (d->captureEngine) {
        d->captureEngine->resume();
    }
    
    updateStatus(Sharing);
    emit shareResumed();
}

bool ScreenShareManager::isSharing() const
{
    QMutexLocker locker(&d->mutex);
    return d->status == Sharing || d->status == Paused;
}

void ScreenShareManager::setShareMode(ShareMode mode)
{
    QMutexLocker locker(&d->mutex);
    if (d->shareMode != mode) {
        d->shareMode = mode;
        d->configuration["shareMode"] = static_cast<int>(mode);
    }
}

IScreenShareManager::ShareMode ScreenShareManager::shareMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->shareMode;
}

void ScreenShareManager::setEncodingFormat(EncodingFormat format)
{
    QMutexLocker locker(&d->mutex);
    if (d->encodingFormat != format) {
        d->encodingFormat = format;
        d->configuration["encodingFormat"] = static_cast<int>(format);
    }
}

IScreenShareManager::EncodingFormat ScreenShareManager::encodingFormat() const
{
    QMutexLocker locker(&d->mutex);
    return d->encodingFormat;
}

void ScreenShareManager::setConfiguration(const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    d->configuration = config;
    
    // 应用配置
    if (config.contains("shareMode")) {
        d->shareMode = static_cast<ShareMode>(config["shareMode"].toInt());
    }
    if (config.contains("encodingFormat")) {
        d->encodingFormat = static_cast<EncodingFormat>(config["encodingFormat"].toInt());
    }
    if (config.contains("quality")) {
        d->quality = static_cast<IScreenCapture::CaptureQuality>(config["quality"].toInt());
    }
    if (config.contains("frameRate")) {
        d->frameRate = config["frameRate"].toInt();
    }
    if (config.contains("bitrate")) {
        d->bitrate = config["bitrate"].toInt();
    }
}

QVariantMap ScreenShareManager::configuration() const
{
    QMutexLocker locker(&d->mutex);
    return d->configuration;
}

QStringList ScreenShareManager::availableScreens() const
{
    QMutexLocker locker(&d->mutex);
    return d->availableScreenIds;
}

QStringList ScreenShareManager::availableWindows() const
{
    QMutexLocker locker(&d->mutex);
    return d->availableWindowIds;
}

bool ScreenShareManager::selectScreen(const QString& screenId)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->availableScreenIds.contains(screenId)) {
        emitError(QString("Screen not available: %1").arg(screenId));
        return false;
    }
    
    d->currentSourceId = screenId;
    d->configuration["sourceType"] = "screen";
    d->configuration["sourceId"] = screenId;
    
    return true;
}

bool ScreenShareManager::selectWindow(const QString& windowId)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->availableWindowIds.contains(windowId)) {
        emitError(QString("Window not available: %1").arg(windowId));
        return false;
    }
    
    d->currentSourceId = windowId;
    d->configuration["sourceType"] = "window";
    d->configuration["sourceId"] = windowId;
    
    return true;
}

QString ScreenShareManager::currentSource() const
{
    QMutexLocker locker(&d->mutex);
    return d->currentSourceId;
}

void ScreenShareManager::setQuality(IScreenCapture::CaptureQuality quality)
{
    QMutexLocker locker(&d->mutex);
    if (d->quality != quality) {
        d->quality = quality;
        d->configuration["quality"] = static_cast<int>(quality);
        
        if (d->currentCapture) {
            d->currentCapture->setCaptureQuality(quality);
        }
        
        emit qualityChanged(quality);
    }
}

IScreenCapture::CaptureQuality ScreenShareManager::quality() const
{
    QMutexLocker locker(&d->mutex);
    return d->quality;
}

void ScreenShareManager::setFrameRate(int fps)
{
    QMutexLocker locker(&d->mutex);
    if (fps > 0 && fps <= d->maxFrameRate && d->frameRate != fps) {
        d->frameRate = fps;
        d->configuration["frameRate"] = fps;
        
        if (d->currentCapture) {
            d->currentCapture->setFrameRate(fps);
        }
        if (d->captureEngine) {
            d->captureEngine->setTargetFrameRate(fps);
        }
    }
}

int ScreenShareManager::frameRate() const
{
    QMutexLocker locker(&d->mutex);
    return d->frameRate;
}

void ScreenShareManager::setBitrate(int kbps)
{
    QMutexLocker locker(&d->mutex);
    if (kbps > 0 && kbps <= d->maxBitrate && d->bitrate != kbps) {
        d->bitrate = kbps;
        d->configuration["bitrate"] = kbps;
    }
}

int ScreenShareManager::bitrate() const
{
    QMutexLocker locker(&d->mutex);
    return d->bitrate;
}

QVariantMap ScreenShareManager::getStatistics() const
{
    QMutexLocker locker(&d->mutex);
    return d->statistics;
}

double ScreenShareManager::getCurrentFPS() const
{
    QMutexLocker locker(&d->mutex);
    return d->currentFPS;
}

int ScreenShareManager::getCurrentBitrate() const
{
    QMutexLocker locker(&d->mutex);
    return d->currentBitrate;
}

qint64 ScreenShareManager::getTotalFrames() const
{
    QMutexLocker locker(&d->mutex);
    return d->totalFrames;
}

void ScreenShareManager::setAutoQualityAdjustment(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    if (d->autoQualityAdjustment != enabled) {
        d->autoQualityAdjustment = enabled;
        
        if (enabled) {
            d->qualityAdjustmentTimer->start(5000);
        } else {
            d->qualityAdjustmentTimer->stop();
        }
    }
}

bool ScreenShareManager::isAutoQualityAdjustmentEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->autoQualityAdjustment;
}

void ScreenShareManager::setMaxFrameRate(int maxFps)
{
    QMutexLocker locker(&d->mutex);
    if (maxFps > 0) {
        d->maxFrameRate = maxFps;
        if (d->frameRate > maxFps) {
            setFrameRate(maxFps);
        }
    }
}

int ScreenShareManager::maxFrameRate() const
{
    QMutexLocker locker(&d->mutex);
    return d->maxFrameRate;
}

void ScreenShareManager::setMaxBitrate(int maxKbps)
{
    QMutexLocker locker(&d->mutex);
    if (maxKbps > 0) {
        d->maxBitrate = maxKbps;
        if (d->bitrate > maxKbps) {
            setBitrate(maxKbps);
        }
    }
}

int ScreenShareManager::maxBitrate() const
{
    QMutexLocker locker(&d->mutex);
    return d->maxBitrate;
}

void ScreenShareManager::refreshAvailableSources()
{
    QMutexLocker locker(&d->mutex);
    
    // 刷新屏幕列表
    d->availableScreenIds.clear();
    const auto screens = QApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        d->availableScreenIds.append(QString("screen_%1").arg(i));
    }
    
    // 刷新窗口列表 (简化实现)
    d->availableWindowIds.clear();
    d->availableWindowIds.append("window_desktop");
    
    emit availableSourcesUpdated();
}

void ScreenShareManager::optimizePerformance()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->captureEngine) {
        d->captureEngine->optimizePerformance();
    }
    
    // 根据系统性能调整设置
    adjustQualityBasedOnPerformance();
}

void ScreenShareManager::resetStatistics()
{
    QMutexLocker locker(&d->mutex);
    
    d->totalFrames = 0;
    d->currentFPS = 0.0;
    d->currentBitrate = 0;
    d->statistics.clear();
    
    if (d->captureEngine) {
        d->captureEngine->resetStatistics();
    }
}

void ScreenShareManager::onCaptureStatusChanged(IScreenCapture::CaptureStatus status)
{
    Q_UNUSED(status)
    // 处理捕获状态变化
}

void ScreenShareManager::onFrameCaptured(const QPixmap& frame)
{
    Q_UNUSED(frame)
    d->totalFrames++;
}

void ScreenShareManager::onCaptureError(const QString& error)
{
    emitError(error);
}

void ScreenShareManager::onStatisticsTimer()
{
    updateStatistics();
}

void ScreenShareManager::onQualityAdjustmentTimer()
{
    if (d->autoQualityAdjustment) {
        adjustQualityBasedOnPerformance();
    }
}

void ScreenShareManager::initializeCapture()
{
    cleanupCapture();
    
    QString sourceType = d->configuration.value("sourceType", "screen").toString();
    QString sourceId = d->configuration.value("sourceId").toString();
    
    if (sourceType == "screen") {
        d->currentCapture = new ScreenCapture(this);
    } else if (sourceType == "window") {
        d->currentCapture = new WindowCapture(this);
    } else {
        d->currentCapture = new ScreenCapture(this); // 默认屏幕捕获
    }
    
    if (d->currentCapture) {
        // 配置捕获对象
        d->currentCapture->setCaptureQuality(d->quality);
        d->currentCapture->setFrameRate(d->frameRate);
        
        // 连接信号
        connect(d->currentCapture, &IScreenCapture::statusChanged,
                this, &ScreenShareManager::onCaptureStatusChanged);
        connect(d->currentCapture, &IScreenCapture::frameCaptured,
                this, &ScreenShareManager::onFrameCaptured);
        connect(d->currentCapture, &IScreenCapture::captureError,
                this, &ScreenShareManager::onCaptureError);
        
        // 初始化捕获
        if (!d->currentCapture->initialize()) {
            cleanupCapture();
            throw std::runtime_error("Failed to initialize capture");
        }
    }
}

void ScreenShareManager::cleanupCapture()
{
    if (d->currentCapture) {
        d->currentCapture->stopCapture();
        d->currentCapture->deleteLater();
        d->currentCapture = nullptr;
    }
}

void ScreenShareManager::updateStatistics()
{
    QMutexLocker locker(&d->mutex);
    
    // 更新FPS
    if (d->captureEngine) {
        d->currentFPS = d->captureEngine->currentFPS();
    }
    
    // 更新统计信息
    d->statistics["totalFrames"] = d->totalFrames;
    d->statistics["currentFPS"] = d->currentFPS;
    d->statistics["currentBitrate"] = d->currentBitrate;
    d->statistics["quality"] = static_cast<int>(d->quality);
    d->statistics["frameRate"] = d->frameRate;
    d->statistics["bitrate"] = d->bitrate;
    
    emit statisticsUpdated(d->statistics);
}

void ScreenShareManager::adjustQualityBasedOnPerformance()
{
    // 简化的性能调整逻辑
    if (d->currentFPS < d->frameRate * 0.8) {
        // FPS过低，降低质量
        if (d->quality > IScreenCapture::LowQuality) {
            setQuality(static_cast<IScreenCapture::CaptureQuality>(d->quality - 1));
        }
    } else if (d->currentFPS > d->frameRate * 0.95) {
        // FPS良好，可以提高质量
        if (d->quality < IScreenCapture::UltraQuality) {
            setQuality(static_cast<IScreenCapture::CaptureQuality>(d->quality + 1));
        }
    }
}

bool ScreenShareManager::validateShareConfiguration(const QVariantMap& config) const
{
    // 基础配置验证
    if (config.contains("frameRate")) {
        int fps = config["frameRate"].toInt();
        if (fps <= 0 || fps > d->maxFrameRate) {
            return false;
        }
    }
    
    if (config.contains("bitrate")) {
        int kbps = config["bitrate"].toInt();
        if (kbps <= 0 || kbps > d->maxBitrate) {
            return false;
        }
    }
    
    return true;
}

void ScreenShareManager::updateStatus(ManagerStatus newStatus)
{
    if (d->status != newStatus) {
        d->status = newStatus;
        emit statusChanged(newStatus);
    }
}

void ScreenShareManager::emitError(const QString& error)
{
    qWarning() << "ScreenShareManager error:" << error;
    emit shareError(error);
}

bool ScreenShareManager::showScreenSelectionDialog()
{
    // Show screen selection dialog and return true if user selected a screen
    // This is a placeholder implementation
    return true;
}

QVideoWidget* ScreenShareManager::localScreenShareWidget() const
{
    // Return the local screen share widget
    // This is a placeholder implementation
    return nullptr;
}