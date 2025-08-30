#ifndef MODULEERROR_H
#define MODULEERROR_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QVariantMap>
#include <QDebug>

/**
 * @brief 模块错误基类 - 统一的错误处理系统
 * 
 * 该类定义了模块化系统中的统一错误处理机制，包括：
 * - 错误分类和严重程度定义
 * - 错误信息的结构化存储
 * - 错误的序列化和反序列化
 * - 错误恢复策略接口
 */
class ModuleError
{
public:
    /**
     * @brief 错误类型枚举
     */
    enum ErrorType {
        InitializationError,    ///< 初始化错误
        ConfigurationError,     ///< 配置错误
        RuntimeError,          ///< 运行时错误
        ResourceError,         ///< 资源错误
        NetworkError,          ///< 网络错误
        PermissionError,       ///< 权限错误
        DependencyError,       ///< 依赖错误
        ValidationError,       ///< 验证错误
        TimeoutError,          ///< 超时错误
        UnknownError           ///< 未知错误
    };

    /**
     * @brief 错误严重程度枚举
     */
    enum Severity {
        Info,       ///< 信息
        Warning,    ///< 警告
        Error,      ///< 错误
        Critical,   ///< 严重错误
        Fatal       ///< 致命错误
    };

    /**
     * @brief 构造函数
     * @param type 错误类型
     * @param severity 严重程度
     * @param message 错误消息
     * @param moduleName 模块名称
     */
    ModuleError(ErrorType type, Severity severity, const QString& message, const QString& moduleName = QString());

    /**
     * @brief 拷贝构造函数
     */
    ModuleError(const ModuleError& other);

    /**
     * @brief 赋值操作符
     */
    ModuleError& operator=(const ModuleError& other);

    /**
     * @brief 析构函数
     */
    virtual ~ModuleError();

    /**
     * @brief 获取错误类型
     * @return 错误类型
     */
    ErrorType type() const;

    /**
     * @brief 获取严重程度
     * @return 严重程度
     */
    Severity severity() const;

    /**
     * @brief 获取错误消息
     * @return 错误消息
     */
    QString message() const;

    /**
     * @brief 获取模块名称
     * @return 模块名称
     */
    QString moduleName() const;

    /**
     * @brief 获取错误代码
     * @return 错误代码
     */
    int errorCode() const;

    /**
     * @brief 设置错误代码
     * @param code 错误代码
     */
    void setErrorCode(int code);

    /**
     * @brief 获取时间戳
     * @return 时间戳
     */
    QDateTime timestamp() const;

    /**
     * @brief 获取详细信息
     * @return 详细信息
     */
    QString details() const;

    /**
     * @brief 设置详细信息
     * @param details 详细信息
     */
    void setDetails(const QString& details);

    /**
     * @brief 获取上下文信息
     * @return 上下文信息
     */
    QVariantMap context() const;

    /**
     * @brief 设置上下文信息
     * @param context 上下文信息
     */
    void setContext(const QVariantMap& context);

    /**
     * @brief 添加上下文信息
     * @param key 键
     * @param value 值
     */
    void addContext(const QString& key, const QVariant& value);

    /**
     * @brief 获取堆栈跟踪
     * @return 堆栈跟踪
     */
    QStringList stackTrace() const;

    /**
     * @brief 设置堆栈跟踪
     * @param stackTrace 堆栈跟踪
     */
    void setStackTrace(const QStringList& stackTrace);

    /**
     * @brief 转换为字符串
     * @return 错误字符串表示
     */
    virtual QString toString() const;

    /**
     * @brief 转换为JSON对象
     * @return JSON对象
     */
    virtual QJsonObject toJson() const;

    /**
     * @brief 从JSON对象创建错误
     * @param json JSON对象
     * @return 模块错误对象
     */
    static ModuleError fromJson(const QJsonObject& json);

    /**
     * @brief 转换为QVariantMap
     * @return QVariantMap对象
     */
    virtual QVariantMap toVariantMap() const;

    /**
     * @brief 从QVariantMap创建错误
     * @param map QVariantMap对象
     * @return 模块错误对象
     */
    static ModuleError fromVariantMap(const QVariantMap& map);

    /**
     * @brief 获取错误类型名称
     * @param type 错误类型
     * @return 类型名称
     */
    static QString errorTypeName(ErrorType type);

