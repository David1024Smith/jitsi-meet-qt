#include "TempFile.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

// 静态成员初始化
QList<TempFile*> TempFile::s_tempFiles;
QMutex TempFile::s_globalMutex;

TempFile::TempFile(const QString& nameTemplate, TempFileType type, QObject* parent)
    : IFileHandler(parent)
    , m_tempFile(nullptr)
    , m_nameTemplate(nameTemplate)
    , m_type(type)
    , m_cleanupPolicy(Immediate)
    , m_timeToLive(0)
    , m_maxSize(0)
    , m_ttlTimer(new QTimer(this))
{
    // 设置TTL定时器
    m_ttlTimer->setSingleShot(true);
    connect(m_ttlTimer, &QTimer::timeout, this, &TempFile::onTTLTimer);
    
    // 注册到全局列表
    registerTempFile();
}

TempFile::~TempFile()
{
    cleanup();
    unregisterTempFile();
}

bool TempFile::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    // 创建临时文件
    return create();
}

void TempFile::cleanup()
{
    QMutexLocker locker(&m_mutex);
    
    // 停止定时器
    if (m_ttlTimer) {
        m_ttlTimer->stop();
    }
    
    // 执行清理
    performCleanup();
}

bool TempFile::exists(const QString& path) const
{
    return QFile::exists(path);
}

IFileHandler::OperationResult TempFile::read(const QString& path, QByteArray& data)
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

IFileHandler::OperationResult TempFile::write(const QString& path, const QByteArray& data, bool append)
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

IFileHandler::OperationResult TempFile::remove(const QString& path)
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

IFileHandler::OperationResult TempFile::copy(const QString& sourcePath, const QString& destPath, bool overwrite)
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

IFileHandler::OperationResult TempFile::move(const QString& sourcePath, const QString& destPath, bool overwrite)
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

IFileHandler::OperationResult TempFile::getAttributes(const QString& path, FileAttributes& attributes)
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

IFileHandler::OperationResult TempFile::setAttributes(const QString& path, const FileAttributes& attributes)
{
    Q_UNUSED(path)
    Q_UNUSED(attributes)
    // 临时文件处理器不支持设置文件属性
    return UnknownError;
}

qint64 TempFile::size(const QString& path) const
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() ? fileInfo.size() : -1;
}

bool TempFile::isReadable(const QString& path) const
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isReadable();
}

bool TempFile::isWritable(const QString& path) const
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isWritable();
}

bool TempFile::isExecutable(const QString& path) const
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isExecutable();
}

QStringList TempFile::supportedExtensions() const
{
    return QStringList() << "*"; // 支持所有扩展名
}

bool TempFile::supports(const QString& path) const
{
    Q_UNUSED(path)
    return true; // 支持所有文件
}

QString TempFile::name() const
{
    return "TempFile";
}

QString TempFile::version() const
{
    return "1.0.0";
}

bool TempFile::create()
{
    if (m_tempFile) {
        delete m_tempFile;
    }
    
    QString templateStr = m_nameTemplate;
    if (templateStr.isEmpty()) {
        templateStr = "jitsi-temp-XXXXXX";
    }
    
    m_tempFile = new QTemporaryFile(templateStr, this);
    
    // 设置自动删除行为
    m_tempFile->setAutoRemove(m_type == AutoDelete);
    
    if (m_tempFile->open()) {
        m_creationTime = QDateTime::currentDateTime();
        
        // 初始化TTL定时器
        if (m_timeToLive > 0) {
            initializeTTLTimer();
        }
        
        emit fileCreated(m_tempFile->fileName());
        return true;
    }
    
    return false;
}

bool TempFile::open(QIODevice::OpenMode mode)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_tempFile) {
        if (!create()) {
            return false;
        }
    }
    
    if (m_tempFile->isOpen()) {
        m_tempFile->close();
    }
    
    return m_tempFile->open(mode);
}

void TempFile::close()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile && m_tempFile->isOpen()) {
        m_tempFile->close();
    }
}

bool TempFile::isOpen() const
{
    QMutexLocker locker(&m_mutex);
    return m_tempFile && m_tempFile->isOpen();
}

