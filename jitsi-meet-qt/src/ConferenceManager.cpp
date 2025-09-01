#include "../include/ConferenceManager.h"
#include "../include/AuthenticationManager.h"
#include "../include/JitsiError.h"
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QDebug>

ConferenceManager::ConferenceManager(QObject *parent)
    : QObject(parent)
    , m_xmppClient(nullptr)
    , m_webrtcEngine(nullptr)
    , m_authManager(nullptr)
    , m_connectionState(Disconnected)
    , m_conferenceState(Idle)
    , m_reconnectTimer(new QTimer(this))
    , m_healthCheckTimer(new QTimer(this))
    , m_reconnectAttempts(0)
    , m_localAudioMuted(false)
    , m_localVideoMuted(false)
    , m_isScreenSharing(false)
{
    initializeComponents();
    
    // 设置重连定时器
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &ConferenceManager::onReconnectTimer);
    
    // 设置健康检查定时器
    m_healthCheckTimer->setInterval(HEALTH_CHECK_INTERVAL);
    connect(m_healthCheckTimer, &QTimer::timeout, this, &ConferenceManager::onConnectionHealthCheck);
    
    qDebug() << "ConferenceManager initialized";
}

ConferenceManager::~ConferenceManager()
{
    cleanup();
    qDebug() << "ConferenceManager destroyed";
}

void ConferenceManager::initializeComponents()
{
    // 创建XMPP客户端
    m_xmppClient = new XMPPClient(this);
    
    // 连接XMPP客户端信号
    connect(m_xmppClient, &XMPPClient::connectionStateChanged,
            this, &ConferenceManager::onXMPPConnectionStateChanged);
    connect(m_xmppClient, &XMPPClient::connected,
            this, &ConferenceManager::onXMPPConnected);
    connect(m_xmppClient, &XMPPClient::disconnected,
            this, &ConferenceManager::onXMPPDisconnected);
    connect(m_xmppClient, &XMPPClient::authenticated,
            this, &ConferenceManager::onXMPPAuthenticated);
    connect(m_xmppClient, &XMPPClient::roomJoined,
            this, &ConferenceManager::onXMPPRoomJoined);
    connect(m_xmppClient, &XMPPClient::roomLeft,
            this, &ConferenceManager::onXMPPRoomLeft);
    connect(m_xmppClient, &XMPPClient::participantJoined,
            this, &ConferenceManager::onXMPPParticipantJoined);
    connect(m_xmppClient, &XMPPClient::participantLeft,
            this, &ConferenceManager::onXMPPParticipantLeft);
    connect(m_xmppClient, &XMPPClient::participantUpdated,
            this, &ConferenceManager::onXMPPParticipantUpdated);
    connect(m_xmppClient, &XMPPClient::chatMessageReceived,
            this, &ConferenceManager::onXMPPChatMessageReceived);
    connect(m_xmppClient, &XMPPClient::errorOccurred,
            this, &ConferenceManager::onXMPPErrorOccurred);
    
    // 创建WebRTC引擎
    m_webrtcEngine = new WebRTCEngine(this);
    
    // 连接WebRTC引擎信号
    connect(m_webrtcEngine, &WebRTCEngine::connectionStateChanged,
            this, &ConferenceManager::onWebRTCConnectionStateChanged);
    connect(m_webrtcEngine, &WebRTCEngine::participantJoined,
            this, &ConferenceManager::onWebRTCParticipantJoined);
    connect(m_webrtcEngine, &WebRTCEngine::participantLeft,
            this, &ConferenceManager::onWebRTCParticipantLeft);
    connect(m_webrtcEngine, &WebRTCEngine::streamAdded,
            this, &ConferenceManager::onWebRTCStreamAdded);
    connect(m_webrtcEngine, &WebRTCEngine::streamRemoved,
            this, &ConferenceManager::onWebRTCStreamRemoved);
    connect(m_webrtcEngine, &WebRTCEngine::messageReceived,
            this, &ConferenceManager::onWebRTCMessageReceived);
    
    // 创建认证管理器
    m_authManager = new AuthenticationManager(this);
    
    // 连接认证管理器信号
    connect(m_authManager, &AuthenticationManager::authenticationSucceeded,
            this, &ConferenceManager::handleAuthentication);
    connect(m_authManager, &AuthenticationManager::authenticationFailed,
            this, [this](const QString& error) {
                emitError(::ErrorType::AuthenticationError, "Authentication failed", error);
            });
}

