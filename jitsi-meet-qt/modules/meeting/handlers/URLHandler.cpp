#include "URLHandler.h"
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

class URLHandler::Private
{
public:
    QString defaultServer;
    QStringList supportedProtocols;
    QMap<QString, QRegularExpression> customPatterns;
    
    // URL patterns
    QRegularExpression jitsiMeetPattern;
    QRegularExpression jitsiProtocolPattern;
    
    Private() {
        // Initialize default protocols
        supportedProtocols << "jitsi" << "meet" << "conference";
        
        // Initialize URL patterns
        jitsiMeetPattern.setPattern(R"(^https?://([^/]+)/([^/?#]+)(?:\?([^#]*))?(?:#(.*))?$)");
        jitsiProtocolPattern.setPattern(R"(^(jitsi|meet|conference)://([^/]+)/([^/?#]+)(?:\?([^#]*))?(?:#(.*))?$)");
    }
};

URLHandler::URLHandler(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    initializePatterns();
}

URLHandler::~URLHandler() = default;

QVariantMap URLHandler::parseURL(const QString& url)
{
    QVariantMap result;
    
    if (url.isEmpty()) {
        emit errorOccurred("Empty URL provided");
        return result;
    }
    
    URLType type = getURLType(url);
    result["type"] = static_cast<int>(type);
    result["originalUrl"] = url;
    
    switch (type) {
    case JitsiMeetURL:
        result = parseJitsiMeetURL(QUrl(url));
        break;
    case JitsiProtocol:
        result = parseJitsiProtocolURL(url);
        break;
    case JitsiMeetProtocol:
        result = handleDeepLink(url);
        break;
    case PlainRoomName:
        result["valid"] = true;
        result["type"] = static_cast<int>(PlainRoomName);
        result["server"] = d->defaultServer;
        result["roomName"] = url.trimmed();
        result["parameters"] = QVariantMap();
        break;
    case CustomURL:
        result = parseCustomURL(url);
        break;
    default:
        result["valid"] = false;
        result["error"] = "Unsupported URL type";
        break;
    }
    
    emit urlParsed(url, result);
    return result;
}

bool URLHandler::validateURL(const QString& url)
{
    if (url.isEmpty()) {
        return false;
    }
    
    URLType type = getURLType(url);
    bool valid = (type != InvalidURL);
    
    if (valid) {
        // Additional validation based on type
        switch (type) {
        case JitsiMeetURL: {
            QUrl qurl(url);
            valid = qurl.isValid() && !qurl.host().isEmpty();
            break;
        }
        case JitsiProtocol:
            valid = d->jitsiProtocolPattern.match(url).hasMatch();
            break;
        case JitsiMeetProtocol:
            valid = url.startsWith("jitsi-meet://") && url.length() > 13;
            break;
        case PlainRoomName:
            valid = validateRoomName(url.trimmed());
            break;
        case CustomURL:
            // Check against custom patterns
            for (const auto& pattern : d->customPatterns) {
                if (pattern.match(url).hasMatch()) {
                    valid = true;
                    break;
                }
            }
            break;
        default:
            valid = false;
            break;
        }
    }
    
    emit urlValidated(url, valid);
    return valid;
}

URLHandler::URLType URLHandler::getURLType(const QString& url)
{
    if (url.isEmpty()) {
        return InvalidURL;
    }
    
    // Check for jitsi-meet:// protocol URLs first
    if (url.startsWith("jitsi-meet://")) {
        return JitsiMeetProtocol;
    }
    
    // Check for other protocol URLs
    if (d->jitsiProtocolPattern.match(url).hasMatch()) {
        return JitsiProtocol;
    }
    
    // Check for HTTPS URLs
    QUrl qurl(url);
    if (qurl.scheme() == "https" || qurl.scheme() == "http") {
        return JitsiMeetURL;
    }
    
    // Check if it's a plain room name (no protocol, no dots)
    if (!url.contains("://") && !url.contains(".") && !url.contains("/")) {
        return PlainRoomName;
    }
    
    // Check custom patterns
    for (const auto& pattern : d->customPatterns) {
        if (pattern.match(url).hasMatch()) {
            return CustomURL;
        }
    }
    
    return InvalidURL;
}

