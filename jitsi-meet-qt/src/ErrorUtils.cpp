#include "ErrorUtils.h"
#include "JitsiConstants.h"
#include <QNetworkInterface>
#include <QSysInfo>
#include <QApplication>
#include <QDateTime>
#include <QDebug>

// 静态成员初始化
const QRegularExpression ErrorUtils::s_urlRegex(
    R"(^https?:\/\/[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)*(:[0-9]{1,5})?(\/.*)?$)"
);

const QRegularExpression ErrorUtils::s_roomNameRegex(
    R"(^[a-zA-Z0-9][a-zA-Z0-9\-_]{0,62}[a-zA-Z0-9]?$)"
);

const QRegularExpression ErrorUtils::s_serverUrlRegex(
    R"(^https:\/\/[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)*(:[0-9]{1,5})?$)"
);

const QRegularExpression ErrorUtils::s_protocolRegex(
    R"(^jitsi-meet:\/\/(.+)$)"
);

ErrorUtils::UrlValidationResult ErrorUtils::validateJitsiUrl(const QString& url)
{
    if (url.isEmpty()) {
        return UrlValidationResult(false, "URL不能为空", "请输入会议室名称或完整URL");
    }
    
    QString trimmedUrl = url.trimmed();
    
    // 检查是否为协议URL
    if (isJitsiProtocolUrl(trimmedUrl)) {
        QRegularExpressionMatch match = s_protocolRegex.match(trimmedUrl);
        if (match.hasMatch()) {
            QString roomPart = match.captured(1);
            if (isValidRoomName(roomPart)) {
                return UrlValidationResult(true);
            } else {
                return UrlValidationResult(false, "协议URL中的房间名格式不正确", 
                                         "房间名只能包含字母、数字、连字符和下划线");
            }
        }
    }
    
    // 检查是否为完整HTTP(S) URL
    if (trimmedUrl.startsWith("http://") || trimmedUrl.startsWith("https://")) {
        QUrl qurl(trimmedUrl);
        if (!qurl.isValid()) {
            return UrlValidationResult(false, "URL格式不正确", "请检查URL格式是否正确");
        }
        
        if (!s_urlRegex.match(trimmedUrl).hasMatch()) {
            return UrlValidationResult(false, "URL格式不符合要求", "请使用有效的HTTP或HTTPS URL");
        }
        
        // 检查是否包含房间名
        QString path = qurl.path();
        if (path.length() <= 1) {
            return UrlValidationResult(false, "URL中缺少房间名", "请在URL中包含会议室名称");
        }
        
        return UrlValidationResult(true);
    }
    
    // 检查是否为纯房间名
    if (isValidRoomName(trimmedUrl)) {
        return UrlValidationResult(true);
    }
    
    // 检查是否包含域名但缺少协议
    if (trimmedUrl.contains('.') && !trimmedUrl.startsWith("http")) {
        return UrlValidationResult(false, "URL缺少协议前缀", "请在URL前添加 https://");
    }
    
    return UrlValidationResult(false, "无效的URL或房间名格式", 
                              "请输入有效的房间名或完整的会议URL");
}

ErrorUtils::UrlValidationResult ErrorUtils::validateServerUrl(const QString& serverUrl)
{
    if (serverUrl.isEmpty()) {
        return UrlValidationResult(false, "服务器URL不能为空", "请输入有效的服务器地址");
    }
    
    QString trimmedUrl = serverUrl.trimmed();
    
    if (!trimmedUrl.startsWith("https://")) {
        return UrlValidationResult(false, "服务器URL必须使用HTTPS协议", "请使用 https:// 开头的URL");
    }
    
    QUrl qurl(trimmedUrl);
    if (!qurl.isValid()) {
        return UrlValidationResult(false, "服务器URL格式不正确", "请检查URL格式");
    }
    
    if (!s_serverUrlRegex.match(trimmedUrl).hasMatch()) {
        return UrlValidationResult(false, "服务器URL格式不符合要求", "请使用有效的HTTPS URL");
    }
    
    QString host = qurl.host();
    if (!isValidServerDomain(host)) {
        return UrlValidationResult(false, "服务器域名格式不正确", "请使用有效的域名");
    }
    
    return UrlValidationResult(true);
}

