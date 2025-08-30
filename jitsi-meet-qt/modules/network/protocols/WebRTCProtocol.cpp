#include "WebRTCProtocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDebug>
#include <QRegularExpression>

class WebRTCProtocol::Private
{
public:
    Private() 
        : status(IProtocolHandler::Inactive)
        , webRTCState(WebRTCProtocol::New)
        , iceState(WebRTCProtocol::ICENew)
        , heartbeatTimer(nullptr)
        , iceGatheringTimer(nullptr)
        , connectionTimer(nullptr)
        , sessionId("")
        , heartbeatInterval(30000)
        , iceGatheringTimeout(10000)
        , connectionTimeout(30000)
    {
    }

    IProtocolHandler::ProtocolStatus status;
    WebRTCProtocol::WebRTCState webRTCState;
    WebRTCProtocol::ICEConnectionState iceState;
    
    QTimer* heartbeatTimer;
    QTimer* iceGatheringTimer;
    QTimer* connectionTimer;
    
    QString sessionId;
    QStringList stunServers;
    QStringList turnServers;
    QVariantMap parameters;
    QVariantMap stats;
    QVariantList localIceCandidates;
    
    int heartbeatInterval;
    int iceGatheringTimeout;
    int connectionTimeout;
    
    QString localSDP;
    QString remoteSDP;
};

WebRTCProtocol::WebRTCProtocol(QObject *parent)
    : IProtocolHandler(parent)
    , d(new Private)
{
    d->heartbeatTimer = new QTimer(this);
    d->iceGatheringTimer = new QTimer(this);
    d->connectionTimer = new QTimer(this);
    
    connect(d->heartbeatTimer, &QTimer::timeout, this, &WebRTCProtocol::handleHeartbeatTimer);
    connect(d->iceGatheringTimer, &QTimer::timeout, this, &WebRTCProtocol::handleIceGatheringTimeout);
    connect(d->connectionTimer, &QTimer::timeout, this, &WebRTCProtocol::handleConnectionTimeout);
    
    d->heartbeatTimer->setSingleShot(false);
    d->iceGatheringTimer->setSingleShot(true);
    d->connectionTimer->setSingleShot(true);
}

WebRTCProtocol::~WebRTCProtocol()
{
    stop();
    delete d;
}

bool WebRTCProtocol::initialize(const QVariantMap& config)
{
    qDebug() << "WebRTCProtocol: Initializing with config:" << config;
    
    d->status = IProtocolHandler::Initializing;
    emit protocolStatusChanged(d->status);
    
    // 应用配置参数
    if (config.contains("stunServers")) {
        d->stunServers = config["stunServers"].toStringList();
    }
    if (config.contains("turnServers")) {
        d->turnServers = config["turnServers"].toStringList();
    }
    if (config.contains("heartbeatInterval")) {
        d->heartbeatInterval = config["heartbeatInterval"].toInt();
    }
    if (config.contains("iceGatheringTimeout")) {
        d->iceGatheringTimeout = config["iceGatheringTimeout"].toInt();
    }
    if (config.contains("connectionTimeout")) {
        d->connectionTimeout = config["connectionTimeout"].toInt();
    }
    
    // 初始化WebRTC引擎
    if (!initializeWebRTCEngine()) {
        d->status = IProtocolHandler::Error;
        emit protocolStatusChanged(d->status);
        emit protocolError("Failed to initialize WebRTC engine");
        return false;
    }
    
    d->status = IProtocolHandler::Active;
    emit protocolStatusChanged(d->status);
    
    qDebug() << "WebRTCProtocol: Initialization completed successfully";
    return true;
}

bool WebRTCProtocol::start()
{
    if (d->status != IProtocolHandler::Active) {
        qWarning() << "WebRTCProtocol: Cannot start - protocol not active";
        return false;
    }
    
    qDebug() << "WebRTCProtocol: Starting protocol";
    
    // 生成会话ID
    d->sessionId = generateSessionId();
    
    // 配置ICE服务器
    configureIceServers();
    
    // 启动心跳定时器
    d->heartbeatTimer->start(d->heartbeatInterval);
    
    updateWebRTCState(WebRTCProtocol::Connecting);
    
    emit protocolStarted();
    qDebug() << "WebRTCProtocol: Protocol started with session ID:" << d->sessionId;
    
    return true;
}

