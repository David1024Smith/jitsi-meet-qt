#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "../interfaces/ILogger.h"
#include <QTextStream>
#include <QMutex>

/**
 * @brief 控制台日志记录器
 * 
 * ConsoleLogger实现了基于控制台的日志记录功能，支持彩色输出、
 * 不同输出流（stdout/stderr）和实时日志显示。
 */
class ConsoleLogger : public ILogger
{
    Q_OBJECT

public:
    /**
     * @brief 输出流类型枚举
     */
    enum OutputStream {
        StandardOutput,     ///< 标准输出流
        StandardError,      ///< 标准错误流
        Auto               ///< 自动选择（错误级别使用stderr，其他使用stdout）
    };
    Q_ENUM(OutputStream)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ConsoleLogger(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ConsoleLogger() override;

    // ILogger接口实现
    bool initialize() override;
    void cleanup() override;
    void log(const LogEntry& entry) override;
    void setLogLevel(LogLevel level) override;
    LogLevel logLevel() const override;
    void setFormat(const QString& format) override;
    QString format() const override;
    bool isEnabled() const override;
    void setEnabled(bool enabled) override;
    QString name() const override;
    void flush() override;

    /**
     * @brief 设置输出流类型
     * @param stream 输出流类型
     */
    void setOutputStream(OutputStream stream);

    /**
     * @brief 获取输出流类型
     * @return 输出流类型
     */
    OutputStream outputStream() const;

    /**
     * @brief 启用/禁用彩色输出
     * @param enabled 是否启用彩色
     */
    void setColorEnabled(bool enabled);

    /**
     * @brief 检查彩色输出是否启用
     * @return 是否启用彩色
     */
    bool isColorEnabled() const;

    /**
     * @brief 设置时间戳显示
     * @param enabled 是否显示时间戳
     */
    void setTimestampEnabled(bool enabled);

    /**
     * @brief 检查时间戳显示是否启用
     * @return 是否显示时间戳
     */
    bool isTimestampEnabled() const;

    /**
     * @brief 设置线程ID显示
     * @param enabled 是否显示线程ID
     */
    void setThreadIdEnabled(bool enabled);

    /**
     * @brief 检查线程ID显示是否启用
     * @return 是否显示线程ID
     */
    bool isThreadIdEnabled() const;

    /**
     * @brief 设置日志级别颜色
     * @param level 日志级别
     * @param color 颜色代码
     */
    void setLevelColor(LogLevel level, const QString& color);

    /**
     * @brief 获取日志级别颜色
     * @param level 日志级别
     * @return 颜色代码
     */
    QString levelColor(LogLevel level) const;

    /**
     * @brief 检查控制台是否支持彩色输出
     * @return 是否支持彩色
     */
    static bool supportsColor();

private:
    /**
     * @brief 格式化带颜色的日志条目
     * @param entry 日志条目
     * @return 格式化后的字符串
     */
    QString formatColoredEntry(const LogEntry& entry) const;

    /**
     * @brief 获取日志级别的颜色代码
     * @param level 日志级别
     * @return 颜色代码
     */
    QString getColorCode(LogLevel level) const;

    /**
     * @brief 选择输出流
     * @param level 日志级别
     * @return 输出流指针
     */
    QTextStream* selectOutputStream(LogLevel level) const;

    /**
     * @brief 初始化默认颜色
     */
    void initializeDefaultColors();

private:
    LogLevel m_logLevel;                    ///< 日志级别
    QString m_format;                       ///< 日志格式
    bool m_enabled;                         ///< 是否启用
    
    OutputStream m_outputStream;            ///< 输出流类型
    bool m_colorEnabled;                    ///< 是否启用彩色
    bool m_timestampEnabled;                ///< 是否显示时间戳
    bool m_threadIdEnabled;                 ///< 是否显示线程ID
    
    QTextStream* m_stdout;                  ///< 标准输出流
    QTextStream* m_stderr;                  ///< 标准错误流
    
    QHash<LogLevel, QString> m_levelColors; ///< 日志级别颜色映射
    
    mutable QMutex m_mutex;                 ///< 线程安全互斥锁

    // ANSI颜色代码常量
    static const QString COLOR_RESET;
    static const QString COLOR_BLACK;
    static const QString COLOR_RED;
    static const QString COLOR_GREEN;
    static const QString COLOR_YELLOW;
    static const QString COLOR_BLUE;
    static const QString COLOR_MAGENTA;
    static const QString COLOR_CYAN;
    static const QString COLOR_WHITE;
    static const QString COLOR_BRIGHT_BLACK;
    static const QString COLOR_BRIGHT_RED;
    static const QString COLOR_BRIGHT_GREEN;
    static const QString COLOR_BRIGHT_YELLOW;
    static const QString COLOR_BRIGHT_BLUE;
    static const QString COLOR_BRIGHT_MAGENTA;
    static const QString COLOR_BRIGHT_CYAN;
    static const QString COLOR_BRIGHT_WHITE;
};

#endif // CONSOLELOGGER_H