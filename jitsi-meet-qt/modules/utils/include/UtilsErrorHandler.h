#ifndef UTILSERRORHANDLER_H
#define UTILSERRORHANDLER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QDateTime>
#include <QMutex>
#include <QQueue>
#include <QTimer>

/**
 * @brief 工具模块错误处理器
 * 
 * UtilsErrorHandler提供统一的错误处理、记录和恢复机制。
 * 支持错误分类、自动恢复策略和错误统计分析。
 */
class UtilsErrorHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 错误级别枚举
     */
    enum ErrorLevel {
        Info,           ///< 信息
        Warning,        ///< 警告
        Error,          ///< 错误
        Critical,       ///< 严重错误
        Fatal           ///< 致命错误
    };
    Q_ENUM(ErrorLevel)

    /**
     * @brief 错误类别枚举
     */
    enum ErrorCategory {
        SystemError,        ///< 系统错误
        ConfigurationError, ///< 配置错误
        FileSystemError,    ///< 文件系统错误
        NetworkError,       ///< 网络错误
        CryptoError,        ///< 加密错误
        ValidationError,    ///< 验证错误
        MemoryError,        ///< 内存错误
        PermissionError,    ///< 权限错误
        TimeoutError,       ///< 超时错误
        UnknownError        ///< 未知错误
    };
    Q_ENUM(ErrorCategory)

    /**
     * @brief 恢复策略枚举
     */
    enum RecoveryStrategy {
        NoRecovery,         ///< 不恢复
        Retry,              ///< 重试
        Fallback,           ///< 回退
        Reset,              ///< 重置
        Restart,            ///< 重启
        Ignore              ///< 忽略
    };
    Q_ENUM(RecoveryStrategy)

    /**
     * @brief 错误信息结构
     */
    struct ErrorInfo {
        QString id;                     ///< 错误ID
        ErrorLevel level;               ///< 错误级别
        ErrorCategory category;         ///< 错误类别
        QString message;                ///< 错误消息
        QString source;                 ///< 错误源
        QString details;                ///< 详细信息
        QVariantMap context;            ///< 上下文信息
        QDateTime timestamp;            ///< 时间戳
        int occurrenceCount;            ///< 发生次数
        RecoveryStrategy strategy;      ///< 恢复策略
        bool recovered;                 ///< 是否已恢复
        
        ErrorInfo() : level(Error), category(UnknownError), 
                     occurrenceCount(1), strategy(NoRecovery), recovered(false) {}
    };

    /**
     * @brief 获取错误处理器单例实例
     * @return UtilsErrorHandler实例指针
     */
    static UtilsErrorHandler* instance();

    /**
     * @brief 析构函数
     */
    ~UtilsErrorHandler();

    /**
     * @brief 初始化错误处理器
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 清理错误处理器
     */
    void cleanup();

    /**
     * @brief 报告错误
     * @param level 错误级别
     * @param category 错误类别
     * @param message 错误消息
     * @param source 错误源
     * @param details 详细信息
     * @param context 上下文信息
     * @return 错误ID
     */
    QString reportError(ErrorLevel level, ErrorCategory category,
                       const QString& message, const QString& source = QString(),
                       const QString& details = QString(),
                       const QVariantMap& context = QVariantMap());

    /**
     * @brief 报告错误（简化版本）
     * @param message 错误消息
     * @param source 错误源
     * @return 错误ID
     */
    QString reportError(const QString& message, const QString& source = QString());

    /**
     * @brief 报告警告
     * @param message 警告消息
     * @param source 错误源
     * @return 错误ID
     */
    QString reportWarning(const QString& message, const QString& source = QString());

    /**
     * @brief 报告信息
     * @param message 信息消息
     * @param source 错误源
     * @return 错误ID
     */
    QString reportInfo(const QString& message, const QString& source = QString());

    /**
     * @brief 标记错误已恢复
     * @param errorId 错误ID
     * @param recoveryDetails 恢复详情
     */
    void markErrorRecovered(const QString& errorId, const QString& recoveryDetails = QString());

    /**
     * @brief 获取错误信息
     * @param errorId 错误ID
     * @return 错误信息
     */
    ErrorInfo getErrorInfo(const QString& errorId) const;

    /**
     * @brief 获取所有错误
     * @return 错误信息列表
     */
    QList<ErrorInfo> getAllErrors() const;

    /**
     * @brief 获取指定级别的错误
     * @param level 错误级别
     * @return 错误信息列表
     */
    QList<ErrorInfo> getErrorsByLevel(ErrorLevel level) const;

    /**
     * @brief 获取指定类别的错误
     * @param category 错误类别
     * @return 错误信息列表
     */
    QList<ErrorInfo> getErrorsByCategory(ErrorCategory category) const;

    /**
     * @brief 获取指定源的错误
     * @param source 错误源
     * @return 错误信息列表
     */
    QList<ErrorInfo> getErrorsBySource(const QString& source) const;

    /**
     * @brief 获取未恢复的错误
     * @return 错误信息列表
     */
    QList<ErrorInfo> getUnrecoveredErrors() const;

    /**
     * @brief 清除所有错误
     */
    void clearAllErrors();

    /**
     * @brief 清除指定级别的错误
     * @param level 错误级别
     */
    void clearErrorsByLevel(ErrorLevel level);

    /**
     * @brief 清除已恢复的错误
     */
    void clearRecoveredErrors();

    /**
     * @brief 获取错误统计信息
     * @return 统计信息映射
     */
    QVariantMap getErrorStatistics() const;

    /**
     * @brief 设置最大错误数量
     * @param maxErrors 最大错误数量
     */
    void setMaxErrorCount(int maxErrors);

    /**
     * @brief 获取最大错误数量
     * @return 最大错误数量
     */
    int maxErrorCount() const;

    /**
     * @brief 设置自动清理间隔
     * @param intervalMs 间隔毫秒数
     */
    void setAutoCleanupInterval(int intervalMs);

    /**
     * @brief 启用/禁用自动清理
     * @param enabled 是否启用
     */
    void setAutoCleanupEnabled(bool enabled);

    /**
     * @brief 设置错误恢复策略
     * @param category 错误类别
     * @param strategy 恢复策略
     */
    void setRecoveryStrategy(ErrorCategory category, RecoveryStrategy strategy);

    /**
     * @brief 获取错误恢复策略
     * @param category 错误类别
     * @return 恢复策略
     */
    RecoveryStrategy getRecoveryStrategy(ErrorCategory category) const;

    /**
     * @brief 尝试自动恢复错误
     * @param errorId 错误ID
     * @return 恢复是否成功
     */
    bool attemptAutoRecovery(const QString& errorId);

    /**
     * @brief 导出错误日志
     * @param filePath 文件路径
     * @param format 导出格式（json/csv/txt）
     * @return 导出是否成功
     */
    bool exportErrorLog(const QString& filePath, const QString& format = "json") const;

    /**
     * @brief 错误级别转字符串
     * @param level 错误级别
     * @return 级别字符串
     */
    static QString errorLevelToString(ErrorLevel level);

    /**
     * @brief 字符串转错误级别
     * @param levelStr 级别字符串
     * @return 错误级别
     */
    static ErrorLevel stringToErrorLevel(const QString& levelStr);

    /**
     * @brief 错误类别转字符串
     * @param category 错误类别
     * @return 类别字符串
     */
    static QString errorCategoryToString(ErrorCategory category);

    /**
     * @brief 字符串转错误类别
     * @param categoryStr 类别字符串
     * @return 错误类别
     */
    static ErrorCategory stringToErrorCategory(const QString& categoryStr);

