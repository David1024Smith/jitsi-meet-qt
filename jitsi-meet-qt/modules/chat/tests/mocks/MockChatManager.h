#ifndef MOCKCHATMANAGER_H
#define MOCKCHATMANAGER_H

#include "IChatManager.h"
#include <QObject>
#include <QVariantMap>
#include <QStringList>
#include <QDateTime>

/**
 * @brief Mock implementation of IChatManager for testing
 * 
 * MockChatManager provides a controllable implementation of the chat manager
 * interface for unit testing purposes.
 */
class MockChatManager : public QObject, public IChatManager
{
    Q_OBJECT
    Q_INTERFACES(IChatManager)

public:
    explicit MockChatManager(QObject *parent = nullptr);
    ~MockChatManager() override;

    // IChatManager interface implementation
    bool initialize(const QVariantMap& config = QVariantMap()) override;
    bool connectToService(const QString& serverUrl, const QVariantMap& credentials = QVariantMap()) override;
    void disconnect() override;
    bool isConnected() const override;
    ConnectionStatus connectionStatus() const override;
    bool joinRoom(const QString& roomId, const QString& password = QString()) override;
    void leaveRoom(const QString& roomId = QString()) override;
    QString currentRoom() const override;
    QStringList joinedRooms() const override;
    bool sendMessage(const QString& message, MessageType type = TextMessage, const QString& roomId = QString()) override;
    bool sendFile(const QString& filePath, const QString& roomId = QString()) override;
    QList<ChatMessage*> getMessageHistory(const QString& roomId, int limit = 50, const QDateTime& before = QDateTime()) override;
    QList<ChatMessage*> searchMessages(const QString& query, const QString& roomId = QString()) override;
    QList<Participant*> getParticipants(const QString& roomId = QString()) override;
    int participantCount(const QString& roomId = QString()) const override;
    bool isMessageHistoryEnabled() const override;
    void setMessageHistoryEnabled(bool enabled) override;
    void clearMessageHistory(const QString& roomId = QString(), const QDateTime& before = QDateTime()) override;
    QVariantMap getStatistics() const override;

public slots:
    void reconnect() override;
    void refreshParticipants(const QString& roomId = QString()) override;
    void markMessageAsRead(const QString& messageId) override;
    void markRoomAsRead(const QString& roomId = QString()) override;

public:
    // Mock control methods
    void setConnected(bool connected);
    void setConnectionStatus(ConnectionStatus status);
    void setCurrentRoom(const QString& roomId);
    void addJoinedRoom(const QString& roomId);
    void removeJoinedRoom(const QString& roomId);
    void setParticipantCount(int count);
    void setMessageHistoryEnabled(bool enabled);
    void addParticipant(Participant* participant);
    void removeParticipant(const QString& participantId);
    void addMessage(ChatMessage* message);
    void clearMessages();
    void simulateConnectionError(const QString& error);
    void simulateMessageReceived(ChatMessage* message);
    void simulateParticipantJoined(Participant* participant, const QString& roomId);
    void simulateParticipantLeft(const QString& participantId, const QString& roomId);

    // Mock verification methods
    bool wasInitializeCalled() const { return m_initializeCalled; }
    bool wasConnectCalled() const { return m_connectCalled; }
    bool wasDisconnectCalled() const { return m_disconnectCalled; }
    bool wasJoinRoomCalled() const { return m_joinRoomCalled; }
    bool wasLeaveRoomCalled() const { return m_leaveRoomCalled; }
    bool wasSendMessageCalled() const { return m_sendMessageCalled; }
    bool wasSendFileCalled() const { return m_sendFileCalled; }
    
    QString lastServerUrl() const { return m_lastServerUrl; }
    QString lastJoinedRoom() const { return m_lastJoinedRoom; }
    QString lastSentMessage() const { return m_lastSentMessage; }
    QString lastSentFile() const { return m_lastSentFile; }
    QVariantMap lastCredentials() const { return m_lastCredentials; }
    
    void resetCallFlags();

private:
    // Mock state
    bool m_connected;
    ConnectionStatus m_connectionStatus;
    QString m_currentRoom;
    QStringList m_joinedRooms;
    int m_participantCount;
    bool m_messageHistoryEnabled;
    QList<Participant*> m_participants;
    QList<ChatMessage*> m_messages;
    QVariantMap m_statistics;

    // Call tracking
    bool m_initializeCalled;
    bool m_connectCalled;
    bool m_disconnectCalled;
    bool m_joinRoomCalled;
    bool m_leaveRoomCalled;
    bool m_sendMessageCalled;
    bool m_sendFileCalled;
    
    QString m_lastServerUrl;
    QString m_lastJoinedRoom;
    QString m_lastSentMessage;
    QString m_lastSentFile;
    QVariantMap m_lastCredentials;
};

#endif // MOCKCHATMANAGER_H