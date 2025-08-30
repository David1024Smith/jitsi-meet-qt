#include "ConfigFile.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDomDocument>
#include <QDir>
#include <QTimer>
#include <QDebug>

ConfigFile::ConfigFile(const QString& filePath, Format format, QObject* parent)
    : IFileHandler(parent)
    , m_filePath(filePath)
    , m_format(format)
    , m_accessMode(ReadWrite)
    , m_autoSave(false)
    , m_autoSaveInterval(5000) // 5秒
    , m_autoSaveTimer(new QTimer(this))
    , m_fileWatchEnabled(false)
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_modified(false)
{
    // 自动检测格式
    if (m_format == AutoDetect) {
        m_format = detectFormat(m_filePath);
    }
    
    // 设置自动保存定时器
    m_autoSaveTimer->setSingleShot(true);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &ConfigFile::onAutoSaveTimer);
    
    // 连接文件监控器
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &ConfigFile::onFileChanged);
}

ConfigFile::~ConfigFile()
{
    cleanup();
}

bool ConfigFile::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    // 确保目录存在
    if (!ensureDirectoryExists(m_filePath)) {
        return false;
    }
    
    // 加载配置文件
    if (QFile::exists(m_filePath)) {
        if (!load()) {
            return false;
        }
    }
    
    // 启用文件监控
    if (m_fileWatchEnabled && QFile::exists(m_filePath)) {
        m_fileWatcher->addPath(m_filePath);
    }
    
    return true;
}

void ConfigFile::cleanup()
{
    QMutexLocker locker(&m_mutex);
    
    // 保存未保存的更改
    if (m_modified && m_autoSave) {
        save();
    }
    
    // 停止定时器
    if (m_autoSaveTimer) {
        m_autoSaveTimer->stop();
    }
    
    // 清理文件监控
    if (m_fileWatcher) {
        m_fileWatcher->removePaths(m_fileWatcher->files());
    }
}

bool ConfigFile::exists(const QString& path) const
{
    return QFile::exists(path);
}

IFileHandler::OperationResult ConfigFile::read(const QString& path, QByteArray& data)
{
    if (!validatePath(path)) {
        return InvalidPath;
    }
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (!file.exists()) {
            return FileNotFound;
        } else {
            return PermissionDenied;
        }
    }
    
    data = file.readAll();
    return Success;
}

IFileHandler::OperationResult ConfigFile::write(const QString& path, const QByteArray& data, bool append)
{
    if (!validatePath(path)) {
        return InvalidPath;
    }
    
    QFile file(path);
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
    if (append) {
        mode |= QIODevice::Append;
    }
    
    if (!file.open(mode)) {
        return PermissionDenied;
    }
    
    qint64 written = file.write(data);
    if (written != data.size()) {
        return DiskFull;
    }
    
    return Success;
}

IFileHandler::OperationResult ConfigFile::remove(const QString& path)
{
    if (!validatePath(path)) {
        return InvalidPath;
    }
    
    if (!QFile::exists(path)) {
        return FileNotFound;
    }
    
    if (QFile::remove(path)) {
        return Success;
    } else {
        return PermissionDenied;
    }
}

IFileHandler::OperationResult ConfigFile::copy(const QString& sourcePath, const QString& destPath, bool overwrite)
{
    if (!validatePath(sourcePath) || !validatePath(destPath)) {
        return InvalidPath;
    }
    
    if (!QFile::exists(sourcePath)) {
        return FileNotFound;
    }
    
    if (QFile::exists(destPath) && !overwrite) {
        return UnknownError; // 文件已存在
    }
    
    if (QFile::exists(destPath)) {
        QFile::remove(destPath);
    }
    
    if (QFile::copy(sourcePath, destPath)) {
        return Success;
    } else {
        return UnknownError;
    }
}

