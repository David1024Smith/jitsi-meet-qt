#include "ModuleResourceManager.h"
#include <QDebug>
#include <QMutexLocker>
#include <QReadLocker>
#include <QWriteLocker>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

ModuleResourceManager* ModuleResourceManager::s_instance = nullptr;
QMutex ModuleResourceManager::s_mutex;

ModuleResourceManager* ModuleResourceManager::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new ModuleResourceManager();
    }
    return s_instance;
}

ModuleResourceManager::ModuleResourceManager(QObject* parent)
    : QObject(parent)
    , m_maxMemoryUsage(512 * 1024 * 1024)  // 512MB default
    , m_currentMemoryUsage(0)
    , m_cleanupInterval(300000)  // 5 minutes
    , m_autoCleanupEnabled(true)
    , m_cleanupTimer(new QTimer(this))
    , m_memoryCheckTimer(new QTimer(this))
    , m_totalAllocations(0)
    , m_totalDeallocations(0)
    , m_peakMemoryUsage(0)
{
    initializeSystem();
}

ModuleResourceManager::~ModuleResourceManager()
{
    shutdownSystem();
}

void ModuleResourceManager::initializeSystem()
{
    // 初始化缓存
    m_cache.setMaxCost(100 * 1024 * 1024);  // 100MB cache
    
    // 初始化统计信息
    m_cacheStats = CacheStatistics();
    m_cacheStats.maxSize = 100 * 1024 * 1024;
    m_cacheStats.maxItems = 10000;
    
    // 配置定时器
    m_cleanupTimer->setSingleShot(false);
    m_cleanupTimer->setInterval(m_cleanupInterval);
    connect(m_cleanupTimer, &QTimer::timeout, this, &ModuleResourceManager::performCleanup);
    
    m_memoryCheckTimer->setSingleShot(false);
    m_memoryCheckTimer->setInterval(30000);  // 30秒检查一次内存
    connect(m_memoryCheckTimer, &QTimer::timeout, this, &ModuleResourceManager::checkMemoryUsage);
    
    if (m_autoCleanupEnabled) {
        startCleanupTimer();
    }
    
    qDebug() << "ModuleResourceManager initialized";
}

void ModuleResourceManager::shutdownSystem()
{
    stopCleanupTimer();
    
    // 清理所有资源
    {
        QWriteLocker locker(&m_resourceLock);
        m_resources.clear();
    }
    
    // 清理缓存
    m_cache.clear();
    
    // 清理资源池
    {
        QWriteLocker locker(&m_poolLock);
        m_resourcePools.clear();
    }
    
    // 清理共享对象
    {
        QWriteLocker locker(&m_sharedObjectLock);
        m_sharedObjects.clear();
    }
    
    qDebug() << "ModuleResourceManager shutdown completed";
}

void ModuleResourceManager::initialize()
{
    // 公共初始化接口
    if (m_autoCleanupEnabled) {
        startCleanupTimer();
    }
}

void ModuleResourceManager::shutdown()
{
    shutdownSystem();
}

bool ModuleResourceManager::storeResource(const QString& resourceId, const QVariant& data, 
                                         ResourceType type, const QString& moduleName)
{
    if (resourceId.isEmpty()) {
        return false;
    }
    
    QString key = generateResourceKey(resourceId, moduleName);
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    ResourceInfo info;
    info.id = resourceId;
    info.moduleName = moduleName;
    info.type = type;
    info.data = data;
    info.size = calculateResourceSize(data);
    info.createTime = currentTime;
    info.lastAccessTime = currentTime;
    info.accessCount = 1;
    info.cachePolicy = (type == TempResource) ? TTL : LRU;
    info.ttl = (type == TempResource) ? 300000 : 0;  // 5分钟TTL for temp resources
    
    {
        QWriteLocker locker(&m_resourceLock);
        m_resources[key] = info;
    }
    
    // 更新内存使用统计
    m_currentMemoryUsage += info.size;
    m_totalAllocations++;
    
    if (m_currentMemoryUsage > m_peakMemoryUsage) {
        m_peakMemoryUsage = m_currentMemoryUsage;
    }
    
    // 检查是否需要缓存
    if (shouldCache(info)) {
        m_cache.insert(key, new QVariant(data), info.size);
        updateCacheStatistics();
    }
    
    emit resourceAdded(resourceId, moduleName);
    
    // 检查内存限制
    checkMemoryLimits();
    
    qDebug() << "Resource stored:" << key << "Size:" << info.size;
    return true;
}

