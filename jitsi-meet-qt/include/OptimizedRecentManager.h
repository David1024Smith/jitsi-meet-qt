#ifndef OPTIMIZEDRECENTMANAGER_H
#define OPTIMIZEDRECENTMANAGER_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QMutex>
#include <QTimer>
#include <QDateTime>
#include <QFuture>
#include <QFutureWatcher>
#include "models/RecentItem.h"

/**
 * @brief 优化的最近项目管理器 - 提供高性能的历史记录管理
 */
class OptimizedRecentManager : public QObject
{
    Q_OBJECT

public:
    explicit OptimizedRecentManager(QObject *parent = nullptr);
    ~OptimizedRecentManager();

    // 异步加载
    void loadRecentItemsAsync();
    void saveRecentItemsAsync();

    // 项目管理
    void addRecentItem(const QString& url, const QString& displayName = QString());
    void removeRecentItem(const QString& url);
    void clearRecentItems();

    // 查询
    QList<RecentItem> getRecentItems(int maxCount = -1) const;
    QList<RecentItem> searchRecentItems(const QString& query) const;
    bool hasRecentItem(const QString& url) const;

    // 性能优化
    void setMaxItems(int maxItems);
    void setLazyLoadingEnabled(bool enabled);
    void optimizeStorage();

    // 统计
    int getItemCount() const;
    qint64 getLoadTime() const;

signals:
    void recentItemsLoaded();
    void recentItemAdded(const RecentItem& item);
    void recentItemRemoved(const QString& url);
    void loadingProgress(int percentage);

private slots:
    void onLoadingFinished();
    void onSavingFinished();
    void onOptimizationTimer();

private:
    void loadRecentItemsSync();
    void saveRecentItemsSync();
    void sortRecentItems();
    void trimToMaxItems();
    void updateUrlIndex();
    QString generateCacheKey(const QString& url) const;

    mutable QMutex m_itemsMutex;
    QList<RecentItem> m_recentItems;
    QHash<QString, int> m_urlToIndex; // 快速查找索引
    
    // 配置
    int m_maxItems;
    bool m_lazyLoadingEnabled;
    bool m_autoSaveEnabled;
    
    // 异步操作
    QFutureWatcher<void>* m_loadWatcher;
    QFutureWatcher<void>* m_saveWatcher;
    QTimer* m_optimizationTimer;
    
    // 性能指标
    qint64 m_lastLoadTime;
    bool m_isLoaded;
    bool m_isDirty;
    
    // 缓存
    mutable QHash<QString, QList<RecentItem>> m_searchCache;
    mutable QMutex m_cacheMutex;
};

#endif // OPTIMIZEDRECENTMANAGER_H