QString URLHandler::normalizeURL(const QString& url)
{
    if (url.isEmpty()) {
        return QString();
    }
    
    QUrl qurl(url);
    
    // Ensure HTTPS scheme for web URLs
    if (qurl.scheme() == "http") {
        qurl.setScheme("https");
    }
    
    // Remove default ports
    if ((qurl.scheme() == "https" && qurl.port() == 443) ||
        (qurl.scheme() == "http" && qurl.port() == 80)) {
        qurl.setPort(-1);
    }
    
    // Normalize path
    QString path = qurl.path();
    if (path.endsWith('/') && path.length() > 1) {
        path.chop(1);
        qurl.setPath(path);
    }
    
    return qurl.toString();
}

QString URLHandler::extractServer(const QString& url)
{
    QUrl qurl(url);
    if (qurl.isValid()) {
        return qurl.host();
    }
    
    // Try protocol URL pattern
    QRegularExpressionMatch match = d->jitsiProtocolPattern.match(url);
    if (match.hasMatch()) {
        return match.captured(2);
    }
    
    return QString();
}

QString URLHandler::extractRoomName(const QString& url)
{
    QUrl qurl(url);
    if (qurl.isValid()) {
        QString path = qurl.path();
        if (path.startsWith('/')) {
            path = path.mid(1);
        }
        
        // Extract first path segment as room name
        int slashIndex = path.indexOf('/');
        if (slashIndex > 0) {
            return path.left(slashIndex);
        }
        return path;
    }
    
    // Try protocol URL pattern
    QRegularExpressionMatch match = d->jitsiProtocolPattern.match(url);
    if (match.hasMatch()) {
        return match.captured(3);
    }
    
    return QString();
}

QVariantMap URLHandler::extractParameters(const QString& url)
{
    QUrl qurl(url);
    if (qurl.isValid()) {
        return parseQueryParameters(qurl);
    }
    
    // Try protocol URL pattern
    QRegularExpressionMatch match = d->jitsiProtocolPattern.match(url);
    if (match.hasMatch() && match.capturedLength(4) > 0) {
        QUrlQuery query(match.captured(4));
        QVariantMap params;
        for (const auto& item : query.queryItems()) {
            params[item.first] = item.second;
        }
        return params;
    }
    
    return QVariantMap();
}

QString URLHandler::buildMeetingURL(const QString& server, const QString& roomName, const QVariantMap& parameters)
{
    if (server.isEmpty() || roomName.isEmpty()) {
        return QString();
    }
    
    QUrl url;
    url.setScheme("https");
    url.setHost(server);
    url.setPath("/" + roomName);
    
    if (!parameters.isEmpty()) {
        QUrlQuery query;
        for (auto it = parameters.begin(); it != parameters.end(); ++it) {
            query.addQueryItem(it.key(), it.value().toString());
        }
        url.setQuery(query);
    }
    
    return url.toString();
}

QString URLHandler::convertProtocolToHttps(const QString& protocolUrl)
{
    QRegularExpressionMatch match = d->jitsiProtocolPattern.match(protocolUrl);
    if (!match.hasMatch()) {
        return QString();
    }
    
    QString server = match.captured(2);
    QString roomName = match.captured(3);
    QString queryString = match.captured(4);
    
    QVariantMap params;
    if (!queryString.isEmpty()) {
        QUrlQuery query(queryString);
        for (const auto& item : query.queryItems()) {
            params[item.first] = item.second;
        }
    }
    
    return buildMeetingURL(server, roomName, params);
}

QString URLHandler::convertHttpsToProtocol(const QString& httpsUrl)
{
    QUrl qurl(httpsUrl);
    if (!qurl.isValid() || (qurl.scheme() != "https" && qurl.scheme() != "http")) {
        return QString();
    }
    
    QString server = qurl.host();
    QString roomName = extractRoomName(httpsUrl);
    QVariantMap params = extractParameters(httpsUrl);
    
    QString protocolUrl = QString("jitsi://%1/%2").arg(server, roomName);
    
    if (!params.isEmpty()) {
        protocolUrl += "?" + buildQueryString(params);
    }
    
    return protocolUrl;
}

QString URLHandler::sanitizeURL(const QString& url)
{
    QString sanitized = url.trimmed();
    
    // Remove potentially dangerous characters
    sanitized.remove(QRegularExpression("[<>\"'`]"));
    
    // Normalize whitespace
    sanitized.replace(QRegularExpression("\\s+"), " ");
    
    return sanitized;
}

bool URLHandler::validateRoomName(const QString& roomName)
{
    if (roomName.isEmpty() || roomName.length() > 100) {
        return false;
    }
    
    // Check for valid characters (alphanumeric, dash, underscore)
    QRegularExpression validPattern("^[a-zA-Z0-9_-]+$");
    return validPattern.match(roomName).hasMatch();
}

