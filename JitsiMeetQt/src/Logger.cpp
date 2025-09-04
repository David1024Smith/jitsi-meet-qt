#include "Logger.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QStringConverter>
#include <iostream>

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::~Logger()
{
    shutdown();
}

void Logger::initialize(bool enableFileLogging, const QString& logFileName)
{
    QMutexLocker locker(&m_mutex);
    
#ifdef DEBUG
    // Debug版本启用日志记录
    m_loggingEnabled = true;
    m_fileLoggingEnabled = enableFileLogging;
    m_minLogLevel = Debug;
#else
    // Release版本禁用所有日志记录
    m_loggingEnabled = false;
    m_fileLoggingEnabled = false;
    m_minLogLevel = Critical;
    return; // Release版本直接返回，不进行任何日志初始化
#endif

    if (m_fileLoggingEnabled) {
        // 获取应用程序所在目录
        QString appDir = QCoreApplication::applicationDirPath();
        m_logFileName = QDir(appDir).absoluteFilePath(logFileName);
        
        // 关闭之前可能打开的文件
        if (m_logFile.isOpen()) {
            m_logStream.setDevice(nullptr);
            m_logFile.close();
        }
        
        // 打开日志文件
        m_logFile.setFileName(m_logFileName);
        if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            m_logStream.setDevice(&m_logFile);
            m_logStream.setEncoding(QStringConverter::Utf8);
            
            // 写入日志开始标记
            QString startMessage = QString("\n=== 日志会话开始 [%1] ===")
                                 .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            m_logStream << startMessage << Qt::endl;
            m_logStream.flush();
        } else {
            m_fileLoggingEnabled = false;
            qWarning() << "无法打开日志文件:" << m_logFileName;
        }
    }
    
    // 输出初始化信息
    info(QString("日志系统初始化完成 - 文件日志: %1, 日志文件: %2")
         .arg(m_fileLoggingEnabled ? "启用" : "禁用")
         .arg(m_fileLoggingEnabled ? m_logFileName : "无"), "Logger");
}

void Logger::setMinLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_minLogLevel = level;
}

void Logger::debug(const QString& message, const QString& category)
{
    log(Debug, message, category);
}

void Logger::info(const QString& message, const QString& category)
{
    log(Info, message, category);
}

void Logger::warning(const QString& message, const QString& category)
{
    log(Warning, message, category);
}

void Logger::error(const QString& message, const QString& category)
{
    log(Error, message, category);
}

void Logger::critical(const QString& message, const QString& category)
{
    log(Critical, message, category);
}

void Logger::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_fileLoggingEnabled && m_logFile.isOpen()) {
        // 写入日志结束标记
        QString endMessage = QString("=== 日志会话结束 [%1] ===\n")
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        m_logStream << endMessage << Qt::endl;
        m_logStream.flush();
        
        m_logStream.setDevice(nullptr);
        m_logFile.close();
    }
    
    m_loggingEnabled = false;
    m_fileLoggingEnabled = false;
}

void Logger::log(LogLevel level, const QString& message, const QString& category)
{
    // Release版本直接返回，不输出任何日志
#ifndef _DEBUG
    Q_UNUSED(level)
    Q_UNUSED(message)
    Q_UNUSED(category)
    return;
#endif

    QMutexLocker locker(&m_mutex);
    
    // 检查是否启用日志记录
    if (!m_loggingEnabled) {
        return;
    }
    
    // 检查日志级别
    if (level < m_minLogLevel) {
        return;
    }
    
    // 格式化日志消息
    QString formattedMessage = formatMessage(level, message, category);
    
    // 输出到控制台（Debug版本）
    std::cout << formattedMessage.toUtf8().constData() << std::endl;
    
    // 输出到文件（如果启用）
    if (m_fileLoggingEnabled && m_logFile.isOpen()) {
        m_logStream << formattedMessage << Qt::endl;
        m_logStream.flush();
    }
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
        case Debug:    return "[调试]";
        case Info:     return "[信息]";
        case Warning:  return "[警告]";
        case Error:    return "[错误]";
        case Critical: return "[严重]";
        default:       return "[未知]";
    }
}

QString Logger::formatMessage(LogLevel level, const QString& message, const QString& category) const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = levelToString(level);
    
    if (category.isEmpty()) {
        return QString("%1 %2 %3").arg(timestamp, levelStr, message);
    } else {
        return QString("%1 %2 [%3] %4").arg(timestamp, levelStr, category, message);
    }
}