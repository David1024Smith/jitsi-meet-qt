#include "MediaPerformanceOptimizer.h"
#include <QDebug>
#include <algorithm>
#include <numeric>

MediaPerformanceOptimizer* MediaPerformanceOptimizer::s_instance = nullptr;

MediaPerformanceOptimizer::MediaPerformanceOptimizer(QObject *parent)
    : QObject(parent)
    , m_monitoringTimer(new QTimer(this))
    , m_currentVideoQuality(VideoQuality::High)
    , m_currentAudioQuality(AudioQuality::High)
    , m_adaptiveQualityEnabled(true)
    , m_maxCpuUsage(80.0)
    , m_monitoringActive(false)
{
    s_instance = this;
    
    // 设置监控定时器
    m_monitoringTimer->setInterval(2000); // 每2秒检查一次
    connect(m_monitoringTimer, &QTimer::timeout, this, &MediaPerformanceOptimizer::updatePerformanceMetrics);
    
    // 初始化默认设置
    applyVideoQuality(m_currentVideoQuality);
    applyAudioQuality(m_currentAudioQuality);
    
    qDebug() << "MediaPerformanceOptimizer: Initialized";
}

MediaPerformanceOptimizer::~MediaPerformanceOptimizer()
{
    stopPerformanceMonitoring();
    s_instance = nullptr;
}

MediaPerformanceOptimizer* MediaPerformanceOptimizer::instance()
{
    return s_instance;
}

void MediaPerformanceOptimizer::setVideoQuality(VideoQuality quality)
{
    if (m_currentVideoQuality != quality) {
        m_currentVideoQuality = quality;
        applyVideoQuality(quality);
        emit videoQualityChanged(quality);
        
        QString qualityName;
        switch (quality) {
            case VideoQuality::Ultra: qualityName = "Ultra"; break;
            case VideoQuality::High: qualityName = "High"; break;
            case VideoQuality::Medium: qualityName = "Medium"; break;
            case VideoQuality::Low: qualityName = "Low"; break;
            case VideoQuality::Minimal: qualityName = "Minimal"; break;
        }
        
        qDebug() << "MediaPerformanceOptimizer: Video quality set to" << qualityName;
        emit qualityAdjusted(QString("Video quality changed to %1").arg(qualityName));
    }
}

void MediaPerformanceOptimizer::setAudioQuality(AudioQuality quality)
{
    if (m_currentAudioQuality != quality) {
        m_currentAudioQuality = quality;
        applyAudioQuality(quality);
        emit audioQualityChanged(quality);
        
        QString qualityName;
        switch (quality) {
            case AudioQuality::Studio: qualityName = "Studio"; break;
            case AudioQuality::High: qualityName = "High"; break;
            case AudioQuality::Standard: qualityName = "Standard"; break;
            case AudioQuality::Low: qualityName = "Low"; break;
        }
        
        qDebug() << "MediaPerformanceOptimizer: Audio quality set to" << qualityName;
        emit qualityAdjusted(QString("Audio quality changed to %1").arg(qualityName));
    }
}

MediaPerformanceOptimizer::VideoQuality MediaPerformanceOptimizer::getVideoQuality() const
{
    return m_currentVideoQuality;
}

MediaPerformanceOptimizer::AudioQuality MediaPerformanceOptimizer::getAudioQuality() const
{
    return m_currentAudioQuality;
}

void MediaPerformanceOptimizer::setVideoSettings(const VideoSettings& settings)
{
    QMutexLocker locker(&m_metricsMutex);
    m_videoSettings = settings;
    qDebug() << "MediaPerformanceOptimizer: Custom video settings applied:"
             << settings.width << "x" << settings.height
             << "@" << settings.frameRate << "fps"
             << settings.bitrate << "kbps";
}

void MediaPerformanceOptimizer::setAudioSettings(const AudioSettings& settings)
{
    QMutexLocker locker(&m_metricsMutex);
    m_audioSettings = settings;
    qDebug() << "MediaPerformanceOptimizer: Custom audio settings applied:"
             << settings.sampleRate << "Hz"
             << settings.channels << "ch"
             << settings.bitrate << "kbps";
}

MediaPerformanceOptimizer::VideoSettings MediaPerformanceOptimizer::getVideoSettings() const
{
    QMutexLocker locker(&m_metricsMutex);
    return m_videoSettings;
}

MediaPerformanceOptimizer::AudioSettings MediaPerformanceOptimizer::getAudioSettings() const
{
    QMutexLocker locker(&m_metricsMutex);
    return m_audioSettings;
}

void MediaPerformanceOptimizer::enableAdaptiveQuality(bool enabled)
{
    m_adaptiveQualityEnabled = enabled;
    qDebug() << "MediaPerformanceOptimizer: Adaptive quality" << (enabled ? "enabled" : "disabled");
    
    if (enabled && m_monitoringActive) {
        // 立即检查性能并调整
        checkPerformanceThresholds();
    }
}

