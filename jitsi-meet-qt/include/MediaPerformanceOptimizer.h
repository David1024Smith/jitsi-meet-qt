#ifndef MEDIAPERFORMANCEOPTIMIZER_H
#define MEDIAPERFORMANCEOPTIMIZER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <memory>

/**
 * @brief 媒体性能优化器 - 负责音视频编解码性能优化
 * 
 * 提供编码参数调整、帧率优化、质量自适应等功能
 */
class MediaPerformanceOptimizer : public QObject
{
    Q_OBJECT

public:
    enum class VideoQuality {
        Ultra,      // 1080p, 30fps, high bitrate
        High,       // 720p, 30fps, medium bitrate
        Medium,     // 480p, 25fps, low bitrate
        Low,        // 360p, 20fps, very low bitrate
        Minimal     // 240p, 15fps, minimal bitrate
    };

    enum class AudioQuality {
        Studio,     // 48kHz, stereo, high bitrate
        High,       // 44.1kHz, stereo, medium bitrate
        Standard,   // 22kHz, mono, standard bitrate
        Low         // 16kHz, mono, low bitrate
    };

    struct VideoSettings {
        int width{1280};
        int height{720};
        int frameRate{30};
        int bitrate{2000}; // kbps
        QString codec{"H.264"};
    };

    struct AudioSettings {
        int sampleRate{44100};
        int channels{2};
        int bitrate{128}; // kbps
        QString codec{"AAC"};
    };

    struct PerformanceMetrics {
        double videoEncodingTime{0.0}; // ms per frame
        double audioEncodingTime{0.0}; // ms per frame
        double videoDecodingTime{0.0}; // ms per frame
        double audioDecodingTime{0.0}; // ms per frame
        int droppedVideoFrames{0};
        int droppedAudioFrames{0};
        double cpuUsage{0.0}; // percentage
    };

    explicit MediaPerformanceOptimizer(QObject *parent = nullptr);
    ~MediaPerformanceOptimizer();

    static MediaPerformanceOptimizer* instance();

    // 质量设置
    void setVideoQuality(VideoQuality quality);
    void setAudioQuality(AudioQuality quality);
    VideoQuality getVideoQuality() const;
    AudioQuality getAudioQuality() const;

    // 自定义设置
    void setVideoSettings(const VideoSettings& settings);
    void setAudioSettings(const AudioSettings& settings);
    VideoSettings getVideoSettings() const;
    AudioSettings getAudioSettings() const;

    // 自适应优化
    void enableAdaptiveQuality(bool enabled);
    void optimizeForCPUUsage(double maxCpuPercent);
    void optimizeForParticipantCount(int count);

    // 性能监控
    void startPerformanceMonitoring();
    void stopPerformanceMonitoring();
    PerformanceMetrics getCurrentMetrics() const;

    // 编码优化
    void recordEncodingTime(bool isVideo, double timeMs);
    void recordDecodingTime(bool isVideo, double timeMs);
    void recordDroppedFrame(bool isVideo);

signals:
    void videoQualityChanged(VideoQuality quality);
    void audioQualityChanged(AudioQuality quality);
    void performanceMetricsUpdated(const PerformanceMetrics& metrics);
    void qualityAdjusted(const QString& reason);

private slots:
    void updatePerformanceMetrics();
    void checkPerformanceThresholds();

private:
    void applyVideoQuality(VideoQuality quality);
    void applyAudioQuality(AudioQuality quality);
    void adjustQualityBasedOnPerformance();
    VideoSettings getQualitySettings(VideoQuality quality);
    AudioSettings getAudioQualitySettings(AudioQuality quality);
    
    static MediaPerformanceOptimizer* s_instance;
    
    mutable QMutex m_metricsMutex;
    QTimer* m_monitoringTimer;
    
    VideoQuality m_currentVideoQuality;
    AudioQuality m_currentAudioQuality;
    VideoSettings m_videoSettings;
    AudioSettings m_audioSettings;
    
    PerformanceMetrics m_metrics;
    bool m_adaptiveQualityEnabled;
    double m_maxCpuUsage;
    
    // 性能历史数据
    std::vector<double> m_encodingTimeHistory;
    std::vector<double> m_cpuUsageHistory;
    
    bool m_monitoringActive;
};

#endif // MEDIAPERFORMANCEOPTIMIZER_H