void ConferenceManager::cleanup()
{
    stopReconnection();
    
    if (m_healthCheckTimer->isActive()) {
        m_healthCheckTimer->stop();
    }
    
    if (m_xmppClient && m_xmppClient->isConnected()) {
        m_xmppClient->disconnect();
    }
    
    if (m_webrtcEngine) {
        m_webrtcEngine->disconnect();
    }
    
    m_participants.clear();
    setConnectionState(Disconnected);
    setConferenceState(Idle);
}

void ConferenceManager::joinConference(const QString& url, const QString& displayName)
{
    qDebug() << "Joining conference:" << url << "as" << displayName;
    
    if (m_conferenceState != Idle) {
        emitError(::ErrorType::InvalidUrl, "Already in conference or joining", 
                 "Current state: " + QString::number(m_conferenceState));
        return;
    }
    
    QString serverUrl, roomName;
    if (!parseConferenceUrl(url, serverUrl, roomName)) {
        emitError(::ErrorType::InvalidUrl, "Invalid conference URL", url);
        return;
    }
    
    // 设置会议信息
    m_currentConference.roomName = roomName;
    m_currentConference.serverUrl = serverUrl;
    m_currentConference.fullUrl = url;
    m_currentConference.displayName = displayName.isEmpty() ? "Anonymous" : displayName;
    m_currentConference.joinTime = QDateTime::currentDateTime();
    m_currentConference.participantCount = 0;
    m_currentConference.isLocked = false;
    m_currentConference.isRecording = false;
    
    // 设置本地参与者信息
    m_localParticipant.displayName = m_currentConference.displayName;
    m_localParticipant.joinTime = m_currentConference.joinTime;
    m_localParticipant.audioMuted = m_localAudioMuted;
    m_localParticipant.videoMuted = m_localVideoMuted;
    m_localParticipant.hasAudio = !m_localAudioMuted;
    m_localParticipant.hasVideo = !m_localVideoMuted;
    m_localParticipant.isScreenSharing = m_isScreenSharing;
    m_localParticipant.status = "joining";
    
    setConferenceState(Joining);
    setConnectionState(Connecting);
    
    // 开始认证流程
    m_authManager->authenticate(serverUrl, roomName, displayName);
}

void ConferenceManager::leaveConference()
{
    qDebug() << "Leaving conference";
    
    if (m_conferenceState == Idle) {
        qDebug() << "Not in conference, nothing to leave";
        return;
    }
    
    setConferenceState(Leaving);
    stopReconnection();
    
    // 离开XMPP房间
    if (m_xmppClient && m_xmppClient->isInRoom()) {
        m_xmppClient->leaveRoom();
    }
    
    // 关闭WebRTC连接
    if (m_webrtcEngine) {
        m_webrtcEngine->disconnect();
    }
    
    // 断开XMPP连接
    if (m_xmppClient && m_xmppClient->isConnected()) {
        m_xmppClient->disconnect();
    }
    
    // 清理状态
    m_participants.clear();
    m_isScreenSharing = false;
    m_screenSharingParticipant.clear();
    
    setConnectionState(Disconnected);
    setConferenceState(Idle);
    
    emit conferenceLeft();
    qDebug() << "Conference left successfully";
}

void ConferenceManager::reconnectToConference()
{
    qDebug() << "Reconnecting to conference";
    
    if (m_currentConference.roomName.isEmpty()) {
        emitError(::ErrorType::NetworkError, "No conference to reconnect to", "");
        return;
    }
    
    if (m_connectionState == Connecting || m_connectionState == Reconnecting) {
        qDebug() << "Already connecting/reconnecting";
        return;
    }
    
    setConnectionState(Reconnecting);
    emit reconnectionStarted(m_reconnectAttempts + 1);
    
    // 重新建立连接
    establishXMPPConnection();
}