QString ErrorUtils::buildConferenceUrl(const QString& roomName, const QString& serverUrl)
{
    QString cleanServerUrl = serverUrl;
    if (cleanServerUrl.endsWith('/')) {
        cleanServerUrl.chop(1);
    }
    
    QString cleanRoomName = roomName.trimmed();
    if (cleanRoomName.startsWith('/')) {
        cleanRoomName = cleanRoomName.mid(1);
    }
    
    return QString("%1/%2").arg(cleanServerUrl, cleanRoomName);
}

QString ErrorUtils::extractRoomName(const QString& url)
{
    QUrl qurl(url);
    QString path = qurl.path();
    
    if (path.startsWith('/')) {
        path = path.mid(1);
    }
    
    // 移除查询参数和片段
    int queryIndex = path.indexOf('?');
    if (queryIndex != -1) {
        path = path.left(queryIndex);
    }
    
    int fragmentIndex = path.indexOf('#');
    if (fragmentIndex != -1) {
        path = path.left(fragmentIndex);
    }
    
    return path;
}

QString ErrorUtils::extractServerUrl(const QString& url)
{
    QUrl qurl(url);
    
    QString serverUrl = QString("%1://%2").arg(qurl.scheme(), qurl.host());
    
    if (qurl.port() != -1) {
        serverUrl += QString(":%1").arg(qurl.port());
    }
    
    return serverUrl;
}

