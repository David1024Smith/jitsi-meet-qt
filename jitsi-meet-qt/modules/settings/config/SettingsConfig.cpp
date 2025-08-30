#include "SettingsConfig.h"

#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

class SettingsConfig::Private
{
public:
    Private()
        : configVersion("1.0.0")
        , storageBackend("local")
        , encryptionEnabled(false)
        , validationEnabled(true)
        , autoSyncEnabled(true)
        , syncInterval(30)
        , configFormat(JsonFormat)
        , encryptionType(NoEncryption)
        , maxBackupCount(5)
        , cacheSizeLimit(100)
        , logLevel("info")
        , debugMode(false)
        , cloudSyncInterval(300)
        , strictValidation(false)
    {
    }

    QString configVersion;
    QString storageBackend;
    QString configPath;
    bool encryptionEnabled;
    bool validationEnabled;
    bool autoSyncEnabled;
    int syncInterval;
    
    ConfigFormat configFormat;
    EncryptionType encryptionType;
    QString encryptionKey;
    QString backupDirectory;
    int maxBackupCount;
    int cacheSizeLimit;
    QString logLevel;
    bool debugMode;
    
    QString cloudServerUrl;
    QString cloudAuthToken;
    int cloudSyncInterval;
    
    QString validationRulesPath;
    bool strictValidation;
    
    QMutex mutex;
};

SettingsConfig::SettingsConfig(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    initializeDefaults();
}

SettingsConfig::~SettingsConfig() = default;

SettingsConfig* SettingsConfig::instance()
{
    static SettingsConfig* instance = nullptr;
    static QMutex mutex;
    
    QMutexLocker locker(&mutex);
    if (!instance) {
        instance = new SettingsConfig();
    }
    return instance;
}

QString SettingsConfig::configVersion() const
{
    QMutexLocker locker(&d->mutex);
    return d->configVersion;
}

void SettingsConfig::setConfigVersion(const QString& version)
{
    QMutexLocker locker(&d->mutex);
    if (d->configVersion != version) {
        d->configVersion = version;
        emit configVersionChanged(version);
    }
}

QString SettingsConfig::storageBackend() const
{
    QMutexLocker locker(&d->mutex);
    return d->storageBackend;
}

void SettingsConfig::setStorageBackend(const QString& backend)
{
    QMutexLocker locker(&d->mutex);
    if (d->storageBackend != backend) {
        d->storageBackend = backend;
        emit storageBackendChanged(backend);
    }
}

void SettingsConfig::setStorageBackend(StorageBackendType backend)
{
    setStorageBackend(backendTypeToString(backend));
}

SettingsConfig::StorageBackendType SettingsConfig::storageBackendType() const
{
    QMutexLocker locker(&d->mutex);
    return stringToBackendType(d->storageBackend);
}

QString SettingsConfig::configPath() const
{
    QMutexLocker locker(&d->mutex);
    return d->configPath.isEmpty() ? getDefaultConfigPath() : d->configPath;
}

void SettingsConfig::setConfigPath(const QString& path)
{
    QMutexLocker locker(&d->mutex);
    if (d->configPath != path) {
        d->configPath = path;
        emit configPathChanged(path);
    }
}

bool SettingsConfig::isEncryptionEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->encryptionEnabled;
}

void SettingsConfig::setEncryptionEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    if (d->encryptionEnabled != enabled) {
        d->encryptionEnabled = enabled;
        emit encryptionEnabledChanged(enabled);
    }
}

bool SettingsConfig::isValidationEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->validationEnabled;
}

void SettingsConfig::setValidationEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    if (d->validationEnabled != enabled) {
        d->validationEnabled = enabled;
        emit validationEnabledChanged(enabled);
    }
}

bool SettingsConfig::isAutoSyncEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->autoSyncEnabled;
}

void SettingsConfig::setAutoSyncEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    if (d->autoSyncEnabled != enabled) {
        d->autoSyncEnabled = enabled;
        emit autoSyncEnabledChanged(enabled);
    }
}

int SettingsConfig::syncInterval() const
{
    QMutexLocker locker(&d->mutex);
    return d->syncInterval;
}

void SettingsConfig::setSyncInterval(int interval)
{
    QMutexLocker locker(&d->mutex);
    if (d->syncInterval != interval) {
        d->syncInterval = interval;
        emit syncIntervalChanged(interval);
    }
}

