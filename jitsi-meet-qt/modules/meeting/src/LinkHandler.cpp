#include "LinkHandler.h"
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

class LinkHandler::Private
{
public:
    QString defaultServer = "meet.jit.si";
    QStringList supportedProtocols = {"https", "jitsi", "meet"};
    int validationTimeout = 10000; // 10秒
    
    QNetworkAccessManager* networkManager = nullptr;
    QTimer* validationTimer = nullptr;
    
    QMap<QString, ValidationResult> validationCache;
    QMap<QString, bool> serverStatusCache;
};

LinkHandler::LinkHandler(QObject* parent)
    : ILinkHandler(parent)
    , d(std::make_unique<Private>())
{
    initializeNetworkManager();
    
    d->validationTimer = new QTimer(this);
    d->validationTimer->setSingleShot(true);
    connect(d->validationTimer, &QTimer::timeout,
            this, &LinkHandler::handleValidationTimeout);
}

LinkHandler::~LinkHandler() = default;

QVariantMap LinkHandler::parseUrl(const QString& url)
{
    QVariantMap result;
    
    if (url.isEmpty()) {
        result["error"] = "Empty URL";
        return result;
    }
    
    LinkType type = getLinkType(url);
    result["type"] = static_cast<int>(type);
    
    switch (type) {
        case HttpsLink:
            result = parseHttpsUrl(url);
            break;
        case JitsiProtocol:
            result = parseJitsiProtocolUrl(url);
            break;
        case CustomProtocol:
            // 处理自定义协议
            break;
        case InvalidLink:
            result["error"] = "Invalid URL format";
            break;
    }
    
    emit urlParsed(url, result);
    return result;
}

LinkHandler::ValidationResult LinkHandler::validateUrl(const QString& url)
{
    if (url.isEmpty()) {
        return InvalidFormat;
    }
    
    // 检查缓存
    ValidationResult cached = getCachedValidationResult(url);
    if (cached != InvalidFormat) {
        return cached;
    }
    
    // 基本格式验证
    if (!isValidUrlFormat(url)) {
        cacheValidationResult(url, InvalidFormat);
        return InvalidFormat;
    }
    
    QUrl qurl(url);
    if (!qurl.isValid()) {
        cacheValidationResult(url, InvalidFormat);
        return InvalidFormat;
    }
    
    // 检查协议支持
    if (!isProtocolSupported(qurl.scheme())) {
        cacheValidationResult(url, InvalidFormat);
        return InvalidFormat;
    }
    
    // 验证服务器
    QString server = extractServer(url);
    if (!validateServer(server)) {
        cacheValidationResult(url, InvalidServer);
        return InvalidServer;
    }
    
    // 验证房间名称
    QString roomName = extractRoomName(url);
    if (!validateRoomName(roomName)) {
        cacheValidationResult(url, InvalidRoom);
        return InvalidRoom;
    }
    
    cacheValidationResult(url, Valid);
    return Valid;
}

QVariantMap LinkHandler::extractParameters(const QString& url)
{
    QUrl qurl(url);
    return parseQueryParameters(qurl);
}

LinkHandler::LinkType LinkHandler::getLinkType(const QString& url)
{
    if (url.startsWith("https://") || url.startsWith("http://")) {
        return HttpsLink;
    } else if (url.startsWith("jitsi://")) {
        return JitsiProtocol;
    } else if (url.contains("://")) {
        return CustomProtocol;
    }
    
    return InvalidLink;
}

QString LinkHandler::buildMeetingUrl(const QString& server, 
                                   const QString& roomName, 
                                   const QVariantMap& parameters)
{
    QString baseUrl = QString("https://%1/%2").arg(server, roomName);
    
    if (!parameters.isEmpty()) {
        QString queryString = buildQueryString(parameters);
        if (!queryString.isEmpty()) {
            baseUrl += "?" + queryString;
        }
    }
    
    return baseUrl;
}

QString LinkHandler::normalizeUrl(const QString& url)
{
    QString normalized = url.trimmed();
    
    // 移除尾部斜杠
    if (normalized.endsWith('/')) {
        normalized.chop(1);
    }
    
    // 确保协议
    if (!normalized.contains("://")) {
        normalized = "https://" + normalized;
    }
    
    return normalized;
}

bool LinkHandler::isServerReachable(const QString& serverUrl)
{
    // 检查缓存
    if (d->serverStatusCache.contains(serverUrl)) {
        return d->serverStatusCache[serverUrl];
    }
    
    // 异步检查服务器可达性
    checkServerAsync(serverUrl);
    
    // 返回默认值
    return true;
}

QVariantMap LinkHandler::getRoomInfo(const QString& roomUrl)
{
    QVariantMap info;
    
    // 解析URL获取基本信息
    QVariantMap parsed = parseUrl(roomUrl);
    info["server"] = parsed.value("server");
    info["roomName"] = parsed.value("roomName");
    info["parameters"] = parsed.value("parameters");
    
    // 异步获取详细信息
    getRoomInfoAsync(roomUrl);
    
    return info;
}

void LinkHandler::setSupportedProtocols(const QStringList& protocols)
{
    d->supportedProtocols = protocols;
}

QStringList LinkHandler::getSupportedProtocols() const
{
    return d->supportedProtocols;
}