IFileHandler::OperationResult ConfigFile::move(const QString& sourcePath, const QString& destPath, bool overwrite)
{
    if (!validatePath(sourcePath) || !validatePath(destPath)) {
        return InvalidPath;
    }
    
    if (!QFile::exists(sourcePath)) {
        return FileNotFound;
    }
    
    if (QFile::exists(destPath) && !overwrite) {
        return UnknownError; // 文件已存在
    }
    
    if (QFile::exists(destPath)) {
        QFile::remove(destPath);
    }
    
    if (QFile::rename(sourcePath, destPath)) {
        return Success;
    } else {
        return UnknownError;
    }
}

IFileHandler::OperationResult ConfigFile::getAttributes(const QString& path, FileAttributes& attributes)
{
    if (!validatePath(path)) {
        return InvalidPath;
    }
    
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        return FileNotFound;
    }
    
    attributes.size = fileInfo.size();
    attributes.created = fileInfo.birthTime();
    attributes.modified = fileInfo.lastModified();
    attributes.accessed = fileInfo.lastRead();
    attributes.readable = fileInfo.isReadable();
    attributes.writable = fileInfo.isWritable();
    attributes.executable = fileInfo.isExecutable();
    attributes.hidden = fileInfo.isHidden();
    
    return Success;
}

IFileHandler::OperationResult ConfigFile::setAttributes(const QString& path, const FileAttributes& attributes)
{
    Q_UNUSED(path)
    Q_UNUSED(attributes)
    // 配置文件处理器不支持设置文件属性
    return UnknownError;
}

qint64 ConfigFile::size(const QString& path) const
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() ? fileInfo.size() : -1;
}

bool ConfigFile::isReadable(const QString& path) const
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isReadable();
}

bool ConfigFile::isWritable(const QString& path) const
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isWritable();
}

bool ConfigFile::isExecutable(const QString& path) const
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isExecutable();
}

QStringList ConfigFile::supportedExtensions() const
{
    return QStringList() << "ini" << "conf" << "cfg" << "json" << "xml";
}

bool ConfigFile::supports(const QString& path) const
{
    QFileInfo fileInfo(path);
    QString extension = fileInfo.suffix().toLower();
    return supportedExtensions().contains(extension);
}

QString ConfigFile::name() const
{
    return "ConfigFile";
}

QString ConfigFile::version() const
{
    return "1.0.0";
}

bool ConfigFile::load()
{
    QMutexLocker locker(&m_mutex);
    
    if (!QFile::exists(m_filePath)) {
        return true; // 文件不存在，使用空配置
    }
    
    bool success = false;
    
    switch (m_format) {
        case IniFormat:
            success = loadIniFormat();
            break;
        case JsonFormat:
            success = loadJsonFormat();
            break;
        case XmlFormat:
            success = loadXmlFormat();
            break;
        default:
            success = false;
            break;
    }
    
    if (success) {
        m_modified = false;
        m_lastModified = QFileInfo(m_filePath).lastModified();
        emit loaded();
    }
    
    return success;
}

bool ConfigFile::save()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_accessMode == ReadOnly) {
        return false;
    }
    
    bool success = false;
    
    switch (m_format) {
        case IniFormat:
            success = saveIniFormat();
            break;
        case JsonFormat:
            success = saveJsonFormat();
            break;
        case XmlFormat:
            success = saveXmlFormat();
            break;
        default:
            success = false;
            break;
    }
    
    if (success) {
        m_modified = false;
        m_lastModified = QFileInfo(m_filePath).lastModified();
        emit saved();
    }
    
    return success;
}

bool ConfigFile::reload()
{
    QMutexLocker locker(&m_mutex);
    
    m_data.clear();
    m_groupStack.clear();
    
    return load();
}

void ConfigFile::setValue(const QString& key, const QVariant& value)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_accessMode == ReadOnly) {
        return;
    }
    
    // 构建完整的键名
    QString fullKey = key;
    if (!m_groupStack.isEmpty()) {
        fullKey = m_groupStack.join("/") + "/" + key;
    }
    
    // 验证值（如果有验证器）
    if (m_validator && !m_validator(fullKey, value)) {
        return;
    }
    
    // 设置值
    QVariant oldValue = m_data.value(fullKey);
    if (oldValue != value) {
        m_data[fullKey] = value;
        markAsModified();
        emit valueChanged(fullKey, value);
    }
}

