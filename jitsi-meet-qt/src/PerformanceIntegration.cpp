#include "PerformanceIntegration.h"
#include "PerformanceManager.h"
#include "MemoryLeakDetector.h"
#include "NetworkOptimizer.h"
#include "MediaPerformanceOptimizer.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

PerformanceIntegration* PerformanceIntegration::s_instance = nullptr;

PerformanceIntegration::PerformanceIntegration(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
    s_instance = this;
    qDebug() << "PerformanceIntegration: Created";
}

PerformanceIntegration::~PerformanceIntegration()
{
    shutdown();
    s_instance = nullptr;
}

PerformanceIntegration* PerformanceIntegration::instance()
{
    return s_instance;
}

void PerformanceIntegration::initialize()
{
    if (m_initialized) {
        return;
    }
    
    qDebug() << "PerformanceIntegration: Initializing performance optimization system";
    
    // 创建所有性能优化器
    m_performanceManager = std::make_unique<PerformanceManager>(this);
    m_memoryLeakDetector = std::make_unique<MemoryLeakDetector>(this);
    m_networkOptimizer = std::make_unique<NetworkOptimizer>(this);
    m_mediaPerformanceOptimizer = std::make_unique<MediaPerformanceOptimizer>(this);
    
    // 连接信号
    connectSignals();
    
    // 应用启动优化
    applyStartupOptimizations();
    
    m_initialized = true;
    
    qDebug() << "PerformanceIntegration: Initialization completed";
    emit performanceOptimized("Performance optimization system initialized");
}

void PerformanceIntegration::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "PerformanceIntegration: Shutting down performance optimization system";
    
    // 停止所有监控
    if (m_performanceManager) {
        m_performanceManager->stopMemoryMonitoring();
    }
    
    if (m_memoryLeakDetector) {
        m_memoryLeakDetector->stopLeakDetection();
    }
    
    if (m_networkOptimizer) {
        m_networkOptimizer->stopNetworkMonitoring();
    }
    
    if (m_mediaPerformanceOptimizer) {
        m_mediaPerformanceOptimizer->stopPerformanceMonitoring();
    }
    
    // 重置所有优化器
    m_mediaPerformanceOptimizer.reset();
    m_networkOptimizer.reset();
    m_memoryLeakDetector.reset();
    m_performanceManager.reset();
    
    m_initialized = false;
    
    qDebug() << "PerformanceIntegration: Shutdown completed";
}

void PerformanceIntegration::optimizeForConference(int participantCount)
{
    if (!m_initialized) {
        qWarning() << "PerformanceIntegration: Not initialized, cannot optimize for conference";
        return;
    }
    
    qDebug() << "PerformanceIntegration: Optimizing for conference with" << participantCount << "participants";
    
    // 应用所有相关的优化
    m_performanceManager->optimizeForLargeConference(participantCount);
    m_networkOptimizer->optimizeForParticipantCount(participantCount);
    m_mediaPerformanceOptimizer->optimizeForParticipantCount(participantCount);
    
    // 根据参与者数量调整监控频率
    if (participantCount > 20) {
        // 大型会议 - 降低监控频率以节省资源
        m_memoryLeakDetector->setLeakDetectionInterval(60); // 1分钟
    } else {
        // 小型会议 - 正常监控频率
        m_memoryLeakDetector->setLeakDetectionInterval(30); // 30秒
    }
    
    QString optimizationDesc = QString("Optimized for %1 participants").arg(participantCount);
    emit performanceOptimized(optimizationDesc);
}

void PerformanceIntegration::optimizeForNetworkQuality(int latencyMs, double bandwidthMbps)
{
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "PerformanceIntegration: Optimizing for network quality - Latency:" 
             << latencyMs << "ms, Bandwidth:" << bandwidthMbps << "Mbps";
    
    // 根据网络质量调整媒体设置
    if (latencyMs > 200 || bandwidthMbps < 1.0) {
        // 网络质量差 - 降低媒体质量
        m_mediaPerformanceOptimizer->setVideoQuality(MediaPerformanceOptimizer::VideoQuality::Low);
        m_mediaPerformanceOptimizer->setAudioQuality(MediaPerformanceOptimizer::AudioQuality::Standard);
    } else if (latencyMs > 100 || bandwidthMbps < 5.0) {
        // 网络质量中等
        m_mediaPerformanceOptimizer->setVideoQuality(MediaPerformanceOptimizer::VideoQuality::Medium);
        m_mediaPerformanceOptimizer->setAudioQuality(MediaPerformanceOptimizer::AudioQuality::High);
    } else {
        // 网络质量好
        m_mediaPerformanceOptimizer->setVideoQuality(MediaPerformanceOptimizer::VideoQuality::High);
        m_mediaPerformanceOptimizer->setAudioQuality(MediaPerformanceOptimizer::AudioQuality::Studio);
    }
    
    // 启用自适应比特率
    m_networkOptimizer->enableAdaptiveBitrate(true);
    
    QString optimizationDesc = QString("Optimized for network: %1ms latency, %2 Mbps")
                              .arg(latencyMs).arg(bandwidthMbps, 0, 'f', 1);
    emit performanceOptimized(optimizationDesc);
}

