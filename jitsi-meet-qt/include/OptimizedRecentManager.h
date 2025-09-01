#ifndef OPTIMIZEDRECENTMANAGER_H
#define OPTIMIZEDRECENTMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QVariantMap>

/**
 * @brief 优化的最近会议管理器
 * 
 * 该类负责管理最近参加的会议列表，提供高效的存储和检索功能，
 * 包括自动排序、限制列表大小和持久化存储。
 */
class OptimizedRecentManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 最近会议项结构
     */
    struct RecentItem {
        QString meetingId;       ///< 会议ID
        QString displayName;     ///< 显示名称
        QString url;             ///< 会议URL
        QDateTime lastJoined;    ///< 最后加入时间
        int joinCount;           ///< 加入次数
        bool favorite;           ///< 是否收藏
        QVariantMap metadata;    ///< 元数据
    };

    explicit OptimizedRecentManager(QObject *parent = nullptr);
    ~OptimizedRecentManager();

    /**
     * @brief 获取单例实例
     */
    static OptimizedRecentManager* instance();

    /**
     * @brief 初始化管理器
     * @return 是否成功初始化
     */
    bool initialize();

    /**
     * @brief 关闭管理器
     */
    void shutdown();

    /**
     * @brief 添加最近会议
     * @param meetingId 会议ID
     * @param displayName 显示名称
     * @param url 会议URL
     * @param metadata 元数据
     * @return 是否成功添加
     */
    bool addRecentMeeting(const QString& meetingId, const QString& displayName, 
                         const QString& url, const QVariantMap& metadata = QVariantMap());

    /**
     * @brief 更新最近会议
     * @param meetingId 会议ID
     * @param displayName 显示名称
     * @param url 会议URL
     * @param metadata 元数据
     * @return 是否成功更新
     */
    bool updateRecentMeeting(const QString& meetingId, const QString& displayName, 
                            const QString& url, const QVariantMap& metadata = QVariantMap());

    /**
     * @brief 移除最近会议
     * @param meetingId 会议ID
     * @return 是否成功移除
     */
    bool removeRecentMeeting(const QString& meetingId);

    /**
     * @brief 清除所有最近会议
     */
    void clearRecentMeetings();

    /**
     * @brief 获取最近会议列表
     * @param limit 限制数量，默认为-1表示不限制
     * @return 最近会议列表
     */
    QList<RecentItem> getRecentMeetings(int limit = -1) const;

    /**
     * @brief 获取收藏会议列表
     * @return 收藏会议列表
     */
    QList<RecentItem> getFavoriteMeetings() const;

    /**
     * @brief 设置会议收藏状态
     * @param meetingId 会议ID
     * @param favorite 是否收藏
     * @return 是否成功设置
     */
    bool setMeetingFavorite(const QString& meetingId, bool favorite);

    /**
     * @brief 检查会议是否存在
     * @param meetingId 会议ID
     * @return 是否存在
     */
    bool containsMeeting(const QString& meetingId) const;

    /**
     * @brief 获取会议详情
     * @param meetingId 会议ID
     * @return 会议详情
     */
    RecentItem getMeetingDetails(const QString& meetingId) const;

    /**
     * @brief 设置最大列表大小
     * @param size 最大大小
     */
    void setMaxListSize(int size);

    /**
     * @brief 获取最大列表大小
     * @return 最大大小
     */
    int maxListSize() const;

    /**
     * @brief 保存最近会议列表
     * @return 是否成功保存
     */
    bool saveRecentMeetings();

    /**
     * @brief 加载最近会议列表
     * @return 是否成功加载
     */
    bool loadRecentMeetings();

signals:
    /**
     * @brief 最近会议列表变化信号
     */
    void recentMeetingsChanged();

    /**
     * @brief 最近会议添加信号
     * @param meetingId 会议ID
     */
    void recentMeetingAdded(const QString& meetingId);

    /**
     * @brief 最近会议更新信号
     * @param meetingId 会议ID
     */
    void recentMeetingUpdated(const QString& meetingId);

    /**
     * @brief 最近会议移除信号
     * @param meetingId 会议ID
     */
    void recentMeetingRemoved(const QString& meetingId);

    /**
     * @brief 收藏状态变化信号
     * @param meetingId 会议ID
     * @param favorite 是否收藏
     */
    void favoriteStatusChanged(const QString& meetingId, bool favorite);

private:
    void sortRecentMeetings();
    bool validateMeetingId(const QString& meetingId) const;
    void pruneRecentMeetings();
    QString getStorageFilePath() const;

    static OptimizedRecentManager* s_instance;
    QList<RecentItem> m_recentMeetings;
    int m_maxListSize;
    bool m_initialized;
};

Q_DECLARE_METATYPE(OptimizedRecentManager::RecentItem)

#endif // OPTIMIZEDRECENTMANAGER_H