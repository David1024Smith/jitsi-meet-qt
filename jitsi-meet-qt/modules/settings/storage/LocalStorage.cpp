#include "LocalStorage.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDataStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QSaveFile>
#include <QDebug>

class LocalStorage::Private
{
public:
    QString filePath;
    StorageFormat format;
    StorageStatus status;
    BackupStrategy backupStrategy;
    int maxBackups;
    bool autoBackupEnabled;
    bool fileWatchingEnabled;
    
    QVariantMap data;
    QFileSystemWatcher* fileWatcher;
    QMutex dataMutex;
    
    // Statistics
    QVariantMap statistics;
    QDateTime lastAccess;
    QDateTime lastModification;
    
    Private() 
        : format(JsonFormat)
        , status(NotInitialized)
        , backupStrategy(NoBackup)
        , maxBackups(5)
        , autoBackupEnabled(false)
        , fileWatchingEnabled(false)
        , fileWatcher(nullptr)
    {
        statistics["reads"] = 0;
        statistics["writes"] = 0;
        statistics["errors"] = 0;
        statistics["backups"] = 0;
    }
};

LocalStorage::LocalStorage(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
}

LocalStorage::LocalStorage(const QString& filePath, StorageFormat format, QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->filePath = filePath;
    d->format = format;
}

LocalStorage::~LocalStorage()
{
    if (d->fileWatcher) {
        d->fileWatcher->deleteLater();
    }
}

bool LocalStorage::initialize()
{
    if (d->filePath.isEmpty()) {
        setStatus(Error);
        emit errorOccurred("File path is empty");
        return false;
    }
    
    // Create directory if it doesn't exist
    QFileInfo fileInfo(d->filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            setStatus(Error);
            emit errorOccurred("Failed to create directory: " + dir.absolutePath());
            return false;
        }
    }
    
    // Setup file watcher if enabled
    if (d->fileWatchingEnabled) {
        setupFileWatcher();
    }
    
    // Load existing data if file exists
    if (fileExists()) {
        if (!load()) {
            setStatus(Error);
            return false;
        }
    }
    
    setStatus(Ready);
    return true;
}

LocalStorage::StorageStatus LocalStorage::status() const
{
    return d->status;
}

QString LocalStorage::filePath() const
{
    return d->filePath;
}

void LocalStorage::setFilePath(const QString& path)
{
    if (d->filePath != path) {
        d->filePath = path;
        emit filePathChanged(path);
        
        // Reinitialize if status was ready
        if (d->status == Ready) {
            initialize();
        }
    }
}

LocalStorage::StorageFormat LocalStorage::format() const
{
    return d->format;
}

void LocalStorage::setFormat(StorageFormat format)
{
    if (d->format != format) {
        d->format = format;
        emit formatChanged(format);
    }
}

bool LocalStorage::isAutoBackupEnabled() const
{
    return d->autoBackupEnabled;
}

void LocalStorage::setAutoBackupEnabled(bool enabled)
{
    if (d->autoBackupEnabled != enabled) {
        d->autoBackupEnabled = enabled;
        emit autoBackupChanged(enabled);
    }
}

bool LocalStorage::isFileWatchingEnabled() const
{
    return d->fileWatchingEnabled;
}

void LocalStorage::setFileWatchingEnabled(bool enabled)
{
    if (d->fileWatchingEnabled != enabled) {
        d->fileWatchingEnabled = enabled;
        emit fileWatchingChanged(enabled);
        
        if (enabled && d->status == Ready) {
            setupFileWatcher();
        } else if (!enabled && d->fileWatcher) {
            d->fileWatcher->deleteLater();
            d->fileWatcher = nullptr;
        }
    }
}

void LocalStorage::setValue(const QString& key, const QVariant& value)
{
    QMutexLocker locker(&d->dataMutex);
    
    QVariant oldValue = d->data.value(key);
    if (oldValue != value) {
        d->data[key] = value;
        d->lastModification = QDateTime::currentDateTime();
        
        emit dataChanged(key, value);
        
        // Auto-save if enabled
        if (d->status == Ready) {
            save();
        }
    }
}

QVariant LocalStorage::value(const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&d->dataMutex);
    d->lastAccess = QDateTime::currentDateTime();
    updateStatistics("read");
    
    return d->data.value(key, defaultValue);
}

bool LocalStorage::contains(const QString& key) const
{
    QMutexLocker locker(&d->dataMutex);
    return d->data.contains(key);
}

void LocalStorage::remove(const QString& key)
{
    QMutexLocker locker(&d->dataMutex);
    
    if (d->data.contains(key)) {
        d->data.remove(key);
        d->lastModification = QDateTime::currentDateTime();
        
        emit dataChanged(key, QVariant());
        
        // Auto-save if enabled
        if (d->status == Ready) {
            save();
        }
    }
}

