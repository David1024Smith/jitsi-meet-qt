#include "HTTPProtocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDebug>
#include <QNetworkRequest>
#include <QSslError>
#include <QUrlQuery>
#include <QTimer>

class HTTPProtocol::Private
{
public:
    Private() 
        : status(IProtocolHandler::Inactive)
        , networkManager(nullptr)
        , requestTimeout(30000)
        , maxConcurrentRequests(10)
        , currentRequestCount(0)
    {
    }

    IProtocolHandler::ProtocolStatus status;
    QNetworkAccessManager* networkManager;
    
    QString baseUrl;
    QVariantMap defaultHeaders;
    QVariantMap parameters;
    QVariantMap stats;
    
    int requestTimeout;
    int maxConcurrentRequests;
    int currentRequestCount;
    
    QMap<QString, QNetworkReply*> activeRequests;
    QMap<QNetworkReply*, QString> replyToRequestId;
    QMap<QString, QTimer*> requestTimers;
    QQueue<QVariantMap> requestQueue;
    
    QTimer* queueProcessTimer;
};

HTTPProtocol::HTTPProtocol(QObject *parent)
    : IProtocolHandler(parent)
    , d(new Private)
{
    d->networkManager = new QNetworkAccessManager(this);
    d->queueProcessTimer = new QTimer(this);
    
    connect(d->queueProcessTimer, &QTimer::timeout, this, &HTTPProtocol::processRequestQueue);
    d->queueProcessTimer->setSingleShot(false);
    d->queueProcessTimer->setInterval(100); // 检查队列间隔100ms
}

HTTPProtocol::~HTTPProtocol()
{
    stop();
    delete d;
}

bool HTTPProtocol::initialize(const QVariantMap& config)
{
    qDebug() << "HTTPProtocol: Initializing with config:" << config;
    
    d->status = IProtocolHandler::Initializing;
    emit protocolStatusChanged(d->status);
    
    // 应用配置参数
    if (config.contains("baseUrl")) {
        d->baseUrl = config["baseUrl"].toString();
    }
    if (config.contains("defaultHeaders")) {
        d->defaultHeaders = config["defaultHeaders"].toMap();
    }
    if (config.contains("requestTimeout")) {
        d->requestTimeout = config["requestTimeout"].toInt();
    }
    if (config.contains("maxConcurrentRequests")) {
        d->maxConcurrentRequests = config["maxConcurrentRequests"].toInt();
    }
    
    // 初始化统计信息
    d->stats["requestsSent"] = 0;
    d->stats["requestsCompleted"] = 0;
    d->stats["requestsFailed"] = 0;
    d->stats["bytesUploaded"] = 0;
    d->stats["bytesDownloaded"] = 0;
    d->stats["startTime"] = QDateTime::currentMSecsSinceEpoch();
    
    d->status = IProtocolHandler::Active;
    emit protocolStatusChanged(d->status);
    
    qDebug() << "HTTPProtocol: Initialization completed successfully";
    return true;
}

bool HTTPProtocol::start()
{
    if (d->status != IProtocolHandler::Active) {
        qWarning() << "HTTPProtocol: Cannot start - protocol not active";
        return false;
    }
    
    qDebug() << "HTTPProtocol: Starting protocol";
    
    // 启动队列处理定时器
    d->queueProcessTimer->start();
    
    emit protocolStarted();
    qDebug() << "HTTPProtocol: Protocol started successfully";
    
    return true;
}

void HTTPProtocol::stop()
{
    qDebug() << "HTTPProtocol: Stopping protocol";
    
    // 停止队列处理
    d->queueProcessTimer->stop();
    
    // 取消所有活动请求
    clearAllRequests();
    
    d->status = IProtocolHandler::Shutdown;
    emit protocolStatusChanged(d->status);
    emit protocolStopped();
    
    qDebug() << "HTTPProtocol: Protocol stopped";
}

IProtocolHandler::ProtocolStatus HTTPProtocol::protocolStatus() const
{
    return d->status;
}

QString HTTPProtocol::protocolName() const
{
    return "HTTP";
}

QString HTTPProtocol::protocolVersion() const
{
    return "1.1";
}

QByteArray HTTPProtocol::encodeMessage(MessageType type, const QVariantMap& data)
{
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
}

