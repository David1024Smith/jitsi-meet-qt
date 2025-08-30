#include "ChatConfig.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QUrl>

class ChatConfig::Private
{
public:
    Private() {
        initializeDefaults();
    }
    
    // 服务器配置
    QString serverUrl = "wss://meet.jit.si/xmpp-websocket";
    int serverPort = 443;
    bool useSSL = true;
    ConnectionMode connectionMode = AutoDetect;
    QVariantMap proxySettings;
    
    // 消息配置
    int maxMessageLength = 1000;
    MessageFilterLevel messageFilterLevel = BasicFilter;
    QStringList filterKeywords;
    bool emojiEnabled = true;
    bool fileShareEnabled = true;
    qint64 maxFileSize = 10 * 1024 * 1024; // 10MB
    QStringList allowedFileTypes = {"txt", "pdf", "doc", "docx", "jpg", "png", "gif"};
    
    // 历史记录配置
    bool historyEnabled = true;
    int historyLimit = 1000;
    int historyRetentionDays = 30;
    bool historySearchEnabled = true;
    
    // 通知配置
    bool notificationsEnabled = true;
    NotificationTypes notificationTypes = Sound | Visual | Desktop;
    bool soundEnabled = true;
    QString notificationSoundPath = ":/sounds/message_received.wav";
    int notificationDisplayTime = 5000;
    
    // 连接配置
    bool autoReconnectEnabled = true;
    int reconnectInterval = 5;
    int maxReconnectAttempts = 3;
    int connectionTimeout = 30;
    
    // 界面配置
    QSize chatWindowSize = QSize(800, 600);
    int fontSize = 12;
    QString themeName = "default";
    bool showTimestamps = true;
    bool showAvatars = true;
    
    // 自定义配置
    QVariantMap customSettings;
    
    void initializeDefaults() {
        // 默认值已在成员变量初始化中设置
    }
};

