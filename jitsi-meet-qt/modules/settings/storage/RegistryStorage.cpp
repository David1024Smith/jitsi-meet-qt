#include "RegistryStorage.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

#ifdef Q_OS_WIN
#include <QWinEventNotifier>
#include <windows.h>
#endif

class RegistryStorage::Private
{
public:
    QString registryPath;
    RegistryScope scope;
    AccessRights accessRights;
    StorageStatus status;
    bool monitoringEnabled;
    
    QVariantMap data;
    QMutex dataMutex;
    
#ifdef Q_OS_WIN
    QSettings* settings;
    HKEY hKey;
    QWinEventNotifier* eventNotifier;
    HANDLE changeEvent;
#else
    // Alternative storage for non-Windows platforms
    QString alternativeStoragePath;
    QSettings* alternativeSettings;
#endif
    
    // Statistics
    QVariantMap statistics;
    QDateTime lastAccess;
    QDateTime lastModification;
    
    Private()
        : scope(CurrentUser)
        , accessRights(ReadWrite)
        , status(NotInitialized)
        , monitoringEnabled(false)
#ifdef Q_OS_WIN
        , settings(nullptr)
        , hKey(nullptr)
        , eventNotifier(nullptr)
        , changeEvent(nullptr)
#else
        , alternativeSettings(nullptr)
#endif
    {
        statistics["reads"] = 0;
        statistics["writes"] = 0;
        statistics["errors"] = 0;
        statistics["backups"] = 0;
        
#ifndef Q_OS_WIN
        // Setup alternative storage path for non-Windows platforms
        alternativeStoragePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/RegistryStorage";
        QDir().mkpath(alternativeStoragePath);
#endif
    }
    
    ~Private()
    {
#ifdef Q_OS_WIN
        if (settings) {
            delete settings;
        }
        if (eventNotifier) {
            delete eventNotifier;
        }
        if (changeEvent) {
            CloseHandle(changeEvent);
        }
        if (hKey) {
            RegCloseKey(hKey);
        }
#else
        if (alternativeSettings) {
            delete alternativeSettings;
        }
#endif
    }
};

RegistryStorage::RegistryStorage(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
}

RegistryStorage::RegistryStorage(const QString& registryPath, RegistryScope scope, QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->registryPath = registryPath;
    d->scope = scope;
}

RegistryStorage::~RegistryStorage()
{
#ifdef Q_OS_WIN
    if (isMonitoring()) {
        stopMonitoring();
    }
#endif
}

bool RegistryStorage::initialize()
{
    if (d->registryPath.isEmpty()) {
        setStatus(Error);
        emit errorOccurred("Registry path is empty");
        return false;
    }
    
#ifdef Q_OS_WIN
    // Initialize QSettings for Windows registry
    QSettings::Format format = QSettings::NativeFormat;
    QSettings::Scope settingsScope = (d->scope == CurrentUser) ? QSettings::UserScope : QSettings::SystemScope;
    
    d->settings = new QSettings(format, settingsScope, "JitsiMeet", d->registryPath, this);
    
    if (d->settings->status() != QSettings::NoError) {
        setStatus(Error);
        emit errorOccurred("Failed to initialize registry settings");
        return false;
    }
    
    // Test access rights
    if (!hasReadAccess()) {
        setStatus(AccessDenied);
        emit errorOccurred("No read access to registry path");
        return false;
    }
    
    if (d->accessRights != ReadOnly && !hasWriteAccess()) {
        setStatus(AccessDenied);
        emit errorOccurred("No write access to registry path");
        return false;
    }
    
    // Load existing data
    loadData();
    
    // Setup monitoring if enabled
    if (d->monitoringEnabled) {
        startMonitoring();
    }
    
#else
    // Initialize alternative storage for non-Windows platforms
    initializeAlternativeStorage();
#endif
    
    setStatus(Ready);
    return true;
}

