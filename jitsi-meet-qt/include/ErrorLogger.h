#ifndef ERRORLOGGER_H
#define ERRORLOGGER_H

#include <QObject>
#include "ModuleError.h"
#include "Logger.h"

/**
 * @brief 错误日志集成器 - 连接错误处理系统和日志系统
 * 
 * 该类负责将模块错误自动记录到日志系统中，提供：
 * - 错误到日志的自动转换
 * - 错误级别映射
 * - 结构化错误日志记录
 * - 错误统计和监控日志
 */
class ErrorLogger : public QObject
{
    Q_OBJECT

public:
    explicit ErrorLogger(QObject *parent = nullptr);
    ~ErrorLogger();

    /**
     * @brief 获取单例实例
     */
    static ErrorLogger* instance();

    /**
     * @brief 初始化错误日志集成
     */
    bool initialize();

    /**
     * @brief 关闭错误日志集成
     */
    void shutdown();

    /**
     * @brief 记录模块错误到日志
     * @param error 模块错误
     */
    void logError(const ModuleError& error);

    /**
     * @brief 设置错误日志级别映射
     * @param errorSeverity 错误严重程度
     * @param logLevel 对应的日志级别
     */
    void setErrorLevelMapping(ModuleError::Severity errorSeverity, ILogger::LogLevel logLevel);

    /**
     * @brief 获取错误日志级别映射
     * @param errorSeverity 错误严重程度
     * @return 对应的日志级别
     */
    ILogger::LogLevel getErrorLevelMapping(ModuleError::Severity errorSeverity) const;

    /**
     * @brief 设置是否记录错误上下文
     * @param enabled 是否启用
     */
    void setLogErrorContext(bool enabled);

    /**
     * @brief 检查是否记录错误上下文
     * @return 是否启用
     */
    bool isLogErrorContext() const;

    /**
     * @brief 设置是否记录堆栈跟踪
     * @param enabled 是否启用
     */
    void setLogStackTrace(bool enabled);

    /**
     * @brief 检查是否记录堆栈跟踪
     * @return 是否启用
     */
    bool isLogStackTrace() const;

    /**
     * @brief 设置错误统计日志间隔
     * @param interval 间隔时间(毫秒)
     */
    void setStatisticsLogInterval(int interval);

    /**
     * @brief 获取错误统计日志间隔
     * @return 间隔时间(毫秒)
     */
    int statisticsLogInterval() const;

    /**
     * @brief 启用错误统计日志
     * @param enabled 是否启用
     */
    void setStatisticsLoggingEnabled(bool enabled);

    /**
     * @brief 检查是否启用错误统计日志
     * @return 是否启用
     */
    bool isStatisticsLoggingEnabled() const;

public slots:
    /**
     * @brief 处理错误报告
     * @param error 模块错误
     */
    void onErrorReported(const ModuleError& error);

    /**
     * @brief 处理模块错误报告
     * @param moduleName 模块名称
     * @param error 模块错误
     */
    void onModuleErrorReported(const QString& moduleName, const ModuleError& error);

    /**
     * @brief 处理错误恢复开始
     * @param error 模块错误
     * @param strategy 恢复策略名称
     */
    void onErrorRecoveryStarted(const ModuleError& error, const QString& strategy);

    /**
     * @brief 处理错误恢复完成
     * @param error 模块错误
     * @param strategy 恢复策略名称
     * @param success 是否成功
     */
    void onErrorRecoveryCompleted(const ModuleError& error, const QString& strategy, bool success);

private slots:
    /**
     * @brief 记录错误统计信息
     */
    void logErrorStatistics();

private:
    /**
     * @brief 映射错误严重程度到日志级别
     * @param severity 错误严重程度
     * @return 日志级别
     */
    ILogger::LogLevel mapErrorSeverityToLogLevel(ModuleError::Severity severity) const;

    /**
     * @brief 格式化错误消息
     * @param error 模块错误
     * @return 格式化后的消息
     */
    QString formatErrorMessage(const ModuleError& error) const;

    /**
     * @brief 创建错误日志条目
     * @param error 模块错误
     * @return 日志条目
     */
    ILogger::LogEntry createErrorLogEntry(const ModuleError& error) const;

    // 单例实例
    static ErrorLogger* s_instance;

    // 配置选项
    QMap<ModuleError::Severity, ILogger::LogLevel> m_levelMappings; ///< 级别映射
    bool m_logErrorContext;                         ///< 记录错误上下文
    bool m_logStackTrace;                          ///< 记录堆栈跟踪
    bool m_statisticsLoggingEnabled;               ///< 统计日志启用状态
    int m_statisticsLogInterval;                   ///< 统计日志间隔

    // 统计定时器
    QTimer* m_statisticsTimer;

    // 状态
    bool m_initialized;                            ///< 初始化状态

    // 常量
    static const int DEFAULT_STATISTICS_INTERVAL = 300000; // 5分钟
};

/**
 * @brief 模块日志助手 - 为模块提供便利的日志记录功能
 */
class ModuleLogHelper
{
public:
    explicit ModuleLogHelper(const QString& moduleName);

    /**
     * @brief 记录模块日志
     * @param level 日志级别
     * @param message 日志消息
     * @param category 日志分类
     */
    void log(ILogger::LogLevel level, const QString& message, const QString& category = QString()) const;

    /**
     * @brief 记录模块错误
     * @param error 模块错误
     */
    void logError(const ModuleError& error) const;

    /**
     * @brief 创建模块错误并记录
     * @param type 错误类型
     * @param severity 严重程度
     * @param message 错误消息
     * @return 创建的错误对象
     */
    ModuleError createAndLogError(ModuleError::ErrorType type, 
                                 ModuleError::Severity severity, 
                                 const QString& message) const;

    /**
     * @brief 获取模块名称
     * @return 模块名称
     */
    QString moduleName() const;

    // 便利方法
    void trace(const QString& message, const QString& category = QString()) const;
    void debug(const QString& message, const QString& category = QString()) const;
    void info(const QString& message, const QString& category = QString()) const;
    void warning(const QString& message, const QString& category = QString()) const;
    void error(const QString& message, const QString& category = QString()) const;
    void critical(const QString& message, const QString& category = QString()) const;
    void fatal(const QString& message, const QString& category = QString()) const;

private:
    QString m_moduleName;                          ///< 模块名称
};

// 便利宏定义
#define DECLARE_MODULE_LOGGER(moduleName) \
    static const ModuleLogHelper s_moduleLogger(#moduleName);

#define MODULE_TRACE(message) s_moduleLogger.trace(message)
#define MODULE_DEBUG(message) s_moduleLogger.debug(message)
#define MODULE_INFO(message) s_moduleLogger.info(message)
#define MODULE_WARNING(message) s_moduleLogger.warning(message)
#define MODULE_ERROR(message) s_moduleLogger.error(message)
#define MODULE_CRITICAL(message) s_moduleLogger.critical(message)
#define MODULE_FATAL(message) s_moduleLogger.fatal(message)

#define MODULE_LOG_ERROR(type, severity, message) \
    s_moduleLogger.createAndLogError(ModuleError::type, ModuleError::severity, message)

#endif // ERRORLOGGER_H