ChatConfig::ChatConfig(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

ChatConfig::ChatConfig(const ChatConfig& other)
    : QObject(other.parent())
    , d(new Private(*other.d))
{
}

ChatConfig& ChatConfig::operator=(const ChatConfig& other)
{
    if (this != &other) {
        *d = *other.d;
        emit configurationChanged();
    }
    return *this;
}

ChatConfig::~ChatConfig()
{
    delete d;
}

// 服务器配置
QString ChatConfig::serverUrl() const
{
    return d->serverUrl;
}

void ChatConfig::setServerUrl(const QString& url)
{
    if (d->serverUrl != url && validateServerUrl(url)) {
        d->serverUrl = url;
        emit serverUrlChanged(url);
        emit configurationChanged();
    }
}

int ChatConfig::serverPort() const
{
    return d->serverPort;
}

void ChatConfig::setServerPort(int port)
{
    if (d->serverPort != port && validatePort(port)) {
        d->serverPort = port;
        emit serverPortChanged(port);
        emit configurationChanged();
    }
}

bool ChatConfig::useSSL() const
{
    return d->useSSL;
}

void ChatConfig::setUseSSL(bool useSSL)
{
    if (d->useSSL != useSSL) {
        d->useSSL = useSSL;
        emit useSSLChanged(useSSL);
        emit configurationChanged();
    }
}

ChatConfig::ConnectionMode ChatConfig::connectionMode() const
{
    return d->connectionMode;
}

void ChatConfig::setConnectionMode(ConnectionMode mode)
{
    if (d->connectionMode != mode) {
        d->connectionMode = mode;
        emit configurationChanged();
    }
}

QVariantMap ChatConfig::proxySettings() const
{
    return d->proxySettings;
}

void ChatConfig::setProxySettings(const QVariantMap& settings)
{
    d->proxySettings = settings;
    emit configurationChanged();
}

// 消息配置
int ChatConfig::maxMessageLength() const
{
    return d->maxMessageLength;
}

void ChatConfig::setMaxMessageLength(int length)
{
    if (d->maxMessageLength != length && length > 0) {
        d->maxMessageLength = length;
        emit maxMessageLengthChanged(length);
        emit configurationChanged();
    }
}

ChatConfig::MessageFilterLevel ChatConfig::messageFilterLevel() const
{
    return d->messageFilterLevel;
}

void ChatConfig::setMessageFilterLevel(MessageFilterLevel level)
{
    if (d->messageFilterLevel != level) {
        d->messageFilterLevel = level;
        emit configurationChanged();
    }
}

QStringList ChatConfig::filterKeywords() const
{
    return d->filterKeywords;
}

void ChatConfig::setFilterKeywords(const QStringList& keywords)
{
    d->filterKeywords = keywords;
    emit configurationChanged();
}

void ChatConfig::addFilterKeyword(const QString& keyword)
{
    if (!d->filterKeywords.contains(keyword)) {
        d->filterKeywords.append(keyword);
        emit configurationChanged();
    }
}

void ChatConfig::removeFilterKeyword(const QString& keyword)
{
    if (d->filterKeywords.removeAll(keyword) > 0) {
        emit configurationChanged();
    }
}

bool ChatConfig::isEmojiEnabled() const
{
    return d->emojiEnabled;
}

void ChatConfig::setEmojiEnabled(bool enabled)
{
    if (d->emojiEnabled != enabled) {
        d->emojiEnabled = enabled;
        emit configurationChanged();
    }
}

bool ChatConfig::isFileShareEnabled() const
{
    return d->fileShareEnabled;
}

void ChatConfig::setFileShareEnabled(bool enabled)
{
    if (d->fileShareEnabled != enabled) {
        d->fileShareEnabled = enabled;
        emit configurationChanged();
    }
}

qint64 ChatConfig::maxFileSize() const
{
    return d->maxFileSize;
}

void ChatConfig::setMaxFileSize(qint64 size)
{
    if (d->maxFileSize != size && size > 0) {
        d->maxFileSize = size;
        emit configurationChanged();
    }
}

QStringList ChatConfig::allowedFileTypes() const
{
    return d->allowedFileTypes;
}

void ChatConfig::setAllowedFileTypes(const QStringList& types)
{
    d->allowedFileTypes = types;
    emit configurationChanged();
}

// 历史记录配置
bool ChatConfig::isHistoryEnabled() const
{
    return d->historyEnabled;
}

void ChatConfig::setHistoryEnabled(bool enabled)
{
    if (d->historyEnabled != enabled) {
        d->historyEnabled = enabled;
        emit historyEnabledChanged(enabled);
        emit configurationChanged();
    }
}

int ChatConfig::historyLimit() const
{
    return d->historyLimit;
}

void ChatConfig::setHistoryLimit(int limit)
{
    if (d->historyLimit != limit && limit > 0) {
        d->historyLimit = limit;
        emit historyLimitChanged(limit);
        emit configurationChanged();
    }
}

int ChatConfig::historyRetentionDays() const
{
    return d->historyRetentionDays;
}

void ChatConfig::setHistoryRetentionDays(int days)
{
    if (d->historyRetentionDays != days && days > 0) {
        d->historyRetentionDays = days;
        emit configurationChanged();
    }
}

bool ChatConfig::isHistorySearchEnabled() const
{
    return d->historySearchEnabled;
}

void ChatConfig::setHistorySearchEnabled(bool enabled)
{
    if (d->historySearchEnabled != enabled) {
        d->historySearchEnabled = enabled;
        emit configurationChanged();
    }
}

// 通知配置
bool ChatConfig::areNotificationsEnabled() const
{
    return d->notificationsEnabled;
}

void ChatConfig::setNotificationsEnabled(bool enabled)
{
    if (d->notificationsEnabled != enabled) {
        d->notificationsEnabled = enabled;
        emit notificationsEnabledChanged(enabled);
        emit configurationChanged();
    }
}

ChatConfig::NotificationTypes ChatConfig::notificationTypes() const
{
    return d->notificationTypes;
}

void ChatConfig::setNotificationTypes(NotificationTypes types)
{
    if (d->notificationTypes != types) {
        d->notificationTypes = types;
        emit configurationChanged();
    }
}

bool ChatConfig::isSoundEnabled() const
{
    return d->soundEnabled;
}

void ChatConfig::setSoundEnabled(bool enabled)
{
    if (d->soundEnabled != enabled) {
        d->soundEnabled = enabled;
        emit soundEnabledChanged(enabled);
        emit configurationChanged();
    }
}

QString ChatConfig::notificationSoundPath() const
{
    return d->notificationSoundPath;
}

void ChatConfig::setNotificationSoundPath(const QString& path)
{
    if (d->notificationSoundPath != path) {
        d->notificationSoundPath = path;
        emit configurationChanged();
    }
}

int ChatConfig::notificationDisplayTime() const
{
    return d->notificationDisplayTime;
}

void ChatConfig::setNotificationDisplayTime(int time)
{
    if (d->notificationDisplayTime != time && time > 0) {
        d->notificationDisplayTime = time;
        emit configurationChanged();
    }
}

// 连接配置
bool ChatConfig::isAutoReconnectEnabled() const
{
    return d->autoReconnectEnabled;
}

void ChatConfig::setAutoReconnectEnabled(bool enabled)
{
    if (d->autoReconnectEnabled != enabled) {
        d->autoReconnectEnabled = enabled;
        emit autoReconnectChanged(enabled);
        emit configurationChanged();
    }
}

int ChatConfig::reconnectInterval() const
{
    return d->reconnectInterval;
}

void ChatConfig::setReconnectInterval(int interval)
{
    if (d->reconnectInterval != interval && interval > 0) {
        d->reconnectInterval = interval;
        emit configurationChanged();
    }
}

int ChatConfig::maxReconnectAttempts() const
{
    return d->maxReconnectAttempts;
}

void ChatConfig::setMaxReconnectAttempts(int attempts)
{
    if (d->maxReconnectAttempts != attempts && attempts > 0) {
        d->maxReconnectAttempts = attempts;
        emit configurationChanged();
    }
}

int ChatConfig::connectionTimeout() const
{
    return d->connectionTimeout;
}

void ChatConfig::setConnectionTimeout(int timeout)
{
    if (d->connectionTimeout != timeout && timeout > 0) {
        d->connectionTimeout = timeout;
        emit configurationChanged();
    }
}

// 界面配置
QSize ChatConfig::chatWindowSize() const
{
    return d->chatWindowSize;
}

void ChatConfig::setChatWindowSize(const QSize& size)
{
    if (d->chatWindowSize != size && size.isValid()) {
        d->chatWindowSize = size;
        emit configurationChanged();
    }
}

int ChatConfig::fontSize() const
{
    return d->fontSize;
}

void ChatConfig::setFontSize(int size)
{
    if (d->fontSize != size && size > 0) {
        d->fontSize = size;
        emit configurationChanged();
    }
}

QString ChatConfig::themeName() const
{
    return d->themeName;
}

void ChatConfig::setThemeName(const QString& theme)
{
    if (d->themeName != theme) {
        d->themeName = theme;
        emit configurationChanged();
    }
}

bool ChatConfig::showTimestamps() const
{
    return d->showTimestamps;
}

void ChatConfig::setShowTimestamps(bool show)
{
    if (d->showTimestamps != show) {
        d->showTimestamps = show;
        emit configurationChanged();
    }
}

bool ChatConfig::showAvatars() const
{
    return d->showAvatars;
}

void ChatConfig::setShowAvatars(bool show)
{
    if (d->showAvatars != show) {
        d->showAvatars = show;
        emit configurationChanged();
    }
}

// 扩展配置
QVariant ChatConfig::customSetting(const QString& key, const QVariant& defaultValue) const
{
    return d->customSettings.value(key, defaultValue);
}

void ChatConfig::setCustomSetting(const QString& key, const QVariant& value)
{
    if (d->customSettings.value(key) != value) {
        d->customSettings[key] = value;
        emit customSettingChanged(key, value);
        emit configurationChanged();
    }
}

QVariantMap ChatConfig::customSettings() const
{
    return d->customSettings;
}

void ChatConfig::setCustomSettings(const QVariantMap& settings)
{
    d->customSettings = settings;
    emit configurationChanged();
}

// 配置管理
bool ChatConfig::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open config file:" << filePath;
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return false;
    }
    
    fromVariantMap(doc.object().toVariantMap());
    return true;
}