qint64 TempFile::write(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_tempFile || !m_tempFile->isOpen()) {
        return -1;
    }
    
    // 检查大小限制
    if (m_maxSize > 0 && (m_tempFile->size() + data.size()) > m_maxSize) {
        emit sizeLimitExceeded(m_tempFile->fileName(), m_tempFile->size() + data.size(), m_maxSize);
        return -1;
    }
    
    return m_tempFile->write(data);
}

qint64 TempFile::write(const QString& text)
{
    return write(text.toUtf8());
}

QByteArray TempFile::readAll()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_tempFile || !m_tempFile->isOpen()) {
        return QByteArray();
    }
    
    return m_tempFile->readAll();
}

QByteArray TempFile::read(qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_tempFile || !m_tempFile->isOpen()) {
        return QByteArray();
    }
    
    return m_tempFile->read(maxSize);
}

QByteArray TempFile::readLine(qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_tempFile || !m_tempFile->isOpen()) {
        return QByteArray();
    }
    
    return m_tempFile->readLine(maxSize);
}

bool TempFile::flush()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_tempFile || !m_tempFile->isOpen()) {
        return false;
    }
    
    return m_tempFile->flush();
}

QString TempFile::fileName() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile) {
        return m_tempFile->fileName();
    }
    
    return QString();
}

qint64 TempFile::size() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile) {
        return m_tempFile->size();
    }
    
    return 0;
}

bool TempFile::setPermissions(QFile::Permissions permissions)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile) {
        return m_tempFile->setPermissions(permissions);
    }
    
    return false;
}

QFile::Permissions TempFile::permissions() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile) {
        return m_tempFile->permissions();
    }
    
    return QFile::Permissions();
}

void TempFile::setTempFileType(TempFileType type)
{
    QMutexLocker locker(&m_mutex);
    
    m_type = type;
    
    if (m_tempFile) {
        m_tempFile->setAutoRemove(type == AutoDelete);
    }
}

TempFile::TempFileType TempFile::tempFileType() const
{
    QMutexLocker locker(&m_mutex);
    return m_type;
}

void TempFile::setCleanupPolicy(CleanupPolicy policy)
{
    QMutexLocker locker(&m_mutex);
    m_cleanupPolicy = policy;
}

TempFile::CleanupPolicy TempFile::cleanupPolicy() const
{
    QMutexLocker locker(&m_mutex);
    return m_cleanupPolicy;
}

void TempFile::setTimeToLive(int ttl)
{
    QMutexLocker locker(&m_mutex);
    
    m_timeToLive = ttl;
    
    if (ttl > 0 && m_tempFile) {
        initializeTTLTimer();
    } else if (m_ttlTimer) {
        m_ttlTimer->stop();
    }
}

int TempFile::timeToLive() const
{
    QMutexLocker locker(&m_mutex);
    return m_timeToLive;
}

void TempFile::setMaxSize(qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);
    m_maxSize = maxSize;
}

qint64 TempFile::maxSize() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxSize;
}

bool TempFile::rename(const QString& newName)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile) {
        return m_tempFile->rename(newName);
    }
    
    return false;
}

bool TempFile::copyTo(const QString& destPath, bool keepOriginal)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_tempFile) {
        return false;
    }
    
    QString sourcePath = m_tempFile->fileName();
    
    if (QFile::copy(sourcePath, destPath)) {
        if (!keepOriginal) {
            remove();
        }
        return true;
    }
    
    return false;
}

bool TempFile::moveTo(const QString& destPath)
{
    return copyTo(destPath, false);
}

void TempFile::setAutoRemove(bool autoDelete)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile) {
        m_tempFile->setAutoRemove(autoDelete);
    }
    
    m_type = autoDelete ? AutoDelete : ManualDelete;
}

bool TempFile::autoRemove() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile) {
        return m_tempFile->autoRemove();
    }
    
    return m_type == AutoDelete;
}

bool TempFile::remove()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile) {
        QString filePath = m_tempFile->fileName();
        bool success = m_tempFile->remove();
        
        if (success) {
            emit fileRemoved(filePath);
        }
        
        return success;
    }
    
    return false;
}

QDateTime TempFile::creationTime() const
{
    QMutexLocker locker(&m_mutex);
    return m_creationTime;
}

QDateTime TempFile::lastAccessTime() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile) {
        QFileInfo fileInfo(m_tempFile->fileName());
        return fileInfo.lastRead();
    }
    
    return QDateTime();
}

