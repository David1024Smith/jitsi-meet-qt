#include "WebSocketProtocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDebug>
#include <QUrl>
#include <QSslError>

class WebSocketProtocol::Private
{
public:
    Private() 
        : status(IProtocolHandler::Inactive)
        , webSocket(nullptr)
        , webSocketState(WebSocketProtocol::Unconnected)
        , messageFormat(WebSocketProtocol::JsonMessage)
        , connectionTimeout(30000)
        , heartbeatInterval(30000)
        , autoReconnect(true)
        , reconnectInterval(5000)
        , maxReconnectAttempts(5)
        , currentReconnectAttempt(0)
        , connectionLatency(0)
    {
    }

    IProtocolHandler::ProtocolStatus status;
    QWebSocket* webSocket;
    WebSocketProtocol::WebSocketState webSocketState;
    WebSocketProtocol::MessageFormat messageFormat;
    
    QString serverUrl;
    QVariantMap parameters;
    QVariantMap stats;
    
    int connectionTimeout;
    int heartbeatInterval;
    bool autoReconnect;
    int reconnectInterval;
    int maxReconnectAttempts;
    int currentReconnectAttempt;
    int connectionLatency;
    
    QTimer* heartbeatTimer;
    QTimer* reconnectTimer;
    QTimer* connectionTimeoutTimer;
    QTimer* sendQueueTimer;
    
    QQueue<QByteArray> sendQueue;
    QDateTime lastHeartbeatSent;
    QDateTime connectionStartTime;
};

WebSocketProtocol::WebSocketProtocol(QObject *parent)
    : IProtocolHandler(parent)
    , d(new Private)
{
    d->heartbeatTimer = new QTimer(this);
    d->reconnectTimer = new QTimer(this);
    d->connectionTimeoutTimer = new QTimer(this);
    d->sendQueueTimer = new QTimer(this);
    
    connect(d->heartbeatTimer, &QTimer::timeout, this, &WebSocketProtocol::handleHeartbeatTimer);
    connect(d->reconnectTimer, &QTimer::timeout, this, &WebSocketProtocol::handleReconnectTimer);
    connect(d->connectionTimeoutTimer, &QTimer::timeout, this, &WebSocketProtocol::handleConnectionTimeout);
    connect(d->sendQueueTimer, &QTimer::timeout, this, &WebSocketProtocol::handleSendQueueTimer);
    
    d->heartbeatTimer->setSingleShot(false);
    d->reconnectTimer->setSingleShot(true);
    d->connectionTimeoutTimer->setSingleShot(true);
    d->sendQueueTimer->setSingleShot(false);
    d->sendQueueTimer->setInterval(100); // 处理发送队列间隔100ms
}

WebSocketProtocol::~WebSocketProtocol()
{
    stop();
    delete d;
}

bool WebSocketProtocol::initialize(const QVariantMap& config)
{
    qDebug() << "WebSocketProtocol: Initializing with config:" << config;
    
    d->status = IProtocolHandler::Initializing;
    emit protocolStatusChanged(d->status);
    
    // 应用配置参数
    if (config.contains("serverUrl")) {
        d->serverUrl = config["serverUrl"].toString();
    }
    if (config.contains("connectionTimeout")) {
        d->connectionTimeout = config["connectionTimeout"].toInt();
    }
    if (config.contains("heartbeatInterval")) {
        d->heartbeatInterval = config["heartbeatInterval"].toInt();
    }
    if (config.contains("autoReconnect")) {
        d->autoReconnect = config["autoReconnect"].toBool();
    }
    if (config.contains("reconnectInterval")) {
        d->reconnectInterval = config["reconnectInterval"].toInt();
    }
    if (config.contains("maxReconnectAttempts")) {
        d->maxReconnectAttempts = config["maxReconnectAttempts"].toInt();
    }
    if (config.contains("messageFormat")) {
        d->messageFormat = static_cast<MessageFormat>(config["messageFormat"].toInt());
    }
    
    // 初始化WebSocket
    if (!initializeWebSocket()) {
        d->status = IProtocolHandler::Error;
        emit protocolStatusChanged(d->status);
        emit protocolError("Failed to initialize WebSocket");
        return false;
    }
    
    // 初始化统计信息
    d->stats["messagesSent"] = 0;
    d->stats["messagesReceived"] = 0;
    d->stats["bytesSent"] = 0;
    d->stats["bytesReceived"] = 0;
    d->stats["reconnectAttempts"] = 0;
    d->stats["startTime"] = QDateTime::currentMSecsSinceEpoch();
    
    d->status = IProtocolHandler::Active;
    emit protocolStatusChanged(d->status);
    
    qDebug() << "WebSocketProtocol: Initialization completed successfully";
    return true;
}

