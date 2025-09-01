#include "SettingsManager.h"
#include "../interfaces/IConfigValidator.h"
#include "../storage/LocalStorage.h"
#include "../storage/CloudStorage.h"
#include "../storage/RegistryStorage.h"

#include <QDebug>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QDateTime>

class SettingsManager::Private
{
public:
    Private()
        : status(Uninitialized)
        , storageBackend(LocalFile)
        , syncStrategy(Manual)
        , encryptionEnabled(false)
        , fileWatchingEnabled(false)
        , validator(nullptr)
        , syncTimer(nullptr)
        , fileWatcher(nullptr)
        , batchMode(false)
        , syncInterval(30000)
    {
    }

    ManagerStatus status;
    StorageBackend storageBackend;
    SyncStrategy syncStrategy;
    bool encryptionEnabled;
    bool fileWatchingEnabled;
    QString encryptionKey;
    QString configPath;
    
    IConfigValidator* validator;
    QTimer* syncTimer;
    QFileSystemWatcher* fileWatcher;
    
    mutable QMutex mutex;
    bool batchMode;
    int syncInterval;
    
    QMap<SettingsScope, QSettings*> settingsMap;
    QVariantMap statistics;
    QVariantMap batchChanges;
    QVariantMap storageParameters;
};

SettingsManager::SettingsManager(QObject* parent)
    : ISettingsManager(parent)
    , d(std::make_unique<Private>())
{
    d->syncTimer = new QTimer(this);
    d->syncTimer->setSingleShot(false);
    connect(d->syncTimer, &QTimer::timeout, this, &SettingsManager::onSyncTimer);
    
    d->fileWatcher = new QFileSystemWatcher(this);
    connect(d->fileWatcher, &QFileSystemWatcher::fileChanged, 
            this, &SettingsManager::onFileChanged);
}

SettingsManager::~SettingsManager()
{
    QMutexLocker locker(&d->mutex);
    
    // Cleanup settings objects
    for (auto it = d->settingsMap.begin(); it != d->settingsMap.end(); ++it) {
        delete it.value();
    }
    d->settingsMap.clear();
}

SettingsManager* SettingsManager::instance()
{
    static SettingsManager* instance = nullptr;
    static QMutex mutex;
    
    QMutexLocker locker(&mutex);
    if (!instance) {
        instance = new SettingsManager();
    }
    return instance;
}

bool SettingsManager::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Uninitialized) {
        qWarning() << "SettingsManager: Already initialized";
        return d->status == Ready;
    }
    
    setStatus(Initializing);
    
    try {
        // Set default config path if not set
        if (d->configPath.isEmpty()) {
            d->configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
            QDir().mkpath(d->configPath);
        }
        
        // Initialize settings objects for each scope
        d->settingsMap[UserScope] = new QSettings(
            d->configPath + "/user_settings.ini", QSettings::IniFormat);
        d->settingsMap[SystemScope] = new QSettings(
            d->configPath + "/system_settings.ini", QSettings::IniFormat);
        d->settingsMap[ApplicationScope] = new QSettings(
            d->configPath + "/app_settings.ini", QSettings::IniFormat);
        
        // Setup file watching if enabled
        if (d->fileWatchingEnabled) {
            setupFileWatcher();
        }
        
        // Start sync timer if needed
        if (d->syncStrategy == Periodic) {
            d->syncTimer->start(d->syncInterval);
        }
        
        // Initialize statistics
        d->statistics["initialized_at"] = QDateTime::currentDateTime();
        d->statistics["read_count"] = 0;
        d->statistics["write_count"] = 0;
        d->statistics["sync_count"] = 0;
        
        setStatus(Ready);
        qDebug() << "SettingsManager: Initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "SettingsManager: Initialization failed:" << e.what();
        setStatus(Error);
        return false;
    }
}

ISettingsManager::ManagerStatus SettingsManager::status() const
{
    QMutexLocker locker(&d->mutex);
    return d->status;
}

