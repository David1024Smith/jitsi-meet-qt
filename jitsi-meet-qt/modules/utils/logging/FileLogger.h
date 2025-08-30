#ifndef FILELOGGER_H
#define FILELOGGER_H

#include "../interfaces/ILogger.h"
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QTimer>

/**
 * @brief 文件日志记录器
 * 
 * FileLogger实现了基于文件的日志记录功能，支持日志轮转、
 * 异步写入和自动刷新等特性。
 */
class FileLogger : public ILogger
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param filePath 日志文件路径
     * @param parent 父对象
     */
    explicit FileLogger(const QString& filePath, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~FileLogger() override;

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
     * @brief 设置日志文件路径
     * @param filePath 文件路径
     */
    void setFilePath(const QString& filePath);

    /**
     * @brief 获取日志文件路径
     * @return 文件路径
     */
    QString filePath() const;

    /**
     * @brief 设置最大文件大小（字节）
     * @param maxSize 最大大小
     */
    void setMaxFileSize(qint64 maxSize);

    /**
     * @brief 获取最大文件大小
     * @return 最大大小
     */
    qint64 maxFileSize() const;

    /**
     * @brief 设置最大备份文件数量
     * @param maxBackups 最大备份数
     */
    void setMaxBackupFiles(int maxBackups);

    /**
     * @brief 获取最大备份文件数量
     * @return 最大备份数
     */
    int maxBackupFiles() const;

    /**
     * @brief 设置自动刷新间隔（毫秒）
     * @param interval 刷新间隔
     */
    void setFlushInterval(int interval);

    /**
     * @brief 获取自动刷新间隔
     * @return 刷新间隔
     */
    int flushInterval() const;

    /**
     * @brief 启用/禁用日志轮转
     * @param enabled 是否启用
     */
    void setRotationEnabled(bool enabled);

    /**
     * @brief 检查日志轮转是否启用
     * @return 是否启用
     */
    bool isRotationEnabled() const;

    /**
     * @brief 手动轮转日志文件
     * @return 轮转是否成功
     */
    bool rotateLog();

    /**
     * @brief 获取当前日志文件大小
     * @return 文件大小
     */
    qint64 currentFileSize() const;

private slots:
    /**
     * @brief 自动刷新定时器槽函数
     */
    void onFlushTimer();

private:
    /**
     * @brief 打开日志文件
     * @return 打开是否成功
     */
    bool openLogFile();

    /**
     * @brief 关闭日志文件
     */
    void closeLogFile();

    /**
     * @brief 检查是否需要轮转
     * @return 是否需要轮转
     */
    bool needsRotation() const;

    /**
     * @brief 执行日志轮转
     */
    void performRotation();

    /**
     * @brief 清理旧的备份文件
     */
    void cleanupOldBackups();

    /**
     * @brief 生成备份文件名
     * @param index 备份索引
     * @return 备份文件名
     */
    QString generateBackupFileName(int index) const;

    /**
     * @brief 写入日志条目到文件
     * @param entry 日志条目
     */
    void writeToFile(const LogEntry& entry);

private:
    QString m_filePath;                 ///< 日志文件路径
    LogLevel m_logLevel;                ///< 日志级别
    QString m_format;                   ///< 日志格式
    bool m_enabled;                     ///< 是否启用
    
    QFile* m_logFile;                   ///< 日志文件对象
    QTextStream* m_stream;              ///< 文本流
    
    qint64 m_maxFileSize;               ///< 最大文件大小
    int m_maxBackupFiles;               ///< 最大备份文件数
    bool m_rotationEnabled;             ///< 是否启用轮转
    
    QTimer* m_flushTimer;               ///< 自动刷新定时器
    int m_flushInterval;                ///< 刷新间隔
    
    mutable QMutex m_mutex;             ///< 线程安全互斥锁
};

#endif // FILELOGGER_H