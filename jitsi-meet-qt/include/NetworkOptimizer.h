#ifndef NETWORKOPTIMIZER_H
#define NETWORKOPTIMIZER_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMutex>
#include <memory>
#include <chrono>
#include <queue>

/**
 * @brief 网络优化器 - 负责网络通信和数据传输效率优化
 * 
 * 提供连接池管理、数据压缩、带宽自适应等功能
 */
class NetworkOptimizer : public QObject
{
    Q_OBJECT

public:
    enum class ConnectionQuality {
        Excellent,  // < 50ms, > 10Mbps
        Good,       // < 100ms, > 5Mbps
        Fair,       // < 200ms, > 1Mbps
        Poor        // > 200ms, < 1Mbps
    };

    struct NetworkMetrics {
        std::chrono::milliseconds latency{0};
        double bandwidthMbps{0.0};
        double packetLoss{0.0};
        int activeConnections{0};
        ConnectionQuality quality{ConnectionQuality::Fair};
    };

    struct OptimizationSettings {
        bool compressionEnabled{true};
        bool connectionPoolingEnabled{true};
        bool adaptiveBitrateEnabled{true};
        int maxConcurrentConnections{6};
        int connectionTimeoutMs{30000};
        int retryAttempts{3};
    };

    explicit NetworkOptimizer(QObject *parent = nullptr);
    ~NetworkOptimizer();

    static NetworkOptimizer* instance();

    // 网络质量监控
    void startNetworkMonitoring();
    void stopNetworkMonitoring();
    NetworkMetrics getCurrentMetrics() const;
    ConnectionQuality getConnectionQuality() const;

    // 优化设置
    void setOptimizationSettings(const OptimizationSettings& settings);
    OptimizationSettings getOptimizationSettings() const;

    // 连接管理
    QNetworkAccessManager* getOptimizedNetworkManager();
    void optimizeForParticipantCount(int count);

    // 带宽自适应
    void enableAdaptiveBitrate(bool enabled);
    void adjustBitrateForQuality(ConnectionQuality quality);

    // 数据压缩
    QByteArray compressData(const QByteArray& data);
    QByteArray decompressData(const QByteArray& compressedData);

signals:
    void networkQualityChanged(ConnectionQuality quality);
    void metricsUpdated(const NetworkMetrics& metrics);
    void optimizationApplied(const QString& description);

private slots:
    void measureNetworkLatency();
    void updateNetworkMetrics();
    void onLatencyTestFinished();

private:
    void initializeNetworkManager();
    void setupConnectionPool();
    void measureBandwidth();
    ConnectionQuality calculateQuality(const NetworkMetrics& metrics);
    
    static NetworkOptimizer* s_instance;
    
    mutable QMutex m_metricsMutex;
    QTimer* m_monitoringTimer;
    QTimer* m_latencyTimer;
    
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    NetworkMetrics m_currentMetrics;
    OptimizationSettings m_settings;
    
    std::queue<std::chrono::steady_clock::time_point> m_latencyMeasurements;
    QNetworkReply* m_latencyTestReply;
    std::chrono::steady_clock::time_point m_latencyTestStart;
    
    bool m_monitoringActive;
};

#endif // NETWORKOPTIMIZER_H