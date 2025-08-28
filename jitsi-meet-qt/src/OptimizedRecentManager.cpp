#include "OptimizedRecentManager.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QtConcurrent>
#include <QDebug>

OptimizedRecentManager::OptimizedRecentManager(QObject *parent)
    : QObject(parent)
    , m_maxItems(50)
    , m_lazyLoadingEnabled(true)
    , m_autoSaveEnabled(true)
    , m_loadWatcher(new QFutureWatcher<void>(this))
    , m_saveWatcher(new QFutureWatcher<void>(this))
    , m_optimizationTimer(new QTimer(this))
    , m_lastLoadTime(0)
    , m_isLoaded(false)
    , m_isDirty(false)
{
    // 连接异步操作完成信号
    connect(m_loadWatcher, &QFutureWatcher<void>::finished, 
            this, &OptimizedRecentManager::onLoadingFinished);
    connect(m_saveWatcher, &QFutureWatcher<void>::finished, 
            this, &OptimizedRecentManager::onSavingFinished);
    
    // 设置优化定时器
    m_optimizationTimer->setInterval(300000); // 5分钟优化一次
    connect(m_optimizationTimer, &QTimer::timeout, 
            this, &OptimizedRecentManager::onOptimizationTimer);
    m_optimizationTimer->start();
    
    // 预分配容器空间
    m_recentItems.reserve(m_maxItems);
    m_urlToIndex.reserve(m_maxItems);
    
    qDebug() << "OptimizedRecentManager: Initialized with max items:" << m_maxItems;
}

OptimizedRecentManager::~OptimizedRecentManager()
{
    // 等待异步操作完成
    if (m_loadWatcher->isRunning()) {
        m_loadWatcher->waitForFinished();
    }
    if (m_saveWatcher->isRunning()) {
        m_saveWatcher->waitForFinished();
    }
    
    // 保存未保存的更改
    if (m_isDirty && m_autoSaveEnabled) {
        saveRecentItemsSync();
    }
}

void OptimizedRecentManager::loadRecentItemsAsync()
{
    if (m_loadWatcher->isRunning()) {
        return; // 已在加载中
    }
    
    QFuture<void> future = QtConcurrent::run([this]() {
        loadRecentItemsSync();
    });
    
    m_loadWatcher->setFuture(future);
    qDebug() << "OptimizedRecentManager: Started async loading";
}

void OptimizedRecentManager::saveRecentItemsAsync()
{
    if (!m_isDirty || m_saveWatcher->isRunning()) {
        return;
    }
    
    // 创建数据副本以避免并发访问问题
    QList<RecentItem> itemsCopy;
    {
        QMutexLocker locker(&m_itemsMutex);
        itemsCopy = m_recentItems;
    }
    
    QFuture<void> future = QtConcurrent::run([this, itemsCopy]() {
        saveRecentItemsSync();
    });
    
    m_saveWatcher->setFuture(future);
    qDebug() << "OptimizedRecentManager: Started async saving";
}

void OptimizedRecentManager::addRecentItem(const QString& url, const QString& displayName)
{
    if (url.isEmpty()) {
        return;
    }
    
    QMutexLocker locker(&m_itemsMutex);
    
    // 检查是否已存在
    auto it = std::find_if(m_recentItems.begin(), m_recentItems.end(),
                          [&url](const RecentItem& item) { return item.url == url; });
    
    if (it != m_recentItems.end()) {
        // 更新现有项目
        it->timestamp = QDateTime::currentDateTime();
        it->accessCount++;
        if (!displayName.isEmpty()) {
            it->displayName = displayName;
        }
    } else {
        // 添加新项目
        RecentItem newItem;
        newItem.url = url;
        newItem.displayName = displayName.isEmpty() ? url : displayName;
        newItem.timestamp = QDateTime::currentDateTime();
        newItem.accessCount = 1;
        
        m_recentItems.append(newItem);
        emit recentItemAdded(newItem);
    }
    
    // 重新排序和修剪
    sortRecentItems();
    trimToMaxItems();
    
    // 更新索引
    updateUrlIndex();
    
    m_isDirty = true;
    
    // 清理搜索缓存
    {
        QMutexLocker cacheLocker(&m_cacheMutex);
        m_searchCache.clear();
    }
    
    // 自动保存
    if (m_autoSaveEnabled) {
        saveRecentItemsAsync();
    }
}

