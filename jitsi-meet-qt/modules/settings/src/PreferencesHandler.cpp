#include "PreferencesHandler.h"
#include "SettingsManager.h"

#include <QDebug>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDateTime>

class PreferencesHandler::Private
{
public:
    Private()
        : settingsManager(nullptr)
        , storageFormat(JsonFormat)
        , conflictResolution(KeepLocal)
        , autoBackupEnabled(false)
        , autoBackupInterval(60)
        , currentProfileName("default")
        , defaultProfileName("default")
    {
    }

    SettingsManager* settingsManager;
    StorageFormat storageFormat;
    ConflictResolution conflictResolution;
    bool autoBackupEnabled;
    int autoBackupInterval;
    QString currentProfileName;
    QString defaultProfileName;
    
    mutable QMutex mutex;
    QTimer* autoBackupTimer;
    
    QMap<QString, QVariantMap> categoryDefaults;
    QMap<QString, QVariantMap> categoryCache;
    QStringList availableCategories;
};

PreferencesHandler::PreferencesHandler(QObject* parent)
    : IPreferencesHandler(parent)
    , d(std::make_unique<Private>())
{
    d->autoBackupTimer = new QTimer(this);
    d->autoBackupTimer->setSingleShot(false);
    connect(d->autoBackupTimer, &QTimer::timeout, this, &PreferencesHandler::onAutoBackupTimer);
    
    // Initialize available categories
    d->availableCategories << "audio" << "video" << "ui" << "network" 
                          << "security" << "performance" << "custom";
}

PreferencesHandler::~PreferencesHandler()
{
}

PreferencesHandler* PreferencesHandler::instance()
{
    static PreferencesHandler* instance = nullptr;
    static QMutex mutex;
    
    QMutexLocker locker(&mutex);
    if (!instance) {
        instance = new PreferencesHandler();
    }
    return instance;
}

bool PreferencesHandler::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    try {
        // Create default categories if they don't exist
        createDefaultCategories();
        
        // Load current profile
        loadProfile(d->currentProfileName);
        
        // Migrate old preferences if needed
        migrateOldPreferences();
        
        qDebug() << "PreferencesHandler: Initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "PreferencesHandler: Initialization failed:" << e.what();
        return false;
    }
}

void PreferencesHandler::setPreference(PreferenceCategory category, const QString& key, 
                                     const QVariant& value, PreferencePriority priority)
{
    setPreference(categoryToString(category), key, value, priority);
}

void PreferencesHandler::setPreference(const QString& category, const QString& key, 
                                     const QVariant& value, PreferencePriority priority)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        qWarning() << "PreferencesHandler: No settings manager set";
        return;
    }
    
    QString prefKey = getPreferenceKey(category, key);
    QString priorityKey = getMetaKey(category, key, "priority");
    QString statusKey = getMetaKey(category, key, "status");
    
    // Set the preference value
    d->settingsManager->setValue(prefKey, value);
    
    // Set metadata
    d->settingsManager->setValue(priorityKey, priorityToString(priority));
    d->settingsManager->setValue(statusKey, statusToString(Modified));
    
    // Update cache
    if (!d->categoryCache.contains(category)) {
        d->categoryCache[category] = QVariantMap();
    }
    d->categoryCache[category][key] = value;
    
    emit preferenceChanged(category, key, value);
}

QVariant PreferencesHandler::preference(PreferenceCategory category, const QString& key, 
                                      const QVariant& defaultValue) const
{
    return preference(categoryToString(category), key, defaultValue);
}

QVariant PreferencesHandler::preference(const QString& category, const QString& key, 
                                      const QVariant& defaultValue) const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return defaultValue;
    }
    
    // Check cache first
    if (d->categoryCache.contains(category) && d->categoryCache[category].contains(key)) {
        return d->categoryCache[category][key];
    }
    
    QString prefKey = getPreferenceKey(category, key);
    QVariant value = d->settingsManager->value(prefKey, defaultValue);
    
    // Update cache
    if (!d->categoryCache.contains(category)) {
        d->categoryCache[category] = QVariantMap();
    }
    d->categoryCache[category][key] = value;
    
    return value;
}

