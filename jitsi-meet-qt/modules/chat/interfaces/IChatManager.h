#ifndef ICHATMANAGER_H
#define ICHATMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDateTime>

// Forward declarations
class ChatMessage;
class ChatRoom;
class Participant;

/**
 * @brief 聊天管理器接口
 * 
 * IChatManager定义了聊天管理器的标准接口，
 * 提供聊天功能的抽象定义。
 */
class IChatManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 连接状态枚举
     */
    enum ConnectionStatus {
        Disconnected,       ///< 未连接
        Connecting,         ///< 连接中
        Connected,          ///< 已连接
        Reconnecting,       ///< 重连中
        Error               ///< 连接错误
    };
    Q_ENUM(ConnectionStatus)

    /**
     * @brief 消息类型枚举
     */
    enum MessageType {
        TextMessage,        ///< 文本消息
        EmojiMessage,       ///< 表情消息
        FileMessage,        ///< 文件消息
        SystemMessage,      ///< 系统消息
        NotificationMessage ///< 通知消息
    };
    Q_ENUM(MessageType)

    /**
     * @brief 虚析构函数
     */
    virtual ~IChatManager() = default;

    /**
     * @brief 初始化聊天管理器
     * @param config 配置参数
     * @return 初始化是否成功
     */
    virtual bool initialize(const QVariantMap& config = QVariantMap()) = 0;

    /**
     * @brief 连接到聊天服务
     * @param serverUrl 服务器地址
     * @param credentials 认证信息
     * @return 连接是否成功
     */
    virtual bool connectToService(const QString& serverUrl, const QVariantMap& credentials = QVariantMap()) = 0;

    /**
     * @brief 断开连接
     */
    virtual void disconnect() = 0;

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief 获取连接状态
     * @return 连接状态
     */
    virtual ConnectionStatus connectionStatus() const = 0;

    /**
     * @brief 加入聊天室
     * @param roomId 房间ID
     * @param password 房间密码（可选）
     * @return 加入是否成功
     */
    virtual bool joinRoom(const QString& roomId, const QString& password = QString()) = 0;

    /**
     * @brief 离开聊天室
     * @param roomId 房间ID
     */
    virtual void leaveRoom(const QString& roomId = QString()) = 0;

    /**
     * @brief 获取当前聊天室
     * @return 当前房间ID
     */
    virtual QString currentRoom() const = 0;

    /**
     * @brief 获取已加入的聊天室列表
     * @return 房间ID列表
     */
    virtual QStringList joinedRooms() const = 0;

    /**
     * @brief 发送消息
     * @param message 消息内容
     * @param type 消息类型
     * @param roomId 目标房间ID（可选，默认当前房间）
     * @return 发送是否成功
     */
    virtual bool sendMessage(const QString& message, MessageType type = TextMessage, const QString& roomId = QString()) = 0;

    /**
     * @brief 发送文件
     * @param filePath 文件路径
     * @param roomId 目标房间ID（可选，默认当前房间）
     * @return 发送是否成功
     */
    virtual bool sendFile(const QString& filePath, const QString& roomId = QString()) = 0;

    /**
     * @brief 获取消息历史
     * @param roomId 房间ID
     * @param limit 消息数量限制
     * @param before 获取此时间之前的消息
     * @return 消息列表
     */
    virtual QList<ChatMessage*> getMessageHistory(const QString& roomId, int limit = 50, const QDateTime& before = QDateTime()) = 0;

    /**
     * @brief 搜索消息
     * @param query 搜索关键词
     * @param roomId 房间ID（可选，默认所有房间）
     * @return 匹配的消息列表
     */
    virtual QList<ChatMessage*> searchMessages(const QString& query, const QString& roomId = QString()) = 0;

    /**
     * @brief 获取参与者列表
     * @param roomId 房间ID（可选，默认当前房间）
     * @return 参与者列表
     */
    virtual QList<Participant*> getParticipants(const QString& roomId = QString()) = 0;

    /**
     * @brief 获取参与者数量
     * @param roomId 房间ID（可选，默认当前房间）
     * @return 参与者数量
     */
    virtual int participantCount(const QString& roomId = QString()) const = 0;

    /**
     * @brief 检查消息历史是否启用
     * @return 是否启用消息历史
     */
    virtual bool isMessageHistoryEnabled() const = 0;

    /**
     * @brief 设置消息历史启用状态
     * @param enabled 是否启用
     */
    virtual void setMessageHistoryEnabled(bool enabled) = 0;

    /**
     * @brief 清除消息历史
     * @param roomId 房间ID（可选，默认所有房间）
     * @param before 清除此时间之前的消息（可选）
     */
    virtual void clearMessageHistory(const QString& roomId = QString(), const QDateTime& before = QDateTime()) = 0;

    /**
     * @brief 获取聊天统计信息
     * @return 统计信息
     */
    virtual QVariantMap getStatistics() const = 0;

public slots:
    /**
     * @brief 重新连接
     */
    virtual void reconnect() = 0;

    /**
     * @brief 刷新参与者列表
     * @param roomId 房间ID（可选，默认当前房间）
     */
    virtual void refreshParticipants(const QString& roomId = QString()) = 0;

    /**
     * @brief 标记消息为已读
     * @param messageId 消息ID
     */
    virtual void markMessageAsRead(const QString& messageId) = 0;

    /**
     * @brief 标记房间所有消息为已读
     * @param roomId 房间ID（可选，默认当前房间）
     */
    virtual void markRoomAsRead(const QString& roomId = QString()) = 0;

signals:
    /**
     * @brief 连接状态改变信号
     * @param connected 是否已连接
     */
    void connectionChanged(bool connected);

    /**
     * @brief 连接状态改变信号
     * @param status 连接状态
     */
    void connectionStatusChanged(ConnectionStatus status);

    /**
     * @brief 当前房间改变信号
     * @param roomId 新的房间ID
     */
    void currentRoomChanged(const QString& roomId);

    /**
     * @brief 加入房间信号
     * @param roomId 房间ID
     */
    void roomJoined(const QString& roomId);

    /**
     * @brief 离开房间信号
     * @param roomId 房间ID
     */
    void roomLeft(const QString& roomId);

    /**
     * @brief 收到消息信号
     * @param message 消息对象
     */
    void messageReceived(ChatMessage* message);

    /**
     * @brief 消息发送成功信号
     * @param messageId 消息ID
     */
    void messageSent(const QString& messageId);

    /**
     * @brief 消息发送失败信号
     * @param messageId 消息ID
     * @param error 错误信息
     */
    void messageSendFailed(const QString& messageId, const QString& error);

    /**
     * @brief 参与者加入信号
     * @param participant 参与者对象
     * @param roomId 房间ID
     */
    void participantJoined(Participant* participant, const QString& roomId);

    /**
     * @brief 参与者离开信号
     * @param participantId 参与者ID
     * @param roomId 房间ID
     */
    void participantLeft(const QString& participantId, const QString& roomId);

    /**
     * @brief 参与者数量改变信号
     * @param count 新的参与者数量
     * @param roomId 房间ID
     */
    void participantCountChanged(int count, const QString& roomId = QString());

    /**
     * @brief 消息历史启用状态改变信号
     * @param enabled 是否启用
     */
    void messageHistoryEnabledChanged(bool enabled);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);
};

Q_DECLARE_INTERFACE(IChatManager, "org.jitsi.chat.IChatManager/1.0")

#endif // ICHATMANAGER_H