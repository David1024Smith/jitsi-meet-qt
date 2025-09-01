#include "../include/CaptureEngine.h"
#include "../interfaces/IScreenCapture.h"
#include "../encoding/VideoEncoder.h"
#include "../encoding/FrameProcessor.h"
#include <QTimer>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QDebug>
#include <QThread>
#include <QApplication>

class CaptureEngine::Private
{
public:
    Private()
        : status(CaptureEngine::Stopped)
        , initialized(false)
        , targetFrameRate(30)
        , performanceMode(CaptureEngine::Balanced)
        , qualityAdjustmentEnabled(true)
        , adaptiveFrameRateEnabled(true)
        , captureSource(nullptr)
        , videoEncoder(nullptr)
        , frameProcessor(nullptr)
        , captureTimer(nullptr)
        , statisticsTimer(nullptr)
        , qualityAdjustmentTimer(nullptr)
        , frameCount(0)
        , currentFPS(0.0)
        , totalProcessingTime(0)
        , lastFPSCalculation(0)
        , framesInLastSecond(0)
    {
    }

    CaptureEngine::EngineStatus status;
    bool initialized;
    int targetFrameRate;
    CaptureEngine::PerformanceMode performanceMode;
    bool qualityAdjustmentEnabled;
    bool adaptiveFrameRateEnabled;
    
    IScreenCapture* captureSource;
    VideoEncoder* videoEncoder;
    FrameProcessor* frameProcessor;
    
    QTimer* captureTimer;
    QTimer* statisticsTimer;
    QTimer* qualityAdjustmentTimer;
    
    // 统计信息
    int frameCount;
    double currentFPS;
    qint64 totalProcessingTime;
    qint64 lastFPSCalculation;
    int framesInLastSecond;
    
    QElapsedTimer processingTimer;
    QElapsedTimer fpsTimer;
    
    mutable QMutex mutex;
};

CaptureEngine::CaptureEngine(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->fpsTimer.start();
    d->lastFPSCalculation = d->fpsTimer.elapsed();
}

CaptureEngine::~CaptureEngine()
{
    shutdown();
    delete d;
}

bool CaptureEngine::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        return true;
    }
    
    try {
        // 初始化定时器
        initializeTimers();
        
        // 创建帧处理器
        if (!d->frameProcessor) {
            d->frameProcessor = new FrameProcessor(this);
        }
        
        // 创建视频编码器
        if (!d->videoEncoder) {
            d->videoEncoder = new VideoEncoder(this);
        }
        
        d->initialized = true;
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "CaptureEngine initialization failed:" << e.what();
        return false;
    }
}

void CaptureEngine::shutdown()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        return;
    }
    
    // 停止引擎
    if (isActive()) {
        stop();
    }
    
    // 清理定时器
    cleanupTimers();
    
    // 清理组件
    d->captureSource = nullptr; // 不删除，由外部管理
    
    if (d->videoEncoder) {
        d->videoEncoder->deleteLater();
        d->videoEncoder = nullptr;
    }
    
    if (d->frameProcessor) {
        d->frameProcessor->deleteLater();
        d->frameProcessor = nullptr;
    }
    
    d->initialized = false;
}

bool CaptureEngine::start()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "CaptureEngine not initialized";
        return false;
    }
    
    if (d->status == Running) {
        return true;
    }
    
    if (!d->captureSource) {
        qWarning() << "No capture source set";
        return false;
    }
    
    updateStatus(Starting);
    
    try {
        // 启动捕获源
        if (!d->captureSource->startCapture()) {
            throw std::runtime_error("Failed to start capture source");
        }
        
        // 连接捕获源信号
        connect(d->captureSource, &IScreenCapture::frameCaptured,
                this, &CaptureEngine::onFrameCaptured, Qt::UniqueConnection);
        connect(d->captureSource, &IScreenCapture::captureError,
                this, &CaptureEngine::onCaptureError, Qt::UniqueConnection);
        
        // 启动定时器
        int interval = 1000 / d->targetFrameRate;
        d->captureTimer->start(interval);
        d->statisticsTimer->start(1000); // 每秒更新统计
        
        if (d->qualityAdjustmentEnabled) {
            d->qualityAdjustmentTimer->start(5000); // 每5秒调整质量
        }
        
        // 重置统计
        resetStatistics();
        
        updateStatus(Running);
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to start CaptureEngine:" << e.what();
        updateStatus(Error);
        return false;
    }
}

