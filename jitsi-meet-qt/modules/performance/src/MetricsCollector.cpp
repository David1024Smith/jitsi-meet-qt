#include "MetricsCollector.h"
#include "BaseMonitor.h"
#include "PerformanceConfig.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QStorageInfo>

MetricsCollector::MetricsCollector(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
    , m_collectionTimer(new QTimer(this))
    , m_cleanupTimer(new QTimer(this))
    , m_isCollecting(false)
    , m_storageStrategy(MemoryOnly)
    , m_collectionInterval(1000)
    , m_dataRetentionHours(24)
    , m_maxStorageSize(100 * 1024 * 1024) // 100MB
{
    // 设置定时器
    m_collectionTimer->setSingleShot(false);
    m_cleanupTimer->setSingleShot(false);
    
    // 连接信号
    connect(m_collectionTimer, &QTimer::timeout, this, &MetricsCollector::performCollection);
    connect(m_cleanupTimer, &QTimer::timeout, this, &MetricsCollector::performDataCleanup);
    
    // 设置清理定时器为每小时执行一次
    m_cleanupTimer->setInterval(3600000); // 1 hour
}

MetricsCollector::~MetricsCollector()
{
    stop();
}

bool MetricsCollector::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    try {
        // 初始化存储
        if (!initializeStorage()) {
            qCritical() << "MetricsCollector: Failed to initialize storage";
            return false;
        }
        
        // 加载历史数据
        if (m_storageStrategy != MemoryOnly) {
            loadHistoricalData();
        }
        
        qDebug() << "MetricsCollector: Initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: Exception during initialization:" << e.what();
        emit errorOccurred(QString("Initialization failed: %1").arg(e.what()));
        return false;
    }
}

bool MetricsCollector::start()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isCollecting) {
        return true;
    }
    
    try {
        // 启动收集定时器
        m_collectionTimer->setInterval(m_collectionInterval);
        m_collectionTimer->start();
        
        // 启动清理定时器
        m_cleanupTimer->start();
        
        m_isCollecting = true;
        qDebug() << "MetricsCollector: Started collecting metrics";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: Exception during start:" << e.what();
        emit errorOccurred(QString("Start failed: %1").arg(e.what()));
        return false;
    }
}

void MetricsCollector::stop()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isCollecting) {
        return;
    }
    
    try {
        // 停止定时器
        m_collectionTimer->stop();
        m_cleanupTimer->stop();
        
        m_isCollecting = false;
        qDebug() << "MetricsCollector: Stopped collecting metrics";
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: Exception during stop:" << e.what();
        emit errorOccurred(QString("Stop failed: %1").arg(e.what()));
    }
}

bool MetricsCollector::isCollecting() const
{
    QMutexLocker locker(&m_mutex);
    return m_isCollecting;
}

void MetricsCollector::setConfig(PerformanceConfig* config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
    
    if (m_config) {
        // 应用配置
        setCollectionInterval(m_config->monitoringInterval());
        setDataRetentionHours(m_config->dataRetentionHours());
        
        // 设置存储路径
        QString storagePath = m_config->storagePath();
        if (!storagePath.isEmpty()) {
            m_storageFilePath = storagePath + "/metrics_data.bin";
        }
    }
}

PerformanceConfig* MetricsCollector::config() const
{
    return m_config;
}

bool MetricsCollector::registerMonitor(BaseMonitor* monitor)
{
    if (!monitor) {
        return false;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString name = monitor->monitorName();
    if (m_monitors.contains(name)) {
        qWarning() << "MetricsCollector: Monitor already registered:" << name;
        return false;
    }
    
    m_monitors[name] = monitor;
    
    // 连接信号
    connect(monitor, &BaseMonitor::dataUpdated,
            this, &MetricsCollector::handleMonitorData);
    
    qDebug() << "MetricsCollector: Registered monitor:" << name;
    return true;
}

bool MetricsCollector::unregisterMonitor(const QString& monitorName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_monitors.contains(monitorName)) {
        return false;
    }
    
    BaseMonitor* monitor = m_monitors.take(monitorName);
    
    // 断开信号连接
    disconnect(monitor, nullptr, this, nullptr);
    
    qDebug() << "MetricsCollector: Unregistered monitor:" << monitorName;
    return true;
}

BaseMonitor* MetricsCollector::getMonitor(const QString& monitorName) const
{
    QMutexLocker locker(&m_mutex);
    return m_monitors.value(monitorName, nullptr);
}

