#include "ProtocolHandler.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#ifdef Q_OS_WIN
#include <QSettings>
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <QProcess>
#include <QTextStream>
#elif defined(Q_OS_MACOS)
#include <QProcess>
#endif

class ProtocolHandler::Private
{
public:
    QString defaultProtocol;
    bool protocolHandlingEnabled;
    QMap<QString, QString> protocolDescriptions;
    QMap<QString, RegistrationStatus> registrationStatus;
    
    Private() 
        : defaultProtocol("jitsi")
        , protocolHandlingEnabled(true)
    {
        // Initialize supported protocols
        protocolDescriptions["jitsi"] = "Jitsi Meet Protocol";
        protocolDescriptions["meet"] = "Generic Meeting Protocol";
        protocolDescriptions["conference"] = "Conference Protocol";
        
        // Initialize registration status
        for (const QString& protocol : protocolDescriptions.keys()) {
            registrationStatus[protocol] = NotRegistered;
        }
    }
};

ProtocolHandler::ProtocolHandler(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    initializeSupportedProtocols();
}

ProtocolHandler::~ProtocolHandler() = default;

bool ProtocolHandler::registerProtocol(const QString& protocol, const QString& applicationPath)
{
    if (protocol.isEmpty()) {
        emit errorOccurred("Empty protocol name");
        return false;
    }
    
    QString appPath = applicationPath;
    if (appPath.isEmpty()) {
        appPath = QApplication::applicationFilePath();
    }
    
    bool success = false;
    
#ifdef Q_OS_WIN
    success = registerProtocolWindows(protocol, appPath);
#elif defined(Q_OS_LINUX)
    success = registerProtocolLinux(protocol, appPath);
#elif defined(Q_OS_MACOS)
    success = registerProtocolMacOS(protocol, appPath);
#endif
    
    if (success) {
        d->registrationStatus[protocol] = Registered;
        emit registrationStatusChanged(protocol, Registered);
    } else {
        d->registrationStatus[protocol] = RegistrationFailed;
        emit registrationStatusChanged(protocol, RegistrationFailed);
        emit errorOccurred(QString("Failed to register protocol: %1").arg(protocol));
    }
    
    return success;
}

bool ProtocolHandler::unregisterProtocol(const QString& protocol)
{
    if (protocol.isEmpty()) {
        return false;
    }
    
    bool success = false;
    
#ifdef Q_OS_WIN
    QSettings registry(QString("HKEY_CLASSES_ROOT\\%1").arg(protocol), QSettings::NativeFormat);
    registry.clear();
    success = true;
#elif defined(Q_OS_LINUX)
    // Remove desktop file
    QString desktopFile = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) 
                         + QString("/jitsi-meet-qt-%1.desktop").arg(protocol);
    success = QFile::remove(desktopFile);
#elif defined(Q_OS_MACOS)
    // macOS protocol unregistration would require modifying Info.plist
    success = true;
#endif
    
    if (success) {
        d->registrationStatus[protocol] = NotRegistered;
        emit registrationStatusChanged(protocol, NotRegistered);
    }
    
    return success;
}

bool ProtocolHandler::isProtocolRegistered(const QString& protocol)
{
    return getRegistrationStatus(protocol) == Registered;
}

ProtocolHandler::RegistrationStatus ProtocolHandler::getRegistrationStatus(const QString& protocol)
{
    if (!d->registrationStatus.contains(protocol)) {
        return NotRegistered;
    }
    
    RegistrationStatus status = NotRegistered;
    
#ifdef Q_OS_WIN
    status = checkRegistrationWindows(protocol);
#elif defined(Q_OS_LINUX)
    status = checkRegistrationLinux(protocol);
#elif defined(Q_OS_MACOS)
    status = checkRegistrationMacOS(protocol);
#endif
    
    d->registrationStatus[protocol] = status;
    return status;
}