QVariant ModuleResourceManager::getResource(const QString& resourceId, const QString& moduleName)
{
    if (resourceId.isEmpty()) {
        return QVariant();
    }
    
    QString key = generateResourceKey(resourceId, moduleName);
    
    // 首先检查缓存
    if (QVariant* cachedData = m_cache.object(key)) {
        {
            QMutexLocker locker(&m_cacheStatsLock);
            m_cacheStats.hitCount++;
        }
        
        // 更新访问信息
        {
            QWriteLocker locker(&m_resourceLock);
            if (m_resources.contains(key)) {
                updateAccessInfo(m_resources[key]);
            }
        }
        
        emit resourceAccessed(resourceId, moduleName);
        return *cachedData;
    }
    
    // 从主存储获取
    QReadLocker locker(&m_resourceLock);
    if (m_resources.contains(key)) {
        ResourceInfo& info = const_cast<ResourceInfo&>(m_resources[key]);
        
        // 检查TTL
        if (info.ttl > 0) {
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            if (currentTime - info.createTime > info.ttl) {
                locker.unlock();
                removeResource(resourceId, moduleName);
                return QVariant();
            }
        }
        
        updateAccessInfo(info);
        
        // 添加到缓存
        if (shouldCache(info)) {
            m_cache.insert(key, new QVariant(info.data), info.size);
        }
        
        {
            QMutexLocker statsLocker(&m_cacheStatsLock);
            m_cacheStats.missCount++;
        }
        
        emit resourceAccessed(resourceId, moduleName);
        return info.data;
    }
    
    {
        QMutexLocker statsLocker(&m_cacheStatsLock);
        m_cacheStats.missCount++;
    }
    
    return QVariant();
}

bool ModuleResourceManager::hasResource(const QString& resourceId, const QString& moduleName) const
{
    if (resourceId.isEmpty()) {
        return false;
    }
    
    QString key = generateResourceKey(resourceId, moduleName);
    
    // 检查缓存
    if (m_cache.contains(key)) {
        return true;
    }
    
    // 检查主存储
    QReadLocker locker(&m_resourceLock);
    return m_resources.contains(key);
}

bool ModuleResourceManager::removeResource(const QString& resourceId, const QString& moduleName)
{
    if (resourceId.isEmpty()) {
        return false;
    }
    
    QString key = generateResourceKey(resourceId, moduleName);
    
    // 从缓存移除
    m_cache.remove(key);
    
    // 从主存储移除
    QWriteLocker locker(&m_resourceLock);
    if (m_resources.contains(key)) {
        ResourceInfo info = m_resources.take(key);
        m_currentMemoryUsage -= info.size;
        m_totalDeallocations++;
        
        emit resourceRemoved(resourceId, moduleName);
        
        qDebug() << "Resource removed:" << key;
        return true;
    }
    
    return false;
}

void ModuleResourceManager::setCachePolicy(const QString& resourceId, CachePolicy policy)
{
    QWriteLocker locker(&m_resourceLock);
    
    for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
        if (it.value().id == resourceId) {
            it.value().cachePolicy = policy;
        }
    }
}

void ModuleResourceManager::setCacheTTL(const QString& resourceId, qint64 ttlMs)
{
    QWriteLocker locker(&m_resourceLock);
    
    for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
        if (it.value().id == resourceId) {
            it.value().ttl = ttlMs;
        }
    }
}

void ModuleResourceManager::setCacheMaxSize(qint64 maxSizeBytes)
{
    m_cache.setMaxCost(maxSizeBytes);
    
    QMutexLocker locker(&m_cacheStatsLock);
    m_cacheStats.maxSize = maxSizeBytes;
}

void ModuleResourceManager::setCacheMaxItems(int maxItems)
{
    QMutexLocker locker(&m_cacheStatsLock);
    m_cacheStats.maxItems = maxItems;
}

void ModuleResourceManager::clearCache(const QString& moduleName)
{
    if (moduleName.isEmpty()) {
        m_cache.clear();
        qDebug() << "All cache cleared";
    } else {
        // 清理特定模块的缓存
        QReadLocker locker(&m_resourceLock);
        for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
            if (it.value().moduleName == moduleName) {
                m_cache.remove(it.key());
            }
        }
        qDebug() << "Cache cleared for module:" << moduleName;
    }
    
    updateCacheStatistics();
}

