#include "NetworkOptimizer.h"
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <QDebug>
#include <QUrl>
#include <QTimer>
#include <algorithm>
#include <numeric>

#ifdef QT_ZLIB_SUPPORT
#include <zlib.h>
#endif

NetworkOptimizer* NetworkOptimizer::s_instance = nullptr;

NetworkOptimizer::NetworkOptimizer(QObject *parent)
    : QObject(parent)
    , m_monitoringTimer(new QTimer(this))
    , m_latencyTimer(new QTimer(this))
    , m_networkManager(std::make_unique<QNetworkAccessManager>(this))
    , m_latencyTestReply(nullptr)
    , m_monitoringActive(false)
{
    s_instance = this;
    
    // 设置监控定时器
    m_monitoringTimer->setInterval(10000); // 每10秒更新一次
    connect(m_monitoringTimer, &QTimer::timeout, this, &NetworkOptimizer::updateNetworkMetrics);
    
    // 设置延迟测试定时器
    m_latencyTimer->setInterval(5000); // 每5秒测试一次延迟
    connect(m_latencyTimer, &QTimer::timeout, this, &NetworkOptimizer::measureNetworkLatency);
    
    initializeNetworkManager();
    
    qDebug() << "NetworkOptimizer: Initialized";
}

NetworkOptimizer::~NetworkOptimizer()
{
    stopNetworkMonitoring();
    
    if (m_latencyTestReply) {
        m_latencyTestReply->abort();
        m_latencyTestReply->deleteLater();
    }
    
    s_instance = nullptr;
}

NetworkOptimizer* NetworkOptimizer::instance()
{
    return s_instance;
}

void NetworkOptimizer::startNetworkMonitoring()
{
    if (!m_monitoringActive) {
        m_monitoringActive = true;
        m_monitoringTimer->start();
        m_latencyTimer->start();
        qDebug() << "NetworkOptimizer: Network monitoring started";
    }
}

void NetworkOptimizer::stopNetworkMonitoring()
{
    if (m_monitoringActive) {
        m_monitoringActive = false;
        m_monitoringTimer->stop();
        m_latencyTimer->stop();
        qDebug() << "NetworkOptimizer: Network monitoring stopped";
    }
}

NetworkOptimizer::NetworkMetrics NetworkOptimizer::getCurrentMetrics() const
{
    QMutexLocker locker(&m_metricsMutex);
    return m_currentMetrics;
}

NetworkOptimizer::ConnectionQuality NetworkOptimizer::getConnectionQuality() const
{
    QMutexLocker locker(&m_metricsMutex);
    return m_currentMetrics.quality;
}

void NetworkOptimizer::setOptimizationSettings(const OptimizationSettings& settings)
{
    m_settings = settings;
    
    // 应用新设置
    if (m_networkManager) {
        // 更新连接超时
        // Qt的QNetworkAccessManager没有直接的超时设置，需要在请求级别设置
        
        // 更新连接池设置
        setupConnectionPool();
    }
    
    qDebug() << "NetworkOptimizer: Settings updated";
    emit optimizationApplied("Network settings updated");
}

NetworkOptimizer::OptimizationSettings NetworkOptimizer::getOptimizationSettings() const
{
    return m_settings;
}

QNetworkAccessManager* NetworkOptimizer::getOptimizedNetworkManager()
{
    return m_networkManager.get();
}

void NetworkOptimizer::optimizeForParticipantCount(int count)
{
    qDebug() << "NetworkOptimizer: Optimizing for" << count << "participants";
    
    OptimizationSettings newSettings = m_settings;
    
    if (count > 20) {
        // 大型会议优化
        newSettings.maxConcurrentConnections = 10;
        newSettings.compressionEnabled = true;
        newSettings.adaptiveBitrateEnabled = true;
        
    } else if (count > 10) {
        // 中型会议优化
        newSettings.maxConcurrentConnections = 8;
        newSettings.compressionEnabled = true;
        
    } else {
        // 小型会议，使用默认设置
        newSettings.maxConcurrentConnections = 6;
    }
    
    setOptimizationSettings(newSettings);
    
    // 根据参与者数量调整比特率
    if (m_settings.adaptiveBitrateEnabled) {
        auto quality = getConnectionQuality();
        adjustBitrateForQuality(quality);
    }
}