bool HTTPProtocol::decodeMessage(const QByteArray& rawData, MessageType& type, QVariantMap& data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "HTTPProtocol: JSON parse error:" << error.errorString();
        return false;
    }
    
    QJsonObject message = doc.object();
    
    if (!message.contains("type") || !message.contains("payload")) {
        qWarning() << "HTTPProtocol: Invalid message format";
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

bool HTTPProtocol::handleReceivedData(const QByteArray& data)
{
    MessageType type;
    QVariantMap messageData;
    
    if (!decodeMessage(data, type, messageData)) {
        return false;
    }
    
    emit messageReceived(type, messageData);
    
    // 更新统计信息
    d->stats["messagesReceived"] = d->stats["messagesReceived"].toInt() + 1;
    d->stats["bytesReceived"] = d->stats["bytesReceived"].toLongLong() + data.size();
    
    return true;
}

bool HTTPProtocol::sendMessage(MessageType type, const QVariantMap& data)
{
    if (d->status != IProtocolHandler::Active) {
        qWarning() << "HTTPProtocol: Cannot send message - protocol not active";
        return false;
    }
    
    // 将消息转换为HTTP请求
    QString url = data.value("url", d->baseUrl).toString();
    RequestMethod method = static_cast<RequestMethod>(data.value("method", GET).toInt());
    QByteArray requestData = data.value("data").toByteArray();
    QVariantMap headers = data.value("headers").toMap();
    
    QString requestId = sendRequest(method, url, requestData, headers);
    
    if (!requestId.isEmpty()) {
        emit messageSent(type, data);
        return true;
    }
    
    return false;
}

bool HTTPProtocol::sendHeartbeat()
{
    if (d->baseUrl.isEmpty()) {
        qWarning() << "HTTPProtocol: Cannot send heartbeat - no base URL configured";
        return false;
    }
    
    QVariantMap heartbeatData;
    heartbeatData["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    QString requestId = get(d->baseUrl + "/heartbeat");
    
    if (!requestId.isEmpty()) {
        emit heartbeatSent();
        return true;
    }
    
    return false;
}

bool HTTPProtocol::supportsFeature(const QString& feature) const
{
    QStringList features = supportedFeatures();
    return features.contains(feature);
}

QStringList HTTPProtocol::supportedFeatures() const
{
    return QStringList() 
        << "get-requests" 
        << "post-requests" 
        << "put-requests" 
        << "delete-requests"
        << "custom-headers"
        << "request-timeout"
        << "concurrent-requests"
        << "request-queue"
        << "ssl-support"
        << "redirect-handling";
}

void HTTPProtocol::setParameter(const QString& key, const QVariant& value)
{
    d->parameters[key] = value;
    
    // 处理特殊参数
    if (key == "requestTimeout") {
        d->requestTimeout = value.toInt();
    } else if (key == "maxConcurrentRequests") {
        d->maxConcurrentRequests = value.toInt();
    } else if (key == "baseUrl") {
        d->baseUrl = value.toString();
    }
}

QVariant HTTPProtocol::parameter(const QString& key) const
{
    return d->parameters.value(key);
}

QVariantMap HTTPProtocol::protocolStats() const
{
    QVariantMap stats = d->stats;
    stats["activeRequests"] = d->activeRequests.size();
    stats["queuedRequests"] = d->requestQueue.size();
    stats["baseUrl"] = d->baseUrl;
    stats["requestTimeout"] = d->requestTimeout;
    stats["maxConcurrentRequests"] = d->maxConcurrentRequests;
    stats["uptime"] = QDateTime::currentMSecsSinceEpoch() - stats.value("startTime", 0).toLongLong();
    
    return stats;
}

QString HTTPProtocol::sendRequest(RequestMethod method, const QString& url, 
                                 const QByteArray& data, const QVariantMap& headers)
{
    if (d->status != IProtocolHandler::Active) {
        qWarning() << "HTTPProtocol: Cannot send request - protocol not active";
        return QString();
    }
    
    QString requestId = generateRequestId();
    
    // 检查是否可以立即发送请求
    if (canSendNewRequest()) {
        QNetworkRequest request = createNetworkRequest(method, url, headers);
        QNetworkReply* reply = executeRequest(request, method, data);
        
        if (reply) {
            d->activeRequests[requestId] = reply;
            d->replyToRequestId[reply] = requestId;
            d->currentRequestCount++;
            
            // 设置请求超时
            QTimer* timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(d->requestTimeout);
            connect(timer, &QTimer::timeout, this, &HTTPProtocol::handleRequestTimeout);
            d->requestTimers[requestId] = timer;
            timer->start();
            
            // 连接信号
            connect(reply, &QNetworkReply::finished, this, &HTTPProtocol::handleReplyFinished);
            connect(reply, &QNetworkReply::errorOccurred,
                    this, &HTTPProtocol::handleNetworkError);
            connect(reply, &QNetworkReply::sslErrors, this, &HTTPProtocol::handleSslErrors);
            connect(reply, &QNetworkReply::downloadProgress, this, &HTTPProtocol::handleDownloadProgress);
            connect(reply, &QNetworkReply::uploadProgress, this, &HTTPProtocol::handleUploadProgress);
            
            // 更新统计信息
            d->stats["requestsSent"] = d->stats["requestsSent"].toInt() + 1;
            
            qDebug() << "HTTPProtocol: Sent" << getMethodString(method) << "request to" << url << "with ID" << requestId;
        }
    } else {
        // 将请求加入队列
        QVariantMap requestInfo;
        requestInfo["id"] = requestId;
        requestInfo["method"] = static_cast<int>(method);
        requestInfo["url"] = url;
        requestInfo["data"] = data;
        requestInfo["headers"] = headers;
        
        enqueueRequest(requestInfo);
        qDebug() << "HTTPProtocol: Queued request" << requestId;
    }
    
    return requestId;
}

QString HTTPProtocol::get(const QString& url, const QVariantMap& headers)
{
    return sendRequest(GET, url, QByteArray(), headers);
}

QString HTTPProtocol::post(const QString& url, const QByteArray& data, const QVariantMap& headers)
{
    return sendRequest(POST, url, data, headers);
}

QString HTTPProtocol::put(const QString& url, const QByteArray& data, const QVariantMap& headers)
{
    return sendRequest(PUT, url, data, headers);
}

QString HTTPProtocol::deleteResource(const QString& url, const QVariantMap& headers)
{
    return sendRequest(DELETE, url, QByteArray(), headers);
}

bool HTTPProtocol::cancelRequest(const QString& requestId)
{
    if (!d->activeRequests.contains(requestId)) {
        qWarning() << "HTTPProtocol: Request" << requestId << "not found";
        return false;
    }
    
    QNetworkReply* reply = d->activeRequests[requestId];
    reply->abort();
    
    // 清理资源
    d->activeRequests.remove(requestId);
    d->replyToRequestId.remove(reply);
    d->currentRequestCount--;
    
    if (d->requestTimers.contains(requestId)) {
        d->requestTimers[requestId]->stop();
        d->requestTimers[requestId]->deleteLater();
        d->requestTimers.remove(requestId);
    }
    
    qDebug() << "HTTPProtocol: Cancelled request" << requestId;
    return true;
}

void HTTPProtocol::setBaseUrl(const QString& baseUrl)
{
    d->baseUrl = baseUrl;
}

QString HTTPProtocol::baseUrl() const
{
    return d->baseUrl;
}

void HTTPProtocol::setDefaultHeaders(const QVariantMap& headers)
{
    d->defaultHeaders = headers;
}

QVariantMap HTTPProtocol::defaultHeaders() const
{
    return d->defaultHeaders;
}

void HTTPProtocol::setRequestTimeout(int timeout)
{
    d->requestTimeout = timeout;
}

int HTTPProtocol::requestTimeout() const
{
    return d->requestTimeout;
}

void HTTPProtocol::setMaxConcurrentRequests(int maxConcurrent)
{
    d->maxConcurrentRequests = maxConcurrent;
}

int HTTPProtocol::maxConcurrentRequests() const
{
    return d->maxConcurrentRequests;
}

void HTTPProtocol::reset()
{
    qDebug() << "HTTPProtocol: Resetting protocol";
    
    stop();
    
    d->baseUrl.clear();
    d->defaultHeaders.clear();
    d->parameters.clear();
    d->stats.clear();
    d->requestQueue.clear();
    
    d->requestTimeout = 30000;
    d->maxConcurrentRequests = 10;
    d->currentRequestCount = 0;
    
    d->status = IProtocolHandler::Inactive;
    emit protocolStatusChanged(d->status);
}

void HTTPProtocol::refresh()
{
    qDebug() << "HTTPProtocol: Refreshing protocol";
    
    // 更新统计信息
    d->stats["lastRefresh"] = QDateTime::currentMSecsSinceEpoch();
    emit statsUpdated(protocolStats());
}

void HTTPProtocol::clearAllRequests()
{
    qDebug() << "HTTPProtocol: Clearing all requests";
    
    // 取消所有活动请求
    QStringList requestIds = d->activeRequests.keys();
    for (const QString& requestId : requestIds) {
        cancelRequest(requestId);
    }
    
    // 清空队列
    d->requestQueue.clear();
}

void HTTPProtocol::retryRequest(const QString& requestId)
{
    // 这里可以实现请求重试逻辑
    qDebug() << "HTTPProtocol: Retrying request" << requestId;
}

void HTTPProtocol::handleReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    QString requestId = d->replyToRequestId.value(reply);
    if (requestId.isEmpty()) {
        reply->deleteLater();
        return;
    }
    
    // 读取响应数据
    QByteArray responseData = reply->readAll();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QVariantMap responseHeaders = parseResponseHeaders(reply);
    
    // 清理资源
    d->activeRequests.remove(requestId);
    d->replyToRequestId.remove(reply);
    d->currentRequestCount--;
    
    if (d->requestTimers.contains(requestId)) {
        d->requestTimers[requestId]->stop();
        d->requestTimers[requestId]->deleteLater();
        d->requestTimers.remove(requestId);
    }
    
    // 更新统计信息
    d->stats["requestsCompleted"] = d->stats["requestsCompleted"].toInt() + 1;
    d->stats["bytesDownloaded"] = d->stats["bytesDownloaded"].toLongLong() + responseData.size();
    
    // 发送完成信号
    emit requestCompleted(requestId, statusCode, responseData, responseHeaders);
    
    reply->deleteLater();
    
    qDebug() << "HTTPProtocol: Request" << requestId << "completed with status" << statusCode;
    
    // 处理队列中的下一个请求
    processRequestQueue();
}

void HTTPProtocol::handleNetworkError(QNetworkReply::NetworkError error)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    QString requestId = d->replyToRequestId.value(reply);
    if (requestId.isEmpty()) {
        return;
    }
    
    QString errorString = reply->errorString();
    
    // 更新统计信息
    d->stats["requestsFailed"] = d->stats["requestsFailed"].toInt() + 1;
    
    // 发送错误信号
    emit requestFailed(requestId, errorString);
    emit protocolError(QString("Network error for request %1: %2").arg(requestId, errorString));
    
    qWarning() << "HTTPProtocol: Network error for request" << requestId << ":" << errorString;
}

