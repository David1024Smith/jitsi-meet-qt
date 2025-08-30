#ifndef IMESSAGESTORAGE_H
#define IMESSAGESTORAGE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>

// Forward declarations
class ChatMessage;

/**
 * @brief 消息存储接口
 * 
 * IMessageStorage定义了消息存储的标准接口，
 * 提供消息持久化功能的抽象定义。
 */
class IMessageStorage : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 存储状态枚举
     */
    enum StorageStatus {
        Uninitialized,      ///< 未初始化
        Ready,              ///< 就绪
        Busy,               ///< 忙碌
        Error,              ///< 错误
        Maintenance         ///< 维护中
    };
    Q_ENUM(StorageStatus)

    /**
     * @brief 查询排序方式枚举
     */
    enum SortOrder {
        Ascending,          ///< 升序
        Descending          ///< 降序
    };
    Q_ENUM(SortOrder)

    /**
     * @brief 存储操作结果枚举
     */
    enum OperationResult {
        Success,            ///< 成功
        Failed,             ///< 失败
        NotFound,           ///< 未找到
        AlreadyExists,      ///< 已存在
        PermissionDenied,   ///< 权限拒绝
        StorageFull         ///< 存储已满
    };
    Q_ENUM(OperationResult)

    /**
     * @brief 虚析构函数
     */
    virtual ~IMessageStorage() = default;

    /**
     * @brief 初始化存储
     * @param config 配置参数
     * @return 初始化是否成功
     */
    virtual bool initialize(const QVariantMap& config = QVariantMap()) = 0;

    /**
     * @brief 关闭存储
     */
    virtual void close() = 0;

    /**
     * @brief 获取存储状态
     * @return 存储状态
     */
    virtual StorageStatus status() const = 0;

    /**
     * @brief 检查存储是否就绪
     * @return 是否就绪
     */
    virtual bool isReady() const = 0;

    /**
     * @brief 存储消息
     * @param message 消息对象
     * @return 操作结果
     */
    virtual OperationResult storeMessage(ChatMessage* message) = 0;

    /**
     * @brief 批量存储消息
     * @param messages 消息列表
     * @return 操作结果
     */
    virtual OperationResult storeMessages(const QList<ChatMessage*>& messages) = 0;

    /**
     * @brief 获取消息
     * @param messageId 消息ID
     * @return 消息对象，如果不存在则返回nullptr
     */
    virtual ChatMessage* getMessage(const QString& messageId) = 0;

    /**
     * @brief 获取房间消息
     * @param roomId 房间ID
     * @param limit 数量限制
     * @param offset 偏移量
     * @param order 排序方式
     * @return 消息列表
     */
    virtual QList<ChatMessage*> getRoomMessages(const QString& roomId, int limit = 50, int offset = 0, SortOrder order = Descending) = 0;

    /**
     * @brief 获取时间范围内的消息
     * @param roomId 房间ID
     * @param startTime 开始时间
     * @param endTime 结束时间
     * @param limit 数量限制
     * @return 消息列表
     */
    virtual QList<ChatMessage*> getMessagesByTimeRange(const QString& roomId, const QDateTime& startTime, const QDateTime& endTime, int limit = 100) = 0;

    /**
     * @brief 搜索消息
     * @param query 搜索关键词
     * @param roomId 房间ID（可选，为空则搜索所有房间）
     * @param limit 数量限制
     * @return 匹配的消息列表
     */
    virtual QList<ChatMessage*> searchMessages(const QString& query, const QString& roomId = QString(), int limit = 50) = 0;

    /**
     * @brief 更新消息
     * @param message 消息对象
     * @return 操作结果
     */
    virtual OperationResult updateMessage(ChatMessage* message) = 0;

    /**
     * @brief 删除消息
     * @param messageId 消息ID
     * @return 操作结果
     */
    virtual OperationResult deleteMessage(const QString& messageId) = 0;

    /**
     * @brief 删除房间所有消息
     * @param roomId 房间ID
     * @return 操作结果
     */
    virtual OperationResult deleteRoomMessages(const QString& roomId) = 0;

    /**
     * @brief 删除时间范围内的消息
     * @param roomId 房间ID
     * @param before 删除此时间之前的消息
     * @return 操作结果
     */
    virtual OperationResult deleteMessagesBefore(const QString& roomId, const QDateTime& before) = 0;

    /**
     * @brief 获取消息数量
     * @param roomId 房间ID（可选，为空则统计所有房间）
     * @return 消息数量
     */
    virtual int getMessageCount(const QString& roomId = QString()) = 0;

    /**
     * @brief 获取房间列表
     * @return 房间ID列表
     */
    virtual QStringList getRoomList() = 0;

    /**
     * @brief 获取房间最后一条消息
     * @param roomId 房间ID
     * @return 最后一条消息，如果不存在则返回nullptr
     */
    virtual ChatMessage* getLastMessage(const QString& roomId) = 0;

    /**
     * @brief 获取未读消息数量
     * @param roomId 房间ID
     * @param userId 用户ID
     * @return 未读消息数量
     */
    virtual int getUnreadCount(const QString& roomId, const QString& userId) = 0;

    /**
     * @brief 标记消息为已读
     * @param messageId 消息ID
     * @param userId 用户ID
     * @return 操作结果
     */
    virtual OperationResult markAsRead(const QString& messageId, const QString& userId) = 0;

    /**
     * @brief 标记房间所有消息为已读
     * @param roomId 房间ID
     * @param userId 用户ID
     * @return 操作结果
     */
    virtual OperationResult markRoomAsRead(const QString& roomId, const QString& userId) = 0;

    /**
     * @brief 获取存储统计信息
     * @return 统计信息
     */
    virtual QVariantMap getStatistics() = 0;

    /**
     * @brief 压缩存储
     * @return 操作结果
     */
    virtual OperationResult compact() = 0;

    /**
     * @brief 备份存储
     * @param backupPath 备份路径
     * @return 操作结果
     */
    virtual OperationResult backup(const QString& backupPath) = 0;

    /**
     * @brief 恢复存储
     * @param backupPath 备份路径
     * @return 操作结果
     */
    virtual OperationResult restore(const QString& backupPath) = 0;