void ConferenceManager::sendChatMessage(const QString& message)
{
    if (!m_xmppClient || !m_xmppClient->isInRoom()) {
        emitError(::ErrorType::NetworkError, "Not connected to conference", "Cannot send chat message");
        return;
    }
    
    m_xmppClient->sendChatMessage(message);
}

void ConferenceManager::setAudioMuted(bool muted)
{
    if (m_localAudioMuted == muted) {
        return;
    }
    
    m_localAudioMuted = muted;
    m_localParticipant.audioMuted = muted;
    m_localParticipant.hasAudio = !muted;
    
    if (m_xmppClient && m_xmppClient->isInRoom()) {
        m_xmppClient->setAudioMuted(muted);
    }
    
    updateLocalParticipant();
    emit localMediaStateChanged(m_localAudioMuted, m_localVideoMuted);
    
    qDebug() << "Audio muted:" << muted;
}

void ConferenceManager::setVideoMuted(bool muted)
{
    if (m_localVideoMuted == muted) {
        return;
    }
    
    m_localVideoMuted = muted;
    m_localParticipant.videoMuted = muted;
    m_localParticipant.hasVideo = !muted;
    
    if (m_xmppClient && m_xmppClient->isInRoom()) {
        m_xmppClient->setVideoMuted(muted);
    }
    
    updateLocalParticipant();
    emit localMediaStateChanged(m_localAudioMuted, m_localVideoMuted);
    
    qDebug() << "Video muted:" << muted;
}

void ConferenceManager::startScreenShare()
{
    if (m_isScreenSharing) {
        qDebug() << "Already screen sharing";
        return;
    }
    
    m_isScreenSharing = true;
    m_localParticipant.isScreenSharing = true;
    m_screenSharingParticipant = m_localParticipant.jid;
    
    updateLocalParticipant();
    emit screenShareStateChanged(true, m_localParticipant.jid);
    
    qDebug() << "Screen sharing started";
}

void ConferenceManager::stopScreenShare()
{
    if (!m_isScreenSharing) {
        qDebug() << "Not screen sharing";
        return;
    }
    
    m_isScreenSharing = false;
    m_localParticipant.isScreenSharing = false;
    
    if (m_screenSharingParticipant == m_localParticipant.jid) {
        m_screenSharingParticipant.clear();
    }
    
    updateLocalParticipant();
    emit screenShareStateChanged(false, m_localParticipant.jid);
    
    qDebug() << "Screen sharing stopped";
}

bool ConferenceManager::parseConferenceUrl(const QString& url, QString& serverUrl, QString& roomName)
{
    // 支持多种URL格式：
    // 1. https://meet.jit.si/RoomName
    // 2. meet.jit.si/RoomName
    // 3. jitsi-meet://meet.jit.si/RoomName
    // 4. RoomName (使用默认服务器)
    
    QString normalizedUrl = url.trimmed();
    
    // 处理协议前缀
    if (normalizedUrl.startsWith("jitsi-meet://")) {
        normalizedUrl = "https://" + normalizedUrl.mid(13);
    } else if (!normalizedUrl.startsWith("http://") && !normalizedUrl.startsWith("https://")) {
        if (normalizedUrl.contains("/")) {
            normalizedUrl = "https://" + normalizedUrl;
        } else {
            // 只有房间名，使用默认服务器
            serverUrl = "https://meet.jit.si";
            roomName = normalizedUrl;
            return !roomName.isEmpty();
        }
    }
    
    QUrl qurl(normalizedUrl);
    if (!qurl.isValid()) {
        return false;
    }
    
    serverUrl = qurl.scheme() + "://" + qurl.host();
    if (qurl.port() != -1) {
        serverUrl += ":" + QString::number(qurl.port());
    }
    
    QString path = qurl.path();
    if (path.startsWith("/")) {
        path = path.mid(1);
    }
    
    roomName = path;
    
    // 验证房间名
    if (roomName.isEmpty()) {
        return false;
    }
    
    // 房间名只能包含字母、数字、连字符和下划线
    QRegularExpression roomNameRegex("^[a-zA-Z0-9_-]+$");
    if (!roomNameRegex.match(roomName).hasMatch()) {
        return false;
    }
    
    return true;
}

