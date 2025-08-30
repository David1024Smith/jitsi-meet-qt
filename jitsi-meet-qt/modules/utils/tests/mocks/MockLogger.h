#ifndef MOCKLOGGER_H
#define MOCKLOGGER_H

#include "../../interfaces/ILogger.h"
#include <QStringList>
#include <QMutex>

/**
 * @brief 模拟日志记录器
 * 
 * 用于测试的模拟日志记录器，记录所有日志调用以便验证。
 */
class MockLogger : public ILogger
{
    Q_OBJECT

public:
    explicit MockLogger(QObject* parent = nullptr);
    ~MockLogger() override = default;

    // ILogger接口实现
    bool initialize() override;
    void cleanup() override;
    void log(const LogEntry& entry) override;
    void flush() override;
    void setLogLevel(LogLevel level) override;
    LogLevel logLevel() const override;
    QString name() const override;
    QString version() const override;

    // 测试辅助方法
    void clearLogs();
    QList<LogEntry> getLogs() const;
    QList<LogEntry> getLogsForLevel(LogLevel level) const;
    QList<LogEntry> getLogsForCategory(const QString& category) const;
    int getLogCount() const;
    int getLogCountForLevel(LogLevel level) const;
    bool hasLogWithMessage(const QString& message) const;
    LogEntry getLastLog() const;
    
    // 模拟控制
    void setInitializeResult(bool result);
    void setFlushDelay(int milliseconds);
    void setThrowOnLog(bool throwException);

private:
    mutable QMutex m_mutex;
    QList<LogEntry> m_logs;
    LogLevel m_logLevel;
    bool m_initialized;
    bool m_initializeResult;
    int m_flushDelay;
    bool m_throwOnLog;
};

#endif // MOCKLOGGER_H