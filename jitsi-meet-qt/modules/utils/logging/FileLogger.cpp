#include "FileLogger.h"
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

FileLogger::FileLogger(const QString& filePath, QObject* parent)
    : ILogger(parent)
    , m_filePath(filePath)
    , m_logLevel(Info)
    , m_format("{timestamp} [{level}] {category}: {message}")
    , m_enabled(true)
    , m_logFile(nullptr)
    , m_stream(nullptr)
    , m_maxFileSize(10 * 1024 * 1024) // 10MB
    , m_maxBackupFiles(5)
    , m_rotationEnabled(true)
    , m_flushTimer(new QTimer(this))
    , m_flushInterval(5000) // 5秒
{
    // 设置自动刷新定时器
    m_flushTimer->setSingleShot(false);
    connect(m_flushTimer, &QTimer::timeout, this, &FileLogger::onFlushTimer);
}

FileLogger::~FileLogger()
{
    cleanup();
}

bool FileLogger::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    // 确保目录存在
    QFileInfo fileInfo(m_filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(dir.absolutePath())) {
            emit errorOccurred(QString("Failed to create log directory: %1").arg(dir.absolutePath()));
            return false;
        }
    }
    
    // 打开日志文件
    if (!openLogFile()) {
        return false;
    }
    
    // 启动自动刷新定时器
    if (m_flushInterval > 0) {
        m_flushTimer->start(m_flushInterval);
    }
    
    return true;
}

void FileLogger::cleanup()
{
    QMutexLocker locker(&m_mutex);
    
    // 停止定时器
    if (m_flushTimer) {
        m_flushTimer->stop();
    }
    
    // 关闭文件
    closeLogFile();
}

void FileLogger::log(const LogEntry& entry)
{
    if (!shouldLog(entry.level)) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // 检查是否需要轮转
    if (m_rotationEnabled && needsRotation()) {
        performRotation();
    }
    
    // 写入日志条目
    writeToFile(entry);
    
    // 发出信号
    emit logRecorded(entry);
}

void FileLogger::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

ILogger::LogLevel FileLogger::logLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_logLevel;
}

void FileLogger::setFormat(const QString& format)
{
    QMutexLocker locker(&m_mutex);
    m_format = format;
}

QString FileLogger::format() const
{
    QMutexLocker locker(&m_mutex);
    return m_format;
}

bool FileLogger::isEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_enabled;
}

void FileLogger::setEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_enabled = enabled;
}

QString FileLogger::name() const
{
    return "FileLogger";
}

void FileLogger::flush()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_stream) {
        m_stream->flush();
    }
    if (m_logFile) {
        m_logFile->flush();
    }
}

void FileLogger::setFilePath(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_filePath != filePath) {
        closeLogFile();
        m_filePath = filePath;
        if (m_enabled) {
            openLogFile();
        }
    }
}

QString FileLogger::filePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_filePath;
}

void FileLogger::setMaxFileSize(qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);
    m_maxFileSize = maxSize;
}

qint64 FileLogger::maxFileSize() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxFileSize;
}

void FileLogger::setMaxBackupFiles(int maxBackups)
{
    QMutexLocker locker(&m_mutex);
    m_maxBackupFiles = maxBackups;
}

int FileLogger::maxBackupFiles() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxBackupFiles;
}

void FileLogger::setFlushInterval(int interval)
{
    QMutexLocker locker(&m_mutex);
    
    m_flushInterval = interval;
    
    if (m_flushTimer) {
        if (interval > 0) {
            m_flushTimer->start(interval);
        } else {
            m_flushTimer->stop();
        }
    }
}

int FileLogger::flushInterval() const
{
    QMutexLocker locker(&m_mutex);
    return m_flushInterval;
}

void FileLogger::setRotationEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_rotationEnabled = enabled;
}

bool FileLogger::isRotationEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_rotationEnabled;
}

bool FileLogger::rotateLog()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_rotationEnabled) {
        return false;
    }
    
    performRotation();
    return true;
}

qint64 FileLogger::currentFileSize() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_logFile) {
        return m_logFile->size();
    }
    
    QFileInfo fileInfo(m_filePath);
    if (fileInfo.exists()) {
        return fileInfo.size();
    }
    
    return 0;
}

void FileLogger::onFlushTimer()
{
    flush();
}

bool FileLogger::openLogFile()
{
    closeLogFile();
    
    m_logFile = new QFile(m_filePath);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        emit errorOccurred(QString("Failed to open log file: %1").arg(m_filePath));
        delete m_logFile;
        m_logFile = nullptr;
        return false;
    }
    
    m_stream = new QTextStream(m_logFile);
    // 在Qt 6中，QTextStream不再支持setCodec
    // 使用默认编码
    
    return true;
}

void FileLogger::closeLogFile()
{
    if (m_stream) {
        delete m_stream;
        m_stream = nullptr;
    }
    
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
}

bool FileLogger::needsRotation() const
{
    if (!m_rotationEnabled || m_maxFileSize <= 0) {
        return false;
    }
    
    return currentFileSize() >= m_maxFileSize;
}

void FileLogger::performRotation()
{
    if (!m_logFile) {
        return;
    }
    
    // 关闭当前文件
    closeLogFile();
    
    // 清理旧的备份文件
    cleanupOldBackups();
    
    // 轮转文件
    for (int i = m_maxBackupFiles - 1; i >= 1; --i) {
        QString oldName = generateBackupFileName(i);
        QString newName = generateBackupFileName(i + 1);
        
        if (QFile::exists(oldName)) {
            QFile::remove(newName);
            QFile::rename(oldName, newName);
        }
    }
    
    // 将当前文件重命名为第一个备份文件
    QString backupName = generateBackupFileName(1);
    if (QFile::exists(m_filePath)) {
        QFile::remove(backupName);
        QFile::rename(m_filePath, backupName);
    }
    
    // 重新打开日志文件
    openLogFile();
}

void FileLogger::cleanupOldBackups()
{
    for (int i = m_maxBackupFiles + 1; i <= m_maxBackupFiles + 10; ++i) {
        QString backupName = generateBackupFileName(i);
        if (QFile::exists(backupName)) {
            QFile::remove(backupName);
        }
    }
}

QString FileLogger::generateBackupFileName(int index) const
{
    QFileInfo fileInfo(m_filePath);
    QString baseName = fileInfo.completeBaseName();
    QString suffix = fileInfo.suffix();
    QString dir = fileInfo.absolutePath();
    
    if (suffix.isEmpty()) {
        return QString("%1/%2.%3").arg(dir).arg(baseName).arg(index);
    } else {
        return QString("%1/%2.%3.%4").arg(dir).arg(baseName).arg(index).arg(suffix);
    }
}

void FileLogger::writeToFile(const LogEntry& entry)
{
    if (!m_stream || !m_enabled) {
        return;
    }
    
    QString formattedEntry = formatEntry(entry, m_format);
    *m_stream << formattedEntry << Qt::endl;
}