void PerformanceIntegration::startApplicationOptimization()
{
    if (!m_initialized) {
        initialize();
    }
    
    qDebug() << "PerformanceIntegration: Starting application optimization";
    
    // 开始启动时间跟踪
    m_performanceManager->startStartupTimer();
    
    // 预加载资源
    m_performanceManager->preloadResources();
    
    // 启用延迟加载
    m_performanceManager->enableLazyLoading(true);
    
    // 开始内存跟踪
    m_memoryLeakDetector->setTrackingEnabled(true);
}

void PerformanceIntegration::endApplicationOptimization()
{
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "PerformanceIntegration: Ending application optimization";
    
    // 结束启动时间跟踪
    m_performanceManager->endStartupTimer();
    
    // 开始所有监控
    m_performanceManager->startMemoryMonitoring();
    m_memoryLeakDetector->startLeakDetection();
    m_networkOptimizer->startNetworkMonitoring();
    m_mediaPerformanceOptimizer->startPerformanceMonitoring();
    
    auto startupTime = m_performanceManager->getStartupTime();
    QString optimizationDesc = QString("Application startup completed in %1ms").arg(startupTime.count());
    emit performanceOptimized(optimizationDesc);
}

PerformanceManager* PerformanceIntegration::performanceManager() const
{
    return m_performanceManager.get();
}

MemoryLeakDetector* PerformanceIntegration::memoryLeakDetector() const
{
    return m_memoryLeakDetector.get();
}

NetworkOptimizer* PerformanceIntegration::networkOptimizer() const
{
    return m_networkOptimizer.get();
}

MediaPerformanceOptimizer* PerformanceIntegration::mediaPerformanceOptimizer() const
{
    return m_mediaPerformanceOptimizer.get();
}

QString PerformanceIntegration::getPerformanceReport() const
{
    if (!m_initialized) {
        return "Performance system not initialized";
    }
    
    QString report;
    QTextStream stream(&report);
    
    stream << "=== Jitsi Meet Qt Performance Report ===\n";
    stream << "Generated: " << QDateTime::currentDateTime().toString() << "\n\n";
    
    // 启动性能
    auto startupTime = m_performanceManager->getStartupTime();
    stream << "Startup Performance:\n";
    stream << "  Startup Time: " << startupTime.count() << " ms\n\n";
    
    // 内存使用
    auto memoryStats = m_memoryLeakDetector->getMemoryStats();
    stream << "Memory Usage:\n";
    stream << "  Current Memory: " << (m_performanceManager->getCurrentMemoryUsage() / (1024*1024)) << " MB\n";
    stream << "  Peak Memory: " << (m_performanceManager->getPeakMemoryUsage() / (1024*1024)) << " MB\n";
    stream << "  Total Allocations: " << memoryStats.totalAllocations << "\n";
    stream << "  Total Deallocations: " << memoryStats.totalDeallocations << "\n";
    stream << "  Current Allocations: " << memoryStats.currentAllocations << "\n\n";
    
    // 网络性能
    auto networkMetrics = m_networkOptimizer->getCurrentMetrics();
    stream << "Network Performance:\n";
    stream << "  Latency: " << networkMetrics.latency.count() << " ms\n";
    stream << "  Bandwidth: " << networkMetrics.bandwidthMbps << " Mbps\n";
    stream << "  Connection Quality: " << static_cast<int>(networkMetrics.quality) << "\n\n";
    
    // 媒体性能
    auto mediaMetrics = m_mediaPerformanceOptimizer->getCurrentMetrics();
    stream << "Media Performance:\n";
    stream << "  Video Encoding Time: " << mediaMetrics.videoEncodingTime << " ms\n";
    stream << "  Audio Encoding Time: " << mediaMetrics.audioEncodingTime << " ms\n";
    stream << "  Dropped Video Frames: " << mediaMetrics.droppedVideoFrames << "\n";
    stream << "  Dropped Audio Frames: " << mediaMetrics.droppedAudioFrames << "\n";
    stream << "  CPU Usage: " << mediaMetrics.cpuUsage << "%\n\n";
    
    // 当前设置
    auto videoSettings = m_mediaPerformanceOptimizer->getVideoSettings();
    auto audioSettings = m_mediaPerformanceOptimizer->getAudioSettings();
    stream << "Current Media Settings:\n";
    stream << "  Video: " << videoSettings.width << "x" << videoSettings.height 
           << " @" << videoSettings.frameRate << "fps, " << videoSettings.bitrate << "kbps\n";
    stream << "  Audio: " << audioSettings.sampleRate << "Hz, " 
           << audioSettings.channels << "ch, " << audioSettings.bitrate << "kbps\n";
    
    return report;
}

