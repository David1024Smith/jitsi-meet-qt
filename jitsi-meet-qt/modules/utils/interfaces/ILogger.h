#ifndef ILOGGER_H
#define ILOGGER_H

#include <QObject>
#include <QString>
#include <QDateTime>

/**
 * @brief 日志记录器接口
 * 
 * ILogger定义了日志记录器的标准接口，所有具体的日志记录器
 * 实现都应该继承此接口。支持不同的日志输出方式和格式。
 */
class ILogger : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 日志级别枚举
     */
    enum LogLevel {
        Debug = 0,      ///< 调试信息
        Info = 1,       ///< 一般信息
        Warning = 2,    ///< 警告信息
        Error = 3,      ///< 错误信息
        Critical = 4    ///< 严重错误
    };
    Q_ENUM(LogLevel)

    /**
     * @brief 日志条目结构
     */
    struct LogEntry {
        QDateTime timestamp;    ///< 时间戳
        LogLevel level;         ///< 日志级别
        QString category;       ///< 日志分类
        QString message;        ///< 日志消息
        QString thread;         ///< 线程ID
        QString file;           ///< 源文件名
        int line;               ///< 源文件行号
        
        LogEntry() : level(Info), line(0) {}
        
        LogEntry(LogLevel lvl, const QString& cat, const QString& msg)
            : timestamp(QDateTime::currentDateTime())
            , level(lvl)
            , category(cat)
            , message(msg)
            , line(0)
        {}
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ILogger(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief 虚析构函数
     */
    virtual ~ILogger() = default;

    /**
     * @brief 初始化日志记录器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 清理日志记录器
     */
    virtual void cleanup() = 0;

    /**
     * @brief 记录日志条目
     * @param entry 日志条目
     */
    virtual void log(const LogEntry& entry) = 0;

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    virtual void setLogLevel(LogLevel level) = 0;

    /**
     * @brief 获取日志级别
     * @return 当前日志级别
     */
    virtual LogLevel logLevel() const = 0;

    /**
     * @brief 设置日志格式
     * @param format 格式字符串
     */
    virtual void setFormat(const QString& format) = 0;

    /**
     * @brief 获取日志格式
     * @return 当前日志格式
     */
    virtual QString format() const = 0;

    /**
     * @brief 检查是否启用
     * @return 是否启用
     */
    virtual bool isEnabled() const = 0;

    /**
     * @brief 设置启用状态
     * @param enabled 是否启用
     */
    virtual void setEnabled(bool enabled) = 0;

    /**
     * @brief 获取日志记录器名称
     * @return 记录器名称
     */
    virtual QString name() const = 0;

    /**
     * @brief 刷新日志缓冲区
     */
    virtual void flush() = 0;

    /**
     * @brief 日志级别转字符串
     * @param level 日志级别
     * @return 级别字符串
     */
    static QString levelToString(LogLevel level) {
        switch (level) {
            case Debug: return "DEBUG";
            case Info: return "INFO";
            case Warning: return "WARNING";
            case Error: return "ERROR";
            case Critical: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }

    /**
     * @brief 字符串转日志级别
     * @param levelStr 级别字符串
     * @return 日志级别
     */
    static LogLevel stringToLevel(const QString& levelStr) {
        QString upper = levelStr.toUpper();
        if (upper == "DEBUG") return Debug;
        if (upper == "INFO") return Info;
        if (upper == "WARNING") return Warning;
        if (upper == "ERROR") return Error;
        if (upper == "CRITICAL") return Critical;
        return Info; // 默认级别
    }

signals:
    /**
     * @brief 日志记录信号
     * @param entry 日志条目
     */
    void logRecorded(const LogEntry& entry);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

protected:
    /**
     * @brief 格式化日志条目
     * @param entry 日志条目
     * @param format 格式字符串
     * @return 格式化后的字符串
     */
    virtual QString formatEntry(const LogEntry& entry, const QString& format) const {
        QString result = format;
        
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

    /**
     * @brief 检查日志级别是否应该记录
     * @param level 日志级别
     * @return 是否应该记录
     */
    virtual bool shouldLog(LogLevel level) const {
        return isEnabled() && level >= logLevel();
    }
};

// 声明元类型以支持信号槽
Q_DECLARE_METATYPE(ILogger::LogLevel)
Q_DECLARE_METATYPE(ILogger::LogEntry)

#endif // ILOGGER_H