bool SettingsConfig::loadConfiguration(const QString& filePath)
{
    QString path = filePath.isEmpty() ? configPath() : filePath;
    
    QJsonObject json = loadJsonFromFile(path);
    if (json.isEmpty()) {
        qWarning() << "SettingsConfig: Failed to load configuration from" << path;
        return false;
    }
    
    bool success = fromJson(json);
    if (success) {
        emit configurationLoaded(true);
        qDebug() << "SettingsConfig: Configuration loaded successfully from" << path;
    } else {
        emit configurationLoaded(false);
        qWarning() << "SettingsConfig: Failed to parse configuration from" << path;
    }
    
    return success;
}

bool SettingsConfig::saveConfiguration(const QString& filePath) const
{
    QString path = filePath.isEmpty() ? configPath() : filePath;
    
    if (!createConfigDirectory()) {
        return false;
    }
    
    QJsonObject json = toJson();
    bool success = saveJsonToFile(json, path);
    
    if (success) {
        emit configurationSaved(true);
        qDebug() << "SettingsConfig: Configuration saved successfully to" << path;
    } else {
        emit configurationSaved(false);
        qWarning() << "SettingsConfig: Failed to save configuration to" << path;
    }
    
    return success;
}

QJsonObject SettingsConfig::toJson() const
{
    QMutexLocker locker(&d->mutex);
    
    QJsonObject json;
    json["version"] = d->configVersion;
    json["storageBackend"] = d->storageBackend;
    json["configPath"] = d->configPath;
    json["encryptionEnabled"] = d->encryptionEnabled;
    json["validationEnabled"] = d->validationEnabled;
    json["autoSyncEnabled"] = d->autoSyncEnabled;
    json["syncInterval"] = d->syncInterval;
    
    // Advanced settings
    json["configFormat"] = formatToString(d->configFormat);
    json["encryptionType"] = encryptionTypeToString(d->encryptionType);
    json["backupDirectory"] = d->backupDirectory;
    json["maxBackupCount"] = d->maxBackupCount;
    json["cacheSizeLimit"] = d->cacheSizeLimit;
    json["logLevel"] = d->logLevel;
    json["debugMode"] = d->debugMode;
    
    // Cloud settings
    QJsonObject cloudSettings;
    cloudSettings["serverUrl"] = d->cloudServerUrl;
    cloudSettings["syncInterval"] = d->cloudSyncInterval;
    json["cloud"] = cloudSettings;
    
    // Validation settings
    QJsonObject validationSettings;
    validationSettings["rulesPath"] = d->validationRulesPath;
    validationSettings["strictMode"] = d->strictValidation;
    json["validation"] = validationSettings;
    
    return json;
}

bool SettingsConfig::fromJson(const QJsonObject& json)
{
    QMutexLocker locker(&d->mutex);
    
    d->configVersion = json["version"].toString("1.0.0");
    d->storageBackend = json["storageBackend"].toString("local");
    d->configPath = json["configPath"].toString();
    d->encryptionEnabled = json["encryptionEnabled"].toBool(false);
    d->validationEnabled = json["validationEnabled"].toBool(true);
    d->autoSyncEnabled = json["autoSyncEnabled"].toBool(true);
    d->syncInterval = json["syncInterval"].toInt(30);
    
    // Advanced settings
    d->configFormat = stringToFormat(json["configFormat"].toString("json"));
    d->encryptionType = stringToEncryptionType(json["encryptionType"].toString("none"));
    d->backupDirectory = json["backupDirectory"].toString();
    d->maxBackupCount = json["maxBackupCount"].toInt(5);
    d->cacheSizeLimit = json["cacheSizeLimit"].toInt(100);
    d->logLevel = json["logLevel"].toString("info");
    d->debugMode = json["debugMode"].toBool(false);
    
    // Cloud settings
    QJsonObject cloudSettings = json["cloud"].toObject();
    d->cloudServerUrl = cloudSettings["serverUrl"].toString();
    d->cloudSyncInterval = cloudSettings["syncInterval"].toInt(300);
    
    // Validation settings
    QJsonObject validationSettings = json["validation"].toObject();
    d->validationRulesPath = validationSettings["rulesPath"].toString();
    d->strictValidation = validationSettings["strictMode"].toBool(false);
    
    return true;
}