signals:
    /**
     * @brief 错误报告信号
     * @param errorInfo 错误信息
     */
    void errorReported(const ErrorInfo& errorInfo);

    /**
     * @brief 错误恢复信号
     * @param errorId 错误ID
     * @param recoveryDetails 恢复详情
     */
    void errorRecovered(const QString& errorId, const QString& recoveryDetails);

    /**
     * @brief 严重错误信号
     * @param errorInfo 错误信息
     */
    void criticalErrorOccurred(const ErrorInfo& errorInfo);

    /**
     * @brief 致命错误信号
     * @param errorInfo 错误信息
     */
    void fatalErrorOccurred(const ErrorInfo& errorInfo);

    /**
     * @brief 错误统计更新信号
     * @param statistics 统计信息
     */
    void errorStatisticsUpdated(const QVariantMap& statistics);

private slots:
    /**
     * @brief 自动清理过期错误
     */
    void performAutoCleanup();

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit UtilsErrorHandler(QObject* parent = nullptr);

    /**
     * @brief 生成错误ID
     * @return 唯一错误ID
     */
    QString generateErrorId();

    /**
     * @brief 添加错误到队列
     * @param errorInfo 错误信息
     */
    void addErrorToQueue(const ErrorInfo& errorInfo);

    /**
     * @brief 清理过期错误
     */
    void cleanupExpiredErrors();

    /**
     * @brief 更新错误统计
     */
    void updateStatistics();

    /**
     * @brief 检查是否需要触发严重错误处理
     * @param errorInfo 错误信息
     */
    void checkCriticalErrorThreshold(const ErrorInfo& errorInfo);

private:
    static UtilsErrorHandler* s_instance;   ///< 单例实例
    static QMutex s_mutex;                  ///< 线程安全互斥锁

    QMap<QString, ErrorInfo> m_errors;      ///< 错误信息映射
    QQueue<QString> m_errorQueue;           ///< 错误队列
    QMap<ErrorCategory, RecoveryStrategy> m_recoveryStrategies; ///< 恢复策略映射
    
    int m_maxErrorCount;                    ///< 最大错误数量
    int m_errorIdCounter;                   ///< 错误ID计数器
    
    QTimer* m_cleanupTimer;                 ///< 自动清理定时器
    bool m_autoCleanupEnabled;              ///< 是否启用自动清理
    int m_autoCleanupInterval;              ///< 自动清理间隔
    
    QVariantMap m_statistics;               ///< 错误统计信息

    // 禁用拷贝构造和赋值操作
    Q_DISABLE_COPY(UtilsErrorHandler)
};

#endif // UTILSERRORHANDLER_H