void HTTPProtocol::handleSslErrors(const QList<QSslError>& errors)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    QString requestId = d->replyToRequestId.value(reply);
    
    QStringList errorStrings;
    for (const QSslError& error : errors) {
        errorStrings << error.errorString();
    }
    
    QString errorMessage = QString("SSL errors for request %1: %2").arg(requestId, errorStrings.join(", "));
    emit protocolError(errorMessage);
    
    qWarning() << "HTTPProtocol:" << errorMessage;
}

void HTTPProtocol::handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    QString requestId = d->replyToRequestId.value(reply);
    if (!requestId.isEmpty()) {
        emit requestProgress(requestId, bytesReceived, bytesTotal);
    }
}

void HTTPProtocol::handleUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    QString requestId = d->replyToRequestId.value(reply);
    if (!requestId.isEmpty()) {
        emit uploadProgress(requestId, bytesSent, bytesTotal);
        
        // 更新统计信息
        d->stats["bytesUploaded"] = d->stats["bytesUploaded"].toLongLong() + bytesSent;
    }
}

void HTTPProtocol::handleRequestTimeout()
{
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if (!timer) {
        return;
    }
    
    // 查找对应的请求ID
    QString requestId;
    for (auto it = d->requestTimers.begin(); it != d->requestTimers.end(); ++it) {
        if (it.value() == timer) {
            requestId = it.key();
            break;
        }
    }
    
    if (!requestId.isEmpty()) {
        emit requestFailed(requestId, "Request timeout");
        cancelRequest(requestId);
        
        qWarning() << "HTTPProtocol: Request" << requestId << "timed out";
    }
}