QStringList PreferencesHandler::categories() const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return d->availableCategories;
    }
    
    QStringList allKeys = d->settingsManager->allKeys();
    QStringList categories;
    
    for (const QString& key : allKeys) {
        if (key.startsWith("preferences/")) {
            QStringList parts = key.split("/");
            if (parts.size() >= 3) {
                QString category = parts[1];
                if (!categories.contains(category)) {
                    categories.append(category);
                }
            }
        }
    }
    
    return categories;
}

QStringList PreferencesHandler::keys(const QString& category) const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return QStringList();
    }
    
    QString groupKey = "preferences/" + category;
    return d->settingsManager->childKeys(groupKey);
}

IPreferencesHandler::PreferenceStatus PreferencesHandler::preferenceStatus(const QString& category, const QString& key) const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return Default;
    }
    
    QString statusKey = getMetaKey(category, key, "status");
    QString statusStr = d->settingsManager->value(statusKey, "default").toString();
    
    return stringToStatus(statusStr);
}

bool PreferencesHandler::hasPreference(const QString& category, const QString& key) const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return false;
    }
    
    QString prefKey = getPreferenceKey(category, key);
    return d->settingsManager->contains(prefKey);
}

void PreferencesHandler::removePreference(const QString& category, const QString& key)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return;
    }
    
    QString prefKey = getPreferenceKey(category, key);
    QString priorityKey = getMetaKey(category, key, "priority");
    QString statusKey = getMetaKey(category, key, "status");
    
    d->settingsManager->remove(prefKey);
    d->settingsManager->remove(priorityKey);
    d->settingsManager->remove(statusKey);
    
    // Update cache
    if (d->categoryCache.contains(category)) {
        d->categoryCache[category].remove(key);
    }
    
    emit preferenceChanged(category, key, QVariant());
}

void PreferencesHandler::resetCategory(const QString& category)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return;
    }
    
    QString groupKey = "preferences/" + category;
    d->settingsManager->resetGroup(groupKey);
    
    // Clear cache for this category
    d->categoryCache.remove(category);
    
    // Restore defaults if available
    if (d->categoryDefaults.contains(category)) {
        setCategoryPreferences(category, d->categoryDefaults[category]);
    }
    
    emit categoryReset(category);
}

void PreferencesHandler::resetAll()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return;
    }
    
    d->settingsManager->resetGroup("preferences");
    d->categoryCache.clear();
    
    // Restore all defaults
    for (auto it = d->categoryDefaults.begin(); it != d->categoryDefaults.end(); ++it) {
        setCategoryPreferences(it.key(), it.value());
    }
    
    emit allPreferencesReset();
}

QVariantMap PreferencesHandler::categoryPreferences(const QString& category) const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return QVariantMap();
    }
    
    QVariantMap preferences;
    QStringList keyList = keys(category);
    
    for (const QString& key : keyList) {
        preferences[key] = preference(category, key);
    }
    
    return preferences;
}

void PreferencesHandler::setCategoryPreferences(const QString& category, const QVariantMap& preferences)
{
    QMutexLocker locker(&d->mutex);
    
    for (auto it = preferences.begin(); it != preferences.end(); ++it) {
        setPreference(category, it.key(), it.value());
    }
}