void LinkHandler::setDefaultServer(const QString& server)
{
    d->defaultServer = server;
}

QString LinkHandler::defaultServer() const
{
    return d->defaultServer;
}

void LinkHandler::setValidationTimeout(int timeout)
{
    d->validationTimeout = timeout;
}

int LinkHandler::validationTimeout() const
{
    return d->validationTimeout;
}

void LinkHandler::validateUrlAsync(const QString& url)
{
    ValidationResult result = validateUrl(url);
    emit urlValidated(url, result);
}

void LinkHandler::checkServerAsync(const QString& serverUrl)
{
    QUrl url(QString("https://%1").arg(serverUrl));
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Jitsi-Meet-Qt");
    
    QNetworkReply* reply = d->networkManager->head(request);
    reply->setProperty("serverUrl", serverUrl);
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        QString serverUrl = reply->property("serverUrl").toString();
        bool reachable = (reply->error() == QNetworkReply::NoError);
        
        d->serverStatusCache[serverUrl] = reachable;
        emit serverChecked(serverUrl, reachable);
        
        reply->deleteLater();
    });
}

void LinkHandler::getRoomInfoAsync(const QString& roomUrl)
{
    // 实现异步获取房间信息
    Q_UNUSED(roomUrl)
    
    // 这里应该调用实际的API获取房间信息
}

QVariantMap LinkHandler::parseJitsiProtocolUrl(const QString& url)
{
    QVariantMap result;
    
    // 解析 jitsi://server/room?params 格式
    QRegularExpression regex(R"(jitsi://([^/]+)/([^?]+)(?:\?(.+))?)");
    QRegularExpressionMatch match = regex.match(url);
    
    if (match.hasMatch()) {
        result["server"] = match.captured(1);
        result["roomName"] = match.captured(2);
        
        if (!match.captured(3).isEmpty()) {
            QUrlQuery query(match.captured(3));
            QVariantMap params;
            for (const auto& item : query.queryItems()) {
                params[item.first] = item.second;
            }
            result["parameters"] = params;
        }
    }
    
    return result;
}

QVariantMap LinkHandler::parseHttpsUrl(const QString& url)
{
    QVariantMap result;
    QUrl qurl(url);
    
    result["server"] = qurl.host();
    
    QString path = qurl.path();
    if (path.startsWith('/')) {
        path = path.mid(1);
    }
    
    result["roomName"] = path;
    result["parameters"] = parseQueryParameters(qurl);
    
    return result;
}

bool LinkHandler::validateRoomName(const QString& roomName)
{
    if (roomName.isEmpty()) {
        return false;
    }
    
    // 检查房间名称格式
    QRegularExpression regex("^[a-zA-Z0-9._-]+$");
    return regex.match(roomName).hasMatch();
}

bool LinkHandler::validateServer(const QString& server)
{
    if (server.isEmpty()) {
        return false;
    }
    
    // 检查服务器地址格式
    QRegularExpression regex(R"(^[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return regex.match(server).hasMatch();
}

QString LinkHandler::sanitizeUrl(const QString& url)
{
    QString sanitized = url;
    
    // 移除危险字符
    sanitized.remove(QRegularExpression("[<>\"']"));
    
    // 规范化空白字符
    sanitized = sanitized.simplified();
    
    return sanitized;
}

void LinkHandler::clearCache()
{
    d->validationCache.clear();
    d->serverStatusCache.clear();
}

void LinkHandler::refreshServerStatus()
{
    d->serverStatusCache.clear();
}

void LinkHandler::handleNetworkReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // 处理网络回复
    reply->deleteLater();
}

void LinkHandler::handleValidationTimeout()
{
    emit errorOccurred("URL validation timeout");
}

void LinkHandler::initializeNetworkManager()
{
    d->networkManager = new QNetworkAccessManager(this);
    d->networkManager->setTransferTimeout(d->validationTimeout);
}

QVariantMap LinkHandler::parseQueryParameters(const QUrl& url)
{
    QVariantMap params;
    QUrlQuery query(url);
    
    for (const auto& item : query.queryItems()) {
        params[item.first] = item.second;
    }
    
    return params;
}

QString LinkHandler::buildQueryString(const QVariantMap& parameters)
{
    QUrlQuery query;
    
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        query.addQueryItem(it.key(), it.value().toString());
    }
    
    return query.toString();
}

bool LinkHandler::isValidUrlFormat(const QString& url)
{
    QUrl qurl(url);
    return qurl.isValid() && !qurl.host().isEmpty();
}

bool LinkHandler::isProtocolSupported(const QString& protocol)
{
    return d->supportedProtocols.contains(protocol);
}

QString LinkHandler::extractServer(const QString& url)
{
    QUrl qurl(url);
    return qurl.host();
}

QString LinkHandler::extractRoomName(const QString& url)
{
    QUrl qurl(url);
    QString path = qurl.path();
    
    if (path.startsWith('/')) {
        path = path.mid(1);
    }
    
    return path;
}

void LinkHandler::cacheValidationResult(const QString& url, ValidationResult result)
{
    d->validationCache[url] = result;
}

LinkHandler::ValidationResult LinkHandler::getCachedValidationResult(const QString& url)
{
    return d->validationCache.value(url, InvalidFormat);
}