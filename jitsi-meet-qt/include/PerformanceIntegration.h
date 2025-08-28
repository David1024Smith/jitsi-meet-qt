#ifndef PERFORMANCEINTEGRATION_H
#define PERFORMANCEINTEGRATION_H

#include <QObject>
#include <memory>

class PerformanceManager;
class MemoryLeakDetector;
class NetworkOptimizer;
class MediaPerformanceOptimizer;

/**
 * @brief 性能集成管理器 - 统一管理所有性能优化组件
 * 
 * 负责协调各个性能优化器之间的工作，提供统一的接口
 */
class PerformanceIntegration : public QObject
{
    Q_OBJECT

public:
    explicit PerformanceIntegration(QObject *parent = nullptr);
    ~PerformanceIntegration();

    static PerformanceIntegration* instance();

    // 初始化和清理
    void initialize();
    void shutdown();

    // 会议优化
    void optimizeForConference(int participantCount);
    void optimizeForNetworkQuality(int latencyMs, double bandwidthMbps);

    // 启动时间优化
    void startApplicationOptimization();
    void endApplicationOptimization();

    // 获取各个优化器的实例
    PerformanceManager* performanceManager() const;
    MemoryLeakDetector* memoryLeakDetector() const;
    NetworkOptimizer* networkOptimizer() const;
    MediaPerformanceOptimizer* mediaPerformanceOptimizer() const;

    // 统一的性能报告
    QString getPerformanceReport() const;
    void exportPerformanceData(const QString& filePath) const;

signals:
    void performanceOptimized(const QString& description);
    void performanceWarning(const QString& warning);
    void memoryLeakDetected(int leakCount, size_t totalBytes);

private slots:
    void onMemoryWarning(size_t currentUsage, size_t threshold);
    void onNetworkQualityChanged(int quality);
    void onPerformanceWarning(int metricType, double value);
    void onMemoryLeakDetected(int leakCount, size_t totalBytes);

private:
    void connectSignals();
    void applyStartupOptimizations();
    
    static PerformanceIntegration* s_instance;
    
    std::unique_ptr<PerformanceManager> m_performanceManager;
    std::unique_ptr<MemoryLeakDetector> m_memoryLeakDetector;
    std::unique_ptr<NetworkOptimizer> m_networkOptimizer;
    std::unique_ptr<MediaPerformanceOptimizer> m_mediaPerformanceOptimizer;
    
    bool m_initialized;
};

#endif // PERFORMANCEINTEGRATION_H