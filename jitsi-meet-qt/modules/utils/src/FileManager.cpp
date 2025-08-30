#include "../include/FileManager.h"
#include "../interfaces/IFileHandler.h"
#include "../file/FileWatcher.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

// 静态成员初始化
FileManager* FileManager::s_instance = nullptr;
QMutex FileManager::s_mutex;

FileManager* FileManager::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new FileManager();
    }
    return s_instance;
}

FileManager::FileManager(QObject* parent)
    : QObject(parent)
    , m_fileWatcher(std::make_unique<FileWatcher>(this))
    , m_cacheEnabled(true)
{
    // 连接文件监控器信号
    connect(m_fileWatcher.get(), &FileWatcher::fileEvent,
            this, [this](const FileWatcher::FileEvent& event) {
                if (event.type == FileWatcher::FileModified) {
                    emit fileChanged(event.path);
                } else if (event.type == FileWatcher::DirectoryModified) {
                    emit directoryChanged(event.path);
                }
            });
}

FileManager::~FileManager()
{
    cleanup();
}

bool FileManager::initialize()
{
    QMutexLocker locker(&m_operationMutex);
    
    // 初始化文件监控器
    if (!m_fileWatcher->initialize()) {
        return false;
    }
    
    return true;
}

void FileManager::cleanup()
{
    QMutexLocker locker(&m_operationMutex);
    
    // 清理文件监控器
    if (m_fileWatcher) {
        m_fileWatcher->cleanup();
    }
    
    // 清理文件处理器
    m_fileHandlers.clear();
    
    // 清理缓存
    clearCache();
}

void FileManager::registerFileHandler(const QString& extension, std::shared_ptr<IFileHandler> handler)
{
    if (!handler) {
        return;
    }
    
    QMutexLocker locker(&m_operationMutex);
    
    // 初始化处理器
    handler->initialize();
    
    // 注册处理器
    m_fileHandlers[extension.toLower()] = handler;
    
    // 连接信号
    connect(handler.get(), &IFileHandler::operationCompleted,
            this, &FileManager::operationCompleted);
}

void FileManager::unregisterFileHandler(const QString& extension)
{
    QMutexLocker locker(&m_operationMutex);
    
    auto it = m_fileHandlers.find(extension.toLower());
    if (it != m_fileHandlers.end()) {
        // 断开信号连接
        disconnect(it.value().get(), nullptr, this, nullptr);
        
        // 清理处理器
        it.value()->cleanup();
        
        // 移除处理器
        m_fileHandlers.erase(it);
    }
}

bool FileManager::exists(const QString& path) const
{
    if (!validatePath(path)) {
        return false;
    }
    
    return QFile::exists(path);
}

FileManager::FileInfo FileManager::getFileInfo(const QString& path) const
{
    FileInfo info;
    
    if (!validatePath(path) || !exists(path)) {
        return info;
    }
    
    QFileInfo fileInfo(path);
    
    info.path = fileInfo.absoluteFilePath();
    info.name = fileInfo.fileName();
    info.size = fileInfo.size();
    info.created = fileInfo.birthTime();
    info.modified = fileInfo.lastModified();
    info.accessed = fileInfo.lastRead();
    info.readable = fileInfo.isReadable();
    info.writable = fileInfo.isWritable();
    info.executable = fileInfo.isExecutable();
    
    if (fileInfo.isFile()) {
        info.type = RegularFile;
    } else if (fileInfo.isDir()) {
        info.type = Directory;
    } else if (fileInfo.isSymLink()) {
        info.type = SymbolicLink;
    } else {
        info.type = Unknown;
    }
    
    return info;
}