QList<BaseMonitor*> MetricsCollector::getAllMonitors() const
{
    QMutexLocker locker(&m_mutex);
    return m_monitors.values();
}

void MetricsCollector::setCollectionInterval(int interval)
{
    QMutexLocker locker(&m_mutex);
    
    if (interval > 0) {
        m_collectionInterval = interval;
        if (m_isCollecting) {
            m_collectionTimer->setInterval(interval);
        }
    }
}

int MetricsCollector::collectionInterval() const
{
    QMutexLocker locker(&m_mutex);
    return m_collectionInterval;
}

void MetricsCollector::setDataRetentionHours(int hours)
{
    QMutexLocker locker(&m_mutex);
    
    if (hours > 0) {
        m_dataRetentionHours = hours;
    }
}

int MetricsCollector::dataRetentionHours() const
{
    QMutexLocker locker(&m_mutex);
    return m_dataRetentionHours;
}

void MetricsCollector::setStorageStrategy(StorageStrategy strategy)
{
    QMutexLocker locker(&m_mutex);
    m_storageStrategy = strategy;
    
    // 重新初始化存储
    initializeStorage();
}

MetricsCollector::StorageStrategy MetricsCollector::storageStrategy() const
{
    QMutexLocker locker(&m_mutex);
    return m_storageStrategy;
}

PerformanceMetrics MetricsCollector::collectCurrentMetrics()
{
    QMutexLocker locker(&m_mutex);
    
    PerformanceMetrics metrics;
    metrics.timestamp = QDateTime::currentDateTime();
    
    try {
        // 从所有监控器收集数据
        for (auto monitor : m_monitors) {
            if (monitor->isTracking()) {
                ResourceUsage usage = monitor->getCurrentUsage();
                
                // 将资源使用数据映射到性能指标
                metrics.system.cpuUsage = usage.cpu.totalUsage;
                metrics.system.memoryUsage = usage.memory.usedMemory / (1024 * 1024); // Convert to MB
                metrics.system.diskUsage = usage.disk.usagePercentage;
                metrics.system.temperature = usage.cpu.temperature;
                
                metrics.network.bandwidth = usage.network.receiveSpeed + usage.network.sendSpeed;
                metrics.network.latency = usage.network.latency;
                metrics.network.packetLoss = 0.0; // 需要从具体监控器获取
                metrics.network.connectionQuality = 100; // 需要计算
            }
        }
        
        // 存储指标数据
        storeMetrics(metrics);
        
        return metrics;
        
    } catch (const std::exception& e) {
        qWarning() << "MetricsCollector: Exception in collectCurrentMetrics:" << e.what();
        emit errorOccurred(QString("Collection failed: %1").arg(e.what()));
        return PerformanceMetrics();
    }
}

PerformanceMetrics MetricsCollector::getLatestMetrics() const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_metricsHistory.isEmpty()) {
        return m_metricsHistory.last();
    }
    
    return PerformanceMetrics();
}

QList<PerformanceMetrics> MetricsCollector::getHistoricalMetrics(const QDateTime& from, const QDateTime& to) const
{
    QMutexLocker locker(&m_mutex);
    
    QList<PerformanceMetrics> result;
    
    for (const auto& metrics : m_metricsHistory) {
        if (metrics.timestamp >= from && metrics.timestamp <= to) {
            result.append(metrics);
        }
    }
    
    return result;
}

PerformanceMetrics MetricsCollector::getAggregatedMetrics(const QDateTime& from, const QDateTime& to, AggregationType type) const
{
    QList<PerformanceMetrics> historicalData = getHistoricalMetrics(from, to);
    return aggregateMetrics(historicalData, type);
}

QVariantMap MetricsCollector::getMetricStatistics(const QString& metricName, const QDateTime& from, const QDateTime& to) const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap stats;
    QList<double> values;
    
    // 收集指定指标的值
    for (const auto& metrics : m_metricsHistory) {
        if (metrics.timestamp >= from && metrics.timestamp <= to) {
            if (metricName == "cpu") {
                values.append(metrics.system.cpuUsage);
            } else if (metricName == "memory") {
                values.append(metrics.system.memoryUsage);
            } else if (metricName == "network_latency") {
                values.append(metrics.network.latency);
            } else if (metricName == "frame_rate") {
                values.append(metrics.video.frameRate);
            }
        }
    }
    
    if (!values.isEmpty()) {
        // 计算统计信息
        std::sort(values.begin(), values.end());
        
        stats["count"] = values.size();
        stats["min"] = values.first();
        stats["max"] = values.last();
        
        double sum = 0;
        for (double value : values) {
            sum += value;
        }
        stats["average"] = sum / values.size();
        
        int medianIndex = values.size() / 2;
        stats["median"] = values.size() % 2 == 0 ? 
            (values[medianIndex - 1] + values[medianIndex]) / 2.0 : 
            values[medianIndex];
    }
    
    return stats;
}