void HTTPProtocol::processRequestQueue()
{
    while (!d->requestQueue.isEmpty() && canSendNewRequest()) {
        QVariantMap requestInfo = d->requestQueue.dequeue();
        
        QString requestId = requestInfo["id"].toString();
        RequestMethod method = static_cast<RequestMethod>(requestInfo["method"].toInt());
        QString url = requestInfo["url"].toString();
        QByteArray data = requestInfo["data"].toByteArray();
        QVariantMap headers = requestInfo["headers"].toMap();
        
        QNetworkRequest request = createNetworkRequest(method, url, headers);
        QNetworkReply* reply = executeRequest(request, method, data);
        
        if (reply) {
            d->activeRequests[requestId] = reply;
            d->replyToRequestId[reply] = requestId;
            d->currentRequestCount++;
            
            // 设置请求超时
            QTimer* timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(d->requestTimeout);
            connect(timer, &QTimer::timeout, this, &HTTPProtocol::handleRequestTimeout);
            d->requestTimers[requestId] = timer;
            timer->start();
            
            // 连接信号
            connect(reply, &QNetworkReply::finished, this, &HTTPProtocol::handleReplyFinished);
            connect(reply, &QNetworkReply::errorOccurred,
                    this, &HTTPProtocol::handleNetworkError);
            connect(reply, &QNetworkReply::sslErrors, this, &HTTPProtocol::handleSslErrors);
            connect(reply, &QNetworkReply::downloadProgress, this, &HTTPProtocol::handleDownloadProgress);
            connect(reply, &QNetworkReply::uploadProgress, this, &HTTPProtocol::handleUploadProgress);
            
            qDebug() << "HTTPProtocol: Processed queued request" << requestId;
        }
    }
}