ErrorUtils::NetworkErrorInfo ErrorUtils::analyzeNetworkError(QNetworkReply::NetworkError error, int httpStatus)
{
    NetworkErrorInfo info;
    info.errorCode = error;
    
    switch (error) {
    case QNetworkReply::NoError:
        info.errorString = "No error";
        info.userMessage = "操作成功完成";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ConnectionRefusedError:
        info.errorString = "Connection refused";
        info.userMessage = "服务器拒绝连接，请检查服务器地址是否正确";
        info.isRetryable = true;
        info.suggestedRetryDelay = 5000;
        break;
        
    case QNetworkReply::RemoteHostClosedError:
        info.errorString = "Remote host closed connection";
        info.userMessage = "服务器关闭了连接，请稍后重试";
        info.isRetryable = true;
        info.suggestedRetryDelay = 3000;
        break;
        
    case QNetworkReply::HostNotFoundError:
        info.errorString = "Host not found";
        info.userMessage = "找不到服务器，请检查网络连接和服务器地址";
        info.isRetryable = true;
        info.suggestedRetryDelay = 10000;
        break;
        
    case QNetworkReply::TimeoutError:
        info.errorString = "Connection timeout";
        info.userMessage = "连接超时，请检查网络连接";
        info.isRetryable = true;
        info.suggestedRetryDelay = 5000;
        break;
        
    case QNetworkReply::OperationCanceledError:
        info.errorString = "Operation canceled";
        info.userMessage = "操作已取消";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::SslHandshakeFailedError:
        info.errorString = "SSL handshake failed";
        info.userMessage = "SSL连接失败，请检查服务器证书";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::TemporaryNetworkFailureError:
        info.errorString = "Temporary network failure";
        info.userMessage = "网络临时故障，请稍后重试";
        info.isRetryable = true;
        info.suggestedRetryDelay = 10000;
        break;
        
    case QNetworkReply::NetworkSessionFailedError:
        info.errorString = "Network session failed";
        info.userMessage = "网络会话失败，请检查网络设置";
        info.isRetryable = true;
        info.suggestedRetryDelay = 15000;
        break;
        
    case QNetworkReply::BackgroundRequestNotAllowedError:
        info.errorString = "Background request not allowed";
        info.userMessage = "后台请求被阻止";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::TooManyRedirectsError:
        info.errorString = "Too many redirects";
        info.userMessage = "重定向次数过多，请检查服务器配置";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::InsecureRedirectError:
        info.errorString = "Insecure redirect";
        info.userMessage = "不安全的重定向，请使用HTTPS连接";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ProxyConnectionRefusedError:
        info.errorString = "Proxy connection refused";
        info.userMessage = "代理服务器拒绝连接，请检查代理设置";
        info.isRetryable = true;
        info.suggestedRetryDelay = 5000;
        break;
        
    case QNetworkReply::ProxyNotFoundError:
        info.errorString = "Proxy not found";
        info.userMessage = "找不到代理服务器，请检查代理设置";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ProxyTimeoutError:
        info.errorString = "Proxy timeout";
        info.userMessage = "代理服务器超时，请检查代理设置";
        info.isRetryable = true;
        info.suggestedRetryDelay = 10000;
        break;
        
    case QNetworkReply::ProxyAuthenticationRequiredError:
        info.errorString = "Proxy authentication required";
        info.userMessage = "代理服务器需要身份验证";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ContentAccessDenied:
        info.errorString = "Content access denied";
        info.userMessage = "访问被拒绝，请检查权限设置";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ContentOperationNotPermittedError:
        info.errorString = "Content operation not permitted";
        info.userMessage = "操作不被允许";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ContentNotFoundError:
        info.errorString = "Content not found";
        info.userMessage = "请求的内容不存在";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::AuthenticationRequiredError:
        info.errorString = "Authentication required";
        info.userMessage = "需要身份验证";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ContentReSendError:
        info.errorString = "Content resend error";
        info.userMessage = "内容重发错误";
        info.isRetryable = true;
        info.suggestedRetryDelay = 3000;
        break;
        
    case QNetworkReply::ContentConflictError:
        info.errorString = "Content conflict";
        info.userMessage = "内容冲突";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ContentGoneError:
        info.errorString = "Content gone";
        info.userMessage = "请求的内容已不存在";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::InternalServerError:
        info.errorString = "Internal server error";
        info.userMessage = "服务器内部错误，请稍后重试";
        info.isRetryable = true;
        info.suggestedRetryDelay = 30000;
        break;
        
    case QNetworkReply::OperationNotImplementedError:
        info.errorString = "Operation not implemented";
        info.userMessage = "服务器不支持此操作";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ServiceUnavailableError:
        info.errorString = "Service unavailable";
        info.userMessage = "服务暂时不可用，请稍后重试";
        info.isRetryable = true;
        info.suggestedRetryDelay = 60000;
        break;
        
    case QNetworkReply::ProtocolUnknownError:
        info.errorString = "Protocol unknown";
        info.userMessage = "未知的网络协议";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ProtocolInvalidOperationError:
        info.errorString = "Protocol invalid operation";
        info.userMessage = "协议操作无效";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::UnknownNetworkError:
        info.errorString = "Unknown network error";
        info.userMessage = "未知网络错误";
        info.isRetryable = true;
        info.suggestedRetryDelay = 10000;
        break;
        
    case QNetworkReply::UnknownProxyError:
        info.errorString = "Unknown proxy error";
        info.userMessage = "未知代理错误";
        info.isRetryable = true;
        info.suggestedRetryDelay = 10000;
        break;
        
    case QNetworkReply::UnknownContentError:
        info.errorString = "Unknown content error";
        info.userMessage = "未知内容错误";
        info.isRetryable = false;
        break;
        
    case QNetworkReply::ProtocolFailure:
        info.errorString = "Protocol failure";
        info.userMessage = "协议失败";
        info.isRetryable = true;
        info.suggestedRetryDelay = 5000;
        break;
        
    case QNetworkReply::UnknownServerError:
        info.errorString = "Unknown server error";
        info.userMessage = "未知服务器错误";
        info.isRetryable = true;
        info.suggestedRetryDelay = 15000;
        break;
        
    default:
        info.errorString = QString("Network error %1").arg(static_cast<int>(error));
        info.userMessage = "网络错误，请检查网络连接";
        info.isRetryable = true;
        info.suggestedRetryDelay = 10000;
        break;
    }
    
    // 根据HTTP状态码调整信息
    if (httpStatus > 0) {
        switch (httpStatus) {
        case 400:
            info.userMessage = "请求格式错误";
            info.isRetryable = false;
            break;
        case 401:
            info.userMessage = "需要身份验证";
            info.isRetryable = false;
            break;
        case 403:
            info.userMessage = "访问被禁止";
            info.isRetryable = false;
            break;
        case 404:
            info.userMessage = "会议室不存在或服务器地址错误";
            info.isRetryable = false;
            break;
        case 429:
            info.userMessage = "请求过于频繁，请稍后重试";
            info.isRetryable = true;
            info.suggestedRetryDelay = 60000;
            break;
        case 500:
            info.userMessage = "服务器内部错误";
            info.isRetryable = true;
            info.suggestedRetryDelay = 30000;
            break;
        case 502:
            info.userMessage = "网关错误";
            info.isRetryable = true;
            info.suggestedRetryDelay = 15000;
            break;
        case 503:
            info.userMessage = "服务暂时不可用";
            info.isRetryable = true;
            info.suggestedRetryDelay = 60000;
            break;
        case 504:
            info.userMessage = "网关超时";
            info.isRetryable = true;
            info.suggestedRetryDelay = 30000;
            break;
        }
    }
    
    return info;
}