bool ChatConfig::saveToFile(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot write config file:" << filePath;
        return false;
    }
    
    QJsonDocument doc(QJsonObject::fromVariantMap(toVariantMap()));
    file.write(doc.toJson());
    return true;
}

QVariantMap ChatConfig::toVariantMap() const
{
    QVariantMap map;
    
    // 服务器配置
    map["serverUrl"] = d->serverUrl;
    map["serverPort"] = d->serverPort;
    map["useSSL"] = d->useSSL;
    map["connectionMode"] = static_cast<int>(d->connectionMode);
    map["proxySettings"] = d->proxySettings;
    
    // 消息配置
    map["maxMessageLength"] = d->maxMessageLength;
    map["messageFilterLevel"] = static_cast<int>(d->messageFilterLevel);
    map["filterKeywords"] = d->filterKeywords;
    map["emojiEnabled"] = d->emojiEnabled;
    map["fileShareEnabled"] = d->fileShareEnabled;
    map["maxFileSize"] = d->maxFileSize;
    map["allowedFileTypes"] = d->allowedFileTypes;
    
    // 历史记录配置
    map["historyEnabled"] = d->historyEnabled;
    map["historyLimit"] = d->historyLimit;
    map["historyRetentionDays"] = d->historyRetentionDays;
    map["historySearchEnabled"] = d->historySearchEnabled;
    
    // 通知配置
    map["notificationsEnabled"] = d->notificationsEnabled;
    map["notificationTypes"] = static_cast<int>(d->notificationTypes);
    map["soundEnabled"] = d->soundEnabled;
    map["notificationSoundPath"] = d->notificationSoundPath;
    map["notificationDisplayTime"] = d->notificationDisplayTime;
    
    // 连接配置
    map["autoReconnectEnabled"] = d->autoReconnectEnabled;
    map["reconnectInterval"] = d->reconnectInterval;
    map["maxReconnectAttempts"] = d->maxReconnectAttempts;
    map["connectionTimeout"] = d->connectionTimeout;
    
    // 界面配置
    map["chatWindowSize"] = d->chatWindowSize;
    map["fontSize"] = d->fontSize;
    map["themeName"] = d->themeName;
    map["showTimestamps"] = d->showTimestamps;
    map["showAvatars"] = d->showAvatars;
    
    // 自定义配置
    map["customSettings"] = d->customSettings;
    
    return map;
}

