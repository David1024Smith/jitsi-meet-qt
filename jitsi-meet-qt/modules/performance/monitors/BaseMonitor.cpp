#include "BaseMonitor.h"
#include <QTimer>
#include <QDebug>
#include <QMutexLocker>

BaseMonitor::BaseMonitor(const QString& monitorName, QObject* parent)
    : QObject(parent)
    , IResourceTracker()
    , m_monitorName(monitorName)
    , m_status(Stopped)
    , m_collectionTimer(new QTimer(this))
    , m_cleanupTimer(new QTimer(this))
    , m_trackingInterval(5000)  // 默认5秒
    , m_dataRetentionHours(24)  // 默认保留24小时
    , m_totalCollections(0)
    , m_successfulCollections(0)
    , m_failedCollections(0)
{
    // 配置数据收集定时器
    m_collectionTimer->setSingleShot(false);
    connect(m_collectionTimer, &QTimer::timeout, this, &BaseMonitor::performDataCollection);
    
    // 配置数据清理定时器
    m_cleanupTimer->setSingleShot(false);
    m_cleanupTimer->setInterval(3600000); // 每小时清理一次
    connect(m_cleanupTimer, &QTimer::timeout, this, &BaseMonitor::performDataCleanup);
    
    m_startTime = QDateTime::currentDateTime();
}

BaseMonitor::~BaseMonitor()
{
    stopTracking();
}

QString BaseMonitor::monitorName() const
{
    return m_monitorName;
}

QString BaseMonitor::version() const
{
    return "1.0.0";
}

QString BaseMonitor::description() const
{
    return QString("Base monitor for %1").arg(m_monitorName);
}

BaseMonitor::MonitorStatus BaseMonitor::status() const
{
    QMutexLocker locker(&m_mutex);
    return m_status;
}

bool BaseMonitor::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != Stopped) {
        return false;
    }
    
    setStatus(Starting);
    
    try {
        if (!initializeMonitor()) {
            setStatus(Error);
            addError("Failed to initialize monitor-specific functionality");
            return false;
        }
        
        setStatus(Stopped);
        return true;
        
    } catch (const std::exception& e) {
        setStatus(Error);
        addError(QString("Initialization failed: %1").arg(e.what()));
        return false;
    }
}

bool BaseMonitor::startTracking()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status == Running) {
        return true;
    }
    
    if (m_status != Stopped) {
        return false;
    }
    
    setStatus(Starting);
    
    try {
        // 启动数据收集
        m_collectionTimer->setInterval(m_trackingInterval);
        m_collectionTimer->start();
        
        // 启动数据清理
        m_cleanupTimer->start();
        
        setStatus(Running);
        return true;
        
    } catch (const std::exception& e) {
        setStatus(Error);
        addError(QString("Failed to start tracking: %1").arg(e.what()));
        return false;
    }
}

void BaseMonitor::stopTracking()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status == Stopped) {
        return;
    }
    
    m_collectionTimer->stop();
    m_cleanupTimer->stop();
    
    setStatus(Stopped);
}

bool BaseMonitor::isTracking() const
{
    QMutexLocker locker(&m_mutex);
    return m_status == Running;
}

ResourceUsage BaseMonitor::getCurrentUsage(ResourceType type) const
{
    Q_UNUSED(type)
    
    try {
        // 调用子类实现的收集方法
        return const_cast<BaseMonitor*>(this)->collectResourceUsage();
    } catch (const std::exception& e) {
        const_cast<BaseMonitor*>(this)->addError(QString("Failed to collect resource usage: %1").arg(e.what()));
        return ResourceUsage();
    }
}

QList<ResourceUsage> BaseMonitor::getHistoricalUsage(ResourceType type, const QDateTime& from, const QDateTime& to) const
{
    return filterHistoricalData(type, from, to);
}

ResourceUsage BaseMonitor::getPeakUsage(ResourceType type, const QDateTime& from, const QDateTime& to) const
{
    QList<ResourceUsage> filteredData = filterHistoricalData(type, from, to);
    return calculatePeakUsage(filteredData, type);
}

ResourceUsage BaseMonitor::getAverageUsage(ResourceType type, const QDateTime& from, const QDateTime& to) const
{
    QList<ResourceUsage> filteredData = filterHistoricalData(type, from, to);
    return calculateAverageUsage(filteredData, type);
}