QVariant ConfigFile::value(const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    
    // 构建完整的键名
    QString fullKey = key;
    if (!m_groupStack.isEmpty()) {
        fullKey = m_groupStack.join("/") + "/" + key;
    }
    
    return m_data.value(fullKey, defaultValue);
}

bool ConfigFile::contains(const QString& key) const
{
    QMutexLocker locker(&m_mutex);
    
    // 构建完整的键名
    QString fullKey = key;
    if (!m_groupStack.isEmpty()) {
        fullKey = m_groupStack.join("/") + "/" + key;
    }
    
    return m_data.contains(fullKey);
}

void ConfigFile::remove(const QString& key)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_accessMode == ReadOnly) {
        return;
    }
    
    // 构建完整的键名
    QString fullKey = key;
    if (!m_groupStack.isEmpty()) {
        fullKey = m_groupStack.join("/") + "/" + key;
    }
    
    if (m_data.remove(fullKey) > 0) {
        markAsModified();
    }
}

QStringList ConfigFile::allKeys() const
{
    QMutexLocker locker(&m_mutex);
    return m_data.keys();
}

QStringList ConfigFile::childKeys(const QString& prefix) const
{
    QMutexLocker locker(&m_mutex);
    
    QString searchPrefix = prefix;
    if (!m_groupStack.isEmpty()) {
        if (searchPrefix.isEmpty()) {
            searchPrefix = m_groupStack.join("/");
        } else {
            searchPrefix = m_groupStack.join("/") + "/" + searchPrefix;
        }
    }
    
    QStringList result;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        const QString& key = it.key();
        if (key.startsWith(searchPrefix + "/")) {
            QString relativePath = key.mid(searchPrefix.length() + 1);
            if (!relativePath.contains("/")) {
                result << relativePath;
            }
        }
    }
    
    return result;
}

QStringList ConfigFile::childGroups(const QString& prefix) const
{
    QMutexLocker locker(&m_mutex);
    
    QString searchPrefix = prefix;
    if (!m_groupStack.isEmpty()) {
        if (searchPrefix.isEmpty()) {
            searchPrefix = m_groupStack.join("/");
        } else {
            searchPrefix = m_groupStack.join("/") + "/" + searchPrefix;
        }
    }
    
    QStringList result;
    QSet<QString> groups;
    
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        const QString& key = it.key();
        if (key.startsWith(searchPrefix + "/")) {
            QString relativePath = key.mid(searchPrefix.length() + 1);
            int slashIndex = relativePath.indexOf("/");
            if (slashIndex > 0) {
                QString groupName = relativePath.left(slashIndex);
                groups.insert(groupName);
            }
        }
    }
    
    return groups.toList();
}

void ConfigFile::beginGroup(const QString& group)
{
    QMutexLocker locker(&m_mutex);
    m_groupStack.append(group);
}

void ConfigFile::endGroup()
{
    QMutexLocker locker(&m_mutex);
    if (!m_groupStack.isEmpty()) {
        m_groupStack.removeLast();
    }
}

QString ConfigFile::group() const
{
    QMutexLocker locker(&m_mutex);
    return m_groupStack.join("/");
}

void ConfigFile::clear()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_accessMode == ReadOnly) {
        return;
    }
    
    if (!m_data.isEmpty()) {
        m_data.clear();
        m_groupStack.clear();
        markAsModified();
    }
}

void ConfigFile::setFormat(Format format)
{
    QMutexLocker locker(&m_mutex);
    m_format = format;
}

ConfigFile::Format ConfigFile::format() const
{
    QMutexLocker locker(&m_mutex);
    return m_format;
}

void ConfigFile::setAccessMode(AccessMode mode)
{
    QMutexLocker locker(&m_mutex);
    m_accessMode = mode;
}

