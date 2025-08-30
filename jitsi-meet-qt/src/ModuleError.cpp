#include "ModuleError.h"
#include <QDebug>
#include <QJsonDocument>
#include <QMetaEnum>

ModuleError::ModuleError(ErrorType type, Severity severity, const QString& message, const QString& moduleName)
    : m_type(type)
    , m_severity(severity)
    , m_message(message)
    , m_moduleName(moduleName)
    , m_errorCode(0)
    , m_timestamp(QDateTime::currentDateTime())
{
}

ModuleError::ModuleError(const ModuleError& other)
    : m_type(other.m_type)
    , m_severity(other.m_severity)
    , m_message(other.m_message)
    , m_moduleName(other.m_moduleName)
    , m_errorCode(other.m_errorCode)
    , m_timestamp(other.m_timestamp)
    , m_details(other.m_details)
    , m_context(other.m_context)
    , m_stackTrace(other.m_stackTrace)
{
}

ModuleError& ModuleError::operator=(const ModuleError& other)
{
    if (this != &other) {
        m_type = other.m_type;
        m_severity = other.m_severity;
        m_message = other.m_message;
        m_moduleName = other.m_moduleName;
        m_errorCode = other.m_errorCode;
        m_timestamp = other.m_timestamp;
        m_details = other.m_details;
        m_context = other.m_context;
        m_stackTrace = other.m_stackTrace;
    }
    return *this;
}

ModuleError::~ModuleError()
{
}

ModuleError::ErrorType ModuleError::type() const
{
    return m_type;
}

ModuleError::Severity ModuleError::severity() const
{
    return m_severity;
}

QString ModuleError::message() const
{
    return m_message;
}

QString ModuleError::moduleName() const
{
    return m_moduleName;
}

int ModuleError::errorCode() const
{
    return m_errorCode;
}

void ModuleError::setErrorCode(int code)
{
    m_errorCode = code;
}

QDateTime ModuleError::timestamp() const
{
    return m_timestamp;
}

QString ModuleError::details() const
{
    return m_details;
}

void ModuleError::setDetails(const QString& details)
{
    m_details = details;
}

QVariantMap ModuleError::context() const
{
    return m_context;
}

void ModuleError::setContext(const QVariantMap& context)
{
    m_context = context;
}

void ModuleError::addContext(const QString& key, const QVariant& value)
{
    m_context[key] = value;
}

QStringList ModuleError::stackTrace() const
{
    return m_stackTrace;
}

void ModuleError::setStackTrace(const QStringList& stackTrace)
{
    m_stackTrace = stackTrace;
}

QString ModuleError::toString() const
{
    QStringList parts;
    
    parts << QString("[%1]").arg(severityName(m_severity));
    parts << QString("[%1]").arg(errorTypeName(m_type));
    
    if (!m_moduleName.isEmpty()) {
        parts << QString("[%1]").arg(m_moduleName);
    }
    
    parts << m_message;
    
    if (m_errorCode != 0) {
        parts << QString("(Code: %1)").arg(m_errorCode);
    }
    
    if (!m_details.isEmpty()) {
        parts << QString("Details: %1").arg(m_details);
    }
    
    return parts.join(" ");
}

QJsonObject ModuleError::toJson() const
{
    QJsonObject json;
    
    json["type"] = errorTypeName(m_type);
    json["severity"] = severityName(m_severity);
    json["message"] = m_message;
    json["moduleName"] = m_moduleName;
    json["errorCode"] = m_errorCode;
    json["timestamp"] = m_timestamp.toString(Qt::ISODate);
    json["details"] = m_details;
    
    if (!m_context.isEmpty()) {
        json["context"] = QJsonObject::fromVariantMap(m_context);
    }
    
    if (!m_stackTrace.isEmpty()) {
        QJsonArray stackArray;
        for (const QString& frame : m_stackTrace) {
            stackArray.append(frame);
        }
        json["stackTrace"] = stackArray;
    }
    
    return json;
}