void NetworkOptimizer::enableAdaptiveBitrate(bool enabled)
{
    m_settings.adaptiveBitrateEnabled = enabled;
    qDebug() << "NetworkOptimizer: Adaptive bitrate" << (enabled ? "enabled" : "disabled");
    
    if (enabled) {
        adjustBitrateForQuality(getConnectionQuality());
    }
}

void NetworkOptimizer::adjustBitrateForQuality(ConnectionQuality quality)
{
    QString bitrateMode;
    
    switch (quality) {
        case ConnectionQuality::Excellent:
            bitrateMode = "high";
            break;
        case ConnectionQuality::Good:
            bitrateMode = "medium";
            break;
        case ConnectionQuality::Fair:
            bitrateMode = "low";
            break;
        case ConnectionQuality::Poor:
            bitrateMode = "very_low";
            break;
    }
    
    qDebug() << "NetworkOptimizer: Adjusting bitrate to" << bitrateMode << "for quality" << static_cast<int>(quality);
    emit optimizationApplied(QString("Bitrate adjusted to %1").arg(bitrateMode));
}

QByteArray NetworkOptimizer::compressData(const QByteArray& data)
{
    if (!m_settings.compressionEnabled || data.isEmpty()) {
        return data;
    }
    
#ifdef QT_ZLIB_SUPPORT
    uLongf compressedSize = compressBound(data.size());
    QByteArray compressed(compressedSize, 0);
    
    int result = compress(reinterpret_cast<Bytef*>(compressed.data()), &compressedSize,
                         reinterpret_cast<const Bytef*>(data.constData()), data.size());
    
    if (result == Z_OK) {
        compressed.resize(compressedSize);
        
        // 只有在压缩效果显著时才返回压缩数据
        if (compressed.size() < data.size() * 0.9) {
            return compressed;
        }
    }
#endif
    
    return data; // 返回原始数据如果压缩失败或效果不佳
}

QByteArray NetworkOptimizer::decompressData(const QByteArray& compressedData)
{
    if (!m_settings.compressionEnabled || compressedData.isEmpty()) {
        return compressedData;
    }
    
#ifdef QT_ZLIB_SUPPORT
    // 假设原始数据不会超过压缩数据的10倍
    uLongf uncompressedSize = compressedData.size() * 10;
    QByteArray uncompressed(uncompressedSize, 0);
    
    int result = uncompress(reinterpret_cast<Bytef*>(uncompressed.data()), &uncompressedSize,
                           reinterpret_cast<const Bytef*>(compressedData.constData()), compressedData.size());
    
    if (result == Z_OK) {
        uncompressed.resize(uncompressedSize);
        return uncompressed;
    }
#endif
    
    return compressedData; // 返回原始数据如果解压失败
}

void NetworkOptimizer::measureNetworkLatency()
{
    if (m_latencyTestReply) {
        return; // 已有测试在进行中
    }
    
    // 使用一个轻量级的服务进行延迟测试
    QNetworkRequest request(QUrl("https://www.google.com/generate_204"));
    request.setRawHeader("User-Agent", "JitsiMeet-Qt/1.0");
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    
    m_latencyTestStart = std::chrono::steady_clock::now();
    m_latencyTestReply = m_networkManager->head(request);
    
    connect(m_latencyTestReply, &QNetworkReply::finished, this, &NetworkOptimizer::onLatencyTestFinished);
    
    // 设置超时
    QTimer::singleShot(5000, this, [this]() {
        if (m_latencyTestReply && m_latencyTestReply->isRunning()) {
            m_latencyTestReply->abort();
        }
    });
}