void ChatConfig::fromVariantMap(const QVariantMap& map)
{
    // 服务器配置
    if (map.contains("serverUrl"))
        setServerUrl(map["serverUrl"].toString());
    if (map.contains("serverPort"))
        setServerPort(map["serverPort"].toInt());
    if (map.contains("useSSL"))
        setUseSSL(map["useSSL"].toBool());
    if (map.contains("connectionMode"))
        setConnectionMode(static_cast<ConnectionMode>(map["connectionMode"].toInt()));
    if (map.contains("proxySettings"))
        setProxySettings(map["proxySettings"].toMap());
    
    // 消息配置
    if (map.contains("maxMessageLength"))
        setMaxMessageLength(map["maxMessageLength"].toInt());
    if (map.contains("messageFilterLevel"))
        setMessageFilterLevel(static_cast<MessageFilterLevel>(map["messageFilterLevel"].toInt()));
    if (map.contains("filterKeywords"))
        setFilterKeywords(map["filterKeywords"].toStringList());
    if (map.contains("emojiEnabled"))
        setEmojiEnabled(map["emojiEnabled"].toBool());
    if (map.contains("fileShareEnabled"))
        setFileShareEnabled(map["fileShareEnabled"].toBool());
    if (map.contains("maxFileSize"))
        setMaxFileSize(map["maxFileSize"].toLongLong());
    if (map.contains("allowedFileTypes"))
        setAllowedFileTypes(map["allowedFileTypes"].toStringList());
    
    // 历史记录配置
    if (map.contains("historyEnabled"))
        setHistoryEnabled(map["historyEnabled"].toBool());
    if (map.contains("historyLimit"))
        setHistoryLimit(map["historyLimit"].toInt());
    if (map.contains("historyRetentionDays"))
        setHistoryRetentionDays(map["historyRetentionDays"].toInt());
    if (map.contains("historySearchEnabled"))
        setHistorySearchEnabled(map["historySearchEnabled"].toBool());
    
    // 通知配置
    if (map.contains("notificationsEnabled"))
        setNotificationsEnabled(map["notificationsEnabled"].toBool());
    if (map.contains("notificationTypes"))
        setNotificationTypes(static_cast<NotificationTypes>(map["notificationTypes"].toInt()));
    if (map.contains("soundEnabled"))
        setSoundEnabled(map["soundEnabled"].toBool());
    if (map.contains("notificationSoundPath"))
        setNotificationSoundPath(map["notificationSoundPath"].toString());
    if (map.contains("notificationDisplayTime"))
        setNotificationDisplayTime(map["notificationDisplayTime"].toInt());
    
    // 连接配置
    if (map.contains("autoReconnectEnabled"))
        setAutoReconnectEnabled(map["autoReconnectEnabled"].toBool());
    if (map.contains("reconnectInterval"))
        setReconnectInterval(map["reconnectInterval"].toInt());
    if (map.contains("maxReconnectAttempts"))
        setMaxReconnectAttempts(map["maxReconnectAttempts"].toInt());
    if (map.contains("connectionTimeout"))
        setConnectionTimeout(map["connectionTimeout"].toInt());
    
    // 界面配置
    if (map.contains("chatWindowSize"))
        setChatWindowSize(map["chatWindowSize"].toSize());
    if (map.contains("fontSize"))
        setFontSize(map["fontSize"].toInt());
    if (map.contains("themeName"))
        setThemeName(map["themeName"].toString());
    if (map.contains("showTimestamps"))
        setShowTimestamps(map["showTimestamps"].toBool());
    if (map.contains("showAvatars"))
        setShowAvatars(map["showAvatars"].toBool());
    
    // 自定义配置
    if (map.contains("customSettings"))
        setCustomSettings(map["customSettings"].toMap());
}