void CaptureEngine::stop()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status == Stopped) {
        return;
    }
    
    updateStatus(Stopping);
    
    // 停止定时器
    if (d->captureTimer) {
        d->captureTimer->stop();
    }
    if (d->statisticsTimer) {
        d->statisticsTimer->stop();
    }
    if (d->qualityAdjustmentTimer) {
        d->qualityAdjustmentTimer->stop();
    }
    
    // 断开捕获源信号
    if (d->captureSource) {
        disconnect(d->captureSource, nullptr, this, nullptr);
        d->captureSource->stopCapture();
    }
    
    updateStatus(Stopped);
}

void CaptureEngine::pause()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Running) {
        return;
    }
    
    updateStatus(Pausing);
    
    // 暂停定时器
    if (d->captureTimer) {
        d->captureTimer->stop();
    }
    
    // 暂停捕获源
    if (d->captureSource) {
        d->captureSource->pauseCapture();
    }
    
    updateStatus(Paused);
}

void CaptureEngine::resume()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Paused) {
        return;
    }
    
    // 恢复捕获源
    if (d->captureSource) {
        d->captureSource->resumeCapture();
    }
    
    // 恢复定时器
    if (d->captureTimer) {
        int interval = 1000 / d->targetFrameRate;
        d->captureTimer->start(interval);
    }
    
    updateStatus(Running);
}

CaptureEngine::EngineStatus CaptureEngine::status() const
{
    QMutexLocker locker(&d->mutex);
    return d->status;
}

bool CaptureEngine::isActive() const
{
    QMutexLocker locker(&d->mutex);
    return d->status == Running || d->status == Paused;
}

bool CaptureEngine::isInitialized() const
{
    QMutexLocker locker(&d->mutex);
    return d->initialized;
}

bool CaptureEngine::isPaused() const
{
    QMutexLocker locker(&d->mutex);
    return d->status == Paused;
}

void CaptureEngine::setCaptureSource(IScreenCapture* capture)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->captureSource != capture) {
        // 如果正在运行，先停止
        bool wasRunning = isActive();
        if (wasRunning) {
            stop();
        }
        
        d->captureSource = capture;
        
        // 如果之前在运行，重新启动
        if (wasRunning && capture) {
            start();
        }
    }
}

IScreenCapture* CaptureEngine::captureSource() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureSource;
}

void CaptureEngine::setTargetFrameRate(int fps)
{
    QMutexLocker locker(&d->mutex);
    
    if (fps > 0 && fps <= 120 && d->targetFrameRate != fps) {
        d->targetFrameRate = fps;
        
        // 更新定时器间隔
        if (d->captureTimer && d->captureTimer->isActive()) {
            int interval = 1000 / fps;
            d->captureTimer->start(interval);
        }
        
        // 更新捕获源帧率
        if (d->captureSource) {
            d->captureSource->setFrameRate(fps);
        }
    }
}

int CaptureEngine::targetFrameRate() const
{
    QMutexLocker locker(&d->mutex);
    return d->targetFrameRate;
}

void CaptureEngine::setPerformanceMode(PerformanceMode mode)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->performanceMode != mode) {
        d->performanceMode = mode;
        
        // 根据性能模式调整设置
        switch (mode) {
        case PowerSaving:
            setTargetFrameRate(15);
            break;
        case Balanced:
            setTargetFrameRate(30);
            break;
        case Performance:
            setTargetFrameRate(60);
            break;
        case UltraPerformance:
            setTargetFrameRate(120);
            break;
        }
    }
}

CaptureEngine::PerformanceMode CaptureEngine::performanceMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->performanceMode;
}

void CaptureEngine::setVideoEncoder(VideoEncoder* encoder)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->videoEncoder != encoder) {
        if (d->videoEncoder) {
            d->videoEncoder->deleteLater();
        }
        d->videoEncoder = encoder;
        if (encoder) {
            encoder->setParent(this);
        }
    }
}

VideoEncoder* CaptureEngine::videoEncoder() const
{
    QMutexLocker locker(&d->mutex);
    return d->videoEncoder;
}