void BaseMonitor::setTrackingInterval(int interval)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_trackingInterval != interval) {
        m_trackingInterval = interval;
        
        if (m_status == Running) {
            m_collectionTimer->setInterval(interval);
        }
    }
}

int BaseMonitor::trackingInterval() const
{
    QMutexLocker locker(&m_mutex);
    return m_trackingInterval;
}

void BaseMonitor::setResourceThreshold(ResourceType type, double threshold)
{
    QMutexLocker locker(&m_mutex);
    m_thresholds[type] = threshold;
}

double BaseMonitor::resourceThreshold(ResourceType type) const
{
    QMutexLocker locker(&m_mutex);
    return m_thresholds.value(type, 0.0);
}

bool BaseMonitor::isThresholdExceeded(ResourceType type) const
{
    QMutexLocker locker(&m_mutex);
    
    double threshold = m_thresholds.value(type, 0.0);
    if (threshold <= 0.0) {
        return false;
    }
    
    ResourceUsage usage = getCurrentUsage(type);
    double currentValue = getResourceValue(usage, type);
    
    return currentValue > threshold;
}

QVariantMap BaseMonitor::getSystemInfo() const
{
    QVariantMap info;
    info["monitorName"] = m_monitorName;
    info["version"] = version();
    info["description"] = description();
    info["supportedTypes"] = QVariant::fromValue(supportedResourceTypes());
    return info;
}

QVariantMap BaseMonitor::getTrackerStatus() const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap status;
    status["status"] = static_cast<int>(m_status);
    status["isTracking"] = (m_status == Running);
    status["trackingInterval"] = m_trackingInterval;
    status["dataRetentionHours"] = m_dataRetentionHours;
    status["historicalDataCount"] = m_historicalData.size();
    status["errorCount"] = m_errors.size();
    
    return status;
}

void BaseMonitor::reset()
{
    QMutexLocker locker(&m_mutex);
    
    m_historicalData.clear();
    m_errors.clear();
    m_totalCollections = 0;
    m_successfulCollections = 0;
    m_failedCollections = 0;
    m_startTime = QDateTime::currentDateTime();
}

void BaseMonitor::clearHistoricalData(const QDateTime& olderThan)
{
    QMutexLocker locker(&m_mutex);
    
    QDateTime cutoffTime = olderThan.isValid() ? olderThan : QDateTime::currentDateTime().addSecs(-m_dataRetentionHours * 3600);
    
    int originalCount = m_historicalData.size();
    
    auto it = std::remove_if(m_historicalData.begin(), m_historicalData.end(),
                            [cutoffTime](const ResourceUsage& usage) {
                                return usage.timestamp < cutoffTime;
                            });
    
    m_historicalData.erase(it, m_historicalData.end());
    
    int removedCount = originalCount - m_historicalData.size();
    if (removedCount > 0) {
        emit dataCleanupCompleted(removedCount);
    }
}

void BaseMonitor::pause()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status == Running) {
        m_collectionTimer->stop();
        setStatus(Paused);
    }
}

void BaseMonitor::resume()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status == Paused) {
        m_collectionTimer->start();
        setStatus(Running);
    }
}

bool BaseMonitor::isPaused() const
{
    QMutexLocker locker(&m_mutex);
    return m_status == Paused;
}

void BaseMonitor::setDataRetentionHours(int hours)
{
    QMutexLocker locker(&m_mutex);
    m_dataRetentionHours = hours;
}

int BaseMonitor::dataRetentionHours() const
{
    QMutexLocker locker(&m_mutex);
    return m_dataRetentionHours;
}

QVariantMap BaseMonitor::getMonitorStatistics() const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap stats;
    stats["totalCollections"] = m_totalCollections;
    stats["successfulCollections"] = m_successfulCollections;
    stats["failedCollections"] = m_failedCollections;
    stats["successRate"] = m_totalCollections > 0 ? (double)m_successfulCollections / m_totalCollections : 0.0;
    stats["startTime"] = m_startTime;
    stats["lastCollectionTime"] = m_lastCollectionTime;
    stats["uptime"] = m_startTime.secsTo(QDateTime::currentDateTime());
    
    return stats;
}

QStringList BaseMonitor::getErrors() const
{
    QMutexLocker locker(&m_mutex);
    return m_errors;
}

void BaseMonitor::clearErrors()
{
    QMutexLocker locker(&m_mutex);
    m_errors.clear();
}