QVariantMap ProtocolHandler::handleProtocolCall(const QString& protocolUrl)
{
    QVariantMap result;
    
    if (!d->protocolHandlingEnabled) {
        result["success"] = false;
        result["error"] = "Protocol handling is disabled";
        return result;
    }
    
    QVariantMap parsed = parseProtocolUrl(protocolUrl);
    if (!parsed.value("valid", false).toBool()) {
        result["success"] = false;
        result["error"] = "Invalid protocol URL";
        return result;
    }
    
    emit protocolCalled(protocolUrl, parsed);
    
    result["success"] = true;
    result["parsed"] = parsed;
    
    emit protocolHandled(protocolUrl, true);
    return result;
}

QVariantMap ProtocolHandler::parseProtocolUrl(const QString& protocolUrl)
{
    QVariantMap result;
    
    if (protocolUrl.isEmpty()) {
        result["valid"] = false;
        result["error"] = "Empty protocol URL";
        return result;
    }
    
    // General protocol URL pattern: protocol://server/room?params#fragment
    QRegularExpression pattern(R"(^([a-zA-Z][a-zA-Z0-9+.-]*):\/\/([^\/\?#]+)\/([^\/\?#]+)(?:\?([^#]*))?(?:#(.*))?$)");
    QRegularExpressionMatch match = pattern.match(protocolUrl);
    
    if (!match.hasMatch()) {
        result["valid"] = false;
        result["error"] = "Invalid protocol URL format";
        return result;
    }
    
    QString protocol = match.captured(1).toLower();
    QString server = match.captured(2);
    QString room = match.captured(3);
    QString queryString = match.captured(4);
    QString fragment = match.captured(5);
    
    result["valid"] = true;
    result["protocol"] = protocol;
    result["server"] = server;
    result["room"] = room;
    result["originalUrl"] = protocolUrl;
    
    // Parse query parameters
    if (!queryString.isEmpty()) {
        QUrlQuery query(queryString);
        QVariantMap params;
        for (const auto& item : query.queryItems()) {
            params[item.first] = item.second;
        }
        result["parameters"] = params;
    }
    
    // Add fragment if present
    if (!fragment.isEmpty()) {
        result["fragment"] = fragment;
    }
    
    // Protocol-specific parsing
    if (protocol == "jitsi") {
        QVariantMap jitsiData = parseJitsiProtocol(protocolUrl);
        result["jitsiData"] = jitsiData;
    } else if (protocol == "jitsi-meet") {
        QVariantMap jitsiMeetData = parseJitsiMeetProtocol(protocolUrl);
        result["jitsiMeetData"] = jitsiMeetData;
    } else if (protocol == "meet") {
        QVariantMap meetData = parseMeetProtocol(protocolUrl);
        result["meetData"] = meetData;
    } else if (protocol == "conference") {
        QVariantMap confData = parseConferenceProtocol(protocolUrl);
        result["conferenceData"] = confData;
    }
    
    return result;
}

bool ProtocolHandler::validateProtocolUrl(const QString& protocolUrl)
{
    QVariantMap parsed = parseProtocolUrl(protocolUrl);
    return parsed.value("valid", false).toBool();
}

QStringList ProtocolHandler::getSupportedProtocols() const
{
    return d->protocolDescriptions.keys();
}

bool ProtocolHandler::addCustomProtocol(const QString& protocol, const QString& description)
{
    if (protocol.isEmpty() || d->protocolDescriptions.contains(protocol)) {
        return false;
    }
    
    d->protocolDescriptions[protocol] = description;
    d->registrationStatus[protocol] = NotRegistered;
    
    return true;
}

bool ProtocolHandler::removeCustomProtocol(const QString& protocol)
{
    if (!d->protocolDescriptions.contains(protocol)) {
        return false;
    }
    
    // Unregister first
    unregisterProtocol(protocol);
    
    d->protocolDescriptions.remove(protocol);
    d->registrationStatus.remove(protocol);
    
    return true;
}