void WebRTCProtocol::stop()
{
    qDebug() << "WebRTCProtocol: Stopping protocol";
    
    // 停止所有定时器
    d->heartbeatTimer->stop();
    d->iceGatheringTimer->stop();
    d->connectionTimer->stop();
    
    // 更新状态
    updateWebRTCState(WebRTCProtocol::Closed);
    updateIceConnectionState(WebRTCProtocol::ICEClosed);
    
    // 清理WebRTC引擎
    cleanupWebRTCEngine();
    
    d->status = IProtocolHandler::Shutdown;
    emit protocolStatusChanged(d->status);
    emit protocolStopped();
    
    qDebug() << "WebRTCProtocol: Protocol stopped";
}

IProtocolHandler::ProtocolStatus WebRTCProtocol::protocolStatus() const
{
    return d->status;
}

QString WebRTCProtocol::protocolName() const
{
    return "WebRTC";
}

QString WebRTCProtocol::protocolVersion() const
{
    return "1.0";
}

QByteArray WebRTCProtocol::encodeMessage(MessageType type, const QVariantMap& data)
{
    QJsonObject message;
    message["type"] = static_cast<int>(type);
    message["sessionId"] = d->sessionId;
    message["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    QJsonObject payload;
    for (auto it = data.begin(); it != data.end(); ++it) {
        payload[it.key()] = QJsonValue::fromVariant(it.value());
    }
    message["payload"] = payload;
    
    QJsonDocument doc(message);
    return doc.toJson(QJsonDocument::Compact);
}

bool WebRTCProtocol::decodeMessage(const QByteArray& rawData, MessageType& type, QVariantMap& data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "WebRTCProtocol: JSON parse error:" << error.errorString();
        return false;
    }
    
    QJsonObject message = doc.object();
    
    if (!message.contains("type") || !message.contains("payload")) {
        qWarning() << "WebRTCProtocol: Invalid message format";
        return false;
    }
    
    type = static_cast<MessageType>(message["type"].toInt());
    
    QJsonObject payload = message["payload"].toObject();
    data.clear();
    for (auto it = payload.begin(); it != payload.end(); ++it) {
        data[it.key()] = it.value().toVariant();
    }
    
    return true;
}

bool WebRTCProtocol::handleReceivedData(const QByteArray& data)
{
    MessageType type;
    QVariantMap messageData;
    
    if (!decodeMessage(data, type, messageData)) {
        return false;
    }
    
    // 处理不同类型的消息
    switch (type) {
        case IProtocolHandler::Control:
            return handleSignalingMessage(messageData);
        case IProtocolHandler::Heartbeat:
            emit heartbeatReceived();
            return true;
        case IProtocolHandler::Data:
            emit messageReceived(type, messageData);
            return true;
        default:
            qWarning() << "WebRTCProtocol: Unknown message type:" << type;
            return false;
    }
}

bool WebRTCProtocol::sendMessage(MessageType type, const QVariantMap& data)
{
    if (d->status != IProtocolHandler::Active) {
        qWarning() << "WebRTCProtocol: Cannot send message - protocol not active";
        return false;
    }
    
    QByteArray encodedData = encodeMessage(type, data);
    
    // 这里应该通过实际的传输层发送数据
    // 目前只是模拟发送成功
    emit messageSent(type, data);
    
    // 更新统计信息
    d->stats["messagesSent"] = d->stats["messagesSent"].toInt() + 1;
    d->stats["bytesSent"] = d->stats["bytesSent"].toLongLong() + encodedData.size();
    
    return true;
}

