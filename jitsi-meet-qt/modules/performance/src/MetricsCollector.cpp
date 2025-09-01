#include "../include/MetricsCollector.h"
#include <QDebug>
#include <QMutexLocker>
#include <QRandomGenerator>
#include <QDateTime>
#include <QVariantMap>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <stdexcept>

// 简化的 MetricsCollector 实现，只包含基本功能

/**
 * @brief MetricsCollector构造函数
 * @param parent 父对象
 */
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
    qDebug() << "MetricsCollector: 构造函数调用";
    
    // 设置定时器
    m_collectionTimer->setSingleShot(false);
    m_cleanupTimer->setSingleShot(false);
    
    // 连接信号
    connect(m_collectionTimer, &QTimer::timeout, this, &MetricsCollector::performCollection);
    connect(m_cleanupTimer, &QTimer::timeout, this, &MetricsCollector::performDataCleanup);
    
    // 设置清理定时器为每小时执行一次
    m_cleanupTimer->setInterval(3600000); // 1 hour
}

/**
 * @brief MetricsCollector析构函数
 */
MetricsCollector::~MetricsCollector()
{
    stop();
    qDebug() << "MetricsCollector: 析构函数调用";
}

/**
 * @brief 初始化指标收集器
 * @return 初始化是否成功
 */
bool MetricsCollector::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    try {
        // 初始化存储
        if (!initializeStorage()) {
            qCritical() << "MetricsCollector: 存储初始化失败";
            return false;
        }
        
        // 加载历史数据
        if (!loadHistoricalData()) {
            qWarning() << "MetricsCollector: 历史数据加载失败";
        }
        
        qDebug() << "MetricsCollector: 初始化成功";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: 初始化时发生异常:" << e.what();
        emit errorOccurred(QString("初始化失败: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief 设置配置
 * @param config 配置对象
 */
void MetricsCollector::setConfig(PerformanceConfig* config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
    qDebug() << "MetricsCollector: 配置已设置";
}

/**
 * @brief 获取配置
 * @return 配置对象
 */
PerformanceConfig* MetricsCollector::config() const
{
    QMutexLocker locker(&m_mutex);
    return m_config;
}

/**
 * @brief 检查是否正在收集
 * @return 是否正在收集
 */
bool MetricsCollector::isCollecting() const
{
    QMutexLocker locker(&m_mutex);
    return m_isCollecting;
}

/**
 * @brief 启动指标收集
 * @return 启动是否成功
 */
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
        qDebug() << "MetricsCollector: 开始收集指标";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: 启动时发生异常:" << e.what();
        emit errorOccurred(QString("启动失败: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief 停止指标收集
 */
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
        qDebug() << "MetricsCollector: 停止收集指标";
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: 停止时发生异常:" << e.what();
        emit errorOccurred(QString("停止失败: %1").arg(e.what()));
    }
}

/**
 * @brief 清除历史数据
 * @param olderThan 清除早于此时间的数据
 */
void MetricsCollector::clearHistoricalData(const QDateTime& olderThan)
{
    QMutexLocker locker(&m_mutex);
    
    try {
        if (olderThan.isValid()) {
            // 清除早于指定时间的数据
            QQueue<PerformanceMetrics> filteredHistory;
            while (!m_metricsHistory.isEmpty()) {
                PerformanceMetrics metrics = m_metricsHistory.dequeue();
                if (metrics.timestamp >= olderThan) {
                    filteredHistory.enqueue(metrics);
                }
            }
            m_metricsHistory = filteredHistory;
            
            // 清除自定义指标中的旧数据
            for (auto it = m_customMetrics.begin(); it != m_customMetrics.end(); ++it) {
                QList<QPair<QDateTime, QVariant>> filteredMetrics;
                for (const auto& metric : it.value()) {
                    if (metric.first >= olderThan) {
                        filteredMetrics.append(metric);
                    }
                }
                it.value() = filteredMetrics;
            }
        } else {
            // 清除所有数据
            m_metricsHistory.clear();
            m_customMetrics.clear();
        }
        
        qDebug() << "MetricsCollector: 历史数据已清除";
        // emit dataCleanupCompleted(removedCount); // 可以在需要时添加计数
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: 清除历史数据时发生异常:" << e.what();
        emit errorOccurred(QString("清除数据失败: %1").arg(e.what()));
    }
}

/**
 * @brief 获取收集器统计信息
 * @return 统计信息映射
 */
QVariantMap MetricsCollector::getCollectorStatistics() const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap stats;
    
    try {
        stats["isCollecting"] = m_isCollecting;
        stats["collectionInterval"] = m_collectionInterval;
        stats["dataRetentionHours"] = m_dataRetentionHours;
        stats["maxStorageSize"] = m_maxStorageSize;
        stats["currentDataCount"] = m_metricsHistory.size();
        stats["customMetricsCount"] = m_customMetrics.size();
        stats["storageStrategy"] = static_cast<int>(m_storageStrategy);
        
        // 计算内存使用量
        qint64 memoryUsage = m_metricsHistory.size() * sizeof(PerformanceMetrics);
        for (const auto& metricList : m_customMetrics) {
            memoryUsage += metricList.size() * sizeof(QPair<QDateTime, QVariant>);
        }
        stats["memoryUsage"] = memoryUsage;
        
        qDebug() << "MetricsCollector: 获取统计信息成功";
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: 获取统计信息时发生异常:" << e.what();
    }
    
    return stats;
}

void MetricsCollector::performCollection()
{
    qDebug() << "MetricsCollector: 执行数据收集";
    
    try {
        // 创建新的性能指标对象
        PerformanceMetrics metrics;
        metrics.timestamp = QDateTime::currentDateTime();
        
        // 简单的模拟数据收集
         metrics.cpuUsage = QRandomGenerator::global()->bounded(100.0);
         metrics.memoryUsage = QRandomGenerator::global()->bounded(static_cast<qint64>(8 * 1024 * 1024 * 1024LL));
        
        // 添加到历史记录
        {
            QMutexLocker locker(&m_mutex);
            m_metricsHistory.enqueue(metrics);
            
            // 限制历史记录大小
            while (m_metricsHistory.size() > 1000) {
                m_metricsHistory.dequeue();
            }
        }
        
        emit metricsCollected(metrics);
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: 收集数据时发生异常:" << e.what();
        emit errorOccurred(QString("数据收集失败: %1").arg(e.what()));
    }
}

void MetricsCollector::performDataCleanup()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_dataRetentionHours <= 0) {
        return;
    }
    
    QDateTime cutoffTime = QDateTime::currentDateTime().addSecs(-m_dataRetentionHours * 3600);
    int removedCount = 0;
    
    // 清理过期的历史数据
    while (!m_metricsHistory.isEmpty() && m_metricsHistory.head().timestamp < cutoffTime) {
        m_metricsHistory.dequeue();
        removedCount++;
    }
    
    // 清理过期的自定义指标
    for (auto it = m_customMetrics.begin(); it != m_customMetrics.end(); ++it) {
        auto& metricList = it.value();
        auto newEnd = std::remove_if(metricList.begin(), metricList.end(),
            [cutoffTime](const QPair<QDateTime, QVariant>& metric) {
                return metric.first < cutoffTime;
            });
        int removed = metricList.end() - newEnd;
        metricList.erase(newEnd, metricList.end());
        removedCount += removed;
    }
    
    if (removedCount > 0) {
        qDebug() << "MetricsCollector: 清理了" << removedCount << "条过期数据";
        emit dataCleanupCompleted(removedCount);
    }
}

