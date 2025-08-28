#include "ChatManager.h"
#include <QDebug>
#include <QUuid>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>

// ChatMessage implementation
QJsonObject ChatManager::ChatMessage::toJson() const
{
    QJsonObject json;
    json["messageId"] = messageId;
    json["senderId"] = senderId;
    json["senderName"] = senderName;
    json["content"] = content;
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    json["isLocal"] = isLocal;
    json["isRead"] = isRead;
    json["roomName"] = roomName;
    return json;
}

ChatManager::ChatMessage ChatManager::ChatMessage::fromJson(const QJsonObject& json)
{
    ChatMessage message;
    message.messageId = json["messageId"].toString();
    message.senderId = json["senderId"].toString();
    message.senderName = json["senderName"].toString();
    message.content = json["content"].toString();
    message.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    message.isLocal = json["isLocal"].toBool();
    message.isRead = json["isRead"].toBool();
    message.roomName = json["roomName"].toString();
    return message;
}

bool ChatManager::ChatMessage::isValid() const
{
    return !messageId.isEmpty() && 
           !senderId.isEmpty() && 
           !content.isEmpty() && 
           timestamp.isValid() &&
           !roomName.isEmpty();
}

// ChatManager implementation
ChatManager::ChatManager(QObject *parent)
    : QObject(parent)
    , m_xmppClient(nullptr)
    , m_settings(nullptr)
    , m_autoSaveTimer(new QTimer(this))
    , m_totalUnreadCount(0)
    , m_maxHistorySize(DEFAULT_MAX_HISTORY_SIZE)
    , m_persistenceEnabled(true)
    , m_autoSaveInterval(DEFAULT_AUTO_SAVE_INTERVAL)
    , m_maxMessageLength(DEFAULT_MAX_MESSAGE_LENGTH)
    , m_historyRetentionDays(DEFAULT_RETENTION_DAYS)
{
    // 初始化设置存储
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    m_settings = new QSettings(configPath + "/chat_history.ini", QSettings::IniFormat, this);

    // 设置自动保存定时器
    m_autoSaveTimer->setSingleShot(false);
    m_autoSaveTimer->setInterval(m_autoSaveInterval);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &ChatManager::onAutoSaveTimer);

    // 加载配置
    loadConfiguration();

    // 加载消息历史
    if (m_persistenceEnabled) {
        loadMessageHistory();
    }

    qDebug() << "ChatManager initialized with persistence:" << m_persistenceEnabled;
}

ChatManager::~ChatManager()
{
    // 保存消息历史
    if (m_persistenceEnabled) {
        saveMessageHistory();
    }
    qDebug() << "ChatManager destroyed";
}

void ChatManager::setXMPPClient(XMPPClient* xmppClient)
{
    // 断开之前的连接
    if (m_xmppClient) {
        disconnect(m_xmppClient, nullptr, this, nullptr);
    }

    m_xmppClient = xmppClient;

    if (m_xmppClient) {
        // 连接XMPP客户端信号
        connect(m_xmppClient, &XMPPClient::chatMessageReceived,
                this, &ChatManager::onXMPPMessageReceived);
        connect(m_xmppClient, &XMPPClient::connectionStateChanged,
                this, &ChatManager::onXMPPConnectionStateChanged);
        connect(m_xmppClient, &XMPPClient::roomJoined,
                this, &ChatManager::onRoomJoined);
        connect(m_xmppClient, &XMPPClient::roomLeft,
                this, &ChatManager::onRoomLeft);

        qDebug() << "XMPP client connected to ChatManager";
    }
}

bool ChatManager::sendMessage(const QString& content)
{
    if (!m_xmppClient || !m_xmppClient->isInRoom()) {
        qWarning() << "Cannot send message: not connected to room";
        emit messageSendFailed(content, "Not connected to room");
        return false;
    }

    if (!validateMessageContent(content)) {
        qWarning() << "Invalid message content";
        emit messageSendFailed(content, "Invalid message content");
        return false;
    }

    // 清理消息内容
    QString sanitizedContent = sanitizeMessageContent(content);

    // 创建本地消息记录
    ChatMessage message;
    message.messageId = generateMessageId();
    message.senderId = m_xmppClient->userJid();
    message.senderName = m_xmppClient->displayName();
    message.content = sanitizedContent;
    message.timestamp = QDateTime::currentDateTime();
    message.isLocal = true;
    message.isRead = true; // 本地发送的消息默认已读
    message.roomName = m_currentRoom;

    // 发送XMPP消息
    m_xmppClient->sendChatMessage(sanitizedContent);

    // 添加到历史记录
    addMessageToHistory(message);

    emit messageSent(message);
    qDebug() << "Message sent:" << sanitizedContent;

    return true;
}