bool URLHandler::validateServer(const QString& server)
{
    if (server.isEmpty()) {
        return false;
    }
    
    // Basic domain validation
    QRegularExpression domainPattern(R"(^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$)");
    return domainPattern.match(server).hasMatch();
}

void URLHandler::setDefaultServer(const QString& server)
{
    d->defaultServer = server;
}

QString URLHandler::defaultServer() const
{
    return d->defaultServer;
}

void URLHandler::setSupportedProtocols(const QStringList& protocols)
{
    d->supportedProtocols = protocols;
}

QStringList URLHandler::supportedProtocols() const
{
    return d->supportedProtocols;
}

void URLHandler::addCustomPattern(const QString& pattern, const QString& name)
{
    QRegularExpression regex(pattern);
    if (regex.isValid()) {
        d->customPatterns[name] = regex;
    }
}

void URLHandler::removeCustomPattern(const QString& name)
{
    d->customPatterns.remove(name);
}

QString URLHandler::getURLSummary(const QString& url)
{
    QVariantMap parsed = parseURL(url);
    
    if (!parsed.value("valid", false).toBool()) {
        return "Invalid URL";
    }
    
    QString server = parsed.value("server").toString();
    QString roomName = parsed.value("roomName").toString();
    
    return QString("Meeting: %1 on %2").arg(roomName, server);
}

void URLHandler::initializePatterns()
{
    // Initialize built-in patterns
    d->jitsiMeetPattern.setPattern(R"(^https?://([^/]+)/([^/?#]+)(?:\?([^#]*))?(?:#(.*))?$)");
    d->jitsiProtocolPattern.setPattern(R"(^(jitsi|meet|conference)://([^/]+)/([^/?#]+)(?:\?([^#]*))?(?:#(.*))?$)");
    
    // Add jitsi-meet:// protocol pattern
    QRegularExpression jitsiMeetProtocolPattern(R"(^jitsi-meet://([^/]+)/([^/?#]+)(?:\?([^#]*))?(?:#(.*))?$)");
    d->customPatterns["jitsi-meet-protocol"] = jitsiMeetProtocolPattern;
}

QVariantMap URLHandler::parseJitsiMeetURL(const QUrl& url)
{
    QVariantMap result;
    result["valid"] = true;
    result["type"] = static_cast<int>(JitsiMeetURL);
    result["server"] = url.host();
    result["port"] = url.port(-1);
    result["scheme"] = url.scheme();
    
    QString path = url.path();
    if (path.startsWith('/')) {
        path = path.mid(1);
    }
    
    // Extract room name (first path segment)
    int slashIndex = path.indexOf('/');
    if (slashIndex > 0) {
        result["roomName"] = path.left(slashIndex);
        result["subPath"] = path.mid(slashIndex + 1);
    } else {
        result["roomName"] = path;
    }
    
    // Parse query parameters
    result["parameters"] = parseQueryParameters(url);
    
    // Parse fragment
    if (url.hasFragment()) {
        result["fragment"] = url.fragment();
    }
    
    return result;
}

QVariantMap URLHandler::parseJitsiProtocolURL(const QString& url)
{
    QVariantMap result;
    QRegularExpressionMatch match = d->jitsiProtocolPattern.match(url);
    
    if (!match.hasMatch()) {
        result["valid"] = false;
        result["error"] = "Invalid protocol URL format";
        return result;
    }
    
    result["valid"] = true;
    result["type"] = static_cast<int>(JitsiProtocol);
    result["protocol"] = match.captured(1);
    result["server"] = match.captured(2);
    result["roomName"] = match.captured(3);
    
    // Parse query parameters
    if (match.capturedLength(4) > 0) {
        QUrlQuery query(match.captured(4));
        QVariantMap params;
        for (const auto& item : query.queryItems()) {
            params[item.first] = item.second;
        }
        result["parameters"] = params;
    }
    
    // Parse fragment
    if (match.capturedLength(5) > 0) {
        result["fragment"] = match.captured(5);
    }
    
    return result;
}