void ModuleResourceManager::optimizeCache()
{
    // 执行缓存优化
    evictExpiredResources();
    
    // 根据策略清理缓存
    QReadLocker locker(&m_resourceLock);
    for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
        const ResourceInfo& info = it.value();
        
        switch (info.cachePolicy) {
        case LRU:
            evictLRUResources();
            break;
        case LFU:
            evictLFUResources();
            break;
        case Adaptive:
            applyAdaptiveEviction();
            break;
        default:
            break;
        }
    }
    
    updateCacheStatistics();
    qDebug() << "Cache optimization completed";
}

QString ModuleResourceManager::createResourcePool(const QString& poolId, const QString& description, int maxSize)
{
    QWriteLocker locker(&m_poolLock);
    
    if (m_resourcePools.contains(poolId)) {
        qWarning() << "Resource pool already exists:" << poolId;
        return QString();
    }
    
    ResourcePool pool;
    pool.poolId = poolId;
    pool.description = description;
    pool.maxSize = maxSize;
    
    m_resourcePools[poolId] = pool;
    
    emit poolCreated(poolId);
    
    qDebug() << "Resource pool created:" << poolId << "Max size:" << maxSize;
    return poolId;
}

bool ModuleResourceManager::destroyResourcePool(const QString& poolId)
{
    QWriteLocker locker(&m_poolLock);
    
    if (!m_resourcePools.contains(poolId)) {
        return false;
    }
    
    ResourcePool& pool = m_resourcePools[poolId];
    
    // 清理池中的对象
    {
        QWriteLocker poolLocker(&pool.lock);
        pool.objects.clear();
    }
    
    m_resourcePools.remove(poolId);
    
    emit poolDestroyed(poolId);
    
    qDebug() << "Resource pool destroyed:" << poolId;
    return true;
}

QSharedPointer<QObject> ModuleResourceManager::acquireFromPool(const QString& poolId, const QString& objectType)
{
    QReadLocker locker(&m_poolLock);
    
    if (!m_resourcePools.contains(poolId)) {
        return QSharedPointer<QObject>();
    }
    
    ResourcePool& pool = m_resourcePools[poolId];
    
    QWriteLocker poolLocker(&pool.lock);
    
    // 查找可用对象
    for (auto it = pool.objects.begin(); it != pool.objects.end(); ++it) {
        if (it.key().startsWith(objectType)) {
            QSharedPointer<QObject> object = it.value();
            pool.objects.erase(it);
            pool.currentSize--;
            pool.totalAllocations++;
            
            qDebug() << "Object acquired from pool:" << poolId << objectType;
            return object;
        }
    }
    
    // 没有可用对象
    return QSharedPointer<QObject>();
}

bool ModuleResourceManager::releaseToPool(const QString& poolId, QSharedPointer<QObject> object)
{
    if (!object) {
        return false;
    }
    
    QReadLocker locker(&m_poolLock);
    
    if (!m_resourcePools.contains(poolId)) {
        return false;
    }
    
    ResourcePool& pool = m_resourcePools[poolId];
    
    QWriteLocker poolLocker(&pool.lock);
    
    // 检查池容量
    if (pool.currentSize >= pool.maxSize) {
        // 池已满，直接丢弃对象
        return false;
    }
    
    QString objectKey = QString("%1_%2").arg(object->metaObject()->className())
                                       .arg(QDateTime::currentMSecsSinceEpoch());
    
    pool.objects[objectKey] = object;
    pool.currentSize++;
    pool.totalDeallocations++;
    
    qDebug() << "Object released to pool:" << poolId << object->metaObject()->className();
    return true;
}

void ModuleResourceManager::clearResourcePool(const QString& poolId)
{
    QReadLocker locker(&m_poolLock);
    
    if (!m_resourcePools.contains(poolId)) {
        return;
    }
    
    ResourcePool& pool = m_resourcePools[poolId];
    
    QWriteLocker poolLocker(&pool.lock);
    pool.objects.clear();
    pool.currentSize = 0;
    
    qDebug() << "Resource pool cleared:" << poolId;
}

bool ModuleResourceManager::removeSharedObject(const QString& objectId)
{
    QWriteLocker locker(&m_sharedObjectLock);
    
    if (m_sharedObjects.contains(objectId)) {
        m_sharedObjects.remove(objectId);
        qDebug() << "Shared object removed:" << objectId;
        return true;
    }
    
    return false;
}