void MetricsCollector::handleMonitorData(const QString& monitorName, const QVariantMap& data)
{
    QMutexLocker locker(&m_mutex);
    
    // 处理来自监控器的数据
    qDebug() << "MetricsCollector: 接收到监控器数据:" << monitorName << "数据项数:" << data.size();
    
    // 将监控器数据转换为自定义指标
    QDateTime timestamp = QDateTime::currentDateTime();
    for (auto it = data.begin(); it != data.end(); ++it) {
        QString metricName = QString("%1_%2").arg(monitorName, it.key());
        addCustomMetric(metricName, it.value(), timestamp);
    }
    
    // 检查存储空间
    qint64 availableSpace = checkStorageSpace();
    if (availableSpace < 100 * 1024 * 1024) { // 小于100MB
        emit storageSpaceLow(availableSpace);
    }
}

/**
 * @brief 初始化存储
 * @return 初始化是否成功
 */
bool MetricsCollector::initializeStorage()
{
    try {
        // 根据存储策略初始化
        switch (m_storageStrategy) {
            case MemoryOnly:
                qDebug() << "MetricsCollector: 使用内存存储策略";
                break;
            case FileStorage:
                // 创建存储目录
                if (!m_storageFilePath.isEmpty()) {
                    QDir dir(QFileInfo(m_storageFilePath).absolutePath());
                    if (!dir.exists()) {
                        dir.mkpath(".");
                    }
                }
                qDebug() << "MetricsCollector: 使用文件存储策略";
                break;
            case DatabaseStorage:
                qDebug() << "MetricsCollector: 使用数据库存储策略";
                break;
            case HybridStorage:
                qDebug() << "MetricsCollector: 使用混合存储策略";
                break;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: 存储初始化失败:" << e.what();
        return false;
    }
}

/**
 * @brief 加载历史数据
 * @return 加载是否成功
 */
bool MetricsCollector::loadHistoricalData()
{
    try {
        // 根据存储策略加载数据
        switch (m_storageStrategy) {
            case MemoryOnly:
                // 内存存储无需加载
                break;
            case FileStorage:
                if (!m_storageFilePath.isEmpty() && QFile::exists(m_storageFilePath)) {
                    // 简化的文件加载逻辑
                    qDebug() << "MetricsCollector: 从文件加载历史数据:" << m_storageFilePath;
                }
                break;
            case DatabaseStorage:
                qDebug() << "MetricsCollector: 从数据库加载历史数据";
                break;
            case HybridStorage:
                qDebug() << "MetricsCollector: 从混合存储加载历史数据";
                break;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: 历史数据加载失败:" << e.what();
        return false;
    }
}

PerformanceMetrics MetricsCollector::collectCurrentMetrics()
{
    PerformanceMetrics metrics;
    metrics.timestamp = QDateTime::currentDateTime();
    
    // 模拟系统指标
    metrics.system.cpuUsage = 25.0;
    metrics.system.memoryUsage = 60.0;
    metrics.system.memoryUsed = 1024LL * 1024LL * 512LL; // 512MB
    metrics.system.memoryTotal = 1024LL * 1024LL * 1024LL * 8LL; // 8GB
    
    // 模拟网络指标
    metrics.network.latency = 50.0;
    metrics.network.bandwidth = 100.0;
    
    // 兼容性字段
    metrics.cpuUsage = metrics.system.cpuUsage;
    metrics.memoryUsage = metrics.system.memoryUsage;
    metrics.networkLatency = metrics.network.latency;
    
    return metrics;
}

PerformanceMetrics MetricsCollector::getLatestMetrics() const
{
    return const_cast<MetricsCollector*>(this)->collectCurrentMetrics();
}

QList<PerformanceMetrics> MetricsCollector::getHistoricalMetrics(const QDateTime& from, const QDateTime& to) const
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    
    QList<PerformanceMetrics> history;
    // 返回空列表，简化实现
    return history;
}

PerformanceMetrics MetricsCollector::getAggregatedMetrics(const QDateTime& from, const QDateTime& to, AggregationType type) const
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    Q_UNUSED(type)
    
    // 返回当前指标作为聚合结果
    return const_cast<MetricsCollector*>(this)->collectCurrentMetrics();
}