bool WebRTCProtocol::sendHeartbeat()
{
    QVariantMap heartbeatData;
    heartbeatData["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    heartbeatData["sessionId"] = d->sessionId;
    
    bool result = sendMessage(IProtocolHandler::Heartbeat, heartbeatData);
    if (result) {
        emit heartbeatSent();
    }
    
    return result;
}bool WebR
TCProtocol::supportsFeature(const QString& feature) const
{
    QStringList features = supportedFeatures();
    return features.contains(feature);
}

QStringList WebRTCProtocol::supportedFeatures() const
{
    return QStringList() 
        << "signaling" 
        << "ice-gathering" 
        << "data-channels" 
        << "media-streams" 
        << "heartbeat"
        << "stun-servers"
        << "turn-servers";
}

void WebRTCProtocol::setParameter(const QString& key, const QVariant& value)
{
    d->parameters[key] = value;
    
    // 处理特殊参数
    if (key == "heartbeatInterval") {
        d->heartbeatInterval = value.toInt();
        if (d->heartbeatTimer->isActive()) {
            d->heartbeatTimer->setInterval(d->heartbeatInterval);
        }
    } else if (key == "iceGatheringTimeout") {
        d->iceGatheringTimeout = value.toInt();
    } else if (key == "connectionTimeout") {
        d->connectionTimeout = value.toInt();
    }
}

QVariant WebRTCProtocol::parameter(const QString& key) const
{
    return d->parameters.value(key);
}

QVariantMap WebRTCProtocol::protocolStats() const
{
    QVariantMap stats = d->stats;
    stats["sessionId"] = d->sessionId;
    stats["webRTCState"] = static_cast<int>(d->webRTCState);
    stats["iceState"] = static_cast<int>(d->iceState);
    stats["stunServers"] = d->stunServers;
    stats["turnServers"] = d->turnServers;
    stats["localIceCandidates"] = d->localIceCandidates.size();
    stats["uptime"] = QDateTime::currentMSecsSinceEpoch() - stats.value("startTime", 0).toLongLong();
    
    return stats;
}

WebRTCProtocol::WebRTCState WebRTCProtocol::webRTCState() const
{
    return d->webRTCState;
}

WebRTCProtocol::ICEConnectionState WebRTCProtocol::iceConnectionState() const
{
    return d->iceState;
}

void WebRTCProtocol::setStunServers(const QStringList& servers)
{
    d->stunServers = servers;
    configureIceServers();
}

QStringList WebRTCProtocol::stunServers() const
{
    return d->stunServers;
}

void WebRTCProtocol::setTurnServers(const QStringList& servers)
{
    d->turnServers = servers;
    configureIceServers();
}

QStringList WebRTCProtocol::turnServers() const
{
    return d->turnServers;
}

QString WebRTCProtocol::createOffer()
{
    qDebug() << "WebRTCProtocol: Creating offer";
    
    // 模拟SDP offer生成
    QString offer = QString(
        "v=0\r\n"
        "o=- %1 2 IN IP4 127.0.0.1\r\n"
        "s=-\r\n"
        "t=0 0\r\n"
        "a=group:BUNDLE 0 1\r\n"
        "a=msid-semantic: WMS\r\n"
        "m=audio 9 UDP/TLS/RTP/SAVPF 111\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "a=rtcp:9 IN IP4 0.0.0.0\r\n"
        "a=ice-ufrag:%2\r\n"
        "a=ice-pwd:%3\r\n"
        "a=fingerprint:sha-256 %4\r\n"
        "a=setup:actpass\r\n"
        "a=mid:0\r\n"
        "a=sendrecv\r\n"
        "m=video 9 UDP/TLS/RTP/SAVPF 96\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "a=rtcp:9 IN IP4 0.0.0.0\r\n"
        "a=ice-ufrag:%2\r\n"
        "a=ice-pwd:%3\r\n"
        "a=fingerprint:sha-256 %4\r\n"
        "a=setup:actpass\r\n"
        "a=mid:1\r\n"
        "a=sendrecv\r\n"
    ).arg(QDateTime::currentMSecsSinceEpoch())
     .arg(QUuid::createUuid().toString().mid(1, 8))
     .arg(QUuid::createUuid().toString().mid(1, 24))
     .arg(QUuid::createUuid().toString().remove('-'));
    
    d->localSDP = offer;
    emit localDescriptionGenerated(offer);
    
    return offer;
}

QString WebRTCProtocol::createAnswer(const QString& offer)
{
    qDebug() << "WebRTCProtocol: Creating answer for offer";
    
    if (!isValidSDP(offer)) {
        qWarning() << "WebRTCProtocol: Invalid offer SDP";
        return QString();
    }
    
    // 模拟SDP answer生成
    QString answer = QString(
        "v=0\r\n"
        "o=- %1 2 IN IP4 127.0.0.1\r\n"
        "s=-\r\n"
        "t=0 0\r\n"
        "a=group:BUNDLE 0 1\r\n"
        "a=msid-semantic: WMS\r\n"
        "m=audio 9 UDP/TLS/RTP/SAVPF 111\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "a=rtcp:9 IN IP4 0.0.0.0\r\n"
        "a=ice-ufrag:%2\r\n"
        "a=ice-pwd:%3\r\n"
        "a=fingerprint:sha-256 %4\r\n"
        "a=setup:active\r\n"
        "a=mid:0\r\n"
        "a=sendrecv\r\n"
        "m=video 9 UDP/TLS/RTP/SAVPF 96\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "a=rtcp:9 IN IP4 0.0.0.0\r\n"
        "a=ice-ufrag:%2\r\n"
        "a=ice-pwd:%3\r\n"
        "a=fingerprint:sha-256 %4\r\n"
        "a=setup:active\r\n"
        "a=mid:1\r\n"
        "a=sendrecv\r\n"
    ).arg(QDateTime::currentMSecsSinceEpoch())
     .arg(QUuid::createUuid().toString().mid(1, 8))
     .arg(QUuid::createUuid().toString().mid(1, 24))
     .arg(QUuid::createUuid().toString().remove('-'));
    
    d->localSDP = answer;
    emit localDescriptionGenerated(answer);
    
    return answer;
}

bool WebRTCProtocol::setRemoteDescription(const QString& sdp)
{
    if (!isValidSDP(sdp)) {
        qWarning() << "WebRTCProtocol: Invalid remote SDP";
        return false;
    }
    
    d->remoteSDP = sdp;
    emit remoteDescriptionReceived(sdp);
    
    qDebug() << "WebRTCProtocol: Remote description set successfully";
    return true;
}

bool WebRTCProtocol::setLocalDescription(const QString& sdp)
{
    if (!isValidSDP(sdp)) {
        qWarning() << "WebRTCProtocol: Invalid local SDP";
        return false;
    }
    
    d->localSDP = sdp;
    
    qDebug() << "WebRTCProtocol: Local description set successfully";
    return true;
}

bool WebRTCProtocol::addIceCandidate(const QVariantMap& candidate)
{
    if (!candidate.contains("candidate") || !candidate.contains("sdpMid")) {
        qWarning() << "WebRTCProtocol: Invalid ICE candidate format";
        return false;
    }
    
    // 这里应该添加到实际的WebRTC引擎
    qDebug() << "WebRTCProtocol: Added ICE candidate:" << candidate["candidate"].toString();
    
    return true;
}

QVariantList WebRTCProtocol::getLocalIceCandidates() const
{
    return d->localIceCandidates;
}

void WebRTCProtocol::reset()
{
    qDebug() << "WebRTCProtocol: Resetting protocol";
    
    stop();
    
    d->sessionId.clear();
    d->localSDP.clear();
    d->remoteSDP.clear();
    d->localIceCandidates.clear();
    d->stats.clear();
    d->parameters.clear();
    
    updateWebRTCState(WebRTCProtocol::New);
    updateIceConnectionState(WebRTCProtocol::ICENew);
    
    d->status = IProtocolHandler::Inactive;
    emit protocolStatusChanged(d->status);
}

void WebRTCProtocol::refresh()
{
    qDebug() << "WebRTCProtocol: Refreshing protocol";
    
    // 更新统计信息
    d->stats["lastRefresh"] = QDateTime::currentMSecsSinceEpoch();
    emit statsUpdated(protocolStats());
}

void WebRTCProtocol::startIceGathering()
{
    qDebug() << "WebRTCProtocol: Starting ICE gathering";
    
    updateIceConnectionState(WebRTCProtocol::ICEChecking);
    
    // 启动ICE收集超时定时器
    d->iceGatheringTimer->start(d->iceGatheringTimeout);
    
    // 模拟ICE候选生成
    QTimer::singleShot(1000, this, [this]() {
        QVariantMap candidate;
        candidate["candidate"] = "candidate:1 1 UDP 2130706431 192.168.1.100 54400 typ host";
        candidate["sdpMid"] = "0";
        candidate["sdpMLineIndex"] = 0;
        
        d->localIceCandidates.append(candidate);
        emit iceCandidateGenerated(candidate);
    });
    
    QTimer::singleShot(2000, this, [this]() {
        updateIceConnectionState(WebRTCProtocol::ICEConnected);
        d->iceGatheringTimer->stop();
    });
}

void WebRTCProtocol::stopIceGathering()
{
    qDebug() << "WebRTCProtocol: Stopping ICE gathering";
    
    d->iceGatheringTimer->stop();
    updateIceConnectionState(WebRTCProtocol::ICEClosed);
}

void WebRTCProtocol::handleHeartbeatTimer()
{
    sendHeartbeat();
}

void WebRTCProtocol::handleIceGatheringTimeout()
{
    qWarning() << "WebRTCProtocol: ICE gathering timeout";
    updateIceConnectionState(WebRTCProtocol::ICEFailed);
    emit protocolError("ICE gathering timeout");
}

void WebRTCProtocol::handleConnectionTimeout()
{
    qWarning() << "WebRTCProtocol: Connection timeout";
    updateWebRTCState(WebRTCProtocol::Failed);
    emit protocolError("Connection timeout");
}

bool WebRTCProtocol::initializeWebRTCEngine()
{
    qDebug() << "WebRTCProtocol: Initializing WebRTC engine";
    
    // 这里应该初始化实际的WebRTC引擎
    // 目前只是模拟初始化成功
    
    d->stats["startTime"] = QDateTime::currentMSecsSinceEpoch();
    d->stats["messagesSent"] = 0;
    d->stats["messagesReceived"] = 0;
    d->stats["bytesSent"] = 0;
    d->stats["bytesReceived"] = 0;
    
    return true;
}

void WebRTCProtocol::cleanupWebRTCEngine()
{
    qDebug() << "WebRTCProtocol: Cleaning up WebRTC engine";
    
    // 这里应该清理实际的WebRTC引擎资源
}

void WebRTCProtocol::configureIceServers()
{
    qDebug() << "WebRTCProtocol: Configuring ICE servers";
    qDebug() << "STUN servers:" << d->stunServers;
    qDebug() << "TURN servers:" << d->turnServers;
    
    // 这里应该配置实际的ICE服务器
}

bool WebRTCProtocol::handleSignalingMessage(const QVariantMap& message)
{
    QString messageType = message.value("messageType").toString();
    
    if (messageType == "offer") {
        QString sdp = message.value("sdp").toString();
        setRemoteDescription(sdp);
        return true;
    } else if (messageType == "answer") {
        QString sdp = message.value("sdp").toString();
        setRemoteDescription(sdp);
        return true;
    } else if (messageType == "ice-candidate") {
        QVariantMap candidate = message.value("candidate").toMap();
        return addIceCandidate(candidate);
    }
    
    qWarning() << "WebRTCProtocol: Unknown signaling message type:" << messageType;
    return false;
}

bool WebRTCProtocol::sendSignalingMessage(const QVariantMap& message)
{
    return sendMessage(IProtocolHandler::Control, message);
}

void WebRTCProtocol::updateWebRTCState(WebRTCState state)
{
    if (d->webRTCState != state) {
        d->webRTCState = state;
        emit webRTCStateChanged(state);
        
        qDebug() << "WebRTCProtocol: State changed to" << state;
    }
}

void WebRTCProtocol::updateIceConnectionState(ICEConnectionState state)
{
    if (d->iceState != state) {
        d->iceState = state;
        emit iceConnectionStateChanged(state);
        
        qDebug() << "WebRTCProtocol: ICE state changed to" << state;
    }
}

QString WebRTCProtocol::generateSessionId()
{
    return QUuid::createUuid().toString().remove('{').remove('}');
}

bool WebRTCProtocol::isValidSDP(const QString& sdp)
{
    if (sdp.isEmpty()) {
        return false;
    }
    
    // 基本的SDP格式验证
    QRegularExpression versionRegex("^v=0");
    if (!versionRegex.match(sdp).hasMatch()) {
        return false;
    }
    
    return true;
}

QVariantMap WebRTCProtocol::parseIceCandidate(const QString& candidateString)
{
    QVariantMap candidate;
    
    // 解析ICE候选字符串
    QRegularExpression regex(R"(candidate:(\d+)\s+(\d+)\s+(\w+)\s+(\d+)\s+([\d\.]+)\s+(\d+)\s+typ\s+(\w+))");
    QRegularExpressionMatch match = regex.match(candidateString);
    
    if (match.hasMatch()) {
        candidate["foundation"] = match.captured(1);
        candidate["component"] = match.captured(2).toInt();
        candidate["protocol"] = match.captured(3);
        candidate["priority"] = match.captured(4).toUInt();
        candidate["address"] = match.captured(5);
        candidate["port"] = match.captured(6).toInt();
        candidate["type"] = match.captured(7);
    }
    
    return candidate;
}