void ChatConfig::resetToDefaults()
{
    d->initializeDefaults();
    emit configurationChanged();
}

bool ChatConfig::validate() const
{
    // 验证服务器URL
    if (!validateServerUrl(d->serverUrl)) {
        return false;
    }
    
    // 验证端口
    if (!validatePort(d->serverPort)) {
        return false;
    }
    
    // 验证其他配置项
    if (d->maxMessageLength <= 0 || d->maxMessageLength > 10000) {
        return false;
    }
    
    if (d->historyLimit <= 0) {
        return false;
    }
    
    return true;
}

ChatConfig* ChatConfig::clone(QObject* parent) const
{
    ChatConfig* cloned = new ChatConfig(parent);
    cloned->fromVariantMap(toVariantMap());
    return cloned;
}

bool ChatConfig::equals(const ChatConfig* other) const
{
    if (!other) {
        return false;
    }
    
    return toVariantMap() == other->toVariantMap();
}

void ChatConfig::applyChanges()
{
    // TODO: 应用配置更改
    emit configurationChanged();
}

void ChatConfig::cancelChanges()
{
    // TODO: 取消配置更改
}

bool ChatConfig::validateServerUrl(const QString& url) const
{
    QUrl qurl(url);
    return qurl.isValid() && (qurl.scheme() == "ws" || qurl.scheme() == "wss");
}

bool ChatConfig::validatePort(int port) const
{
    return port > 0 && port <= 65535;
}

void ChatConfig::initializeDefaults()
{
    d->initializeDefaults();
}