void OptimizedRecentManager::removeRecentItem(const QString& url)
{
    QMutexLocker locker(&m_itemsMutex);
    
    auto it = std::find_if(m_recentItems.begin(), m_recentItems.end(),
                          [&url](const RecentItem& item) { return item.url == url; });
    
    if (it != m_recentItems.end()) {
        m_recentItems.erase(it);
        updateUrlIndex();
        m_isDirty = true;
        
        emit recentItemRemoved(url);
        
        // 清理搜索缓存
        QMutexLocker cacheLocker(&m_cacheMutex);
        m_searchCache.clear();
        
        if (m_autoSaveEnabled) {
            saveRecentItemsAsync();
        }
    }
}

void OptimizedRecentManager::clearRecentItems()
{
    QMutexLocker locker(&m_itemsMutex);
    
    if (!m_recentItems.isEmpty()) {
        m_recentItems.clear();
        m_urlToIndex.clear();
        m_isDirty = true;
        
        // 清理搜索缓存
        QMutexLocker cacheLocker(&m_cacheMutex);
        m_searchCache.clear();
        
        if (m_autoSaveEnabled) {
            saveRecentItemsAsync();
        }
    }
}

QList<RecentItem> OptimizedRecentManager::getRecentItems(int maxCount) const
{
    QMutexLocker locker(&m_itemsMutex);
    
    if (maxCount < 0 || maxCount >= m_recentItems.size()) {
        return m_recentItems;
    }
    
    return m_recentItems.mid(0, maxCount);
}

QList<RecentItem> OptimizedRecentManager::searchRecentItems(const QString& query) const
{
    if (query.isEmpty()) {
        return getRecentItems();
    }
    
    // 检查缓存
    {
        QMutexLocker cacheLocker(&m_cacheMutex);
        auto cacheIt = m_searchCache.find(query.toLower());
        if (cacheIt != m_searchCache.end()) {
            return cacheIt.value();
        }
    }
    
    QMutexLocker locker(&m_itemsMutex);
    
    QList<RecentItem> results;
    QString lowerQuery = query.toLower();
    
    for (const RecentItem& item : m_recentItems) {
        if (item.url.toLower().contains(lowerQuery) || 
            item.displayName.toLower().contains(lowerQuery)) {
            results.append(item);
        }
    }
    
    // 缓存结果
    {
        QMutexLocker cacheLocker(&m_cacheMutex);
        m_searchCache[lowerQuery] = results;
        
        // 限制缓存大小
        if (m_searchCache.size() > 100) {
            // 在Qt 6中，需要逐个删除元素
            auto keys = m_searchCache.keys();
            for (int i = 50; i < keys.size(); ++i) {
                m_searchCache.remove(keys[i]);
            }
        }
    }
    
    return results;
}

bool OptimizedRecentManager::hasRecentItem(const QString& url) const
{
    QMutexLocker locker(&m_itemsMutex);
    return m_urlToIndex.contains(url);
}

void OptimizedRecentManager::setMaxItems(int maxItems)
{
    if (maxItems != m_maxItems && maxItems > 0) {
        m_maxItems = maxItems;
        
        QMutexLocker locker(&m_itemsMutex);
        trimToMaxItems();
        
        qDebug() << "OptimizedRecentManager: Max items set to" << maxItems;
    }
}

void OptimizedRecentManager::setLazyLoadingEnabled(bool enabled)
{
    m_lazyLoadingEnabled = enabled;
    qDebug() << "OptimizedRecentManager: Lazy loading" << (enabled ? "enabled" : "disabled");
}

