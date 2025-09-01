#include "MeetingConfig.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

class MeetingConfig::Private
{
public:
    // 基本设置
    QString defaultServer = "meet.jit.si";
    QStringList serverList;
    QString defaultDisplayName;
    QString defaultEmail;
    
    // 会议设置
    bool autoJoin = false;
    bool defaultAudioEnabled = true;
    bool defaultVideoEnabled = true;
    AudioQuality audioQuality = StandardAudio;
    VideoQuality videoQuality = StandardVideo;
    
    // 认证设置
    AuthenticationMethod authMethod = GuestAuth;
    bool rememberAuth = false;
    
    // 网络设置
    int connectionTimeout = 30000;
    int reconnectAttempts = 3;
    QStringList supportedProtocols = {"https", "jitsi"};
    
    // 界面设置
    bool showJoinDialog = true;
    bool minimizeToTray = false;
    
    // 高级设置
    bool debugEnabled = false;
    QString logLevel = "info";
    
    // 自定义设置
    QVariantMap customSettings;
    
    // 配置文件路径
    QString configFilePath;
};

MeetingConfig::MeetingConfig(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    initializeDefaults();
    
    // 设置默认配置文件路径
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    d->configFilePath = configDir + "/meeting-config.json";
}

MeetingConfig::~MeetingConfig() = default;

bool MeetingConfig::loadConfiguration(const QString& configFile)
{
    QString filePath = configFile.isEmpty() ? d->configFilePath : configFile;
    
    QSettings settings(filePath, QSettings::IniFormat);
    
    // 加载服务器设置
    d->defaultServer = settings.value("server/default", d->defaultServer).toString();
    d->serverList = settings.value("server/list", d->serverList).toStringList();
    
    // 加载用户设置
    d->defaultDisplayName = settings.value("user/displayName", d->defaultDisplayName).toString();
    d->defaultEmail = settings.value("user/email", d->defaultEmail).toString();
    
    // 加载会议设置
    d->autoJoin = settings.value("meeting/autoJoin", d->autoJoin).toBool();
    d->defaultAudioEnabled = settings.value("meeting/audioEnabled", d->defaultAudioEnabled).toBool();
    d->defaultVideoEnabled = settings.value("meeting/videoEnabled", d->defaultVideoEnabled).toBool();
    d->audioQuality = static_cast<AudioQuality>(settings.value("meeting/audioQuality", d->audioQuality).toInt());
    d->videoQuality = static_cast<VideoQuality>(settings.value("meeting/videoQuality", d->videoQuality).toInt());
    
    // 加载认证设置
    d->authMethod = static_cast<AuthenticationMethod>(settings.value("auth/method", d->authMethod).toInt());
    d->rememberAuth = settings.value("auth/remember", d->rememberAuth).toBool();
    
    // 加载网络设置
    d->connectionTimeout = settings.value("network/timeout", d->connectionTimeout).toInt();
    d->reconnectAttempts = settings.value("network/retryAttempts", d->reconnectAttempts).toInt();
    d->supportedProtocols = settings.value("network/protocols", d->supportedProtocols).toStringList();
    
    // 加载界面设置
    d->showJoinDialog = settings.value("ui/showJoinDialog", d->showJoinDialog).toBool();
    d->minimizeToTray = settings.value("ui/minimizeToTray", d->minimizeToTray).toBool();
    
    // 加载高级设置
    d->debugEnabled = settings.value("advanced/debug", d->debugEnabled).toBool();
    d->logLevel = settings.value("advanced/logLevel", d->logLevel).toString();
    
    qDebug() << "Configuration loaded from:" << filePath;
    return true;
}

