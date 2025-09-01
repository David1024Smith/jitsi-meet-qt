#include "../include/Logger.h"
#include "../interfaces/ILogger.h"
#include "../logging/FileLogger.h"
#include "../logging/ConsoleLogger.h"
#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QDir>

// 静态成员初始化
Logger* Logger::s_instance = nullptr;
QMutex Logger::s_mutex;

Logger* Logger::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new Logger();
    }
    return s_instance;
}

Logger::Logger(QObject* parent)
    : QObject(parent)
    , m_globalLogLevel(Info)
    , m_logFormat("{timestamp} [{level}] {category}: {message}")
{
    // 构造函数中不创建默认日志记录器，等待initialize()调用
}

Logger::~Logger()
{
    cleanup();
}

bool Logger::initialize()
{
    QMutexLocker locker(&m_logMutex);
    
    // 清理现有的日志记录器
    m_loggers.clear();
    
    // 创建默认日志记录器
    createDefaultLoggers();
    
    return !m_loggers.isEmpty();
}

void Logger::cleanup()
{
    QMutexLocker locker(&m_logMutex);
    
    // 清理所有日志记录器
    for (auto& logger : m_loggers) {
        if (logger) {
            logger->cleanup();
        }
    }
    m_loggers.clear();
}

void Logger::createDefaultLoggers()
{
    // 创建控制台日志记录器
    auto consoleLogger = std::make_shared<ConsoleLogger>();
    consoleLogger->initialize();
    consoleLogger->setLogLevel(static_cast<ILogger::LogLevel>(m_globalLogLevel));
    consoleLogger->setFormat(m_logFormat);
    m_loggers.append(consoleLogger);
    
    // 创建文件日志记录器（如果可能）
    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir().mkpath(logDir);
    QString logFile = logDir + "/jitsi-meet-qt.log";
    
    auto fileLogger = std::make_shared<FileLogger>(logFile);
    if (fileLogger->initialize()) {
        fileLogger->setLogLevel(static_cast<ILogger::LogLevel>(m_globalLogLevel));
        fileLogger->setFormat(m_logFormat);
        fileLogger->setMaxFileSize(10 * 1024 * 1024); // 10MB
        fileLogger->setMaxBackupFiles(5);
        fileLogger->setRotationEnabled(true);
        m_loggers.append(fileLogger);
    }
}

void Logger::addLogger(std::shared_ptr<ILogger> logger)
{
    if (!logger) {
        return;
    }
    
    QMutexLocker locker(&m_logMutex);
    
    // 设置日志记录器的级别和格式
    logger->setLogLevel(static_cast<ILogger::LogLevel>(m_globalLogLevel));
    logger->setFormat(m_logFormat);
    
    // 连接信号 - 使用静态类型转换解决接口类信号槽连接问题
    // 修复信号槽连接问题，使用旧式连接语法
    connect(logger.get(), SIGNAL(logRecorded(const ILogger::LogEntry&)),
            this, SLOT(logRecorded(const Logger::LogEntry&)));
    
    m_loggers.append(logger);
}

void Logger::removeLogger(std::shared_ptr<ILogger> logger)
{
    if (!logger) {
        return;
    }
    
    QMutexLocker locker(&m_logMutex);
    
    // 断开信号连接
    disconnect(logger.get(), nullptr, this, nullptr);
    
    // 从列表中移除
    m_loggers.removeAll(logger);
}

void Logger::setGlobalLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_logMutex);
    
    m_globalLogLevel = level;
    
    // 更新所有日志记录器的级别
    for (auto& logger : m_loggers) {
        if (logger) {
            logger->setLogLevel(static_cast<ILogger::LogLevel>(level));
        }
    }
    
    emit logLevelChanged(level);
}

Logger::LogLevel Logger::globalLogLevel() const
{
    QMutexLocker locker(&m_logMutex);
    return m_globalLogLevel;
}

void Logger::setLogFormat(const QString& format)
{
    QMutexLocker locker(&m_logMutex);
    
    m_logFormat = format;
    
    // 更新所有日志记录器的格式
    for (auto& logger : m_loggers) {
        if (logger) {
            logger->setFormat(format);
        }
    }
}

QString Logger::logFormat() const
{
    QMutexLocker locker(&m_logMutex);
    return m_logFormat;
}

void Logger::log(LogLevel level, const QString& category, const QString& message, 
                const QString& file, int line)
{
    if (!shouldLog(level)) {
        return;
    }
    
    // 创建日志条目
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;
    entry.category = category;
    entry.message = message;
    entry.thread = QString("0x%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 16);
    entry.file = file;
    entry.line = line;
    
    QMutexLocker locker(&m_logMutex);
    
    // 发送到所有日志记录器
    for (auto& logger : m_loggers) {
        if (logger && logger->isEnabled()) {
            ILogger::LogEntry iEntry;
            iEntry.timestamp = entry.timestamp;
            iEntry.level = static_cast<ILogger::LogLevel>(entry.level);
            iEntry.category = entry.category;
            iEntry.message = entry.message;
            iEntry.thread = entry.thread;
            iEntry.file = entry.file;
            iEntry.line = entry.line;
            
            logger->log(iEntry);
        }
    }
    
    // 发出信号
    emit logRecorded(entry);
}

QString Logger::formatLogEntry(const LogEntry& entry) const
{
    QString result = m_logFormat;
    
    // 替换格式占位符
    result.replace("{timestamp}", entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz"));
    result.replace("{level}", levelToString(entry.level));
    result.replace("{category}", entry.category);
    result.replace("{message}", entry.message);
    result.replace("{thread}", entry.thread);
    result.replace("{file}", entry.file);
    result.replace("{line}", QString::number(entry.line));
    
    return result;
}

QString Logger::levelToString(LogLevel level)
{
    switch (level) {
        case Debug: return "DEBUG";
        case Info: return "INFO";
        case Warning: return "WARNING";
        case Error: return "ERROR";
        case Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

Logger::LogLevel Logger::stringToLevel(const QString& levelStr)
{
    QString upper = levelStr.toUpper();
    if (upper == "DEBUG") return Debug;
    if (upper == "INFO") return Info;
    if (upper == "WARNING") return Warning;
    if (upper == "ERROR") return Error;
    if (upper == "CRITICAL") return Critical;
    return Info; // 默认级别
}

bool Logger::shouldLog(LogLevel level) const
{
    return level >= m_globalLogLevel;
}

// 静态便捷方法
void Logger::debug(const QString& message, const QString& category)
{
    instance()->log(Debug, category, message);
}

void Logger::info(const QString& message, const QString& category)
{
    instance()->log(Info, category, message);
}

void Logger::warning(const QString& message, const QString& category)
{
    instance()->log(Warning, category, message);
}

void Logger::error(const QString& message, const QString& category)
{
    instance()->log(Error, category, message);
}

void Logger::critical(const QString& message, const QString& category)
{
    instance()->log(Critical, category, message);
}