QDateTime TempFile::lastModifiedTime() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tempFile) {
        QFileInfo fileInfo(m_tempFile->fileName());
        return fileInfo.lastModified();
    }
    
    return QDateTime();
}

bool TempFile::isExpired() const
{
    if (m_timeToLive <= 0) {
        return false;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    return m_creationTime.msecsTo(now) >= m_timeToLive;
}

QString TempFile::createTempFile(const QString& nameTemplate, const QByteArray& data)
{
    TempFile tempFile(nameTemplate, AutoDelete);
    if (tempFile.create()) {
        if (!data.isEmpty()) {
            tempFile.write(data);
        }
        return tempFile.fileName();
    }
    
    return QString();
}

QString TempFile::createTempDir(const QString& nameTemplate)
{
    QString templateStr = nameTemplate;
    if (templateStr.isEmpty()) {
        templateStr = "jitsi-temp-dir-XXXXXX";
    }
    
    QTemporaryDir tempDir(templateStr);
    if (tempDir.isValid()) {
        tempDir.setAutoRemove(false); // 不自动删除，由调用者管理
        return tempDir.path();
    }
    
    return QString();
}

QString TempFile::tempPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}

int TempFile::cleanupExpiredFiles(const QString& directory, int maxAge)
{
    QDir dir(directory);
    if (!dir.exists()) {
        return 0;
    }
    
    QDateTime cutoffTime = QDateTime::currentDateTime().addSecs(-maxAge);
    QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    
    int cleanedCount = 0;
    for (const QFileInfo& fileInfo : files) {
        if (fileInfo.lastModified() < cutoffTime) {
            if (QFile::remove(fileInfo.absoluteFilePath())) {
                cleanedCount++;
            }
        }
    }
    
    return cleanedCount;
}

QVariantMap TempFile::getStatistics()
{
    QMutexLocker locker(&s_globalMutex);
    
    QVariantMap stats;
    stats["totalTempFiles"] = s_tempFiles.size();
    
    int activeFiles = 0;
    int expiredFiles = 0;
    qint64 totalSize = 0;
    
    for (TempFile* tempFile : s_tempFiles) {
        if (tempFile->m_tempFile && QFile::exists(tempFile->fileName())) {
            activeFiles++;
            totalSize += tempFile->size();
            
            if (tempFile->isExpired()) {
                expiredFiles++;
            }
        }
    }
    
    stats["activeFiles"] = activeFiles;
    stats["expiredFiles"] = expiredFiles;
    stats["totalSize"] = static_cast<qint64>(totalSize);
    
    return stats;
}

void TempFile::onTTLTimer()
{
    QMutexLocker locker(&m_mutex);
    
    emit fileExpired(fileName());
    
    // 根据清理策略执行操作
    if (m_cleanupPolicy == Immediate) {
        performCleanup();
    }
}

void TempFile::onCleanupTimer()
{
    // 全局清理定时器的处理（如果需要）
}

void TempFile::initializeTTLTimer()
{
    if (m_timeToLive > 0 && m_ttlTimer) {
        m_ttlTimer->start(m_timeToLive);
    }
}

bool TempFile::checkSizeLimit()
{
    if (m_maxSize <= 0 || !m_tempFile) {
        return true;
    }
    
    return m_tempFile->size() <= m_maxSize;
}

void TempFile::performCleanup()
{
    if (m_tempFile) {
        QString filePath = m_tempFile->fileName();
        
        switch (m_cleanupPolicy) {
            case Immediate:
                if (m_tempFile->exists()) {
                    m_tempFile->remove();
                    emit fileRemoved(filePath);
                }
                break;
            case Delayed:
                // 延迟删除可以通过定时器实现
                break;
            case OnExit:
                // 程序退出时删除，由析构函数处理
                break;
            case Never:
                // 永不删除
                m_tempFile->setAutoRemove(false);
                break;
        }
        
        delete m_tempFile;
        m_tempFile = nullptr;
    }
}

void TempFile::registerTempFile()
{
    QMutexLocker locker(&s_globalMutex);
    s_tempFiles.append(this);
}

void TempFile::unregisterTempFile()
{
    QMutexLocker locker(&s_globalMutex);
    s_tempFiles.removeAll(this);
}