void SettingsConfig::initializeDefaults()
{
    // Set default paths
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    d->configPath = QDir(appDataPath).filePath("settings.json");
    d->backupDirectory = QDir(appDataPath).filePath("backups");
    d->validationRulesPath = QDir(appDataPath).filePath("validation_rules.json");
}

QString SettingsConfig::getDefaultConfigPath() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(appDataPath).filePath("settings.json");
}

bool SettingsConfig::createConfigDirectory() const
{
    QDir dir(QFileInfo(configPath()).absolutePath());
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

QJsonObject SettingsConfig::loadJsonFromFile(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonObject();
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return QJsonObject();
    }
    
    return doc.object();
}

bool SettingsConfig::saveJsonToFile(const QJsonObject& json, const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(json);
    return file.write(doc.toJson()) != -1;
}

QString SettingsConfig::backendTypeToString(StorageBackendType type) const
{
    switch (type) {
        case LocalFileBackend: return "local";
        case RegistryBackend: return "registry";
        case CloudBackend: return "cloud";
        case DatabaseBackend: return "database";
        case MemoryBackend: return "memory";
        default: return "local";
    }
}

SettingsConfig::StorageBackendType SettingsConfig::stringToBackendType(const QString& str) const
{
    if (str == "registry") return RegistryBackend;
    if (str == "cloud") return CloudBackend;
    if (str == "database") return DatabaseBackend;
    if (str == "memory") return MemoryBackend;
    return LocalFileBackend;
}

QString SettingsConfig::formatToString(ConfigFormat format) const
{
    switch (format) {
        case JsonFormat: return "json";
        case IniFormat: return "ini";
        case XmlFormat: return "xml";
        case BinaryFormat: return "binary";
        default: return "json";
    }
}

SettingsConfig::ConfigFormat SettingsConfig::stringToFormat(const QString& str) const
{
    if (str == "ini") return IniFormat;
    if (str == "xml") return XmlFormat;
    if (str == "binary") return BinaryFormat;
    return JsonFormat;
}

QString SettingsConfig::encryptionTypeToString(EncryptionType type) const
{
    switch (type) {
        case NoEncryption: return "none";
        case AESEncryption: return "aes";
        case RSAEncryption: return "rsa";
        case CustomEncryption: return "custom";
        default: return "none";
    }
}

SettingsConfig::EncryptionType SettingsConfig::stringToEncryptionType(const QString& str) const
{
    if (str == "aes") return AESEncryption;
    if (str == "rsa") return RSAEncryption;
    if (str == "custom") return CustomEncryption;
    return NoEncryption;
}

// Additional methods implementation
SettingsConfig::ConfigFormat SettingsConfig::configFormat() const
{
    QMutexLocker locker(&d->mutex);
    return d->configFormat;
}

void SettingsConfig::setConfigFormat(ConfigFormat format)
{
    QMutexLocker locker(&d->mutex);
    if (d->configFormat != format) {
        d->configFormat = format;
    }
}

SettingsConfig::EncryptionType SettingsConfig::encryptionType() const
{
    QMutexLocker locker(&d->mutex);
    return d->encryptionType;
}

void SettingsConfig::setEncryptionType(EncryptionType type)
{
    QMutexLocker locker(&d->mutex);
    if (d->encryptionType != type) {
        d->encryptionType = type;
    }
}

QString SettingsConfig::encryptionKey() const
{
    QMutexLocker locker(&d->mutex);
    return d->encryptionKey;
}

void SettingsConfig::setEncryptionKey(const QString& key)
{
    QMutexLocker locker(&d->mutex);
    d->encryptionKey = key;
}

QString SettingsConfig::backupDirectory() const
{
    QMutexLocker locker(&d->mutex);
    return d->backupDirectory;
}

void SettingsConfig::setBackupDirectory(const QString& directory)
{
    QMutexLocker locker(&d->mutex);
    d->backupDirectory = directory;
}

int SettingsConfig::maxBackupCount() const
{
    QMutexLocker locker(&d->mutex);
    return d->maxBackupCount;
}

void SettingsConfig::setMaxBackupCount(int count)
{
    QMutexLocker locker(&d->mutex);
    d->maxBackupCount = count;
}

int SettingsConfig::cacheSizeLimit() const
{
    QMutexLocker locker(&d->mutex);
    return d->cacheSizeLimit;
}

void SettingsConfig::setCacheSizeLimit(int sizeMB)
{
    QMutexLocker locker(&d->mutex);
    d->cacheSizeLimit = sizeMB;
}

