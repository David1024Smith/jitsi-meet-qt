#include "ErrorLogger.h"
#include "ErrorEventBus.h"
#include <QTimer>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(errorLogger, "jitsi.errorlogger")

// 静态成员初始化
ErrorLogger* ErrorLogger::s_instance = nullptr;

ErrorLogger::ErrorLogger(QObject *parent)
    : QObject(parent)
    , m_logErrorContext(true)
    , m_logStackTrace(false)
    , m_statisticsLoggingEnabled(true)
    , m_statisticsLogInterval(DEFAULT_STATISTICS_INTERVAL)
    , m_statisticsTimer(new QTimer(this))
    , m_initialized(false)
{
    // 设置默认的错误级别映射
    m_levelMappings[ModuleError::Info] = ILogger::Info;
    m_levelMappings[ModuleError::Warning] = ILogger::Warning;
    m_levelMappings[ModuleError::Error] = ILogger::Error;
    m_levelMappings[ModuleError::Critical] = ILogger::Critical;
    m_levelMappings[ModuleError::Fatal] = ILogger::Fatal;

    // 设置统计定时器
    m_statisticsTimer->setSingleShot(false);
    m_statisticsTimer->setInterval(m_statisticsLogInterval);
    connect(m_statisticsTimer, &QTimer::timeout, this, &ErrorLogger::logErrorStatistics);
}

ErrorLogger::~ErrorLogger()
{
    shutdown();
}

ErrorLogger* ErrorLogger::instance()
{
    if (!s_instance) {
        s_instance = new ErrorLogger();
    }
    return s_instance;
}

bool ErrorLogger::initialize()
{
    if (m_initialized) {
        return true;
    }

    qCDebug(errorLogger) << "ErrorLogger: Initializing error logger integration...";

    // 连接到错误事件总线
    ErrorEventBus* errorBus = ErrorEventBus::instance();
    if (errorBus) {
        connect(errorBus, &ErrorEventBus::errorReported,
                this, &ErrorLogger::onErrorReported);
        connect(errorBus, &ErrorEventBus::moduleErrorReported,
                this, &ErrorLogger::onModuleErrorReported);
        connect(errorBus, &ErrorEventBus::errorRecoveryStarted,
                this, &ErrorLogger::onErrorRecoveryStarted);
        connect(errorBus, &ErrorEventBus::errorRecoveryCompleted,
                this, &ErrorLogger::onErrorRecoveryCompleted);
    }

    // 启动统计日志定时器
    if (m_statisticsLoggingEnabled) {
        m_statisticsTimer->start();
    }

    m_initialized = true;

    qCDebug(errorLogger) << "ErrorLogger: Error logger integration initialized successfully";
    Logger::instance()->log(ILogger::Info, "Error logger integration initialized", "ErrorLogger");

    return true;
}

void ErrorLogger::shutdown()
{
    if (!m_initialized) {
        return;
    }

    qCDebug(errorLogger) << "ErrorLogger: Shutting down error logger integration...";

    // 停止统计定时器
    m_statisticsTimer->stop();

    // 断开连接
    ErrorEventBus* errorBus = ErrorEventBus::instance();
    if (errorBus) {
        disconnect(errorBus, nullptr, this, nullptr);
    }

    m_initialized = false;

    qCDebug(errorLogger) << "ErrorLogger: Error logger integration shut down";
}

void ErrorLogger::logError(const ModuleError& error)
{
    if (!m_initialized) {
        return;
    }

    ILogger::LogEntry entry = createErrorLogEntry(error);
    Logger::instance()->log(entry);
}

void ErrorLogger::setErrorLevelMapping(ModuleError::Severity errorSeverity, ILogger::LogLevel logLevel)
{
    m_levelMappings[errorSeverity] = logLevel;
    
    qCDebug(errorLogger) << "ErrorLogger: Error level mapping set:"
                        << ModuleError::severityName(errorSeverity) << "->" << ILogger::levelName(logLevel);
}

ILogger::LogLevel ErrorLogger::getErrorLevelMapping(ModuleError::Severity errorSeverity) const
{
    return m_levelMappings.value(errorSeverity, ILogger::Error);
}

void ErrorLogger::setLogErrorContext(bool enabled)
{
    m_logErrorContext = enabled;
    qCDebug(errorLogger) << "ErrorLogger: Error context logging" << (enabled ? "enabled" : "disabled");
}

bool ErrorLogger::isLogErrorContext() const
{
    return m_logErrorContext;
}