void ProtocolHandler::setDefaultProtocol(const QString& protocol)
{
    if (d->protocolDescriptions.contains(protocol)) {
        d->defaultProtocol = protocol;
    }
}

QString ProtocolHandler::defaultProtocol() const
{
    return d->defaultProtocol;
}

QString ProtocolHandler::buildProtocolUrl(const QString& protocol, const QString& server, 
                                        const QString& roomName, const QVariantMap& parameters)
{
    if (protocol.isEmpty() || server.isEmpty() || roomName.isEmpty()) {
        return QString();
    }
    
    QString url = QString("%1://%2/%3").arg(protocol, server, roomName);
    
    if (!parameters.isEmpty()) {
        QUrlQuery query;
        for (auto it = parameters.begin(); it != parameters.end(); ++it) {
            query.addQueryItem(it.key(), it.value().toString());
        }
        url += "?" + query.toString();
    }
    
    return url;
}

QString ProtocolHandler::convertToStandardUrl(const QString& protocolUrl)
{
    QVariantMap parsed = parseProtocolUrl(protocolUrl);
    if (!parsed.value("valid", false).toBool()) {
        return QString();
    }
    
    QString server = parsed.value("server").toString();
    QString room = parsed.value("room").toString();
    QVariantMap params = parsed.value("parameters").toMap();
    
    QString standardUrl = QString("https://%1/%2").arg(server, room);
    
    if (!params.isEmpty()) {
        QUrlQuery query;
        for (auto it = params.begin(); it != params.end(); ++it) {
            query.addQueryItem(it.key(), it.value().toString());
        }
        standardUrl += "?" + query.toString();
    }
    
    return standardUrl;
}

QString ProtocolHandler::convertToProtocolUrl(const QString& standardUrl, const QString& protocol)
{
    QUrl url(standardUrl);
    if (!url.isValid()) {
        return QString();
    }
    
    QString server = url.host();
    QString path = url.path();
    if (path.startsWith('/')) {
        path = path.mid(1);
    }
    
    // Extract room name (first path segment)
    QString room = path;
    int slashIndex = path.indexOf('/');
    if (slashIndex > 0) {
        room = path.left(slashIndex);
    }
    
    QVariantMap params;
    QUrlQuery query(url);
    for (const auto& item : query.queryItems()) {
        params[item.first] = item.second;
    }
    
    return buildProtocolUrl(protocol, server, room, params);
}

QVariantMap ProtocolHandler::getProtocolInfo(const QString& protocol)
{
    QVariantMap info;
    
    if (!d->protocolDescriptions.contains(protocol)) {
        return info;
    }
    
    info["name"] = protocol;
    info["description"] = d->protocolDescriptions[protocol];
    info["registrationStatus"] = static_cast<int>(getRegistrationStatus(protocol));
    info["isDefault"] = (protocol == d->defaultProtocol);
    info["isDefaultHandler"] = isDefaultHandler(protocol);
    
    return info;
}

bool ProtocolHandler::setAsDefaultHandler(const QString& protocol)
{
    // This would require platform-specific implementation
    // For now, just update our internal state
    setDefaultProtocol(protocol);
    return true;
}

bool ProtocolHandler::isDefaultHandler(const QString& protocol)
{
    // Platform-specific check would be needed
    // For now, check if it's our default protocol
    return (protocol == d->defaultProtocol);
}

void ProtocolHandler::setProtocolHandlingEnabled(bool enabled)
{
    d->protocolHandlingEnabled = enabled;
}

bool ProtocolHandler::isProtocolHandlingEnabled() const
{
    return d->protocolHandlingEnabled;
}

