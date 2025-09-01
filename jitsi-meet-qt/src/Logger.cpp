#include "../include/Logger.h"
#include <QDebug>
#include <QDateTime>
#include <QDir>

// Static member initialization
Logger* Logger::s_instance = nullptr;

Logger::Logger(QObject* parent)
    : QObject(parent)
    , m_logFile(nullptr)
    , m_stream(nullptr)
    , m_logLevel(LogLevel::Debug)
{
}

Logger::~Logger()
{
    cleanup();
}

Logger* Logger::instance()
{
    if (!s_instance) {
        s_instance = new Logger();
    }
    return s_instance;
}

void Logger::setLogFile(const QString& filePath)
{
    if (m_logFile) {
        m_logFile->close();
        delete m_stream;
        delete m_logFile;
    }
    
    m_logFile = new QFile(filePath);
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_stream = new QTextStream(m_logFile);
    }
}

void Logger::setLogLevel(LogLevel level)
{
    m_logLevel = level;
}

void Logger::log(LogLevel level, const QString& message)
{
    log(level, "General", message);
}

void Logger::log(LogLevel level, const QString& category, const QString& message)
{
    if (level < m_logLevel) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logMessage = QString("[%1] [%2] [%3] %4")
                         .arg(timestamp)
                         .arg(levelToString(level))
                         .arg(category)
                         .arg(message);
    
    // Output to console
    qDebug() << logMessage;
    
    // Output to file if available
    if (m_stream) {
        *m_stream << logMessage << Qt::endl;
        m_stream->flush();
    }
}

void Logger::debug(const QString& message)
{
    log(LogLevel::Debug, message);
}

void Logger::info(const QString& message)
{
    log(LogLevel::Info, message);
}

void Logger::warning(const QString& message)
{
    log(LogLevel::Warning, message);
}

void Logger::error(const QString& message)
{
    log(LogLevel::Error, message);
}

void Logger::critical(const QString& message)
{
    log(LogLevel::Critical, message);
}

bool Logger::initialize()
{
    qDebug() << "Logger initialized";
    return true;
}

void Logger::cleanup()
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
    
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}