#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <functional>

/**
 * @brief 聊天管理器
 * 
 * 负责管理会议中的聊天功能
 */
class ChatManager : public QObject
{
    Q_OBJECT

public:
    explicit ChatManager(QObject* parent = nullptr);
    ~ChatManager();

    /**
     * @brief 发送聊天消息
     * @param message 消息内容
     * @param recipient 接收者（空表示发送给所有人）
     */
    void sendMessage(const QString& message, const QString& recipient = QString());

    /**
     * @brief 获取聊天历史
     * @return 聊天消息列表
     */
    QStringList getChatHistory() const;

    /**
     * @brief 清空聊天历史
     */
    void clearChatHistory();

    /**
     * @brief 设置XMPP客户端
     * @param client XMPP客户端实例
     */
    void setXMPPClient(QObject* client);

    /**
     * @brief 设置当前房间
     * @param roomName 房间名称
     */
    void setCurrentRoom(const QString& roomName);

    /**
     * @brief 标记所有消息为已读
     */
    void markAllAsRead();

    /**
     * @brief 标记房间所有消息为已读
     * @param roomId 房间ID（可选，默认当前房间）
     */
    void markRoomAsRead(const QString& roomId = QString());

    /**
     * @brief 初始化聊天管理器
     * @param config 配置参数
     * @return 初始化是否成功
     */
    bool initialize(const QVariantMap& config = QVariantMap());

    /**
     * @brief 连接到聊天服务
     * @param serverUrl 服务器地址
     * @param credentials 认证信息
     * @return 连接是否成功
     */
    bool connectToService(const QString& serverUrl, const QVariantMap& credentials = QVariantMap());

    /**
     * @brief 断开连接
     */
    void disconnect();

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 加入聊天室
     * @param roomId 房间ID
     * @param password 房间密码（可选）
     * @return 加入是否成功
     */
    bool joinRoom(const QString& roomId, const QString& password = QString());

    /**
     * @brief 离开聊天室
     * @param roomId 房间ID
     */
    void leaveRoom(const QString& roomId = QString());

    /**
     * @brief 获取当前聊天室
     * @return 当前房间ID
     */
    QString currentRoom() const;

    /**
     * @brief 获取已加入的聊天室列表
     * @return 房间ID列表
     */
    QStringList joinedRooms() const;

    /**
     * @brief 发送消息（重载版本）
     * @param message 消息内容
     * @param type 消息类型
     * @param roomId 目标房间ID（可选，默认当前房间）
     * @return 发送是否成功
     */
    bool sendMessage(const QString& message, int type, const QString& roomId = QString());

    /**
     * @brief 发送文件
     * @param filePath 文件路径
     * @param roomId 目标房间ID（可选，默认当前房间）
     * @return 发送是否成功
     */
    bool sendFile(const QString& filePath, const QString& roomId = QString());

    /**
     * @brief 获取消息历史
     * @param roomId 房间ID
     * @param limit 消息数量限制
     * @param before 获取此时间之前的消息
     * @return 消息列表
     */
    QList<class ChatMessage*> getMessageHistory(const QString& roomId, int limit = 50, const QDateTime& before = QDateTime());

    /**
     * @brief 搜索消息
     * @param query 搜索关键词
     * @param roomId 房间ID（可选，默认所有房间）
     * @return 匹配的消息列表
     */
    QList<class ChatMessage*> searchMessages(const QString& query, const QString& roomId = QString());

    /**
     * @brief 获取参与者列表
     * @param roomId 房间ID（可选，默认当前房间）
     * @return 参与者列表
     */
    QList<class Participant*> getParticipants(const QString& roomId = QString());

    /**
     * @brief 获取参与者数量
     * @param roomId 房间ID（可选，默认当前房间）
     * @return 参与者数量
     */
    int participantCount(const QString& roomId = QString()) const;

    /**
     * @brief 检查消息历史是否启用
     * @return 是否启用消息历史
     */
    bool isMessageHistoryEnabled() const;

    /**
     * @brief 设置消息历史启用状态
     * @param enabled 是否启用
     */
    void setMessageHistoryEnabled(bool enabled);

    /**
     * @brief 清除消息历史
     * @param roomId 房间ID（可选，默认所有房间）
     * @param before 清除此时间之前的消息（可选）
     */
    void clearMessageHistory(const QString& roomId = QString(), const QDateTime& before = QDateTime());

    /**
     * @brief 设置消息过滤器
     * @param filter 过滤器函数
     */
    void setMessageFilter(std::function<bool(const class ChatMessage*)> filter);

    /**
     * @brief 获取聊天统计信息
     * @return 统计信息
     */
    QVariantMap getStatistics() const;

    /**
     * @brief 重新连接
     */
    void reconnect();

    /**
     * @brief 刷新参与者列表
     * @param roomId 房间ID（可选，默认当前房间）
     */
    void refreshParticipants(const QString& roomId = QString());

    /**
     * @brief 标记消息为已读
     * @param messageId 消息ID
     */
    void markMessageAsRead(const QString& messageId);

    /**
     * @brief 处理接收到的消息
     * @param data 消息数据
     */
    void handleReceivedMessage(const QVariantMap& data);

    /**
     * @brief 处理连接错误
     * @param error 错误信息
     */
    void handleConnectionError(const QString& error);

    /**
     * @brief 验证房间ID
     * @param roomId 房间ID
     * @return 是否有效
     */
    bool validateRoomId(const QString& roomId) const;

    /**
     * @brief 处理消息过滤
     * @param message 消息对象
     * @return 是否通过过滤
     */
    bool filterMessage(class ChatMessage* message) const;

signals:
    /**
     * @brief 收到新消息信号
     * @param sender 发送者
     * @param message 消息内容
     */
    void messageReceived(const QString& sender, const QString& message);

    /**
     * @brief 消息发送成功信号
     * @param message 消息内容
     */
    void messageSent(const QString& message);

private:
    QStringList m_chatHistory;
};

#endif // CHATMANAGER_H