QList<ChatManager::ChatMessage> ChatManager::messageHistory() const
{
    return messageHistory(m_currentRoom);
}

QList<ChatManager::ChatMessage> ChatManager::messageHistory(const QString& roomName) const
{
    return m_messageHistory.value(roomName, QList<ChatMessage>());
}

void ChatManager::clearHistory()
{
    clearHistory(m_currentRoom);
}

void ChatManager::clearHistory(const QString& roomName)
{
    if (m_messageHistory.contains(roomName)) {
        m_messageHistory[roomName].clear();
        m_unreadCounts[roomName] = 0;
        updateUnreadCount();
        
        if (m_persistenceEnabled) {
            saveMessageHistory();
        }
        
        emit historyChanged();
        qDebug() << "Cleared message history for room:" << roomName;
    }
}

void ChatManager::clearAllHistory()
{
    m_messageHistory.clear();
    m_unreadCounts.clear();
    m_totalUnreadCount = 0;
    
    if (m_persistenceEnabled) {
        saveMessageHistory();
    }
    
    emit unreadCountChanged(0);
    emit historyChanged();
    qDebug() << "Cleared all message history";
}

int ChatManager::unreadCount() const
{
    return m_totalUnreadCount;
}

int ChatManager::unreadCount(const QString& roomName) const
{
    return m_unreadCounts.value(roomName, 0);
}

void ChatManager::markAllAsRead()
{
    markAllAsRead(m_currentRoom);
}

void ChatManager::markAllAsRead(const QString& roomName)
{
    if (!m_messageHistory.contains(roomName)) {
        return;
    }

    bool changed = false;
    QList<ChatMessage>& messages = m_messageHistory[roomName];
    
    for (ChatMessage& message : messages) {
        if (!message.isRead) {
            message.isRead = true;
            changed = true;
        }
    }

    if (changed) {
        m_unreadCounts[roomName] = 0;
        updateUnreadCount();
        
        if (m_persistenceEnabled) {
            saveMessageHistory();
        }
        
        qDebug() << "Marked all messages as read for room:" << roomName;
    }
}