void ProtocolHandler::refreshRegistrationStatus()
{
    for (const QString& protocol : d->protocolDescriptions.keys()) {
        RegistrationStatus oldStatus = d->registrationStatus[protocol];
        RegistrationStatus newStatus = getRegistrationStatus(protocol);
        
        if (oldStatus != newStatus) {
            emit registrationStatusChanged(protocol, newStatus);
        }
    }
}

void ProtocolHandler::reregisterAllProtocols()
{
    QString appPath = QApplication::applicationFilePath();
    
    for (const QString& protocol : d->protocolDescriptions.keys()) {
        registerProtocol(protocol, appPath);
    }
}

void ProtocolHandler::initializeSupportedProtocols()
{
    // Initialize default protocols
    d->protocolDescriptions.clear();
    d->protocolDescriptions["jitsi"] = "Jitsi Meet Protocol";
    d->protocolDescriptions["jitsi-meet"] = "Jitsi Meet Deep Link Protocol";
    d->protocolDescriptions["meet"] = "Generic Meeting Protocol";
    d->protocolDescriptions["conference"] = "Conference Protocol";
    
    // Check current registration status
    refreshRegistrationStatus();
}

#ifdef Q_OS_WIN
bool ProtocolHandler::registerProtocolWindows(const QString& protocol, const QString& applicationPath)
{
    try {
        QSettings registry(QString("HKEY_CLASSES_ROOT\\%1").arg(protocol), QSettings::NativeFormat);
        registry.setValue("Default", QString("%1 Protocol").arg(protocol));
        registry.setValue("URL Protocol", "");
        
        registry.beginGroup("DefaultIcon");
        registry.setValue("Default", QString("\"%1\",0").arg(applicationPath));
        registry.endGroup();
        
        registry.beginGroup("shell");
        registry.beginGroup("open");
        registry.beginGroup("command");
        registry.setValue("Default", QString("\"%1\" \"%2\"").arg(applicationPath, "%1"));
        registry.endGroup();
        registry.endGroup();
        registry.endGroup();
        
        return true;
    } catch (...) {
        return false;
    }
}

ProtocolHandler::RegistrationStatus ProtocolHandler::checkRegistrationWindows(const QString& protocol)
{
    QSettings registry(QString("HKEY_CLASSES_ROOT\\%1").arg(protocol), QSettings::NativeFormat);
    
    if (!registry.contains("URL Protocol")) {
        return NotRegistered;
    }
    
    QString command = registry.value("shell/open/command/Default").toString();
    QString currentApp = QApplication::applicationFilePath();
    
    if (command.contains(currentApp)) {
        return Registered;
    }
    
    return RegistrationFailed;
}
#endif

#ifdef Q_OS_LINUX
bool ProtocolHandler::registerProtocolLinux(const QString& protocol, const QString& applicationPath)
{
    QString desktopFile = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) 
                         + QString("/jitsi-meet-qt-%1.desktop").arg(protocol);
    
    QDir().mkpath(QFileInfo(desktopFile).absolutePath());
    
    QFile file(desktopFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Type=Application\n";
    out << QString("Name=Jitsi Meet Qt (%1)\n").arg(protocol);
    out << QString("Exec=%1 %u\n").arg(applicationPath);
    out << "NoDisplay=true\n";
    out << "StartupNotify=true\n";
    out << QString("MimeType=x-scheme-handler/%1;\n").arg(protocol);
    
    file.close();
    
    // Update MIME database
    QProcess::execute("update-desktop-database", 
                     QStringList() << QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation));
    
    return true;
}

ProtocolHandler::RegistrationStatus ProtocolHandler::checkRegistrationLinux(const QString& protocol)
{
    QString desktopFile = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) 
                         + QString("/jitsi-meet-qt-%1.desktop").arg(protocol);
    
    if (!QFile::exists(desktopFile)) {
        return NotRegistered;
    }
    
    return Registered;
}
#endif