RegistryStorage::StorageStatus RegistryStorage::status() const
{
    return d->status;
}

QString RegistryStorage::registryPath() const
{
    return d->registryPath;
}

void RegistryStorage::setRegistryPath(const QString& path)
{
    if (d->registryPath != path) {
        d->registryPath = path;
        emit registryPathChanged(path);
        
        // Reinitialize if status was ready
        if (d->status == Ready) {
            initialize();
        }
    }
}

RegistryStorage::RegistryScope RegistryStorage::scope() const
{
    return d->scope;
}

void RegistryStorage::setScope(RegistryScope scope)
{
    if (d->scope != scope) {
        d->scope = scope;
        emit scopeChanged(scope);
        
        // Reinitialize if status was ready
        if (d->status == Ready) {
            initialize();
        }
    }
}

bool RegistryStorage::isMonitoringEnabled() const
{
    return d->monitoringEnabled;
}

void RegistryStorage::setMonitoringEnabled(bool enabled)
{
    if (d->monitoringEnabled != enabled) {
        d->monitoringEnabled = enabled;
        emit monitoringEnabledChanged(enabled);
        
#ifdef Q_OS_WIN
        if (enabled && d->status == Ready) {
            startMonitoring();
        } else if (!enabled) {
            stopMonitoring();
        }
#endif
    }
}

void RegistryStorage::setAccessRights(AccessRights rights)
{
    if (d->accessRights != rights) {
        d->accessRights = rights;
        emit accessRightsChanged(rights);
    }
}

RegistryStorage::AccessRights RegistryStorage::accessRights() const
{
    return d->accessRights;
}

bool RegistryStorage::hasReadAccess() const
{
#ifdef Q_OS_WIN
    if (d->settings) {
        // Try to read a test value
        d->settings->value("__test__");
        return d->settings->status() == QSettings::NoError;
    }
    return false;
#else
    return d->alternativeSettings != nullptr;
#endif
}

bool RegistryStorage::hasWriteAccess() const
{
#ifdef Q_OS_WIN
    if (d->settings && d->accessRights != ReadOnly) {
        // Try to write a test value
        d->settings->setValue("__test__", "test");
        bool canWrite = (d->settings->status() == QSettings::NoError);
        d->settings->remove("__test__");
        return canWrite;
    }
    return false;
#else
    return d->alternativeSettings != nullptr;
#endif
}

bool RegistryStorage::requestElevatedAccess()
{
#ifdef Q_OS_WIN
    // This would typically require UAC elevation
    // For now, just return false as this requires special handling
    return false;
#else
    return true; // No elevation needed on non-Windows platforms
#endif
}

void RegistryStorage::setValue(const QString& key, const QVariant& value, DataType dataType)
{
    if (d->accessRights == ReadOnly) {
        emit errorOccurred("Cannot write in read-only mode");
        return;
    }
    
    QMutexLocker locker(&d->dataMutex);
    
    QVariant oldValue = d->data.value(key);
    if (oldValue != value) {
        d->data[key] = value;
        d->lastModification = QDateTime::currentDateTime();
        
#ifdef Q_OS_WIN
        if (d->settings) {
            d->settings->setValue(key, value);
            d->settings->sync();
        }
#else
        if (d->alternativeSettings) {
            d->alternativeSettings->setValue(key, value);
            d->alternativeSettings->sync();
        }
#endif
        
        updateStatistics("write");
        emit dataChanged(key, value);
    }
}

QVariant RegistryStorage::value(const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&d->dataMutex);
    d->lastAccess = QDateTime::currentDateTime();
    updateStatistics("read");
    
#ifdef Q_OS_WIN
    if (d->settings) {
        return d->settings->value(key, defaultValue);
    }
#else
    if (d->alternativeSettings) {
        return d->alternativeSettings->value(key, defaultValue);
    }
#endif
    
    return d->data.value(key, defaultValue);
}

