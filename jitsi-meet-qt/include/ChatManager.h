#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QSettings>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "XMPPClient.h"

/**
 * @brief 聊天管理器类，处理Jitsi Meet会议中的文字消息功能
 * 
 * 该类实现了聊天系统的核心功能，包括：
 * - 消息发送和接收
 * - 消息历史记录管理
 * - 消息持久化存储
 * - 未读消息计数
 * - 消息通知机制
 */
class ChatManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 聊天消息结构
     */
    struct ChatMessage {
        QString messageId;      ///< 消息唯一ID
        QString senderId;       ///< 发送者JID
        QString senderName;     ///< 发送者显示名称
        QString content;        ///< 消息内容
        QDateTime timestamp;    ///< 发送时间戳
        bool isLocal;          ///< 是否为本地发送的消息
        bool isRead;           ///< 是否已读
        QString roomName;      ///< 所属房间名称
        
        /**
         * @brief 转换为JSON对象
         */
        QJsonObject toJson() const;
        
        /**
         * @brief 从JSON对象创建消息
         */
        static ChatMessage fromJson(const QJsonObject& json);
        
        /**
         * @brief 检查消息是否有效
         */
        bool isValid() const;
    };

    explicit ChatManager(QObject *parent = nullptr);
    ~ChatManager();

    /**
     * @brief 设置XMPP客户端
     * @param xmppClient XMPP客户端实例
     */
    void setXMPPClient(XMPPClient* xmppClient);

    /**
     * @brief 发送聊天消息
     * @param content 消息内容
     * @return 是否发送成功
     */
    bool sendMessage(const QString& content);

    /**
     * @brief 获取当前房间的消息历史
     * @return 消息列表
     */
    QList<ChatMessage> messageHistory() const;

    /**
     * @brief 获取指定房间的消息历史
     * @param roomName 房间名称
     * @return 消息列表
     */
    QList<ChatMessage> messageHistory(const QString& roomName) const;

    /**
     * @brief 清空当前房间的消息历史
     */
    void clearHistory();

    /**
     * @brief 清空指定房间的消息历史
     * @param roomName 房间名称
     */
    void clearHistory(const QString& roomName);

    /**
     * @brief 清空所有消息历史
     */
    void clearAllHistory();

    /**
     * @brief 获取未读消息数量
     * @return 未读消息数量
     */
    int unreadCount() const;

    /**
     * @brief 获取指定房间的未读消息数量
     * @param roomName 房间名称
     * @return 未读消息数量
     */
    int unreadCount(const QString& roomName) const;

    /**
     * @brief 标记所有消息为已读
     */
    void markAllAsRead();

    /**
     * @brief 标记指定房间的所有消息为已读
     * @param roomName 房间名称
     */
    void markAllAsRead(const QString& roomName);

    /**
     * @brief 标记指定消息为已读
     * @param messageId 消息ID
     */
    void markAsRead(const QString& messageId);

    /**
     * @brief 获取当前房间名称
     * @return 房间名称
     */
    QString currentRoom() const;

    /**
     * @brief 设置当前房间
     * @param roomName 房间名称
     */
    void setCurrentRoom(const QString& roomName);

    /**
     * @brief 获取最大历史消息数量
     * @return 最大消息数量
     */
    int maxHistorySize() const;

    /**
     * @brief 设置最大历史消息数量
     * @param maxSize 最大消息数量
     */
    void setMaxHistorySize(int maxSize);

    /**
     * @brief 启用/禁用消息持久化
     * @param enabled 是否启用
     */
    void setPersistenceEnabled(bool enabled);

    /**
     * @brief 检查消息持久化是否启用
     * @return 是否启用
     */
    bool isPersistenceEnabled() const;

    /**
     * @brief 搜索消息
     * @param query 搜索关键词
     * @param roomName 房间名称（空表示搜索所有房间）
     * @return 匹配的消息列表
     */
    QList<ChatMessage> searchMessages(const QString& query, const QString& roomName = QString()) const;

    /**
     * @brief 导出消息历史
     * @param filePath 导出文件路径
     * @param roomName 房间名称（空表示导出所有房间）
     * @return 是否导出成功
     */
    bool exportHistory(const QString& filePath, const QString& roomName = QString()) const;

    /**
     * @brief 导入消息历史
     * @param filePath 导入文件路径
     * @return 是否导入成功
     */
    bool importHistory(const QString& filePath);

