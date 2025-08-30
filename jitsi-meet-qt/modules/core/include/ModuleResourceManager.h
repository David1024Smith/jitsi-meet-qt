#ifndef MODULERESOURCEMANAGER_H
#define MODULERESOURCEMANAGER_H

#include <QObject>
#include <QVariant>
#include <QMutex>
#include <QReadWriteLock>
#include <QTimer>
#include <QHash>
#include <QCache>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QDateTime>

/**
 * @brief 模块资源管理器 - 优化的资源共享和缓存系统
 * 
 * 提供高效的资源管理，支持：
 * - 智能缓存系统
 * - 资源共享池
 * - 内存优化
 * - 自动清理
 * - 资源监控
 */
class ModuleResourceManager : public QObject
{
    Q_OBJECT

public:
    enum ResourceType {
        Configuration,   // 配置资源
        Data,           // 数据资源
        Cache,          // 缓存资源
        SharedObject,   // 共享对象
        TempResource,   // 临时资源
        StaticResource  // 静态资源
    };

    enum CachePolicy {
        NoCache,        // 不缓存
        LRU,           // 最近最少使用
        LFU,           // 最少使用频率
        TTL,           // 时间过期
        Adaptive       // 自适应策略
    };

    struct ResourceInfo {
        QString id;
        QString moduleName;
        ResourceType type;
        CachePolicy cachePolicy;
        QVariant data;
        qint64 size;
        qint64 createTime;
        qint64 lastAccessTime;
        qint64 accessCount;
        qint64 ttl;  // Time to live in milliseconds
        QVariantMap metadata;
        
        ResourceInfo() : type(Data), cachePolicy(LRU), size(0), 
                        createTime(0), lastAccessTime(0), accessCount(0), ttl(0) {}
    };

    struct CacheStatistics {
        qint64 totalSize;
        qint64 maxSize;
        int itemCount;
        int maxItems;
        qint64 hitCount;
        qint64 missCount;
        double hitRatio;
        qint64 evictionCount;
        qint64 memoryUsage;
    };

    struct ResourcePool {
        QString poolId;
        QString description;
        QHash<QString, QSharedPointer<QObject>> objects;
        int maxSize;
        int currentSize;
        qint64 totalAllocations;
        qint64 totalDeallocations;
        mutable QReadWriteLock lock;
        
        ResourcePool() : maxSize(100), currentSize(0), 
                        totalAllocations(0), totalDeallocations(0) {}
    };

    static ModuleResourceManager* instance();
    ~ModuleResourceManager();

    // 资源存储和获取
    bool storeResource(const QString& resourceId, const QVariant& data, 
                      ResourceType type = Data, const QString& moduleName = QString());
    QVariant getResource(const QString& resourceId, const QString& moduleName = QString());
    bool hasResource(const QString& resourceId, const QString& moduleName = QString()) const;
    bool removeResource(const QString& resourceId, const QString& moduleName = QString());

    // 缓存管理
    void setCachePolicy(const QString& resourceId, CachePolicy policy);
    void setCacheTTL(const QString& resourceId, qint64 ttlMs);
    void setCacheMaxSize(qint64 maxSizeBytes);
    void setCacheMaxItems(int maxItems);
    void clearCache(const QString& moduleName = QString());
    void optimizeCache();

    // 资源池管理
    QString createResourcePool(const QString& poolId, const QString& description, int maxSize = 100);
    bool destroyResourcePool(const QString& poolId);
    QSharedPointer<QObject> acquireFromPool(const QString& poolId, const QString& objectType);
    bool releaseToPool(const QString& poolId, QSharedPointer<QObject> object);
    void clearResourcePool(const QString& poolId);

    // 共享对象管理
    template<typename T>
    QSharedPointer<T> getSharedObject(const QString& objectId, std::function<T*()> factory = nullptr);
    
    template<typename T>
    bool setSharedObject(const QString& objectId, QSharedPointer<T> object);
    
    bool removeSharedObject(const QString& objectId);

    // 内存优化
    void compactMemory();
    void freeUnusedResources();
    qint64 getMemoryUsage() const;
    qint64 getMaxMemoryUsage() const;
    void setMaxMemoryUsage(qint64 maxBytes);