bool RegistryStorage::contains(const QString& key) const
{
    QMutexLocker locker(&d->dataMutex);
    
#ifdef Q_OS_WIN
    if (d->settings) {
        return d->settings->contains(key);
    }
#else
    if (d->alternativeSettings) {
        return d->alternativeSettings->contains(key);
    }
#endif
    
    return d->data.contains(key);
}

void RegistryStorage::remove(const QString& key)
{
    if (d->accessRights == ReadOnly) {
        emit errorOccurred("Cannot remove in read-only mode");
        return;
    }
    
    QMutexLocker locker(&d->dataMutex);
    
    if (d->data.contains(key)) {
        d->data.remove(key);
        d->lastModification = QDateTime::currentDateTime();
        
#ifdef Q_OS_WIN
        if (d->settings) {
            d->settings->remove(key);
            d->settings->sync();
        }
#else
        if (d->alternativeSettings) {
            d->alternativeSettings->remove(key);
            d->alternativeSettings->sync();
        }
#endif
        
        emit dataChanged(key, QVariant());
    }
}

QStringList RegistryStorage::allKeys() const
{
    QMutexLocker locker(&d->dataMutex);
    
#ifdef Q_OS_WIN
    if (d->settings) {
        return d->settings->allKeys();
    }
#else
    if (d->alternativeSettings) {
        return d->alternativeSettings->allKeys();
    }
#endif
    
    return d->data.keys();
}

QStringList RegistryStorage::childKeys(const QString& group) const
{
    QMutexLocker locker(&d->dataMutex);
    
#ifdef Q_OS_WIN
    if (d->settings) {
        if (!group.isEmpty()) {
            d->settings->beginGroup(group);
        }
        QStringList keys = d->settings->childKeys();
        if (!group.isEmpty()) {
            d->settings->endGroup();
        }
        return keys;
    }
#else
    if (d->alternativeSettings) {
        if (!group.isEmpty()) {
            d->alternativeSettings->beginGroup(group);
        }
        QStringList keys = d->alternativeSettings->childKeys();
        if (!group.isEmpty()) {
            d->alternativeSettings->endGroup();
        }
        return keys;
    }
#endif
    
    // Fallback implementation
    QStringList keys;
    QString prefix = group.isEmpty() ? QString() : group + "/";
    
    for (auto it = d->data.constBegin(); it != d->data.constEnd(); ++it) {
        const QString& key = it.key();
        if (key.startsWith(prefix)) {
            QString childKey = key.mid(prefix.length());
            if (!childKey.contains('/')) {
                keys.append(childKey);
            }
        }
    }
    
    return keys;
}

QStringList RegistryStorage::childGroups(const QString& group) const
{
    QMutexLocker locker(&d->dataMutex);
    
#ifdef Q_OS_WIN
    if (d->settings) {
        if (!group.isEmpty()) {
            d->settings->beginGroup(group);
        }
        QStringList groups = d->settings->childGroups();
        if (!group.isEmpty()) {
            d->settings->endGroup();
        }
        return groups;
    }
#else
    if (d->alternativeSettings) {
        if (!group.isEmpty()) {
            d->alternativeSettings->beginGroup(group);
        }
        QStringList groups = d->alternativeSettings->childGroups();
        if (!group.isEmpty()) {
            d->alternativeSettings->endGroup();
        }
        return groups;
    }
#endif
    
    // Fallback implementation
    QStringList groups;
    QString prefix = group.isEmpty() ? QString() : group + "/";
    
    for (auto it = d->data.constBegin(); it != d->data.constEnd(); ++it) {
        const QString& key = it.key();
        if (key.startsWith(prefix)) {
            QString childKey = key.mid(prefix.length());
            int slashIndex = childKey.indexOf('/');
            if (slashIndex > 0) {
                QString groupName = childKey.left(slashIndex);
                if (!groups.contains(groupName)) {
                    groups.append(groupName);
                }
            }
        }
    }
    
    return groups;
}