QStringList LocalStorage::allKeys() const
{
    QMutexLocker locker(&d->dataMutex);
    return d->data.keys();
}

QStringList LocalStorage::childKeys(const QString& group) const
{
    QMutexLocker locker(&d->dataMutex);
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

QStringList LocalStorage::childGroups(const QString& group) const
{
    QMutexLocker locker(&d->dataMutex);
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

void LocalStorage::clear()
{
    QMutexLocker locker(&d->dataMutex);
    
    if (!d->data.isEmpty()) {
        d->data.clear();
        d->lastModification = QDateTime::currentDateTime();
        
        emit dataChanged(QString(), QVariant());
        
        // Auto-save if enabled
        if (d->status == Ready) {
            save();
        }
    }
}

bool LocalStorage::load()
{
    if (d->filePath.isEmpty()) {
        emit errorOccurred("File path is empty");
        return false;
    }
    
    setStatus(Loading);
    
    QVariant loadedData = readFromFile(d->filePath);
    if (loadedData.isNull()) {
        setStatus(Error);
        emit dataLoaded(false);
        return false;
    }
    
    QMutexLocker locker(&d->dataMutex);
    d->data = loadedData.toMap();
    d->lastAccess = QDateTime::currentDateTime();
    
    setStatus(Ready);
    updateStatistics("load");
    emit dataLoaded(true);
    
    return true;
}

bool LocalStorage::save()
{
    if (d->filePath.isEmpty()) {
        emit errorOccurred("File path is empty");
        return false;
    }
    
    setStatus(Saving);
    
    // Create backup if enabled
    if (d->autoBackupEnabled && fileExists()) {
        createBackup();
    }
    
    QMutexLocker locker(&d->dataMutex);
    bool success = writeToFile(d->data, d->filePath);
    
    if (success) {
        setStatus(Ready);
        updateStatistics("save");
        emit dataSaved(true);
    } else {
        setStatus(Error);
        emit dataSaved(false);
    }
    
    return success;
}

bool LocalStorage::sync()
{
    return save();
}

bool LocalStorage::reload()
{
    return load();
}

void LocalStorage::setBackupStrategy(BackupStrategy strategy, int maxBackups)
{
    d->backupStrategy = strategy;
    d->maxBackups = maxBackups;
}

LocalStorage::BackupStrategy LocalStorage::backupStrategy() const
{
    return d->backupStrategy;
}

bool LocalStorage::createBackup(const QString& backupName)
{
    if (!fileExists()) {
        return false;
    }
    
    QString actualBackupName = backupName.isEmpty() ? generateBackupName() : backupName;
    QString backupPath = getBackupPath(actualBackupName);
    
    // Create backup directory
    QFileInfo backupInfo(backupPath);
    QDir backupDir = backupInfo.absoluteDir();
    if (!backupDir.exists()) {
        backupDir.mkpath(".");
    }
    
    // Copy file
    bool success = QFile::copy(d->filePath, backupPath);
    
    if (success) {
        updateStatistics("backup");
        emit backupCreated(actualBackupName, true);
        
        // Clean up old backups if needed
        if (d->backupStrategy == MultipleBackup || d->backupStrategy == TimestampBackup) {
            cleanupOldBackups();
        }
    } else {
        emit backupCreated(actualBackupName, false);
    }
    
    return success;
}

bool LocalStorage::restoreBackup(const QString& backupName)
{
    QString backupPath = getBackupPath(backupName);
    
    if (!QFile::exists(backupPath)) {
        emit errorOccurred("Backup file not found: " + backupName);
        return false;
    }
    
    // Create backup of current file before restore
    if (fileExists()) {
        createBackup("pre_restore_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    }
    
    bool success = QFile::copy(backupPath, d->filePath);
    
    if (success) {
        // Reload data
        load();
        emit backupRestored(backupName, true);
    } else {
        emit backupRestored(backupName, false);
    }
    
    return success;
}

QStringList LocalStorage::availableBackups() const
{
    QFileInfo fileInfo(d->filePath);
    QString backupDir = fileInfo.absolutePath() + "/.backups";
    
    QDir dir(backupDir);
    if (!dir.exists()) {
        return QStringList();
    }
    
    QStringList filters;
    filters << "*" + formatToExtension(d->format);
    
    QStringList backupFiles = dir.entryList(filters, QDir::Files, QDir::Time | QDir::Reversed);
    
    // Remove extension from names
    QStringList backupNames;
    for (const QString& file : backupFiles) {
        QString name = file;
        name.remove(formatToExtension(d->format));
        backupNames.append(name);
    }
    
    return backupNames;
}

bool LocalStorage::deleteBackup(const QString& backupName)
{
    QString backupPath = getBackupPath(backupName);
    return QFile::remove(backupPath);
}

void LocalStorage::cleanupOldBackups()
{
    QStringList backups = availableBackups();
    
    while (backups.size() > d->maxBackups) {
        QString oldestBackup = backups.takeLast();
        deleteBackup(oldestBackup);
    }
}

bool LocalStorage::exportToFile(const QString& exportPath, StorageFormat exportFormat) const
{
    QMutexLocker locker(&d->dataMutex);
    return writeToFile(d->data, exportPath, exportFormat);
}

bool LocalStorage::importFromFile(const QString& importPath, bool merge)
{
    QVariant importedData = readFromFile(importPath);
    if (importedData.isNull()) {
        return false;
    }
    
    QMutexLocker locker(&d->dataMutex);
    
    if (merge) {
        QVariantMap importedMap = importedData.toMap();
        for (auto it = importedMap.constBegin(); it != importedMap.constEnd(); ++it) {
            d->data[it.key()] = it.value();
        }
    } else {
        d->data = importedData.toMap();
    }
    
    d->lastModification = QDateTime::currentDateTime();
    
    // Save changes
    if (d->status == Ready) {
        save();
    }
    
    return true;
}

QJsonObject LocalStorage::exportToJson() const
{
    QMutexLocker locker(&d->dataMutex);
    return QJsonObject::fromVariantMap(d->data);
}

bool LocalStorage::importFromJson(const QJsonObject& json, bool merge)
{
    QMutexLocker locker(&d->dataMutex);
    
    QVariantMap importedData = json.toVariantMap();
    
    if (merge) {
        for (auto it = importedData.constBegin(); it != importedData.constEnd(); ++it) {
            d->data[it.key()] = it.value();
        }
    } else {
        d->data = importedData;
    }
    
    d->lastModification = QDateTime::currentDateTime();
    
    // Save changes
    if (d->status == Ready) {
        save();
    }
    
    return true;
}

bool LocalStorage::fileExists() const
{
    return QFile::exists(d->filePath);
}

qint64 LocalStorage::fileSize() const
{
    QFileInfo info(d->filePath);
    return info.size();
}

QDateTime LocalStorage::lastModified() const
{
    QFileInfo info(d->filePath);
    return info.lastModified();
}

bool LocalStorage::isReadable() const
{
    QFileInfo info(d->filePath);
    return info.isReadable();
}

bool LocalStorage::isWritable() const
{
    QFileInfo info(d->filePath);
    return info.isWritable();
}

QVariantMap LocalStorage::statistics() const
{
    QVariantMap stats = d->statistics;
    stats["fileSize"] = fileSize();
    stats["lastAccess"] = d->lastAccess;
    stats["lastModification"] = d->lastModification;
    stats["keyCount"] = d->data.size();
    return stats;
}

bool LocalStorage::validateIntegrity() const
{
    if (!fileExists()) {
        return false;
    }
    
    // Try to read and parse the file
    QVariant data = readFromFile(d->filePath);
    return !data.isNull();
}

bool LocalStorage::repairCorruption()
{
    // Try to restore from the most recent backup
    QStringList backups = availableBackups();
    if (backups.isEmpty()) {
        return false;
    }
    
    return const_cast<LocalStorage*>(this)->restoreBackup(backups.first());
}

void LocalStorage::forceSync()
{
    sync();
}

void LocalStorage::refresh()
{
    reload();
}

void LocalStorage::compact()
{
    // For file-based storage, compacting means rewriting the file
    save();
}

void LocalStorage::onFileChanged(const QString& path)
{
    if (path == d->filePath) {
        emit fileChanged(path);
        
        // Optionally reload data
        if (d->status == Ready) {
            reload();
        }
    }
}

void LocalStorage::setStatus(StorageStatus newStatus)
{
    if (d->status != newStatus) {
        d->status = newStatus;
    }
}

QString LocalStorage::getBackupPath(const QString& backupName) const
{
    QFileInfo fileInfo(d->filePath);
    QString backupDir = fileInfo.absolutePath() + "/.backups";
    return backupDir + "/" + backupName + formatToExtension(d->format);
}

QString LocalStorage::generateBackupName() const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    
    switch (d->backupStrategy) {
    case SingleBackup:
        return "backup";
    case MultipleBackup:
        return QString("backup_%1").arg(timestamp);
    case TimestampBackup:
        return timestamp;
    default:
        return "backup";
    }
}

bool LocalStorage::writeToFile(const QVariant& data, const QString& filePath) const
{
    return writeToFile(data, filePath, d->format);
}

bool LocalStorage::writeToFile(const QVariant& data, const QString& filePath, StorageFormat format) const
{
    QByteArray formattedData = formatData(data, format);
    if (formattedData.isEmpty()) {
        return false;
    }
    
    return atomicWrite(formattedData, filePath);
}

QVariant LocalStorage::readFromFile(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit const_cast<LocalStorage*>(this)->errorOccurred("Failed to open file for reading: " + filePath);
        return QVariant();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (data.isEmpty()) {
        return QVariantMap(); // Return empty map for empty files
    }
    
    // Determine format from file extension if not specified
    StorageFormat fileFormat = d->format;
    if (filePath != d->filePath) {
        QFileInfo info(filePath);
        fileFormat = extensionToFormat(info.suffix());
    }
    
    return parseData(data, fileFormat);
}

bool LocalStorage::atomicWrite(const QByteArray& data, const QString& filePath) const
{
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit const_cast<LocalStorage*>(this)->errorOccurred("Failed to open file for writing: " + filePath);
        return false;
    }
    
    qint64 written = file.write(data);
    if (written != data.size()) {
        emit const_cast<LocalStorage*>(this)->errorOccurred("Failed to write all data to file: " + filePath);
        return false;
    }
    
    if (!file.commit()) {
        emit const_cast<LocalStorage*>(this)->errorOccurred("Failed to commit file: " + filePath);
        return false;
    }
    
    return true;
}

QByteArray LocalStorage::formatData(const QVariant& data, StorageFormat format) const
{
    switch (format) {
    case JsonFormat: {
        QJsonDocument doc = QJsonDocument::fromVariant(data);
        return doc.toJson();
    }
    case IniFormat: {
        // Convert to QSettings format
        QByteArray result;
        QDataStream stream(&result, QIODevice::WriteOnly);
        
        QVariantMap map = data.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            QString line = QString("%1=%2\n").arg(it.key(), it.value().toString());
            result.append(line.toUtf8());
        }
        return result;
    }
    case XmlFormat: {
        QByteArray result;
        QXmlStreamWriter writer(&result);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("settings");
        
        QVariantMap map = data.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            writer.writeStartElement("setting");
            writer.writeAttribute("key", it.key());
            writer.writeCharacters(it.value().toString());
            writer.writeEndElement();
        }
        
        writer.writeEndElement();
        writer.writeEndDocument();
        return result;
    }
    case BinaryFormat: {
        QByteArray result;
        QDataStream stream(&result, QIODevice::WriteOnly);
        stream << data;
        return result;
    }
    }
    
    return QByteArray();
}