QJsonObject PreferencesHandler::exportToJson(const QString& category) const
{
    QMutexLocker locker(&d->mutex);
    
    QJsonObject jsonObj;
    
    if (category.isEmpty()) {
        // Export all categories
        QStringList categoryList = categories();
        for (const QString& cat : categoryList) {
            QVariantMap prefs = categoryPreferences(cat);
            QJsonObject catObj;
            for (auto it = prefs.begin(); it != prefs.end(); ++it) {
                catObj[it.key()] = QJsonValue::fromVariant(it.value());
            }
            jsonObj[cat] = catObj;
        }
    } else {
        // Export specific category
        QVariantMap prefs = categoryPreferences(category);
        for (auto it = prefs.begin(); it != prefs.end(); ++it) {
            jsonObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
    }
    
    return jsonObj;
}

bool PreferencesHandler::importFromJson(const QJsonObject& json, const QString& category)
{
    QMutexLocker locker(&d->mutex);
    
    try {
        if (category.isEmpty()) {
            // Import all categories
            for (auto it = json.begin(); it != json.end(); ++it) {
                QString catName = it.key();
                QJsonObject catObj = it.value().toObject();
                
                for (auto catIt = catObj.begin(); catIt != catObj.end(); ++catIt) {
                    setPreference(catName, catIt.key(), catIt.value().toVariant());
                }
            }
        } else {
            // Import to specific category
            for (auto it = json.begin(); it != json.end(); ++it) {
                setPreference(category, it.key(), it.value().toVariant());
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "PreferencesHandler: Import failed:" << e.what();
        return false;
    }
}

bool PreferencesHandler::createProfile(const QString& profileName)
{
    QMutexLocker locker(&d->mutex);
    
    if (profileName.isEmpty()) {
        return false;
    }
    
    QString profilePath = getProfilePath(profileName);
    QDir().mkpath(QFileInfo(profilePath).absolutePath());
    
    // Create empty profile file
    QFile file(profilePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject emptyProfile;
        emptyProfile["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        emptyProfile["version"] = "1.0";
        
        QJsonDocument doc(emptyProfile);
        file.write(doc.toJson());
        return true;
    }
    
    return false;
}

bool PreferencesHandler::switchToProfile(const QString& profileName)
{
    QMutexLocker locker(&d->mutex);
    
    if (profileName == d->currentProfileName) {
        return true;
    }
    
    // Save current profile
    saveProfile(d->currentProfileName);
    
    QString oldProfile = d->currentProfileName;
    d->currentProfileName = profileName;
    
    // Load new profile
    loadProfile(profileName);
    
    emit profileChanged(oldProfile, profileName);
    return true;
}

bool PreferencesHandler::deleteProfile(const QString& profileName)
{
    QMutexLocker locker(&d->mutex);
    
    if (profileName == d->currentProfileName || profileName == "default") {
        return false; // Cannot delete current or default profile
    }
    
    QString profilePath = getProfilePath(profileName);
    return QFile::remove(profilePath);
}

QStringList PreferencesHandler::availableProfiles() const
{
    QMutexLocker locker(&d->mutex);
    
    QString profilesDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/profiles";
    QDir dir(profilesDir);
    
    QStringList profiles;
    QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
    
    for (const QString& file : files) {
        QString profileName = QFileInfo(file).baseName();
        profiles.append(profileName);
    }
    
    return profiles;
}

QString PreferencesHandler::currentProfile() const
{
    QMutexLocker locker(&d->mutex);
    return d->currentProfileName;
}

bool PreferencesHandler::sync()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return false;
    }
    
    // Save current profile
    saveProfile(d->currentProfileName);
    
    // Sync settings manager
    bool success = d->settingsManager->sync();
    
    emit syncCompleted(success);
    return success;
}

// Extended functionality implementation

void PreferencesHandler::setSettingsManager(SettingsManager* manager)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->settingsManager) {
        disconnect(d->settingsManager, nullptr, this, nullptr);
    }
    
    d->settingsManager = manager;
    
    if (d->settingsManager) {
        connect(d->settingsManager, &SettingsManager::valueChanged,
                this, &PreferencesHandler::onSettingsChanged);
    }
}

SettingsManager* PreferencesHandler::settingsManager() const
{
    QMutexLocker locker(&d->mutex);
    return d->settingsManager;
}

void PreferencesHandler::setStorageFormat(StorageFormat format)
{
    QMutexLocker locker(&d->mutex);
    d->storageFormat = format;
}

PreferencesHandler::StorageFormat PreferencesHandler::storageFormat() const
{
    QMutexLocker locker(&d->mutex);
    return d->storageFormat;
}

void PreferencesHandler::setConflictResolution(ConflictResolution strategy)
{
    QMutexLocker locker(&d->mutex);
    d->conflictResolution = strategy;
}

PreferencesHandler::ConflictResolution PreferencesHandler::conflictResolution() const
{
    QMutexLocker locker(&d->mutex);
    return d->conflictResolution;
}

void PreferencesHandler::setPreferencePriority(const QString& category, const QString& key, PreferencePriority priority)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return;
    }
    
    QString priorityKey = getMetaKey(category, key, "priority");
    d->settingsManager->setValue(priorityKey, priorityToString(priority));
}

IPreferencesHandler::PreferencePriority PreferencesHandler::preferencePriority(const QString& category, const QString& key) const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->settingsManager) {
        return NormalPriority;
    }
    
    QString priorityKey = getMetaKey(category, key, "priority");
    QString priorityStr = d->settingsManager->value(priorityKey, "normal").toString();
    
    return stringToPriority(priorityStr);
}