void MediaPerformanceOptimizer::optimizeForCPUUsage(double maxCpuPercent)
{
    m_maxCpuUsage = maxCpuPercent;
    qDebug() << "MediaPerformanceOptimizer: Max CPU usage set to" << maxCpuPercent << "%";
    
    if (m_adaptiveQualityEnabled) {
        checkPerformanceThresholds();
    }
}

void MediaPerformanceOptimizer::optimizeForParticipantCount(int count)
{
    qDebug() << "MediaPerformanceOptimizer: Optimizing for" << count << "participants";
    
    VideoQuality targetVideoQuality;
    AudioQuality targetAudioQuality;
    
    if (count > 20) {
        // 大型会议 - 降低质量以节省资源
        targetVideoQuality = VideoQuality::Low;
        targetAudioQuality = AudioQuality::Standard;
    } else if (count > 10) {
        // 中型会议 - 中等质量
        targetVideoQuality = VideoQuality::Medium;
        targetAudioQuality = AudioQuality::High;
    } else if (count > 5) {
        // 小型会议 - 高质量
        targetVideoQuality = VideoQuality::High;
        targetAudioQuality = AudioQuality::High;
    } else {
        // 很小的会议 - 最高质量
        targetVideoQuality = VideoQuality::Ultra;
        targetAudioQuality = AudioQuality::Studio;
    }
    
    setVideoQuality(targetVideoQuality);
    setAudioQuality(targetAudioQuality);
}

void MediaPerformanceOptimizer::startPerformanceMonitoring()
{
    if (!m_monitoringActive) {
        m_monitoringActive = true;
        m_monitoringTimer->start();
        qDebug() << "MediaPerformanceOptimizer: Performance monitoring started";
    }
}

void MediaPerformanceOptimizer::stopPerformanceMonitoring()
{
    if (m_monitoringActive) {
        m_monitoringActive = false;
        m_monitoringTimer->stop();
        qDebug() << "MediaPerformanceOptimizer: Performance monitoring stopped";
    }
}

MediaPerformanceOptimizer::PerformanceMetrics MediaPerformanceOptimizer::getCurrentMetrics() const
{
    QMutexLocker locker(&m_metricsMutex);
    return m_metrics;
}

void MediaPerformanceOptimizer::recordEncodingTime(bool isVideo, double timeMs)
{
    QMutexLocker locker(&m_metricsMutex);
    
    if (isVideo) {
        m_metrics.videoEncodingTime = timeMs;
    } else {
        m_metrics.audioEncodingTime = timeMs;
    }
    
    // 记录到历史数据
    m_encodingTimeHistory.push_back(timeMs);
    if (m_encodingTimeHistory.size() > 100) {
        m_encodingTimeHistory.erase(m_encodingTimeHistory.begin());
    }
}

void MediaPerformanceOptimizer::recordDecodingTime(bool isVideo, double timeMs)
{
    QMutexLocker locker(&m_metricsMutex);
    
    if (isVideo) {
        m_metrics.videoDecodingTime = timeMs;
    } else {
        m_metrics.audioDecodingTime = timeMs;
    }
}

void MediaPerformanceOptimizer::recordDroppedFrame(bool isVideo)
{
    QMutexLocker locker(&m_metricsMutex);
    
    if (isVideo) {
        m_metrics.droppedVideoFrames++;
    } else {
        m_metrics.droppedAudioFrames++;
    }
    
    // 如果丢帧过多，触发质量调整
    if (m_adaptiveQualityEnabled) {
        int totalDropped = m_metrics.droppedVideoFrames + m_metrics.droppedAudioFrames;
        if (totalDropped > 10) { // 每10个丢帧检查一次
            QTimer::singleShot(0, this, &MediaPerformanceOptimizer::checkPerformanceThresholds);
        }
    }
}

void MediaPerformanceOptimizer::updatePerformanceMetrics()
{
    // 这里可以从系统获取实际的CPU使用率
    // 简化实现，使用模拟数据
    
    QMutexLocker locker(&m_metricsMutex);
    
    // 基于编码时间估算CPU使用率
    if (!m_encodingTimeHistory.empty()) {
        double avgEncodingTime = std::accumulate(m_encodingTimeHistory.begin(), 
                                               m_encodingTimeHistory.end(), 0.0) / m_encodingTimeHistory.size();
        
        // 简化的CPU使用率估算
        m_metrics.cpuUsage = std::min(100.0, avgEncodingTime * 2.0);
    }
    
    m_cpuUsageHistory.push_back(m_metrics.cpuUsage);
    if (m_cpuUsageHistory.size() > 50) {
        m_cpuUsageHistory.erase(m_cpuUsageHistory.begin());
    }
    
    emit performanceMetricsUpdated(m_metrics);
    
    // 检查是否需要调整质量
    if (m_adaptiveQualityEnabled) {
        checkPerformanceThresholds();
    }
}

