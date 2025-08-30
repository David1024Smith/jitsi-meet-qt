#include "MockChatManager.h"
#include "ChatMessage.h"
#include "Participant.h"
#include <QTimer>

MockChatManager::MockChatManager(QObject *parent)
    : QObject(parent)
    , m_connected(false)
    , m_connectionStatus(Disconnected)
    , m_participantCount(0)
    , m_messageHistoryEnabled(true)
    , m_initializeCalled(false)
    , m_connectCalled(false)
    , m_disconnectCalled(false)
    , m_joinRoomCalled(false)
    , m_leaveRoomCalled(false)
    , m_sendMessageCalled(false)
    , m_sendFileCalled(false)
{
    // Initialize mock statistics
    m_statistics["messagesReceived"] = 0;
    m_statistics["messagesSent"] = 0;
    m_statistics["connectionsCount"] = 0;
    m_statistics["uptime"] = 0;
}

MockChatManager::~MockChatManager()
{
    qDeleteAll(m_participants);
    qDeleteAll(m_messages);
}

bool MockChatManager::initialize(const QVariantMap& config)
{
    Q_UNUSED(config)
    m_initializeCalled = true;
    return true;
}

bool MockChatManager::connectToService(const QString& serverUrl, const QVariantMap& credentials)
{
    m_connectCalled = true;
    m_lastServerUrl = serverUrl;
    m_lastCredentials = credentials;
    
    // Simulate connection delay
    QTimer::singleShot(100, this, [this]() {
        setConnected(true);
        setConnectionStatus(Connected);
    });
    
    return true;
}

void MockChatManager::disconnect()
{
    m_disconnectCalled = true;
    setConnected(false);
    setConnectionStatus(Disconnected);
    m_currentRoom.clear();
    m_joinedRooms.clear();
}

bool MockChatManager::isConnected() const
{
    return m_connected;
}

IChatManager::ConnectionStatus MockChatManager::connectionStatus() const
{
    return m_connectionStatus;
}

bool MockChatManager::joinRoom(const QString& roomId, const QString& password)
{
    Q_UNUSED(password)
    
    if (!m_connected) {
        return false;
    }
    
    m_joinRoomCalled = true;
    m_lastJoinedRoom = roomId;
    
    if (!m_joinedRooms.contains(roomId)) {
        m_joinedRooms.append(roomId);
    }
    
    setCurrentRoom(roomId);
    
    // Simulate room join delay
    QTimer::singleShot(50, this, [this, roomId]() {
        emit roomJoined(roomId);
    });
    
    return true;
}

void MockChatManager::leaveRoom(const QString& roomId)
{
    m_leaveRoomCalled = true;
    
    QString targetRoom = roomId.isEmpty() ? m_currentRoom : roomId;
    
    if (m_joinedRooms.contains(targetRoom)) {
        m_joinedRooms.removeAll(targetRoom);
        
        if (m_currentRoom == targetRoom) {
            m_currentRoom.clear();
        }
        
        emit roomLeft(targetRoom);
    }
}

QString MockChatManager::currentRoom() const
{
    return m_currentRoom;
}

QStringList MockChatManager::joinedRooms() const
{
    return m_joinedRooms;
}

bool MockChatManager::sendMessage(const QString& message, MessageType type, const QString& roomId)
{
    Q_UNUSED(type)
    Q_UNUSED(roomId)
    
    if (!m_connected || message.isEmpty()) {
        return false;
    }
    
    m_sendMessageCalled = true;
    m_lastSentMessage = message;
    
    // Update statistics
    m_statistics["messagesSent"] = m_statistics["messagesSent"].toInt() + 1;
    
    // Simulate message sending delay
    QTimer::singleShot(50, this, [this]() {
        emit messageSent("mock_message_id");
    });
    
    return true;
}

bool MockChatManager::sendFile(const QString& filePath, const QString& roomId)
{
    Q_UNUSED(roomId)
    
    if (!m_connected || filePath.isEmpty()) {
        return false;
    }
    
    m_sendFileCalled = true;
    m_lastSentFile = filePath;
    
    // Simulate file sending delay
    QTimer::singleShot(100, this, [this]() {
        emit messageSent("mock_file_message_id");
    });
    
    return true;
}

QList<ChatMessage*> MockChatManager::getMessageHistory(const QString& roomId, int limit, const QDateTime& before)
{
    Q_UNUSED(before)
    
    QList<ChatMessage*> result;
    
    for (ChatMessage* message : m_messages) {
        if (roomId.isEmpty() || message->roomId() == roomId) {
            result.append(message);
            if (result.size() >= limit) {
                break;
            }
        }
    }
    
    return result;
}

QList<ChatMessage*> MockChatManager::searchMessages(const QString& query, const QString& roomId)
{
    QList<ChatMessage*> result;
    
    for (ChatMessage* message : m_messages) {
        if ((roomId.isEmpty() || message->roomId() == roomId) &&
            message->content().contains(query, Qt::CaseInsensitive)) {
            result.append(message);
        }
    }
    
    return result;
}

QList<Participant*> MockChatManager::getParticipants(const QString& roomId)
{
    Q_UNUSED(roomId)
    return m_participants;
}

