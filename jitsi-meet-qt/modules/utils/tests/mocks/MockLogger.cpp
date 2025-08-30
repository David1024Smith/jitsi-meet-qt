#include "MockLogger.h"
#include <QThread>
#include <QMutexLocker>

MockLogger::MockLogger(QObject* parent)
    : ILogger(parent)
    , m_logLevel(Info)
    , m_initialized(false)
    , m_initializeResult(true)
    , m_flushDelay(0)
    , m_throwOnLog(false)
{
}

bool MockLogger::initialize()
{
    QMutexLocker locker(&m_mutex);
    m_initialized = m_initializeResult;
    return m_initialized;
}

void MockLogger::cleanup()
{
    QMutexLocker locker(&m_mutex);
    m_initialized = false;
    m_logs.clear();
}

void MockLogger::log(const LogEntry& entry)
{
    if (m_throwOnLog) {
        throw std::runtime_error("Mock logger exception");
    }
    
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    if (entry.level >= m_logLevel) {
        m_logs.append(entry);
    }
}

void MockLogger::flush()
{
    if (m_flushDelay > 0) {
        QThread::msleep(m_flushDelay);
    }
    
    // 模拟flush操作，实际上不需要做什么
}

void MockLogger::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

ILogger::LogLevel MockLogger::logLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_logLevel;
}

QString MockLogger::name() const
{
    return "Mock Logger";
}

QString MockLogger::version() const
{
    return "1.0.0";
}

void MockLogger::clearLogs()
{
    QMutexLocker locker(&m_mutex);
    m_logs.clear();
}

QList<ILogger::LogEntry> MockLogger::getLogs() const
{
    QMutexLocker locker(&m_mutex);
    return m_logs;
}

QList<ILogger::LogEntry> MockLogger::getLogsForLevel(LogLevel level) const
{
    QMutexLocker locker(&m_mutex);
    QList<LogEntry> result;
    
    for (const LogEntry& entry : m_logs) {
        if (entry.level == level) {
            result.append(entry);
        }
    }
    
    return result;
}

QList<ILogger::LogEntry> MockLogger::getLogsForCategory(const QString& category) const
{
    QMutexLocker locker(&m_mutex);
    QList<LogEntry> result;
    
    for (const LogEntry& entry : m_logs) {
        if (entry.category == category) {
            result.append(entry);
        }
    }
    
    return result;
}

int MockLogger::getLogCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_logs.size();
}

int MockLogger::getLogCountForLevel(LogLevel level) const
{
    QMutexLocker locker(&m_mutex);
    int count = 0;
    
    for (const LogEntry& entry : m_logs) {
        if (entry.level == level) {
            count++;
        }
    }
    
    return count;
}

bool MockLogger::hasLogWithMessage(const QString& message) const
{
    QMutexLocker locker(&m_mutex);
    
    for (const LogEntry& entry : m_logs) {
        if (entry.message.contains(message)) {
            return true;
        }
    }
    
    return false;
}

ILogger::LogEntry MockLogger::getLastLog() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_logs.isEmpty()) {
        return LogEntry();
    }
    
    return m_logs.last();
}

void MockLogger::setInitializeResult(bool result)
{
    QMutexLocker locker(&m_mutex);
    m_initializeResult = result;
}

void MockLogger::setFlushDelay(int milliseconds)
{
    QMutexLocker locker(&m_mutex);
    m_flushDelay = milliseconds;
}

void MockLogger::setThrowOnLog(bool throwException)
{
    QMutexLocker locker(&m_mutex);
    m_throwOnLog = throwException;
}