void ChatManager::markAsRead(const QString& messageId)
{
    bool found = false;
    
    for (auto& roomMessages : m_messageHistory) {
        for (ChatMessage& message : roomMessages) {
            if (message.messageId == messageId && !message.isRead) {
                message.isRead = true;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (found) {
        updateUnreadCount();
        
        if (m_persistenceEnabled) {
            saveMessageHistory();
        }
        
        qDebug() << "Marked message as read:" << messageId;
    }
}

QString ChatManager::currentRoom() const
{
    return m_currentRoom;
}

void ChatManager::setCurrentRoom(const QString& roomName)
{
    if (m_currentRoom != roomName) {
        m_currentRoom = roomName;
        qDebug() << "Current room changed to:" << roomName;
        
        // 标记当前房间的消息为已读
        if (!roomName.isEmpty()) {
            markAllAsRead(roomName);
        }
    }
}

int ChatManager::maxHistorySize() const
{
    return m_maxHistorySize;
}

void ChatManager::setMaxHistorySize(int maxSize)
{
    if (maxSize > 0 && m_maxHistorySize != maxSize) {
        m_maxHistorySize = maxSize;
        
        // 清理超出限制的消息
        for (const QString& roomName : m_messageHistory.keys()) {
            limitHistorySize(roomName);
        }
        
        qDebug() << "Max history size changed to:" << maxSize;
    }
}

void ChatManager::setPersistenceEnabled(bool enabled)
{
    if (m_persistenceEnabled != enabled) {
        m_persistenceEnabled = enabled;
        
        if (enabled) {
            loadMessageHistory();
            m_autoSaveTimer->start();
        } else {
            m_autoSaveTimer->stop();
        }
        
        qDebug() << "Persistence enabled:" << enabled;
    }
}

bool ChatManager::isPersistenceEnabled() const
{
    return m_persistenceEnabled;
}

QList<ChatManager::ChatMessage> ChatManager::searchMessages(const QString& query, const QString& roomName) const
{
    QList<ChatMessage> results;
    
    if (query.isEmpty()) {
        return results;
    }

    QRegularExpression regex(QRegularExpression::escape(query), QRegularExpression::CaseInsensitiveOption);
    
    auto searchInRoom = [&](const QList<ChatMessage>& messages) {
        for (const ChatMessage& message : messages) {
            if (message.content.contains(regex) || 
                message.senderName.contains(regex)) {
                results.append(message);
            }
        }
    };

    if (roomName.isEmpty()) {
        // 搜索所有房间
        for (const QList<ChatMessage>& messages : m_messageHistory) {
            searchInRoom(messages);
        }
    } else {
        // 搜索指定房间
        if (m_messageHistory.contains(roomName)) {
            searchInRoom(m_messageHistory[roomName]);
        }
    }

    // 按时间戳排序
    std::sort(results.begin(), results.end(), 
              [](const ChatMessage& a, const ChatMessage& b) {
                  return a.timestamp > b.timestamp;
              });

    qDebug() << "Search found" << results.size() << "messages for query:" << query;
    return results;
}

bool ChatManager::exportHistory(const QString& filePath, const QString& roomName) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for export:" << filePath;
        return false;
    }

    QJsonObject exportData;
    exportData["exportTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    exportData["version"] = "1.0";

    QJsonArray roomsArray;
    
    auto exportRoom = [&](const QString& room, const QList<ChatMessage>& messages) {
        QJsonObject roomObject;
        roomObject["roomName"] = room;
        
        QJsonArray messagesArray;
        for (const ChatMessage& message : messages) {
            messagesArray.append(message.toJson());
        }
        roomObject["messages"] = messagesArray;
        roomsArray.append(roomObject);
    };

    if (roomName.isEmpty()) {
        // 导出所有房间
        for (auto it = m_messageHistory.constBegin(); it != m_messageHistory.constEnd(); ++it) {
            exportRoom(it.key(), it.value());
        }
    } else {
        // 导出指定房间
        if (m_messageHistory.contains(roomName)) {
            exportRoom(roomName, m_messageHistory[roomName]);
        }
    }

    exportData["rooms"] = roomsArray;

    QTextStream stream(&file);
    stream << QJsonDocument(exportData).toJson();
    
    qDebug() << "Exported message history to:" << filePath;
    return true;
}

bool ChatManager::importHistory(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for import:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse import file:" << parseError.errorString();
        return false;
    }

    QJsonObject importData = doc.object();
    QJsonArray roomsArray = importData["rooms"].toArray();
    
    int importedCount = 0;
    
    for (const QJsonValue& roomValue : roomsArray) {
        QJsonObject roomObject = roomValue.toObject();
        QString roomName = roomObject["roomName"].toString();
        QJsonArray messagesArray = roomObject["messages"].toArray();
        
        for (const QJsonValue& messageValue : messagesArray) {
            ChatMessage message = ChatMessage::fromJson(messageValue.toObject());
            if (message.isValid()) {
                addMessageToHistory(message);
                importedCount++;
            }
        }
    }

    if (m_persistenceEnabled) {
        saveMessageHistory();
    }
    
    emit historyChanged();
    qDebug() << "Imported" << importedCount << "messages from:" << filePath;
    return true;
}

// Private slots implementation

void ChatManager::onXMPPMessageReceived(const QString& from, const QString& message, const QDateTime& timestamp)
{
    // 创建消息对象
    ChatMessage chatMessage;
    chatMessage.messageId = generateMessageId();
    chatMessage.senderId = from;
    chatMessage.senderName = extractSenderName(from);
    chatMessage.content = message;
    chatMessage.timestamp = timestamp;
    chatMessage.isLocal = false;
    chatMessage.isRead = (from == m_xmppClient->userJid()); // 自己发送的消息标记为已读
    chatMessage.roomName = m_currentRoom;

    // 添加到历史记录
    addMessageToHistory(chatMessage);

    emit messageReceived(chatMessage);
    
    // 发送通知（如果不是自己发送的消息）
    if (!chatMessage.isLocal) {
        emit messageNotification(chatMessage.senderName, chatMessage.content, chatMessage.roomName);
    }

    qDebug() << "Received message from" << chatMessage.senderName << ":" << message;
}

void ChatManager::onXMPPConnectionStateChanged(XMPPClient::ConnectionState state)
{
    if (state == XMPPClient::Connected || state == XMPPClient::InRoom) {
        // 连接成功，启动自动保存
        if (m_persistenceEnabled) {
            m_autoSaveTimer->start();
        }
    } else if (state == XMPPClient::Disconnected || state == XMPPClient::Error) {
        // 连接断开，停止自动保存
        m_autoSaveTimer->stop();
        
        // 保存当前消息历史
        if (m_persistenceEnabled) {
            saveMessageHistory();
        }
    }
}

void ChatManager::onRoomJoined()
{
    if (m_xmppClient) {
        setCurrentRoom(m_xmppClient->currentRoom());
        qDebug() << "Joined room, chat manager updated to room:" << m_currentRoom;
    }
}

void ChatManager::onRoomLeft()
{
    // 保存消息历史
    if (m_persistenceEnabled) {
        saveMessageHistory();
    }
    
    qDebug() << "Left room, chat history saved";
}

void ChatManager::onAutoSaveTimer()
{
    if (m_persistenceEnabled) {
        saveMessageHistory();
        qDebug() << "Auto-saved message history";
    }
}

// Private methods implementation

void ChatManager::addMessageToHistory(const ChatMessage& message)
{
    if (!message.isValid()) {
        qWarning() << "Invalid message, not adding to history";
        return;
    }

    QString roomName = message.roomName.isEmpty() ? m_currentRoom : message.roomName;
    
    // 添加消息到历史记录
    m_messageHistory[roomName].append(message);
    
    // 更新未读计数
    if (!message.isRead) {
        m_unreadCounts[roomName]++;
        updateUnreadCount();
    }
    
    // 限制历史大小
    limitHistorySize(roomName);
    
    emit historyChanged();
}

QString ChatManager::generateMessageId()
{
    return QUuid::createUuid().toString().remove('{').remove('}').remove('-');
}

QString ChatManager::extractSenderName(const QString& jid)
{
    // 从JID中提取显示名称
    // 格式: room@conference.domain/displayName
    QRegularExpression jidRegex("^.*?/(.+)$");
    QRegularExpressionMatch match = jidRegex.match(jid);
    
    if (match.hasMatch()) {
        return match.captured(1);
    }
    
    // 如果无法解析，返回JID的最后部分
    int lastSlash = jid.lastIndexOf('/');
    if (lastSlash >= 0) {
        return jid.mid(lastSlash + 1);
    }
    
    return jid;
}

void ChatManager::loadMessageHistory()
{
    m_settings->beginGroup("MessageHistory");
    QStringList rooms = m_settings->childGroups();
    
    for (const QString& room : rooms) {
        m_settings->beginGroup(room);
        
        QList<ChatMessage> messages;
        int messageCount = m_settings->beginReadArray("Messages");
        
        for (int i = 0; i < messageCount; ++i) {
            m_settings->setArrayIndex(i);
            
            ChatMessage message;
            message.messageId = m_settings->value("messageId").toString();
            message.senderId = m_settings->value("senderId").toString();
            message.senderName = m_settings->value("senderName").toString();
            message.content = m_settings->value("content").toString();
            message.timestamp = m_settings->value("timestamp").toDateTime();
            message.isLocal = m_settings->value("isLocal").toBool();
            message.isRead = m_settings->value("isRead").toBool();
            message.roomName = room;
            
            if (message.isValid()) {
                messages.append(message);
                
                if (!message.isRead) {
                    m_unreadCounts[room]++;
                }
            }
        }
        
        m_settings->endArray();
        m_settings->endGroup();
        
        if (!messages.isEmpty()) {
            m_messageHistory[room] = messages;
        }
    }
    
    m_settings->endGroup();
    
    // 清理过期消息
    cleanupOldMessages();
    
    // 更新未读计数
    updateUnreadCount();
    
    qDebug() << "Loaded message history for" << m_messageHistory.size() << "rooms";
}

void ChatManager::saveMessageHistory()
{
    m_settings->clear();
    m_settings->beginGroup("MessageHistory");
    
    for (auto it = m_messageHistory.constBegin(); it != m_messageHistory.constEnd(); ++it) {
        const QString& room = it.key();
        const QList<ChatMessage>& messages = it.value();
        
        if (messages.isEmpty()) {
            continue;
        }
        
        m_settings->beginGroup(room);
        m_settings->beginWriteArray("Messages");
        
        for (int i = 0; i < messages.size(); ++i) {
            const ChatMessage& message = messages[i];
            m_settings->setArrayIndex(i);
            
            m_settings->setValue("messageId", message.messageId);
            m_settings->setValue("senderId", message.senderId);
            m_settings->setValue("senderName", message.senderName);
            m_settings->setValue("content", message.content);
            m_settings->setValue("timestamp", message.timestamp);
            m_settings->setValue("isLocal", message.isLocal);
            m_settings->setValue("isRead", message.isRead);
        }
        
        m_settings->endArray();
        m_settings->endGroup();
    }
    
    m_settings->endGroup();
    m_settings->sync();
    
    qDebug() << "Saved message history";
}

void ChatManager::cleanupOldMessages()
{
    if (m_historyRetentionDays <= 0) {
        return;
    }
    
    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-m_historyRetentionDays);
    bool changed = false;
    
    for (auto& roomMessages : m_messageHistory) {
        auto it = roomMessages.begin();
        while (it != roomMessages.end()) {
            if (it->timestamp < cutoffTime) {
                if (!it->isRead) {
                    m_unreadCounts[it->roomName]--;
                }
                it = roomMessages.erase(it);
                changed = true;
            } else {
                ++it;
            }
        }
    }
    
    if (changed) {
        updateUnreadCount();
        qDebug() << "Cleaned up old messages older than" << m_historyRetentionDays << "days";
    }
}

