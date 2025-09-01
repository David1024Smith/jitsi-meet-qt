#include "ConsoleLogger.h"
#include <QCoreApplication>
#include <QThread>
#include <iostream>

// ANSI颜色代码常量
const QString ConsoleLogger::COLOR_RESET = "\033[0m";
const QString ConsoleLogger::COLOR_BLACK = "\033[30m";
const QString ConsoleLogger::COLOR_RED = "\033[31m";
const QString ConsoleLogger::COLOR_GREEN = "\033[32m";
const QString ConsoleLogger::COLOR_YELLOW = "\033[33m";
const QString ConsoleLogger::COLOR_BLUE = "\033[34m";
const QString ConsoleLogger::COLOR_MAGENTA = "\033[35m";
const QString ConsoleLogger::COLOR_CYAN = "\033[36m";
const QString ConsoleLogger::COLOR_WHITE = "\033[37m";
const QString ConsoleLogger::COLOR_BRIGHT_BLACK = "\033[90m";
const QString ConsoleLogger::COLOR_BRIGHT_RED = "\033[91m";
const QString ConsoleLogger::COLOR_BRIGHT_GREEN = "\033[92m";
const QString ConsoleLogger::COLOR_BRIGHT_YELLOW = "\033[93m";
const QString ConsoleLogger::COLOR_BRIGHT_BLUE = "\033[94m";
const QString ConsoleLogger::COLOR_BRIGHT_MAGENTA = "\033[95m";
const QString ConsoleLogger::COLOR_BRIGHT_CYAN = "\033[96m";
const QString ConsoleLogger::COLOR_BRIGHT_WHITE = "\033[97m";

ConsoleLogger::ConsoleLogger(QObject* parent)
    : ILogger(parent)
    , m_logLevel(Info)
    , m_format("{timestamp} [{level}] {category}: {message}")
    , m_enabled(true)
    , m_outputStream(Auto)
    , m_colorEnabled(supportsColor())
    , m_timestampEnabled(true)
    , m_threadIdEnabled(false)
    , m_stdout(new QTextStream(stdout))
    , m_stderr(new QTextStream(stderr))
{
    // 在Qt 6中，QTextStream不再支持setCodec
    // 使用标准输出和错误流的默认编码
    
    // 初始化默认颜色
    initializeDefaultColors();
}

ConsoleLogger::~ConsoleLogger()
{
    cleanup();
}

bool ConsoleLogger::initialize()
{
    // 控制台日志记录器不需要特殊初始化
    return true;
}

void ConsoleLogger::cleanup()
{
    QMutexLocker locker(&m_mutex);
    
    // 刷新输出流
    if (m_stdout) {
        m_stdout->flush();
        delete m_stdout;
        m_stdout = nullptr;
    }
    
    if (m_stderr) {
        m_stderr->flush();
        delete m_stderr;
        m_stderr = nullptr;
    }
}

void ConsoleLogger::log(const LogEntry& entry)
{
    if (!shouldLog(entry.level)) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    if (!m_enabled) {
        return;
    }
    
    // 选择输出流
    QTextStream* stream = selectOutputStream(entry.level);
    if (!stream) {
        return;
    }
    
    // 格式化日志条目
    QString formattedEntry;
    if (m_colorEnabled) {
        formattedEntry = formatColoredEntry(entry);
    } else {
        formattedEntry = formatEntry(entry, m_format);
    }
    
    // 输出到控制台
    *stream << formattedEntry << Qt::endl;
    stream->flush();
    
    // 发出信号
    emit logRecorded(entry);
}

void ConsoleLogger::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

ILogger::LogLevel ConsoleLogger::logLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_logLevel;
}

void ConsoleLogger::setFormat(const QString& format)
{
    QMutexLocker locker(&m_mutex);
    m_format = format;
}

QString ConsoleLogger::format() const
{
    QMutexLocker locker(&m_mutex);
    return m_format;
}

bool ConsoleLogger::isEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_enabled;
}