void ConferenceManager::setConnectionState(ConnectionState state)
{
    if (m_connectionState == state) {
        return;
    }
    
    ConnectionState oldState = m_connectionState;
    m_connectionState = state;
    
    qDebug() << "Connection state changed:" << oldState << "->" << state;
    emit connectionStateChanged(state);
    
    // 根据连接状态启动或停止健康检查
    if (state == Connected) {
        m_healthCheckTimer->start();
        stopReconnection();
    } else {
        m_healthCheckTimer->stop();
    }
}

void ConferenceManager::setConferenceState(ConferenceState state)
{
    if (m_conferenceState == state) {
        return;
    }
    
    ConferenceState oldState = m_conferenceState;
    m_conferenceState = state;
    
    qDebug() << "Conference state changed:" << oldState << "->" << state;
    emit conferenceStateChanged(state);
}

void ConferenceManager::establishXMPPConnection()
{
    if (!m_xmppClient) {
        emitError(::ErrorType::NetworkError, "XMPP client not available", "");
        return;
    }
    
    qDebug() << "Establishing XMPP connection to" << m_currentConference.serverUrl 
             << "room" << m_currentConference.roomName;
    
    m_xmppClient->connectToServer(
        m_currentConference.serverUrl,
        m_currentConference.roomName,
        m_currentConference.displayName
    );
}

void ConferenceManager::establishWebRTCConnection()
{
    if (!m_webrtcEngine) {
        emitError(::ErrorType::WebRTCError, "WebRTC engine not available", "");
        return;
    }
    
    qDebug() << "Establishing WebRTC connection";
    // WebRTC 连接将在 connect 方法中建立
}

void ConferenceManager::handleAuthentication()
{
    qDebug() << "Authentication successful, establishing XMPP connection";
    establishXMPPConnection();
}

void ConferenceManager::synchronizeParticipants()
{
    if (!m_xmppClient) {
        return;
    }
    
    // 获取XMPP客户端的参与者列表并同步到本地
    QList<XMPPClient::Participant> xmppParticipants = m_xmppClient->participants();
    
    // 清理已离开的参与者
    QStringList currentJids;
    for (const auto& participant : xmppParticipants) {
        currentJids.append(participant.jid);
    }
    
    QStringList toRemove;
    for (auto it = m_participants.begin(); it != m_participants.end(); ++it) {
        if (!currentJids.contains(it.key())) {
            toRemove.append(it.key());
        }
    }
    
    for (const QString& jid : toRemove) {
        m_participants.remove(jid);
        emit participantLeft(jid);
    }
    
    // 更新或添加参与者
    for (const auto& xmppParticipant : xmppParticipants) {
        ParticipantInfo participant = ParticipantInfo::fromXMPPParticipant(xmppParticipant);
        
        if (m_participants.contains(participant.jid)) {
            // 更新现有参与者
            m_participants[participant.jid] = participant;
            emit participantUpdated(participant);
        } else {
            // 新参与者
            m_participants[participant.jid] = participant;
            emit participantJoined(participant);
        }
    }
    
    // 更新会议信息
    m_currentConference.participantCount = m_participants.size();
    updateConferenceInfo();
}

void ConferenceManager::updateConferenceInfo()
{
    emit conferenceInfoUpdated(m_currentConference);
}

void ConferenceManager::updateLocalParticipant()
{
    m_localParticipant.audioMuted = m_localAudioMuted;
    m_localParticipant.videoMuted = m_localVideoMuted;
    m_localParticipant.hasAudio = !m_localAudioMuted;
    m_localParticipant.hasVideo = !m_localVideoMuted;
    m_localParticipant.isScreenSharing = m_isScreenSharing;
    
    // 如果本地参与者在参与者列表中，更新它
    if (m_participants.contains(m_localParticipant.jid)) {
        m_participants[m_localParticipant.jid] = m_localParticipant;
        emit participantUpdated(m_localParticipant);
    }
}