#ifdef Q_OS_MACOS
bool ProtocolHandler::registerProtocolMacOS(const QString& protocol, const QString& applicationPath)
{
    // macOS protocol registration requires modifying the application's Info.plist
    // This is typically done at build time, not runtime
    Q_UNUSED(protocol)
    Q_UNUSED(applicationPath)
    return true;
}

ProtocolHandler::RegistrationStatus ProtocolHandler::checkRegistrationMacOS(const QString& protocol)
{
    Q_UNUSED(protocol)
    // Would need to check LSHandlers or similar
    return Registered;
}
#endif

QVariantMap ProtocolHandler::parseJitsiProtocol(const QString& url)
{
    QVariantMap result;
    
    QRegularExpression pattern(R"(^jitsi:\/\/([^\/\?#]+)\/([^\/\?#]+)(?:\?([^#]*))?(?:#(.*))?$)");
    QRegularExpressionMatch match = pattern.match(url);
    
    if (!match.hasMatch()) {
        result["valid"] = false;
        return result;
    }
    
    result["valid"] = true;
    result["server"] = match.captured(1);
    result["room"] = match.captured(2);
    
    // Parse Jitsi-specific parameters
    if (match.capturedLength(3) > 0) {
        QUrlQuery query(match.captured(3));
        QVariantMap params;
        
        for (const auto& item : query.queryItems()) {
            QString key = item.first;
            QString value = item.second;
            
            // Handle Jitsi-specific parameters
            if (key == "jwt") {
                params["authToken"] = value;
            } else if (key == "config.startWithAudioMuted") {
                params["audioMuted"] = (value == "true");
            } else if (key == "config.startWithVideoMuted") {
                params["videoMuted"] = (value == "true");
            } else {
                params[key] = value;
            }
        }
        
        result["parameters"] = params;
    }
    
    return result;
}