void ChatManager::limitHistorySize(const QString& roomName)
{
    if (!m_messageHistory.contains(roomName)) {
        return;
    }
    
    QList<ChatMessage>& messages = m_messageHistory[roomName];
    
    if (messages.size() > m_maxHistorySize) {
        int removeCount = messages.size() - m_maxHistorySize;
        
        // 移除最旧的消息
        for (int i = 0; i < removeCount; ++i) {
            const ChatMessage& message = messages[i];
            if (!message.isRead) {
                m_unreadCounts[roomName]--;
            }
        }
        
        messages = messages.mid(removeCount);
        updateUnreadCount();
        
        qDebug() << "Limited history size for room" << roomName << ", removed" << removeCount << "messages";
    }
}

void ChatManager::updateUnreadCount()
{
    int newTotal = 0;
    for (int count : m_unreadCounts) {
        newTotal += count;
    }
    
    if (m_totalUnreadCount != newTotal) {
        m_totalUnreadCount = newTotal;
        emit unreadCountChanged(m_totalUnreadCount);
    }
}

bool ChatManager::validateMessageContent(const QString& content) const
{
    if (content.isEmpty() || content.trimmed().isEmpty()) {
        return false;
    }
    
    if (content.length() > m_maxMessageLength) {
        return false;
    }
    
    return true;
}

