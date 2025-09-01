#ifndef URLHANDLER_H
#define URLHANDLER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QRegularExpression>

/**
 * @brief URL处理器类
 * 
 * 专门处理各种类型的会议URL解析、验证和转换
 */
class URLHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief URL类型枚举
     */
    enum URLType {
        JitsiMeetURL,      ///< Jitsi Meet标准URL (https://meet.jit.si/room)
        JitsiProtocol,     ///< Jitsi协议URL (jitsi-meet://meet.jit.si/room)
        JitsiMeetProtocol, ///< Jitsi Meet协议URL (jitsi-meet://)
        CustomURL,         ///< 自定义URL
        PlainRoomName,     ///< 纯房间名（使用默认服务器）
        InvalidURL         ///< 无效URL
    };
    Q_ENUM(URLType)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit URLHandler(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~URLHandler();

    /**
     * @brief 解析URL
     * @param url URL字符串
     * @return 解析结果映射
     */
    QVariantMap parseURL(const QString& url);

    /**
     * @brief 验证URL格式
     * @param url URL字符串
     * @return 是否有效
     */
    bool validateURL(const QString& url);

    /**
     * @brief 获取URL类型
     * @param url URL字符串
     * @return URL类型
     */
    URLType getURLType(const QString& url);

    /**
     * @brief 规范化URL
     * @param url 原始URL
     * @return 规范化后的URL
     */
    QString normalizeURL(const QString& url);

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
     * @brief 提取URL参数
     * @param url URL字符串
     * @return 参数映射
     */
    QVariantMap extractParameters(const QString& url);

    /**
     * @brief 解析URL片段中的配置参数
     * @param fragment URL片段字符串
     * @return 配置参数映射
     */
    QVariantMap parseFragmentConfig(const QString& fragment);

    /**
     * @brief 处理深度链接URL
     * @param url 深度链接URL
     * @return 处理结果映射
     */
    QVariantMap handleDeepLink(const QString& url);

    /**
     * @brief 支持的URL格式检查
     * @param url URL字符串
     * @return 是否为支持的格式
     */
    bool isSupportedFormat(const QString& url);

    /**
     * @brief 构建会议URL
     * @param server 服务器地址
     * @param roomName 房间名称
     * @param parameters 参数映射
     * @return 构建的URL
     */
    QString buildMeetingURL(const QString& server, 
                          const QString& roomName, 
                          const QVariantMap& parameters = QVariantMap());

    /**
     * @brief 转换协议URL为HTTPS URL
     * @param protocolUrl 协议URL
     * @return HTTPS URL
     */
    QString convertProtocolToHttps(const QString& protocolUrl);

    /**
     * @brief 转换HTTPS URL为协议URL
     * @param httpsUrl HTTPS URL
     * @return 协议URL
     */
    QString convertHttpsToProtocol(const QString& httpsUrl);

    /**
     * @brief 清理URL
     * @param url 原始URL
     * @return 清理后的URL
     */
    QString sanitizeURL(const QString& url);

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
     * @brief 设置支持的协议
     * @param protocols 协议列表
     */
    void setSupportedProtocols(const QStringList& protocols);

    /**
     * @brief 获取支持的协议
     * @return 协议列表
     */
    QStringList supportedProtocols() const;

    /**
     * @brief 添加自定义URL模式
     * @param pattern 正则表达式模式
     * @param name 模式名称
     */
    void addCustomPattern(const QString& pattern, const QString& name);

    /**
     * @brief 移除自定义URL模式
     * @param name 模式名称
     */
    void removeCustomPattern(const QString& name);

    /**
     * @brief 获取URL信息摘要
     * @param url URL字符串
     * @return 信息摘要
     */
    QString getURLSummary(const QString& url);

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
     * @param valid 是否有效
     */
    void urlValidated(const QString& url, bool valid);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private:
    /**
     * @brief 初始化URL模式
     */
    void initializePatterns();

    /**
     * @brief 解析Jitsi Meet URL
     * @param url URL对象
     * @return 解析结果
     */
    QVariantMap parseJitsiMeetURL(const QUrl& url);

    /**
     * @brief 解析Jitsi协议URL
     * @param url URL字符串
     * @return 解析结果
     */
    QVariantMap parseJitsiProtocolURL(const QString& url);

    /**
     * @brief 解析自定义URL
     * @param url URL字符串
     * @return 解析结果
     */
    QVariantMap parseCustomURL(const QString& url);

    /**
     * @brief 解析查询参数
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
     * @brief 匹配URL模式
     * @param url URL字符串
     * @param pattern 正则表达式模式
     * @return 匹配结果
     */
    QVariantMap matchPattern(const QString& url, const QRegularExpression& pattern);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // URLHANDLER_H