void MetricsCollector::addCustomMetric(const QString& name, const QVariant& value, const QDateTime& timestamp)
{
    QMutexLocker locker(&m_mutex);
    
    m_customMetrics[name].append(qMakePair(timestamp, value));
    
    // 限制自定义指标的数量
    const int maxCustomMetrics = 10000;
    if (m_customMetrics[name].size() > maxCustomMetrics) {
        m_customMetrics[name].removeFirst();
    }
}

QList<QPair<QDateTime, QVariant>> MetricsCollector::getCustomMetrics(const QString& name, const QDateTime& from, const QDateTime& to) const
{
    QMutexLocker locker(&m_mutex);
    
    QList<QPair<QDateTime, QVariant>> result;
    
    if (m_customMetrics.contains(name)) {
        for (const auto& pair : m_customMetrics[name]) {
            if (pair.first >= from && pair.first <= to) {
                result.append(pair);
            }
        }
    }
    
    return result;
}

void MetricsCollector::clearHistoricalData(const QDateTime& olderThan)
{
    QMutexLocker locker(&m_mutex);
    
    QDateTime cutoff = olderThan.isValid() ? olderThan : 
        QDateTime::currentDateTime().addSecs(-m_dataRetentionHours * 3600);
    
    int removedCount = 0;
    
    // 清理主要指标历史
    auto it = m_metricsHistory.begin();
    while (it != m_metricsHistory.end()) {
        if (it->timestamp < cutoff) {
            it = m_metricsHistory.erase(it);
            removedCount++;
        } else {
            ++it;
        }
    }
    
    // 清理自定义指标
    for (auto& metricList : m_customMetrics) {
        auto customIt = metricList.begin();
        while (customIt != metricList.end()) {
            if (customIt->first < cutoff) {
                customIt = metricList.erase(customIt);
                removedCount++;
            } else {
                ++customIt;
            }
        }
    }
    
    emit dataCleanupCompleted(removedCount);
    qDebug() << "MetricsCollector: Cleaned up" << removedCount << "old data entries";
}

QVariantMap MetricsCollector::getCollectorStatistics() const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap stats;
    stats["collecting"] = m_isCollecting;
    stats["collectionInterval"] = m_collectionInterval;
    stats["dataRetentionHours"] = m_dataRetentionHours;
    stats["storageStrategy"] = QVariant::fromValue(m_storageStrategy);
    stats["metricsCount"] = m_metricsHistory.size();
    stats["customMetricsCount"] = m_customMetrics.size();
    stats["registeredMonitors"] = m_monitors.size();
    stats["dataSize"] = getDataSize();
    
    return stats;
}

qint64 MetricsCollector::getDataSize() const
{
    QMutexLocker locker(&m_mutex);
    
    // 估算内存中数据的大小
    qint64 size = 0;
    size += m_metricsHistory.size() * sizeof(PerformanceMetrics);
    
    for (const auto& metricList : m_customMetrics) {
        size += metricList.size() * (sizeof(QDateTime) + sizeof(QVariant));
    }
    
    return size;
}

int MetricsCollector::getDataCount() const
{
    QMutexLocker locker(&m_mutex);
    
    int count = m_metricsHistory.size();
    
    for (const auto& metricList : m_customMetrics) {
        count += metricList.size();
    }
    
    return count;
}

void MetricsCollector::performCollection()
{
    try {
        PerformanceMetrics metrics = collectCurrentMetrics();
        emit metricsCollected(metrics);
        
    } catch (const std::exception& e) {
        qWarning() << "MetricsCollector: Exception in performCollection:" << e.what();
        emit errorOccurred(QString("Collection failed: %1").arg(e.what()));
    }
}

void MetricsCollector::performDataCleanup()
{
    try {
        clearHistoricalData();
        
        // 检查存储空间
        qint64 availableSpace = checkStorageSpace();
        if (availableSpace < 100 * 1024 * 1024) { // < 100MB
            emit storageSpaceLow(availableSpace);
        }
        
    } catch (const std::exception& e) {
        qWarning() << "MetricsCollector: Exception in performDataCleanup:" << e.what();
        emit errorOccurred(QString("Cleanup failed: %1").arg(e.what()));
    }
}

