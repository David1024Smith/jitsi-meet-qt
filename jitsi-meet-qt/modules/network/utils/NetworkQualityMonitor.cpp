#include "NetworkQualityMonitor.h"
#include "NetworkUtils.h"
#include <QTimer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRandomGenerator>
#include <QDebug>

class NetworkQualityMonitor::Private
{
public:
    Private()
        : status(NetworkQualityMonitor::Stopped)
        , currentLevel(NetworkQualityMonitor::Fair)
        , currentScore(50)
        , currentLatency(0)
        , currentPacketLoss(0.0)
        , currentBandwidth(0)
        , monitorTimer(nullptr)
        , networkManager(nullptr)
        , monitorInterval(5000)
        , excellentThreshold(90)
        , goodThreshold(70)
        , fairThreshold(50)
        , poorThreshold(30)
        , testCount(0)
        , totalLatency(0)
        , totalPacketLoss(0.0)
        , totalBandwidth(0)
        , maxHistoryMinutes(60)
    {
    }

    NetworkQualityMonitor::MonitorStatus status;
    NetworkQualityMonitor::QualityLevel currentLevel;
    
    int currentScore;
    int currentLatency;
    double currentPacketLoss;
    int currentBandwidth;
    
    QString targetHost;
    QTimer* monitorTimer;
    QNetworkAccessManager* networkManager;
    
    int monitorInterval;
    int excellentThreshold;
    int goodThreshold;
    int fairThreshold;
    int poorThreshold;
    
    // 统计信息
    int testCount;
    qint64 totalLatency;
    double totalPacketLoss;
    qint64 totalBandwidth;
    
    // 历史数据
    QQueue<QVariantMap> historyData;
    int maxHistoryMinutes;
    
    QDateTime startTime;
    QDateTime lastTestTime;
};

NetworkQualityMonitor::NetworkQualityMonitor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->monitorTimer = new QTimer(this);
    d->networkManager = new QNetworkAccessManager(this);
    
    connect(d->monitorTimer, &QTimer::timeout, this, &NetworkQualityMonitor::handleMonitorTimer);
    
    d->monitorTimer->setSingleShot(false);
}

NetworkQualityMonitor::~NetworkQualityMonitor()
{
    stopMonitoring();
    delete d;
}

bool NetworkQualityMonitor::startMonitoring(const QString& targetHost, int interval)
{
    if (d->status == Running) {
        qWarning() << "NetworkQualityMonitor: Already running";
        return false;
    }
    
    if (targetHost.isEmpty()) {
        qWarning() << "NetworkQualityMonitor: Target host is empty";
        return false;
    }
    
    d->targetHost = targetHost;
    d->monitorInterval = interval;
    d->startTime = QDateTime::currentDateTime();
    
    qDebug() << "NetworkQualityMonitor: Starting monitoring of" << targetHost << "with interval" << interval << "ms";
    
    d->status = Starting;
    emit monitorStatusChanged(d->status);
    
    // 执行初始测试
    QVariantMap initialResult = performSingleTest();
    if (initialResult.isEmpty()) {
        d->status = Error;
        emit monitorStatusChanged(d->status);
        emit monitorError("Failed to perform initial network test");
        return false;
    }
    
    // 启动定时器
    d->monitorTimer->start(d->monitorInterval);
    
    d->status = Running;
    emit monitorStatusChanged(d->status);
    
    qDebug() << "NetworkQualityMonitor: Monitoring started successfully";
    return true;
}

void NetworkQualityMonitor::stopMonitoring()
{
    if (d->status == Stopped) {
        return;
    }
    
    qDebug() << "NetworkQualityMonitor: Stopping monitoring";
    
    d->monitorTimer->stop();
    
    d->status = Stopped;
    emit monitorStatusChanged(d->status);
    
    qDebug() << "NetworkQualityMonitor: Monitoring stopped";
}

void NetworkQualityMonitor::pauseMonitoring()
{
    if (d->status != Running) {
        return;
    }
    
    qDebug() << "NetworkQualityMonitor: Pausing monitoring";
    
    d->monitorTimer->stop();
    d->status = Paused;
    emit monitorStatusChanged(d->status);
}

void NetworkQualityMonitor::resumeMonitoring()
{
    if (d->status != Paused) {
        return;
    }
    
    qDebug() << "NetworkQualityMonitor: Resuming monitoring";
    
    d->monitorTimer->start(d->monitorInterval);
    d->status = Running;
    emit monitorStatusChanged(d->status);
}

NetworkQualityMonitor::MonitorStatus NetworkQualityMonitor::monitorStatus() const
{
    return d->status;
}

NetworkQualityMonitor::QualityLevel NetworkQualityMonitor::currentQualityLevel() const
{
    return d->currentLevel;
}

