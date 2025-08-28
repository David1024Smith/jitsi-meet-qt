#include "XMPPClient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QDateTime>
#include <QRegularExpression>

XMPPClient::XMPPClient(QObject *parent)
    : QObject(parent)
    , m_webSocket(nullptr)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
    , m_connectionState(Disconnected)
    , m_reconnectAttempts(0)
    , m_audioMuted(false)
    , m_videoMuted(false)
{
    // 设置心跳定时器
    m_heartbeatTimer->setSingleShot(false);
    m_heartbeatTimer->setInterval(HEARTBEAT_INTERVAL);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &XMPPClient::onHeartbeatTimer);

    // 设置重连定时器
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &XMPPClient::onReconnectTimer);

    qDebug() << "XMPPClient initialized";
}

XMPPClient::~XMPPClient()
{
    disconnect();
    qDebug() << "XMPPClient destroyed";
}

void XMPPClient::connectToServer(const QString& serverUrl, const QString& roomName, const QString& displayName)
{
    if (m_connectionState != Disconnected && m_connectionState != Error) {
        qWarning() << "Already connected or connecting";
        return;
    }

    qDebug() << "Connecting to server:" << serverUrl << "room:" << roomName;

    m_serverUrl = serverUrl;
    m_roomName = roomName;
    m_displayName = displayName.isEmpty() ? QString("User_%1").arg(QUuid::createUuid().toString().mid(1, 8)) : displayName;
    m_sessionId = generateUniqueId();

    // 重置连接状态
    resetConnection();
    setConnectionState(Connecting);

    // 首先获取服务器配置
    fetchServerConfiguration();
}

void XMPPClient::disconnect()
{
    if (m_connectionState == Disconnected) {
        return;
    }

    qDebug() << "Disconnecting from server";
    setConnectionState(Disconnecting);

    // 停止定时器
    m_heartbeatTimer->stop();
    stopReconnection();

    // 如果在房间中，先离开房间
    if (m_connectionState == InRoom) {
        leaveRoom();
    }

    // 关闭WebSocket连接
    if (m_webSocket) {
        m_webSocket->close();
    }

    // 重置状态
    resetConnection();
    setConnectionState(Disconnected);
    emit disconnected();
}

void XMPPClient::sendChatMessage(const QString& message)
{
    if (!isInRoom() || message.isEmpty()) {
        qWarning() << "Cannot send message: not in room or empty message";
        return;
    }

    QString messageStanza = QString(
        "<message to='%1' type='groupchat' id='%2'>"
        "<body>%3</body>"
        "</message>"
    ).arg(m_roomJid, generateUniqueId(), message.toHtmlEscaped());

    sendXMPPStanza(messageStanza);
    qDebug() << "Sent chat message:" << message;
}

void XMPPClient::sendPresence(const QString& status)
{
    if (!isConnected()) {
        qWarning() << "Cannot send presence: not connected";
        return;
    }

    QString presenceStanza = QString(
        "<presence to='%1/%2' id='%3'>"
    ).arg(m_roomJid, m_displayName, generateUniqueId());

    if (!status.isEmpty()) {
        presenceStanza += QString("<status>%1</status>").arg(status.toHtmlEscaped());
    }

    // 添加音视频状态
    presenceStanza += QString(
        "<audiomuted xmlns='http://jitsi.org/jitmeet/audio'>%1</audiomuted>"
        "<videomuted xmlns='http://jitsi.org/jitmeet/video'>%2</videomuted>"
    ).arg(m_audioMuted ? "true" : "false", m_videoMuted ? "true" : "false");

    presenceStanza += "</presence>";

    sendXMPPStanza(presenceStanza);
    qDebug() << "Sent presence with status:" << status;
}

void XMPPClient::setAudioMuted(bool muted)
{
    if (m_audioMuted != muted) {
        m_audioMuted = muted;
        sendPresence(); // 发送更新的存在信息
        qDebug() << "Audio muted state changed to:" << muted;
    }
}

void XMPPClient::setVideoMuted(bool muted)
{
    if (m_videoMuted != muted) {
        m_videoMuted = muted;
        sendPresence(); // 发送更新的存在信息
        qDebug() << "Video muted state changed to:" << muted;
    }
}