JitsiError ErrorUtils::createNetworkError(QNetworkReply::NetworkError error, const QString& url, int httpStatus)
{
    NetworkErrorInfo info = analyzeNetworkError(error, httpStatus);
    
    JitsiError jitsiError = JitsiError::networkError(info.userMessage, info.errorString);
    jitsiError.addContext("url", url);
    jitsiError.addContext("networkError", QString::number(static_cast<int>(error)));
    jitsiError.addContext("isRetryable", info.isRetryable ? "true" : "false");
    jitsiError.addContext("retryDelay", QString::number(info.suggestedRetryDelay));
    
    if (httpStatus > 0) {
        jitsiError.addContext("httpStatus", QString::number(httpStatus));
    }
    
    return jitsiError;
}

JitsiError ErrorUtils::createUrlValidationError(const QString& url, const QString& reason)
{
    JitsiError error = JitsiError::invalidUrlError(url, reason);
    error.addContext("originalUrl", url);
    error.addContext("validationReason", reason);
    
    return error;
}

bool ErrorUtils::isJitsiProtocolUrl(const QString& url)
{
    return url.startsWith(JitsiConstants::PROTOCOL_PREFIX, Qt::CaseInsensitive);
}

QString ErrorUtils::normalizeUrl(const QString& url)
{
    QString normalized = url.trimmed();
    
    // 移除末尾的斜杠
    while (normalized.endsWith('/') && normalized.length() > 1) {
        normalized.chop(1);
    }
    
    // 如果是纯房间名，不做处理
    if (!normalized.contains("://") && !normalized.contains('.')) {
        return normalized;
    }
    
    // 如果包含域名但没有协议，添加https://
    if (normalized.contains('.') && !normalized.startsWith("http")) {
        normalized = "https://" + normalized;
    }
    
    return normalized;
}

bool ErrorUtils::isNetworkAvailable()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface& interface : interfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            interface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            return true;
        }
    }
    
    return false;
}

QString ErrorUtils::getErrorDescription(const JitsiError& error)
{
    return error.toUserMessage();
}

QStringList ErrorUtils::getErrorSuggestions(const JitsiError& error)
{
    QStringList suggestions;
    
    switch (error.type()) {
    case ErrorType::NetworkError:
        suggestions << "检查网络连接是否正常"
                   << "尝试使用其他网络"
                   << "检查防火墙设置"
                   << "稍后重试";
        break;
        
    case ErrorType::InvalidUrl:
        suggestions << "检查URL格式是否正确"
                   << "确保包含完整的服务器地址"
                   << "尝试使用房间名而不是完整URL"
                   << "检查是否有拼写错误";
        break;
        
    case ErrorType::WebEngineError:
        suggestions << "刷新页面重试"
                   << "清除浏览器缓存"
                   << "检查是否支持WebRTC"
                   << "尝试使用其他浏览器";
        break;
        
    case ErrorType::ConfigurationError:
        suggestions << "重置应用程序设置"
                   << "检查配置文件权限"
                   << "重新安装应用程序"
                   << "联系技术支持";
        break;
        
    case ErrorType::ProtocolError:
        suggestions << "直接在应用中输入会议室地址"
                   << "检查协议处理器注册"
                   << "重新安装应用程序"
                   << "使用完整URL而不是协议链接";
        break;
        
    case ErrorType::ValidationError:
        suggestions << "检查输入格式是否正确"
                   << "参考输入示例"
                   << "移除特殊字符"
                   << "使用英文字符";
        break;
        
    case ErrorType::SystemError:
        suggestions << "重启应用程序"
                   << "重启计算机"
                   << "检查系统资源使用情况"
                   << "联系技术支持";
        break;
    }
    
    return suggestions;
}

