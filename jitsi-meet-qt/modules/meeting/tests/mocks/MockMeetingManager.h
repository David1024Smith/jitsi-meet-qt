#ifndef MOCKMEETINGMANAGER_H
#define MOCKMEETINGMANAGER_H

#include "../../interfaces/IMeetingManager.h"
#include <QObject>
#include <QVariantMap>
#include <QTimer>

/**
 * @brief Mock implementation of IMeetingManager for testing
 * 
 * Provides a controllable mock implementation that can simulate
 * various meeting scenarios and conditions for testing purposes.
 */
class MockMeetingManager : public IMeetingManager
{
    Q_OBJECT

public:
    explicit MockMeetingManager(QObject* parent = nullptr);
    ~MockMeetingManager();

    // IMeetingManager interface implementation
    bool initialize() override;
    MeetingState currentState() const override;
    bool createMeeting(const QString& meetingName, const QVariantMap& settings = QVariantMap()) override;
    bool joinMeeting(const QString& meetingUrl, 
                    const QString& displayName = QString(),
                    bool audioEnabled = true, 
                    bool videoEnabled = true) override;
    bool leaveMeeting() override;
    bool validateMeetingUrl(const QString& meetingUrl) override;
    QVariantMap getCurrentMeetingInfo() const override;
    void setConfiguration(const QVariantMap& config) override;
    QVariantMap getConfiguration() const override;
    QVariantList getParticipants() const override;
    bool inviteParticipant(const QString& email, const QString& message = QString()) override;

    // Mock-specific methods for test control
    void setMockState(MeetingState state);
    void setMockNetworkAvailable(bool available);
    void setMockServerReachable(bool reachable);
    void setMockAuthenticationRequired(bool required);
    void setMockError(const QString& error);
    void setMockDelay(int milliseconds);
    
    void simulateParticipantJoined(const QVariantMap& participant);
    void simulateParticipantLeft(const QString& participantId);
    void simulateConnectionQualityChange(int quality);
    void simulateNetworkError();
    void simulateServerError();
    void simulateAuthenticationFailure();
    
    // Test verification methods
    int getCreateMeetingCallCount() const;
    int getJoinMeetingCallCount() const;
    int getLeaveMeetingCallCount() const;
    QString getLastMeetingName() const;
    QString getLastMeetingUrl() const;
    QString getLastDisplayName() const;
    QVariantMap getLastSettings() const;
    
    void resetCallCounts();
    void clearHistory();

private slots:
    void onDelayTimer();
    void onStateTransitionTimer();

private:
    void setState(MeetingState newState);
    void emitDelayedSignal(const QString& signalName, const QVariantList& arguments = QVariantList());
    QString generateMockMeetingUrl(const QString& meetingName) const;
    QVariantMap generateMockMeetingInfo(const QString& meetingName) const;

private:
    MeetingState m_currentState;
    QVariantMap m_configuration;
    QVariantMap m_currentMeetingInfo;
    QVariantList m_participants;
    
    // Mock control settings
    bool m_networkAvailable;
    bool m_serverReachable;
    bool m_authenticationRequired;
    QString m_mockError;
    int m_mockDelay;
    bool m_initialized;
    
    // Call tracking
    int m_createMeetingCallCount;
    int m_joinMeetingCallCount;
    int m_leaveMeetingCallCount;
    QString m_lastMeetingName;
    QString m_lastMeetingUrl;
    QString m_lastDisplayName;
    QVariantMap m_lastSettings;
    
    // Timers for delayed operations
    QTimer* m_delayTimer;
    QTimer* m_stateTransitionTimer;
    
    // Pending operations
    struct PendingOperation {
        QString type;
        QVariantList arguments;
        QString signalName;
    };
    QList<PendingOperation> m_pendingOperations;
};

#endif // MOCKMEETINGMANAGER_H