// Additional profile management methods would be implemented here...
// For brevity, I'll implement the key private methods

// Private methods

QString PreferencesHandler::categoryToString(PreferenceCategory category) const
{
    switch (category) {
        case AudioPreferences: return "audio";
        case VideoPreferences: return "video";
        case UIPreferences: return "ui";
        case NetworkPreferences: return "network";
        case SecurityPreferences: return "security";
        case PerformancePreferences: return "performance";
        case CustomPreferences: return "custom";
        default: return "custom";
    }
}

IPreferencesHandler::PreferenceCategory PreferencesHandler::stringToCategory(const QString& category) const
{
    if (category == "audio") return AudioPreferences;
    if (category == "video") return VideoPreferences;
    if (category == "ui") return UIPreferences;
    if (category == "network") return NetworkPreferences;
    if (category == "security") return SecurityPreferences;
    if (category == "performance") return PerformancePreferences;
    return CustomPreferences;
}

QString PreferencesHandler::priorityToString(PreferencePriority priority) const
{
    switch (priority) {
        case LowPriority: return "low";
        case NormalPriority: return "normal";
        case HighPriority: return "high";
        case CriticalPriority: return "critical";
        default: return "normal";
    }
}

IPreferencesHandler::PreferencePriority PreferencesHandler::stringToPriority(const QString& priority) const
{
    if (priority == "low") return LowPriority;
    if (priority == "normal") return NormalPriority;
    if (priority == "high") return HighPriority;
    if (priority == "critical") return CriticalPriority;
    return NormalPriority;
}

QString PreferencesHandler::statusToString(PreferenceStatus status) const
{
    switch (status) {
        case Default: return "default";
        case Modified: return "modified";
        case Synced: return "synced";
        case Conflict: return "conflict";
        default: return "default";
    }
}

IPreferencesHandler::PreferenceStatus PreferencesHandler::stringToStatus(const QString& status) const
{
    if (status == "default") return Default;
    if (status == "modified") return Modified;
    if (status == "synced") return Synced;
    if (status == "conflict") return Conflict;
    return Default;
}

QString PreferencesHandler::getPreferenceKey(const QString& category, const QString& key) const
{
    return QString("preferences/%1/%2").arg(category, key);
}

QString PreferencesHandler::getMetaKey(const QString& category, const QString& key, const QString& metaType) const
{
    return QString("preferences_meta/%1/%2/%3").arg(category, key, metaType);
}

QString PreferencesHandler::getProfilePath(const QString& profileName) const
{
    QString profilesDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/profiles";
    return QString("%1/%2.json").arg(profilesDir, profileName);
}

QString PreferencesHandler::getBackupPath(const QString& backupName) const
{
    QString backupsDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/backups";
    return QString("%1/%2.json").arg(backupsDir, backupName);
}

void PreferencesHandler::loadProfile(const QString& profileName)
{
    QString profilePath = getProfilePath(profileName);
    QFile file(profilePath);
    
    if (!file.exists()) {
        // Create default profile
        createProfile(profileName);
        return;
    }
    
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject profileObj = doc.object();
        
        // Load preferences from profile
        for (auto it = profileObj.begin(); it != profileObj.end(); ++it) {
            if (it.key() != "created" && it.key() != "version") {
                QJsonObject categoryObj = it.value().toObject();
                for (auto catIt = categoryObj.begin(); catIt != categoryObj.end(); ++catIt) {
                    setPreference(it.key(), catIt.key(), catIt.value().toVariant());
                }
            }
        }
    }
}

