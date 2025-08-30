#include "MockMeetingManager.h"
#include <QDateTime>
#include <QUrl>
#include <QDebug>

MockMeetingManager::MockMeetingManager(QObject* parent)
    : IMeetingManager(parent)
    , m_currentState(Disconnected)
    , m_networkAvailable(true)
    , m_serverReachable(true)
    , m_authenticationRequired(false)
    , m_mockDelay(0)
    , m_initialized(false)
    , m_createMeetingCallCount(0)
    , m_joinMeetingCallCount(0)
    , m_leaveMeetingCallCount(0)
{
    m_delayTimer = new QTimer(this);
    m_delayTimer->setSingleShot(true);
    connect(m_delayTimer, &QTimer::timeout, this, &MockMeetingManager::onDelayTimer);
    
    m_stateTransitionTimer = new QTimer(this);
    m_stateTransitionTimer->setSingleShot(true);
    connect(m_stateTransitionTimer, &QTimer::timeout, this, &MockMeetingManager::onStateTransitionTimer);
    
    // Set default configuration
    m_configuration["server"] = "mock.jitsi.server.com";
    m_configuration["timeout"] = 10000;
    m_configuration["audioEnabled"] = true;
    m_configuration["videoEnabled"] = true;
}

MockMeetingManager::~MockMeetingManager()
{
    // Cleanup is handled by Qt parent-child relationship
}

bool MockMeetingManager::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    if (!m_networkAvailable) {
        m_mockError = "Network not available";
        return false;
    }
    
    m_initialized = true;
    setState(Disconnected);
    
    qDebug() << "MockMeetingManager initialized";
    return true;
}

IMeetingManager::MeetingState MockMeetingManager::currentState() const
{
    return m_currentState;
}