bool WebSocketProtocol::start()
{
    if (d->status != IProtocolHandler::Active) {
        qWarning() << "WebSocketProtocol: Cannot start - protocol not active";
        return false;
    }
    
    qDebug() << "WebSocketProtocol: Starting protocol";
    
    // 启动发送队列处理
    d->sendQueueTimer->start();
    
    // 如果有服务器URL，自动连接
    if (!d->serverUrl.isEmpty()) {
        connectToServer(d->serverUrl);
    }
    
    emit protocolStarted();
    qDebug() << "WebSocketProtocol: Protocol started successfully";
    
    return true;
}

void WebSocketProtocol::stop()
{
    qDebug() << "WebSocketProtocol: Stopping protocol";
    
    // 停止所有定时器
    stopHeartbeatTimer();
    stopReconnectTimer();
    d->connectionTimeoutTimer->stop();
    d->sendQueueTimer->stop();
    
    // 断开WebSocket连接
    disconnectFromServer();
    
    // 清理WebSocket
    cleanupWebSocket();
    
    d->status = IProtocolHandler::Shutdown;
    emit protocolStatusChanged(d->status);
    emit protocolStopped();
    
    qDebug() << "WebSocketProtocol: Protocol stopped";
}

IProtocolHandler::ProtocolStatus WebSocketProtocol::protocolStatus() const
{
    return d->status;
}

QString WebSocketProtocol::protocolName() const
{
    return "WebSocket";
}

QString WebSocketProtocol::protocolVersion() const
{
    return "13"; // WebSocket协议版本
}

QByteArray WebSocketProtocol::encodeMessage(MessageType type, const QVariantMap& data)
{
    if (d->messageFormat == JsonMessage) {
        QJsonObject message;
        message["type"] = static_cast<int>(type);
        message["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        
        QJsonObject payload;
        for (auto it = data.begin(); it != data.end(); ++it) {
            payload[it.key()] = QJsonValue::fromVariant(it.value());
        }
        message["payload"] = payload;
        
        QJsonDocument doc(message);
        return doc.toJson(QJsonDocument::Compact);
    } else {
        // 对于文本或二进制消息，直接返回数据
        return data.value("data").toByteArray();
    }
}

bool WebSocketProtocol::decodeMessage(const QByteArray& rawData, MessageType& type, QVariantMap& data)
{
    if (d->messageFormat == JsonMessage) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(rawData, &error);
        
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "WebSocketProtocol: JSON parse error:" << error.errorString();
            return false;
        }
        
        QJsonObject message = doc.object();
        
        if (!message.contains("type") || !message.contains("payload")) {
            qWarning() << "WebSocketProtocol: Invalid message format";
            return false;
        }
        
        type = static_cast<MessageType>(message["type"].toInt());
        
        QJsonObject payload = message["payload"].toObject();
        data.clear();
        for (auto it = payload.begin(); it != payload.end(); ++it) {
            data[it.key()] = it.value().toVariant();
        }
        
        return true;
    } else {
        // 对于文本或二进制消息，设置为数据类型
        type = IProtocolHandler::Data;
        data.clear();
        data["data"] = rawData;
        return true;
    }
}

bool WebSocketProtocol::handleReceivedData(const QByteArray& data)
{
    MessageType type;
    QVariantMap messageData;
    
    if (!decodeMessage(data, type, messageData)) {
        return false;
    }
    
    // 检查是否为心跳响应
    if (isHeartbeatResponse(data)) {
        d->connectionLatency = d->lastHeartbeatSent.msecsTo(QDateTime::currentDateTime());
        emit heartbeatReceived();
        return true;
    }
    
    emit messageReceived(type, messageData);
    
    // 更新统计信息
    d->stats["messagesReceived"] = d->stats["messagesReceived"].toInt() + 1;
    d->stats["bytesReceived"] = d->stats["bytesReceived"].toLongLong() + data.size();
    
    return true;
}