void ConferenceManager::startReconnection()
{
    if (m_reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
        emitError(::ErrorType::NetworkError, "Maximum reconnection attempts reached", 
                 QString("Attempted %1 times").arg(m_reconnectAttempts));
        return;
    }
    
    m_reconnectAttempts++;
    setConnectionState(Reconnecting);
    
    qDebug() << "Starting reconnection attempt" << m_reconnectAttempts;
    
    m_reconnectTimer->start(RECONNECT_INTERVAL);
}

void ConferenceManager::stopReconnection()
{
    if (m_reconnectTimer->isActive()) {
        m_reconnectTimer->stop();
    }
    m_reconnectAttempts = 0;
}

void ConferenceManager::checkConnectionHealth()
{
    if (!m_xmppClient || !m_xmppClient->isConnected()) {
        qDebug() << "Connection health check failed: XMPP not connected";
        startReconnection();
        return;
    }
    
    // 可以添加更多健康检查逻辑，如ping测试等
    qDebug() << "Connection health check passed";
}

void ConferenceManager::emitError(::ErrorType type, const QString& message, const QString& details)
{
    m_lastError = message;
    JitsiError error(type, message, details);
    emit errorOccurred(error);
    
    qWarning() << "ConferenceManager error:" << message << details;
}

// ParticipantInfo静态方法实现
ConferenceManager::ParticipantInfo ConferenceManager::ParticipantInfo::fromXMPPParticipant(const XMPPClient::Participant& xmppParticipant)
{
    ParticipantInfo participant;
    participant.jid = xmppParticipant.jid;
    participant.displayName = xmppParticipant.displayName;
    participant.role = xmppParticipant.role;
    participant.audioMuted = xmppParticipant.audioMuted;
    participant.videoMuted = xmppParticipant.videoMuted;
    participant.status = xmppParticipant.status;
    participant.joinTime = xmppParticipant.joinTime;
    participant.hasAudio = !xmppParticipant.audioMuted;
    participant.hasVideo = !xmppParticipant.videoMuted;
    participant.isScreenSharing = false; // 需要从其他地方获取
    participant.connectionQuality = "good"; // 默认值
    
    return participant;
}

// XMPP客户端事件处理槽函数
void ConferenceManager::onXMPPConnectionStateChanged(XMPPClient::ConnectionState state)
{
    qDebug() << "XMPP connection state changed:" << state;
    
    switch (state) {
    case XMPPClient::Connected:
        setConnectionState(Connected);
        break;
    case XMPPClient::Connecting:
        setConnectionState(Connecting);
        break;
    case XMPPClient::Disconnected:
        setConnectionState(Disconnected);
        if (m_conferenceState == InConference) {
            startReconnection();
        }
        break;
    case XMPPClient::Error:
        setConnectionState(Failed);
        emitError(::ErrorType::XMPPConnectionError, "XMPP connection error", "");
        break;
    default:
        break;
    }
}

void ConferenceManager::onXMPPConnected()
{
    qDebug() << "XMPP connected";
    setConnectionState(Connected);
}

void ConferenceManager::onXMPPDisconnected()
{
    qDebug() << "XMPP disconnected";
    setConnectionState(Disconnected);
    
    if (m_conferenceState == InConference) {
        startReconnection();
    }
}

void ConferenceManager::onXMPPAuthenticated()
{
    qDebug() << "XMPP authenticated";
}

void ConferenceManager::onXMPPRoomJoined()
{
    qDebug() << "XMPP room joined";
    
    setConferenceState(InConference);
    m_localParticipant.jid = m_xmppClient->userJid();
    m_localParticipant.status = "connected";
    
    // 建立WebRTC连接
    establishWebRTCConnection();
    
    // 同步参与者列表
    synchronizeParticipants();
    
    emit conferenceJoined(m_currentConference);
    
    // 重置重连计数
    m_reconnectAttempts = 0;
    emit reconnectionSucceeded();
}

void ConferenceManager::onXMPPRoomLeft()
{
    qDebug() << "XMPP room left";
    
    if (m_conferenceState != Leaving) {
        // 意外离开房间，尝试重连
        startReconnection();
    }
}

