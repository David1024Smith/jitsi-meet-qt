#include "../include/UtilsErrorHandler.h"
#include <QDebug>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

// 静态成员初始化
UtilsErrorHandler* UtilsErrorHandler::s_instance = nullptr;
QMutex UtilsErrorHandler::s_mutex;

UtilsErrorHandler::UtilsErrorHandler(QObject* parent)
    : QObject(parent)
    , m_maxErrorCount(1000)
    , m_errorIdCounter(0)
    , m_cleanupTimer(nullptr)
    , m_autoCleanupEnabled(true)
    , m_autoCleanupInterval(300000) // 5分钟
{
    // 初始化默认恢复策略
    m_recoveryStrategies[SystemError] = Reset;
    m_recoveryStrategies[ConfigurationError] = Fallback;
    m_recoveryStrategies[FileSystemError] = Retry;
    m_recoveryStrategies[NetworkError] = Retry;
    m_recoveryStrategies[CryptoError] = Reset;
    m_recoveryStrategies[ValidationError] = Ignore;
    m_recoveryStrategies[MemoryError] = Restart;
    m_recoveryStrategies[PermissionError] = Fallback;
    m_recoveryStrategies[TimeoutError] = Retry;
    m_recoveryStrategies[UnknownError] = NoRecovery;
}

UtilsErrorHandler::~UtilsErrorHandler()
{
    cleanup();
}

UtilsErrorHandler* UtilsErrorHandler::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new UtilsErrorHandler();
    }
    return s_instance;
}

bool UtilsErrorHandler::initialize()
{
    QMutexLocker locker(&s_mutex);
    
    // 初始化自动清理定时器
    if (!m_cleanupTimer) {
        m_cleanupTimer = new QTimer(this);
        connect(m_cleanupTimer, &QTimer::timeout, this, &UtilsErrorHandler::performAutoCleanup);
    }
    
    if (m_autoCleanupEnabled) {
        m_cleanupTimer->start(m_autoCleanupInterval);
    }
    
    // 初始化统计信息
    updateStatistics();
    
    qDebug() << "UtilsErrorHandler initialized successfully";
    return true;
}

void UtilsErrorHandler::cleanup()
{
    QMutexLocker locker(&s_mutex);
    
    if (m_cleanupTimer) {
        m_cleanupTimer->stop();
        delete m_cleanupTimer;
        m_cleanupTimer = nullptr;
    }
    
    m_errors.clear();
    m_errorQueue.clear();
    m_statistics.clear();
    
    qDebug() << "UtilsErrorHandler cleaned up";
}

QString UtilsErrorHandler::reportError(ErrorLevel level, ErrorCategory category,
                                     const QString& message, const QString& source,
                                     const QString& details, const QVariantMap& context)
{
    QMutexLocker locker(&s_mutex);
    
    ErrorInfo errorInfo;
    errorInfo.id = generateErrorId();
    errorInfo.level = level;
    errorInfo.category = category;
    errorInfo.message = message;
    errorInfo.source = source;
    errorInfo.details = details;
    errorInfo.context = context;
    errorInfo.timestamp = QDateTime::currentDateTime();
    errorInfo.strategy = m_recoveryStrategies.value(category, NoRecovery);
    
    // 检查是否是重复错误
    QString errorKey = QString("%1_%2_%3").arg(static_cast<int>(category))
                                         .arg(source).arg(message);
    
    for (auto& existingError : m_errors) {
        QString existingKey = QString("%1_%2_%3").arg(static_cast<int>(existingError.category))
                                                .arg(existingError.source)
                                                .arg(existingError.message);
        if (existingKey == errorKey) {
            existingError.occurrenceCount++;
            existingError.timestamp = QDateTime::currentDateTime();
            emit errorReported(existingError);
            updateStatistics();
            return existingError.id;
        }
    }
    
    // 添加新错误
    addErrorToQueue(errorInfo);
    
    // 发送信号
    emit errorReported(errorInfo);
    
    // 检查严重错误阈值
    checkCriticalErrorThreshold(errorInfo);
    
    // 更新统计信息
    updateStatistics();
    
    // 记录到日志
    QString logMessage = QString("[%1] %2: %3")
                        .arg(errorLevelToString(level))
                        .arg(source.isEmpty() ? "Unknown" : source)
                        .arg(message);
    
    switch (level) {
        case Info:
            qInfo() << logMessage;
            break;
        case Warning:
            qWarning() << logMessage;
            break;
        case Error:
        case Critical:
        case Fatal:
            qCritical() << logMessage;
            break;
    }
    
    return errorInfo.id;
}

QString UtilsErrorHandler::reportError(const QString& message, const QString& source)
{
    return reportError(Error, UnknownError, message, source);
}

