#ifndef LINKHANDLER_H
#define LINKHANDLER_H

#include "ILinkHandler.h"
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QTimer>
#include <memory>

/**
 * @brief 链接处理器实现类
 * 
 * 实现ILinkHandler接口，提供完整的会议链接处理功能
 */
class LinkHandler : public ILinkHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit LinkHandler(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~LinkHandler();

    // ILinkHandler接口实现
    QVariantMap parseUrl(const QString& url) override;
    ValidationResult validateUrl(const QString& url) override;
    QVariantMap extractParameters(const QString& url) override;
    LinkType getLinkType(const QString& url) override;
    QString buildMeetingUrl(const QString& server, 
                          const QString& roomName, 
                          const QVariantMap& parameters = QVariantMap()) override;
    QString normalizeUrl(const QString& url) override;
    bool isServerReachable(const QString& serverUrl) override;
    QVariantMap getRoomInfo(const QString& roomUrl) override;
    void setSupportedProtocols(const QStringList& protocols) override;
    QStringList getSupportedProtocols() const override;

    /**
     * @brief 设置默认服务器
     * @param server 服务器地址
     */
    void setDefaultServer(const QString& server);

    /**
     * @brief 获取默认服务器
     * @return 服务器地址
     */
    QString defaultServer() const;

    /**
     * @brief 设置URL验证超时时间
     * @param timeout 超时时间（毫秒）
     */
    void setValidationTimeout(int timeout);

    /**
     * @brief 获取URL验证超时时间
     * @return 超时时间（毫秒）
     */
    int validationTimeout() const;

    /**
     * @brief 异步验证URL
     * @param url 会议链接
     */
    void validateUrlAsync(const QString& url);

    /**
     * @brief 异步检查服务器可达性
     * @param serverUrl 服务器URL
     */
    void checkServerAsync(const QString& serverUrl);

    /**
     * @brief 异步获取房间信息
     * @param roomUrl 房间URL
     */
    void getRoomInfoAsync(const QString& roomUrl);

    /**
     * @brief 解析Jitsi协议URL
     * @param url Jitsi协议URL
     * @return 解析结果
     */
    QVariantMap parseJitsiProtocolUrl(const QString& url);

    /**
     * @brief 解析HTTPS URL
     * @param url HTTPS URL
     * @return 解析结果
     */
    QVariantMap parseHttpsUrl(const QString& url);

    /**
     * @brief 验证房间名称
     * @param roomName 房间名称
     * @return 是否有效
     */
    bool validateRoomName(const QString& roomName);

    /**
     * @brief 验证服务器地址
     * @param server 服务器地址
     * @return 是否有效
     */
    bool validateServer(const QString& server);

    /**
     * @brief 清理URL参数
     * @param url 原始URL
     * @return 清理后的URL
     */
    QString sanitizeUrl(const QString& url);

public slots:
    /**
     * @brief 清除缓存
     */
    void clearCache();

    /**
     * @brief 刷新服务器状态
     */
    void refreshServerStatus();

private slots:
    /**
     * @brief 处理网络回复
     */
    void handleNetworkReply();

    /**
     * @brief 处理验证超时
     */
    void handleValidationTimeout();

private:
    /**
     * @brief 初始化网络管理器
     */
    void initializeNetworkManager();

    /**
     * @brief 解析URL查询参数
     * @param url URL对象
     * @return 参数映射
     */
    QVariantMap parseQueryParameters(const QUrl& url);

    /**
     * @brief 构建查询字符串
     * @param parameters 参数映射
     * @return 查询字符串
     */
    QString buildQueryString(const QVariantMap& parameters);

    /**
     * @brief 验证URL格式
     * @param url URL字符串
     * @return 是否有效
     */
    bool isValidUrlFormat(const QString& url);

    /**
     * @brief 检查协议支持
     * @param protocol 协议名称
     * @return 是否支持
     */
    bool isProtocolSupported(const QString& protocol);

    /**
     * @brief 提取服务器地址
     * @param url URL字符串
     * @return 服务器地址
     */
    QString extractServer(const QString& url);

    /**
     * @brief 提取房间名称
     * @param url URL字符串
     * @return 房间名称
     */
    QString extractRoomName(const QString& url);

    /**
     * @brief 缓存验证结果
     * @param url URL字符串
     * @param result 验证结果
     */
    void cacheValidationResult(const QString& url, ValidationResult result);

    /**
     * @brief 获取缓存的验证结果
     * @param url URL字符串
     * @return 验证结果（如果存在）
     */
    ValidationResult getCachedValidationResult(const QString& url);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // LINKHANDLER_H