void ErrorLogger::setLogStackTrace(bool enabled)
{
    m_logStackTrace = enabled;
    qCDebug(errorLogger) << "ErrorLogger: Stack trace logging" << (enabled ? "enabled" : "disabled");
}

bool ErrorLogger::isLogStackTrace() const
{
    return m_logStackTrace;
}

void ErrorLogger::setStatisticsLogInterval(int interval)
{
    m_statisticsLogInterval = qMax(60000, interval); // 最小1分钟
    m_statisticsTimer->setInterval(m_statisticsLogInterval);
    
    qCDebug(errorLogger) << "ErrorLogger: Statistics log interval set to" << m_statisticsLogInterval << "ms";
}

int ErrorLogger::statisticsLogInterval() const
{
    return m_statisticsLogInterval;
}

void ErrorLogger::setStatisticsLoggingEnabled(bool enabled)
{
    m_statisticsLoggingEnabled = enabled;
    
    if (enabled && m_initialized) {
        m_statisticsTimer->start();
    } else {
        m_statisticsTimer->stop();
    }
    
    qCDebug(errorLogger) << "ErrorLogger: Statistics logging" << (enabled ? "enabled" : "disabled");
}

bool ErrorLogger::isStatisticsLoggingEnabled() const
{
    return m_statisticsLoggingEnabled;
}

void ErrorLogger::onErrorReported(const ModuleError& error)
{
    logError(error);
}

void ErrorLogger::onModuleErrorReported(const QString& moduleName, const ModuleError& error)
{
    // 模块特定的错误处理
    logError(error);
    
    // 记录模块错误统计
    if (error.severity() >= ModuleError::Error) {
        QString message = QString("Module '%1' reported %2 error: %3")
                         .arg(moduleName)
                         .arg(ModuleError::severityName(error.severity()).toLower())
                         .arg(error.message());
        
        Logger::instance()->log(ILogger::Warning, message, "ModuleErrorTracker");
    }
}

void ErrorLogger::onErrorRecoveryStarted(const ModuleError& error, const QString& strategy)
{
    QString message = QString("Starting error recovery for module '%1' using strategy '%2': %3")
                     .arg(error.moduleName())
                     .arg(strategy)
                     .arg(error.message());
    
    Logger::instance()->log(ILogger::Info, message, "ErrorRecovery");
}

void ErrorLogger::onErrorRecoveryCompleted(const ModuleError& error, const QString& strategy, bool success)
{
    QString message = QString("Error recovery %1 for module '%2' using strategy '%3': %4")
                     .arg(success ? "succeeded" : "failed")
                     .arg(error.moduleName())
                     .arg(strategy)
                     .arg(error.message());
    
    ILogger::LogLevel level = success ? ILogger::Info : ILogger::Warning;
    Logger::instance()->log(level, message, "ErrorRecovery");
}

void ErrorLogger::logErrorStatistics()
{
    ErrorEventBus* errorBus = ErrorEventBus::instance();
    if (!errorBus) {
        return;
    }

    ErrorEventBus::ErrorStatistics stats = errorBus->getStatistics();
    
    if (stats.totalErrors == 0) {
        return; // 没有错误，不记录统计
    }

    QString message = QString("Error Statistics - Total: %1, Rate: %2/min, Last: %3")
                     .arg(stats.totalErrors)
                     .arg(stats.errorRate, 0, 'f', 2)
                     .arg(stats.lastError.toString("yyyy-MM-dd hh:mm:ss"));

    Logger::instance()->log(ILogger::Info, message, "ErrorStatistics");

    // 记录按类型分组的错误统计
    if (!stats.errorsByType.isEmpty()) {
        QStringList typeStats;
        for (auto it = stats.errorsByType.begin(); it != stats.errorsByType.end(); ++it) {
            typeStats << QString("%1: %2").arg(ModuleError::errorTypeName(it.key())).arg(it.value());
        }
        
        QString typeMessage = QString("Error Types - %1").arg(typeStats.join(", "));
        Logger::instance()->log(ILogger::Debug, typeMessage, "ErrorStatistics");
    }

    // 记录按严重程度分组的错误统计
    if (!stats.errorsBySeverity.isEmpty()) {
        QStringList severityStats;
        for (auto it = stats.errorsBySeverity.begin(); it != stats.errorsBySeverity.end(); ++it) {
            severityStats << QString("%1: %2").arg(ModuleError::severityName(it.key())).arg(it.value());
        }
        
        QString severityMessage = QString("Error Severities - %1").arg(severityStats.join(", "));
        Logger::instance()->log(ILogger::Debug, severityMessage, "ErrorStatistics");
    }

    // 记录按模块分组的错误统计
    if (!stats.errorsByModule.isEmpty()) {
        QStringList moduleStats;
        for (auto it = stats.errorsByModule.begin(); it != stats.errorsByModule.end(); ++it) {
            if (it.value() > 0) {
                moduleStats << QString("%1: %2").arg(it.key()).arg(it.value());
            }
        }
        
        if (!moduleStats.isEmpty()) {
            QString moduleMessage = QString("Error Modules - %1").arg(moduleStats.join(", "));
            Logger::instance()->log(ILogger::Debug, moduleMessage, "ErrorStatistics");
        }
    }
}