QString SettingsConfig::logLevel() const
{
    QMutexLocker locker(&d->mutex);
    return d->logLevel;
}

void SettingsConfig::setLogLevel(const QString& level)
{
    QMutexLocker locker(&d->mutex);
    d->logLevel = level;
}

bool SettingsConfig::isDebugMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->debugMode;
}

void SettingsConfig::setDebugMode(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    d->debugMode = enabled;
}

QString SettingsConfig::cloudServerUrl() const
{
    QMutexLocker locker(&d->mutex);
    return d->cloudServerUrl;
}

void SettingsConfig::setCloudServerUrl(const QString& url)
{
    QMutexLocker locker(&d->mutex);
    d->cloudServerUrl = url;
}

QString SettingsConfig::cloudAuthToken() const
{
    QMutexLocker locker(&d->mutex);
    return d->cloudAuthToken;
}

void SettingsConfig::setCloudAuthToken(const QString& token)
{
    QMutexLocker locker(&d->mutex);
    d->cloudAuthToken = token;
}

int SettingsConfig::cloudSyncInterval() const
{
    QMutexLocker locker(&d->mutex);
    return d->cloudSyncInterval;
}

void SettingsConfig::setCloudSyncInterval(int interval)
{
    QMutexLocker locker(&d->mutex);
    d->cloudSyncInterval = interval;
}

QString SettingsConfig::validationRulesPath() const
{
    QMutexLocker locker(&d->mutex);
    return d->validationRulesPath;
}

void SettingsConfig::setValidationRulesPath(const QString& path)
{
    QMutexLocker locker(&d->mutex);
    d->validationRulesPath = path;
}

bool SettingsConfig::isStrictValidation() const
{
    QMutexLocker locker(&d->mutex);
    return d->strictValidation;
}

void SettingsConfig::setStrictValidation(bool strict)
{
    QMutexLocker locker(&d->mutex);
    d->strictValidation = strict;
}

QPair<bool, QStringList> SettingsConfig::validateConfiguration() const
{
    QStringList errors;
    
    // Basic validation
    if (d->configVersion.isEmpty()) {
        errors << "Configuration version is empty";
    }
    
    if (d->syncInterval < 1) {
        errors << "Sync interval must be greater than 0";
    }
    
    if (d->maxBackupCount < 0) {
        errors << "Max backup count cannot be negative";
    }
    
    if (d->cacheSizeLimit < 1) {
        errors << "Cache size limit must be greater than 0";
    }
    
    // Path validation
    if (!d->configPath.isEmpty() && !QDir().exists(QFileInfo(d->configPath).absolutePath())) {
        errors << "Configuration directory does not exist: " + QFileInfo(d->configPath).absolutePath();
    }
    
    if (!d->backupDirectory.isEmpty() && !QDir().exists(d->backupDirectory)) {
        errors << "Backup directory does not exist: " + d->backupDirectory;
    }
    
    // Cloud settings validation
    if (d->storageBackend == "cloud") {
        if (d->cloudServerUrl.isEmpty()) {
            errors << "Cloud server URL is required for cloud backend";
        }
        if (d->cloudSyncInterval < 60) {
            errors << "Cloud sync interval must be at least 60 seconds";
        }
    }
    
    // Encryption validation
    if (d->encryptionEnabled && d->encryptionType != NoEncryption && d->encryptionKey.isEmpty()) {
        errors << "Encryption key is required when encryption is enabled";
    }
    
    bool isValid = errors.isEmpty();
    emit configurationValidated(isValid, errors);
    
    return qMakePair(isValid, errors);
}

void SettingsConfig::resetToDefaults()
{
    QMutexLocker locker(&d->mutex);
    
    d->configVersion = "1.0.0";
    d->storageBackend = "local";
    d->configPath.clear();
    d->encryptionEnabled = false;
    d->validationEnabled = true;
    d->autoSyncEnabled = true;
    d->syncInterval = 30;
    d->configFormat = JsonFormat;
    d->encryptionType = NoEncryption;
    d->encryptionKey.clear();
    d->maxBackupCount = 5;
    d->cacheSizeLimit = 100;
    d->logLevel = "info";
    d->debugMode = false;
    d->cloudServerUrl.clear();
    d->cloudAuthToken.clear();
    d->cloudSyncInterval = 300;
    d->validationRulesPath.clear();
    d->strictValidation = false;
    
    initializeDefaults();
    emit configurationReset();
}

