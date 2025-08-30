#include "NetworkConfig.h"
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

class NetworkConfig::Private
{
public:
    // 服务器配置
    QString serverUrl;
    int serverPort;
    QString serverDomain;
    
    // 连接配置
    int connectionTimeout;
    bool autoReconnect;
    int reconnectInterval;
    int maxReconnectAttempts;
    
    // 协议配置
    bool webRTCEnabled;
    bool webSocketEnabled;
    bool httpsOnly;
    QList<NetworkConfig::Protocol> enabledProtocols;
    
    // STUN/TURN配置
    QStringList stunServers;
    QStringList turnServers;
    QString turnUsername;
    QString turnPassword;
    
    // 性能配置
    NetworkConfig::QualityLevel qualityLevel;
    int bandwidthLimit;
    bool compressionEnabled;
    
    // 内部状态
    bool hasChanges;
    QVariantMap originalConfig;
    
    Private() {
        initializeDefaults();
    }
    
    void initializeDefaults() {
        // 服务器默认配置
        serverUrl = "https://meet.jit.si";
        serverPort = 443;
        serverDomain = "meet.jit.si";
        
        // 连接默认配置
        connectionTimeout = 30000;  // 30秒
        autoReconnect = true;
        reconnectInterval = 5000;   // 5秒
        maxReconnectAttempts = 3;
        
        // 协议默认配置
        webRTCEnabled = true;
        webSocketEnabled = true;
        httpsOnly = true;
        enabledProtocols = {
            NetworkConfig::HTTPS,
            NetworkConfig::WebSocketSecure,
            NetworkConfig::WebRTC
        };
        
        // STUN/TURN默认配置
        stunServers = {
            "stun:stun.l.google.com:19302",
            "stun:stun1.l.google.com:19302"
        };
        turnServers.clear();
        turnUsername.clear();
        turnPassword.clear();
        
        // 性能默认配置
        qualityLevel = NetworkConfig::Auto;
        bandwidthLimit = 0;  // 无限制
        compressionEnabled = true;
        
        hasChanges = false;
    }
};

NetworkConfig::NetworkConfig(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    initializeDefaults();
}

NetworkConfig::~NetworkConfig()
{
    delete d;
}

void NetworkConfig::initializeDefaults()
{
    d->initializeDefaults();
    d->originalConfig = toVariantMap();
}

// 服务器配置实现
void NetworkConfig::setServerUrl(const QString& url)
{
    if (d->serverUrl != url) {
        d->serverUrl = url;
        d->hasChanges = true;
        emit serverUrlChanged(url);
        emit configurationChanged();
    }
}

QString NetworkConfig::serverUrl() const
{
    return d->serverUrl;
}

void NetworkConfig::setServerPort(int port)
{
    if (d->serverPort != port) {
        d->serverPort = port;
        d->hasChanges = true;
        emit serverPortChanged(port);
        emit configurationChanged();
    }
}

int NetworkConfig::serverPort() const
{
    return d->serverPort;
}

void NetworkConfig::setServerDomain(const QString& domain)
{
    if (d->serverDomain != domain) {
        d->serverDomain = domain;
        d->hasChanges = true;
        emit configurationChanged();
    }
}

QString NetworkConfig::serverDomain() const
{
    return d->serverDomain;
}

// 连接配置实现
void NetworkConfig::setConnectionTimeout(int timeout)
{
    if (d->connectionTimeout != timeout) {
        d->connectionTimeout = timeout;
        d->hasChanges = true;
        emit connectionTimeoutChanged(timeout);
        emit configurationChanged();
    }
}

int NetworkConfig::connectionTimeout() const
{
    return d->connectionTimeout;
}

void NetworkConfig::setAutoReconnect(bool enabled)
{
    if (d->autoReconnect != enabled) {
        d->autoReconnect = enabled;
        d->hasChanges = true;
        emit autoReconnectChanged(enabled);
        emit configurationChanged();
    }
}

bool NetworkConfig::autoReconnect() const
{
    return d->autoReconnect;
}

void NetworkConfig::setReconnectInterval(int interval)
{
    if (d->reconnectInterval != interval) {
        d->reconnectInterval = interval;
        d->hasChanges = true;
        emit reconnectIntervalChanged(interval);
        emit configurationChanged();
    }
}

int NetworkConfig::reconnectInterval() const
{
    return d->reconnectInterval;
}

void NetworkConfig::setMaxReconnectAttempts(int attempts)
{
    if (d->maxReconnectAttempts != attempts) {
        d->maxReconnectAttempts = attempts;
        d->hasChanges = true;
        emit configurationChanged();
    }
}