void XMPPClient::leaveRoom()
{
    if (m_connectionState != InRoom) {
        return;
    }

    qDebug() << "Leaving room:" << m_roomName;

    // 发送离开房间的存在信息
    QString leavePresence = QString(
        "<presence to='%1/%2' type='unavailable' id='%3'/>"
    ).arg(m_roomJid, m_displayName, generateUniqueId());

    sendXMPPStanza(leavePresence);

    // 清空参与者列表
    m_participants.clear();
    emit roomLeft();
}

void XMPPClient::setConnectionState(ConnectionState state)
{
    if (m_connectionState != state) {
        ConnectionState oldState = m_connectionState;
        m_connectionState = state;
        qDebug() << "Connection state changed from" << oldState << "to" << state;
        emit connectionStateChanged(state);
    }
}

void XMPPClient::fetchServerConfiguration()
{
    QUrl configUrl(m_serverUrl + "/config.js");
    QNetworkRequest request(configUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "JitsiMeetQt/1.0");

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &XMPPClient::onConfigurationReceived);
    connect(reply, &QNetworkReply::errorOccurred,
            [this, reply](QNetworkReply::NetworkError error) {
                qWarning() << "Failed to fetch server configuration:" << error << reply->errorString();
                // 使用默认配置继续连接
                establishWebSocketConnection();
                reply->deleteLater();
            });

    qDebug() << "Fetching server configuration from:" << configUrl;
}

void XMPPClient::establishWebSocketConnection()
{
    // 构建WebSocket URL
    QUrl serverUrl(m_serverUrl);
    QString wsProtocol = (serverUrl.scheme() == "https") ? "wss" : "ws";
    
    // 使用默认的Jitsi Meet WebSocket路径
    m_websocketUrl = QString("%1://%2/xmpp-websocket?room=%3")
                     .arg(wsProtocol, serverUrl.host(), m_roomName);

    // 设置域名信息
    m_domain = serverUrl.host();
    m_mucDomain = QString("conference.%1").arg(m_domain);
    m_roomJid = QString("%1@%2").arg(m_roomName, m_mucDomain);
    m_focusJid = QString("focus@auth.%1").arg(m_domain);

    qDebug() << "Establishing WebSocket connection to:" << m_websocketUrl;
    qDebug() << "Room JID:" << m_roomJid;
    qDebug() << "Domain:" << m_domain;

    // 创建WebSocket连接
    if (m_webSocket) {
        m_webSocket->deleteLater();
    }

    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    
    // 连接信号
    connect(m_webSocket, &QWebSocket::connected, this, &XMPPClient::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &XMPPClient::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &XMPPClient::onWebSocketMessageReceived);
    connect(m_webSocket, &QWebSocket::errorOccurred,
            this, &XMPPClient::onWebSocketError);

    // 开始连接
    m_webSocket->open(QUrl(m_websocketUrl));
}

void XMPPClient::sendXMPPStanza(const QString& stanza)
{
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send XMPP stanza: WebSocket not connected";
        return;
    }

    qDebug() << "Sending XMPP stanza:" << stanza;
    m_webSocket->sendTextMessage(stanza);
}

void XMPPClient::processXMPPMessage(const QDomDocument& doc)
{
    QDomElement root = doc.documentElement();
    QString tagName = root.tagName();

    qDebug() << "Processing XMPP message:" << tagName;

    if (tagName == "presence") {
        handlePresence(root);
    } else if (tagName == "message") {
        handleMessage(root);
    } else if (tagName == "iq") {
        handleIQ(root);
    } else {
        qDebug() << "Unknown XMPP stanza type:" << tagName;
    }
}

void XMPPClient::handlePresence(const QDomElement& element)
{
    QString from = element.attribute("from");
    QString type = element.attribute("type");
    
    qDebug() << "Handling presence from:" << from << "type:" << type;

    // 解析JID
    QString node, domain, resource;
    parseJID(from, node, domain, resource);

    if (type == "unavailable") {
        // 参与者离开
        if (m_participants.contains(from)) {
            m_participants.remove(from);
            emit participantLeft(from);
            qDebug() << "Participant left:" << from;
        }
    } else {
        // 参与者加入或更新
        Participant participant;
        participant.jid = from;
        participant.displayName = resource.isEmpty() ? node : resource;
        participant.role = "participant";
        participant.joinTime = QDateTime::currentDateTime();

        // 解析音视频状态
        QDomElement audioMuted = element.firstChildElement("audiomuted");
        if (!audioMuted.isNull()) {
            participant.audioMuted = (audioMuted.text() == "true");
        }

        QDomElement videoMuted = element.firstChildElement("videomuted");
        if (!videoMuted.isNull()) {
            participant.videoMuted = (videoMuted.text() == "true");
        }

        // 解析状态
        QDomElement status = element.firstChildElement("status");
        if (!status.isNull()) {
            participant.status = status.text();
        }

        bool isNewParticipant = !m_participants.contains(from);
        m_participants[from] = participant;

        if (isNewParticipant) {
            emit participantJoined(participant);
            qDebug() << "New participant joined:" << participant.displayName;
        } else {
            emit participantUpdated(participant);
            qDebug() << "Participant updated:" << participant.displayName;
        }
    }
}