bool ErrorUtils::requiresImmediateAttention(const JitsiError& error)
{
    return error.severity() == ErrorSeverity::Critical ||
           error.type() == ErrorType::SystemError;
}

QString ErrorUtils::generateErrorReport(const JitsiError& error, bool includeSystemInfo)
{
    QString report;
    QTextStream stream(&report);
    
    stream << "=== Jitsi Meet Qt 错误报告 ===" << Qt::endl;
    stream << "时间: " << error.timestamp().toString("yyyy-MM-dd hh:mm:ss") << Qt::endl;
    stream << "错误代码: " << error.errorCode() << Qt::endl;
    stream << "错误类型: " << error.typeString() << Qt::endl;
    stream << "严重程度: " << error.severityString() << Qt::endl;
    stream << "错误消息: " << error.message() << Qt::endl;
    
    if (!error.details().isEmpty()) {
        stream << "详细信息: " << error.details() << Qt::endl;
    }
    
    // 上下文信息
    QMap<QString, QString> context = error.getAllContext();
    if (!context.isEmpty()) {
        stream << Qt::endl << "上下文信息:" << Qt::endl;
        for (auto it = context.constBegin(); it != context.constEnd(); ++it) {
            stream << "  " << it.key() << ": " << it.value() << Qt::endl;
        }
    }
    
    // 建议解决方案
    QStringList suggestions = getErrorSuggestions(error);
    if (!suggestions.isEmpty()) {
        stream << Qt::endl << "建议解决方案:" << Qt::endl;
        for (int i = 0; i < suggestions.size(); ++i) {
            stream << "  " << (i + 1) << ". " << suggestions[i] << Qt::endl;
        }
    }
    
    // 系统信息
    if (includeSystemInfo) {
        stream << Qt::endl << "系统信息:" << Qt::endl;
        stream << getSystemInfo();
    }
    
    return report;
}

// 私有方法实现
bool ErrorUtils::isValidRoomName(const QString& roomName)
{
    if (roomName.isEmpty() || roomName.length() > 64) {
        return false;
    }
    
    return s_roomNameRegex.match(roomName).hasMatch();
}

bool ErrorUtils::isValidServerDomain(const QString& domain)
{
    if (domain.isEmpty() || domain.length() > 253) {
        return false;
    }
    
    // 简单的域名验证
    QRegularExpression domainRegex(R"(^[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)*$)");
    return domainRegex.match(domain).hasMatch();
}

QString ErrorUtils::sanitizeUrl(const QString& url)
{
    QString sanitized = url.trimmed();
    
    // 移除危险字符
    sanitized.remove(QRegularExpression(R"([<>\"'`])"));
    
    return sanitized;
}

QString ErrorUtils::getSystemInfo()
{
    QString info;
    QTextStream stream(&info);
    
    stream << "  操作系统: " << QSysInfo::prettyProductName() << Qt::endl;
    stream << "  系统版本: " << QSysInfo::productVersion() << Qt::endl;
    stream << "  CPU架构: " << QSysInfo::currentCpuArchitecture() << Qt::endl;
    stream << "  Qt版本: " << qVersion() << Qt::endl;
    stream << "  应用程序版本: " << QApplication::applicationVersion() << Qt::endl;
    stream << "  网络可用: " << (isNetworkAvailable() ? "是" : "否") << Qt::endl;
    
    return info;
}