QNetworkRequest HTTPProtocol::createNetworkRequest(RequestMethod method, const QString& url, const QVariantMap& headers)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    
    // 应用默认和自定义请求头
    applyHeaders(request, headers);
    
    return request;
}

QNetworkReply* HTTPProtocol::executeRequest(const QNetworkRequest& request, RequestMethod method, const QByteArray& data)
{
    QNetworkReply* reply = nullptr;
    
    switch (method) {
        case GET:
            reply = d->networkManager->get(request);
            break;
        case POST:
            reply = d->networkManager->post(request, data);
            break;
        case PUT:
            reply = d->networkManager->put(request, data);
            break;
        case DELETE:
            reply = d->networkManager->deleteResource(request);
            break;
        case HEAD:
            reply = d->networkManager->head(request);
            break;
        default:
            qWarning() << "HTTPProtocol: Unsupported method" << method;
            break;
    }
    
    return reply;
}

QString HTTPProtocol::generateRequestId()
{
    return QUuid::createUuid().toString().remove('{').remove('}');
}

QString HTTPProtocol::getMethodString(RequestMethod method) const
{
    switch (method) {
        case GET: return "GET";
        case POST: return "POST";
        case PUT: return "PUT";
        case DELETE: return "DELETE";
        case HEAD: return "HEAD";
        case OPTIONS: return "OPTIONS";
        case PATCH: return "PATCH";
        default: return "UNKNOWN";
    }
}

QVariantMap HTTPProtocol::parseResponseHeaders(QNetworkReply* reply)
{
    QVariantMap headers;
    
    const QList<QNetworkReply::RawHeaderPair> rawHeaders = reply->rawHeaderPairs();
    for (const auto& pair : rawHeaders) {
        headers[QString::fromUtf8(pair.first)] = QString::fromUtf8(pair.second);
    }
    
    return headers;
}

bool HTTPProtocol::handleRedirect(QNetworkReply* reply)
{
    QVariant redirectTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (redirectTarget.isValid()) {
        QUrl redirectUrl = redirectTarget.toUrl();
        if (redirectUrl.isRelative()) {
            redirectUrl = reply->url().resolved(redirectUrl);
        }
        
        qDebug() << "HTTPProtocol: Redirect to" << redirectUrl.toString();
        return true;
    }
    
    return false;
}

void HTTPProtocol::applyHeaders(QNetworkRequest& request, const QVariantMap& customHeaders)
{
    // 应用默认请求头
    for (auto it = d->defaultHeaders.begin(); it != d->defaultHeaders.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
    }
    
    // 应用自定义请求头
    for (auto it = customHeaders.begin(); it != customHeaders.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
    }
}

bool HTTPProtocol::canSendNewRequest() const
{
    return d->currentRequestCount < d->maxConcurrentRequests;
}

void HTTPProtocol::enqueueRequest(const QVariantMap& requestInfo)
{
    d->requestQueue.enqueue(requestInfo);
}