void SettingsManager::setValue(const QString& key, const QVariant& value, SettingsScope scope)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Ready) {
        qWarning() << "SettingsManager: Not ready, cannot set value for key:" << key;
        return;
    }
    
    QVariant finalValue = d->encryptionEnabled ? encryptValue(value) : value;
    
    if (d->batchMode) {
        QString fullKey = scopeToString(scope) + "/" + key;
        d->batchChanges[fullKey] = finalValue;
        return;
    }
    
    QSettings* settings = getSettings(scope);
    if (settings) {
        settings->setValue(key, finalValue);
        updateStatistics("write");
        
        if (d->syncStrategy == OnChange) {
            performAutoSync();
        }
        
        emit valueChanged(key, value, scope);
    }
}

QVariant SettingsManager::value(const QString& key, const QVariant& defaultValue, SettingsScope scope) const
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Ready) {
        qWarning() << "SettingsManager: Not ready, cannot get value for key:" << key;
        return defaultValue;
    }
    
    QSettings* settings = getSettings(scope);
    if (!settings) {
        return defaultValue;
    }
    
    QVariant storedValue = settings->value(key, defaultValue);
    const_cast<SettingsManager*>(this)->updateStatistics("read");
    
    return d->encryptionEnabled ? decryptValue(storedValue) : storedValue;
}

bool SettingsManager::contains(const QString& key, SettingsScope scope) const
{
    QMutexLocker locker(&d->mutex);
    
    QSettings* settings = getSettings(scope);
    return settings ? settings->contains(key) : false;
}

void SettingsManager::remove(const QString& key, SettingsScope scope)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Ready) {
        return;
    }
    
    QSettings* settings = getSettings(scope);
    if (settings) {
        settings->remove(key);
        updateStatistics("remove");
        
        if (d->syncStrategy == OnChange) {
            performAutoSync();
        }
        
        emit valueChanged(key, QVariant(), scope);
    }
}

QStringList SettingsManager::allKeys(SettingsScope scope) const
{
    QMutexLocker locker(&d->mutex);
    
    QSettings* settings = getSettings(scope);
    return settings ? settings->allKeys() : QStringList();
}

QStringList SettingsManager::childKeys(const QString& group, SettingsScope scope) const
{
    QMutexLocker locker(&d->mutex);
    
    QSettings* settings = getSettings(scope);
    if (!settings) {
        return QStringList();
    }
    
    settings->beginGroup(group);
    QStringList keys = settings->childKeys();
    settings->endGroup();
    
    return keys;
}

QStringList SettingsManager::childGroups(const QString& group, SettingsScope scope) const
{
    QMutexLocker locker(&d->mutex);
    
    QSettings* settings = getSettings(scope);
    if (!settings) {
        return QStringList();
    }
    
    settings->beginGroup(group);
    QStringList groups = settings->childGroups();
    settings->endGroup();
    
    return groups;
}

bool SettingsManager::sync()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Ready) {
        return false;
    }
    
    setStatus(Syncing);
    
    try {
        // Sync all settings objects
        for (auto it = d->settingsMap.begin(); it != d->settingsMap.end(); ++it) {
            it.value()->sync();
        }
        
        updateStatistics("sync");
        setStatus(Ready);
        
        emit syncCompleted(true);
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "SettingsManager: Sync failed:" << e.what();
        setStatus(Error);
        emit syncCompleted(false);
        return false;
    }
}

bool SettingsManager::validate() const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->validator) {
        qWarning() << "SettingsManager: No validator set";
        return true; // No validator means validation passes
    }
    
    // Collect all settings for validation
    QVariantMap allSettings;
    for (auto it = d->settingsMap.begin(); it != d->settingsMap.end(); ++it) {
        QString scopeName = scopeToString(it.key());
        QStringList keys = it.value()->allKeys();
        
        for (const QString& key : keys) {
            QString fullKey = scopeName + "/" + key;
            QVariant value = it.value()->value(key);
            allSettings[fullKey] = d->encryptionEnabled ? decryptValue(value) : value;
        }
    }
    
    return d->validator->validateConfig(allSettings).isEmpty();
}