bool MockMeetingManager::createMeeting(const QString& meetingName, const QVariantMap& settings)
{
    m_createMeetingCallCount++;
    m_lastMeetingName = meetingName;
    m_lastSettings = settings;
    
    if (!m_initialized) {
        m_mockError = "Manager not initialized";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    if (meetingName.isEmpty()) {
        m_mockError = "Meeting name cannot be empty";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    if (!m_networkAvailable) {
        m_mockError = "Network connection not available";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    if (!m_serverReachable) {
        m_mockError = "Server not reachable";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    // Simulate state transition
    setState(Connecting);
    
    // Generate mock meeting info
    QString meetingUrl = generateMockMeetingUrl(meetingName);
    QVariantMap meetingInfo = generateMockMeetingInfo(meetingName);
    m_currentMeetingInfo = meetingInfo;
    
    // Simulate async operation
    if (m_mockDelay > 0) {
        m_pendingOperations.append({
            "createMeeting",
            {meetingUrl, meetingInfo},
            "meetingCreated"
        });
        m_delayTimer->start(m_mockDelay);
    } else {
        setState(Connected);
        emit meetingCreated(meetingUrl, meetingInfo);
    }
    
    return true;
}

bool MockMeetingManager::joinMeeting(const QString& meetingUrl, 
                                   const QString& displayName,
                                   bool audioEnabled, 
                                   bool videoEnabled)
{
    m_joinMeetingCallCount++;
    m_lastMeetingUrl = meetingUrl;
    m_lastDisplayName = displayName;
    
    if (!m_initialized) {
        m_mockError = "Manager not initialized";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    if (!validateMeetingUrl(meetingUrl)) {
        m_mockError = "Invalid meeting URL";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    if (!m_networkAvailable) {
        m_mockError = "Network connection not available";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    if (m_authenticationRequired) {
        m_mockError = "Authentication required";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    // Simulate state transition
    setState(Connecting);
    
    // Generate mock meeting info
    QVariantMap meetingInfo;
    meetingInfo["url"] = meetingUrl;
    meetingInfo["displayName"] = displayName;
    meetingInfo["audioEnabled"] = audioEnabled;
    meetingInfo["videoEnabled"] = videoEnabled;
    meetingInfo["joinedAt"] = QDateTime::currentDateTime();
    meetingInfo["participants"] = m_participants;
    
    m_currentMeetingInfo = meetingInfo;
    
    // Simulate async operation
    if (m_mockDelay > 0) {
        m_pendingOperations.append({
            "joinMeeting",
            {meetingInfo},
            "meetingJoined"
        });
        m_delayTimer->start(m_mockDelay);
    } else {
        setState(InMeeting);
        emit meetingJoined(meetingInfo);
    }
    
    return true;
}

bool MockMeetingManager::leaveMeeting()
{
    m_leaveMeetingCallCount++;
    
    if (m_currentState != InMeeting && m_currentState != Connected) {
        m_mockError = "Not in a meeting";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    // Simulate state transition
    setState(Leaving);
    
    // Clear meeting info
    m_currentMeetingInfo.clear();
    m_participants.clear();
    
    // Simulate async operation
    if (m_mockDelay > 0) {
        m_pendingOperations.append({
            "leaveMeeting",
            {},
            "meetingLeft"
        });
        m_delayTimer->start(m_mockDelay);
    } else {
        setState(Disconnected);
        emit meetingLeft();
    }
    
    return true;
}

bool MockMeetingManager::validateMeetingUrl(const QString& meetingUrl)
{
    if (meetingUrl.isEmpty()) {
        return false;
    }
    
    QUrl url(meetingUrl);
    if (!url.isValid()) {
        return false;
    }
    
    if (url.scheme() != "https" && url.scheme() != "jitsi") {
        return false;
    }
    
    if (url.host().isEmpty() || url.path().isEmpty()) {
        return false;
    }
    
    return true;
}

QVariantMap MockMeetingManager::getCurrentMeetingInfo() const
{
    return m_currentMeetingInfo;
}

void MockMeetingManager::setConfiguration(const QVariantMap& config)
{
    m_configuration = config;
}

QVariantMap MockMeetingManager::getConfiguration() const
{
    return m_configuration;
}

QVariantList MockMeetingManager::getParticipants() const
{
    return m_participants;
}

bool MockMeetingManager::inviteParticipant(const QString& email, const QString& message)
{
    Q_UNUSED(message)
    
    if (email.isEmpty() || !email.contains("@")) {
        m_mockError = "Invalid email address";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    if (m_currentState != InMeeting && m_currentState != Connected) {
        m_mockError = "Not in a meeting";
        emitDelayedSignal("errorOccurred", {m_mockError});
        return false;
    }
    
    // Simulate successful invitation
    return true;
}

// Mock control methods
void MockMeetingManager::setMockState(MeetingState state)
{
    setState(state);
}

void MockMeetingManager::setMockNetworkAvailable(bool available)
{
    m_networkAvailable = available;
}

void MockMeetingManager::setMockServerReachable(bool reachable)
{
    m_serverReachable = reachable;
}

void MockMeetingManager::setMockAuthenticationRequired(bool required)
{
    m_authenticationRequired = required;
}

void MockMeetingManager::setMockError(const QString& error)
{
    m_mockError = error;
}

void MockMeetingManager::setMockDelay(int milliseconds)
{
    m_mockDelay = milliseconds;
}

void MockMeetingManager::simulateParticipantJoined(const QVariantMap& participant)
{
    m_participants.append(participant);
    emit participantJoined(participant);
}

void MockMeetingManager::simulateParticipantLeft(const QString& participantId)
{
    // Remove participant from list
    for (int i = 0; i < m_participants.size(); ++i) {
        QVariantMap participant = m_participants[i].toMap();
        if (participant["id"].toString() == participantId) {
            m_participants.removeAt(i);
            break;
        }
    }
    
    emit participantLeft(participantId);
}

void MockMeetingManager::simulateConnectionQualityChange(int quality)
{
    emit connectionQualityChanged(quality);
}

void MockMeetingManager::simulateNetworkError()
{
    m_networkAvailable = false;
    setState(Error);
    emit errorOccurred("Network connection lost");
}

void MockMeetingManager::simulateServerError()
{
    setState(Error);
    emit errorOccurred("Server error occurred");
}

void MockMeetingManager::simulateAuthenticationFailure()
{
    setState(Error);
    emit errorOccurred("Authentication failed");
}

// Test verification methods
int MockMeetingManager::getCreateMeetingCallCount() const
{
    return m_createMeetingCallCount;
}

int MockMeetingManager::getJoinMeetingCallCount() const
{
    return m_joinMeetingCallCount;
}

int MockMeetingManager::getLeaveMeetingCallCount() const
{
    return m_leaveMeetingCallCount;
}

QString MockMeetingManager::getLastMeetingName() const
{
    return m_lastMeetingName;
}

QString MockMeetingManager::getLastMeetingUrl() const
{
    return m_lastMeetingUrl;
}

QString MockMeetingManager::getLastDisplayName() const
{
    return m_lastDisplayName;
}

QVariantMap MockMeetingManager::getLastSettings() const
{
    return m_lastSettings;
}

void MockMeetingManager::resetCallCounts()
{
    m_createMeetingCallCount = 0;
    m_joinMeetingCallCount = 0;
    m_leaveMeetingCallCount = 0;
}

void MockMeetingManager::clearHistory()
{
    resetCallCounts();
    m_lastMeetingName.clear();
    m_lastMeetingUrl.clear();
    m_lastDisplayName.clear();
    m_lastSettings.clear();
    m_currentMeetingInfo.clear();
    m_participants.clear();
    m_pendingOperations.clear();
}

// Private slots
void MockMeetingManager::onDelayTimer()
{
    if (!m_pendingOperations.isEmpty()) {
        PendingOperation op = m_pendingOperations.takeFirst();
        
        if (op.type == "createMeeting") {
            setState(Connected);
            emit meetingCreated(op.arguments[0].toString(), op.arguments[1].toMap());
        } else if (op.type == "joinMeeting") {
            setState(InMeeting);
            emit meetingJoined(op.arguments[0].toMap());
        } else if (op.type == "leaveMeeting") {
            setState(Disconnected);
            emit meetingLeft();
        }
    }
}

void MockMeetingManager::onStateTransitionTimer()
{
    // Handle automatic state transitions if needed
}

// Private methods
void MockMeetingManager::setState(MeetingState newState)
{
    if (m_currentState != newState) {
        m_currentState = newState;
        emit stateChanged(newState);
    }
}

void MockMeetingManager::emitDelayedSignal(const QString& signalName, const QVariantList& arguments)
{
    if (m_mockDelay > 0) {
        QTimer::singleShot(m_mockDelay, [this, signalName, arguments]() {
            if (signalName == "errorOccurred" && !arguments.isEmpty()) {
                emit errorOccurred(arguments[0].toString());
            }
        });
    } else {
        if (signalName == "errorOccurred" && !arguments.isEmpty()) {
            emit errorOccurred(arguments[0].toString());
        }
    }
}

QString MockMeetingManager::generateMockMeetingUrl(const QString& meetingName) const
{
    QString server = m_configuration["server"].toString();
    if (server.isEmpty()) {
        server = "mock.jitsi.server.com";
    }
    
    QString roomName = meetingName.toLower().replace(" ", "-");
    return QString("https://%1/%2").arg(server, roomName);
}

QVariantMap MockMeetingManager::generateMockMeetingInfo(const QString& meetingName) const
{
    QVariantMap info;
    info["id"] = QString("mock-meeting-%1").arg(QDateTime::currentMSecsSinceEpoch());
    info["name"] = meetingName;
    info["url"] = generateMockMeetingUrl(meetingName);
    info["createdAt"] = QDateTime::currentDateTime();
    info["participants"] = m_participants;
    info["audioEnabled"] = m_configuration["audioEnabled"];
    info["videoEnabled"] = m_configuration["videoEnabled"];
    info["server"] = m_configuration["server"];
    
    return info;
}