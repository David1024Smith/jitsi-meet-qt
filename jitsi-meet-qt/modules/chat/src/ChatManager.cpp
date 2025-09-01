#include "ChatManager.h"
#include "MessageHandler.h"
#include "IMessageStorage.h"
#include "ChatMessage.h"
#include "ChatRoom.h"
#include "Participant.h"

#include <QDebug>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <functional>

class ChatManager::Private
{
public:
    Private(ChatManager* q) : q_ptr(q) {}
    
    ChatManager* q_ptr;
    
    // 状态管理
    ConnectionStatus connectionStatus = Disconnected;
    bool initialized = false;
    
    // 连接信息
    QString serverUrl;
    QVariantMap credentials;
    QString currentRoomId;
    QStringList joinedRoomIds;
    
    // 组件
    MessageHandler* messageHandler = nullptr;
    IMessageStorage* messageStorage = nullptr;
    QNetworkAccessManager* networkManager = nullptr;
    
    // 数据
    QHash<QString, ChatRoom*> rooms;
    QHash<QString, QList<Participant*>> roomParticipants;
    QHash<QString, ChatMessage*> messages;
    
    // 配置
    bool messageHistoryEnabled = true;
    std::function<bool(const ChatMessage*)> messageFilter;
    
    // 统计
    int messagesSent = 0;
    int messagesReceived = 0;
    QDateTime connectionTime;
    
    // 定时器
    QTimer* reconnectTimer = nullptr;
    QTimer* heartbeatTimer = nullptr;
    
    void setConnectionStatus(ConnectionStatus status) {
        if (connectionStatus != status) {
            connectionStatus = status;
            emit q_ptr->connectionStatusChanged(status);
            emit q_ptr->connectionChanged(status == Connected);
            
            if (status == Connected) {
                connectionTime = QDateTime::currentDateTime();
                startHeartbeat();
            } else {
                stopHeartbeat();
            }
        }
    }
    
    void startHeartbeat() {
        if (!heartbeatTimer) {
            heartbeatTimer = new QTimer(q_ptr);
            connect(heartbeatTimer, &QTimer::timeout, q_ptr, [this]() {
                // 发送心跳包
                sendHeartbeat();
            });
        }
        heartbeatTimer->start(30000); // 30秒心跳
    }
    
    void stopHeartbeat() {
        if (heartbeatTimer) {
            heartbeatTimer->stop();
        }
    }
    
    void sendHeartbeat() {
        // TODO: 实现心跳包发送
        qDebug() << "Sending heartbeat...";
    }
    
    void startReconnectTimer() {
        if (!reconnectTimer) {
            reconnectTimer = new QTimer(q_ptr);
            reconnectTimer->setSingleShot(true);
            connect(reconnectTimer, &QTimer::timeout, q_ptr, &ChatManager::reconnect);
        }
        reconnectTimer->start(5000); // 5秒后重连
    }
    
    ChatRoom* getOrCreateRoom(const QString& roomId) {
        if (!rooms.contains(roomId)) {
            ChatRoom* room = new ChatRoom(roomId, roomId, ChatRoom::PublicRoom, q_ptr);
            rooms[roomId] = room;
        }
        return rooms[roomId];
    }
    
    void addParticipantToRoom(const QString& roomId, Participant* participant) {
        if (!roomParticipants.contains(roomId)) {
            roomParticipants[roomId] = QList<Participant*>();
        }
        
        if (!roomParticipants[roomId].contains(participant)) {
            roomParticipants[roomId].append(participant);
            emit q_ptr->participantJoined(participant, roomId);
            emit q_ptr->participantCountChanged(roomParticipants[roomId].size(), roomId);
        }
    }
    
    void removeParticipantFromRoom(const QString& roomId, const QString& participantId) {
        if (roomParticipants.contains(roomId)) {
            auto& participants = roomParticipants[roomId];
            for (int i = 0; i < participants.size(); ++i) {
                if (participants[i]->id() == participantId) {
                    participants.removeAt(i);
                    emit q_ptr->participantLeft(participantId, roomId);
                    emit q_ptr->participantCountChanged(participants.size(), roomId);
                    break;
                }
            }
        }
    }
};