bool MeetingConfig::saveConfiguration(const QString& configFile)
{
    QString filePath = configFile.isEmpty() ? d->configFilePath : configFile;
    
    QSettings settings(filePath, QSettings::IniFormat);
    
    // 保存服务器设置
    settings.setValue("server/default", d->defaultServer);
    settings.setValue("server/list", d->serverList);
    
    // 保存用户设置
    settings.setValue("user/displayName", d->defaultDisplayName);
    settings.setValue("user/email", d->defaultEmail);
    
    // 保存会议设置
    settings.setValue("meeting/autoJoin", d->autoJoin);
    settings.setValue("meeting/audioEnabled", d->defaultAudioEnabled);
    settings.setValue("meeting/videoEnabled", d->defaultVideoEnabled);
    settings.setValue("meeting/audioQuality", static_cast<int>(d->audioQuality));
    settings.setValue("meeting/videoQuality", static_cast<int>(d->videoQuality));
    
    // 保存认证设置
    settings.setValue("auth/method", static_cast<int>(d->authMethod));
    settings.setValue("auth/remember", d->rememberAuth);
    
    // 保存网络设置
    settings.setValue("network/timeout", d->connectionTimeout);
    settings.setValue("network/retryAttempts", d->reconnectAttempts);
    settings.setValue("network/protocols", d->supportedProtocols);
    
    // 保存界面设置
    settings.setValue("ui/showJoinDialog", d->showJoinDialog);
    settings.setValue("ui/minimizeToTray", d->minimizeToTray);
    
    // 保存高级设置
    settings.setValue("advanced/debug", d->debugEnabled);
    settings.setValue("advanced/logLevel", d->logLevel);
    
    settings.sync();
    
    qDebug() << "Configuration saved to:" << filePath;
    return settings.status() == QSettings::NoError;
}

void MeetingConfig::resetToDefaults()
{
    initializeDefaults();
    emit configurationChanged("all", QVariant());
}

bool MeetingConfig::validateConfiguration() const
{
    // 验证服务器地址
    if (d->defaultServer.isEmpty()) {
        return false;
    }
    
    // 验证超时时间
    if (d->connectionTimeout <= 0) {
        return false;
    }
    
    // 验证重连次数
    if (d->reconnectAttempts < 0) {
        return false;
    }
    
    return true;
}

// 服务器设置
void MeetingConfig::setDefaultServer(const QString& server)
{
    if (d->defaultServer != server) {
        d->defaultServer = server;
        emitConfigurationChanged("defaultServer", server);
    }
}

QString MeetingConfig::defaultServer() const
{
    return d->defaultServer;
}

void MeetingConfig::setServerList(const QStringList& servers)
{
    if (d->serverList != servers) {
        d->serverList = servers;
        emit serverListChanged(servers);
        emitConfigurationChanged("serverList", servers);
    }
}

QStringList MeetingConfig::serverList() const
{
    return d->serverList;
}

void MeetingConfig::addServer(const QString& server)
{
    if (!d->serverList.contains(server)) {
        d->serverList.append(server);
        emit serverListChanged(d->serverList);
        emitConfigurationChanged("serverList", d->serverList);
    }
}

void MeetingConfig::removeServer(const QString& server)
{
    if (d->serverList.removeOne(server)) {
        emit serverListChanged(d->serverList);
        emitConfigurationChanged("serverList", d->serverList);
    }
}

// 用户设置
void MeetingConfig::setDefaultDisplayName(const QString& name)
{
    if (d->defaultDisplayName != name) {
        d->defaultDisplayName = name;
        emitConfigurationChanged("defaultDisplayName", name);
    }
}

QString MeetingConfig::defaultDisplayName() const
{
    return d->defaultDisplayName;
}

void MeetingConfig::setDefaultEmail(const QString& email)
{
    if (d->defaultEmail != email) {
        d->defaultEmail = email;
        emitConfigurationChanged("defaultEmail", email);
    }
}

QString MeetingConfig::defaultEmail() const
{
    return d->defaultEmail;
}

// 会议设置
void MeetingConfig::setAutoJoin(bool autoJoin)
{
    if (d->autoJoin != autoJoin) {
        d->autoJoin = autoJoin;
        emitConfigurationChanged("autoJoin", autoJoin);
    }
}

bool MeetingConfig::autoJoin() const
{
    return d->autoJoin;
}

void MeetingConfig::setDefaultAudioEnabled(bool enabled)
{
    if (d->defaultAudioEnabled != enabled) {
        d->defaultAudioEnabled = enabled;
        emitConfigurationChanged("defaultAudioEnabled", enabled);
    }
}

bool MeetingConfig::defaultAudioEnabled() const
{
    return d->defaultAudioEnabled;
}

void MeetingConfig::setDefaultVideoEnabled(bool enabled)
{
    if (d->defaultVideoEnabled != enabled) {
        d->defaultVideoEnabled = enabled;
        emitConfigurationChanged("defaultVideoEnabled", enabled);
    }
}

bool MeetingConfig::defaultVideoEnabled() const
{
    return d->defaultVideoEnabled;
}