QVariantMap ProtocolHandler::parseJitsiMeetProtocol(const QString& url)
{
    QVariantMap result;
    
    // 支持多种jitsi-meet://协议格式
    // 格式1: jitsi-meet://meet.jit.si/roomname
    // 格式2: jitsi-meet://roomname (默认服务器)
    // 格式3: jitsi-meet://server/roomname?params#config
    
    QRegularExpression pattern(R"(^jitsi-meet:\/\/([^\/\?#]+)(?:\/([^\/\?#]+))?(?:\?([^#]*))?(?:#(.*))?$)");
    QRegularExpressionMatch match = pattern.match(url);
    
    if (!match.hasMatch()) {
        result["valid"] = false;
        result["error"] = "Invalid jitsi-meet protocol URL format";
        return result;
    }
    
    result["valid"] = true;
    
    QString firstPart = match.captured(1);
    QString secondPart = match.captured(2);
    QString queryString = match.captured(3);
    QString fragment = match.captured(4);
    
    // 判断URL格式
    if (secondPart.isEmpty()) {
        // 格式: jitsi-meet://roomname 或 jitsi-meet://server
        if (firstPart.contains('.')) {
            // 包含点号，可能是服务器地址
            result["server"] = firstPart;
            result["room"] = "";
        } else {
            // 纯房间名，使用默认服务器
            result["server"] = "meet.jit.si";
            result["room"] = firstPart;
        }
    } else {
        // 格式: jitsi-meet://server/roomname
        result["server"] = firstPart;
        result["room"] = secondPart;
    }
    
    // 解析查询参数
    if (!queryString.isEmpty()) {
        QUrlQuery query(queryString);
        QVariantMap params;
        
        for (const auto& item : query.queryItems()) {
            QString key = item.first;
            QString value = item.second;
            
            // 处理Jitsi Meet特定参数
            if (key == "jwt") {
                params["authToken"] = value;
            } else if (key.startsWith("config.")) {
                // 配置参数
                QString configKey = key.mid(7); // 移除"config."前缀
                if (configKey == "startWithAudioMuted") {
                    params["audioMuted"] = (value.toLower() == "true");
                } else if (configKey == "startWithVideoMuted") {
                    params["videoMuted"] = (value.toLower() == "true");
                } else if (configKey == "prejoinPageEnabled") {
                    params["prejoinEnabled"] = (value.toLower() == "true");
                } else if (configKey == "requireDisplayName") {
                    params["requireDisplayName"] = (value.toLower() == "true");
                } else {
                    params[configKey] = value;
                }
            } else if (key.startsWith("interfaceConfig.")) {
                // 界面配置参数
                QString interfaceKey = key.mid(16); // 移除"interfaceConfig."前缀
                params[interfaceKey] = value;
            } else {
                params[key] = value;
            }
        }
        
        result["parameters"] = params;
    }
    
    // 解析片段配置
    if (!fragment.isEmpty()) {
        QVariantMap fragmentConfig;
        
        // 片段可能包含JSON配置或简单的键值对
        if (fragment.startsWith("{") && fragment.endsWith("}")) {
            // JSON格式配置
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(fragment.toUtf8(), &error);
            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                for (auto it = obj.begin(); it != obj.end(); ++it) {
                    fragmentConfig[it.key()] = it.value().toVariant();
                }
            }
        } else {
            // 简单键值对格式: key1=value1&key2=value2
            QUrlQuery fragmentQuery(fragment);
            for (const auto& item : fragmentQuery.queryItems()) {
                fragmentConfig[item.first] = item.second;
            }
        }
        
        if (!fragmentConfig.isEmpty()) {
            result["fragmentConfig"] = fragmentConfig;
        }
    }
    
    // 构建标准URL
    QString server = result["server"].toString();
    QString room = result["room"].toString();
    if (!server.isEmpty() && !room.isEmpty()) {
        QString standardUrl = QString("https://%1/%2").arg(server, room);
        
        // 添加查询参数到标准URL
        if (result.contains("parameters")) {
            QVariantMap params = result["parameters"].toMap();
            if (!params.isEmpty()) {
                QUrlQuery urlQuery;
                for (auto it = params.begin(); it != params.end(); ++it) {
                    urlQuery.addQueryItem(it.key(), it.value().toString());
                }
                standardUrl += "?" + urlQuery.toString();
            }
        }
        
        result["standardUrl"] = standardUrl;
    }
    
    return result;
}

QVariantMap ProtocolHandler::parseMeetProtocol(const QString& url)
{
    QVariantMap result;
    
    QRegularExpression pattern(R"(^meet:\/\/([^\/\?#]+)\/([^\/\?#]+)(?:\?([^#]*))?(?:#(.*))?$)");
    QRegularExpressionMatch match = pattern.match(url);
    
    if (!match.hasMatch()) {
        result["valid"] = false;
        return result;
    }
    
    result["valid"] = true;
    result["server"] = match.captured(1);
    result["room"] = match.captured(2);
    
    if (match.capturedLength(3) > 0) {
        QUrlQuery query(match.captured(3));
        QVariantMap params;
        for (const auto& item : query.queryItems()) {
            params[item.first] = item.second;
        }
        result["parameters"] = params;
    }
    
    return result;
}

QVariantMap ProtocolHandler::parseConferenceProtocol(const QString& url)
{
    QVariantMap result;
    
    QRegularExpression pattern(R"(^conference:\/\/([^\/\?#]+)\/([^\/\?#]+)(?:\?([^#]*))?(?:#(.*))?$)");
    QRegularExpressionMatch match = pattern.match(url);
    
    if (!match.hasMatch()) {
        result["valid"] = false;
        return result;
    }
    
    result["valid"] = true;
    result["server"] = match.captured(1);
    result["room"] = match.captured(2);
    
    if (match.capturedLength(3) > 0) {
        QUrlQuery query(match.captured(3));
        QVariantMap params;
        for (const auto& item : query.queryItems()) {
            params[item.first] = item.second;
        }
        result["parameters"] = params;
    }
    
    return result;
}