void XMPPClient::handleMessage(const QDomElement& element)
{
    QString from = element.attribute("from");
    QString type = element.attribute("type");
    
    if (type == "groupchat") {
        QDomElement body = element.firstChildElement("body");
        if (!body.isNull()) {
            QString message = body.text();
            QDateTime timestamp = QDateTime::currentDateTime();
            
            // 检查是否有延迟标记
            QDomElement delay = element.firstChildElement("delay");
            if (!delay.isNull()) {
                QString stamp = delay.attribute("stamp");
                timestamp = QDateTime::fromString(stamp, Qt::ISODate);
            }

            emit chatMessageReceived(from, message, timestamp);
            qDebug() << "Received chat message from" << from << ":" << message;
        }
    }
}

void XMPPClient::handleIQ(const QDomElement& element)
{
    QString type = element.attribute("type");
    QString id = element.attribute("id");
    
    qDebug() << "Handling IQ type:" << type << "id:" << id;

    if (type == "result") {
        // 处理IQ结果
        qDebug() << "IQ result received for id:" << id;
    } else if (type == "error") {
        // 处理IQ错误
        QDomElement error = element.firstChildElement("error");
        if (!error.isNull()) {
            QString errorType = error.attribute("type");
            QString errorText = error.firstChildElement("text").text();
            qWarning() << "IQ error:" << errorType << errorText;
            emit errorOccurred(QString("XMPP IQ Error: %1 - %2").arg(errorType, errorText));
        }
    }
}

void XMPPClient::sendInitialPresence()
{
    // 生成用户JID
    m_userJid = QString("%1@%2/%3").arg(generateUniqueId(), m_domain, m_displayName);
    
    qDebug() << "Sending initial presence with JID:" << m_userJid;
    
    // 发送初始存在信息
    sendPresence("available");
    
    // 设置为已认证状态
    setConnectionState(Authenticated);
    emit authenticated();
    
    // 加入MUC房间
    joinMUCRoom();
}

void XMPPClient::joinMUCRoom()
{
    qDebug() << "Joining MUC room:" << m_roomJid;
    setConnectionState(JoiningRoom);

    // 发送加入房间的存在信息
    QString joinPresence = QString(
        "<presence to='%1/%2' id='%3'>"
        "<x xmlns='http://jabber.org/protocol/muc'/>"
        "<audiomuted xmlns='http://jitsi.org/jitmeet/audio'>%4</audiomuted>"
        "<videomuted xmlns='http://jitsi.org/jitmeet/video'>%5</videomuted>"
        "</presence>"
    ).arg(m_roomJid, m_displayName, generateUniqueId())
     .arg(m_audioMuted ? "true" : "false")
     .arg(m_videoMuted ? "true" : "false");

    sendXMPPStanza(joinPresence);

    // 设置为在房间中状态
    setConnectionState(InRoom);
    emit roomJoined();
    qDebug() << "Successfully joined room:" << m_roomName;
}

QString XMPPClient::generateUniqueId()
{
    return QUuid::createUuid().toString().remove('{').remove('}').remove('-');
}

QString XMPPClient::buildJID(const QString& node, const QString& domain, const QString& resource)
{
    QString jid = node + "@" + domain;
    if (!resource.isEmpty()) {
        jid += "/" + resource;
    }
    return jid;
}

void XMPPClient::parseJID(const QString& jid, QString& node, QString& domain, QString& resource)
{
    QRegularExpression jidRegex("^(?:([^@/]+)@)?([^@/]+)(?:/(.+))?$");
    QRegularExpressionMatch match = jidRegex.match(jid);
    
    if (match.hasMatch()) {
        node = match.captured(1);
        domain = match.captured(2);
        resource = match.captured(3);
    } else {
        qWarning() << "Invalid JID format:" << jid;
        node.clear();
        domain.clear();
        resource.clear();
    }
}