void MeetingConfig::setAudioQuality(AudioQuality quality)
{
    if (d->audioQuality != quality) {
        d->audioQuality = quality;
        emitConfigurationChanged("audioQuality", static_cast<int>(quality));
    }
}

MeetingConfig::AudioQuality MeetingConfig::audioQuality() const
{
    return d->audioQuality;
}

void MeetingConfig::setVideoQuality(VideoQuality quality)
{
    if (d->videoQuality != quality) {
        d->videoQuality = quality;
        emitConfigurationChanged("videoQuality", static_cast<int>(quality));
    }
}

MeetingConfig::VideoQuality MeetingConfig::videoQuality() const
{
    return d->videoQuality;
}

// 认证设置
void MeetingConfig::setAuthenticationMethod(AuthenticationMethod method)
{
    if (d->authMethod != method) {
        d->authMethod = method;
        emitConfigurationChanged("authenticationMethod", static_cast<int>(method));
    }
}

MeetingConfig::AuthenticationMethod MeetingConfig::authenticationMethod() const
{
    return d->authMethod;
}

void MeetingConfig::setRememberAuthentication(bool remember)
{
    if (d->rememberAuth != remember) {
        d->rememberAuth = remember;
        emitConfigurationChanged("rememberAuthentication", remember);
    }
}

bool MeetingConfig::rememberAuthentication() const
{
    return d->rememberAuth;
}

// 网络设置
void MeetingConfig::setConnectionTimeout(int timeout)
{
    if (d->connectionTimeout != timeout) {
        d->connectionTimeout = timeout;
        emitConfigurationChanged("connectionTimeout", timeout);
    }
}

int MeetingConfig::connectionTimeout() const
{
    return d->connectionTimeout;
}

void MeetingConfig::setReconnectAttempts(int attempts)
{
    if (d->reconnectAttempts != attempts) {
        d->reconnectAttempts = attempts;
        emitConfigurationChanged("reconnectAttempts", attempts);
    }
}

int MeetingConfig::reconnectAttempts() const
{
    return d->reconnectAttempts;
}

void MeetingConfig::setSupportedProtocols(const QStringList& protocols)
{
    if (d->supportedProtocols != protocols) {
        d->supportedProtocols = protocols;
        emitConfigurationChanged("supportedProtocols", protocols);
    }
}

QStringList MeetingConfig::supportedProtocols() const
{
    return d->supportedProtocols;
}

// 界面设置
void MeetingConfig::setShowJoinDialog(bool show)
{
    if (d->showJoinDialog != show) {
        d->showJoinDialog = show;
        emitConfigurationChanged("showJoinDialog", show);
    }
}

bool MeetingConfig::showJoinDialog() const
{
    return d->showJoinDialog;
}

void MeetingConfig::setMinimizeToTray(bool minimize)
{
    if (d->minimizeToTray != minimize) {
        d->minimizeToTray = minimize;
        emitConfigurationChanged("minimizeToTray", minimize);
    }
}

bool MeetingConfig::minimizeToTray() const
{
    return d->minimizeToTray;
}

// 高级设置
void MeetingConfig::setDebugEnabled(bool enabled)
{
    if (d->debugEnabled != enabled) {
        d->debugEnabled = enabled;
        emitConfigurationChanged("debugEnabled", enabled);
    }
}

bool MeetingConfig::debugEnabled() const
{
    return d->debugEnabled;
}

void MeetingConfig::setLogLevel(const QString& level)
{
    if (d->logLevel != level) {
        d->logLevel = level;
        emitConfigurationChanged("logLevel", level);
    }
}

QString MeetingConfig::logLevel() const
{
    return d->logLevel;
}

void MeetingConfig::setCustomSetting(const QString& key, const QVariant& value)
{
    if (d->customSettings.value(key) != value) {
        d->customSettings[key] = value;
        emitConfigurationChanged(key, value);
    }
}

QVariant MeetingConfig::getCustomSetting(const QString& key, const QVariant& defaultValue) const
{
    return d->customSettings.value(key, defaultValue);
}

void MeetingConfig::setOption(const QString& key, const QVariant& value)
{
    // 兼容方法，将选项设置为自定义设置
    setCustomSetting(key, value);
}

void MeetingConfig::setValue(const QString& key, const QVariant& value)
{
    // 兼容方法，将值设置为自定义设置
    setCustomSetting(key, value);
}

