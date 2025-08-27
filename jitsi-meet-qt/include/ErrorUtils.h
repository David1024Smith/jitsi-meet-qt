#ifndef ERRORUTILS_H
#define ERRORUTILS_H

#include <QString>
#include <QUrl>
#include <QNetworkReply>
#include <QRegularExpression>
#include "JitsiError.h"

/**
 * @brief 错误处理工具类
 * 
 * 提供各种错误检测、验证和处理的工具函数
 */
class ErrorUtils
{
public:
    /**
     * @brief URL验证结果
     */
    struct UrlValidationResult {
        bool isValid;
        QString errorMessage;
        QString suggestion;
        
        UrlValidationResult(bool valid = false, const QString& error = QString(), const QString& suggest = QString())
            : isValid(valid), errorMessage(error), suggestion(suggest) {}
    };
    
    /**
     * @brief 网络错误信息
     */
    struct NetworkErrorInfo {
        QNetworkReply::NetworkError errorCode;
        QString errorString;
        QString userMessage;
        bool isRetryable;
        int suggestedRetryDelay;
        
        NetworkErrorInfo()
            : errorCode(QNetworkReply::NoError)
            , isRetryable(false)
            , suggestedRetryDelay(0) {}
    };

public:
    /**
     * @brief 验证Jitsi Meet URL格式
     * @param url 要验证的URL
     * @return 验证结果
     */
    static UrlValidationResult validateJitsiUrl(const QString& url);
    
    /**
     * @brief 验证服务器URL格式
     * @param serverUrl 服务器URL
     * @return 验证结果
     */
    static UrlValidationResult validateServerUrl(const QString& serverUrl);
    
    /**
     * @brief 从房间名生成完整URL
     * @param roomName 房间名
     * @param serverUrl 服务器URL
     * @return 完整的会议URL
     */
    static QString buildConferenceUrl(const QString& roomName, const QString& serverUrl);
    
    /**
     * @brief 从URL提取房间名
     * @param url 完整URL
     * @return 房间名
     */
    static QString extractRoomName(const QString& url);
    
    /**
     * @brief 从URL提取服务器地址
     * @param url 完整URL
     * @return 服务器地址
     */
    static QString extractServerUrl(const QString& url);
    
    /**
     * @brief 分析网络错误
     * @param error 网络错误代码
     * @param httpStatus HTTP状态码（可选）
     * @return 网络错误信息
     */
    static NetworkErrorInfo analyzeNetworkError(QNetworkReply::NetworkError error, int httpStatus = 0);
    
    /**
     * @brief 创建网络错误对象
     * @param error 网络错误代码
     * @param url 相关URL
     * @param httpStatus HTTP状态码
     * @return JitsiError对象
     */
    static JitsiError createNetworkError(QNetworkReply::NetworkError error, const QString& url, int httpStatus = 0);
    
    /**
     * @brief 创建URL验证错误对象
     * @param url 无效的URL
     * @param reason 错误原因
     * @return JitsiError对象
     */
    static JitsiError createUrlValidationError(const QString& url, const QString& reason);
    
    /**
     * @brief 检查URL是否为Jitsi Meet协议
     * @param url URL字符串
     * @return 是否为jitsi-meet://协议
     */
    static bool isJitsiProtocolUrl(const QString& url);
    
    /**
     * @brief 规范化URL格式
     * @param url 原始URL
     * @return 规范化后的URL
     */
    static QString normalizeUrl(const QString& url);
    
    /**
     * @brief 检查网络连接状态
     * @return 是否有网络连接
     */
    static bool isNetworkAvailable();
    
    /**
     * @brief 获取错误的用户友好描述
     * @param error JitsiError对象
     * @return 用户友好的错误描述
     */
    static QString getErrorDescription(const JitsiError& error);
    
    /**
     * @brief 获取错误的建议解决方案
     * @param error JitsiError对象
     * @return 建议的解决方案
     */
    static QStringList getErrorSuggestions(const JitsiError& error);
    
    /**
     * @brief 检查错误是否需要立即处理
     * @param error JitsiError对象
     * @return 是否需要立即处理
     */
    static bool requiresImmediateAttention(const JitsiError& error);
    
    /**
     * @brief 生成错误报告
     * @param error JitsiError对象
     * @param includeSystemInfo 是否包含系统信息
     * @return 错误报告字符串
     */
    static QString generateErrorReport(const JitsiError& error, bool includeSystemInfo = true);

private:
    // URL验证相关的正则表达式
    static const QRegularExpression s_urlRegex;
    static const QRegularExpression s_roomNameRegex;
    static const QRegularExpression s_serverUrlRegex;
    static const QRegularExpression s_protocolRegex;
    
    // 网络错误映射
    static const QMap<QNetworkReply::NetworkError, NetworkErrorInfo> s_networkErrorMap;
    
    // 私有辅助方法
    static bool isValidRoomName(const QString& roomName);
    static bool isValidServerDomain(const QString& domain);
    static QString sanitizeUrl(const QString& url);
    static QString getSystemInfo();
};

#endif // ERRORUTILS_H