ChatManager::ChatManager(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>(this))
{
    qDebug() << "ChatManager created";
}

ChatManager::~ChatManager()
{
    disconnect();
    qDebug() << "ChatManager destroyed";
}

bool ChatManager::initialize(const QVariantMap& config)
{
    if (d->initialized) {
        qWarning() << "ChatManager already initialized";
        return true;
    }
    
    qDebug() << "Initializing ChatManager...";
    
    try {
        // 创建网络管理器
        d->networkManager = new QNetworkAccessManager(this);
        
        // 创建消息处理器
        d->messageHandler = new MessageHandler(this);
        if (!d->messageHandler->initialize(config)) {
            qCritical() << "Failed to initialize MessageHandler";
            return false;
        }
        
        // 连接消息处理器信号
        connect(d->messageHandler, &MessageHandler::messageProcessed,
                this, [this](ChatMessage* message, MessageHandler::ProcessingResult result) {
                    if (result == IMessageHandler::Success) {
                        emit messageReceived(message);
                    }
                });
        connect(d->messageHandler, &MessageHandler::processingError,
                this, &ChatManager::handleConnectionError);
        
        // 应用配置
        if (config.contains("messageHistoryEnabled")) {
            d->messageHistoryEnabled = config["messageHistoryEnabled"].toBool();
        }
        
        d->initialized = true;
        qDebug() << "ChatManager initialized successfully";
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during ChatManager initialization:" << e.what();
        return false;
    }
}

bool ChatManager::connectToService(const QString& serverUrl, const QVariantMap& credentials)
{
    if (!d->initialized) {
        qWarning() << "ChatManager not initialized";
        return false;
    }
    
    if (d->connectionStatus == Connected || d->connectionStatus == Connecting) {
        qWarning() << "Already connected or connecting";
        return false;
    }
    
    qDebug() << "Connecting to chat service:" << serverUrl;
    
    d->serverUrl = serverUrl;
    d->credentials = credentials;
    d->setConnectionStatus(Connecting);
    
    // TODO: 实现实际的连接逻辑
    // 这里模拟连接过程
    QTimer::singleShot(1000, this, [this]() {
        // 模拟连接成功
        d->setConnectionStatus(Connected);
        qDebug() << "Connected to chat service";
    });
    
    return true;
}

void ChatManager::disconnect()
{
    if (d->connectionStatus == Disconnected) {
        return;
    }
    
    qDebug() << "Disconnecting from chat service...";
    
    d->setConnectionStatus(Disconnected);
    d->serverUrl.clear();
    d->credentials.clear();
    d->currentRoomId.clear();
    d->joinedRoomIds.clear();
    
    // 清理房间和参与者
    qDeleteAll(d->rooms);
    d->rooms.clear();
    d->roomParticipants.clear();
    
    qDebug() << "Disconnected from chat service";
}

bool ChatManager::isConnected() const
{
    return d->connectionStatus == Connected;
}

IChatManager::ConnectionStatus ChatManager::connectionStatus() const
{
    return static_cast<IChatManager::ConnectionStatus>(d->connectionStatus);
}

bool ChatManager::joinRoom(const QString& roomId, const QString& password)
{
    if (!isConnected()) {
        qWarning() << "Not connected to service";
        return false;
    }
    
    if (roomId.isEmpty()) {
        qWarning() << "Invalid room ID";
        return false;
    }
    
    if (d->joinedRoomIds.contains(roomId)) {
        qWarning() << "Already joined room:" << roomId;
        return true;
    }
    
    qDebug() << "Joining room:" << roomId;
    
    // TODO: 实现实际的加入房间逻辑
    // 这里模拟加入过程
    
    d->joinedRoomIds.append(roomId);
    d->currentRoomId = roomId;
    
    // 创建或获取房间对象
    ChatRoom* room = d->getOrCreateRoom(roomId);
    
    emit roomJoined(roomId);
    emit currentRoomChanged(roomId);
    
    qDebug() << "Joined room successfully:" << roomId;
    return true;
}