    // 统计和监控
    CacheStatistics getCacheStatistics() const;
    QList<ResourceInfo> getResourceList(const QString& moduleName = QString()) const;
    QStringList getResourcePools() const;
    ResourcePool getResourcePoolInfo(const QString& poolId) const;

    // 系统控制
    void initialize();
    void shutdown();
    void startCleanupTimer();
    void stopCleanupTimer();

signals:
    void resourceAdded(const QString& resourceId, const QString& moduleName);
    void resourceRemoved(const QString& resourceId, const QString& moduleName);
    void resourceAccessed(const QString& resourceId, const QString& moduleName);
    void cacheEviction(const QString& resourceId, const QString& reason);
    void memoryWarning(qint64 currentUsage, qint64 maxUsage);
    void poolCreated(const QString& poolId);
    void poolDestroyed(const QString& poolId);

private slots:
    void performCleanup();
    void checkMemoryUsage();

private:
    explicit ModuleResourceManager(QObject* parent = nullptr);
    
    void initializeSystem();
    void shutdownSystem();
    
    QString generateResourceKey(const QString& resourceId, const QString& moduleName) const;
    bool shouldCache(const ResourceInfo& info) const;
    void updateAccessInfo(ResourceInfo& info);
    void evictExpiredResources();
    void evictLRUResources();
    void evictLFUResources();
    void applyAdaptiveEviction();
    
    qint64 calculateResourceSize(const QVariant& data) const;
    void updateCacheStatistics();
    void checkMemoryLimits();

    static ModuleResourceManager* s_instance;
    static QMutex s_mutex;

    // 资源存储
    QHash<QString, ResourceInfo> m_resources;
    mutable QReadWriteLock m_resourceLock;

    // 缓存系统
    QCache<QString, QVariant> m_cache;
    CacheStatistics m_cacheStats;
    mutable QMutex m_cacheStatsLock;

    // 资源池
    QHash<QString, ResourcePool> m_resourcePools;
    mutable QReadWriteLock m_poolLock;

    // 共享对象
    QHash<QString, QWeakPointer<QObject>> m_sharedObjects;
    mutable QReadWriteLock m_sharedObjectLock;

    // 系统配置
    qint64 m_maxMemoryUsage;
    qint64 m_currentMemoryUsage;
    int m_cleanupInterval;
    bool m_autoCleanupEnabled;

    // 定时器
    QTimer* m_cleanupTimer;
    QTimer* m_memoryCheckTimer;

    // 统计信息
    qint64 m_totalAllocations;
    qint64 m_totalDeallocations;
    qint64 m_peakMemoryUsage;
};

// 模板方法实现
template<typename T>
QSharedPointer<T> ModuleResourceManager::getSharedObject(const QString& objectId, std::function<T*()> factory)
{
    QReadLocker locker(&m_sharedObjectLock);
    
    if (m_sharedObjects.contains(objectId)) {
        auto weakPtr = m_sharedObjects[objectId];
        if (auto sharedPtr = weakPtr.lock()) {
            auto typedPtr = qSharedPointerCast<T>(sharedPtr);
            if (typedPtr) {
                return typedPtr;
            }
        }
        // 弱引用已失效，移除
        locker.unlock();
        QWriteLocker writeLocker(&m_sharedObjectLock);
        m_sharedObjects.remove(objectId);
        writeLocker.unlock();
        locker.relock();
    }
    
    // 创建新对象
    if (factory) {
        locker.unlock();
        T* rawPtr = factory();
        if (rawPtr) {
            QSharedPointer<T> sharedPtr(rawPtr);
            setSharedObject(objectId, sharedPtr);
            return sharedPtr;
        }
    }
    
    return QSharedPointer<T>();
}

template<typename T>
bool ModuleResourceManager::setSharedObject(const QString& objectId, QSharedPointer<T> object)
{
    if (!object) {
        return false;
    }
    
    QWriteLocker locker(&m_sharedObjectLock);
    m_sharedObjects[objectId] = object.template staticCast<QObject>();
    return true;
}

#endif // MODULERESOURCEMANAGER_H