void MediaPerformanceOptimizer::checkPerformanceThresholds()
{
    QMutexLocker locker(&m_metricsMutex);
    
    bool shouldDowngrade = false;
    bool shouldUpgrade = false;
    QString reason;
    
    // 检查CPU使用率
    if (m_metrics.cpuUsage > m_maxCpuUsage) {
        shouldDowngrade = true;
        reason = QString("High CPU usage: %1%").arg(m_metrics.cpuUsage, 0, 'f', 1);
    }
    
    // 检查编码时间
    if (m_metrics.videoEncodingTime > 33.0) { // 超过33ms意味着无法维持30fps
        shouldDowngrade = true;
        reason = QString("High encoding time: %1ms").arg(m_metrics.videoEncodingTime, 0, 'f', 1);
    }
    
    // 检查丢帧率
    int totalFrames = 1000; // 假设的总帧数
    double dropRate = static_cast<double>(m_metrics.droppedVideoFrames) / totalFrames;
    if (dropRate > 0.05) { // 丢帧率超过5%
        shouldDowngrade = true;
        reason = QString("High drop rate: %1%").arg(dropRate * 100, 0, 'f', 1);
    }
    
    // 检查是否可以升级质量
    if (!shouldDowngrade && m_cpuUsageHistory.size() >= 10) {
        double avgCpu = std::accumulate(m_cpuUsageHistory.end() - 10, m_cpuUsageHistory.end(), 0.0) / 10.0;
        if (avgCpu < m_maxCpuUsage * 0.6 && m_metrics.videoEncodingTime < 20.0) {
            shouldUpgrade = true;
            reason = QString("Low resource usage, can upgrade quality");
        }
    }
    
    locker.unlock();
    
    if (shouldDowngrade) {
        adjustQualityBasedOnPerformance();
        emit qualityAdjusted(QString("Quality downgraded: %1").arg(reason));
    } else if (shouldUpgrade) {
        // 尝试升级质量
        if (m_currentVideoQuality != VideoQuality::Ultra) {
            VideoQuality newQuality = static_cast<VideoQuality>(static_cast<int>(m_currentVideoQuality) - 1);
            setVideoQuality(newQuality);
        }
    }
}

void MediaPerformanceOptimizer::applyVideoQuality(VideoQuality quality)
{
    m_videoSettings = getQualitySettings(quality);
    qDebug() << "MediaPerformanceOptimizer: Applied video quality settings:"
             << m_videoSettings.width << "x" << m_videoSettings.height
             << "@" << m_videoSettings.frameRate << "fps";
}

void MediaPerformanceOptimizer::applyAudioQuality(AudioQuality quality)
{
    m_audioSettings = getAudioQualitySettings(quality);
    qDebug() << "MediaPerformanceOptimizer: Applied audio quality settings:"
             << m_audioSettings.sampleRate << "Hz"
             << m_audioSettings.channels << "ch";
}

void MediaPerformanceOptimizer::adjustQualityBasedOnPerformance()
{
    // 降级视频质量
    if (m_currentVideoQuality != VideoQuality::Minimal) {
        VideoQuality newQuality = static_cast<VideoQuality>(static_cast<int>(m_currentVideoQuality) + 1);
        setVideoQuality(newQuality);
    }
    
    // 如果视频质量已经是最低，降级音频质量
    if (m_currentVideoQuality == VideoQuality::Minimal && m_currentAudioQuality != AudioQuality::Low) {
        AudioQuality newQuality = static_cast<AudioQuality>(static_cast<int>(m_currentAudioQuality) + 1);
        setAudioQuality(newQuality);
    }
}

MediaPerformanceOptimizer::VideoSettings MediaPerformanceOptimizer::getQualitySettings(VideoQuality quality)
{
    VideoSettings settings;
    
    switch (quality) {
        case VideoQuality::Ultra:
            settings = {1920, 1080, 30, 4000, "H.264"};
            break;
        case VideoQuality::High:
            settings = {1280, 720, 30, 2000, "H.264"};
            break;
        case VideoQuality::Medium:
            settings = {854, 480, 25, 1000, "H.264"};
            break;
        case VideoQuality::Low:
            settings = {640, 360, 20, 500, "H.264"};
            break;
        case VideoQuality::Minimal:
            settings = {426, 240, 15, 250, "H.264"};
            break;
    }
    
    return settings;
}

MediaPerformanceOptimizer::AudioSettings MediaPerformanceOptimizer::getAudioQualitySettings(AudioQuality quality)
{
    AudioSettings settings;
    
    switch (quality) {
        case AudioQuality::Studio:
            settings = {48000, 2, 192, "AAC"};
            break;
        case AudioQuality::High:
            settings = {44100, 2, 128, "AAC"};
            break;
        case AudioQuality::Standard:
            settings = {22050, 1, 64, "AAC"};
            break;
        case AudioQuality::Low:
            settings = {16000, 1, 32, "AAC"};
            break;
    }
    
    return settings;
}