int NetworkConfig::maxReconnectAttempts() const
{
    return d->maxReconnectAttempts;
}

// 协议配置实现
void NetworkConfig::setWebRTCEnabled(bool enabled)
{
    if (d->webRTCEnabled != enabled) {
        d->webRTCEnabled = enabled;
        d->hasChanges = true;
        emit webRTCEnabledChanged(enabled);
        emit configurationChanged();
    }
}

bool NetworkConfig::webRTCEnabled() const
{
    return d->webRTCEnabled;
}

void NetworkConfig::setWebSocketEnabled(bool enabled)
{
    if (d->webSocketEnabled != enabled) {
        d->webSocketEnabled = enabled;
        d->hasChanges = true;
        emit webSocketEnabledChanged(enabled);
        emit configurationChanged();
    }
}

bool NetworkConfig::webSocketEnabled() const
{
    return d->webSocketEnabled;
}

void NetworkConfig::setHttpsOnly(bool httpsOnly)
{
    if (d->httpsOnly != httpsOnly) {
        d->httpsOnly = httpsOnly;
        d->hasChanges = true;
        emit httpsOnlyChanged(httpsOnly);
        emit configurationChanged();
    }
}

bool NetworkConfig::httpsOnly() const
{
    return d->httpsOnly;
}

void NetworkConfig::setEnabledProtocols(const QList<Protocol>& protocols)
{
    if (d->enabledProtocols != protocols) {
        d->enabledProtocols = protocols;
        d->hasChanges = true;
        emit configurationChanged();
    }
}

QList<NetworkConfig::Protocol> NetworkConfig::enabledProtocols() const
{
    return d->enabledProtocols;
}

// STUN/TURN配置实现
void NetworkConfig::setStunServers(const QStringList& servers)
{
    if (d->stunServers != servers) {
        d->stunServers = servers;
        d->hasChanges = true;
        emit configurationChanged();
    }
}

QStringList NetworkConfig::stunServers() const
{
    return d->stunServers;
}

void NetworkConfig::setTurnServers(const QStringList& servers)
{
    if (d->turnServers != servers) {
        d->turnServers = servers;
        d->hasChanges = true;
        emit configurationChanged();
    }
}

QStringList NetworkConfig::turnServers() const
{
    return d->turnServers;
}

void NetworkConfig::setTurnUsername(const QString& username)
{
    if (d->turnUsername != username) {
        d->turnUsername = username;
        d->hasChanges = true;
        emit configurationChanged();
    }
}

QString NetworkConfig::turnUsername() const
{
    return d->turnUsername;
}

void NetworkConfig::setTurnPassword(const QString& password)
{
    if (d->turnPassword != password) {
        d->turnPassword = password;
        d->hasChanges = true;
        emit configurationChanged();
    }
}

QString NetworkConfig::turnPassword() const
{
    return d->turnPassword;
}

// 性能配置实现
void NetworkConfig::setQualityLevel(QualityLevel level)
{
    if (d->qualityLevel != level) {
        d->qualityLevel = level;
        d->hasChanges = true;
        emit configurationChanged();
    }
}

NetworkConfig::QualityLevel NetworkConfig::qualityLevel() const
{
    return d->qualityLevel;
}

void NetworkConfig::setBandwidthLimit(int bandwidth)
{
    if (d->bandwidthLimit != bandwidth) {
        d->bandwidthLimit = bandwidth;
        d->hasChanges = true;
        emit configurationChanged();
    }
}

int NetworkConfig::bandwidthLimit() const
{
    return d->bandwidthLimit;
}

void NetworkConfig::setCompressionEnabled(bool enabled)
{
    if (d->compressionEnabled != enabled) {
        d->compressionEnabled = enabled;
        d->hasChanges = true;
        emit configurationChanged();
    }
}

bool NetworkConfig::compressionEnabled() const
{
    return d->compressionEnabled;
}

