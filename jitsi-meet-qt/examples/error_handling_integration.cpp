/**
 * @file error_handling_integration.cpp
 * @brief 示例：如何在现有组件中集成错误处理系统
 * 
 * 本文件展示了如何在WelcomeWindow中集成新的错误处理系统，
 * 包括URL验证、错误显示和恢复机制。
 */

#include "WelcomeWindow.h"
#include "ErrorRecoveryManager.h"
#include "ErrorDialog.h"
#include "ErrorUtils.h"
#include "JitsiError.h"

// 在WelcomeWindow类中添加错误处理相关成员
class WelcomeWindowWithErrorHandling : public WelcomeWindow
{
    Q_OBJECT

public:
    explicit WelcomeWindowWithErrorHandling(QWidget *parent = nullptr);
    ~WelcomeWindowWithErrorHandling();
    
    /**
     * @brief 设置错误恢复管理器
     */
    void setErrorRecoveryManager(ErrorRecoveryManager* errorManager);

private slots:
    void onJoinButtonClickedWithValidation();
    void onUrlChangedWithValidation(const QString& text);
    void onErrorRecoveryResult(const JitsiError& error, const ErrorRecoveryManager::RecoveryResult& result);
    void onUserInterventionRequired(const JitsiError& error);

private:
    void validateUrlWithErrorHandling(const QString& url);
    void showErrorWithRecovery(const JitsiError& error);
    void handleValidationError(const JitsiError& error);
    void handleNetworkError(const JitsiError& error);
    
private:
    ErrorRecoveryManager* m_errorManager;
    ErrorDialog* m_currentErrorDialog;
};

WelcomeWindowWithErrorHandling::WelcomeWindowWithErrorHandling(QWidget *parent)
    : WelcomeWindow(parent)
    , m_errorManager(nullptr)
    , m_currentErrorDialog(nullptr)
{
    // 连接错误处理信号
    // 注意：这些连接应该在设置了errorManager之后进行
}

WelcomeWindowWithErrorHandling::~WelcomeWindowWithErrorHandling()
{
    if (m_currentErrorDialog) {
        m_currentErrorDialog->deleteLater();
    }
}

void WelcomeWindowWithErrorHandling::setErrorRecoveryManager(ErrorRecoveryManager* errorManager)
{
    m_errorManager = errorManager;
    
    if (m_errorManager) {
        // 连接错误处理信号
        connect(m_errorManager, &ErrorRecoveryManager::errorHandled,
                this, &WelcomeWindowWithErrorHandling::onErrorRecoveryResult);
        connect(m_errorManager, &ErrorRecoveryManager::userInterventionRequired,
                this, &WelcomeWindowWithErrorHandling::onUserInterventionRequired);
    }
}

void WelcomeWindowWithErrorHandling::onJoinButtonClickedWithValidation()
{
    QString url = getUrlText().trimmed();
    
    if (url.isEmpty()) {
        // 使用随机房间名
        url = generateRandomRoomName();
        setUrlText(url);
    }
    
    // 使用错误处理系统验证URL
    validateUrlWithErrorHandling(url);
}

void WelcomeWindowWithErrorHandling::onUrlChangedWithValidation(const QString& text)
{
    // 实时验证URL（但不显示错误，只更新按钮状态）
    if (!text.isEmpty()) {
        ErrorUtils::UrlValidationResult result = ErrorUtils::validateJitsiUrl(text);
        
        // 更新UI状态
        updateJoinButtonState();
        
        if (result.isValid) {
            clearError();
        }
        // 不在实时验证中显示错误，只在提交时显示
    }
}