void ChatManager::leaveRoom(const QString& roomId)
{
    QString targetRoomId = roomId.isEmpty() ? d->currentRoomId : roomId;
    
    if (targetRoomId.isEmpty() || !d->joinedRoomIds.contains(targetRoomId)) {
        qWarning() << "Not in room:" << targetRoomId;
        return;
    }
    
    qDebug() << "Leaving room:" << targetRoomId;
    
    // TODO: 实现实际的离开房间逻辑
    
    d->joinedRoomIds.removeAll(targetRoomId);
    
    if (d->currentRoomId == targetRoomId) {
        d->currentRoomId = d->joinedRoomIds.isEmpty() ? QString() : d->joinedRoomIds.first();
        emit currentRoomChanged(d->currentRoomId);
    }
    
    // 清理房间参与者
    if (d->roomParticipants.contains(targetRoomId)) {
        d->roomParticipants.remove(targetRoomId);
    }
    
    emit roomLeft(targetRoomId);
    qDebug() << "Left room:" << targetRoomId;
}

QString ChatManager::currentRoom() const
{
    return d->currentRoomId;
}

QStringList ChatManager::joinedRooms() const
{
    return d->joinedRoomIds;
}

bool ChatManager::sendMessage(const QString& message, IChatManager::MessageType type, const QString& roomId)
{
    if (!isConnected()) {
        qWarning() << "Not connected to service";
        return false;
    }
    
    QString targetRoomId = roomId.isEmpty() ? d->currentRoomId : roomId;
    if (targetRoomId.isEmpty()) {
        qWarning() << "No target room specified";
        return false;
    }
    
    if (message.isEmpty()) {
        qWarning() << "Empty message";
        return false;
    }
    
    // 创建消息对象
    ChatMessage* chatMessage = new ChatMessage(message, "self", targetRoomId, 
                                               static_cast<ChatMessage::MessageType>(type), this);
    chatMessage->setStatus(ChatMessage::Sending);
    
    QString messageId = chatMessage->id();
    d->messages[messageId] = chatMessage;
    
    qDebug() << "Sending message to room" << targetRoomId << ":" << message;
    
    // 通过消息处理器处理
    IMessageHandler::ProcessingResult result = d->messageHandler->processOutgoingMessage(chatMessage);
    
    if (result == IMessageHandler::Success) {
        chatMessage->setStatus(ChatMessage::Sent);
        d->messagesSent++;
        emit messageSent(messageId);
        
        // TODO: 实际发送到服务器
        
        return true;
    } else {
        chatMessage->setStatus(ChatMessage::Failed);
        emit messageSendFailed(messageId, "Processing failed");
        return false;
    }
}

bool ChatManager::sendFile(const QString& filePath, const QString& roomId)
{
    // TODO: 实现文件发送
    Q_UNUSED(filePath)
    Q_UNUSED(roomId)
    qDebug() << "File sending not implemented yet";
    return false;
}

QList<ChatMessage*> ChatManager::getMessageHistory(const QString& roomId, int limit, const QDateTime& before)
{
    if (!d->messageHistoryEnabled) {
        return QList<ChatMessage*>();
    }
    
    // TODO: 从存储中获取消息历史
    Q_UNUSED(roomId)
    Q_UNUSED(limit)
    Q_UNUSED(before)
    
    QList<ChatMessage*> messages;
    // 返回当前内存中的消息作为示例
    for (auto it = d->messages.begin(); it != d->messages.end(); ++it) {
        messages.append(it.value());
    }
    
    return messages;
}

QList<ChatMessage*> ChatManager::searchMessages(const QString& query, const QString& roomId)
{
    QList<ChatMessage*> results;
    
    for (auto it = d->messages.begin(); it != d->messages.end(); ++it) {
        ChatMessage* message = it.value();
        if ((roomId.isEmpty() || message->roomId() == roomId) &&
            message->content().contains(query, Qt::CaseInsensitive)) {
            results.append(message);
        }
    }
    
    return results;
}

QList<Participant*> ChatManager::getParticipants(const QString& roomId)
{
    QString targetRoomId = roomId.isEmpty() ? d->currentRoomId : roomId;
    
    if (d->roomParticipants.contains(targetRoomId)) {
        return d->roomParticipants[targetRoomId];
    }
    
    return QList<Participant*>();
}

int ChatManager::participantCount(const QString& roomId) const
{
    QString targetRoomId = roomId.isEmpty() ? d->currentRoomId : roomId;
    
    if (d->roomParticipants.contains(targetRoomId)) {
        return d->roomParticipants[targetRoomId].size();
    }
    
    return 0;
}