int MockChatManager::participantCount(const QString& roomId) const
{
    Q_UNUSED(roomId)
    return m_participantCount;
}

bool MockChatManager::isMessageHistoryEnabled() const
{
    return m_messageHistoryEnabled;
}

void MockChatManager::setMessageHistoryEnabled(bool enabled)
{
    if (m_messageHistoryEnabled != enabled) {
        m_messageHistoryEnabled = enabled;
        emit messageHistoryEnabledChanged(enabled);
    }
}

void MockChatManager::clearMessageHistory(const QString& roomId, const QDateTime& before)
{
    Q_UNUSED(before)
    
    if (roomId.isEmpty()) {
        qDeleteAll(m_messages);
        m_messages.clear();
    } else {
        auto it = m_messages.begin();
        while (it != m_messages.end()) {
            if ((*it)->roomId() == roomId) {
                delete *it;
                it = m_messages.erase(it);
            } else {
                ++it;
            }
        }
    }
}

QVariantMap MockChatManager::getStatistics() const
{
    return m_statistics;
}

void MockChatManager::reconnect()
{
    if (m_connected) {
        disconnect();
    }
    
    QTimer::singleShot(200, this, [this]() {
        setConnected(true);
        setConnectionStatus(Connected);
    });
}

void MockChatManager::refreshParticipants(const QString& roomId)
{
    Q_UNUSED(roomId)
    // Simulate participant refresh
    emit participantCountChanged(m_participantCount);
}

void MockChatManager::markMessageAsRead(const QString& messageId)
{
    for (ChatMessage* message : m_messages) {
        if (message->id() == messageId) {
            message->setRead(true);
            break;
        }
    }
}

void MockChatManager::markRoomAsRead(const QString& roomId)
{
    QString targetRoom = roomId.isEmpty() ? m_currentRoom : roomId;
    
    for (ChatMessage* message : m_messages) {
        if (message->roomId() == targetRoom) {
            message->setRead(true);
        }
    }
}

// Mock control methods
void MockChatManager::setConnected(bool connected)
{
    if (m_connected != connected) {
        m_connected = connected;
        emit connectionChanged(connected);
    }
}

void MockChatManager::setConnectionStatus(ConnectionStatus status)
{
    if (m_connectionStatus != status) {
        m_connectionStatus = status;
        emit connectionStatusChanged(status);
    }
}

void MockChatManager::setCurrentRoom(const QString& roomId)
{
    if (m_currentRoom != roomId) {
        m_currentRoom = roomId;
        emit currentRoomChanged(roomId);
    }
}

void MockChatManager::addJoinedRoom(const QString& roomId)
{
    if (!m_joinedRooms.contains(roomId)) {
        m_joinedRooms.append(roomId);
    }
}

void MockChatManager::removeJoinedRoom(const QString& roomId)
{
    m_joinedRooms.removeAll(roomId);
}

void MockChatManager::setParticipantCount(int count)
{
    if (m_participantCount != count) {
        m_participantCount = count;
        emit participantCountChanged(count);
    }
}

void MockChatManager::addParticipant(Participant* participant)
{
    if (participant && !m_participants.contains(participant)) {
        m_participants.append(participant);
        setParticipantCount(m_participants.size());
    }
}

void MockChatManager::removeParticipant(const QString& participantId)
{
    auto it = m_participants.begin();
    while (it != m_participants.end()) {
        if ((*it)->id() == participantId) {
            delete *it;
            it = m_participants.erase(it);
            setParticipantCount(m_participants.size());
            emit participantLeft(participantId, m_currentRoom);
            break;
        } else {
            ++it;
        }
    }
}

void MockChatManager::addMessage(ChatMessage* message)
{
    if (message) {
        m_messages.append(message);
        m_statistics["messagesReceived"] = m_statistics["messagesReceived"].toInt() + 1;
    }
}

void MockChatManager::clearMessages()
{
    qDeleteAll(m_messages);
    m_messages.clear();
}

void MockChatManager::simulateConnectionError(const QString& error)
{
    setConnectionStatus(Error);
    emit errorOccurred(error);
}

void MockChatManager::simulateMessageReceived(ChatMessage* message)
{
    if (message) {
        addMessage(message);
        emit messageReceived(message);
    }
}

void MockChatManager::simulateParticipantJoined(Participant* participant, const QString& roomId)
{
    if (participant) {
        addParticipant(participant);
        emit participantJoined(participant, roomId);
    }
}

void MockChatManager::simulateParticipantLeft(const QString& participantId, const QString& roomId)
{
    removeParticipant(participantId);
    emit participantLeft(participantId, roomId);
}

void MockChatManager::resetCallFlags()
{
    m_initializeCalled = false;
    m_connectCalled = false;
    m_disconnectCalled = false;
    m_joinRoomCalled = false;
    m_leaveRoomCalled = false;
    m_sendMessageCalled = false;
    m_sendFileCalled = false;
    
    m_lastServerUrl.clear();
    m_lastJoinedRoom.clear();
    m_lastSentMessage.clear();
    m_lastSentFile.clear();
    m_lastCredentials.clear();
}