void ConferenceManager::onXMPPParticipantJoined(const XMPPClient::Participant& participant)
{
    qDebug() << "XMPP participant joined:" << participant.jid << participant.displayName;
    
    ParticipantInfo participantInfo = ParticipantInfo::fromXMPPParticipant(participant);
    m_participants[participant.jid] = participantInfo;
    
    m_currentConference.participantCount = m_participants.size();
    updateConferenceInfo();
    
    emit participantJoined(participantInfo);
}

void ConferenceManager::onXMPPParticipantLeft(const QString& jid)
{
    qDebug() << "XMPP participant left:" << jid;
    
    if (m_participants.contains(jid)) {
        m_participants.remove(jid);
        m_currentConference.participantCount = m_participants.size();
        updateConferenceInfo();
        
        emit participantLeft(jid);
    }
}

void ConferenceManager::onXMPPParticipantUpdated(const XMPPClient::Participant& participant)
{
    qDebug() << "XMPP participant updated:" << participant.jid;
    
    ParticipantInfo participantInfo = ParticipantInfo::fromXMPPParticipant(participant);
    m_participants[participant.jid] = participantInfo;
    
    emit participantUpdated(participantInfo);
}

void ConferenceManager::onXMPPChatMessageReceived(const QString& from, const QString& message, const QDateTime& timestamp)
{
    qDebug() << "XMPP chat message received from" << from << ":" << message;
    emit chatMessageReceived(from, message, timestamp);
}

void ConferenceManager::onXMPPErrorOccurred(const QString& error)
{
    qWarning() << "XMPP error occurred:" << error;
    emitError(::ErrorType::XMPPConnectionError, "XMPP error", error);
}

// WebRTC引擎事件处理槽函数
void ConferenceManager::onWebRTCConnectionStateChanged(WebRTCEngine::ConnectionState state)
{
    qDebug() << "WebRTC connection state changed:" << state;
    
    switch (state) {
    case WebRTCEngine::Connected:
        qDebug() << "WebRTC connection established";
        break;
    case WebRTCEngine::Failed:
        emitError(::ErrorType::WebRTCError, "WebRTC connection failed", "");
        break;
    default:
        break;
    }
}

// 旧的 WebRTC 方法已被删除，使用新的方法实现

void ConferenceManager::handleMediaStreamEvent(const QString& participantJid, bool hasVideo, bool hasAudio)
{
    if (m_participants.contains(participantJid)) {
        ParticipantInfo& participant = m_participants[participantJid];
        participant.hasVideo = hasVideo;
        participant.hasAudio = hasAudio;
        
        emit participantUpdated(participant);
    }
}

// 定时器事件处理槽函数
void ConferenceManager::onReconnectTimer()
{
    qDebug() << "Reconnect timer triggered, attempt" << m_reconnectAttempts;
    
    if (m_reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
        emit reconnectionFailed("Maximum reconnection attempts reached");
        setConnectionState(Failed);
        return;
    }
    
    // 尝试重新连接
    establishXMPPConnection();
}

void ConferenceManager::onConnectionHealthCheck()
{
    checkConnectionHealth();
}

// New WebRTC event handling methods
void ConferenceManager::onWebRTCParticipantJoined(const QString& participantId, const QVariantMap& info)
{
    qDebug() << "WebRTC participant joined:" << participantId;
    // These methods will replace old WebRTC methods
    Q_UNUSED(info)
}

void ConferenceManager::onWebRTCParticipantLeft(const QString& participantId)
{
    qDebug() << "WebRTC participant left:" << participantId;
}

void ConferenceManager::onWebRTCStreamAdded(const QString& streamId, const WebRTCEngine::MediaStreamInfo& info)
{
    qDebug() << "WebRTC stream added:" << streamId << "from participant:" << info.participantId;
}

void ConferenceManager::onWebRTCStreamRemoved(const QString& streamId)
{
    qDebug() << "WebRTC stream removed:" << streamId;
}

void ConferenceManager::onWebRTCMessageReceived(const QString& message, const QString& from)
{
    qDebug() << "WebRTC message received from" << from << ":" << message;
}