ILogger::LogLevel ErrorLogger::mapErrorSeverityToLogLevel(ModuleError::Severity severity) const
{
    return m_levelMappings.value(severity, ILogger::Error);
}

QString ErrorLogger::formatErrorMessage(const ModuleError& error) const
{
    QString message = QString("[%1] [%2] %3")
                     .arg(ModuleError::errorTypeName(error.type()))
                     .arg(ModuleError::severityName(error.severity()))
                     .arg(error.message());

    if (error.errorCode() != 0) {
        message += QString(" (Code: %1)").arg(error.errorCode());
    }

    if (!error.details().isEmpty()) {
        message += QString(" - %1").arg(error.details());
    }

    return message;
}

ILogger::LogEntry ErrorLogger::createErrorLogEntry(const ModuleError& error) const
{
    ILogger::LogEntry entry;
    entry.timestamp = error.timestamp();
    entry.level = mapErrorSeverityToLogLevel(error.severity());
    entry.category = "ModuleError";
    entry.message = formatErrorMessage(error);
    entry.moduleName = error.moduleName();

    // 添加错误上下文信息
    if (m_logErrorContext && !error.context().isEmpty()) {
        QStringList contextItems;
        for (auto it = error.context().begin(); it != error.context().end(); ++it) {
            contextItems << QString("%1=%2").arg(it.key()).arg(it.value().toString());
        }
        entry.context["errorContext"] = contextItems.join(", ");
    }

    // 添加错误类型和严重程度
    entry.context["errorType"] = ModuleError::errorTypeName(error.type());
    entry.context["errorSeverity"] = ModuleError::severityName(error.severity());
    entry.context["errorCode"] = error.errorCode();

    // 添加堆栈跟踪
    if (m_logStackTrace && !error.stackTrace().isEmpty()) {
        entry.context["stackTrace"] = error.stackTrace().join("\n");
    }

    return entry;
}

// ModuleLogHelper实现
ModuleLogHelper::ModuleLogHelper(const QString& moduleName)
    : m_moduleName(moduleName)
{
}

void ModuleLogHelper::log(ILogger::LogLevel level, const QString& message, const QString& category) const
{
    ILogger::LogEntry entry = Logger::createEntry(level, message, category, m_moduleName);
    Logger::instance()->log(entry);
}

void ModuleLogHelper::logError(const ModuleError& error) const
{
    ErrorLogger::instance()->logError(error);
}

ModuleError ModuleLogHelper::createAndLogError(ModuleError::ErrorType type, 
                                              ModuleError::Severity severity, 
                                              const QString& message) const
{
    ModuleError error(type, severity, message, m_moduleName);
    
    // 报告到错误事件总线
    ErrorEventBus::instance()->reportError(error);
    
    return error;
}

QString ModuleLogHelper::moduleName() const
{
    return m_moduleName;
}

void ModuleLogHelper::trace(const QString& message, const QString& category) const
{
    log(ILogger::Trace, message, category);
}

void ModuleLogHelper::debug(const QString& message, const QString& category) const
{
    log(ILogger::Debug, message, category);
}

void ModuleLogHelper::info(const QString& message, const QString& category) const
{
    log(ILogger::Info, message, category);
}

void ModuleLogHelper::warning(const QString& message, const QString& category) const
{
    log(ILogger::Warning, message, category);
}

void ModuleLogHelper::error(const QString& message, const QString& category) const
{
    log(ILogger::Error, message, category);
}

void ModuleLogHelper::critical(const QString& message, const QString& category) const
{
    log(ILogger::Critical, message, category);
}

void ModuleLogHelper::fatal(const QString& message, const QString& category) const
{
    log(ILogger::Fatal, message, category);
}