void CaptureEngine::setFrameProcessor(FrameProcessor* processor)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->frameProcessor != processor) {
        if (d->frameProcessor) {
            d->frameProcessor->deleteLater();
        }
        d->frameProcessor = processor;
        if (processor) {
            processor->setParent(this);
        }
    }
}

FrameProcessor* CaptureEngine::frameProcessor() const
{
    QMutexLocker locker(&d->mutex);
    return d->frameProcessor;
}

double CaptureEngine::currentFPS() const
{
    QMutexLocker locker(&d->mutex);
    return d->currentFPS;
}

int CaptureEngine::frameCount() const
{
    QMutexLocker locker(&d->mutex);
    return d->frameCount;
}

qint64 CaptureEngine::totalProcessingTime() const
{
    QMutexLocker locker(&d->mutex);
    return d->totalProcessingTime;
}

double CaptureEngine::averageProcessingTime() const
{
    QMutexLocker locker(&d->mutex);
    if (d->frameCount > 0) {
        return static_cast<double>(d->totalProcessingTime) / d->frameCount;
    }
    return 0.0;
}

QVariantMap CaptureEngine::getPerformanceMetrics() const
{
    QMutexLocker locker(&d->mutex);
    
    QVariantMap metrics;
    metrics["frameCount"] = d->frameCount;
    metrics["currentFPS"] = d->currentFPS;
    metrics["targetFPS"] = d->targetFrameRate;
    metrics["totalProcessingTime"] = d->totalProcessingTime;
    metrics["averageProcessingTime"] = averageProcessingTime();
    metrics["performanceMode"] = static_cast<int>(d->performanceMode);
    metrics["status"] = static_cast<int>(d->status);
    
    return metrics;
}

void CaptureEngine::setQualityAdjustmentEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->qualityAdjustmentEnabled != enabled) {
        d->qualityAdjustmentEnabled = enabled;
        
        if (enabled && isActive()) {
            d->qualityAdjustmentTimer->start(5000);
        } else {
            d->qualityAdjustmentTimer->stop();
        }
    }
}

bool CaptureEngine::isQualityAdjustmentEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->qualityAdjustmentEnabled;
}

void CaptureEngine::setAdaptiveFrameRate(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    d->adaptiveFrameRateEnabled = enabled;
}

bool CaptureEngine::isAdaptiveFrameRateEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->adaptiveFrameRateEnabled;
}

void CaptureEngine::captureFrame()
{
    if (d->captureSource && d->captureSource->isCapturing()) {
        QPixmap frame = d->captureSource->captureFrame();
        if (!frame.isNull()) {
            processFrame(frame);
        }
    }
}

void CaptureEngine::resetStatistics()
{
    QMutexLocker locker(&d->mutex);
    
    d->frameCount = 0;
    d->currentFPS = 0.0;
    d->totalProcessingTime = 0;
    d->framesInLastSecond = 0;
    d->lastFPSCalculation = d->fpsTimer.elapsed();
    
    emit frameCountChanged(0);
    emit fpsChanged(0.0);
}

void CaptureEngine::optimizePerformance()
{
    QMutexLocker locker(&d->mutex);
    
    // 根据当前性能调整设置
    if (d->currentFPS < d->targetFrameRate * 0.8) {
        // 性能不足，降低设置
        if (d->performanceMode > PowerSaving) {
            setPerformanceMode(static_cast<PerformanceMode>(d->performanceMode - 1));
            emit performanceWarning("Performance degraded, switching to lower performance mode");
        }
    } else if (d->currentFPS > d->targetFrameRate * 0.95) {
        // 性能良好，可以提升设置
        if (d->performanceMode < UltraPerformance) {
            setPerformanceMode(static_cast<PerformanceMode>(d->performanceMode + 1));
        }
    }
}

void CaptureEngine::onCaptureTimer()
{
    captureFrame();
}

void CaptureEngine::onFrameCaptured(const QPixmap& frame)
{
    processFrame(frame);
}

void CaptureEngine::onCaptureError(const QString& error)
{
    emit engineError(error);
}

void CaptureEngine::onStatisticsTimer()
{
    updateStatistics();
}

