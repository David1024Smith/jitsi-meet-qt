#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QMutex>
#include <QTextStream>
#include <QList>
#include <memory>

// 前向声明
class ILogger;

/**
 * @brief 统一日志记录系统
 * 
 * Logger类提供统一的日志记录接口，支持多种日志输出方式，
 * 包括文件、控制台和网络日志。支持日志级别过滤和格式化。
 */
class Logger : public QObject
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
    };

    /**
     * @brief 获取Logger单例实例
     * @return Logger实例指针
     */
    static Logger* instance();

    /**
     * @brief 析构函数
     */
    ~Logger();

    /**
     * @brief 初始化日志系统
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 清理日志系统
     */
    void cleanup();

    /**
     * @brief 添加日志记录器
     * @param logger 日志记录器实例
     */
    void addLogger(std::shared_ptr<ILogger> logger);

    /**
     * @brief 移除日志记录器
     * @param logger 日志记录器实例
     */
    void removeLogger(std::shared_ptr<ILogger> logger);

    /**
     * @brief 设置全局日志级别
     * @param level 日志级别
     */
    void setGlobalLogLevel(LogLevel level);

    /**
     * @brief 获取全局日志级别
     * @return 当前全局日志级别
     */
    LogLevel globalLogLevel() const;

    /**
     * @brief 设置日志格式
     * @param format 格式字符串
     */
    void setLogFormat(const QString& format);

    /**
     * @brief 获取日志格式
     * @return 当前日志格式
     */
    QString logFormat() const;

    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param category 日志分类
     * @param message 日志消息
     * @param file 源文件名
     * @param line 源文件行号
     */
    void log(LogLevel level, const QString& category, const QString& message, 
             const QString& file = QString(), int line = 0);

    /**
     * @brief 格式化日志条目
     * @param entry 日志条目
     * @return 格式化后的字符串
     */
    QString formatLogEntry(const LogEntry& entry) const;

    /**
     * @brief 日志级别转字符串
     * @param level 日志级别
     * @return 级别字符串
     */
    static QString levelToString(LogLevel level);

    /**
     * @brief 字符串转日志级别
     * @param levelStr 级别字符串
     * @return 日志级别
     */
    static LogLevel stringToLevel(const QString& levelStr);

    // 便捷的静态日志方法
    static void debug(const QString& message, const QString& category = "General");
    static void info(const QString& message, const QString& category = "General");
    static void warning(const QString& message, const QString& category = "General");
    static void error(const QString& message, const QString& category = "General");
    static void critical(const QString& message, const QString& category = "General");

signals:
    /**
     * @brief 日志记录信号
     * @param entry 日志条目
     */
    void logRecorded(const LogEntry& entry);

    /**
     * @brief 日志级别改变信号
     * @param level 新的日志级别
     */
    void logLevelChanged(LogLevel level);

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit Logger(QObject* parent = nullptr);

    /**
     * @brief 创建默认日志记录器
     */
    void createDefaultLoggers();

    /**
     * @brief 检查日志级别是否应该记录
     * @param level 日志级别
     * @return 是否应该记录
     */
    bool shouldLog(LogLevel level) const;

private:
    static Logger* s_instance;                          ///< 单例实例
    static QMutex s_mutex;                              ///< 线程安全互斥锁

    QList<std::shared_ptr<ILogger>> m_loggers;          ///< 日志记录器列表
    LogLevel m_globalLogLevel;                          ///< 全局日志级别
    QString m_logFormat;                                ///< 日志格式
    mutable QMutex m_logMutex;                          ///< 日志记录互斥锁

    // 禁用拷贝构造和赋值操作
    Q_DISABLE_COPY(Logger)
};

// 便捷宏定义
#define LOG_DEBUG(msg) Logger::debug(msg, __FILE__)
#define LOG_INFO(msg) Logger::info(msg, __FILE__)
#define LOG_WARNING(msg) Logger::warning(msg, __FILE__)
#define LOG_ERROR(msg) Logger::error(msg, __FILE__)
#define LOG_CRITICAL(msg) Logger::critical(msg, __FILE__)

#define LOG_DEBUG_CAT(msg, cat) Logger::debug(msg, cat)
#define LOG_INFO_CAT(msg, cat) Logger::info(msg, cat)
#define LOG_WARNING_CAT(msg, cat) Logger::warning(msg, cat)
#define LOG_ERROR_CAT(msg, cat) Logger::error(msg, cat)
#define LOG_CRITICAL_CAT(msg, cat) Logger::critical(msg, cat)

#endif // LOGGER_H