void WelcomeWindowWithErrorHandling::validateUrlWithErrorHandling(const QString& url)
{
    // 使用ErrorUtils进行URL验证
    ErrorUtils::UrlValidationResult result = ErrorUtils::validateJitsiUrl(url);
    
    if (!result.isValid) {
        // 创建验证错误
        JitsiError error = ErrorUtils::createUrlValidationError(url, result.errorMessage);
        error.addContext("suggestion", result.suggestion);
        
        // 处理错误
        handleValidationError(error);
        return;
    }
    
    // URL有效，构建完整URL并发射信号
    QString fullUrl;
    if (url.startsWith("http://") || url.startsWith("https://")) {
        fullUrl = url;
    } else if (ErrorUtils::isJitsiProtocolUrl(url)) {
        // 协议URL，提取房间名并构建HTTP URL
        QString roomName = ErrorUtils::extractRoomName(url);
        fullUrl = ErrorUtils::buildConferenceUrl(roomName, getDefaultServerUrl());
    } else {
        // 纯房间名，构建完整URL
        fullUrl = ErrorUtils::buildConferenceUrl(url, getDefaultServerUrl());
    }
    
    // 规范化URL
    fullUrl = ErrorUtils::normalizeUrl(fullUrl);
    
    // 添加到最近列表
    addToRecentItems(fullUrl);
    
    // 发射加入会议信号
    emit joinConference(fullUrl);
}

void WelcomeWindowWithErrorHandling::handleValidationError(const JitsiError& error)
{
    if (m_errorManager) {
        // 使用错误恢复管理器处理错误
        ErrorRecoveryManager::RecoveryResult result = m_errorManager->handleError(error);
        
        if (result.strategy == ErrorRecoveryManager::RecoveryStrategy::UserIntervention) {
            showErrorWithRecovery(error);
        }
    } else {
        // 回退到简单错误显示
        showError(error.toUserMessage());
    }
}

void WelcomeWindowWithErrorHandling::handleNetworkError(const JitsiError& error)
{
    if (m_errorManager) {
        // 网络错误可能支持自动重试
        ErrorRecoveryManager::RecoveryResult result = m_errorManager->handleError(error);
        
        if (result.strategy == ErrorRecoveryManager::RecoveryStrategy::Retry) {
            // 显示重试信息
            showError(QString("网络连接失败，正在重试... (%1)").arg(result.message));
        } else {
            showErrorWithRecovery(error);
        }
    } else {
        showError(error.toUserMessage());
    }
}

void WelcomeWindowWithErrorHandling::showErrorWithRecovery(const JitsiError& error)
{
    // 关闭之前的错误对话框
    if (m_currentErrorDialog) {
        m_currentErrorDialog->deleteLater();
        m_currentErrorDialog = nullptr;
    }
    
    // 创建新的错误对话框
    m_currentErrorDialog = new ErrorDialog(error, this);
    
    // 根据错误类型配置对话框
    switch (error.type()) {
    case ErrorType::InvalidUrl:
        m_currentErrorDialog->setRetryEnabled(true);
        m_currentErrorDialog->setAutoCloseTimeout(0); // 不自动关闭
        break;
    case ErrorType::NetworkError:
        m_currentErrorDialog->setRetryEnabled(true);
        m_currentErrorDialog->setAutoCloseTimeout(30); // 30秒后自动关闭
        break;
    case ErrorType::ValidationError:
        m_currentErrorDialog->setRetryEnabled(false);
        m_currentErrorDialog->setAutoCloseTimeout(0);
        break;
    default:
        break;
    }
    
    // 连接对话框信号
    connect(m_currentErrorDialog, &ErrorDialog::finished, [this](int result) {
        if (m_currentErrorDialog) {
            ErrorDialog::Result dialogResult = m_currentErrorDialog->result();
            
            switch (dialogResult) {
            case ErrorDialog::Retry:
                // 重试操作
                onJoinButtonClickedWithValidation();
                break;
            case ErrorDialog::Reset:
                // 重置输入
                setUrlText("");
                clearError();
                break;
            case ErrorDialog::Ignore:
            case ErrorDialog::Cancel:
            default:
                // 忽略或取消，清除错误显示
                clearError();
                break;
            }
            
            m_currentErrorDialog->deleteLater();
            m_currentErrorDialog = nullptr;
        }
    });
    
    // 显示对话框
    m_currentErrorDialog->show();
}

void WelcomeWindowWithErrorHandling::onErrorRecoveryResult(
    const JitsiError& error, 
    const ErrorRecoveryManager::RecoveryResult& result)
{
    if (result.success) {
        // 恢复成功，清除错误显示
        clearError();
        
        // 根据恢复策略执行相应操作
        switch (result.strategy) {
        case ErrorRecoveryManager::RecoveryStrategy::Retry:
            // 自动重试成功，可以继续操作
            break;
        case ErrorRecoveryManager::RecoveryStrategy::Reset:
            // 重置成功，可能需要重新加载配置
            loadRecentItems();
            break;
        default:
            break;
        }
    } else {
        // 恢复失败，显示错误信息
        showError(QString("错误恢复失败: %1").arg(result.message));
    }
}

