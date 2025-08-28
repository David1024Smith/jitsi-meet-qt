#ifndef JITSIERROR_H
#define JITSIERROR_H

#include <QString>
#include <QDateTime>
#include <QDebug>

/**
 * @brief 错误类型枚举
 */
enum class ErrorType {
    NetworkError,       // 网络连接错误
    InvalidUrl,         // 无效URL格式
    WebEngineError,     // WebEngine相关错误
    ConfigurationError, // 配置文件错误
    ProtocolError,      // 协议处理错误
    ValidationError,    // 数据验证错误
    SystemError,        // 系统级错误
    WebRTCError,        // WebRTC相关错误
    XMPPConnectionError,// XMPP连接错误
    AuthenticationError,// 认证错误
    MediaDeviceError    // 媒体设备错误
};

/**
 * @brief 错误严重程度枚举
 */
enum class ErrorSeverity {
    Info,       // 信息级别
    Warning,    // 警告级别
    Error,      // 错误级别
    Critical    // 严重错误级别
};

/**
 * @brief Jitsi应用程序统一错误类
 * 
 * 提供统一的错误处理机制，包含错误类型、消息、详细信息和时间戳
 */
class JitsiError
{
public:
    /**
     * @brief 构造函数
     * @param type 错误类型
     * @param message 错误消息
     * @param details 详细信息（可选）
     * @param severity 错误严重程度（默认为Error）
     */
    JitsiError(ErrorType type, 
               const QString& message, 
               const QString& details = QString(),
               ErrorSeverity severity = ErrorSeverity::Error);
    
    /**
     * @brief 拷贝构造函数
     */
    JitsiError(const JitsiError& other);
    
    /**
     * @brief 赋值操作符
     */
    JitsiError& operator=(const JitsiError& other);
    
    /**
     * @brief 析构函数
     */
    ~JitsiError() = default;
    
    // Getter方法
    ErrorType type() const { return m_type; }
    ErrorSeverity severity() const { return m_severity; }
    QString message() const { return m_message; }
    QString details() const { return m_details; }
    QDateTime timestamp() const { return m_timestamp; }
    QString errorCode() const { return m_errorCode; }
    
    /**
     * @brief 获取错误类型的字符串表示
     */
    QString typeString() const;
    
    /**
     * @brief 获取错误严重程度的字符串表示
     */
    QString severityString() const;
    
    /**
     * @brief 获取完整的错误描述
     */
    QString toString() const;
    
    /**
     * @brief 获取用于日志记录的格式化字符串
     */
    QString toLogString() const;
    
    /**
     * @brief 获取用于用户显示的友好消息
     */
    QString toUserMessage() const;
    
    /**
     * @brief 检查错误是否可恢复
     */
    bool isRecoverable() const;
    
    /**
     * @brief 设置错误代码
     */
    void setErrorCode(const QString& code) { m_errorCode = code; }
    
    /**
     * @brief 添加上下文信息
     */
    void addContext(const QString& key, const QString& value);
    
    /**
     * @brief 获取上下文信息
     */
    QString getContext(const QString& key) const;
    
    /**
     * @brief 获取所有上下文信息
     */
    QMap<QString, QString> getAllContext() const { return m_context; }
    
    // 静态工厂方法
    static JitsiError networkError(const QString& message, const QString& details = QString());
    static JitsiError invalidUrlError(const QString& url, const QString& reason = QString());
    static JitsiError webEngineError(const QString& message, const QString& details = QString());
    static JitsiError configurationError(const QString& message, const QString& details = QString());
    static JitsiError protocolError(const QString& message, const QString& details = QString());
    static JitsiError validationError(const QString& field, const QString& value, const QString& reason = QString());
    static JitsiError systemError(const QString& message, const QString& details = QString());
    static JitsiError webRTCError(const QString& message, const QString& details = QString());
    static JitsiError xmppConnectionError(const QString& message, const QString& details = QString());
    static JitsiError authenticationError(const QString& message, const QString& details = QString());
    static JitsiError mediaDeviceError(const QString& message, const QString& details = QString());

private:
    ErrorType m_type;
    ErrorSeverity m_severity;
    QString m_message;
    QString m_details;
    QDateTime m_timestamp;
    QString m_errorCode;
    QMap<QString, QString> m_context;
    
    void generateErrorCode();
};

// 调试输出支持
QDebug operator<<(QDebug debug, const JitsiError& error);

#endif // JITSIERROR_H