void RegistryStorage::clear()
{
    if (d->accessRights == ReadOnly) {
        emit errorOccurred("Cannot clear in read-only mode");
        return;
    }
    
    QMutexLocker locker(&d->dataMutex);
    
    if (!d->data.isEmpty()) {
        d->data.clear();
        d->lastModification = QDateTime::currentDateTime();
        
#ifdef Q_OS_WIN
        if (d->settings) {
            d->settings->clear();
            d->settings->sync();
        }
#else
        if (d->alternativeSettings) {
            d->alternativeSettings->clear();
            d->alternativeSettings->sync();
        }
#endif
        
        emit dataChanged(QString(), QVariant());
    }
}// Regis
try-specific operations
bool RegistryStorage::createKey(const QString& keyPath)
{
#ifdef Q_OS_WIN
    if (d->accessRights == ReadOnly) {
        return false;
    }
    
    HKEY hKey;
    DWORD disposition;
    LONG result = RegCreateKeyEx(
        getScopeHandle(d->scope),
        reinterpret_cast<const wchar_t*>(keyPath.utf16()),
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        getAccessMask(d->accessRights),
        nullptr,
        &hKey,
        &disposition
    );
    
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        emit keyCreated(keyPath);
        return true;
    }
    
    return false;
#else
    // For non-Windows platforms, just create a group
    if (d->alternativeSettings) {
        d->alternativeSettings->beginGroup(keyPath);
        d->alternativeSettings->endGroup();
        emit keyCreated(keyPath);
        return true;
    }
    return false;
#endif
}

bool RegistryStorage::deleteKey(const QString& keyPath, bool recursive)
{
#ifdef Q_OS_WIN
    if (d->accessRights == ReadOnly) {
        return false;
    }
    
    LONG result;
    if (recursive) {
        result = RegDeleteTree(getScopeHandle(d->scope), reinterpret_cast<const wchar_t*>(keyPath.utf16()));
    } else {
        result = RegDeleteKey(getScopeHandle(d->scope), reinterpret_cast<const wchar_t*>(keyPath.utf16()));
    }
    
    if (result == ERROR_SUCCESS) {
        emit keyDeleted(keyPath);
        return true;
    }
    
    return false;
#else
    // For non-Windows platforms, remove the group
    if (d->alternativeSettings) {
        d->alternativeSettings->remove(keyPath);
        emit keyDeleted(keyPath);
        return true;
    }
    return false;
#endif
}

bool RegistryStorage::keyExists(const QString& keyPath) const
{
#ifdef Q_OS_WIN
    HKEY hKey;
    LONG result = RegOpenKeyEx(
        getScopeHandle(d->scope),
        reinterpret_cast<const wchar_t*>(keyPath.utf16()),
        0,
        KEY_READ,
        &hKey
    );
    
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    
    return false;
#else
    // For non-Windows platforms, check if group exists
    if (d->alternativeSettings) {
        d->alternativeSettings->beginGroup(keyPath);
        bool exists = !d->alternativeSettings->childKeys().isEmpty() || !d->alternativeSettings->childGroups().isEmpty();
        d->alternativeSettings->endGroup();
        return exists;
    }
    return false;
#endif
}

RegistryStorage::DataType RegistryStorage::getDataType(const QString& key) const
{
#ifdef Q_OS_WIN
    // This would require direct registry API calls to get the actual type
    // For simplicity, we'll return StringType as default
    Q_UNUSED(key)
    return StringType;
#else
    Q_UNUSED(key)
    return StringType;
#endif
}

void RegistryStorage::setDataType(const QString& key, DataType dataType)
{
    // Store the data type information separately
    Q_UNUSED(key)
    Q_UNUSED(dataType)
    // Implementation would depend on how we want to store type information
}

qint64 RegistryStorage::getDataSize(const QString& key) const
{
    QVariant val = value(key);
    if (val.isValid()) {
        return val.toString().size();
    }
    return 0;
}