QVariantMap URLHandler::parseCustomURL(const QString& url)
{
    QVariantMap result;
    result["valid"] = false;
    
    // Try each custom pattern
    for (auto it = d->customPatterns.begin(); it != d->customPatterns.end(); ++it) {
        QRegularExpressionMatch match = it.value().match(url);
        if (match.hasMatch()) {
            result["valid"] = true;
            result["type"] = static_cast<int>(CustomURL);
            result["patternName"] = it.key();
            
            // Extract captured groups
            QVariantList captures;
            for (int i = 1; i < match.lastCapturedIndex() + 1; ++i) {
                captures.append(match.captured(i));
            }
            result["captures"] = captures;
            break;
        }
    }
    
    if (!result.value("valid").toBool()) {
        result["error"] = "No matching custom pattern found";
    }
    
    return result;
}

QVariantMap URLHandler::parseQueryParameters(const QUrl& url)
{
    QVariantMap params;
    QUrlQuery query(url);
    
    for (const auto& item : query.queryItems()) {
        params[item.first] = item.second;
    }
    
    return params;
}

QString URLHandler::buildQueryString(const QVariantMap& parameters)
{
    QUrlQuery query;
    
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        query.addQueryItem(it.key(), it.value().toString());
    }
    
    return query.toString();
}

QVariantMap URLHandler::matchPattern(const QString& url, const QRegularExpression& pattern)
{
    QVariantMap result;
    QRegularExpressionMatch match = pattern.match(url);
    
    result["hasMatch"] = match.hasMatch();
    
    if (match.hasMatch()) {
        QVariantList captures;
        for (int i = 0; i <= match.lastCapturedIndex(); ++i) {
            captures.append(match.captured(i));
        }
        result["captures"] = captures;
    }
    
    return result;
}

QVariantMap URLHandler::parseFragmentConfig(const QString& fragment)
{
    QVariantMap config;
    
    if (fragment.isEmpty()) {
        return config;
    }
    
    // 解析类似 "config.p2p.enabled=false&config.startWithAudioMuted=true" 的片段
    QStringList pairs = fragment.split('&');
    
    for (const QString& pair : pairs) {
        QStringList keyValue = pair.split('=');
        if (keyValue.size() == 2) {
            QString key = keyValue[0].trimmed();
            QString value = keyValue[1].trimmed();
            
            // 处理嵌套配置（如 config.p2p.enabled）
            QStringList keyParts = key.split('.');
            QVariantMap* currentMap = &config;
            
            for (int i = 0; i < keyParts.size() - 1; ++i) {
                QString part = keyParts[i];
                if (!currentMap->contains(part)) {
                    currentMap->insert(part, QVariantMap());
                }
                QVariant& variant = (*currentMap)[part];
                currentMap = reinterpret_cast<QVariantMap*>(&variant);
            }
            
            // 设置最终值
            QString finalKey = keyParts.last();
            if (value.toLower() == "true") {
                currentMap->insert(finalKey, true);
            } else if (value.toLower() == "false") {
                currentMap->insert(finalKey, false);
            } else {
                bool ok;
                int intValue = value.toInt(&ok);
                if (ok) {
                    currentMap->insert(finalKey, intValue);
                } else {
                    currentMap->insert(finalKey, value);
                }
            }
        }
    }
    
    return config;
}

QVariantMap URLHandler::handleDeepLink(const QString& url)
{
    QVariantMap result;
    
    if (!url.startsWith("jitsi-meet://")) {
        result["valid"] = false;
        result["error"] = "Not a jitsi-meet:// deep link";
        return result;
    }
    
    // 移除协议前缀
    QString cleanUrl = url.mid(13); // 移除 "jitsi-meet://"
    
    // 解析格式: server/room?params#config
    QRegularExpression deepLinkPattern(R"(^([^/]+)/([^?#]+)(?:\?([^#]*))?(?:#(.*))?$)");
    QRegularExpressionMatch match = deepLinkPattern.match(cleanUrl);
    
    if (!match.hasMatch()) {
        result["valid"] = false;
        result["error"] = "Invalid deep link format";
        return result;
    }
    
    result["valid"] = true;
    result["type"] = static_cast<int>(JitsiMeetProtocol);
    result["server"] = match.captured(1);
    result["roomName"] = match.captured(2);
    
    // 解析查询参数
    if (match.capturedLength(3) > 0) {
        QUrlQuery query(match.captured(3));
        QVariantMap params;
        for (const auto& item : query.queryItems()) {
            params[item.first] = item.second;
        }
        result["parameters"] = params;
    }
    
    // 解析片段配置
    if (match.capturedLength(4) > 0) {
        result["config"] = parseFragmentConfig(match.captured(4));
    }
    
    return result;
}

bool URLHandler::isSupportedFormat(const QString& url)
{
    URLType type = getURLType(url);
    return type != InvalidURL;
}