signals:
    /**
     * @brief 消息接收信号
     * @param message 接收到的消息
     */
    void messageReceived(const ChatMessage& message);

    /**
     * @brief 消息发送信号
     * @param message 发送的消息
     */
    void messageSent(const ChatMessage& message);

    /**
     * @brief 消息发送失败信号
     * @param content 消息内容
     * @param error 错误信息
     */
    void messageSendFailed(const QString& content, const QString& error);

    /**
     * @brief 未读消息数量变化信号
     * @param count 新的未读消息数量
     */
    void unreadCountChanged(int count);

    /**
     * @brief 消息历史变化信号
     */
    void historyChanged();

    /**
     * @brief 消息通知信号（用于显示通知）
     * @param senderName 发送者名称
     * @param content 消息内容
     * @param roomName 房间名称
     */
    void messageNotification(const QString& senderName, const QString& content, const QString& roomName);

private slots:
    /**
     * @brief 处理XMPP消息接收
     * @param from 发送者JID
     * @param message 消息内容
     * @param timestamp 时间戳
     */
    void onXMPPMessageReceived(const QString& from, const QString& message, const QDateTime& timestamp);

    /**
     * @brief 处理XMPP连接状态变化
     * @param state 连接状态
     */
    void onXMPPConnectionStateChanged(XMPPClient::ConnectionState state);

    /**
     * @brief 处理房间加入事件
     */
    void onRoomJoined();

    /**
     * @brief 处理房间离开事件
     */
    void onRoomLeft();

    /**
     * @brief 自动保存定时器处理
     */
    void onAutoSaveTimer();

private:
    /**
     * @brief 添加消息到历史记录
     * @param message 消息对象
     */
    void addMessageToHistory(const ChatMessage& message);

    /**
     * @brief 生成唯一消息ID
     * @return 消息ID
     */
    QString generateMessageId();

    /**
     * @brief 解析发送者名称从JID
     * @param jid 发送者JID
     * @return 显示名称
     */
    QString extractSenderName(const QString& jid);

    /**
     * @brief 加载消息历史从持久化存储
     */
    void loadMessageHistory();

    /**
     * @brief 保存消息历史到持久化存储
     */
    void saveMessageHistory();

    /**
     * @brief 清理过期消息
     */
    void cleanupOldMessages();

    /**
     * @brief 限制消息历史大小
     * @param roomName 房间名称
     */
    void limitHistorySize(const QString& roomName);

    /**
     * @brief 更新未读计数
     */
    void updateUnreadCount();

    /**
     * @brief 验证消息内容
     * @param content 消息内容
     * @return 是否有效
     */
    bool validateMessageContent(const QString& content) const;

    /**
     * @brief 过滤和清理消息内容
     * @param content 原始内容
     * @return 清理后的内容
     */
    QString sanitizeMessageContent(const QString& content) const;

    /**
     * @brief 加载配置选项
     */
    void loadConfiguration();

    // 核心组件
    XMPPClient* m_xmppClient;               ///< XMPP客户端
    QSettings* m_settings;                  ///< 设置存储
    QTimer* m_autoSaveTimer;               ///< 自动保存定时器

    // 消息数据
    QMap<QString, QList<ChatMessage>> m_messageHistory;  ///< 按房间分组的消息历史
    QString m_currentRoom;                  ///< 当前房间名称
    int m_totalUnreadCount;                ///< 总未读消息数量
    QMap<QString, int> m_unreadCounts;     ///< 按房间分组的未读计数

    // 配置选项
    int m_maxHistorySize;                  ///< 最大历史消息数量
    bool m_persistenceEnabled;             ///< 是否启用持久化
    int m_autoSaveInterval;                ///< 自动保存间隔（毫秒）
    int m_maxMessageLength;                ///< 最大消息长度
    int m_historyRetentionDays;            ///< 历史消息保留天数

    // 常量
    static const int DEFAULT_MAX_HISTORY_SIZE = 1000;      ///< 默认最大历史大小
    static const int DEFAULT_AUTO_SAVE_INTERVAL = 30000;   ///< 默认自动保存间隔
    static const int DEFAULT_MAX_MESSAGE_LENGTH = 4096;    ///< 默认最大消息长度
    static const int DEFAULT_RETENTION_DAYS = 30;          ///< 默认保留天数
};

#endif // CHATMANAGER_H