public slots:
    /**
     * @brief 清理过期消息
     * @param days 保留天数
     */
    virtual void cleanupOldMessages(int days = 30) = 0;

    /**
     * @brief 优化存储
     */
    virtual void optimize() = 0;

    /**
     * @brief 刷新缓存
     */
    virtual void refreshCache() = 0;

signals:
    /**
     * @brief 存储状态改变信号
     * @param status 新状态
     */
    void statusChanged(StorageStatus status);

    /**
     * @brief 消息存储成功信号
     * @param messageId 消息ID
     */
    void messageStored(const QString& messageId);

    /**
     * @brief 消息更新成功信号
     * @param messageId 消息ID
     */
    void messageUpdated(const QString& messageId);

    /**
     * @brief 消息删除成功信号
     * @param messageId 消息ID
     */
    void messageDeleted(const QString& messageId);

    /**
     * @brief 存储错误信号
     * @param error 错误信息
     */
    void storageError(const QString& error);

    /**
     * @brief 存储空间不足信号
     * @param availableSpace 可用空间（字节）
     */
    void lowStorageSpace(qint64 availableSpace);

    /**
     * @brief 备份完成信号
     * @param backupPath 备份路径
     * @param success 是否成功
     */
    void backupCompleted(const QString& backupPath, bool success);

    /**
     * @brief 恢复完成信号
     * @param backupPath 备份路径
     * @param success 是否成功
     */
    void restoreCompleted(const QString& backupPath, bool success);
};

Q_DECLARE_INTERFACE(IMessageStorage, "org.jitsi.chat.IMessageStorage/1.0")

#endif // IMESSAGESTORAGE_H