QString ChatManager::sanitizeMessageContent(const QString& content) const
{
    QString sanitized = content.trimmed();
    
    // 移除多余的空白字符
    sanitized = sanitized.replace(QRegularExpression("\\s+"), " ");
    
    // 限制长度
    if (sanitized.length() > m_maxMessageLength) {
        sanitized = sanitized.left(m_maxMessageLength - 3) + "...";
    }
    
    return sanitized;
}

void ChatManager::loadConfiguration()
{
    m_settings->beginGroup("Configuration");
    
    m_maxHistorySize = m_settings->value("maxHistorySize", DEFAULT_MAX_HISTORY_SIZE).toInt();
    m_persistenceEnabled = m_settings->value("persistenceEnabled", true).toBool();
    m_autoSaveInterval = m_settings->value("autoSaveInterval", DEFAULT_AUTO_SAVE_INTERVAL).toInt();
    m_maxMessageLength = m_settings->value("maxMessageLength", DEFAULT_MAX_MESSAGE_LENGTH).toInt();
    m_historyRetentionDays = m_settings->value("historyRetentionDays", DEFAULT_RETENTION_DAYS).toInt();
    
    m_settings->endGroup();
    
    // 更新定时器间隔
    m_autoSaveTimer->setInterval(m_autoSaveInterval);
}