void ModuleResourceManager::compactMemory()
{
    // 清理过期资源
    evictExpiredResources();
    
    // 优化缓存
    optimizeCache();
    
    // 清理弱引用
    {
        QWriteLocker locker(&m_sharedObjectLock);
        auto it = m_sharedObjects.begin();
        while (it != m_sharedObjects.end()) {
            if (it.value().isNull()) {
                it = m_sharedObjects.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // 清理空的资源池
    {
        QWriteLocker locker(&m_poolLock);
        for (auto& pool : m_resourcePools) {
            QWriteLocker poolLocker(&pool.lock);
            auto it = pool.objects.begin();
            while (it != pool.objects.end()) {
                if (it.value().isNull()) {
                    it = pool.objects.erase(it);
                    pool.currentSize--;
                } else {
                    ++it;
                }
            }
        }
    }
    
    qDebug() << "Memory compaction completed";
}

void ModuleResourceManager::freeUnusedResources()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 unusedThreshold = 300000;  // 5分钟未使用
    
    QStringList toRemove;
    
    {
        QReadLocker locker(&m_resourceLock);
        for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
            const ResourceInfo& info = it.value();
            if (currentTime - info.lastAccessTime > unusedThreshold && 
                info.type == TempResource) {
                toRemove.append(it.key());
            }
        }
    }
    
    // 移除未使用的资源
    for (const QString& key : toRemove) {
        // 从key中提取resourceId和moduleName
        QStringList parts = key.split("::");
        if (parts.size() >= 2) {
            removeResource(parts[1], parts[0]);
        }
    }
    
    qDebug() << "Freed" << toRemove.size() << "unused resources";
}

qint64 ModuleResourceManager::getMemoryUsage() const
{
    return m_currentMemoryUsage;
}

qint64 ModuleResourceManager::getMaxMemoryUsage() const
{
    return m_maxMemoryUsage;
}

void ModuleResourceManager::setMaxMemoryUsage(qint64 maxBytes)
{
    m_maxMemoryUsage = maxBytes;
    qDebug() << "Max memory usage set to:" << maxBytes;
}

ModuleResourceManager::CacheStatistics ModuleResourceManager::getCacheStatistics() const
{
    QMutexLocker locker(&m_cacheStatsLock);
    CacheStatistics stats = m_cacheStats;
    
    // 计算命中率
    qint64 totalAccess = stats.hitCount + stats.missCount;
    if (totalAccess > 0) {
        stats.hitRatio = (double)stats.hitCount / totalAccess;
    }
    
    stats.totalSize = m_cache.totalCost();
    stats.itemCount = m_cache.count();
    stats.memoryUsage = m_currentMemoryUsage;
    
    return stats;
}

QList<ModuleResourceManager::ResourceInfo> ModuleResourceManager::getResourceList(const QString& moduleName) const
{
    QReadLocker locker(&m_resourceLock);
    QList<ResourceInfo> result;
    
    for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
        const ResourceInfo& info = it.value();
        if (moduleName.isEmpty() || info.moduleName == moduleName) {
            result.append(info);
        }
    }
    
    return result;
}

QStringList ModuleResourceManager::getResourcePools() const
{
    QReadLocker locker(&m_poolLock);
    return m_resourcePools.keys();
}

ModuleResourceManager::ResourcePool ModuleResourceManager::getResourcePoolInfo(const QString& poolId) const
{
    QReadLocker locker(&m_poolLock);
    return m_resourcePools.value(poolId, ResourcePool());
}

void ModuleResourceManager::startCleanupTimer()
{
    m_cleanupTimer->start();
    m_memoryCheckTimer->start();
    qDebug() << "Cleanup timers started";
}

void ModuleResourceManager::stopCleanupTimer()
{
    m_cleanupTimer->stop();
    m_memoryCheckTimer->stop();
    qDebug() << "Cleanup timers stopped";
}

QString ModuleResourceManager::generateResourceKey(const QString& resourceId, const QString& moduleName) const
{
    if (moduleName.isEmpty()) {
        return QString("global::%1").arg(resourceId);
    }
    return QString("%1::%2").arg(moduleName, resourceId);
}

bool ModuleResourceManager::shouldCache(const ResourceInfo& info) const
{
    switch (info.cachePolicy) {
    case NoCache:
        return false;
    case LRU:
    case LFU:
    case TTL:
    case Adaptive:
        return true;
    }
    return false;
}

void ModuleResourceManager::updateAccessInfo(ResourceInfo& info)
{
    info.lastAccessTime = QDateTime::currentMSecsSinceEpoch();
    info.accessCount++;
}