void BaseMonitor::setStatus(MonitorStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void BaseMonitor::addError(const QString& error)
{
    m_errors.append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString(), error));
    
    // 限制错误列表大小
    if (m_errors.size() > 100) {
        m_errors.removeFirst();
    }
    
    emit errorOccurred(error);
}

void BaseMonitor::storeUsageData(const ResourceUsage& usage)
{
    QMutexLocker locker(&m_mutex);
    
    m_historicalData.append(usage);
    m_lastCollectionTime = usage.timestamp;
    
    // 限制历史数据大小
    if (m_historicalData.size() > 10000) {
        m_historicalData.removeFirst();
    }
    
    emit dataUpdated(usage);
}

void BaseMonitor::checkThresholds(const ResourceUsage& usage)
{
    for (auto it = m_thresholds.begin(); it != m_thresholds.end(); ++it) {
        ResourceType type = it.key();
        double threshold = it.value();
        double currentValue = getResourceValue(usage, type);
        
        if (currentValue > threshold) {
            emit thresholdExceeded(type, currentValue, threshold);
        }
    }
}

double BaseMonitor::getResourceValue(const ResourceUsage& usage, ResourceType type) const
{
    switch (type) {
    case CPU:
        return usage.cpu.totalUsage;
    case Memory:
        return usage.memory.usagePercentage;
    case Network:
        return (usage.network.receiveSpeed + usage.network.sendSpeed) / 2.0;
    case Disk:
        return usage.disk.usagePercentage;
    case All:
    default:
        return (usage.cpu.totalUsage + usage.memory.usagePercentage + 
                usage.disk.usagePercentage + 
                (usage.network.receiveSpeed + usage.network.sendSpeed) / 2.0) / 4.0;
    }
}

void BaseMonitor::performDataCollection()
{
    if (m_status != Running) {
        return;
    }
    
    try {
        m_totalCollections++;
        
        ResourceUsage usage = collectResourceUsage();
        usage.timestamp = QDateTime::currentDateTime();
        
        storeUsageData(usage);
        checkThresholds(usage);
        
        m_successfulCollections++;
        
    } catch (const std::exception& e) {
        m_failedCollections++;
        addError(QString("Data collection failed: %1").arg(e.what()));
    }
}

void BaseMonitor::performDataCleanup()
{
    clearHistoricalData();
}

ResourceUsage BaseMonitor::calculatePeakUsage(const QList<ResourceUsage>& usageList, ResourceType type) const
{
    if (usageList.isEmpty()) {
        return ResourceUsage();
    }
    
    ResourceUsage peak = usageList.first();
    double peakValue = getResourceValue(peak, type);
    
    for (const ResourceUsage& usage : usageList) {
        double currentValue = getResourceValue(usage, type);
        if (currentValue > peakValue) {
            peak = usage;
            peakValue = currentValue;
        }
    }
    
    return peak;
}

ResourceUsage BaseMonitor::calculateAverageUsage(const QList<ResourceUsage>& usageList, ResourceType type) const
{
    if (usageList.isEmpty()) {
        return ResourceUsage();
    }
    
    ResourceUsage average;
    double totalCpu = 0, totalMemory = 0, totalNetworkReceive = 0, totalNetworkSend = 0, totalDisk = 0;
    
    for (const ResourceUsage& usage : usageList) {
        totalCpu += usage.cpu.totalUsage;
        totalMemory += usage.memory.usagePercentage;
        totalNetworkReceive += usage.network.receiveSpeed;
        totalNetworkSend += usage.network.sendSpeed;
        totalDisk += usage.disk.usagePercentage;
    }
    
    int count = usageList.size();
    average.cpu.totalUsage = totalCpu / count;
    average.memory.usagePercentage = totalMemory / count;
    average.network.receiveSpeed = totalNetworkReceive / count;
    average.network.sendSpeed = totalNetworkSend / count;
    average.disk.usagePercentage = totalDisk / count;
    average.timestamp = QDateTime::currentDateTime();
    
    return average;
}

QList<ResourceUsage> BaseMonitor::filterHistoricalData(ResourceType type, const QDateTime& from, const QDateTime& to) const
{
    Q_UNUSED(type)
    
    QMutexLocker locker(&m_mutex);
    
    QList<ResourceUsage> filtered;
    
    for (const ResourceUsage& usage : m_historicalData) {
        if (usage.timestamp >= from && usage.timestamp <= to) {
            filtered.append(usage);
        }
    }
    
    return filtered;
}