void OptimizedRecentManager::optimizeStorage()
{
    QMutexLocker locker(&m_itemsMutex);
    
    // 移除过期项目（超过30天未访问）
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-30);
    
    auto it = m_recentItems.begin();
    while (it != m_recentItems.end()) {
        if (it->timestamp < cutoffDate && it->accessCount < 2) {
            it = m_recentItems.erase(it);
        } else {
            ++it;
        }
    }
    
    // 重新排序和更新索引
    sortRecentItems();
    updateUrlIndex();
    
    // 清理缓存
    {
        QMutexLocker cacheLocker(&m_cacheMutex);
        m_searchCache.clear();
    }
    
    m_isDirty = true;
    
    qDebug() << "OptimizedRecentManager: Storage optimized, items count:" << m_recentItems.size();
}

int OptimizedRecentManager::getItemCount() const
{
    QMutexLocker locker(&m_itemsMutex);
    return m_recentItems.size();
}

qint64 OptimizedRecentManager::getLoadTime() const
{
    return m_lastLoadTime;
}

void OptimizedRecentManager::onLoadingFinished()
{
    m_isLoaded = true;
    emit recentItemsLoaded();
    qDebug() << "OptimizedRecentManager: Loading completed in" << m_lastLoadTime << "ms";
}

void OptimizedRecentManager::onSavingFinished()
{
    m_isDirty = false;
    qDebug() << "OptimizedRecentManager: Saving completed";
}

void OptimizedRecentManager::onOptimizationTimer()
{
    optimizeStorage();
    
    if (m_isDirty && m_autoSaveEnabled) {
        saveRecentItemsAsync();
    }
}

void OptimizedRecentManager::loadRecentItemsSync()
{
    QElapsedTimer timer;
    timer.start();
    
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    QString filePath = configPath + "/recent_items.json";
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "OptimizedRecentManager: No existing recent items file";
        m_lastLoadTime = timer.elapsed();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray array = doc.array();
    
    QMutexLocker locker(&m_itemsMutex);
    m_recentItems.clear();
    m_recentItems.reserve(array.size());
    
    for (const QJsonValue& value : array) {
        QJsonObject obj = value.toObject();
        
        RecentItem item;
        item.url = obj["url"].toString();
        item.displayName = obj["displayName"].toString();
        item.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        item.accessCount = obj["accessCount"].toInt();
        
        if (!item.url.isEmpty()) {
            m_recentItems.append(item);
        }
    }
    
    sortRecentItems();
    trimToMaxItems();
    updateUrlIndex();
    
    m_lastLoadTime = timer.elapsed();
    qDebug() << "OptimizedRecentManager: Loaded" << m_recentItems.size() << "items in" << m_lastLoadTime << "ms";
}

void OptimizedRecentManager::saveRecentItemsSync()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    QString filePath = configPath + "/recent_items.json";
    
    QJsonArray array;
    
    {
        QMutexLocker locker(&m_itemsMutex);
        for (const RecentItem& item : m_recentItems) {
            QJsonObject obj;
            obj["url"] = item.url;
            obj["displayName"] = item.displayName;
            obj["timestamp"] = item.timestamp.toString(Qt::ISODate);
            obj["accessCount"] = item.accessCount;
            array.append(obj);
        }
    }
    
    QJsonDocument doc(array);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        qDebug() << "OptimizedRecentManager: Saved" << array.size() << "items";
    } else {
        qWarning() << "OptimizedRecentManager: Failed to save recent items";
    }
}

void OptimizedRecentManager::sortRecentItems()
{
    std::sort(m_recentItems.begin(), m_recentItems.end());
}

void OptimizedRecentManager::trimToMaxItems()
{
    if (m_recentItems.size() > m_maxItems) {
        m_recentItems.resize(m_maxItems);
    }
}

void OptimizedRecentManager::updateUrlIndex()
{
    m_urlToIndex.clear();
    m_urlToIndex.reserve(m_recentItems.size());
    
    for (int i = 0; i < m_recentItems.size(); ++i) {
        m_urlToIndex[m_recentItems[i].url] = i;
    }
}

QString OptimizedRecentManager::generateCacheKey(const QString& url) const
{
    return QString::number(qHash(url));
}