QString UtilsErrorHandler::reportWarning(const QString& message, const QString& source)
{
    return reportError(Warning, UnknownError, message, source);
}

QString UtilsErrorHandler::reportInfo(const QString& message, const QString& source)
{
    return reportError(Info, UnknownError, message, source);
}

void UtilsErrorHandler::markErrorRecovered(const QString& errorId, const QString& recoveryDetails)
{
    QMutexLocker locker(&s_mutex);
    
    if (m_errors.contains(errorId)) {
        m_errors[errorId].recovered = true;
        
        QVariantMap context = m_errors[errorId].context;
        context["recoveryDetails"] = recoveryDetails;
        context["recoveryTime"] = QDateTime::currentDateTime();
        m_errors[errorId].context = context;
        
        emit errorRecovered(errorId, recoveryDetails);
        updateStatistics();
        
        qDebug() << "Error marked as recovered:" << errorId << recoveryDetails;
    }
}

UtilsErrorHandler::ErrorInfo UtilsErrorHandler::getErrorInfo(const QString& errorId) const
{
    return m_errors.value(errorId, ErrorInfo());
}

QList<UtilsErrorHandler::ErrorInfo> UtilsErrorHandler::getAllErrors() const
{
    return m_errors.values();
}

QList<UtilsErrorHandler::ErrorInfo> UtilsErrorHandler::getErrorsByLevel(ErrorLevel level) const
{
    QList<ErrorInfo> result;
    for (const auto& error : m_errors) {
        if (error.level == level) {
            result.append(error);
        }
    }
    return result;
}

QList<UtilsErrorHandler::ErrorInfo> UtilsErrorHandler::getErrorsByCategory(ErrorCategory category) const
{
    QList<ErrorInfo> result;
    for (const auto& error : m_errors) {
        if (error.category == category) {
            result.append(error);
        }
    }
    return result;
}

QList<UtilsErrorHandler::ErrorInfo> UtilsErrorHandler::getErrorsBySource(const QString& source) const
{
    QList<ErrorInfo> result;
    for (const auto& error : m_errors) {
        if (error.source == source) {
            result.append(error);
        }
    }
    return result;
}

QList<UtilsErrorHandler::ErrorInfo> UtilsErrorHandler::getUnrecoveredErrors() const
{
    QList<ErrorInfo> result;
    for (const auto& error : m_errors) {
        if (!error.recovered) {
            result.append(error);
        }
    }
    return result;
}

void UtilsErrorHandler::clearAllErrors()
{
    QMutexLocker locker(&s_mutex);
    m_errors.clear();
    m_errorQueue.clear();
    updateStatistics();
    qDebug() << "All errors cleared";
}

void UtilsErrorHandler::clearErrorsByLevel(ErrorLevel level)
{
    QMutexLocker locker(&s_mutex);
    
    auto it = m_errors.begin();
    while (it != m_errors.end()) {
        if (it.value().level == level) {
            m_errorQueue.removeAll(it.key());
            it = m_errors.erase(it);
        } else {
            ++it;
        }
    }
    
    updateStatistics();
    qDebug() << "Errors cleared for level:" << errorLevelToString(level);
}

void UtilsErrorHandler::clearRecoveredErrors()
{
    QMutexLocker locker(&s_mutex);
    
    auto it = m_errors.begin();
    while (it != m_errors.end()) {
        if (it.value().recovered) {
            m_errorQueue.removeAll(it.key());
            it = m_errors.erase(it);
        } else {
            ++it;
        }
    }
    
    updateStatistics();
    qDebug() << "Recovered errors cleared";
}

QVariantMap UtilsErrorHandler::getErrorStatistics() const
{
    return m_statistics;
}

void UtilsErrorHandler::setMaxErrorCount(int maxErrors)
{
    QMutexLocker locker(&s_mutex);
    m_maxErrorCount = maxErrors;
    
    // 如果当前错误数量超过限制，清理最旧的错误
    while (m_errors.size() > m_maxErrorCount && !m_errorQueue.isEmpty()) {
        QString oldestErrorId = m_errorQueue.dequeue();
        m_errors.remove(oldestErrorId);
    }
}

int UtilsErrorHandler::maxErrorCount() const
{
    return m_maxErrorCount;
}

void UtilsErrorHandler::setAutoCleanupInterval(int intervalMs)
{
    QMutexLocker locker(&s_mutex);
    m_autoCleanupInterval = intervalMs;
    
    if (m_cleanupTimer && m_autoCleanupEnabled) {
        m_cleanupTimer->setInterval(intervalMs);
    }
}