FileManager::OperationResult FileManager::readFile(const QString& path, QByteArray& data)
{
    if (!validatePath(path)) {
        return InvalidPath;
    }
    
    // 尝试从缓存获取
    if (m_cacheEnabled && getFromCache(path, data)) {
        return Success;
    }
    
    // 获取文件处理器
    auto handler = getFileHandler(path);
    if (handler) {
        IFileHandler::OperationResult result = handler->read(path, data);
        
        // 存入缓存
        if (result == IFileHandler::Success && m_cacheEnabled) {
            putToCache(path, data);
        }
        
        return static_cast<OperationResult>(result);
    }
    
    // 使用默认文件操作
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (!file.exists()) {
            return FileNotFound;
        } else if (!QFileInfo(path).isReadable()) {
            return PermissionDenied;
        } else {
            return UnknownError;
        }
    }
    
    data = file.readAll();
    
    // 存入缓存
    if (m_cacheEnabled) {
        putToCache(path, data);
    }
    
    return Success;
}

FileManager::OperationResult FileManager::writeFile(const QString& path, const QByteArray& data, bool append)
{
    if (!validatePath(path)) {
        return InvalidPath;
    }
    
    // 获取文件处理器
    auto handler = getFileHandler(path);
    if (handler) {
        IFileHandler::OperationResult result = handler->write(path, data, append);
        
        // 更新缓存
        if (result == IFileHandler::Success && m_cacheEnabled && !append) {
            putToCache(path, data);
        }
        
        return static_cast<OperationResult>(result);
    }
    
    // 使用默认文件操作
    QFile file(path);
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
    if (append) {
        mode |= QIODevice::Append;
    }
    
    if (!file.open(mode)) {
        QFileInfo fileInfo(path);
        if (!fileInfo.dir().exists()) {
            return InvalidPath;
        } else if (fileInfo.exists() && !fileInfo.isWritable()) {
            return PermissionDenied;
        } else {
            return UnknownError;
        }
    }
    
    qint64 written = file.write(data);
    if (written != data.size()) {
        return DiskFull;
    }
    
    // 更新缓存
    if (m_cacheEnabled && !append) {
        putToCache(path, data);
    }
    
    return Success;
}

FileManager::OperationResult FileManager::copyFile(const QString& sourcePath, const QString& destPath, bool overwrite)
{
    if (!validatePath(sourcePath) || !validatePath(destPath)) {
        return InvalidPath;
    }
    
    if (!exists(sourcePath)) {
        return FileNotFound;
    }
    
    if (exists(destPath) && !overwrite) {
        return UnknownError; // 文件已存在
    }
    
    // 获取文件处理器
    auto handler = getFileHandler(sourcePath);
    if (handler) {
        return static_cast<OperationResult>(handler->copy(sourcePath, destPath, overwrite));
    }
    
    // 使用默认文件操作
    if (exists(destPath)) {
        QFile::remove(destPath);
    }
    
    if (QFile::copy(sourcePath, destPath)) {
        return Success;
    } else {
        return UnknownError;
    }
}

FileManager::OperationResult FileManager::moveFile(const QString& sourcePath, const QString& destPath, bool overwrite)
{
    if (!validatePath(sourcePath) || !validatePath(destPath)) {
        return InvalidPath;
    }
    
    if (!exists(sourcePath)) {
        return FileNotFound;
    }
    
    if (exists(destPath) && !overwrite) {
        return UnknownError; // 文件已存在
    }
    
    // 获取文件处理器
    auto handler = getFileHandler(sourcePath);
    if (handler) {
        return static_cast<OperationResult>(handler->move(sourcePath, destPath, overwrite));
    }
    
    // 使用默认文件操作
    if (exists(destPath)) {
        QFile::remove(destPath);
    }
    
    if (QFile::rename(sourcePath, destPath)) {
        return Success;
    } else {
        return UnknownError;
    }
}

FileManager::OperationResult FileManager::deleteFile(const QString& path)
{
    if (!validatePath(path)) {
        return InvalidPath;
    }
    
    if (!exists(path)) {
        return FileNotFound;
    }
    
    // 获取文件处理器
    auto handler = getFileHandler(path);
    if (handler) {
        return static_cast<OperationResult>(handler->remove(path));
    }
    
    // 使用默认文件操作
    if (QFile::remove(path)) {
        // 从缓存中移除
        QMutexLocker locker(&m_cacheMutex);
        m_fileCache.remove(path);
        
        return Success;
    } else {
        return UnknownError;
    }
}

