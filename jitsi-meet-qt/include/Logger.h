#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>

/**
 * @brief 日志记录器
 * 
 * 提供统一的日志记录功能
 */
class Logger : public QObject
{
    Q_OBJECT

public:
    enum LogLevel {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    explicit Logger(QObject* parent = nullptr);
    ~Logger();

    /**
     * @brief 获取单例实例
     * @return Logger实例
     */
    static Logger* instance();

    /**
     * @brief 设置日志文件路径
     * @param filePath 文件路径
     */
    void setLogFile(const QString& filePath);

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param message 日志消息
     */
    void log(LogLevel level, const QString& message);

    /**
     * @brief 记录日志（带分类）
     * @param level 日志级别
     * @param category 日志分类
     * @param message 日志消息
     */
    void log(LogLevel level, const QString& category, const QString& message);

    /**
     * @brief 记录调试信息
     * @param message 消息
     */
    void debug(const QString& message);

    /**
     * @brief 记录信息
     * @param message 消息
     */
    void info(const QString& message);

    /**
     * @brief 记录警告
     * @param message 消息
     */
    void warning(const QString& message);

    /**
     * @brief 记录错误
     * @param message 消息
     */
    void error(const QString& message);

    /**
     * @brief 记录严重错误
     * @param message 消息
     */
    void critical(const QString& message);

    /**
     * @brief 初始化日志系统
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 清理日志系统
     */
    void cleanup();

private:
    static Logger* s_instance;
    QFile* m_logFile;
    QTextStream* m_stream;
    LogLevel m_logLevel;
    QMutex m_mutex;

    QString levelToString(LogLevel level) const;
};

#endif // LOGGER_H