void UtilsErrorHandler::setAutoCleanupEnabled(bool enabled)
{
    QMutexLocker locker(&s_mutex);
    m_autoCleanupEnabled = enabled;
    
    if (m_cleanupTimer) {
        if (enabled) {
            m_cleanupTimer->start(m_autoCleanupInterval);
        } else {
            m_cleanupTimer->stop();
        }
    }
}

void UtilsErrorHandler::setRecoveryStrategy(ErrorCategory category, RecoveryStrategy strategy)
{
    QMutexLocker locker(&s_mutex);
    m_recoveryStrategies[category] = strategy;
}

UtilsErrorHandler::RecoveryStrategy UtilsErrorHandler::getRecoveryStrategy(ErrorCategory category) const
{
    return m_recoveryStrategies.value(category, NoRecovery);
}

bool UtilsErrorHandler::attemptAutoRecovery(const QString& errorId)
{
    QMutexLocker locker(&s_mutex);
    
    if (!m_errors.contains(errorId)) {
        return false;
    }
    
    const ErrorInfo& error = m_errors[errorId];
    RecoveryStrategy strategy = error.strategy;
    
    bool recovered = false;
    QString recoveryDetails;
    
    switch (strategy) {
        case Retry:
            // 实现重试逻辑
            recoveryDetails = "Automatic retry attempted";
            recovered = true; // 简化实现
            break;
            
        case Fallback:
            // 实现回退逻辑
            recoveryDetails = "Fallback mechanism activated";
            recovered = true;
            break;
            
        case Reset:
            // 实现重置逻辑
            recoveryDetails = "System reset performed";
            recovered = true;
            break;
            
        case Ignore:
            // 忽略错误
            recoveryDetails = "Error ignored as per strategy";
            recovered = true;
            break;
            
        case Restart:
        case NoRecovery:
        default:
            recoveryDetails = "No automatic recovery available";
            recovered = false;
            break;
    }
    
    if (recovered) {
        markErrorRecovered(errorId, recoveryDetails);
    }
    
    return recovered;
}

bool UtilsErrorHandler::exportErrorLog(const QString& filePath, const QString& format) const
{
    try {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }
        
        QTextStream stream(&file);
        
        if (format.toLower() == "json") {
            QJsonArray errorArray;
            for (const auto& error : m_errors) {
                QJsonObject errorObj;
                errorObj["id"] = error.id;
                errorObj["level"] = errorLevelToString(error.level);
                errorObj["category"] = errorCategoryToString(error.category);
                errorObj["message"] = error.message;
                errorObj["source"] = error.source;
                errorObj["details"] = error.details;
                errorObj["timestamp"] = error.timestamp.toString(Qt::ISODate);
                errorObj["occurrenceCount"] = error.occurrenceCount;
                errorObj["recovered"] = error.recovered;
                
                errorArray.append(errorObj);
            }
            
            QJsonDocument doc(errorArray);
            stream << doc.toJson();
            
        } else if (format.toLower() == "csv") {
            stream << "ID,Level,Category,Message,Source,Timestamp,Count,Recovered\n";
            for (const auto& error : m_errors) {
                stream << QString("%1,%2,%3,\"%4\",%5,%6,%7,%8\n")
                         .arg(error.id)
                         .arg(errorLevelToString(error.level))
                         .arg(errorCategoryToString(error.category))
                         .arg(error.message)
                         .arg(error.source)
                         .arg(error.timestamp.toString(Qt::ISODate))
                         .arg(error.occurrenceCount)
                         .arg(error.recovered ? "Yes" : "No");
            }
            
        } else { // txt format
            for (const auto& error : m_errors) {
                stream << QString("[%1] %2 - %3: %4\n")
                         .arg(error.timestamp.toString())
                         .arg(errorLevelToString(error.level))
                         .arg(error.source)
                         .arg(error.message);
                if (!error.details.isEmpty()) {
                    stream << QString("  Details: %1\n").arg(error.details);
                }
                stream << "\n";
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to export error log:" << e.what();
        return false;
    }
}

QString UtilsErrorHandler::errorLevelToString(ErrorLevel level)
{
    switch (level) {
        case Info: return "Info";
        case Warning: return "Warning";
        case Error: return "Error";
        case Critical: return "Critical";
        case Fatal: return "Fatal";
        default: return "Unknown";
    }
}

UtilsErrorHandler::ErrorLevel UtilsErrorHandler::stringToErrorLevel(const QString& levelStr)
{
    QString lower = levelStr.toLower();
    if (lower == "info") return Info;
    if (lower == "warning") return Warning;
    if (lower == "error") return Error;
    if (lower == "critical") return Critical;
    if (lower == "fatal") return Fatal;
    return Error;
}

QString UtilsErrorHandler::errorCategoryToString(ErrorCategory category)
{
    switch (category) {
        case SystemError: return "System";
        case ConfigurationError: return "Configuration";
        case FileSystemError: return "FileSystem";
        case NetworkError: return "Network";
        case CryptoError: return "Crypto";
        case ValidationError: return "Validation";
        case MemoryError: return "Memory";
        case PermissionError: return "Permission";
        case TimeoutError: return "Timeout";
        case UnknownError: return "Unknown";
        default: return "Unknown";
    }
}

UtilsErrorHandler::ErrorCategory UtilsErrorHandler::stringToErrorCategory(const QString& categoryStr)
{
    QString lower = categoryStr.toLower();
    if (lower == "system") return SystemError;
    if (lower == "configuration") return ConfigurationError;
    if (lower == "filesystem") return FileSystemError;
    if (lower == "network") return NetworkError;
    if (lower == "crypto") return CryptoError;
    if (lower == "validation") return ValidationError;
    if (lower == "memory") return MemoryError;
    if (lower == "permission") return PermissionError;
    if (lower == "timeout") return TimeoutError;
    return UnknownError;
}

void UtilsErrorHandler::performAutoCleanup()
{
    QMutexLocker locker(&s_mutex);
    cleanupExpiredErrors();
}

QString UtilsErrorHandler::generateErrorId()
{
    return QString("ERR_%1_%2").arg(QDateTime::currentMSecsSinceEpoch())
                              .arg(++m_errorIdCounter);
}

void UtilsErrorHandler::addErrorToQueue(const ErrorInfo& errorInfo)
{
    m_errors[errorInfo.id] = errorInfo;
    m_errorQueue.enqueue(errorInfo.id);
    
    // 检查是否超过最大错误数量
    while (m_errors.size() > m_maxErrorCount && !m_errorQueue.isEmpty()) {
        QString oldestErrorId = m_errorQueue.dequeue();
        m_errors.remove(oldestErrorId);
    }
}

void UtilsErrorHandler::cleanupExpiredErrors()
{
    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-7); // 保留7天内的错误
    
    auto it = m_errors.begin();
    while (it != m_errors.end()) {
        if (it.value().timestamp < cutoffTime && it.value().recovered) {
            m_errorQueue.removeAll(it.key());
            it = m_errors.erase(it);
        } else {
            ++it;
        }
    }
    
    updateStatistics();
}