bool WebSocketProtocol::sendMessage(MessageType type, const QVariantMap& data)
{
    if (d->webSocketState != Connected) {
        qWarning() << "WebSocketProtocol: Cannot send message - not connected";
        return false;
    }
    
    QByteArray encodedData = encodeMessage(type, data);
    
    bool result = false;
    if (d->messageFormat == JsonMessage || d->messageFormat == TextMessage) {
        result = sendTextMessage(QString::fromUtf8(encodedData));
    } else {
        result = sendBinaryMessage(encodedData);
    }
    
    if (result) {
        emit messageSent(type, data);
        
        // 更新统计信息
        d->stats["messagesSent"] = d->stats["messagesSent"].toInt() + 1;
        d->stats["bytesSent"] = d->stats["bytesSent"].toLongLong() + encodedData.size();
    }
    
    return result;
}

bool WebSocketProtocol::sendHeartbeat()
{
    QByteArray heartbeatData = generateHeartbeatMessage();
    d->lastHeartbeatSent = QDateTime::currentDateTime();
    
    bool result = false;
    if (d->messageFormat == JsonMessage || d->messageFormat == TextMessage) {
        result = sendTextMessage(QString::fromUtf8(heartbeatData));
    } else {
        result = sendBinaryMessage(heartbeatData);
    }
    
    if (result) {
        emit heartbeatSent();
    }
    
    return result;
}

bool WebSocketProtocol::supportsFeature(const QString& feature) const
{
    QStringList features = supportedFeatures();
    return features.contains(feature);
}

QStringList WebSocketProtocol::supportedFeatures() const
{
    return QStringList() 
        << "text-messages" 
        << "binary-messages" 
        << "json-messages"
        << "heartbeat"
        << "auto-reconnect"
        << "connection-timeout"
        << "ssl-support"
        << "message-queue";
}

void WebSocketProtocol::setParameter(const QString& key, const QVariant& value)
{
    d->parameters[key] = value;
    
    // 处理特殊参数
    if (key == "connectionTimeout") {
        d->connectionTimeout = value.toInt();
    } else if (key == "heartbeatInterval") {
        d->heartbeatInterval = value.toInt();
        if (d->heartbeatTimer->isActive()) {
            d->heartbeatTimer->setInterval(d->heartbeatInterval);
        }
    } else if (key == "autoReconnect") {
        d->autoReconnect = value.toBool();
    } else if (key == "reconnectInterval") {
        d->reconnectInterval = value.toInt();
    } else if (key == "maxReconnectAttempts") {
        d->maxReconnectAttempts = value.toInt();
    } else if (key == "messageFormat") {
        d->messageFormat = static_cast<MessageFormat>(value.toInt());
    }
}

QVariant WebSocketProtocol::parameter(const QString& key) const
{
    return d->parameters.value(key);
}

QVariantMap WebSocketProtocol::protocolStats() const
{
    QVariantMap stats = d->stats;
    stats["webSocketState"] = static_cast<int>(d->webSocketState);
    stats["serverUrl"] = d->serverUrl;
    stats["connectionLatency"] = d->connectionLatency;
    stats["sendQueueSize"] = d->sendQueue.size();
    stats["currentReconnectAttempt"] = d->currentReconnectAttempt;
    stats["uptime"] = QDateTime::currentMSecsSinceEpoch() - stats.value("startTime", 0).toLongLong();
    
    return stats;
}

bool WebSocketProtocol::connectToServer(const QString& url)
{
    if (!isValidServerUrl(url)) {
        qWarning() << "WebSocketProtocol: Invalid server URL:" << url;
        return false;
    }
    
    if (d->webSocketState == Connected || d->webSocketState == Connecting) {
        qWarning() << "WebSocketProtocol: Already connected or connecting";
        return false;
    }
    
    d->serverUrl = url;
    d->connectionStartTime = QDateTime::currentDateTime();
    
    qDebug() << "WebSocketProtocol: Connecting to" << url;
    
    updateWebSocketState(Connecting);
    
    // 启动连接超时定时器
    d->connectionTimeoutTimer->start(d->connectionTimeout);
    
    // 开始连接
    d->webSocket->open(QUrl(url));
    
    return true;
}