void MetricsCollector::handleMonitorData(const QString& monitorName, const QVariantMap& data)
{
    Q_UNUSED(monitorName)
    Q_UNUSED(data)
    
    // 处理来自监控器的数据
    // 这里可以根据需要进行特殊处理
}

bool MetricsCollector::initializeStorage()
{
    switch (m_storageStrategy) {
    case MemoryOnly:
        // 仅内存存储，无需特殊初始化
        return true;
        
    case FileStorage:
    case HybridStorage:
        // 创建存储目录
        if (m_storageFilePath.isEmpty()) {
            QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir().mkpath(dataPath);
            m_storageFilePath = dataPath + "/metrics_data.bin";
        }
        return true;
        
    case DatabaseStorage:
        // 数据库存储初始化
        // 这里需要具体的数据库实现
        return true;
    }
    
    return false;
}

bool MetricsCollector::storeMetrics(const PerformanceMetrics& metrics)
{
    // 添加到内存历史
    m_metricsHistory.enqueue(metrics);
    
    // 限制内存中的数据量
    const int maxMemoryEntries = 10000;
    while (m_metricsHistory.size() > maxMemoryEntries) {
        m_metricsHistory.dequeue();
    }
    
    // 根据存储策略进行持久化
    if (m_storageStrategy == FileStorage || m_storageStrategy == HybridStorage) {
        // 这里应该实现文件存储逻辑
        // 为了简化，暂时跳过
    }
    
    emit dataStored(true);
    return true;
}

bool MetricsCollector::loadHistoricalData()
{
    if (m_storageStrategy == MemoryOnly) {
        return true;
    }
    
    // 这里应该实现从文件或数据库加载历史数据的逻辑
    // 为了简化，暂时跳过
    
    return true;
}

PerformanceMetrics MetricsCollector::aggregateMetrics(const QList<PerformanceMetrics>& metricsList, AggregationType type) const
{
    if (metricsList.isEmpty()) {
        return PerformanceMetrics();
    }
    
    PerformanceMetrics result;
    result.timestamp = QDateTime::currentDateTime();
    
    switch (type) {
    case Average: {
        double cpuSum = 0, memorySum = 0, latencySum = 0, frameRateSum = 0;
        
        for (const auto& metrics : metricsList) {
            cpuSum += metrics.system.cpuUsage;
            memorySum += metrics.system.memoryUsage;
            latencySum += metrics.network.latency;
            frameRateSum += metrics.video.frameRate;
        }
        
        int count = metricsList.size();
        result.system.cpuUsage = cpuSum / count;
        result.system.memoryUsage = memorySum / count;
        result.network.latency = latencySum / count;
        result.video.frameRate = frameRateSum / count;
        break;
    }
    
    case Maximum: {
        for (const auto& metrics : metricsList) {
            result.system.cpuUsage = qMax(result.system.cpuUsage, metrics.system.cpuUsage);
            result.system.memoryUsage = qMax(result.system.memoryUsage, metrics.system.memoryUsage);
            result.network.latency = qMax(result.network.latency, metrics.network.latency);
            result.video.frameRate = qMax(result.video.frameRate, metrics.video.frameRate);
        }
        break;
    }
    
    case Minimum: {
        result = metricsList.first();
        for (const auto& metrics : metricsList) {
            result.system.cpuUsage = qMin(result.system.cpuUsage, metrics.system.cpuUsage);
            result.system.memoryUsage = qMin(result.system.memoryUsage, metrics.system.memoryUsage);
            result.network.latency = qMin(result.network.latency, metrics.network.latency);
            result.video.frameRate = qMin(result.video.frameRate, metrics.video.frameRate);
        }
        break;
    }
    
    case Sum: {
        for (const auto& metrics : metricsList) {
            result.system.cpuUsage += metrics.system.cpuUsage;
            result.system.memoryUsage += metrics.system.memoryUsage;
            result.network.latency += metrics.network.latency;
            result.video.frameRate += metrics.video.frameRate;
        }
        break;
    }
    
    case Count:
        result.system.cpuUsage = metricsList.size();
        result.system.memoryUsage = metricsList.size();
        result.network.latency = metricsList.size();
        result.video.frameRate = metricsList.size();
        break;
    }
    
    return result;
}

qint64 MetricsCollector::checkStorageSpace() const
{
    if (m_storageFilePath.isEmpty()) {
        return -1;
    }
    
    QStorageInfo storage(QFileInfo(m_storageFilePath).absolutePath());
    return storage.bytesAvailable();
}