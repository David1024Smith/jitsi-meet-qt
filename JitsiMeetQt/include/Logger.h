#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

/**
 * @brief 统一的日志系统类
 * 
 * 该类提供统一的日志输出功能，支持不同的日志级别。
 * Debug版本：输出所有级别的日志到控制台和文件
 * Release版本：不输出任何日志信息
 */
class Logger
{
public:
    /**
     * @brief 日志级别枚举
     */
    enum LogLevel {
        Debug = 0,    ///< 调试信息
        Info = 1,     ///< 一般信息
        Warning = 2,  ///< 警告信息
        Error = 3,    ///< 错误信息
        Critical = 4  ///< 严重错误
    };

    /**
     * @brief 获取Logger单例实例
     * @return Logger实例引用
     */
    static Logger& instance();

    /**
     * @brief 初始化日志系统
     * @param enableFileLogging 是否启用文件日志（Debug版本为true，Release版本为false）
     * @param logFileName 日志文件名（默认为"jitsi_meet_qt.log"）
     */
    void initialize(bool enableFileLogging = true, const QString& logFileName = "jitsi_meet_qt.log");

    /**
     * @brief 设置最小日志级别
     * @param level 最小日志级别
     */
    void setMinLogLevel(LogLevel level);

    /**
     * @brief 输出调试信息
     * @param message 日志消息
     * @param category 日志分类（可选）
     */
    void debug(const QString& message, const QString& category = QString());

    /**
     * @brief 输出一般信息
     * @param message 日志消息
     * @param category 日志分类（可选）
     */
    void info(const QString& message, const QString& category = QString());

    /**
     * @brief 输出警告信息
     * @param message 日志消息
     * @param category 日志分类（可选）
     */
    void warning(const QString& message, const QString& category = QString());

    /**
     * @brief 输出错误信息
     * @param message 日志消息
     * @param category 日志分类（可选）
     */
    void error(const QString& message, const QString& category = QString());

    /**
     * @brief 输出严重错误信息
     * @param message 日志消息
     * @param category 日志分类（可选）
     */
    void critical(const QString& message, const QString& category = QString());

    /**
     * @brief 关闭日志系统
     */
    void shutdown();

    /**
     * @brief 检查是否启用了日志记录
     * @return 如果启用了日志记录返回true，否则返回false
     */
    bool isLoggingEnabled() const { return m_loggingEnabled; }

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief 内部日志输出函数
     * @param level 日志级别
     * @param message 日志消息
     * @param category 日志分类
     */
    void log(LogLevel level, const QString& message, const QString& category);

    /**
     * @brief 获取日志级别字符串
     * @param level 日志级别
     * @return 日志级别字符串
     */
    QString levelToString(LogLevel level) const;

    /**
     * @brief 格式化日志消息
     * @param level 日志级别
     * @param message 原始消息
     * @param category 日志分类
     * @return 格式化后的日志消息
     */
    QString formatMessage(LogLevel level, const QString& message, const QString& category) const;

private:
    bool m_loggingEnabled = false;        ///< 是否启用日志记录
    bool m_fileLoggingEnabled = false;    ///< 是否启用文件日志
    LogLevel m_minLogLevel = Debug;       ///< 最小日志级别
    QString m_logFileName;                ///< 日志文件名
    QFile m_logFile;                      ///< 日志文件对象
    QTextStream m_logStream;              ///< 日志文件流
    QMutex m_mutex;                       ///< 线程安全互斥锁
};

// 便捷的全局日志宏定义
#ifdef _DEBUG
    #define LOG_DEBUG(msg, ...) Logger::instance().debug(QString(msg).arg(__VA_ARGS__))
    #define LOG_INFO(msg, ...) Logger::instance().info(QString(msg).arg(__VA_ARGS__))
    #define LOG_WARNING(msg, ...) Logger::instance().warning(QString(msg).arg(__VA_ARGS__))
    #define LOG_ERROR(msg, ...) Logger::instance().error(QString(msg).arg(__VA_ARGS__))
    #define LOG_CRITICAL(msg, ...) Logger::instance().critical(QString(msg).arg(__VA_ARGS__))
#else
    #define LOG_DEBUG(msg, ...) do {} while(0)
    #define LOG_INFO(msg, ...) do {} while(0)
    #define LOG_WARNING(msg, ...) do {} while(0)
    #define LOG_ERROR(msg, ...) do {} while(0)
    #define LOG_CRITICAL(msg, ...) do {} while(0)
#endif

// 带分类的日志宏
#ifdef _DEBUG
    #define LOG_DEBUG_CAT(category, msg, ...) Logger::instance().debug(QString(msg).arg(__VA_ARGS__), category)
    #define LOG_INFO_CAT(category, msg, ...) Logger::instance().info(QString(msg).arg(__VA_ARGS__), category)
    #define LOG_WARNING_CAT(category, msg, ...) Logger::instance().warning(QString(msg).arg(__VA_ARGS__), category)
    #define LOG_ERROR_CAT(category, msg, ...) Logger::instance().error(QString(msg).arg(__VA_ARGS__), category)
    #define LOG_CRITICAL_CAT(category, msg, ...) Logger::instance().critical(QString(msg).arg(__VA_ARGS__), category)
#else
    #define LOG_DEBUG_CAT(category, msg, ...) do {} while(0)
    #define LOG_INFO_CAT(category, msg, ...) do {} while(0)
    #define LOG_WARNING_CAT(category, msg, ...) do {} while(0)
    #define LOG_ERROR_CAT(category, msg, ...) do {} while(0)
    #define LOG_CRITICAL_CAT(category, msg, ...) do {} while(0)
#endif

#endif // LOGGER_H