ConfigFile::AccessMode ConfigFile::accessMode() const
{
    QMutexLocker locker(&m_mutex);
    return m_accessMode;
}

void ConfigFile::setAutoSave(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_autoSave = enabled;
}

bool ConfigFile::isAutoSave() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoSave;
}

void ConfigFile::setAutoSaveInterval(int interval)
{
    QMutexLocker locker(&m_mutex);
    m_autoSaveInterval = interval;
}

int ConfigFile::autoSaveInterval() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoSaveInterval;
}

void ConfigFile::setFileWatchEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    
    m_fileWatchEnabled = enabled;
    
    if (enabled && QFile::exists(m_filePath)) {
        if (!m_fileWatcher->files().contains(m_filePath)) {
            m_fileWatcher->addPath(m_filePath);
        }
    } else {
        m_fileWatcher->removePath(m_filePath);
    }
}

bool ConfigFile::isFileWatchEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_fileWatchEnabled;
}

void ConfigFile::setValidator(std::function<bool(const QString&, const QVariant&)> validator)
{
    QMutexLocker locker(&m_mutex);
    m_validator = validator;
}

bool ConfigFile::validate() const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_validator) {
        return true;
    }
    
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        if (!m_validator(it.key(), it.value())) {
            return false;
        }
    }
    
    return true;
}

QString ConfigFile::filePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_filePath;
}

bool ConfigFile::isModified() const
{
    QMutexLocker locker(&m_mutex);
    return m_modified;
}

QDateTime ConfigFile::lastModified() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastModified;
}

bool ConfigFile::createBackup(const QString& backupPath)
{
    QMutexLocker locker(&m_mutex);
    
    QString backup = backupPath;
    if (backup.isEmpty()) {
        backup = m_filePath + ".backup";
    }
    
    return QFile::copy(m_filePath, backup);
}

bool ConfigFile::restoreFromBackup(const QString& backupPath)
{
    QMutexLocker locker(&m_mutex);
    
    QString backup = backupPath;
    if (backup.isEmpty()) {
        backup = m_filePath + ".backup";
    }
    
    if (!QFile::exists(backup)) {
        return false;
    }
    
    if (QFile::exists(m_filePath)) {
        QFile::remove(m_filePath);
    }
    
    if (QFile::copy(backup, m_filePath)) {
        return reload();
    }
    
    return false;
}

bool ConfigFile::exportTo(const QString& exportPath, Format exportFormat)
{
    // 创建临时配置文件对象
    ConfigFile tempConfig(exportPath, exportFormat);
    tempConfig.m_data = m_data;
    
    return tempConfig.save();
}

bool ConfigFile::importFrom(const QString& importPath, bool merge)
{
    ConfigFile tempConfig(importPath);
    if (!tempConfig.load()) {
        return false;
    }
    
    QMutexLocker locker(&m_mutex);
    
    if (!merge) {
        m_data.clear();
    }
    
    // 合并数据
    for (auto it = tempConfig.m_data.begin(); it != tempConfig.m_data.end(); ++it) {
        m_data[it.key()] = it.value();
    }
    
    markAsModified();
    return true;
}

void ConfigFile::onFileChanged(const QString& path)
{
    if (path == m_filePath) {
        emit fileChanged();
        
        // 可选择自动重新加载
        // reload();
    }
}

void ConfigFile::onAutoSaveTimer()
{
    if (m_modified) {
        save();
    }
}

ConfigFile::Format ConfigFile::detectFormat(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "json") {
        return JsonFormat;
    } else if (extension == "xml") {
        return XmlFormat;
    } else {
        return IniFormat; // 默认格式
    }
}

bool ConfigFile::loadIniFormat()
{
    QSettings settings(m_filePath, QSettings::IniFormat);
    
    QStringList keys = settings.allKeys();
    for (const QString& key : keys) {
        m_data[key] = settings.value(key);
    }
    
    return true;
}

