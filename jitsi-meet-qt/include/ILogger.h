#ifndef ILOGGER_H
#define ILOGGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariantMap>

/**
 * @brief 日志接口 - 统一的日志记录系统
 * 
 * 该接口定义了模块化系统中的统一日志记录机制，包括：
 * - 多级别日志记录
 * - 结构化日志信息
 * - 日志过滤和格式化
 * - 多种输出目标支持
 */
class ILogger
{
public:
    /**
     * @brief 日志级别枚举
     */
    enum LogLevel {
        Trace = 0,      ///< 跟踪信息
        Debug = 1,      ///< 调试信息
        Info = 2,       ///< 一般信息
        Warning = 3,    ///< 警告信息
        Error = 4,      ///< 错误信息
        Critical = 5,   ///< 严重错误
        Fatal = 6       ///< 致命错误
    };

    /**
     * @brief 日志条目结构
     */
    struct LogEntry {
        QDateTime timestamp;        ///< 时间戳
        LogLevel level;            ///< 日志级别
        QString category;          ///< 日志分类
        QString message;           ///< 日志消息
        QString moduleName;        ///< 模块名称
        QString fileName;          ///< 文件名
        QString functionName;      ///< 函数名
        int lineNumber;            ///< 行号
        QVariantMap context;       ///< 上下文信息
        qint64 threadId;           ///< 线程ID
        
        LogEntry() : level(Info), lineNumber(0), threadId(0) {}
    };

    virtual ~ILogger() = default;

    /**
     * @brief 记录日志
     * @param entry 日志条目
     */
    virtual void log(const LogEntry& entry) = 0;

    /**
     * @brief 记录日志 (简化版本)
     * @param level 日志级别
     * @param message 日志消息
     * @param category 日志分类
     */
    virtual void log(LogLevel level, const QString& message, const QString& category = QString()) = 0;

    /**
     * @brief 设置日志级别
     * @param level 最小日志级别
     */
    virtual void setLogLevel(LogLevel level) = 0;

    /**
     * @brief 获取日志级别
     * @return 当前日志级别
     */
    virtual LogLevel logLevel() const = 0;

    /**
     * @brief 检查是否启用指定级别的日志
     * @param level 日志级别
     * @return 是否启用
     */
    virtual bool isEnabled(LogLevel level) const = 0;

    /**
     * @brief 刷新日志缓冲区
     */
    virtual void flush() = 0;

    /**
     * @brief 获取日志级别名称
     * @param level 日志级别
     * @return 级别名称
     */
    static QString levelName(LogLevel level);

    /**
     * @brief 从字符串解析日志级别
     * @param levelName 级别名称
     * @return 日志级别
     */
    static LogLevel parseLevel(const QString& levelName);
};

Q_DECLARE_METATYPE(ILogger::LogLevel)
Q_DECLARE_METATYPE(ILogger::LogEntry)

#endif // ILOGGER_H