void SettingsManager::reset(SettingsScope scope)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Ready) {
        return;
    }
    
    QSettings* settings = getSettings(scope);
    if (settings) {
        settings->clear();
        settings->sync();
        
        updateStatistics("reset");
        emit settingsReset(scope);
    }
}

void SettingsManager::resetGroup(const QString& group, SettingsScope scope)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Ready) {
        return;
    }
    
    QSettings* settings = getSettings(scope);
    if (settings) {
        settings->beginGroup(group);
        settings->remove("");
        settings->endGroup();
        settings->sync();
        
        updateStatistics("reset_group");
    }
}

bool SettingsManager::exportSettings(const QString& filePath, SettingsScope scope) const
{
    QMutexLocker locker(&d->mutex);
    
    QSettings* settings = getSettings(scope);
    if (!settings) {
        return false;
    }
    
    try {
        QJsonObject jsonObj;
        QStringList keys = settings->allKeys();
        
        for (const QString& key : keys) {
            QVariant value = settings->value(key);
            if (d->encryptionEnabled) {
                value = decryptValue(value);
            }
            jsonObj[key] = QJsonValue::fromVariant(value);
        }
        
        QJsonDocument doc(jsonObj);
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            return true;
        }
        
    } catch (const std::exception& e) {
        qCritical() << "SettingsManager: Export failed:" << e.what();
    }
    
    return false;
}

bool SettingsManager::importSettings(const QString& filePath, SettingsScope scope)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Ready) {
        return false;
    }
    
    try {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject jsonObj = doc.object();
        
        QSettings* settings = getSettings(scope);
        if (!settings) {
            return false;
        }
        
        for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
            QVariant value = it.value().toVariant();
            if (d->encryptionEnabled) {
                value = encryptValue(value);
            }
            settings->setValue(it.key(), value);
        }
        
        settings->sync();
        updateStatistics("import");
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "SettingsManager: Import failed:" << e.what();
        return false;
    }
}

// Extended functionality implementation

void SettingsManager::setStorageBackend(StorageBackend backend, const QVariantMap& parameters)
{
    QMutexLocker locker(&d->mutex);
    d->storageBackend = backend;
    d->storageParameters = parameters;
}

SettingsManager::StorageBackend SettingsManager::storageBackend() const
{
    QMutexLocker locker(&d->mutex);
    return d->storageBackend;
}

void SettingsManager::setValidator(IConfigValidator* validator)
{
    QMutexLocker locker(&d->mutex);
    d->validator = validator;
}

IConfigValidator* SettingsManager::validator() const
{
    QMutexLocker locker(&d->mutex);
    return d->validator;
}

void SettingsManager::setSyncStrategy(SyncStrategy strategy, int interval)
{
    QMutexLocker locker(&d->mutex);
    d->syncStrategy = strategy;
    d->syncInterval = interval;
    
    if (strategy == Periodic) {
        d->syncTimer->start(interval);
    } else {
        d->syncTimer->stop();
    }
}

SettingsManager::SyncStrategy SettingsManager::syncStrategy() const
{
    QMutexLocker locker(&d->mutex);
    return d->syncStrategy;
}

void SettingsManager::setEncryption(bool enabled, const QString& key)
{
    QMutexLocker locker(&d->mutex);
    d->encryptionEnabled = enabled;
    d->encryptionKey = key.isEmpty() ? "default_key" : key;
}

bool SettingsManager::isEncryptionEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->encryptionEnabled;
}

void SettingsManager::setConfigPath(const QString& path)
{
    QMutexLocker locker(&d->mutex);
    d->configPath = path;
}

QString SettingsManager::configPath() const
{
    QMutexLocker locker(&d->mutex);
    return d->configPath;
}

void SettingsManager::setFileWatchingEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    d->fileWatchingEnabled = enabled;
    
    if (enabled && d->status == Ready) {
        setupFileWatcher();
    }
}

bool SettingsManager::isFileWatchingEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->fileWatchingEnabled;
}

QVariantMap SettingsManager::statistics() const
{
    QMutexLocker locker(&d->mutex);
    return d->statistics;
}

void SettingsManager::clearCache()
{
    QMutexLocker locker(&d->mutex);
    // Implementation would clear any internal caches
}

