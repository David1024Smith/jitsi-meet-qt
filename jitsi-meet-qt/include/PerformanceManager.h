#ifndef PERFORMANCEMANAGER_H
#define PERFORMANCEMANAGER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>
#include <QThread>
#include <memory>
#include <chrono>
#include <unordered_map>

class QApplication;

/**
 * @brief 性能管理器 - 负责应用程序性能监控和优化
 * 
 * 提供启动时间优化、资源加载优化、内存监控等功能
 */
class PerformanceManager : public QObject
{
    Q_OBJECT

public:
    enum class MetricType {
        StartupTime,
        MemoryUsage,
        NetworkLatency,
        VideoFrameRate,
        AudioLatency,
        CPUUsage
    };

    struct PerformanceMetrics {
        std::chrono::milliseconds startupTime{0};
        size_t memoryUsageMB{0};
        std::chrono::milliseconds networkLatency{0};
        double videoFrameRate{0.0};
        std::chrono::milliseconds audioLatency{0};
        double cpuUsagePercent{0.0};
        std::chrono::steady_clock::time_point timestamp;
    };

    explicit PerformanceManager(QObject *parent = nullptr);
    ~PerformanceManager();

    static PerformanceManager* instance();

    // 启动时间优化
    void startStartupTimer();
    void endStartupTimer();
    std::chrono::milliseconds getStartupTime() const;

    // 内存管理
    void startMemoryMonitoring();
    void stopMemoryMonitoring();
    size_t getCurrentMemoryUsage() const;
    size_t getPeakMemoryUsage() const;

    // 性能指标记录
    void recordMetric(MetricType type, double value);
    PerformanceMetrics getCurrentMetrics() const;

    // 资源预加载
    void preloadResources();
    void enableLazyLoading(bool enabled);

    // 大型会议优化
    void optimizeForLargeConference(int participantCount);
    void setVideoQualityMode(const QString& mode);

signals:
    void memoryWarning(size_t currentUsage, size_t threshold);
    void performanceWarning(MetricType type, double value);
    void metricsUpdated(const PerformanceMetrics& metrics);

private slots:
    void updateMemoryMetrics();
    void updatePerformanceMetrics();

private:
    void initializeOptimizations();
    void setupMemoryThresholds();
    void checkPerformanceThresholds(MetricType type, double value);
    
    static PerformanceManager* s_instance;
    
    QElapsedTimer m_startupTimer;
    QTimer* m_memoryMonitorTimer;
    QTimer* m_metricsTimer;
    
    mutable QMutex m_metricsMutex;
    PerformanceMetrics m_currentMetrics;
    
    size_t m_peakMemoryUsage;
    size_t m_memoryWarningThreshold;
    bool m_lazyLoadingEnabled;
    
    std::unordered_map<MetricType, std::vector<double>> m_metricHistory;
};

#endif // PERFORMANCEMANAGER_H