// Backup and restore operations
bool RegistryStorage::exportToFile(const QString& filePath, const QString& format) const
{
    if (format == "json") {
        QJsonObject json = exportToJson();
        QJsonDocument doc(json);
        
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
            return true;
        }
    } else if (format == "reg") {
#ifdef Q_OS_WIN
        // Export to .reg file format
        QString regContent = QString("[%1\\%2]\n").arg(scopeToString(d->scope), d->registryPath);
        
        QStringList keys = allKeys();
        for (const QString& key : keys) {
            QVariant val = value(key);
            regContent += QString("\"%1\"=\"%2\"\n").arg(key, val.toString());
        }
        
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(regContent.toUtf8());
            file.close();
            return true;
        }
#endif
    }
    
    return false;
}

bool RegistryStorage::importFromFile(const QString& filePath, bool merge)
{
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();
    
    if (suffix == "json") {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();
            
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(data, &error);
            
            if (error.error == QJsonParseError::NoError) {
                return importFromJson(doc.object(), merge);
            }
        }
    } else if (suffix == "reg") {
        // Import from .reg file format
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            QString content = stream.readAll();
            file.close();
            
            // Simple .reg file parsing
            QStringList lines = content.split('\n', Qt::SkipEmptyParts);
            for (const QString& line : lines) {
                if (line.startsWith('"') && line.contains('=')) {
                    int equalIndex = line.indexOf('=');
                    QString key = line.left(equalIndex);
                    QString value = line.mid(equalIndex + 1);
                    
                    // Remove quotes
                    key = key.mid(1, key.length() - 2);
                    if (value.startsWith('"') && value.endsWith('"')) {
                        value = value.mid(1, value.length() - 2);
                    }
                    
                    setValue(key, value);
                }
            }
            return true;
        }
    }
    
    return false;
}

bool RegistryStorage::createBackup(const QString& backupName)
{
    QString backupPath = getBackupPath(backupName);
    return exportToFile(backupPath, "json");
}

bool RegistryStorage::restoreBackup(const QString& backupName)
{
    QString backupPath = getBackupPath(backupName);
    
    if (!QFile::exists(backupPath)) {
        emit errorOccurred("Backup file not found: " + backupName);
        return false;
    }
    
    bool success = importFromFile(backupPath, false);
    
    if (success) {
        emit backupRestored(backupName, true);
    } else {
        emit backupRestored(backupName, false);
    }
    
    return success;
}

QStringList RegistryStorage::availableBackups() const
{
    QString backupDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/RegistryBackups";
    
    QDir dir(backupDir);
    if (!dir.exists()) {
        return QStringList();
    }
    
    QStringList filters;
    filters << "*.json";
    
    QStringList backupFiles = dir.entryList(filters, QDir::Files, QDir::Time | QDir::Reversed);
    
    // Remove extension from names
    QStringList backupNames;
    for (const QString& file : backupFiles) {
        QString name = file;
        name.remove(".json");
        backupNames.append(name);
    }
    
    return backupNames;
}

bool RegistryStorage::deleteBackup(const QString& backupName)
{
    QString backupPath = getBackupPath(backupName);
    return QFile::remove(backupPath);
}

#ifdef Q_OS_WIN
bool RegistryStorage::startMonitoring(bool watchSubtree)
{
    if (d->eventNotifier || d->changeEvent) {
        return false; // Already monitoring
    }
    
    HKEY hKey;
    LONG result = RegOpenKeyEx(
        getScopeHandle(d->scope),
        reinterpret_cast<const wchar_t*>(d->registryPath.utf16()),
        0,
        KEY_NOTIFY,
        &hKey
    );
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    d->hKey = hKey;
    d->changeEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    
    if (!d->changeEvent) {
        RegCloseKey(d->hKey);
        d->hKey = nullptr;
        return false;
    }
    
    result = RegNotifyChangeKeyValue(
        d->hKey,
        watchSubtree,
        REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
        d->changeEvent,
        TRUE
    );
    
    if (result != ERROR_SUCCESS) {
        CloseHandle(d->changeEvent);
        RegCloseKey(d->hKey);
        d->changeEvent = nullptr;
        d->hKey = nullptr;
        return false;
    }
    
    d->eventNotifier = new QWinEventNotifier(d->changeEvent, this);
    connect(d->eventNotifier, &QWinEventNotifier::activated, this, &RegistryStorage::onRegistryChanged);
    
    return true;
}