QJsonObject SettingsConfig::defaultConfiguration()
{
    QJsonObject defaults;
    defaults["version"] = "1.0.0";
    defaults["storageBackend"] = "local";
    defaults["encryptionEnabled"] = false;
    defaults["validationEnabled"] = true;
    defaults["autoSyncEnabled"] = true;
    defaults["syncInterval"] = 30;
    defaults["configFormat"] = "json";
    defaults["encryptionType"] = "none";
    defaults["maxBackupCount"] = 5;
    defaults["cacheSizeLimit"] = 100;
    defaults["logLevel"] = "info";
    defaults["debugMode"] = false;
    
    QJsonObject cloudSettings;
    cloudSettings["syncInterval"] = 300;
    defaults["cloud"] = cloudSettings;
    
    QJsonObject validationSettings;
    validationSettings["strictMode"] = false;
    defaults["validation"] = validationSettings;
    
    return defaults;
}

QVariantMap SettingsConfig::configurationSummary() const
{
    QMutexLocker locker(&d->mutex);
    
    QVariantMap summary;
    summary["version"] = d->configVersion;
    summary["backend"] = d->storageBackend;
    summary["encryption"] = d->encryptionEnabled;
    summary["validation"] = d->validationEnabled;
    summary["autoSync"] = d->autoSyncEnabled;
    summary["format"] = formatToString(d->configFormat);
    summary["debugMode"] = d->debugMode;
    
    return summary;
}

bool SettingsConfig::isCompatibleWith(const SettingsConfig& otherConfig) const
{
    // Check version compatibility
    QString thisVersion = configVersion();
    QString otherVersion = otherConfig.configVersion();
    
    // Simple version check - same major version
    QStringList thisParts = thisVersion.split('.');
    QStringList otherParts = otherVersion.split('.');
    
    if (thisParts.isEmpty() || otherParts.isEmpty()) {
        return false;
    }
    
    return thisParts[0] == otherParts[0];
}

void SettingsConfig::mergeConfiguration(const SettingsConfig& otherConfig, bool overwrite)
{
    QMutexLocker locker(&d->mutex);
    
    if (overwrite || d->storageBackend.isEmpty()) {
        d->storageBackend = otherConfig.storageBackend();
    }
    
    if (overwrite || d->configPath.isEmpty()) {
        d->configPath = otherConfig.configPath();
    }
    
    if (overwrite) {
        d->encryptionEnabled = otherConfig.isEncryptionEnabled();
        d->validationEnabled = otherConfig.isValidationEnabled();
        d->autoSyncEnabled = otherConfig.isAutoSyncEnabled();
        d->syncInterval = otherConfig.syncInterval();
        d->configFormat = otherConfig.configFormat();
        d->encryptionType = otherConfig.encryptionType();
        d->maxBackupCount = otherConfig.maxBackupCount();
        d->cacheSizeLimit = otherConfig.cacheSizeLimit();
        d->logLevel = otherConfig.logLevel();
        d->debugMode = otherConfig.isDebugMode();
        d->cloudSyncInterval = otherConfig.cloudSyncInterval();
        d->strictValidation = otherConfig.isStrictValidation();
    }
}

QStringList SettingsConfig::configurationDifferences(const SettingsConfig& otherConfig) const
{
    QStringList differences;
    
    if (configVersion() != otherConfig.configVersion()) {
        differences << QString("Version: %1 vs %2").arg(configVersion(), otherConfig.configVersion());
    }
    
    if (storageBackend() != otherConfig.storageBackend()) {
        differences << QString("Storage Backend: %1 vs %2").arg(storageBackend(), otherConfig.storageBackend());
    }
    
    if (isEncryptionEnabled() != otherConfig.isEncryptionEnabled()) {
        differences << QString("Encryption: %1 vs %2").arg(isEncryptionEnabled() ? "enabled" : "disabled", 
                                                           otherConfig.isEncryptionEnabled() ? "enabled" : "disabled");
    }
    
    if (syncInterval() != otherConfig.syncInterval()) {
        differences << QString("Sync Interval: %1 vs %2").arg(syncInterval()).arg(otherConfig.syncInterval());
    }
    
    return differences;
}

void SettingsConfig::reloadConfiguration()
{
    loadConfiguration();
}

void SettingsConfig::applyChanges()
{
    saveConfiguration();
}