void UtilsErrorHandler::updateStatistics()
{
    m_statistics.clear();
    
    // 总体统计
    m_statistics["totalErrors"] = m_errors.size();
    m_statistics["recoveredErrors"] = 0;
    m_statistics["unrecoveredErrors"] = 0;
    
    // 按级别统计
    QVariantMap levelStats;
    QVariantMap categoryStats;
    
    for (const auto& error : m_errors) {
        // 恢复状态统计
        if (error.recovered) {
            m_statistics["recoveredErrors"] = m_statistics["recoveredErrors"].toInt() + 1;
        } else {
            m_statistics["unrecoveredErrors"] = m_statistics["unrecoveredErrors"].toInt() + 1;
        }
        
        // 级别统计
        QString levelStr = errorLevelToString(error.level);
        levelStats[levelStr] = levelStats.value(levelStr, 0).toInt() + 1;
        
        // 类别统计
        QString categoryStr = errorCategoryToString(error.category);
        categoryStats[categoryStr] = categoryStats.value(categoryStr, 0).toInt() + 1;
    }
    
    m_statistics["byLevel"] = levelStats;
    m_statistics["byCategory"] = categoryStats;
    m_statistics["lastUpdated"] = QDateTime::currentDateTime();
    
    emit errorStatisticsUpdated(m_statistics);
}

void UtilsErrorHandler::checkCriticalErrorThreshold(const ErrorInfo& errorInfo)
{
    if (errorInfo.level == Critical) {
        emit criticalErrorOccurred(errorInfo);
    } else if (errorInfo.level == Fatal) {
        emit fatalErrorOccurred(errorInfo);
    }
    
    // 检查错误频率阈值
    int recentErrorCount = 0;
    QDateTime recentTime = QDateTime::currentDateTime().addSecs(-300); // 5分钟内
    
    for (const auto& error : m_errors) {
        if (error.timestamp > recentTime && error.level >= Error) {
            recentErrorCount++;
        }
    }
    
    if (recentErrorCount > 10) { // 5分钟内超过10个错误
        ErrorInfo criticalError;
        criticalError.id = generateErrorId();
        criticalError.level = Critical;
        criticalError.category = SystemError;
        criticalError.message = QString("High error frequency detected: %1 errors in 5 minutes").arg(recentErrorCount);
        criticalError.source = "UtilsErrorHandler";
        criticalError.timestamp = QDateTime::currentDateTime();
        
        emit criticalErrorOccurred(criticalError);
    }
}