void WebSocketProtocol::disconnectFromServer()
{
    if (d->webSocketState == Unconnected || d->webSocketState == Closed) {
        return;
    }
    
    qDebug() << "WebSocketProtocol: Disconnecting from server";
    
    // 停止心跳和重连定时器
    stopHeartbeatTimer();
    stopReconnectTimer();
    d->connectionTimeoutTimer->stop();
    
    updateWebSocketState(Closing);
    
    if (d->webSocket) {
        d->webSocket->close();
    }
}

WebSocketProtocol::WebSocketState WebSocketProtocol::webSocketState() const
{
    return d->webSocketState;
}

bool WebSocketProtocol::sendTextMessage(const QString& message)
{
    if (d->webSocketState != Connected) {
        // 将消息加入发送队列
        enqueueMessage(message.toUtf8());
        return true;
    }
    
    qint64 bytesSent = d->webSocket->sendTextMessage(message);
    return bytesSent > 0;
}

bool WebSocketProtocol::sendBinaryMessage(const QByteArray& data)
{
    if (d->webSocketState != Connected) {
        // 将消息加入发送队列
        enqueueMessage(data);
        return true;
    }
    
    qint64 bytesSent = d->webSocket->sendBinaryMessage(data);
    return bytesSent > 0;
}

bool WebSocketProtocol::sendJsonMessage(const QVariantMap& json)
{
    QByteArray jsonData = serializeJsonMessage(json);
    return sendTextMessage(QString::fromUtf8(jsonData));
}

void WebSocketProtocol::setServerUrl(const QString& url)
{
    d->serverUrl = url;
}

QString WebSocketProtocol::serverUrl() const
{
    return d->serverUrl;
}

void WebSocketProtocol::setConnectionTimeout(int timeout)
{
    d->connectionTimeout = timeout;
}

int WebSocketProtocol::connectionTimeout() const
{
    return d->connectionTimeout;
}

void WebSocketProtocol::setHeartbeatInterval(int interval)
{
    d->heartbeatInterval = interval;
    if (d->heartbeatTimer->isActive()) {
        d->heartbeatTimer->setInterval(interval);
    }
}

int WebSocketProtocol::heartbeatInterval() const
{
    return d->heartbeatInterval;
}

void WebSocketProtocol::setAutoReconnect(bool enabled)
{
    d->autoReconnect = enabled;
}

bool WebSocketProtocol::autoReconnect() const
{
    return d->autoReconnect;
}

void WebSocketProtocol::setReconnectInterval(int interval)
{
    d->reconnectInterval = interval;
}

int WebSocketProtocol::reconnectInterval() const
{
    return d->reconnectInterval;
}

void WebSocketProtocol::setMaxReconnectAttempts(int maxAttempts)
{
    d->maxReconnectAttempts = maxAttempts;
}

int WebSocketProtocol::maxReconnectAttempts() const
{
    return d->maxReconnectAttempts;
}

void WebSocketProtocol::setMessageFormat(MessageFormat format)
{
    d->messageFormat = format;
}

WebSocketProtocol::MessageFormat WebSocketProtocol::messageFormat() const
{
    return d->messageFormat;
}

int WebSocketProtocol::connectionLatency() const
{
    return d->connectionLatency;
}

int WebSocketProtocol::sendQueueSize() const
{
    return d->sendQueue.size();
}

void WebSocketProtocol::reset()
{
    qDebug() << "WebSocketProtocol: Resetting protocol";
    
    stop();
    
    d->serverUrl.clear();
    d->parameters.clear();
    d->stats.clear();
    d->sendQueue.clear();
    d->currentReconnectAttempt = 0;
    d->connectionLatency = 0;
    
    d->connectionTimeout = 30000;
    d->heartbeatInterval = 30000;
    d->autoReconnect = true;
    d->reconnectInterval = 5000;
    d->maxReconnectAttempts = 5;
    d->messageFormat = JsonMessage;
    
    updateWebSocketState(Unconnected);
    
    d->status = IProtocolHandler::Inactive;
    emit protocolStatusChanged(d->status);
}

void WebSocketProtocol::refresh()
{
    qDebug() << "WebSocketProtocol: Refreshing protocol";
    
    // 更新统计信息
    d->stats["lastRefresh"] = QDateTime::currentMSecsSinceEpoch();
    emit statsUpdated(protocolStats());
}