int NetworkQualityMonitor::currentQualityScore() const
{
    return d->currentScore;
}

int NetworkQualityMonitor::currentLatency() const
{
    return d->currentLatency;
}

double NetworkQualityMonitor::currentPacketLoss() const
{
    return d->currentPacketLoss;
}

int NetworkQualityMonitor::currentBandwidth() const
{
    return d->currentBandwidth;
}

QVariantMap NetworkQualityMonitor::getQualityStats() const
{
    QVariantMap stats;
    
    stats["status"] = static_cast<int>(d->status);
    stats["quality_level"] = static_cast<int>(d->currentLevel);
    stats["quality_score"] = d->currentScore;
    stats["current_latency"] = d->currentLatency;
    stats["current_packet_loss"] = d->currentPacketLoss;
    stats["current_bandwidth"] = d->currentBandwidth;
    
    stats["target_host"] = d->targetHost;
    stats["monitor_interval"] = d->monitorInterval;
    stats["test_count"] = d->testCount;
    
    if (d->testCount > 0) {
        stats["average_latency"] = static_cast<int>(d->totalLatency / d->testCount);
        stats["average_packet_loss"] = d->totalPacketLoss / d->testCount;
        stats["average_bandwidth"] = static_cast<int>(d->totalBandwidth / d->testCount);
    }
    
    stats["start_time"] = d->startTime;
    stats["last_test_time"] = d->lastTestTime;
    stats["uptime"] = d->startTime.secsTo(QDateTime::currentDateTime());
    
    return stats;
}

QVariantList NetworkQualityMonitor::getHistoryData(int minutes) const
{
    QVariantList historyList;
    QDateTime cutoffTime = QDateTime::currentDateTime().addSecs(-minutes * 60);
    
    for (const QVariantMap& data : d->historyData) {
        QDateTime timestamp = data.value("timestamp").toDateTime();
        if (timestamp >= cutoffTime) {
            historyList.append(data);
        }
    }
    
    return historyList;
}

void NetworkQualityMonitor::setTargetHost(const QString& host)
{
    d->targetHost = host;
}

QString NetworkQualityMonitor::targetHost() const
{
    return d->targetHost;
}

void NetworkQualityMonitor::setMonitorInterval(int interval)
{
    d->monitorInterval = interval;
    if (d->monitorTimer->isActive()) {
        d->monitorTimer->setInterval(interval);
    }
}

int NetworkQualityMonitor::monitorInterval() const
{
    return d->monitorInterval;
}

void NetworkQualityMonitor::setQualityThresholds(int excellent, int good, int fair, int poor)
{
    d->excellentThreshold = excellent;
    d->goodThreshold = good;
    d->fairThreshold = fair;
    d->poorThreshold = poor;
}

QVariantMap NetworkQualityMonitor::performSingleTest()
{
    if (d->targetHost.isEmpty()) {
        qWarning() << "NetworkQualityMonitor: No target host set";
        return QVariantMap();
    }
    
    qDebug() << "NetworkQualityMonitor: Performing single test to" << d->targetHost;
    
    QVariantMap result;
    
    // 执行延迟测试
    int latency = performLatencyTest();
    
    // 执行丢包测试
    double packetLoss = performPacketLossTest();
    
    // 执行带宽测试
    int bandwidth = performBandwidthTest();
    
    // 计算质量分数
    int score = calculateQualityScore(latency, packetLoss, bandwidth);
    QualityLevel level = getQualityLevel(score);
    
    // 更新当前值
    QualityLevel oldLevel = d->currentLevel;
    d->currentLatency = latency;
    d->currentPacketLoss = packetLoss;
    d->currentBandwidth = bandwidth;
    d->currentScore = score;
    d->currentLevel = level;
    d->lastTestTime = QDateTime::currentDateTime();
    
    // 更新统计信息
    updateStats(latency, packetLoss, bandwidth, score);
    
    // 构建结果
    result["timestamp"] = d->lastTestTime;
    result["latency"] = latency;
    result["packet_loss"] = packetLoss;
    result["bandwidth"] = bandwidth;
    result["quality_score"] = score;
    result["quality_level"] = static_cast<int>(level);
    
    // 添加到历史数据
    addHistoryDataPoint(result);
    
    // 发送信号
    emit testCompleted(result);
    emit latencyChanged(latency);
    emit packetLossChanged(packetLoss);
    emit bandwidthChanged(bandwidth);
    
    if (level != oldLevel) {
        emit qualityChanged(level, score);
        checkQualityWarning(level, oldLevel);
    }
    
    qDebug() << "NetworkQualityMonitor: Test completed - Latency:" << latency << "ms, PacketLoss:" << packetLoss << "%, Bandwidth:" << bandwidth << "kbps, Score:" << score;
    
    return result;
}