void ModuleResourceManager::evictExpiredResources()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    QStringList toRemove;
    
    {
        QReadLocker locker(&m_resourceLock);
        for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
            const ResourceInfo& info = it.value();
            if (info.ttl > 0 && currentTime - info.createTime > info.ttl) {
                toRemove.append(it.key());
            }
        }
    }
    
    for (const QString& key : toRemove) {
        m_cache.remove(key);
        emit cacheEviction(key, "TTL expired");
    }
    
    {
        QMutexLocker locker(&m_cacheStatsLock);
        m_cacheStats.evictionCount += toRemove.size();
    }
}

void ModuleResourceManager::evictLRUResources()
{
    // LRU缓存由QCache自动处理
    // 这里可以添加额外的LRU逻辑
}

void ModuleResourceManager::evictLFUResources()
{
    // 实现LFU淘汰策略
    QList<QPair<qint64, QString>> accessCounts;
    
    {
        QReadLocker locker(&m_resourceLock);
        for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
            const ResourceInfo& info = it.value();
            if (info.cachePolicy == LFU) {
                accessCounts.append(qMakePair(info.accessCount, it.key()));
            }
        }
    }
    
    // 按访问次数排序
    std::sort(accessCounts.begin(), accessCounts.end());
    
    // 淘汰访问次数最少的资源
    int evictCount = qMin(10, accessCounts.size() / 10);  // 淘汰10%
    for (int i = 0; i < evictCount; ++i) {
        const QString& key = accessCounts[i].second;
        m_cache.remove(key);
        emit cacheEviction(key, "LFU eviction");
    }
    
    {
        QMutexLocker locker(&m_cacheStatsLock);
        m_cacheStats.evictionCount += evictCount;
    }
}

void ModuleResourceManager::applyAdaptiveEviction()
{
    // 自适应淘汰策略 - 结合LRU和LFU
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    QList<QPair<double, QString>> scores;
    
    {
        QReadLocker locker(&m_resourceLock);
        for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
            const ResourceInfo& info = it.value();
            if (info.cachePolicy == Adaptive) {
                // 计算综合得分：时间权重 + 频率权重
                double timeScore = (double)(currentTime - info.lastAccessTime) / 1000.0;  // 秒
                double freqScore = 1.0 / (info.accessCount + 1);
                double totalScore = timeScore * 0.7 + freqScore * 0.3;
                
                scores.append(qMakePair(totalScore, it.key()));
            }
        }
    }
    
    // 按得分排序（得分高的优先淘汰）
    std::sort(scores.begin(), scores.end(), std::greater<QPair<double, QString>>());
    
    // 淘汰得分最高的资源
    int evictCount = qMin(5, scores.size() / 20);  // 淘汰5%
    for (int i = 0; i < evictCount; ++i) {
        const QString& key = scores[i].second;
        m_cache.remove(key);
        emit cacheEviction(key, "Adaptive eviction");
    }
    
    {
        QMutexLocker locker(&m_cacheStatsLock);
        m_cacheStats.evictionCount += evictCount;
    }
}

qint64 ModuleResourceManager::calculateResourceSize(const QVariant& data) const
{
    // 简化的大小计算
    QJsonDocument doc = QJsonDocument::fromVariant(data);
    return doc.toJson().size();
}

void ModuleResourceManager::updateCacheStatistics()
{
    QMutexLocker locker(&m_cacheStatsLock);
    m_cacheStats.totalSize = m_cache.totalCost();
    m_cacheStats.itemCount = m_cache.count();
}

void ModuleResourceManager::checkMemoryLimits()
{
    if (m_currentMemoryUsage > m_maxMemoryUsage) {
        emit memoryWarning(m_currentMemoryUsage, m_maxMemoryUsage);
        
        // 自动清理
        freeUnusedResources();
        compactMemory();
    }
}

void ModuleResourceManager::performCleanup()
{
    evictExpiredResources();
    freeUnusedResources();
    updateCacheStatistics();
    
    qDebug() << "Periodic cleanup completed. Memory usage:" << m_currentMemoryUsage;
}

void ModuleResourceManager::checkMemoryUsage()
{
    checkMemoryLimits();
    
    // 更新峰值内存使用
    if (m_currentMemoryUsage > m_peakMemoryUsage) {
        m_peakMemoryUsage = m_currentMemoryUsage;
    }
}