void SettingsManager::beginBatch()
{
    QMutexLocker locker(&d->mutex);
    d->batchMode = true;
    d->batchChanges.clear();
}

void SettingsManager::endBatch(bool commit)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->batchMode) {
        return;
    }
    
    if (commit) {
        // Apply all batch changes
        for (auto it = d->batchChanges.begin(); it != d->batchChanges.end(); ++it) {
            QStringList parts = it.key().split("/");
            if (parts.size() >= 2) {
                QString scopeStr = parts[0];
                QString key = parts.mid(1).join("/");
                
                SettingsScope scope = UserScope;
                if (scopeStr == "system") scope = SystemScope;
                else if (scopeStr == "application") scope = ApplicationScope;
                
                QSettings* settings = getSettings(scope);
                if (settings) {
                    settings->setValue(key, it.value());
                }
            }
        }
        
        // Sync all changes
        sync();
    }
    
    d->batchMode = false;
    d->batchChanges.clear();
}

bool SettingsManager::isBatchMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->batchMode;
}

// Public slots

void SettingsManager::forceSync()
{
    sync();
}

void SettingsManager::reload()
{
    QMutexLocker locker(&d->mutex);
    
    for (auto it = d->settingsMap.begin(); it != d->settingsMap.end(); ++it) {
        it.value()->sync();
    }
}

void SettingsManager::backup(const QString& backupPath)
{
    // Implementation would create a backup of all settings
    Q_UNUSED(backupPath)
}

void SettingsManager::restore(const QString& backupPath)
{
    // Implementation would restore settings from backup
    Q_UNUSED(backupPath)
}

// Private slots

void SettingsManager::onSyncTimer()
{
    if (d->syncStrategy == Periodic) {
        sync();
    }
}

void SettingsManager::onFileChanged(const QString& path)
{
    Q_UNUSED(path)
    // Implementation would handle file changes
    reload();
}

void SettingsManager::onValidationCompleted(bool success, const QStringList& errors)
{
    emit validationCompleted(success, errors);
}

// Private methods

void SettingsManager::setStatus(ManagerStatus newStatus)
{
    if (d->status != newStatus) {
        d->status = newStatus;
        emit statusChanged(newStatus);
    }
}

QSettings* SettingsManager::getSettings(SettingsScope scope) const
{
    return d->settingsMap.value(scope, nullptr);
}

QString SettingsManager::scopeToString(SettingsScope scope) const
{
    switch (scope) {
        case UserScope: return "user";
        case SystemScope: return "system";
        case ApplicationScope: return "application";
        default: return "user";
    }
}

void SettingsManager::setupFileWatcher()
{
    if (d->fileWatcher && !d->configPath.isEmpty()) {
        d->fileWatcher->addPath(d->configPath);
    }
}

void SettingsManager::performAutoSync()
{
    if (d->syncStrategy == OnChange || d->syncStrategy == Automatic) {
        QTimer::singleShot(100, this, &SettingsManager::sync);
    }
}

QVariant SettingsManager::encryptValue(const QVariant& value) const
{
    if (!d->encryptionEnabled || d->encryptionKey.isEmpty()) {
        return value;
    }
    
    // Simple encryption implementation
    QByteArray data = value.toByteArray();
    QByteArray key = d->encryptionKey.toUtf8();
    
    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ key[i % key.size()];
    }
    
    return data.toBase64();
}

QVariant SettingsManager::decryptValue(const QVariant& value) const
{
    if (!d->encryptionEnabled || d->encryptionKey.isEmpty()) {
        return value;
    }
    
    // Simple decryption implementation
    QByteArray data = QByteArray::fromBase64(value.toByteArray());
    QByteArray key = d->encryptionKey.toUtf8();
    
    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ key[i % key.size()];
    }
    
    return QVariant(data);
}

void SettingsManager::updateStatistics(const QString& operation)
{
    QString countKey = operation + "_count";
    int currentCount = d->statistics.value(countKey, 0).toInt();
    d->statistics[countKey] = currentCount + 1;
    d->statistics["last_" + operation] = QDateTime::currentDateTime();
}