/**
 * @brief 添加自定义指标
 * @param name 指标名称
 * @param value 指标值
 * @param timestamp 时间戳
 */
void MetricsCollector::addCustomMetric(const QString& name, const QVariant& value, const QDateTime& timestamp)
{
    QMutexLocker locker(&m_mutex);
    
    if (name.isEmpty()) {
        qWarning() << "MetricsCollector: 指标名称不能为空";
        return;
    }
    
    QDateTime actualTimestamp = timestamp.isValid() ? timestamp : QDateTime::currentDateTime();
    
    // 添加到自定义指标映射中
    m_customMetrics[name].append(qMakePair(actualTimestamp, value));
    
    // 限制每个指标的历史记录数量
    auto& metricList = m_customMetrics[name];
    while (metricList.size() > 1000) {
        metricList.removeFirst();
    }
    
    qDebug() << "MetricsCollector: 添加自定义指标:" << name << "值:" << value << "时间:" << actualTimestamp;
}



/**
 * @brief 检查存储空间
 * @return 可用空间(字节)
 */
qint64 MetricsCollector::checkStorageSpace() const
{
    try {
        // 简化的存储空间检查
        qint64 currentSize = m_metricsHistory.size() * sizeof(PerformanceMetrics);
        
        if (currentSize > m_maxStorageSize) {
            qWarning() << "MetricsCollector: 存储空间不足，当前大小:" << currentSize << "最大限制:" << m_maxStorageSize;
            const_cast<MetricsCollector*>(this)->storageSpaceLow(m_maxStorageSize - currentSize);
        }
        
        return m_maxStorageSize - currentSize;
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsCollector: 检查存储空间时发生异常:" << e.what();
        return 0;
    }
}