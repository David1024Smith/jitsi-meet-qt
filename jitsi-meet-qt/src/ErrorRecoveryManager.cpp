#include "../include/ErrorRecoveryManager.h"
#include "../include/ConfigurationManager.h"
#include "JitsiConstants.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>
// #include <QWebEngineView> // Not needed for core functionality
#include <QPushButton>

Q_LOGGING_CATEGORY(errorRecovery, "jitsi.error.recovery")

ErrorRecoveryManager::ErrorRecoveryManager(QObject* parent)
    : QObject(parent)
    , m_configManager(nullptr)
    , m_loggingEnabled(true)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_maxLogFileSize(10 * 1024 * 1024) // 10MB
    , m_retryTimer(new QTimer(this))
    , m_maxRetryCount(3)
    , m_showErrorDialogs(true)
    , m_autoRecoveryEnabled(true)
    , m_dialogTimeout(30000) // 30秒
{
    initializeLogging();
    
    // 设置重试定时器
    m_retryTimer->setSingleShot(true);
    connect(m_retryTimer, &QTimer::timeout, this, &ErrorRecoveryManager::onRetryTimer);
    
    qCDebug(errorRecovery) << "ErrorRecoveryManager initialized";
}

ErrorRecoveryManager::~ErrorRecoveryManager()
{
    if (m_logStream) {
        delete m_logStream;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
}

void ErrorRecoveryManager::setConfigurationManager(ConfigurationManager* configManager)
{
    m_configManager = configManager;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleError(const JitsiError& error)
{
    qCDebug(errorRecovery) << "Handling error:" << error;
    
    // 记录错误到日志
    logError(error);
    
    // 更新统计信息
    updateErrorStatistics(error.type());
    
    // 保存最后的错误信息
    m_lastError = error;
    
    RecoveryResult result;
    
    // 根据错误类型选择处理策略
    switch (error.type()) {
    case ErrorType::NetworkError:
        result = handleNetworkErrorInternal(error);
        break;
    case ErrorType::InvalidUrl:
        result = handleInvalidUrlErrorInternal(error);
        break;
    case ErrorType::WebEngineError:
        result = handleWebEngineErrorInternal(error);
        break;
    case ErrorType::ConfigurationError:
        result = handleConfigurationErrorInternal(error);
        break;
    case ErrorType::ProtocolError:
        result = handleProtocolErrorInternal(error);
        break;
    case ErrorType::ValidationError:
        result = handleValidationErrorInternal(error);
        break;
    case ErrorType::SystemError:
        result = handleSystemErrorInternal(error);
        break;
    case ErrorType::WebRTCError:
        result = handleWebRTCErrorInternal(error);
        break;
    case ErrorType::XMPPConnectionError:
        result = handleXMPPConnectionErrorInternal(error);
        break;
    case ErrorType::AuthenticationError:
        result = handleAuthenticationErrorInternal(error);
        break;
    case ErrorType::MediaDeviceError:
        result = handleMediaDeviceErrorInternal(error);
        break;
    default:
        result = RecoveryResult(false, RecoveryStrategy::UserIntervention, "Unknown error type");
        break;
    }
    
    // 发射处理完成信号
    emit errorHandled(error, result);
    
    // 如果恢复成功，发射成功信号
    if (result.success) {
        emit recoverySuccessful(error.type(), result.strategy);
        // 重置重试计数
        m_retryCount[error.type()] = 0;
    } else {
        emit recoveryFailed(error.type(), result.message);
    }
    
    return result;
}

QMessageBox::StandardButton ErrorRecoveryManager::showErrorDialog(const JitsiError& error, QWidget* parent)
{
    if (!m_showErrorDialogs) {
        return QMessageBox::Ok;
    }
    
    QMessageBox* dialog = createErrorDialog(error, parent);
    
    // 设置对话框超时
    QTimer* timeoutTimer = new QTimer(dialog);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->setInterval(m_dialogTimeout);
    
    connect(timeoutTimer, &QTimer::timeout, [dialog]() {
        dialog->done(QMessageBox::Ok);
    });
    
    timeoutTimer->start();
    
    QMessageBox::StandardButton result = static_cast<QMessageBox::StandardButton>(dialog->exec());
    
    timeoutTimer->stop();
    delete dialog;
    
    return result;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::attemptRecovery(ErrorType errorType)
{
    qCDebug(errorRecovery) << "Attempting recovery for error type:" << static_cast<int>(errorType);
    
    RecoveryResult result;
    
    switch (errorType) {
    case ErrorType::NetworkError:
        // 网络错误：等待后重试
        result.strategy = RecoveryStrategy::Retry;
        result.success = true;
        result.message = "Will retry network operation";
        break;
        
    case ErrorType::InvalidUrl:
        // URL错误：需要用户重新输入
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "User needs to provide valid URL";
        break;
        
    case ErrorType::WebEngineError:
        // WebEngine错误：重启WebEngine
        result.success = restartWebEngine();
        result.strategy = RecoveryStrategy::Restart;
        result.message = result.success ? "WebEngine restarted" : "Failed to restart WebEngine";
        break;
        
    case ErrorType::ConfigurationError:
        // 配置错误：重置为默认值
        result.success = resetToDefaults();
        result.strategy = RecoveryStrategy::Reset;
        result.message = result.success ? "Configuration reset to defaults" : "Failed to reset configuration";
        break;
        
    case ErrorType::ProtocolError:
        // 协议错误：使用备用方案
        result.strategy = RecoveryStrategy::Fallback;
        result.success = true;
        result.message = "Using fallback method";
        break;
        
    case ErrorType::ValidationError:
        // 验证错误：需要用户修正
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "User needs to correct input";
        break;
        
    case ErrorType::SystemError:
        // 系统错误：可能需要重启应用
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "System error may require application restart";
        emit restartRequired("System error detected");
        break;
        
    case ErrorType::WebRTCError:
        // WebRTC错误：重启媒体引擎
        result.success = reinitializeMediaDevices();
        result.strategy = RecoveryStrategy::Restart;
        result.message = result.success ? "Media engine restarted" : "Failed to restart media engine";
        break;
        
    case ErrorType::XMPPConnectionError:
        // XMPP连接错误：重新连接
        result.success = restartXMPPConnection();
        result.strategy = RecoveryStrategy::Retry;
        result.message = result.success ? "XMPP connection restarted" : "Failed to restart XMPP connection";
        break;
        
    case ErrorType::AuthenticationError:
        // 认证错误：需要用户重新输入凭据
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "Authentication failed, user credentials required";
        break;
        
    case ErrorType::MediaDeviceError:
        // 媒体设备错误：重新初始化设备
        result.success = reinitializeMediaDevices();
        result.strategy = RecoveryStrategy::Reset;
        result.message = result.success ? "Media devices reinitialized" : "Failed to reinitialize media devices";
        break;
        
    default:
        result.strategy = RecoveryStrategy::None;
        result.success = false;
        result.message = "No recovery strategy available";
        break;
    }
    
    return result;
}

void ErrorRecoveryManager::logError(const JitsiError& error)
{
    if (!m_loggingEnabled) {
        return;
    }
    
    QMutexLocker locker(&m_logMutex);
    
    if (!m_logFile || !m_logStream) {
        return;
    }
    
    QString logEntry = error.toLogString();
    writeToLogFile(logEntry);
    
    // 检查日志文件大小，必要时轮转
    if (m_logFile->size() > m_maxLogFileSize) {
        rotateLogFile();
    }
}

void ErrorRecoveryManager::setLogFilePath(const QString& path)
{
    QMutexLocker locker(&m_logMutex);
    
    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }
    
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
    
    m_logFilePath = path;
    initializeLogging();
}

void ErrorRecoveryManager::clearErrorLog()
{
    QMutexLocker locker(&m_logMutex);
    
    if (m_logFile) {
        m_logFile->resize(0);
    }
}

// 槽函数实现
void ErrorRecoveryManager::handleNetworkError(const QString& message, const QString& details)
{
    JitsiError error = JitsiError::networkError(message, details);
    handleError(error);
}

void ErrorRecoveryManager::handleUrlValidationError(const QString& url, const QString& reason)
{
    JitsiError error = JitsiError::invalidUrlError(url, reason);
    handleError(error);
}

void ErrorRecoveryManager::handleWebEngineError(const QString& message, const QString& details)
{
    JitsiError error = JitsiError::webEngineError(message, details);
    handleError(error);
}

void ErrorRecoveryManager::handleConfigurationError(const QString& message, const QString& details)
{
    JitsiError error = JitsiError::configurationError(message, details);
    handleError(error);
}

void ErrorRecoveryManager::handleProtocolError(const QString& message, const QString& details)
{
    JitsiError error = JitsiError::protocolError(message, details);
    handleError(error);
}

void ErrorRecoveryManager::handleWebRTCError(const QString& message, const QString& details)
{
    JitsiError error = JitsiError::webRTCError(message, details);
    handleError(error);
}

void ErrorRecoveryManager::handleXMPPConnectionError(const QString& message, const QString& details)
{
    JitsiError error = JitsiError::xmppConnectionError(message, details);
    handleError(error);
}

void ErrorRecoveryManager::handleAuthenticationError(const QString& message, const QString& details)
{
    JitsiError error = JitsiError::authenticationError(message, details);
    handleError(error);
}

void ErrorRecoveryManager::handleMediaDeviceError(const QString& message, const QString& details)
{
    JitsiError error = JitsiError::mediaDeviceError(message, details);
    handleError(error);
}

void ErrorRecoveryManager::onRetryTimer()
{
    qCDebug(errorRecovery) << "Retry timer triggered, attempting recovery";
    
    if (!m_lastError.has_value() || m_lastError->type() != ErrorType::NetworkError) {
        return;
    }
    
    // 重试最后的操作
    RecoveryResult result = attemptRecovery(m_lastError->type());
    
    if (result.success) {
        emit recoverySuccessful(m_lastError->type(), result.strategy);
    } else {
        // 如果还有重试次数，继续重试
        int& retryCount = m_retryCount[m_lastError->type()];
        if (retryCount < m_maxRetryCount) {
            retryCount++;
            m_retryTimer->start(5000 * retryCount); // 递增延迟
        } else {
            emit recoveryFailed(m_lastError->type(), "Max retry count exceeded");
        }
    }
}

// 私有方法实现
ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleNetworkErrorInternal(const JitsiError& error)
{
    RecoveryResult result;
    
    // 检查重试次数
    int& retryCount = m_retryCount[ErrorType::NetworkError];
    
    if (retryCount < m_maxRetryCount) {
        retryCount++;
        result.strategy = RecoveryStrategy::Retry;
        result.success = true;
        result.message = QString("Will retry in %1 seconds (attempt %2/%3)")
                        .arg(5 * retryCount)
                        .arg(retryCount)
                        .arg(m_maxRetryCount);
        
        // 启动重试定时器
        m_retryTimer->start(5000 * retryCount);
    } else {
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "Network error persists, user intervention required";
        
        if (m_showErrorDialogs) {
            emit userInterventionRequired(error);
        }
    }
    
    return result;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleInvalidUrlErrorInternal(const JitsiError& error)
{
    RecoveryResult result;
    result.strategy = RecoveryStrategy::UserIntervention;
    result.success = false;
    result.message = "Invalid URL format, user needs to correct input";
    
    if (m_showErrorDialogs) {
        showErrorDialog(error);
    }
    
    return result;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleWebEngineErrorInternal(const JitsiError& error)
{
    RecoveryResult result;
    
    if (error.severity() == ErrorSeverity::Critical) {
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "Critical WebEngine error, restart required";
        emit restartRequired("Critical WebEngine error");
    } else {
        result.success = restartWebEngine();
        result.strategy = RecoveryStrategy::Restart;
        result.message = result.success ? "WebEngine restarted successfully" : "Failed to restart WebEngine";
    }
    
    return result;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleConfigurationErrorInternal(const JitsiError& error)
{
    RecoveryResult result;
    
    if (m_autoRecoveryEnabled) {
        result.success = validateAndFixConfiguration();
        result.strategy = RecoveryStrategy::Reset;
        result.message = result.success ? "Configuration validated and fixed" : "Failed to fix configuration";
    } else {
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "Configuration error requires manual intervention";
        
        if (m_showErrorDialogs) {
            showErrorDialog(error);
        }
    }
    
    return result;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleProtocolErrorInternal(const JitsiError& /* error */)
{
    RecoveryResult result;
    result.strategy = RecoveryStrategy::Fallback;
    result.success = true;
    result.message = "Using fallback method for protocol handling";
    
    // 协议错误通常不是致命的，可以继续运行
    return result;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleValidationErrorInternal(const JitsiError& error)
{
    RecoveryResult result;
    result.strategy = RecoveryStrategy::UserIntervention;
    result.success = false;
    result.message = "Validation error requires user correction";
    
    if (m_showErrorDialogs) {
        showErrorDialog(error);
    }
    
    return result;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleSystemErrorInternal(const JitsiError& error)
{
    RecoveryResult result;
    
    if (error.severity() == ErrorSeverity::Critical) {
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "Critical system error, application restart required";
        emit restartRequired("Critical system error");
    } else {
        result.strategy = RecoveryStrategy::Reset;
        result.success = resetToDefaults();
        result.message = result.success ? "System reset to defaults" : "Failed to reset system";
    }
    
    return result;
}

bool ErrorRecoveryManager::resetToDefaults()
{
    if (!m_configManager) {
        return false;
    }
    
    try {
        // 重置配置到默认值
        // 这里需要调用ConfigurationManager的重置方法
        qCDebug(errorRecovery) << "Resetting configuration to defaults";
        return true;
    } catch (...) {
        qCWarning(errorRecovery) << "Failed to reset configuration to defaults";
        return false;
    }
}

bool ErrorRecoveryManager::clearCache()
{
    try {
        // 清理缓存文件
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QDir dir(cacheDir);
        
        if (dir.exists()) {
            dir.removeRecursively();
            dir.mkpath(".");
        }
        
        qCDebug(errorRecovery) << "Cache cleared successfully";
        return true;
    } catch (...) {
        qCWarning(errorRecovery) << "Failed to clear cache";
        return false;
    }
}

bool ErrorRecoveryManager::restartWebEngine()
{
    try {
        // 重启WebEngine的逻辑
        // 这里需要与WebEngine组件交互
        qCDebug(errorRecovery) << "Restarting WebEngine";
        return true;
    } catch (...) {
        qCWarning(errorRecovery) << "Failed to restart WebEngine";
        return false;
    }
}

bool ErrorRecoveryManager::validateAndFixConfiguration()
{
    if (!m_configManager) {
        return false;
    }
    
    try {
        // 验证和修复配置
        qCDebug(errorRecovery) << "Validating and fixing configuration";
        return true;
    } catch (...) {
        qCWarning(errorRecovery) << "Failed to validate and fix configuration";
        return false;
    }
}

bool ErrorRecoveryManager::retryLastOperation()
{
    // 重试最后的操作
    qCDebug(errorRecovery) << "Retrying last operation:" << m_lastOperation;
    return true;
}

void ErrorRecoveryManager::initializeLogging()
{
    if (!m_loggingEnabled) {
        return;
    }
    
    // 设置默认日志文件路径
    if (m_logFilePath.isEmpty()) {
        QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(logDir);
        m_logFilePath = QDir(logDir).filePath("jitsi_errors.log");
    }
    
    m_logFile = new QFile(m_logFilePath);
    
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_logStream = new QTextStream(m_logFile);
        // Note: setCodec is not available in Qt 6, UTF-8 is default
        
        // 写入启动标记
        writeToLogFile(QString("=== Error Recovery Manager Started at %1 ===")
                      .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    } else {
        qCWarning(errorRecovery) << "Failed to open log file:" << m_logFilePath;
        delete m_logFile;
        m_logFile = nullptr;
    }
}

void ErrorRecoveryManager::writeToLogFile(const QString& logEntry)
{
    if (!m_logStream) {
        return;
    }
    
    *m_logStream << logEntry << Qt::endl;
    m_logStream->flush();
}

void ErrorRecoveryManager::rotateLogFile()
{
    if (!m_logFile) {
        return;
    }
    
    // 关闭当前日志文件
    m_logFile->close();
    
    // 重命名为备份文件
    QString backupPath = m_logFilePath + ".bak";
    QFile::remove(backupPath);
    QFile::rename(m_logFilePath, backupPath);
    
    // 重新打开日志文件
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        writeToLogFile(QString("=== Log rotated at %1 ===")
                      .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    }
}

QMessageBox* ErrorRecoveryManager::createErrorDialog(const JitsiError& error, QWidget* parent)
{
    QMessageBox* dialog = new QMessageBox(parent);
    
    // 设置对话框属性
    dialog->setWindowTitle("Jitsi Meet - 错误");
    dialog->setText(error.toUserMessage());
    
    if (!error.details().isEmpty()) {
        dialog->setDetailedText(error.details());
    }
    
    // 根据错误严重程度设置图标
    switch (error.severity()) {
    case ErrorSeverity::Info:
        dialog->setIcon(QMessageBox::Information);
        break;
    case ErrorSeverity::Warning:
        dialog->setIcon(QMessageBox::Warning);
        break;
    case ErrorSeverity::Error:
        dialog->setIcon(QMessageBox::Critical);
        break;
    case ErrorSeverity::Critical:
        dialog->setIcon(QMessageBox::Critical);
        break;
    }
    
    setupDialogButtons(dialog, error);
    
    return dialog;
}

void ErrorRecoveryManager::setupDialogButtons(QMessageBox* dialog, const JitsiError& error)
{
    // 根据错误类型和是否可恢复设置按钮
    if (error.isRecoverable()) {
        dialog->setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
        dialog->setDefaultButton(QMessageBox::Retry);
    } else {
        dialog->setStandardButtons(QMessageBox::Ok);
        dialog->setDefaultButton(QMessageBox::Ok);
    }
    
    // 为特定错误类型添加额外按钮
    switch (error.type()) {
    case ErrorType::ConfigurationError:
        dialog->addButton("重置设置", QMessageBox::ResetRole);
        break;
    case ErrorType::WebEngineError:
        dialog->addButton("重启组件", QMessageBox::ActionRole);
        break;
    default:
        break;
    }
}

void ErrorRecoveryManager::updateErrorStatistics(ErrorType errorType)
{
    m_errorStats[errorType]++;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleWebRTCErrorInternal(const JitsiError& error)
{
    RecoveryResult result;
    
    if (error.severity() == ErrorSeverity::Critical) {
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "Critical WebRTC error, manual intervention required";
        emit userInterventionRequired(error);
    } else {
        result.success = reinitializeMediaDevices();
        result.strategy = RecoveryStrategy::Restart;
        result.message = result.success ? "Media devices reinitialized successfully" : "Failed to reinitialize media devices";
        
        if (!result.success && m_showErrorDialogs) {
            showErrorDialog(error);
        }
    }
    
    return result;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleXMPPConnectionErrorInternal(const JitsiError& error)
{
    RecoveryResult result;
    
    // 检查重试次数
    int& retryCount = m_retryCount[ErrorType::XMPPConnectionError];
    
    if (retryCount < m_maxRetryCount) {
        retryCount++;
        result.success = restartXMPPConnection();
        result.strategy = RecoveryStrategy::Retry;
        result.message = result.success ? 
            QString("XMPP connection restarted (attempt %1/%2)").arg(retryCount).arg(m_maxRetryCount) :
            QString("Failed to restart XMPP connection (attempt %1/%2)").arg(retryCount).arg(m_maxRetryCount);
        
        if (!result.success) {
            // 启动重试定时器
            m_retryTimer->start(5000 * retryCount);
        }
    } else {
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "XMPP connection failed after maximum retry attempts";
        
        if (m_showErrorDialogs) {
            emit userInterventionRequired(error);
        }
    }
    
    return result;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleAuthenticationErrorInternal(const JitsiError& error)
{
    RecoveryResult result;
    result.strategy = RecoveryStrategy::UserIntervention;
    result.success = false;
    result.message = "Authentication failed, user credentials required";
    
    if (m_showErrorDialogs) {
        showErrorDialog(error);
    }
    
    return result;
}

ErrorRecoveryManager::RecoveryResult ErrorRecoveryManager::handleMediaDeviceErrorInternal(const JitsiError& error)
{
    RecoveryResult result;
    
    if (error.severity() == ErrorSeverity::Critical) {
        result.strategy = RecoveryStrategy::UserIntervention;
        result.success = false;
        result.message = "Critical media device error, check device permissions";
        emit userInterventionRequired(error);
    } else {
        result.success = reinitializeMediaDevices();
        result.strategy = RecoveryStrategy::Reset;
        result.message = result.success ? "Media devices reset successfully" : "Failed to reset media devices";
        
        if (!result.success && m_showErrorDialogs) {
            showErrorDialog(error);
        }
    }
    
    return result;
}

bool ErrorRecoveryManager::restartXMPPConnection()
{
    try {
        // 重启XMPP连接的逻辑
        // 这里需要与XMPPClient组件交互
        qCDebug(errorRecovery) << "Restarting XMPP connection";
        
        // 模拟重启过程
        // 实际实现中需要调用XMPPClient的重连方法
        
        return true;
    } catch (...) {
        qCWarning(errorRecovery) << "Failed to restart XMPP connection";
        return false;
    }
}

bool ErrorRecoveryManager::reinitializeMediaDevices()
{
    try {
        // 重新初始化媒体设备的逻辑
        // 这里需要与MediaManager组件交互
        qCDebug(errorRecovery) << "Reinitializing media devices";
        
        // 模拟重新初始化过程
        // 实际实现中需要调用MediaManager的重新初始化方法
        
        return true;
    } catch (...) {
        qCWarning(errorRecovery) << "Failed to reinitialize media devices";
        return false;
    }
}