void WelcomeWindowWithErrorHandling::onUserInterventionRequired(const JitsiError& error)
{
    // 需要用户干预，显示错误对话框
    showErrorWithRecovery(error);
}

// 使用示例：在MainApplication中集成错误处理
class MainApplicationWithErrorHandling : public QApplication
{
public:
    MainApplicationWithErrorHandling(int argc, char *argv[])
        : QApplication(argc, argv)
        , m_errorManager(new ErrorRecoveryManager(this))
        , m_welcomeWindow(new WelcomeWindowWithErrorHandling())
    {
        setupErrorHandling();
    }
    
private:
    void setupErrorHandling()
    {
        // 配置错误管理器
        m_errorManager->setLoggingEnabled(true);
        m_errorManager->setMaxRetryCount(3);
        
        // 设置日志文件路径
        QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(logPath);
        m_errorManager->setLogFilePath(QDir(logPath).filePath("jitsi_errors.log"));
        
        // 将错误管理器设置到窗口
        m_welcomeWindow->setErrorRecoveryManager(m_errorManager);
        
        // 连接全局错误处理信号
        connect(m_errorManager, &ErrorRecoveryManager::restartRequired,
                this, &MainApplicationWithErrorHandling::onRestartRequired);
    }
    
private slots:
    void onRestartRequired(const QString& reason)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            m_welcomeWindow,
            "需要重启应用程序",
            QString("由于以下原因，应用程序需要重启：\n%1\n\n是否立即重启？").arg(reason),
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            // 重启应用程序
            QProcess::startDetached(QApplication::applicationFilePath());
            quit();
        }
    }
    
private:
    ErrorRecoveryManager* m_errorManager;
    WelcomeWindowWithErrorHandling* m_welcomeWindow;
};

// 网络请求错误处理示例
class NetworkRequestWithErrorHandling : public QObject
{
    Q_OBJECT
    
public:
    NetworkRequestWithErrorHandling(ErrorRecoveryManager* errorManager, QObject* parent = nullptr)
        : QObject(parent)
        , m_errorManager(errorManager)
        , m_networkManager(new QNetworkAccessManager(this))
    {
        connect(m_networkManager, &QNetworkAccessManager::finished,
                this, &NetworkRequestWithErrorHandling::onRequestFinished);
    }
    
    void testServerConnection(const QString& serverUrl)
    {
        QNetworkRequest request(QUrl(serverUrl));
        request.setRawHeader("User-Agent", "JitsiMeetQt/1.0");
        
        QNetworkReply* reply = m_networkManager->get(request);
        reply->setProperty("serverUrl", serverUrl);
    }
    
private slots:
    void onRequestFinished(QNetworkReply* reply)
    {
        QString serverUrl = reply->property("serverUrl").toString();
        
        if (reply->error() != QNetworkReply::NoError) {
            // 创建网络错误
            JitsiError error = ErrorUtils::createNetworkError(
                reply->error(), 
                serverUrl, 
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
            );
            
            // 使用错误管理器处理
            if (m_errorManager) {
                ErrorRecoveryManager::RecoveryResult result = m_errorManager->handleError(error);
                
                if (result.success && result.strategy == ErrorRecoveryManager::RecoveryStrategy::Retry) {
                    // 延迟重试
                    QTimer::singleShot(result.data.value("retryDelay", 5000).toInt(), [this, serverUrl]() {
                        testServerConnection(serverUrl);
                    });
                }
            }
        } else {
            // 连接成功
            emit serverConnectionSuccessful(serverUrl);
        }
        
        reply->deleteLater();
    }
    
signals:
    void serverConnectionSuccessful(const QString& serverUrl);
    void serverConnectionFailed(const JitsiError& error);
    
private:
    ErrorRecoveryManager* m_errorManager;
    QNetworkAccessManager* m_networkManager;
};

#include "error_handling_integration.moc"