ModuleError ModuleError::fromJson(const QJsonObject& json)
{
    ErrorType type = parseErrorType(json["type"].toString());
    Severity severity = parseSeverity(json["severity"].toString());
    QString message = json["message"].toString();
    QString moduleName = json["moduleName"].toString();
    
    ModuleError error(type, severity, message, moduleName);
    
    error.setErrorCode(json["errorCode"].toInt());
    error.setDetails(json["details"].toString());
    
    QString timestampStr = json["timestamp"].toString();
    if (!timestampStr.isEmpty()) {
        error.m_timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
    }
    
    if (json.contains("context")) {
        QJsonObject contextObj = json["context"].toObject();
        error.setContext(contextObj.toVariantMap());
    }
    
    if (json.contains("stackTrace")) {
        QJsonArray stackArray = json["stackTrace"].toArray();
        QStringList stackTrace;
        for (const QJsonValue& value : stackArray) {
            stackTrace.append(value.toString());
        }
        error.setStackTrace(stackTrace);
    }
    
    return error;
}

QVariantMap ModuleError::toVariantMap() const
{
    QVariantMap map;
    
    map["type"] = static_cast<int>(m_type);
    map["severity"] = static_cast<int>(m_severity);
    map["message"] = m_message;
    map["moduleName"] = m_moduleName;
    map["errorCode"] = m_errorCode;
    map["timestamp"] = m_timestamp;
    map["details"] = m_details;
    map["context"] = m_context;
    map["stackTrace"] = m_stackTrace;
    
    return map;
}

ModuleError ModuleError::fromVariantMap(const QVariantMap& map)
{
    ErrorType type = static_cast<ErrorType>(map["type"].toInt());
    Severity severity = static_cast<Severity>(map["severity"].toInt());
    QString message = map["message"].toString();
    QString moduleName = map["moduleName"].toString();
    
    ModuleError error(type, severity, message, moduleName);
    
    error.setErrorCode(map["errorCode"].toInt());
    error.setDetails(map["details"].toString());
    error.m_timestamp = map["timestamp"].toDateTime();
    error.setContext(map["context"].toMap());
    error.setStackTrace(map["stackTrace"].toStringList());
    
    return error;
}

QString ModuleError::errorTypeName(ErrorType type)
{
    switch (type) {
    case InitializationError: return "InitializationError";
    case ConfigurationError: return "ConfigurationError";
    case RuntimeError: return "RuntimeError";
    case ResourceError: return "ResourceError";
    case NetworkError: return "NetworkError";
    case PermissionError: return "PermissionError";
    case DependencyError: return "DependencyError";
    case ValidationError: return "ValidationError";
    case TimeoutError: return "TimeoutError";
    case UnknownError: return "UnknownError";
    default: return "UnknownError";
    }
}

QString ModuleError::severityName(Severity severity)
{
    switch (severity) {
    case Info: return "Info";
    case Warning: return "Warning";
    case Error: return "Error";
    case Critical: return "Critical";
    case Fatal: return "Fatal";
    default: return "Unknown";
    }
}

ModuleError::ErrorType ModuleError::parseErrorType(const QString& typeName)
{
    if (typeName == "InitializationError") return InitializationError;
    if (typeName == "ConfigurationError") return ConfigurationError;
    if (typeName == "RuntimeError") return RuntimeError;
    if (typeName == "ResourceError") return ResourceError;
    if (typeName == "NetworkError") return NetworkError;
    if (typeName == "PermissionError") return PermissionError;
    if (typeName == "DependencyError") return DependencyError;
    if (typeName == "ValidationError") return ValidationError;
    if (typeName == "TimeoutError") return TimeoutError;
    return UnknownError;
}

ModuleError::Severity ModuleError::parseSeverity(const QString& severityName)
{
    if (severityName == "Info") return Info;
    if (severityName == "Warning") return Warning;
    if (severityName == "Error") return Error;
    if (severityName == "Critical") return Critical;
    if (severityName == "Fatal") return Fatal;
    return Error;
}

bool ModuleError::operator==(const ModuleError& other) const
{
    return m_type == other.m_type &&
           m_severity == other.m_severity &&
           m_message == other.m_message &&
           m_moduleName == other.m_moduleName &&
           m_errorCode == other.m_errorCode;
}

bool ModuleError::operator!=(const ModuleError& other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug debug, const ModuleError& error)
{
    debug.nospace() << "ModuleError("
                   << "type=" << ModuleError::errorTypeName(error.type())
                   << ", severity=" << ModuleError::severityName(error.severity())
                   << ", module=" << error.moduleName()
                   << ", message=" << error.message()
                   << ")";
    return debug;
}