void PreferencesHandler::saveProfile(const QString& profileName)
{
    QString profilePath = getProfilePath(profileName);
    QDir().mkpath(QFileInfo(profilePath).absolutePath());
    
    QFile file(profilePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject profileObj = exportToJson();
        profileObj["saved"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        profileObj["version"] = "1.0";
        
        QJsonDocument doc(profileObj);
        file.write(doc.toJson());
    }
}

void PreferencesHandler::createDefaultCategories()
{
    // Create default preferences for each category
    d->categoryDefaults["audio"] = QVariantMap{
        {"volume", 0.8},
        {"muted", false},
        {"quality", "high"}
    };
    
    d->categoryDefaults["video"] = QVariantMap{
        {"resolution", "1920x1080"},
        {"framerate", 30},
        {"quality", "high"}
    };
    
    d->categoryDefaults["ui"] = QVariantMap{
        {"theme", "default"},
        {"language", "en"},
        {"animations", true}
    };
    
    d->categoryDefaults["network"] = QVariantMap{
        {"timeout", 30},
        {"retries", 3},
        {"bandwidth", "auto"}
    };
    
    d->categoryDefaults["security"] = QVariantMap{
        {"encryption", true},
        {"authentication", "required"}
    };
    
    d->categoryDefaults["performance"] = QVariantMap{
        {"optimization", "balanced"},
        {"caching", true}
    };
}

void PreferencesHandler::migrateOldPreferences()
{
    // Implementation would migrate old preference formats
}

bool PreferencesHandler::resolveConflict(const QString& category, const QString& key, 
                                       const QVariant& localValue, const QVariant& remoteValue)
{
    Q_UNUSED(category)
    Q_UNUSED(key)
    Q_UNUSED(localValue)
    Q_UNUSED(remoteValue)
    
    // Implementation would resolve conflicts based on strategy
    return true;
}

// Slots

void PreferencesHandler::refresh()
{
    QMutexLocker locker(&d->mutex);
    d->categoryCache.clear();
    loadProfile(d->currentProfileName);
}

void PreferencesHandler::cleanup()
{
    // Implementation would clean up old data
}

void PreferencesHandler::compact()
{
    // Implementation would compact storage
}

void PreferencesHandler::onAutoBackupTimer()
{
    if (d->autoBackupEnabled) {
        QString backupName = QString("auto_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        createBackup(backupName);
    }
}

void PreferencesHandler::onSettingsChanged(const QString& key, const QVariant& value)
{
    // Handle settings changes
    if (key.startsWith("preferences/")) {
        QStringList parts = key.split("/");
        if (parts.size() >= 3) {
            QString category = parts[1];
            QString prefKey = parts.mid(2).join("/");
            emit preferenceChanged(category, prefKey, value);
        }
    }
}

// Stub implementations for remaining methods
bool PreferencesHandler::copyProfile(const QString& sourceProfile, const QString& targetProfile)
{
    Q_UNUSED(sourceProfile)
    Q_UNUSED(targetProfile)
    return false;
}

bool PreferencesHandler::renameProfile(const QString& oldName, const QString& newName)
{
    Q_UNUSED(oldName)
    Q_UNUSED(newName)
    return false;
}

bool PreferencesHandler::exportProfile(const QString& profileName, const QString& filePath) const
{
    Q_UNUSED(profileName)
    Q_UNUSED(filePath)
    return false;
}

bool PreferencesHandler::importProfile(const QString& filePath, const QString& profileName)
{
    Q_UNUSED(filePath)
    Q_UNUSED(profileName)
    return false;
}

QVariantMap PreferencesHandler::profileInfo(const QString& profileName) const
{
    Q_UNUSED(profileName)
    return QVariantMap();
}

void PreferencesHandler::setDefaultProfile(const QString& profileName)
{
    QMutexLocker locker(&d->mutex);
    d->defaultProfileName = profileName;
}

QString PreferencesHandler::defaultProfile() const
{
    QMutexLocker locker(&d->mutex);
    return d->defaultProfileName;
}

void PreferencesHandler::setAutoBackup(bool enabled, int interval)
{
    QMutexLocker locker(&d->mutex);
    d->autoBackupEnabled = enabled;
    d->autoBackupInterval = interval;
    
    if (enabled) {
        d->autoBackupTimer->start(interval * 60 * 1000); // Convert to milliseconds
    } else {
        d->autoBackupTimer->stop();
    }
}

bool PreferencesHandler::isAutoBackupEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->autoBackupEnabled;
}

bool PreferencesHandler::createBackup(const QString& backupName)
{
    Q_UNUSED(backupName)
    return false;
}

bool PreferencesHandler::restoreBackup(const QString& backupName)
{
    Q_UNUSED(backupName)
    return false;
}

QStringList PreferencesHandler::availableBackups() const
{
    return QStringList();
}

bool PreferencesHandler::deleteBackup(const QString& backupName)
{
    Q_UNUSED(backupName)
    return false;
}