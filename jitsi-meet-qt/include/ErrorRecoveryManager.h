#ifndef ERRORRECOVERYMANAGER_H
#define ERRORRECOVERYMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMessageBox>
#include <QLoggingCategory>
#include <QTextStream>
#include <QFile>
#include <QMutex>
#include "JitsiError.h"

Q_DECLARE_LOGGING_CATEGORY(errorRecovery)

class ConfigurationManager;
class QWebEngineView;

/**
 * @brief 错误恢复管理器
 * 
 * 负责处理应用程序中的各种错误情况，提供错误恢复机制、
 * 用户反馈界面和错误日志记录功能
 */
class ErrorRecoveryManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 恢复策略枚举
     */
    enum class RecoveryStrategy {
        None,           // 不进行恢复
        Retry,          // 重试操作
        Reset,          // 重置到默认状态
        Restart,        // 重启组件
        Fallback,       // 使用备用方案
        UserIntervention // 需要用户干预
    };
    
    /**
     * @brief 错误处理结果
     */
    struct RecoveryResult {
        bool success;
        RecoveryStrategy strategy;
        QString message;
        QVariantMap data;
        
        RecoveryResult(bool s = false, RecoveryStrategy st = RecoveryStrategy::None, const QString& msg = QString())
            : success(s), strategy(st), message(msg) {}
    };

public:
    explicit ErrorRecoveryManager(QObject* parent = nullptr);
    ~ErrorRecoveryManager();
    
    /**
     * @brief 设置配置管理器引用
     */
    void setConfigurationManager(ConfigurationManager* configManager);
    
    /**
     * @brief 处理错误
     * @param error 错误对象
     * @return 恢复结果
     */
    RecoveryResult handleError(const JitsiError& error);
    
    /**
     * @brief 显示错误对话框
     * @param error 错误对象
     * @param parent 父窗口
     * @return 用户选择的按钮
     */
    QMessageBox::StandardButton showErrorDialog(const JitsiError& error, QWidget* parent = nullptr);
    
    /**
     * @brief 尝试自动恢复
     * @param errorType 错误类型
     * @return 恢复结果
     */
    RecoveryResult attemptRecovery(ErrorType errorType);
    
    /**
     * @brief 记录错误到日志文件
     * @param error 错误对象
     */
    void logError(const JitsiError& error);
    
    /**
     * @brief 启用/禁用错误日志记录
     */
    void setLoggingEnabled(bool enabled) { m_loggingEnabled = enabled; }
    bool isLoggingEnabled() const { return m_loggingEnabled; }
    
    /**
     * @brief 设置日志文件路径
     */
    void setLogFilePath(const QString& path);
    QString logFilePath() const { return m_logFilePath; }
    
    /**
     * @brief 清理错误日志
     */
    void clearErrorLog();
    
    /**
     * @brief 获取错误统计信息
     */
    QMap<ErrorType, int> getErrorStatistics() const { return m_errorStats; }
    
    /**
     * @brief 重置错误统计
     */
    void resetErrorStatistics() { m_errorStats.clear(); }
    
    /**
     * @brief 设置最大重试次数
     */
    void setMaxRetryCount(int count) { m_maxRetryCount = count; }
    int maxRetryCount() const { return m_maxRetryCount; }

public slots:
    /**
     * @brief 处理网络错误
     */
    void handleNetworkError(const QString& message, const QString& details = QString());
    
    /**
     * @brief 处理URL验证错误
     */
    void handleUrlValidationError(const QString& url, const QString& reason = QString());
    
    /**
     * @brief 处理WebEngine错误
     */
    void handleWebEngineError(const QString& message, const QString& details = QString());
    
    /**
     * @brief 处理配置错误
     */
    void handleConfigurationError(const QString& message, const QString& details = QString());
    
    /**
     * @brief 处理协议错误
     */
    void handleProtocolError(const QString& message, const QString& details = QString());
    
    /**
     * @brief 处理WebRTC错误
     */
    void handleWebRTCError(const QString& message, const QString& details = QString());
    
    /**
     * @brief 处理XMPP连接错误
     */
    void handleXMPPConnectionError(const QString& message, const QString& details = QString());
    
    /**
     * @brief 处理认证错误
     */
    void handleAuthenticationError(const QString& message, const QString& details = QString());
    
    /**
     * @brief 处理媒体设备错误
     */
    void handleMediaDeviceError(const QString& message, const QString& details = QString());

signals:
    /**
     * @brief 错误已处理信号
     */
    void errorHandled(const JitsiError& error, const RecoveryResult& result);
    
    /**
     * @brief 需要重启应用程序信号
     */
    void restartRequired(const QString& reason);
    
    /**
     * @brief 需要用户干预信号
     */
    void userInterventionRequired(const JitsiError& error);
    
    /**
     * @brief 恢复成功信号
     */
    void recoverySuccessful(ErrorType errorType, RecoveryStrategy strategy);
    
    /**
     * @brief 恢复失败信号
     */
    void recoveryFailed(ErrorType errorType, const QString& reason);

private slots:
    void onRetryTimer();

private:
    // 错误处理方法
    RecoveryResult handleNetworkErrorInternal(const JitsiError& error);
    RecoveryResult handleInvalidUrlErrorInternal(const JitsiError& error);
    RecoveryResult handleWebEngineErrorInternal(const JitsiError& error);
    RecoveryResult handleConfigurationErrorInternal(const JitsiError& error);
    RecoveryResult handleProtocolErrorInternal(const JitsiError& error);
    RecoveryResult handleValidationErrorInternal(const JitsiError& error);
    RecoveryResult handleSystemErrorInternal(const JitsiError& error);
    RecoveryResult handleWebRTCErrorInternal(const JitsiError& error);
    RecoveryResult handleXMPPConnectionErrorInternal(const JitsiError& error);
    RecoveryResult handleAuthenticationErrorInternal(const JitsiError& error);
    RecoveryResult handleMediaDeviceErrorInternal(const JitsiError& error);
    
    // 恢复策略实现
    bool resetToDefaults();
    bool clearCache();
    bool restartWebEngine();
    bool validateAndFixConfiguration();
    bool retryLastOperation();
    bool restartXMPPConnection();
    bool reinitializeMediaDevices();
    
    // 日志相关
    void initializeLogging();
    void writeToLogFile(const QString& logEntry);
    void rotateLogFile();
    
    // 对话框创建
    QMessageBox* createErrorDialog(const JitsiError& error, QWidget* parent);
    void setupDialogButtons(QMessageBox* dialog, const JitsiError& error);
    
    // 统计更新
    void updateErrorStatistics(ErrorType errorType);
    
private:
    ConfigurationManager* m_configManager;
    
    // 日志相关
    bool m_loggingEnabled;
    QString m_logFilePath;
    QFile* m_logFile;
    QTextStream* m_logStream;
    QMutex m_logMutex;
    qint64 m_maxLogFileSize;
    
    // 重试机制
    QTimer* m_retryTimer;
    int m_maxRetryCount;
    QMap<ErrorType, int> m_retryCount;
    
    // 统计信息
    QMap<ErrorType, int> m_errorStats;
    
    // 最后的错误和操作
    JitsiError m_lastError;
    QString m_lastOperation;
    QVariantMap m_lastOperationData;
    
    // 配置
    bool m_showErrorDialogs;
    bool m_autoRecoveryEnabled;
    int m_dialogTimeout;
};

#endif // ERRORRECOVERYMANAGER_H