bool ConfigFile::saveIniFormat()
{
    QSettings settings(m_filePath, QSettings::IniFormat);
    settings.clear();
    
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        settings.setValue(it.key(), it.value());
    }
    
    settings.sync();
    return settings.status() == QSettings::NoError;
}

bool ConfigFile::loadJsonFormat()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    // 递归解析JSON对象
    std::function<void(const QJsonObject&, const QString&)> parseObject;
    parseObject = [&](const QJsonObject& obj, const QString& prefix) {
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            QString key = prefix.isEmpty() ? it.key() : prefix + "/" + it.key();
            QJsonValue value = it.value();
            
            if (value.isObject()) {
                parseObject(value.toObject(), key);
            } else {
                m_data[key] = value.toVariant();
            }
        }
    };
    
    parseObject(doc.object(), QString());
    return true;
}

bool ConfigFile::saveJsonFormat()
{
    // 构建JSON对象
    QJsonObject rootObj;
    
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        QStringList keyParts = it.key().split("/");
        QJsonObject* currentObj = &rootObj;
        
        for (int i = 0; i < keyParts.size() - 1; ++i) {
            const QString& part = keyParts[i];
            if (!currentObj->contains(part)) {
                currentObj->insert(part, QJsonObject());
            }
            QJsonValue value = currentObj->value(part);
            if (value.isObject()) {
                QJsonObject obj = value.toObject();
                currentObj = &obj;
                currentObj->insert(part, obj);
            }
        }
        
        currentObj->insert(keyParts.last(), QJsonValue::fromVariant(it.value()));
    }
    
    // 写入文件
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(rootObj);
    file.write(doc.toJson());
    
    return true;
}

bool ConfigFile::loadXmlFormat()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        return false;
    }
    
    // 递归解析XML元素
    std::function<void(const QDomElement&, const QString&)> parseElement;
    parseElement = [&](const QDomElement& element, const QString& prefix) {
        QDomNodeList children = element.childNodes();
        
        for (int i = 0; i < children.count(); ++i) {
            QDomNode node = children.at(i);
            if (node.isElement()) {
                QDomElement childElement = node.toElement();
                QString key = prefix.isEmpty() ? childElement.tagName() : prefix + "/" + childElement.tagName();
                
                if (childElement.hasChildNodes() && childElement.firstChild().isText()) {
                    // 叶子节点，存储文本值
                    m_data[key] = childElement.text();
                } else {
                    // 继续递归
                    parseElement(childElement, key);
                }
            }
        }
    };
    
    parseElement(doc.documentElement(), QString());
    return true;
}

bool ConfigFile::saveXmlFormat()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("configuration");
    doc.appendChild(root);
    
    // 构建XML结构
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        QStringList keyParts = it.key().split("/");
        QDomElement currentElement = root;
        
        for (int i = 0; i < keyParts.size(); ++i) {
            const QString& part = keyParts[i];
            
            // 查找或创建子元素
            QDomElement childElement;
            QDomNodeList children = currentElement.childNodes();
            bool found = false;
            
            for (int j = 0; j < children.count(); ++j) {
                QDomNode node = children.at(j);
                if (node.isElement() && node.toElement().tagName() == part) {
                    childElement = node.toElement();
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                childElement = doc.createElement(part);
                currentElement.appendChild(childElement);
            }
            
            if (i == keyParts.size() - 1) {
                // 最后一个部分，设置文本值
                childElement.appendChild(doc.createTextNode(it.value().toString()));
            } else {
                currentElement = childElement;
            }
        }
    }
    
    // 写入文件
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << doc.toString();
    
    return true;
}

bool ConfigFile::ensureDirectoryExists(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    
    if (!dir.exists()) {
        return dir.mkpath(dir.absolutePath());
    }
    
    return true;
}

void ConfigFile::markAsModified()
{
    m_modified = true;
    
    if (m_autoSave && m_autoSaveInterval > 0) {
        m_autoSaveTimer->start(m_autoSaveInterval);
    }
}