void CaptureEngine::onQualityAdjustmentTimer()
{
    if (d->qualityAdjustmentEnabled) {
        adjustQuality();
    }
}

void CaptureEngine::initializeTimers()
{
    if (!d->captureTimer) {
        d->captureTimer = new QTimer(this);
        connect(d->captureTimer, &QTimer::timeout,
                this, &CaptureEngine::onCaptureTimer);
    }
    
    if (!d->statisticsTimer) {
        d->statisticsTimer = new QTimer(this);
        connect(d->statisticsTimer, &QTimer::timeout,
                this, &CaptureEngine::onStatisticsTimer);
    }
    
    if (!d->qualityAdjustmentTimer) {
        d->qualityAdjustmentTimer = new QTimer(this);
        connect(d->qualityAdjustmentTimer, &QTimer::timeout,
                this, &CaptureEngine::onQualityAdjustmentTimer);
    }
}

void CaptureEngine::cleanupTimers()
{
    if (d->captureTimer) {
        d->captureTimer->stop();
        d->captureTimer->deleteLater();
        d->captureTimer = nullptr;
    }
    
    if (d->statisticsTimer) {
        d->statisticsTimer->stop();
        d->statisticsTimer->deleteLater();
        d->statisticsTimer = nullptr;
    }
    
    if (d->qualityAdjustmentTimer) {
        d->qualityAdjustmentTimer->stop();
        d->qualityAdjustmentTimer->deleteLater();
        d->qualityAdjustmentTimer = nullptr;
    }
}

void CaptureEngine::processFrame(const QPixmap& frame)
{
    d->processingTimer.start();
    
    try {
        QPixmap processedFrame = frame;
        
        // 帧处理
        if (d->frameProcessor) {
            processedFrame = d->frameProcessor->processFrame(frame);
        }
        
        // 编码
        if (d->videoEncoder) {
            QByteArray encodedData = d->videoEncoder->encodeFrameRaw(processedFrame);
            if (!encodedData.isEmpty()) {
                emit encodedDataReady(encodedData);
            }
        }
        
        qint64 processingTime = d->processingTimer.elapsed();
        d->totalProcessingTime += processingTime;
        d->frameCount++;
        d->framesInLastSecond++;
        
        emit frameProcessed(processedFrame, processingTime);
        emit frameCountChanged(d->frameCount);
        
    } catch (const std::exception& e) {
        emit engineError(QString("Frame processing failed: %1").arg(e.what()));
    }
}

void CaptureEngine::updateStatistics()
{
    calculateFPS();
}

void CaptureEngine::adjustQuality()
{
    // 简化的质量调整逻辑
    if (d->captureSource && d->adaptiveFrameRateEnabled) {
        if (d->currentFPS < d->targetFrameRate * 0.8) {
            // 降低质量以提高性能
            auto currentQuality = d->captureSource->captureQuality();
            if (currentQuality > IScreenCapture::LowQuality) {
                auto newQuality = static_cast<IScreenCapture::CaptureQuality>(currentQuality - 1);
                d->captureSource->setCaptureQuality(newQuality);
            }
        } else if (d->currentFPS > d->targetFrameRate * 0.95) {
            // 提高质量
            auto currentQuality = d->captureSource->captureQuality();
            if (currentQuality < IScreenCapture::UltraQuality) {
                auto newQuality = static_cast<IScreenCapture::CaptureQuality>(currentQuality + 1);
                d->captureSource->setCaptureQuality(newQuality);
            }
        }
    }
}

void CaptureEngine::updateStatus(EngineStatus newStatus)
{
    if (d->status != newStatus) {
        d->status = newStatus;
        emit statusChanged(newStatus);
        
        bool active = (newStatus == Running || newStatus == Paused);
        emit activeChanged(active);
    }
}

void CaptureEngine::calculateFPS()
{
    qint64 currentTime = d->fpsTimer.elapsed();
    qint64 timeDiff = currentTime - d->lastFPSCalculation;
    
    if (timeDiff >= 1000) { // 每秒计算一次
        d->currentFPS = (d->framesInLastSecond * 1000.0) / timeDiff;
        d->framesInLastSecond = 0;
        d->lastFPSCalculation = currentTime;
        
        emit fpsChanged(d->currentFPS);
    }
}