void ConsoleLogger::setEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_enabled = enabled;
}

QString ConsoleLogger::name() const
{
    return "ConsoleLogger";
}

void ConsoleLogger::flush()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_stdout) {
        m_stdout->flush();
    }
    if (m_stderr) {
        m_stderr->flush();
    }
}

void ConsoleLogger::setOutputStream(OutputStream stream)
{
    QMutexLocker locker(&m_mutex);
    m_outputStream = stream;
}

ConsoleLogger::OutputStream ConsoleLogger::outputStream() const
{
    QMutexLocker locker(&m_mutex);
    return m_outputStream;
}

void ConsoleLogger::setColorEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_colorEnabled = enabled && supportsColor();
}

bool ConsoleLogger::isColorEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_colorEnabled;
}

void ConsoleLogger::setTimestampEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_timestampEnabled = enabled;
}

bool ConsoleLogger::isTimestampEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_timestampEnabled;
}

void ConsoleLogger::setThreadIdEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_threadIdEnabled = enabled;
}

bool ConsoleLogger::isThreadIdEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_threadIdEnabled;
}

void ConsoleLogger::setLevelColor(LogLevel level, const QString& color)
{
    QMutexLocker locker(&m_mutex);
    m_levelColors[level] = color;
}

QString ConsoleLogger::levelColor(LogLevel level) const
{
    QMutexLocker locker(&m_mutex);
    return m_levelColors.value(level, COLOR_RESET);
}

bool ConsoleLogger::supportsColor()
{
#ifdef Q_OS_WIN
    // Windows控制台颜色支持检测
    return true; // 简化处理，假设支持
#else
    // Unix/Linux系统检测TERM环境变量
    const char* term = getenv("TERM");
    if (!term) {
        return false;
    }
    
    QString termStr = QString::fromLatin1(term);
    return termStr.contains("color") || termStr.contains("xterm") || 
           termStr.contains("screen") || termStr.contains("tmux");
#endif
}

QString ConsoleLogger::formatColoredEntry(const LogEntry& entry) const
{
    QString result = m_format;
    
    // 获取颜色代码
    QString colorCode = getColorCode(entry.level);
    
    // 替换格式占位符
    if (m_timestampEnabled) {
        result.replace("{timestamp}", entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz"));
    } else {
        result.replace("{timestamp}", "");
    }
    
    // 为日志级别添加颜色
    QString levelStr = levelToString(entry.level);
    if (m_colorEnabled && !colorCode.isEmpty()) {
        levelStr = colorCode + levelStr + COLOR_RESET;
    }
    result.replace("{level}", levelStr);
    
    result.replace("{category}", entry.category);
    result.replace("{message}", entry.message);
    
    if (m_threadIdEnabled) {
        result.replace("{thread}", entry.thread);
    } else {
        result.replace("{thread}", "");
    }
    
    result.replace("{file}", entry.file);
    result.replace("{line}", QString::number(entry.line));
    
    // 清理多余的空格
    result = result.simplified();
    
    return result;
}

QString ConsoleLogger::getColorCode(LogLevel level) const
{
    return m_levelColors.value(level, COLOR_RESET);
}

QTextStream* ConsoleLogger::selectOutputStream(LogLevel level) const
{
    switch (m_outputStream) {
        case StandardOutput:
            return m_stdout;
        case StandardError:
            return m_stderr;
        case Auto:
            // 错误和严重错误使用stderr，其他使用stdout
            if (level >= Error) {
                return m_stderr;
            } else {
                return m_stdout;
            }
        default:
            return m_stdout;
    }
}

void ConsoleLogger::initializeDefaultColors()
{
    m_levelColors[Debug] = COLOR_BRIGHT_BLACK;
    m_levelColors[Info] = COLOR_WHITE;
    m_levelColors[Warning] = COLOR_YELLOW;
    m_levelColors[Error] = COLOR_RED;
    m_levelColors[Critical] = COLOR_BRIGHT_RED;
}