void NetworkOptimizer::updateNetworkMetrics()
{
    QMutexLocker locker(&m_metricsMutex);
    
    // 计算平均延迟
    if (!m_latencyMeasurements.empty()) {
        // 这里简化处理，实际应该计算RTT
        // 当前只是记录测量时间点
    }
    
    // 更新连接质量
    auto oldQuality = m_currentMetrics.quality;
    m_currentMetrics.quality = calculateQuality(m_currentMetrics);
    
    if (oldQuality != m_currentMetrics.quality) {
        emit networkQualityChanged(m_currentMetrics.quality);
        
        if (m_settings.adaptiveBitrateEnabled) {
            adjustBitrateForQuality(m_currentMetrics.quality);
        }
    }
    
    emit metricsUpdated(m_currentMetrics);
}

void NetworkOptimizer::onLatencyTestFinished()
{
    if (!m_latencyTestReply) {
        return;
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_latencyTestStart);
    
    if (m_latencyTestReply->error() == QNetworkReply::NoError) {
        QMutexLocker locker(&m_metricsMutex);
        m_currentMetrics.latency = latency;
        
        // 保持最近的延迟测量
        m_latencyMeasurements.push(m_latencyTestStart);
        if (m_latencyMeasurements.size() > 10) {
            m_latencyMeasurements.pop();
        }
        
        qDebug() << "NetworkOptimizer: Latency measured:" << latency.count() << "ms";
    } else {
        qWarning() << "NetworkOptimizer: Latency test failed:" << m_latencyTestReply->errorString();
    }
    
    m_latencyTestReply->deleteLater();
    m_latencyTestReply = nullptr;
}

void NetworkOptimizer::initializeNetworkManager()
{
    if (!m_networkManager) {
        return;
    }
    
    // 设置网络管理器的基本配置
    setupConnectionPool();
    
    // 设置代理（如果需要）
    // m_networkManager->setProxy(QNetworkProxy::NoProxy);
    
    qDebug() << "NetworkOptimizer: Network manager initialized";
}

void NetworkOptimizer::setupConnectionPool()
{
    // Qt的QNetworkAccessManager会自动管理连接池
    // 这里可以设置一些相关的属性
    
    qDebug() << "NetworkOptimizer: Connection pool configured for max" 
             << m_settings.maxConcurrentConnections << "connections";
}

void NetworkOptimizer::measureBandwidth()
{
    // 带宽测量的简化实现
    // 实际应用中可能需要更复杂的测量方法
    
    QMutexLocker locker(&m_metricsMutex);
    
    // 基于延迟估算带宽（简化方法）
    if (m_currentMetrics.latency.count() < 50) {
        m_currentMetrics.bandwidthMbps = 10.0; // 假设高速连接
    } else if (m_currentMetrics.latency.count() < 100) {
        m_currentMetrics.bandwidthMbps = 5.0;
    } else if (m_currentMetrics.latency.count() < 200) {
        m_currentMetrics.bandwidthMbps = 2.0;
    } else {
        m_currentMetrics.bandwidthMbps = 1.0;
    }
}

NetworkOptimizer::ConnectionQuality NetworkOptimizer::calculateQuality(const NetworkMetrics& metrics)
{
    // 基于延迟和带宽计算连接质量
    if (metrics.latency.count() < 50 && metrics.bandwidthMbps > 10.0) {
        return ConnectionQuality::Excellent;
    } else if (metrics.latency.count() < 100 && metrics.bandwidthMbps > 5.0) {
        return ConnectionQuality::Good;
    } else if (metrics.latency.count() < 200 && metrics.bandwidthMbps > 1.0) {
        return ConnectionQuality::Fair;
    } else {
        return ConnectionQuality::Poor;
    }
}