QVariantMap MeetingConfig::toVariantMap() const
{
    QVariantMap map;
    
    // 服务器设置
    map["defaultServer"] = d->defaultServer;
    map["serverList"] = d->serverList;
    
    // 用户设置
    map["defaultDisplayName"] = d->defaultDisplayName;
    map["defaultEmail"] = d->defaultEmail;
    
    // 会议设置
    map["autoJoin"] = d->autoJoin;
    map["defaultAudioEnabled"] = d->defaultAudioEnabled;
    map["defaultVideoEnabled"] = d->defaultVideoEnabled;
    map["audioQuality"] = static_cast<int>(d->audioQuality);
    map["videoQuality"] = static_cast<int>(d->videoQuality);
    
    // 认证设置
    map["authenticationMethod"] = static_cast<int>(d->authMethod);
    map["rememberAuthentication"] = d->rememberAuth;
    
    // 网络设置
    map["connectionTimeout"] = d->connectionTimeout;
    map["reconnectAttempts"] = d->reconnectAttempts;
    map["supportedProtocols"] = d->supportedProtocols;
    
    // 界面设置
    map["showJoinDialog"] = d->showJoinDialog;
    map["minimizeToTray"] = d->minimizeToTray;
    
    // 高级设置
    map["debugEnabled"] = d->debugEnabled;
    map["logLevel"] = d->logLevel;
    
    // 自定义设置
    for (auto it = d->customSettings.begin(); it != d->customSettings.end(); ++it) {
        map[it.key()] = it.value();
    }
    
    return map;
}

void MeetingConfig::fromVariantMap(const QVariantMap& map)
{
    // 服务器设置
    setDefaultServer(map.value("defaultServer", d->defaultServer).toString());
    setServerList(map.value("serverList", d->serverList).toStringList());
    
    // 用户设置
    setDefaultDisplayName(map.value("defaultDisplayName", d->defaultDisplayName).toString());
    setDefaultEmail(map.value("defaultEmail", d->defaultEmail).toString());
    
    // 会议设置
    setAutoJoin(map.value("autoJoin", d->autoJoin).toBool());
    setDefaultAudioEnabled(map.value("defaultAudioEnabled", d->defaultAudioEnabled).toBool());
    setDefaultVideoEnabled(map.value("defaultVideoEnabled", d->defaultVideoEnabled).toBool());
    setAudioQuality(static_cast<AudioQuality>(map.value("audioQuality", d->audioQuality).toInt()));
    setVideoQuality(static_cast<VideoQuality>(map.value("videoQuality", d->videoQuality).toInt()));
    
    // 认证设置
    setAuthenticationMethod(static_cast<AuthenticationMethod>(map.value("authenticationMethod", d->authMethod).toInt()));
    setRememberAuthentication(map.value("rememberAuthentication", d->rememberAuth).toBool());
    
    // 网络设置
    setConnectionTimeout(map.value("connectionTimeout", d->connectionTimeout).toInt());
    setReconnectAttempts(map.value("reconnectAttempts", d->reconnectAttempts).toInt());
    setSupportedProtocols(map.value("supportedProtocols", d->supportedProtocols).toStringList());
    
    // 界面设置
    setShowJoinDialog(map.value("showJoinDialog", d->showJoinDialog).toBool());
    setMinimizeToTray(map.value("minimizeToTray", d->minimizeToTray).toBool());
    
    // 高级设置
    setDebugEnabled(map.value("debugEnabled", d->debugEnabled).toBool());
    setLogLevel(map.value("logLevel", d->logLevel).toString());
}

void MeetingConfig::initializeDefaults()
{
    d->defaultServer = "meet.jit.si";
    d->serverList = {"meet.jit.si", "8x8.vc"};
    d->defaultDisplayName.clear();
    d->defaultEmail.clear();
    
    d->autoJoin = false;
    d->defaultAudioEnabled = true;
    d->defaultVideoEnabled = true;
    d->audioQuality = StandardAudio;
    d->videoQuality = StandardVideo;
    
    d->authMethod = GuestAuth;
    d->rememberAuth = false;
    
    d->connectionTimeout = 30000;
    d->reconnectAttempts = 3;
    d->supportedProtocols = {"https", "jitsi"};
    
    d->showJoinDialog = true;
    d->minimizeToTray = false;
    
    d->debugEnabled = false;
    d->logLevel = "info";
    
    d->customSettings.clear();
}

void MeetingConfig::emitConfigurationChanged(const QString& key, const QVariant& value)
{
    emit configurationChanged(key, value);
}