FileManager::OperationResult FileManager::createDirectory(const QString& path, bool recursive)
{
    if (!validatePath(path)) {
        return InvalidPath;
    }
    
    QDir dir;
    bool success;
    
    if (recursive) {
        success = dir.mkpath(path);
    } else {
        success = dir.mkdir(path);
    }
    
    return success ? Success : UnknownError;
}

FileManager::OperationResult FileManager::removeDirectory(const QString& path, bool recursive)
{
    if (!validatePath(path)) {
        return InvalidPath;
    }
    
    if (!exists(path)) {
        return FileNotFound;
    }
    
    QDir dir(path);
    bool success;
    
    if (recursive) {
        success = dir.removeRecursively();
    } else {
        success = dir.rmdir(path);
    }
    
    return success ? Success : UnknownError;
}

QStringList FileManager::listDirectory(const QString& path, const QStringList& nameFilters, bool recursive) const
{
    QStringList result;
    
    if (!validatePath(path) || !exists(path)) {
        return result;
    }
    
    QDir dir(path);
    if (!nameFilters.isEmpty()) {
        dir.setNameFilters(nameFilters);
    }
    
    if (recursive) {
        // 递归列出所有文件
        QDirIterator it(path, nameFilters, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            result << it.next();
        }
    } else {
        // 只列出当前目录的文件
        QStringList entries = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString& entry : entries) {
            result << dir.absoluteFilePath(entry);
        }
    }
    
    return result;
}

QString FileManager::tempPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}

QString FileManager::appDataPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString FileManager::documentsPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

void FileManager::setCacheEnabled(bool enabled)
{
    QMutexLocker locker(&m_cacheMutex);
    m_cacheEnabled = enabled;
    
    if (!enabled) {
        m_fileCache.clear();
    }
}

bool FileManager::isCacheEnabled() const
{
    QMutexLocker locker(&m_cacheMutex);
    return m_cacheEnabled;
}

void FileManager::clearCache()
{
    QMutexLocker locker(&m_cacheMutex);
    m_fileCache.clear();
}

bool FileManager::addFileWatch(const QString& path)
{
    if (!validatePath(path) || !m_fileWatcher) {
        return false;
    }
    
    return m_fileWatcher->addWatch(path);
}

bool FileManager::removeFileWatch(const QString& path)
{
    if (!m_fileWatcher) {
        return false;
    }
    
    return m_fileWatcher->removeWatch(path);
}

QString FileManager::resultToString(OperationResult result)
{
    switch (result) {
        case Success: return "Success";
        case FileNotFound: return "File not found";
        case PermissionDenied: return "Permission denied";
        case DiskFull: return "Disk full";
        case InvalidPath: return "Invalid path";
        case UnknownError: return "Unknown error";
        default: return "Unknown result";
    }
}

std::shared_ptr<IFileHandler> FileManager::getFileHandler(const QString& path) const
{
    QFileInfo fileInfo(path);
    QString extension = fileInfo.suffix().toLower();
    
    auto it = m_fileHandlers.find(extension);
    if (it != m_fileHandlers.end()) {
        return it.value();
    }
    
    return nullptr;
}

bool FileManager::validatePath(const QString& path) const
{
    if (path.isEmpty()) {
        return false;
    }
    
    // 检查路径中是否包含危险字符
    if (path.contains("..") || path.contains("//")) {
        return false;
    }
    
    return true;
}

bool FileManager::getFromCache(const QString& path, QByteArray& data) const
{
    QMutexLocker locker(&m_cacheMutex);
    
    auto it = m_fileCache.find(path);
    if (it != m_fileCache.end()) {
        data = it.value();
        return true;
    }
    
    return false;
}

void FileManager::putToCache(const QString& path, const QByteArray& data)
{
    QMutexLocker locker(&m_cacheMutex);
    
    // 简单的缓存大小限制（可以改进）
    const int maxCacheSize = 100;
    if (m_fileCache.size() >= maxCacheSize) {
        // 移除一些旧的缓存项
        auto it = m_fileCache.begin();
        for (int i = 0; i < 10 && it != m_fileCache.end(); ++i) {
            it = m_fileCache.erase(it);
        }
    }
    
    m_fileCache[path] = data;
}