    /**
     * @brief 获取严重程度名称
     * @param severity 严重程度
     * @return 严重程度名称
     */
    static QString severityName(Severity severity);

    /**
     * @brief 从字符串解析错误类型
     * @param typeName 类型名称
     * @return 错误类型
     */
    static ErrorType parseErrorType(const QString& typeName);

    /**
     * @brief 从字符串解析严重程度
     * @param severityName 严重程度名称
     * @return 严重程度
     */
    static Severity parseSeverity(const QString& severityName);

    /**
     * @brief 比较操作符
     */
    bool operator==(const ModuleError& other) const;
    bool operator!=(const ModuleError& other) const;

private:
    ErrorType m_type;               ///< 错误类型
    Severity m_severity;            ///< 严重程度
    QString m_message;              ///< 错误消息
    QString m_moduleName;           ///< 模块名称
    int m_errorCode;                ///< 错误代码
    QDateTime m_timestamp;          ///< 时间戳
    QString m_details;              ///< 详细信息
    QVariantMap m_context;          ///< 上下文信息
    QStringList m_stackTrace;       ///< 堆栈跟踪
};

/**
 * @brief 错误恢复策略接口
 */
class ErrorRecoveryStrategy : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 恢复动作枚举
     */
    enum RecoveryAction {
        Ignore,         ///< 忽略错误
        Retry,          ///< 重试操作
        Restart,        ///< 重启模块
        Fallback,       ///< 使用备用方案
        Shutdown,       ///< 关闭模块
        Escalate        ///< 上报错误
    };

    explicit ErrorRecoveryStrategy(QObject *parent = nullptr);
    virtual ~ErrorRecoveryStrategy();

    /**
     * @brief 建议恢复动作
     * @param error 模块错误
     * @return 建议的恢复动作
     */
    virtual RecoveryAction suggestAction(const ModuleError& error) = 0;

    /**
     * @brief 执行恢复操作
     * @param error 模块错误
     * @return 是否恢复成功
     */
    virtual bool executeRecovery(const ModuleError& error) = 0;

    /**
     * @brief 检查是否可以恢复
     * @param error 模块错误
     * @return 是否可以恢复
     */
    virtual bool canRecover(const ModuleError& error) const = 0;

    /**
     * @brief 获取恢复策略名称
     * @return 策略名称
     */
    virtual QString strategyName() const = 0;

    /**
     * @brief 获取恢复动作名称
     * @param action 恢复动作
     * @return 动作名称
     */
    static QString recoveryActionName(RecoveryAction action);

signals:
    /**
     * @brief 恢复开始信号
     * @param error 模块错误
     * @param action 恢复动作
     */
    void recoveryStarted(const ModuleError& error, RecoveryAction action);

    /**
     * @brief 恢复完成信号
     * @param error 模块错误
     * @param action 恢复动作
     * @param success 是否成功
     */
    void recoveryCompleted(const ModuleError& error, RecoveryAction action, bool success);

    /**
     * @brief 恢复失败信号
     * @param error 模块错误
     * @param action 恢复动作
     * @param reason 失败原因
     */
    void recoveryFailed(const ModuleError& error, RecoveryAction action, const QString& reason);
};

/**
 * @brief 默认错误恢复策略
 */
class DefaultErrorRecoveryStrategy : public ErrorRecoveryStrategy
{
    Q_OBJECT

public:
    explicit DefaultErrorRecoveryStrategy(QObject *parent = nullptr);

    // ErrorRecoveryStrategy接口实现
    RecoveryAction suggestAction(const ModuleError& error) override;
    bool executeRecovery(const ModuleError& error) override;
    bool canRecover(const ModuleError& error) const override;
    QString strategyName() const override;

private:
    int m_maxRetryCount;            ///< 最大重试次数
    QMap<QString, int> m_retryCounts; ///< 重试计数
};

// 调试输出支持
QDebug operator<<(QDebug debug, const ModuleError& error);

Q_DECLARE_METATYPE(ModuleError)
Q_DECLARE_METATYPE(ModuleError::ErrorType)
Q_DECLARE_METATYPE(ModuleError::Severity)
Q_DECLARE_METATYPE(ErrorRecoveryStrategy::RecoveryAction)

#endif // MODULEERROR_H