void XMPPClient::startReconnection()
{
    if (m_reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
        qWarning() << "Maximum reconnection attempts reached";
        setConnectionState(Error);
        emit errorOccurred("Maximum reconnection attempts reached");
        return;
    }

    m_reconnectAttempts++;
    qDebug() << "Starting reconnection attempt" << m_reconnectAttempts << "of" << MAX_RECONNECT_ATTEMPTS;
    
    m_reconnectTimer->start(RECONNECT_INTERVAL);
}

void XMPPClient::stopReconnection()
{
    m_reconnectTimer->stop();
    m_reconnectAttempts = 0;
}

void XMPPClient::resetConnection()
{
    m_participants.clear();
    m_userJid.clear();
    m_roomJid.clear();
    m_sessionId.clear();
    stopReconnection();
}

// Slots implementation

void XMPPClient::onWebSocketConnected()
{
    qDebug() << "WebSocket connected successfully";
    setConnectionState(Connected);
    emit connected();

    // 启动心跳
    m_heartbeatTimer->start();
    
    // 重置重连计数
    m_reconnectAttempts = 0;

    // 发送初始存在信息
    sendInitialPresence();
}

void XMPPClient::onWebSocketDisconnected()
{
    qDebug() << "WebSocket disconnected";
    m_heartbeatTimer->stop();

    if (m_connectionState == Disconnecting) {
        // 主动断开连接
        setConnectionState(Disconnected);
        emit disconnected();
    } else {
        // 意外断开，尝试重连
        qWarning() << "Unexpected disconnection, attempting to reconnect";
        setConnectionState(Error);
        startReconnection();
    }
}

void XMPPClient::onWebSocketMessageReceived(const QString& message)
{
    qDebug() << "Received WebSocket message:" << message;

    // 解析XML消息
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    
    if (!doc.setContent(message, &errorMsg, &errorLine, &errorColumn)) {
        qWarning() << "Failed to parse XML message:" << errorMsg 
                   << "at line" << errorLine << "column" << errorColumn;
        return;
    }

    // 处理XMPP消息
    processXMPPMessage(doc);
}

void XMPPClient::onWebSocketError(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket error:" << error;
    setConnectionState(Error);
    emit errorOccurred(QString("WebSocket error: %1").arg(error));
    
    // 尝试重连
    startReconnection();
}

void XMPPClient::onHeartbeatTimer()
{
    if (isConnected()) {
        // 发送心跳ping
        QString pingStanza = QString(
            "<iq type='get' to='%1' id='%2'>"
            "<ping xmlns='urn:xmpp:ping'/>"
            "</iq>"
        ).arg(m_domain, generateUniqueId());
        
        sendXMPPStanza(pingStanza);
        qDebug() << "Sent heartbeat ping";
    }
}

void XMPPClient::onReconnectTimer()
{
    qDebug() << "Attempting to reconnect...";
    
    // 重置WebSocket连接
    if (m_webSocket) {
        m_webSocket->deleteLater();
        m_webSocket = nullptr;
    }
    
    // 重新建立连接
    establishWebSocketConnection();
}

void XMPPClient::onConfigurationReceived()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QString configText = QString::fromUtf8(data);
        
        // 解析JavaScript配置文件
        // 简单的正则表达式解析，实际项目中可能需要更复杂的解析
        QRegularExpression configRegex("config\\s*=\\s*({.*?});", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match = configRegex.match(configText);
        
        if (match.hasMatch()) {
            QString jsonText = match.captured(1);
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &parseError);
            
            if (parseError.error == QJsonParseError::NoError) {
                m_serverConfig = doc.object();
                qDebug() << "Server configuration loaded successfully";
                
                // 从配置中提取WebSocket URL等信息
                QJsonObject websocket = m_serverConfig["websocket"].toObject();
                if (!websocket.isEmpty()) {
                    QString wsUrl = websocket["url"].toString();
                    if (!wsUrl.isEmpty()) {
                        m_websocketUrl = wsUrl;
                    }
                }
            } else {
                qWarning() << "Failed to parse server configuration JSON:" << parseError.errorString();
            }
        } else {
            qWarning() << "Failed to extract configuration from response";
        }
    } else {
        qWarning() << "Failed to fetch server configuration:" << reply->errorString();
    }

    reply->deleteLater();
    
    // 继续建立WebSocket连接
    establishWebSocketConnection();
}