// ErrorRecoveryStrategy 实现

ErrorRecoveryStrategy::ErrorRecoveryStrategy(QObject *parent)
    : QObject(parent)
{
}

ErrorRecoveryStrategy::~ErrorRecoveryStrategy()
{
}

QString ErrorRecoveryStrategy::recoveryActionName(RecoveryAction action)
{
    switch (action) {
    case Ignore: return "Ignore";
    case Retry: return "Retry";
    case Restart: return "Restart";
    case Fallback: return "Fallback";
    case Shutdown: return "Shutdown";
    case Escalate: return "Escalate";
    default: return "Unknown";
    }
}

// DefaultErrorRecoveryStrategy 实现

DefaultErrorRecoveryStrategy::DefaultErrorRecoveryStrategy(QObject *parent)
    : ErrorRecoveryStrategy(parent)
    , m_maxRetryCount(3)
{
}

ErrorRecoveryStrategy::RecoveryAction DefaultErrorRecoveryStrategy::suggestAction(const ModuleError& error)
{
    // 根据错误类型和严重程度建议恢复动作
    
    switch (error.severity()) {
    case ModuleError::Fatal:
        return Shutdown;
        
    case ModuleError::Critical:
        if (error.type() == ModuleError::DependencyError) {
            return Restart;
        }
        return Escalate;
        
    case ModuleError::Error:
        if (error.type() == ModuleError::NetworkError || 
            error.type() == ModuleError::TimeoutError) {
            return Retry;
        }
        if (error.type() == ModuleError::ConfigurationError) {
            return Fallback;
        }
        return Restart;
        
    case ModuleError::Warning:
        return Fallback;
        
    case ModuleError::Info:
    default:
        return Ignore;
    }
}

bool DefaultErrorRecoveryStrategy::executeRecovery(const ModuleError& error)
{
    RecoveryAction action = suggestAction(error);
    
    emit recoveryStarted(error, action);
    
    QString moduleName = error.moduleName();
    bool success = false;
    
    switch (action) {
    case Ignore:
        qDebug() << "DefaultErrorRecoveryStrategy: Ignoring error in" << moduleName;
        success = true;
        break;
        
    case Retry:
        {
            int retryCount = m_retryCounts.value(moduleName, 0);
            if (retryCount < m_maxRetryCount) {
                m_retryCounts[moduleName] = retryCount + 1;
                qDebug() << "DefaultErrorRecoveryStrategy: Retrying operation in" << moduleName 
                        << "(attempt" << (retryCount + 1) << "of" << m_maxRetryCount << ")";
                success = true;
            } else {
                qWarning() << "DefaultErrorRecoveryStrategy: Max retry count reached for" << moduleName;
                success = false;
            }
        }
        break;
        
    case Restart:
        qDebug() << "DefaultErrorRecoveryStrategy: Restarting module" << moduleName;
        // 这里应该调用模块管理器重启模块
        success = true;
        break;
        
    case Fallback:
        qDebug() << "DefaultErrorRecoveryStrategy: Using fallback for" << moduleName;
        success = true;
        break;
        
    case Shutdown:
        qWarning() << "DefaultErrorRecoveryStrategy: Shutting down module" << moduleName;
        success = true;
        break;
        
    case Escalate:
        qWarning() << "DefaultErrorRecoveryStrategy: Escalating error in" << moduleName;
        success = false;
        break;
    }
    
    if (success && action == Retry) {
        // 重置重试计数
        m_retryCounts.remove(moduleName);
    }
    
    emit recoveryCompleted(error, action, success);
    
    if (!success) {
        emit recoveryFailed(error, action, "Recovery action failed");
    }
    
    return success;
}

bool DefaultErrorRecoveryStrategy::canRecover(const ModuleError& error) const
{
    // 检查是否可以恢复
    switch (error.severity()) {
    case ModuleError::Fatal:
        return false; // 致命错误通常无法恢复
        
    case ModuleError::Critical:
        return error.type() != ModuleError::DependencyError; // 依赖错误难以恢复
        
    default:
        return true;
    }
}

QString DefaultErrorRecoveryStrategy::strategyName() const
{
    return "DefaultErrorRecoveryStrategy";
}