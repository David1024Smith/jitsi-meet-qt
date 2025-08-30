#ifndef PERFORMANCEVALIDATOR_H
#define PERFORMANCEVALIDATOR_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QMutex>
#include <QElapsedTimer>

/**
 * @brief 性能验证器
 * 
 * 负责验证各个模块的性能是否符合要求。
 */
class PerformanceValidator : public QObject
{
    Q_OBJECT

public:
    struct PerformanceMetrics {
        double cpuUsage;        // CPU使用率 (%)
        qint64 memoryUsage;     // 内存使用 (bytes)
        double executionTime;   // 执行时间 (ms)
        double throughput;      // 吞吐量 (operations/sec)
        double latency;         // 延迟 (ms)
    };

    struct PerformanceThresholds {
        double maxCpuUsage = 80.0;      // 最大CPU使用率
        qint64 maxMemoryUsage = 512 * 1024 * 1024; // 最大内存使用 (512MB)
        double maxExecutionTime = 5000.0; // 最大执行时间 (5秒)
        double minThroughput = 10.0;     // 最小吞吐量
        double maxLatency = 1000.0;      // 最大延迟 (1秒)
    };

    explicit PerformanceValidator(QObject *parent = nullptr);
    ~PerformanceValidator();

    bool initialize();
    
    bool validateModulePerformance(const QString& moduleName);
    PerformanceMetrics measurePerformance(const QString& moduleName);
    
    void setPerformanceThresholds(const PerformanceThresholds& thresholds);
    PerformanceThresholds getPerformanceThresholds() const;
    
    QVariantMap getPerformanceReport(const QString& moduleName) const;
    QStringList getPerformanceHistory(const QString& moduleName) const;
    
    void clearPerformanceHistory();

signals:
    void performanceMeasured(const QString& moduleName, const PerformanceMetrics& metrics);
    void performanceThresholdExceeded(const QString& moduleName, const QString& metric, double value);

private:
    PerformanceMetrics measureAudioPerformance();
    PerformanceMetrics measureNetworkPerformance();
    PerformanceMetrics measureUIPerformance();
    PerformanceMetrics measureChatPerformance();
    PerformanceMetrics measureScreenSharePerformance();
    PerformanceMetrics measureMeetingPerformance();
    PerformanceMetrics measurePerformanceModulePerformance();
    PerformanceMetrics measureSettingsPerformance();
    PerformanceMetrics measureUtilsPerformance();
    
    double getCurrentCpuUsage();
    qint64 getCurrentMemoryUsage();
    double measureExecutionTime(std::function<void()> operation);
    
    bool validateMetrics(const PerformanceMetrics& metrics);
    void recordPerformanceHistory(const QString& moduleName, const PerformanceMetrics& metrics);

    bool m_initialized;
    PerformanceThresholds m_thresholds;
    QHash<QString, QList<PerformanceMetrics>> m_performanceHistory;
    mutable QMutex m_mutex;
};

Q_DECLARE_METATYPE(PerformanceValidator::PerformanceMetrics)

#endif // PERFORMANCEVALIDATOR_H