QVariant LocalStorage::parseData(const QByteArray& data, StorageFormat format) const
{
    switch (format) {
    case JsonFormat: {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            emit const_cast<LocalStorage*>(this)->errorOccurred("JSON parse error: " + error.errorString());
            return QVariant();
        }
        return doc.toVariant();
    }
    case IniFormat: {
        // Simple INI parsing
        QVariantMap result;
        QStringList lines = QString::fromUtf8(data).split('\n', Qt::SkipEmptyParts);
        
        for (const QString& line : lines) {
            int equalIndex = line.indexOf('=');
            if (equalIndex > 0) {
                QString key = line.left(equalIndex).trimmed();
                QString value = line.mid(equalIndex + 1).trimmed();
                result[key] = value;
            }
        }
        return result;
    }
    case XmlFormat: {
        QVariantMap result;
        QXmlStreamReader reader(data);
        
        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.isStartElement() && reader.name() == "setting") {
                QString key = reader.attributes().value("key").toString();
                QString value = reader.readElementText();
                result[key] = value;
            }
        }
        
        if (reader.hasError()) {
            emit const_cast<LocalStorage*>(this)->errorOccurred("XML parse error: " + reader.errorString());
            return QVariant();
        }
        
        return result;
    }
    case BinaryFormat: {
        QDataStream stream(data);
        QVariant result;
        stream >> result;
        return result;
    }
    }
    
    return QVariant();
}

QString LocalStorage::formatToExtension(StorageFormat format) const
{
    switch (format) {
    case JsonFormat: return ".json";
    case IniFormat: return ".ini";
    case XmlFormat: return ".xml";
    case BinaryFormat: return ".dat";
    }
    return ".json";
}

LocalStorage::StorageFormat LocalStorage::extensionToFormat(const QString& extension) const
{
    QString ext = extension.toLower();
    if (ext == "json") return JsonFormat;
    if (ext == "ini") return IniFormat;
    if (ext == "xml") return XmlFormat;
    if (ext == "dat") return BinaryFormat;
    return JsonFormat;
}

void LocalStorage::setupFileWatcher()
{
    if (!d->fileWatcher) {
        d->fileWatcher = new QFileSystemWatcher(this);
        connect(d->fileWatcher, &QFileSystemWatcher::fileChanged,
                this, &LocalStorage::onFileChanged);
    }
    
    if (!d->filePath.isEmpty() && fileExists()) {
        d->fileWatcher->addPath(d->filePath);
    }
}

void LocalStorage::updateStatistics(const QString& operation) const
{
    d->statistics[operation] = d->statistics[operation].toInt() + 1;
}