void PerformanceIntegration::exportPerformanceData(const QString& filePath) const
{
    QString report = getPerformanceReport();
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << report;
        qDebug() << "PerformanceIntegration: Performance report exported to" << filePath;
    } else {
        qWarning() << "PerformanceIntegration: Failed to export performance report to" << filePath;
    }
}

void PerformanceIntegration::onMemoryWarning(size_t currentUsage, size_t threshold)
{
    QString warning = QString("Memory usage warning: %1 MB (threshold: %2 MB)")
                     .arg(currentUsage / (1024*1024))
                     .arg(threshold / (1024*1024));
    
    qWarning() << "PerformanceIntegration:" << warning;
    emit performanceWarning(warning);
    
    // 触发内存清理
    m_memoryLeakDetector->forceGarbageCollection();
}

void PerformanceIntegration::onNetworkQualityChanged(int quality)
{
    qDebug() << "PerformanceIntegration: Network quality changed to" << quality;
    
    // 根据网络质量自动调整媒体设置
    if (m_mediaPerformanceOptimizer) {
        auto networkQuality = static_cast<NetworkOptimizer::ConnectionQuality>(quality);
        
        switch (networkQuality) {
            case NetworkOptimizer::ConnectionQuality::Excellent:
                m_mediaPerformanceOptimizer->setVideoQuality(MediaPerformanceOptimizer::VideoQuality::Ultra);
                break;
            case NetworkOptimizer::ConnectionQuality::Good:
                m_mediaPerformanceOptimizer->setVideoQuality(MediaPerformanceOptimizer::VideoQuality::High);
                break;
            case NetworkOptimizer::ConnectionQuality::Fair:
                m_mediaPerformanceOptimizer->setVideoQuality(MediaPerformanceOptimizer::VideoQuality::Medium);
                break;
            case NetworkOptimizer::ConnectionQuality::Poor:
                m_mediaPerformanceOptimizer->setVideoQuality(MediaPerformanceOptimizer::VideoQuality::Low);
                break;
        }
    }
}

void PerformanceIntegration::onPerformanceWarning(int metricType, double value)
{
    QString metricName;
    switch (metricType) {
        case 0: metricName = "Startup Time"; break;
        case 1: metricName = "Memory Usage"; break;
        case 2: metricName = "Network Latency"; break;
        case 3: metricName = "Video Frame Rate"; break;
        case 4: metricName = "Audio Latency"; break;
        case 5: metricName = "CPU Usage"; break;
        default: metricName = "Unknown"; break;
    }
    
    QString warning = QString("Performance warning: %1 = %2").arg(metricName).arg(value);
    qWarning() << "PerformanceIntegration:" << warning;
    emit performanceWarning(warning);
}

void PerformanceIntegration::onMemoryLeakDetected(int leakCount, size_t totalBytes)
{
    QString warning = QString("Memory leak detected: %1 leaks, %2 bytes total")
                     .arg(leakCount).arg(totalBytes);
    
    qWarning() << "PerformanceIntegration:" << warning;
    emit memoryLeakDetected(leakCount, totalBytes);
    
    // 强制清理
    m_memoryLeakDetector->cleanupUnusedResources();
}

void PerformanceIntegration::connectSignals()
{
    // 连接性能管理器信号
    if (m_performanceManager) {
        connect(m_performanceManager.get(), &PerformanceManager::memoryWarning,
                this, &PerformanceIntegration::onMemoryWarning);
        connect(m_performanceManager.get(), &PerformanceManager::performanceWarning,
                this, &PerformanceIntegration::onPerformanceWarning);
    }
    
    // 连接内存泄漏检测器信号
    if (m_memoryLeakDetector) {
        connect(m_memoryLeakDetector.get(), &MemoryLeakDetector::memoryLeakDetected,
                this, &PerformanceIntegration::onMemoryLeakDetected);
    }
    
    // 连接网络优化器信号
    if (m_networkOptimizer) {
        connect(m_networkOptimizer.get(), &NetworkOptimizer::networkQualityChanged,
                this, &PerformanceIntegration::onNetworkQualityChanged);
    }
}

void PerformanceIntegration::applyStartupOptimizations()
{
    qDebug() << "PerformanceIntegration: Applying startup optimizations";
    
    // 启用所有启动优化
    if (m_performanceManager) {
        m_performanceManager->enableLazyLoading(true);
    }
    
    if (m_networkOptimizer) {
        NetworkOptimizer::OptimizationSettings settings;
        settings.compressionEnabled = true;
        settings.connectionPoolingEnabled = true;
        settings.adaptiveBitrateEnabled = true;
        m_networkOptimizer->setOptimizationSettings(settings);
    }
    
    if (m_mediaPerformanceOptimizer) {
        m_mediaPerformanceOptimizer->enableAdaptiveQuality(true);
    }
}