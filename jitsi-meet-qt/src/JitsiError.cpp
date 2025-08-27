#include "JitsiError.h"
#include <QCryptographicHash>
#include <QRandomGenerator>

JitsiError::JitsiError(ErrorType type, 
                       const QString& message, 
                       const QString& details,
                       ErrorSeverity severity)
    : m_type(type)
    , m_severity(severity)
    , m_message(message)
    , m_details(details)
    , m_timestamp(QDateTime::currentDateTime())
{
    generateErrorCode();
}

JitsiError::JitsiError(const JitsiError& other)
    : m_type(other.m_type)
    , m_severity(other.m_severity)
    , m_message(other.m_message)
    , m_details(other.m_details)
    , m_timestamp(other.m_timestamp)
    , m_errorCode(other.m_errorCode)
    , m_context(other.m_context)
{
}

JitsiError& JitsiError::operator=(const JitsiError& other)
{
    if (this != &other) {
        m_type = other.m_type;
        m_severity = other.m_severity;
        m_message = other.m_message;
        m_details = other.m_details;
        m_timestamp = other.m_timestamp;
        m_errorCode = other.m_errorCode;
        m_context = other.m_context;
    }
    return *this;
}

QString JitsiError::typeString() const
{
    switch (m_type) {
    case ErrorType::NetworkError:
        return "NetworkError";
    case ErrorType::InvalidUrl:
        return "InvalidUrl";
    case ErrorType::WebEngineError:
        return "WebEngineError";
    case ErrorType::ConfigurationError:
        return "ConfigurationError";
    case ErrorType::ProtocolError:
        return "ProtocolError";
    case ErrorType::ValidationError:
        return "ValidationError";
    case ErrorType::SystemError:
        return "SystemError";
    default:
        return "UnknownError";
    }
}

QString JitsiError::severityString() const
{
    switch (m_severity) {
    case ErrorSeverity::Info:
        return "Info";
    case ErrorSeverity::Warning:
        return "Warning";
    case ErrorSeverity::Error:
        return "Error";
    case ErrorSeverity::Critical:
        return "Critical";
    default:
        return "Unknown";
    }
}

QString JitsiError::toString() const
{
    QString result = QString("[%1] %2: %3")
                    .arg(severityString())
                    .arg(typeString())
                    .arg(m_message);
    
    if (!m_details.isEmpty()) {
        result += QString(" - %1").arg(m_details);
    }
    
    if (!m_errorCode.isEmpty()) {
        result += QString(" (Code: %1)").arg(m_errorCode);
    }
    
    return result;
}

QString JitsiError::toLogString() const
{
    QString result = QString("%1 [%2] %3: %4")
                    .arg(m_timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz"))
                    .arg(severityString())
                    .arg(typeString())
                    .arg(m_message);
    
    if (!m_details.isEmpty()) {
        result += QString(" | Details: %1").arg(m_details);
    }
    
    if (!m_errorCode.isEmpty()) {
        result += QString(" | Code: %1").arg(m_errorCode);
    }
    
    // 添加上下文信息
    if (!m_context.isEmpty()) {
        QStringList contextList;
        for (auto it = m_context.constBegin(); it != m_context.constEnd(); ++it) {
            contextList << QString("%1=%2").arg(it.key(), it.value());
        }
        result += QString(" | Context: %1").arg(contextList.join(", "));
    }
    
    return result;
}

QString JitsiError::toUserMessage() const
{
    // 根据错误类型返回用户友好的消息
    switch (m_type) {
    case ErrorType::NetworkError:
        return QString("网络连接失败，请检查您的网络连接后重试。");
    case ErrorType::InvalidUrl:
        return QString("输入的会议室地址格式不正确，请检查后重新输入。");
    case ErrorType::WebEngineError:
        return QString("会议页面加载失败，请稍后重试。");
    case ErrorType::ConfigurationError:
        return QString("应用程序配置出现问题，将使用默认设置。");
    case ErrorType::ProtocolError:
        return QString("无法处理会议链接，请直接在应用中输入会议室地址。");
    case ErrorType::ValidationError:
        return QString("输入的信息不符合要求，请检查后重新输入。");
    case ErrorType::SystemError:
        return QString("系统出现错误，请重启应用程序后重试。");
    default:
        return m_message;
    }
}

bool JitsiError::isRecoverable() const
{
    switch (m_type) {
    case ErrorType::NetworkError:
    case ErrorType::InvalidUrl:
    case ErrorType::ValidationError:
        return true;
    case ErrorType::WebEngineError:
        return m_severity != ErrorSeverity::Critical;
    case ErrorType::ConfigurationError:
    case ErrorType::ProtocolError:
        return m_severity == ErrorSeverity::Warning || m_severity == ErrorSeverity::Info;
    case ErrorType::SystemError:
        return m_severity != ErrorSeverity::Critical;
    default:
        return false;
    }
}

void JitsiError::addContext(const QString& key, const QString& value)
{
    m_context[key] = value;
}

QString JitsiError::getContext(const QString& key) const
{
    return m_context.value(key);
}

void JitsiError::generateErrorCode()
{
    // 生成基于时间戳和错误类型的唯一错误代码
    QString source = QString("%1_%2_%3")
                    .arg(static_cast<int>(m_type))
                    .arg(m_timestamp.toMSecsSinceEpoch())
                    .arg(QRandomGenerator::global()->bounded(1000, 9999));
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(source.toUtf8());
    m_errorCode = hash.result().toHex().left(8).toUpper();
}

// 静态工厂方法实现
JitsiError JitsiError::networkError(const QString& message, const QString& details)
{
    JitsiError error(ErrorType::NetworkError, message, details, ErrorSeverity::Error);
    error.addContext("category", "network");
    return error;
}

JitsiError JitsiError::invalidUrlError(const QString& url, const QString& reason)
{
    QString message = QString("Invalid URL: %1").arg(url);
    JitsiError error(ErrorType::InvalidUrl, message, reason, ErrorSeverity::Warning);
    error.addContext("url", url);
    error.addContext("category", "validation");
    return error;
}

JitsiError JitsiError::webEngineError(const QString& message, const QString& details)
{
    JitsiError error(ErrorType::WebEngineError, message, details, ErrorSeverity::Error);
    error.addContext("category", "webengine");
    return error;
}

JitsiError JitsiError::configurationError(const QString& message, const QString& details)
{
    JitsiError error(ErrorType::ConfigurationError, message, details, ErrorSeverity::Warning);
    error.addContext("category", "configuration");
    return error;
}

JitsiError JitsiError::protocolError(const QString& message, const QString& details)
{
    JitsiError error(ErrorType::ProtocolError, message, details, ErrorSeverity::Warning);
    error.addContext("category", "protocol");
    return error;
}

JitsiError JitsiError::validationError(const QString& field, const QString& value, const QString& reason)
{
    QString message = QString("Validation failed for field '%1' with value '%2'").arg(field, value);
    JitsiError error(ErrorType::ValidationError, message, reason, ErrorSeverity::Warning);
    error.addContext("field", field);
    error.addContext("value", value);
    error.addContext("category", "validation");
    return error;
}

JitsiError JitsiError::systemError(const QString& message, const QString& details)
{
    JitsiError error(ErrorType::SystemError, message, details, ErrorSeverity::Critical);
    error.addContext("category", "system");
    return error;
}

// 调试输出支持
QDebug operator<<(QDebug debug, const JitsiError& error)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "JitsiError(" 
                   << error.typeString() << ", "
                   << error.severityString() << ", "
                   << error.message() << ")";
    return debug;
}