#ifndef MESSAGESTORAGE_H
#define MESSAGESTORAGE_H

#include "IMessageStorage.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <QSqlDatabase>
#include <QMutex>
#include <memory>

// Forward declarations
class ChatMessage;
class QSqlQuery;
class QTimer;

/**
 * @brief 消息存储实现类
 * 
 * MessageStorage实现了IMessageStorage接口，提供基于SQLite的
 * 消息持久化存储功能。
 */
class MessageStorage : public IMessageStorage
{
    Q_OBJECT
    Q_PROPERTY(QString databasePath READ databasePath WRITE setDatabasePath NOTIFY databasePathChanged)
    Q_PROPERTY(qint64 totalSize READ totalSize NOTIFY totalSizeChanged)
    Q_PROPERTY(int messageCount READ messageCount NOTIFY messageCountChanged)
    Q_PROPERTY(bool cacheEnabled READ isCacheEnabled WRITE setCacheEnabled NOTIFY cacheEnabledChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MessageStorage(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MessageStorage();

    // IMessageStorage接口实现
    bool initialize(const QVariantMap& config = QVariantMap()) override;
    void close() override;
    StorageStatus status() const override;
    bool isReady() const override;

    OperationResult storeMessage(ChatMessage* message) override;
    OperationResult storeMessages(const QList<ChatMessage*>& messages) override;
    ChatMessage* getMessage(const QString& messageId) override;
    QList<ChatMessage*> getRoomMessages(const QString& roomId, int limit = 50, int offset = 0, SortOrder order = Descending) override;
    QList<ChatMessage*> getMessagesByTimeRange(const QString& roomId, const QDateTime& startTime, const QDateTime& endTime, int limit = 100) override;
    QList<ChatMessage*> searchMessages(const QString& query, const QString& roomId = QString(), int limit = 50) override;

    OperationResult updateMessage(ChatMessage* message) override;
    OperationResult deleteMessage(const QString& messageId) override;
    OperationResult deleteRoomMessages(const QString& roomId) override;
    OperationResult deleteMessagesBefore(const QString& roomId, const QDateTime& before) override;

    int getMessageCount(const QString& roomId = QString()) override;
    QStringList getRoomList() override;
    ChatMessage* getLastMessage(const QString& roomId) override;

    int getUnreadCount(const QString& roomId, const QString& userId) override;
    OperationResult markAsRead(const QString& messageId, const QString& userId) override;
    OperationResult markRoomAsRead(const QString& roomId, const QString& userId) override;

    QVariantMap getStatistics() override;
    OperationResult compact() override;
    OperationResult backup(const QString& backupPath) override;
    OperationResult restore(const QString& backupPath) override;

    // 扩展功能
    /**
     * @brief 获取数据库路径
     * @return 数据库路径
     */
    QString databasePath() const;

    /**
     * @brief 设置数据库路径
     * @param path 数据库路径
     */
    void setDatabasePath(const QString& path);

    /**
     * @brief 获取数据库总大小（字节）
     * @return 总大小
     */
    qint64 totalSize() const;

    /**
     * @brief 获取消息总数
     * @return 消息总数
     */
    int messageCount() const;

    /**
     * @brief 检查缓存是否启用
     * @return 是否启用缓存
     */
    bool isCacheEnabled() const;

    /**
     * @brief 设置缓存启用状态
     * @param enabled 是否启用
     */
    void setCacheEnabled(bool enabled);

    /**
     * @brief 获取缓存大小限制
     * @return 缓存大小限制
     */
    int cacheLimit() const;

    /**
     * @brief 设置缓存大小限制
     * @param limit 缓存大小限制
     */
    void setCacheLimit(int limit);

    /**
     * @brief 获取缓存命中率
     * @return 缓存命中率（0.0-1.0）
     */
    double cacheHitRate() const;

    /**
     * @brief 设置自动清理间隔（小时）
     * @param hours 间隔小时数
     */
    void setAutoCleanupInterval(int hours);

    /**
     * @brief 获取自动清理间隔（小时）
     * @return 间隔小时数
     */
    int autoCleanupInterval() const;

    /**
     * @brief 设置最大存储大小（字节）
     * @param size 最大大小
     */
    void setMaxStorageSize(qint64 size);

    /**
     * @brief 获取最大存储大小（字节）
     * @return 最大大小
     */
    qint64 maxStorageSize() const;

    /**
     * @brief 检查存储空间是否充足
     * @return 是否充足
     */
    bool hasEnoughSpace() const;

    /**
     * @brief 获取可用空间（字节）
     * @return 可用空间
     */
    qint64 availableSpace() const;

    /**
     * @brief 执行数据库维护
     * @return 操作结果
     */
    OperationResult performMaintenance();

    /**
     * @brief 重建索引
     * @return 操作结果
     */
    OperationResult rebuildIndexes();

    /**
     * @brief 分析数据库
     * @return 操作结果
     */
    OperationResult analyzeDatabase();

    /**
     * @brief 检查数据库完整性
     * @return 检查结果
     */
    bool checkIntegrity();

    /**
     * @brief 修复数据库
     * @return 修复结果
     */
    OperationResult repairDatabase();

public slots:
    void cleanupOldMessages(int days = 30) override;
    void optimize() override;
    void refreshCache() override;

    /**
     * @brief 清空缓存
     */
    void clearCache();

    /**
     * @brief 预加载房间消息到缓存
     * @param roomId 房间ID
     * @param limit 消息数量
     */
    void preloadRoomMessages(const QString& roomId, int limit = 100);

    /**
     * @brief 执行自动清理
     */
    void performAutoCleanup();

    /**
     * @brief 同步到磁盘
     */
    void syncToDisk();

signals:
    /**
     * @brief 数据库路径改变信号
     * @param path 新路径
     */
    void databasePathChanged(const QString& path);

    /**
     * @brief 总大小改变信号
     * @param size 新大小
     */
    void totalSizeChanged(qint64 size);

    /**
     * @brief 消息数量改变信号
     * @param count 新数量
     */
    void messageCountChanged(int count);

    /**
     * @brief 缓存启用状态改变信号
     * @param enabled 是否启用
     */
    void cacheEnabledChanged(bool enabled);

    /**
     * @brief 缓存统计更新信号
     * @param hitRate 命中率
     * @param size 缓存大小
     */
    void cacheStatsUpdated(double hitRate, int size);

    /**
     * @brief 维护完成信号
     * @param success 是否成功
     */
    void maintenanceCompleted(bool success);

    /**
     * @brief 清理完成信号
     * @param deletedCount 删除的消息数量
     */
    void cleanupCompleted(int deletedCount);

private slots:
    /**
     * @brief 处理自动清理定时器
     */
    void handleAutoCleanupTimer();

    /**
     * @brief 处理缓存清理定时器
     */
    void handleCacheCleanupTimer();

private:
    /**
     * @brief 初始化数据库
     * @return 初始化是否成功
     */
    bool initializeDatabase();

    /**
     * @brief 创建数据库表
     * @return 创建是否成功
     */
    bool createTables();

    /**
     * @brief 创建索引
     * @return 创建是否成功
     */
    bool createIndexes();

    /**
     * @brief 升级数据库架构
     * @param fromVersion 源版本
     * @param toVersion 目标版本
     * @return 升级是否成功
     */
    bool upgradeSchema(int fromVersion, int toVersion);

    /**
     * @brief 获取数据库版本
     * @return 数据库版本
     */
    int getDatabaseVersion();

    /**
     * @brief 设置数据库版本
     * @param version 版本号
     */
    void setDatabaseVersion(int version);

    /**
     * @brief 执行SQL查询
     * @param query SQL查询
     * @return 查询结果
     */
    bool executeQuery(QSqlQuery& query);

    /**
     * @brief 准备SQL语句
     * @param sql SQL语句
     * @return 准备好的查询对象
     */
    std::unique_ptr<QSqlQuery> prepareQuery(const QString& sql);

    /**
     * @brief 从查询结果创建消息对象
     * @param query 查询对象
     * @return 消息对象
     */
    ChatMessage* createMessageFromQuery(QSqlQuery& query);

    /**
     * @brief 将消息绑定到查询
     * @param query 查询对象
     * @param message 消息对象
     */
    void bindMessageToQuery(QSqlQuery& query, ChatMessage* message);

    /**
     * @brief 从缓存获取消息
     * @param messageId 消息ID
     * @return 消息对象，如果不存在则返回nullptr
     */
    ChatMessage* getFromCache(const QString& messageId);

    /**
     * @brief 添加消息到缓存
     * @param message 消息对象
     */
    void addToCache(ChatMessage* message);

    /**
     * @brief 从缓存移除消息
     * @param messageId 消息ID
     */
    void removeFromCache(const QString& messageId);

    /**
     * @brief 清理缓存
     */
    void cleanupCache();

    /**
     * @brief 更新统计信息
     */
    void updateStatistics();

    /**
     * @brief 设置存储状态
     * @param status 新状态
     */
    void setStatus(StorageStatus status);

    /**
     * @brief 处理数据库错误
     * @param error 错误信息
     */
    void handleDatabaseError(const QString& error);

    /**
     * @brief 验证消息数据
     * @param message 消息对象
     * @return 验证是否通过
     */
    bool validateMessage(ChatMessage* message);

    /**
     * @brief 生成备份文件名
     * @param basePath 基础路径
     * @return 备份文件路径
     */
    QString generateBackupFileName(const QString& basePath);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // MESSAGESTORAGE_H