void RegistryStorage::stopMonitoring()
{
    if (d->eventNotifier) {
        d->eventNotifier->setEnabled(false);
        delete d->eventNotifier;
        d->eventNotifier = nullptr;
    }
    
    if (d->changeEvent) {
        CloseHandle(d->changeEvent);
        d->changeEvent = nullptr;
    }
    
    if (d->hKey) {
        RegCloseKey(d->hKey);
        d->hKey = nullptr;
    }
}

bool RegistryStorage::isMonitoring() const
{
    return d->eventNotifier != nullptr;
}

void RegistryStorage::onRegistryChanged()
{
    // Reload data when registry changes
    loadData();
    
    // Reset the event for next notification
    if (d->changeEvent && d->hKey) {
        ResetEvent(d->changeEvent);
        RegNotifyChangeKeyValue(
            d->hKey,
            TRUE,
            REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
            d->changeEvent,
            TRUE
        );
    }
}
#endif

QString RegistryStorage::fullRegistryPath() const
{
    return scopeToString(d->scope) + "\\" + d->registryPath;
}

QVariantMap RegistryStorage::statistics() const
{
    QVariantMap stats = d->statistics;
    stats["lastAccess"] = d->lastAccess;
    stats["lastModification"] = d->lastModification;
    stats["keyCount"] = allKeys().size();
    return stats;
}

bool RegistryStorage::validateIntegrity() const
{
    // Basic integrity check - ensure we can read the registry path
    return keyExists(d->registryPath);
}

bool RegistryStorage::compactRegistry()
{
#ifdef Q_OS_WIN
    // Registry compaction is typically handled by the system
    // We can't directly compact it from application code
    return false;
#else
    // For alternative storage, we can rewrite the file
    if (d->alternativeSettings) {
        d->alternativeSettings->sync();
        return true;
    }
    return false;
#endif
}

bool RegistryStorage::isSupported()
{
#ifdef Q_OS_WIN
    return true;
#else
    return false; // Registry is Windows-specific
#endif
}

QString RegistryStorage::alternativeStoragePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/RegistryStorage";
}

QJsonObject RegistryStorage::exportToJson() const
{
    QMutexLocker locker(&d->dataMutex);
    
    QJsonObject json;
    QStringList keys = allKeys();
    
    for (const QString& key : keys) {
        QVariant val = value(key);
        json[key] = QJsonValue::fromVariant(val);
    }
    
    return json;
}

bool RegistryStorage::importFromJson(const QJsonObject& json, bool merge)
{
    if (!merge) {
        clear();
    }
    
    for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
        setValue(it.key(), it.value().toVariant());
    }
    
    return true;
}

void RegistryStorage::sync()
{
#ifdef Q_OS_WIN
    if (d->settings) {
        d->settings->sync();
    }
#else
    if (d->alternativeSettings) {
        d->alternativeSettings->sync();
    }
#endif
}

void RegistryStorage::refresh()
{
    loadData();
}

void RegistryStorage::cleanup()
{
    // Remove empty keys and invalid entries
    // This is a placeholder for cleanup logic
}

// Private helper methods
void RegistryStorage::setStatus(StorageStatus newStatus)
{
    if (d->status != newStatus) {
        d->status = newStatus;
    }
}