void WebSocketProtocol::reconnect()
{
    qDebug() << "WebSocketProtocol: Manual reconnect requested";
    
    disconnectFromServer();
    
    QTimer::singleShot(1000, this, [this]() {
        if (!d->serverUrl.isEmpty()) {
            connectToServer(d->serverUrl);
        }
    });
}

void WebSocketProtocol::clearSendQueue()
{
    qDebug() << "WebSocketProtocol: Clearing send queue";
    d->sendQueue.clear();
}

void WebSocketProtocol::flushSendQueue()
{
    qDebug() << "WebSocketProtocol: Flushing send queue";
    processMessageQueue();
}

void WebSocketProtocol::handleConnected()
{
    qDebug() << "WebSocketProtocol: Connected to server";
    
    d->connectionTimeoutTimer->stop();
    d->currentReconnectAttempt = 0;
    
    updateWebSocketState(Connected);
    
    // 启动心跳定时器
    startHeartbeatTimer();
    
    // 处理发送队列中的消息
    processMessageQueue();
    
    emit connected();
}

void WebSocketProtocol::handleDisconnected()
{
    qDebug() << "WebSocketProtocol: Disconnected from server";
    
    stopHeartbeatTimer();
    d->connectionTimeoutTimer->stop();
    
    updateWebSocketState(Unconnected);
    
    emit disconnected();
    
    // 如果启用了自动重连，开始重连
    if (d->autoReconnect && d->currentReconnectAttempt < d->maxReconnectAttempts) {
        startReconnectTimer();
    }
}

void WebSocketProtocol::handleTextMessageReceived(const QString& message)
{
    QByteArray data = message.toUtf8();
    
    if (d->messageFormat == JsonMessage) {
        QVariantMap json = parseJsonMessage(data);
        emit jsonMessageReceived(json);
    } else {
        emit textMessageReceived(message);
    }
    
    handleReceivedData(data);
}

void WebSocketProtocol::handleBinaryMessageReceived(const QByteArray& data)
{
    emit binaryMessageReceived(data);
    handleReceivedData(data);
}

void WebSocketProtocol::handleWebSocketError(QAbstractSocket::SocketError error)
{
    QString errorString = d->webSocket->errorString();
    
    qWarning() << "WebSocketProtocol: WebSocket error:" << errorString;
    
    emit connectionError(errorString);
    emit protocolError(QString("WebSocket error: %1").arg(errorString));
    
    // 如果启用了自动重连，开始重连
    if (d->autoReconnect && d->currentReconnectAttempt < d->maxReconnectAttempts) {
        startReconnectTimer();
    }
}

void WebSocketProtocol::handleSslErrors(const QList<QSslError>& errors)
{
    QStringList errorStrings;
    for (const QSslError& error : errors) {
        errorStrings << error.errorString();
    }
    
    QString errorMessage = QString("SSL errors: %1").arg(errorStrings.join(", "));
    emit protocolError(errorMessage);
    
    qWarning() << "WebSocketProtocol:" << errorMessage;
}

void WebSocketProtocol::handleHeartbeatTimer()
{
    sendHeartbeat();
}

void WebSocketProtocol::handleReconnectTimer()
{
    if (d->currentReconnectAttempt >= d->maxReconnectAttempts) {
        qWarning() << "WebSocketProtocol: Max reconnect attempts reached";
        emit reconnectFailed("Max reconnect attempts reached");
        return;
    }
    
    d->currentReconnectAttempt++;
    d->stats["reconnectAttempts"] = d->stats["reconnectAttempts"].toInt() + 1;
    
    qDebug() << "WebSocketProtocol: Reconnect attempt" << d->currentReconnectAttempt;
    
    emit reconnectStarted(d->currentReconnectAttempt);
    
    if (!d->serverUrl.isEmpty()) {
        connectToServer(d->serverUrl);
    }
}

void WebSocketProtocol::handleConnectionTimeout()
{
    qWarning() << "WebSocketProtocol: Connection timeout";
    
    emit connectionError("Connection timeout");
    
    if (d->webSocket) {
        d->webSocket->abort();
    }
    
    updateWebSocketState(Unconnected);
    
    // 如果启用了自动重连，开始重连
    if (d->autoReconnect && d->currentReconnectAttempt < d->maxReconnectAttempts) {
        startReconnectTimer();
    }
}

