#ifndef ILINKHANDLER_H
#define ILINKHANDLER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantMap>

/**
 * @brief 链接处理器接口
 * 
 * 定义会议链接解析、验证和处理的核心功能
 */
class ILinkHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 链接类型枚举
     */
    enum LinkType {
        HttpsLink,       ///< HTTPS链接
        JitsiProtocol,   ///< Jitsi协议链接
        CustomProtocol,  ///< 自定义协议链接
        InvalidLink      ///< 无效链接
    };
    Q_ENUM(LinkType)

    /**
     * @brief 验证结果枚举
     */
    enum ValidationResult {
        Valid,           ///< 有效
        InvalidFormat,   ///< 格式无效
        InvalidServer,   ///< 服务器无效
        InvalidRoom,     ///< 房间无效
        NetworkError,    ///< 网络错误
        PermissionDenied ///< 权限拒绝
    };
    Q_ENUM(ValidationResult)

    explicit ILinkHandler(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ILinkHandler() = default;

    /**
     * @brief 解析会议URL
     * @param url 会议链接
     * @return 解析结果映射
     */
    virtual QVariantMap parseUrl(const QString& url) = 0;

    /**
     * @brief 验证URL有效性
     * @param url 会议链接
     * @return 验证结果
     */
    virtual ValidationResult validateUrl(const QString& url) = 0;

    /**
     * @brief 提取会议参数
     * @param url 会议链接
     * @return 参数映射
     */
    virtual QVariantMap extractParameters(const QString& url) = 0;

    /**
     * @brief 获取链接类型
     * @param url 会议链接
     * @return 链接类型
     */
    virtual LinkType getLinkType(const QString& url) = 0;

    /**
     * @brief 构建会议URL
     * @param server 服务器地址
     * @param roomName 房间名称
     * @param parameters 附加参数
     * @return 构建的URL
     */
    virtual QString buildMeetingUrl(const QString& server, 
                                  const QString& roomName, 
                                  const QVariantMap& parameters = QVariantMap()) = 0;

    /**
     * @brief 规范化URL
     * @param url 原始URL
     * @return 规范化后的URL
     */
    virtual QString normalizeUrl(const QString& url) = 0;

    /**
     * @brief 检查服务器可达性
     * @param serverUrl 服务器URL
     * @return 是否可达
     */
    virtual bool isServerReachable(const QString& serverUrl) = 0;

    /**
     * @brief 获取房间信息
     * @param roomUrl 房间URL
     * @return 房间信息映射
     */
    virtual QVariantMap getRoomInfo(const QString& roomUrl) = 0;

    /**
     * @brief 设置支持的协议
     * @param protocols 协议列表
     */
    virtual void setSupportedProtocols(const QStringList& protocols) = 0;

    /**
     * @brief 获取支持的协议
     * @return 协议列表
     */
    virtual QStringList getSupportedProtocols() const = 0;

signals:
    /**
     * @brief URL解析完成信号
     * @param url 原始URL
     * @param result 解析结果
     */
    void urlParsed(const QString& url, const QVariantMap& result);

    /**
     * @brief URL验证完成信号
     * @param url 原始URL
     * @param result 验证结果
     */
    void urlValidated(const QString& url, ValidationResult result);

    /**
     * @brief 服务器检查完成信号
     * @param serverUrl 服务器URL
     * @param reachable 是否可达
     */
    void serverChecked(const QString& serverUrl, bool reachable);

    /**
     * @brief 房间信息获取完成信号
     * @param roomUrl 房间URL
     * @param roomInfo 房间信息
     */
    void roomInfoReceived(const QString& roomUrl, const QVariantMap& roomInfo);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);
};

#endif // ILINKHANDLER_H