QString RegistryStorage::scopeToString(RegistryScope scope) const
{
    switch (scope) {
    case CurrentUser: return "HKEY_CURRENT_USER";
    case LocalMachine: return "HKEY_LOCAL_MACHINE";
    case ClassesRoot: return "HKEY_CLASSES_ROOT";
    case Users: return "HKEY_USERS";
    case CurrentConfig: return "HKEY_CURRENT_CONFIG";
    }
    return "HKEY_CURRENT_USER";
}

RegistryStorage::RegistryScope RegistryStorage::stringToScope(const QString& str) const
{
    if (str == "HKEY_LOCAL_MACHINE") return LocalMachine;
    if (str == "HKEY_CLASSES_ROOT") return ClassesRoot;
    if (str == "HKEY_USERS") return Users;
    if (str == "HKEY_CURRENT_CONFIG") return CurrentConfig;
    return CurrentUser;
}

QString RegistryStorage::dataTypeToString(DataType type) const
{
    switch (type) {
    case StringType: return "REG_SZ";
    case DWordType: return "REG_DWORD";
    case QWordType: return "REG_QWORD";
    case BinaryType: return "REG_BINARY";
    case MultiStringType: return "REG_MULTI_SZ";
    case ExpandStringType: return "REG_EXPAND_SZ";
    }
    return "REG_SZ";
}

RegistryStorage::DataType RegistryStorage::stringToDataType(const QString& str) const
{
    if (str == "REG_DWORD") return DWordType;
    if (str == "REG_QWORD") return QWordType;
    if (str == "REG_BINARY") return BinaryType;
    if (str == "REG_MULTI_SZ") return MultiStringType;
    if (str == "REG_EXPAND_SZ") return ExpandStringType;
    return StringType;
}

QString RegistryStorage::getBackupPath(const QString& backupName) const
{
    QString backupDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/RegistryBackups";
    QDir().mkpath(backupDir);
    return backupDir + "/" + backupName + ".json";
}

#ifdef Q_OS_WIN
HKEY RegistryStorage::getScopeHandle(RegistryScope scope) const
{
    switch (scope) {
    case CurrentUser: return HKEY_CURRENT_USER;
    case LocalMachine: return HKEY_LOCAL_MACHINE;
    case ClassesRoot: return HKEY_CLASSES_ROOT;
    case Users: return HKEY_USERS;
    case CurrentConfig: return HKEY_CURRENT_CONFIG;
    }
    return HKEY_CURRENT_USER;
}

REGSAM RegistryStorage::getAccessMask(AccessRights rights) const
{
    switch (rights) {
    case ReadOnly: return KEY_READ;
    case ReadWrite: return KEY_READ | KEY_WRITE;
    case FullControl: return KEY_ALL_ACCESS;
    }
    return KEY_READ;
}
#endif

void RegistryStorage::initializeAlternativeStorage()
{
#ifndef Q_OS_WIN
    QString configFile = d->alternativeStoragePath + "/" + d->registryPath + ".ini";
    d->alternativeSettings = new QSettings(configFile, QSettings::IniFormat, this);
    
    if (d->alternativeSettings->status() != QSettings::NoError) {
        setStatus(Error);
        emit errorOccurred("Failed to initialize alternative storage");
        return;
    }
    
    loadData();
#endif
}

void RegistryStorage::loadData()
{
    QMutexLocker locker(&d->dataMutex);
    
#ifdef Q_OS_WIN
    if (d->settings) {
        QStringList keys = d->settings->allKeys();
        d->data.clear();
        
        for (const QString& key : keys) {
            d->data[key] = d->settings->value(key);
        }
    }
#else
    if (d->alternativeSettings) {
        QStringList keys = d->alternativeSettings->allKeys();
        d->data.clear();
        
        for (const QString& key : keys) {
            d->data[key] = d->alternativeSettings->value(key);
        }
    }
#endif
    
    d->lastAccess = QDateTime::currentDateTime();
}

void RegistryStorage::updateStatistics(const QString& operation) const
{
    d->statistics[operation] = d->statistics[operation].toInt() + 1;
}