void WebSocketProtocol::handleSendQueueTimer()
{
    if (d->webSocketState == Connected && !d->sendQueue.isEmpty()) {
        processMessageQueue();
    }
}

bool WebSocketProtocol::initializeWebSocket()
{
    if (d->webSocket) {
        cleanupWebSocket();
    }
    
    d->webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    
    // 连接信号
    connect(d->webSocket, &QWebSocket::connected, this, &WebSocketProtocol::handleConnected);
    connect(d->webSocket, &QWebSocket::disconnected, this, &WebSocketProtocol::handleDisconnected);
    connect(d->webSocket, &QWebSocket::textMessageReceived, this, &WebSocketProtocol::handleTextMessageReceived);
    connect(d->webSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketProtocol::handleBinaryMessageReceived);
    connect(d->webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &WebSocketProtocol::handleWebSocketError);
    connect(d->webSocket, &QWebSocket::sslErrors, this, &WebSocketProtocol::handleSslErrors);
    
    return true;
}

void WebSocketProtocol::cleanupWebSocket()
{
    if (d->webSocket) {
        d->webSocket->disconnect();
        d->webSocket->deleteLater();
        d->webSocket = nullptr;
    }
}

void WebSocketProtocol::startHeartbeatTimer()
{
    if (d->heartbeatInterval > 0) {
        d->heartbeatTimer->start(d->heartbeatInterval);
    }
}

void WebSocketProtocol::stopHeartbeatTimer()
{
    d->heartbeatTimer->stop();
}

void WebSocketProtocol::startReconnectTimer()
{
    if (d->reconnectInterval > 0) {
        d->reconnectTimer->start(d->reconnectInterval);
    }
}

void WebSocketProtocol::stopReconnectTimer()
{
    d->reconnectTimer->stop();
}

void WebSocketProtocol::updateWebSocketState(WebSocketState state)
{
    if (d->webSocketState != state) {
        d->webSocketState = state;
        emit webSocketStateChanged(state);
        
        qDebug() << "WebSocketProtocol: State changed to" << state;
    }
}

void WebSocketProtocol::processMessageQueue()
{
    while (!d->sendQueue.isEmpty() && d->webSocketState == Connected) {
        QByteArray message = d->sendQueue.dequeue();
        
        if (d->messageFormat == BinaryMessage) {
            d->webSocket->sendBinaryMessage(message);
        } else {
            d->webSocket->sendTextMessage(QString::fromUtf8(message));
        }
    }
}

void WebSocketProtocol::enqueueMessage(const QByteArray& message)
{
    d->sendQueue.enqueue(message);
    
    // 如果队列太大，移除最旧的消息
    const int maxQueueSize = 1000;
    while (d->sendQueue.size() > maxQueueSize) {
        QByteArray droppedMessage = d->sendQueue.dequeue();
        emit messageSendFailed(droppedMessage, "Send queue overflow");
    }
}

bool WebSocketProtocol::isValidServerUrl(const QString& url) const
{
    QUrl qurl(url);
    return qurl.isValid() && (qurl.scheme() == "ws" || qurl.scheme() == "wss");
}

QVariantMap WebSocketProtocol::parseJsonMessage(const QByteArray& data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "WebSocketProtocol: JSON parse error:" << error.errorString();
        return QVariantMap();
    }
    
    return doc.object().toVariantMap();
}

QByteArray WebSocketProtocol::serializeJsonMessage(const QVariantMap& json)
{
    QJsonDocument doc(QJsonObject::fromVariantMap(json));
    return doc.toJson(QJsonDocument::Compact);
}

QByteArray WebSocketProtocol::generateHeartbeatMessage()
{
    if (d->messageFormat == JsonMessage) {
        QVariantMap heartbeat;
        heartbeat["type"] = "heartbeat";
        heartbeat["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        return serializeJsonMessage(heartbeat);
    } else {
        return QByteArray("PING");
    }
}

bool WebSocketProtocol::isHeartbeatResponse(const QByteArray& data)
{
    if (d->messageFormat == JsonMessage) {
        QVariantMap json = parseJsonMessage(data);
        return json.value("type").toString() == "heartbeat" || json.value("type").toString() == "pong";
    } else {
        return data == "PONG";
    }
}