bool ChatManager::isMessageHistoryEnabled() const
{
    return d->messageHistoryEnabled;
}

void ChatManager::setMessageHistoryEnabled(bool enabled)
{
    if (d->messageHistoryEnabled != enabled) {
        d->messageHistoryEnabled = enabled;
        emit messageHistoryEnabledChanged(enabled);
    }
}

void ChatManager::clearMessageHistory(const QString& roomId, const QDateTime& before)
{
    // TODO: 实现消息历史清理
    Q_UNUSED(roomId)
    Q_UNUSED(before)
    qDebug() << "Message history clearing not implemented yet";
}

void ChatManager::setMessageFilter(std::function<bool(const ChatMessage*)> filter)
{
    d->messageFilter = filter;
}

QVariantMap ChatManager::getStatistics() const
{
    QVariantMap stats;
    stats["messagesSent"] = d->messagesSent;
    stats["messagesReceived"] = d->messagesReceived;
    stats["joinedRooms"] = d->joinedRoomIds.size();
    stats["totalMessages"] = d->messages.size();
    
    if (d->connectionStatus == Connected && d->connectionTime.isValid()) {
        stats["connectionDuration"] = d->connectionTime.secsTo(QDateTime::currentDateTime());
    }
    
    return stats;
}

void ChatManager::reconnect()
{
    if (d->connectionStatus == Connected) {
        return;
    }
    
    qDebug() << "Attempting to reconnect...";
    d->setConnectionStatus(Reconnecting);
    
    if (!d->serverUrl.isEmpty()) {
        connectToService(d->serverUrl, d->credentials);
    }
}

void ChatManager::refreshParticipants(const QString& roomId)
{
    QString targetRoomId = roomId.isEmpty() ? d->currentRoomId : roomId;
    
    if (targetRoomId.isEmpty()) {
        return;
    }
    
    // TODO: 从服务器刷新参与者列表
    qDebug() << "Refreshing participants for room:" << targetRoomId;
}

void ChatManager::markMessageAsRead(const QString& messageId)
{
    if (d->messages.contains(messageId)) {
        ChatMessage* message = d->messages[messageId];
        message->markAsRead();
    }
}

void ChatManager::markRoomAsRead(const QString& roomId)
{
    QString targetRoomId = roomId.isEmpty() ? d->currentRoomId : roomId;
    
    for (auto it = d->messages.begin(); it != d->messages.end(); ++it) {
        ChatMessage* message = it.value();
        if (message->roomId() == targetRoomId) {
            message->markAsRead();
        }
    }
}

void ChatManager::handleReceivedMessage(const QVariantMap& data)
{
    // 解析接收到的消息
    ChatMessage* message = d->messageHandler->parseMessage(data);
    if (!message) {
        qWarning() << "Failed to parse received message";
        return;
    }
    
    // 应用过滤器
    if (!filterMessage(message)) {
        delete message;
        return;
    }
    
    d->messages[message->id()] = message;
    d->messagesReceived++;
    
    emit messageReceived(message);
}

void ChatManager::handleConnectionError(const QString& error)
{
    qWarning() << "Connection error:" << error;
    
    if (d->connectionStatus == Connected) {
        d->setConnectionStatus(Error);
        d->startReconnectTimer();
    }
    
    emit errorOccurred(error);
}

bool ChatManager::validateRoomId(const QString& roomId) const
{
    return !roomId.isEmpty() && roomId.length() <= 255;
}

bool ChatManager::filterMessage(ChatMessage* message) const
{
    if (d->messageFilter) {
        return d->messageFilter(message);
    }
    return true;
}

void ChatManager::setXMPPClient(QObject* client)
{
    // Store XMPP client reference for chat functionality
    Q_UNUSED(client)
}

void ChatManager::setCurrentRoom(const QString& roomId)
{
    if (d->currentRoomId != roomId) {
        d->currentRoomId = roomId;
        emit currentRoomChanged(roomId);
    }
}

void ChatManager::markAllAsRead()
{
    // Mark all messages in current room as read
    markRoomAsRead(d->currentRoomId);
}