void NetworkQualityMonitor::resetStats()
{
    qDebug() << "NetworkQualityMonitor: Resetting statistics";
    
    d->testCount = 0;
    d->totalLatency = 0;
    d->totalPacketLoss = 0.0;
    d->totalBandwidth = 0;
    d->historyData.clear();
    d->startTime = QDateTime::currentDateTime();
}

void NetworkQualityMonitor::triggerTest()
{
    if (d->status == Running || d->status == Paused) {
        performSingleTest();
    }
}

void NetworkQualityMonitor::handleMonitorTimer()
{
    performSingleTest();
    cleanupHistoryData();
}

void NetworkQualityMonitor::handleLatencyTestCompleted()
{
    // 延迟测试完成处理
}

void NetworkQualityMonitor::handleBandwidthTestCompleted()
{
    // 带宽测试完成处理
}

int NetworkQualityMonitor::performLatencyTest()
{
    return NetworkUtils::pingHost(d->targetHost, 5000);
}

double NetworkQualityMonitor::performPacketLossTest()
{
    const int testCount = 10;
    int successCount = 0;
    
    for (int i = 0; i < testCount; ++i) {
        if (NetworkUtils::isPortReachable(d->targetHost, 80, 2000)) {
            successCount++;
        }
    }
    
    double packetLoss = (1.0 - static_cast<double>(successCount) / testCount) * 100.0;
    return qMax(0.0, qMin(100.0, packetLoss));
}

int NetworkQualityMonitor::performBandwidthTest()
{
    // 简化的带宽测试 - 实际实现可能需要更复杂的逻辑
    // 这里返回一个基于延迟的估算值
    int latency = d->currentLatency;
    
    if (latency < 0) {
        return 0;
    }
    
    // 基于延迟估算带宽
    if (latency < 20) {
        return QRandomGenerator::global()->bounded(5000, 10000); // 5-10 Mbps
    } else if (latency < 50) {
        return QRandomGenerator::global()->bounded(2000, 5000);  // 2-5 Mbps
    } else if (latency < 100) {
        return QRandomGenerator::global()->bounded(1000, 2000); // 1-2 Mbps
    } else if (latency < 200) {
        return QRandomGenerator::global()->bounded(500, 1000);  // 0.5-1 Mbps
    } else {
        return QRandomGenerator::global()->bounded(100, 500);   // 0.1-0.5 Mbps
    }
}

int NetworkQualityMonitor::calculateQualityScore(int latency, double packetLoss, int bandwidth)
{
    return NetworkUtils::calculateNetworkQuality(latency, packetLoss, bandwidth);
}

NetworkQualityMonitor::QualityLevel NetworkQualityMonitor::getQualityLevel(int score)
{
    if (score >= d->excellentThreshold) {
        return Excellent;
    } else if (score >= d->goodThreshold) {
        return Good;
    } else if (score >= d->fairThreshold) {
        return Fair;
    } else if (score >= d->poorThreshold) {
        return Poor;
    } else {
        return VeryPoor;
    }
}

void NetworkQualityMonitor::updateStats(int latency, double packetLoss, int bandwidth, int score)
{
    d->testCount++;
    
    if (latency >= 0) {
        d->totalLatency += latency;
    }
    
    d->totalPacketLoss += packetLoss;
    d->totalBandwidth += bandwidth;
}

void NetworkQualityMonitor::addHistoryDataPoint(const QVariantMap& data)
{
    d->historyData.enqueue(data);
    
    // 限制历史数据大小
    const int maxDataPoints = d->maxHistoryMinutes * 60 / (d->monitorInterval / 1000);
    while (d->historyData.size() > maxDataPoints) {
        d->historyData.dequeue();
    }
}

void NetworkQualityMonitor::cleanupHistoryData()
{
    QDateTime cutoffTime = QDateTime::currentDateTime().addSecs(-d->maxHistoryMinutes * 60);
    
    while (!d->historyData.isEmpty()) {
        QVariantMap data = d->historyData.head();
        QDateTime timestamp = data.value("timestamp").toDateTime();
        
        if (timestamp >= cutoffTime) {
            break;
        }
        
        d->historyData.dequeue();
    }
}

void NetworkQualityMonitor::checkQualityWarning(QualityLevel newLevel, QualityLevel oldLevel)
{
    if (newLevel < oldLevel) {
        QString message;
        
        switch (newLevel) {
            case VeryPoor:
                message = "Network quality is very poor. Connection may be unstable.";
                break;
            case Poor:
                message = "Network quality is poor. You may experience connection issues.";
                break;
            case Fair:
                message = "Network quality is fair. Some features may be affected.";
                break;
            default:
                return; // No warning for good or excellent quality
        }
        
        emit qualityWarning(newLevel, message);
    }
}