// 配置管理实现
void NetworkConfig::fromVariantMap(const QVariantMap& config)
{
    // 服务器配置
    if (config.contains("serverUrl"))
        setServerUrl(config["serverUrl"].toString());
    if (config.contains("serverPort"))
        setServerPort(config["serverPort"].toInt());
    if (config.contains("serverDomain"))
        setServerDomain(config["serverDomain"].toString());
    
    // 连接配置
    if (config.contains("connectionTimeout"))
        setConnectionTimeout(config["connectionTimeout"].toInt());
    if (config.contains("autoReconnect"))
        setAutoReconnect(config["autoReconnect"].toBool());
    if (config.contains("reconnectInterval"))
        setReconnectInterval(config["reconnectInterval"].toInt());
    if (config.contains("maxReconnectAttempts"))
        setMaxReconnectAttempts(config["maxReconnectAttempts"].toInt());
    
    // 协议配置
    if (config.contains("webRTCEnabled"))
        setWebRTCEnabled(config["webRTCEnabled"].toBool());
    if (config.contains("webSocketEnabled"))
        setWebSocketEnabled(config["webSocketEnabled"].toBool());
    if (config.contains("httpsOnly"))
        setHttpsOnly(config["httpsOnly"].toBool());
    
    // STUN/TURN配置
    if (config.contains("stunServers"))
        setStunServers(config["stunServers"].toStringList());
    if (config.contains("turnServers"))
        setTurnServers(config["turnServers"].toStringList());
    if (config.contains("turnUsername"))
        setTurnUsername(config["turnUsername"].toString());
    if (config.contains("turnPassword"))
        setTurnPassword(config["turnPassword"].toString());
    
    // 性能配置
    if (config.contains("qualityLevel"))
        setQualityLevel(static_cast<QualityLevel>(config["qualityLevel"].toInt()));
    if (config.contains("bandwidthLimit"))
        setBandwidthLimit(config["bandwidthLimit"].toInt());
    if (config.contains("compressionEnabled"))
        setCompressionEnabled(config["compressionEnabled"].toBool());
    
    d->hasChanges = false;
    d->originalConfig = toVariantMap();
}

QVariantMap NetworkConfig::toVariantMap() const
{
    QVariantMap config;
    
    // 服务器配置
    config["serverUrl"] = d->serverUrl;
    config["serverPort"] = d->serverPort;
    config["serverDomain"] = d->serverDomain;
    
    // 连接配置
    config["connectionTimeout"] = d->connectionTimeout;
    config["autoReconnect"] = d->autoReconnect;
    config["reconnectInterval"] = d->reconnectInterval;
    config["maxReconnectAttempts"] = d->maxReconnectAttempts;
    
    // 协议配置
    config["webRTCEnabled"] = d->webRTCEnabled;
    config["webSocketEnabled"] = d->webSocketEnabled;
    config["httpsOnly"] = d->httpsOnly;
    
    // STUN/TURN配置
    config["stunServers"] = d->stunServers;
    config["turnServers"] = d->turnServers;
    config["turnUsername"] = d->turnUsername;
    config["turnPassword"] = d->turnPassword;
    
    // 性能配置
    config["qualityLevel"] = static_cast<int>(d->qualityLevel);
    config["bandwidthLimit"] = d->bandwidthLimit;
    config["compressionEnabled"] = d->compressionEnabled;
    
    return config;
}

bool NetworkConfig::validate() const
{
    QStringList errors;
    
    // 验证服务器URL
    if (!isValidUrl(d->serverUrl)) {
        errors << "Invalid server URL";
    }
    
    // 验证端口
    if (!isValidPort(d->serverPort)) {
        errors << "Invalid server port";
    }
    
    // 验证超时时间
    if (d->connectionTimeout <= 0) {
        errors << "Connection timeout must be positive";
    }
    
    // 验证重连间隔
    if (d->reconnectInterval <= 0) {
        errors << "Reconnect interval must be positive";
    }
    
    if (!errors.isEmpty()) {
        emit const_cast<NetworkConfig*>(this)->validationFailed(errors);
        return false;
    }
    
    return true;
}

void NetworkConfig::resetToDefaults()
{
    d->initializeDefaults();
    d->originalConfig = toVariantMap();
    emit configurationChanged();
}

QVariantMap NetworkConfig::defaultConfiguration()
{
    NetworkConfig defaultConfig;
    return defaultConfig.toVariantMap();
}

bool NetworkConfig::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    fromVariantMap(doc.object().toVariantMap());
    return true;
}

bool NetworkConfig::saveToFile(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(QJsonObject::fromVariantMap(toVariantMap()));
    file.write(doc.toJson());
    return true;
}

void NetworkConfig::applyChanges()
{
    d->originalConfig = toVariantMap();
    d->hasChanges = false;
}

void NetworkConfig::cancelChanges()
{
    fromVariantMap(d->originalConfig);
    d->hasChanges = false;
}

bool NetworkConfig::isValidUrl(const QString& url) const
{
    QUrl qurl(url);
    return qurl.isValid() && !qurl.scheme().isEmpty() && !qurl.host().isEmpty();
}

bool NetworkConfig::isValidPort(int port) const
{
    return port > 0 && port <= 65535;
}