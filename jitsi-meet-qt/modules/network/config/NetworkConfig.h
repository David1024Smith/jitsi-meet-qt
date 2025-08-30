#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QUrl>
#include <QStringList>

/**
 * @brief 网络配置类
 * 
 * NetworkConfig负责管理网络模块的所有配置参数，包括服务器设置、
 * 连接参数、协议配置和性能优化选项。
 */
class NetworkConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(int connectionTimeout READ connectionTimeout WRITE setConnectionTimeout NOTIFY connectionTimeoutChanged)
    Q_PROPERTY(bool autoReconnect READ autoReconnect WRITE setAutoReconnect NOTIFY autoReconnectChanged)
    Q_PROPERTY(int reconnectInterval READ reconnectInterval WRITE setReconnectInterval NOTIFY reconnectIntervalChanged)
    Q_PROPERTY(bool webRTCEnabled READ webRTCEnabled WRITE setWebRTCEnabled NOTIFY webRTCEnabledChanged)
    Q_PROPERTY(bool webSocketEnabled READ webSocketEnabled WRITE setWebSocketEnabled NOTIFY webSocketEnabledChanged)
    Q_PROPERTY(bool httpsOnly READ httpsOnly WRITE setHttpsOnly NOTIFY httpsOnlyChanged)

public:
    /**
     * @brief 连接协议枚举
     */
    enum Protocol {
        HTTP,               ///< HTTP协议
        HTTPS,              ///< HTTPS协议
        WebSocket,          ///< WebSocket协议
        WebSocketSecure,    ///< 安全WebSocket协议
        WebRTC,             ///< WebRTC协议
        XMPP                ///< XMPP协议
    };
    Q_ENUM(Protocol)

    /**
     * @brief 网络质量级别枚举
     */
    enum QualityLevel {
        Auto,               ///< 自动调整
        Low,                ///< 低质量
        Medium,             ///< 中等质量
        High,               ///< 高质量
        Ultra               ///< 超高质量
    };
    Q_ENUM(QualityLevel)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit NetworkConfig(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~NetworkConfig();

    // 服务器配置
    /**
     * @brief 设置服务器URL
     * @param url 服务器URL
     */
    void setServerUrl(const QString& url);

    /**
     * @brief 获取服务器URL
     * @return 服务器URL
     */
    QString serverUrl() const;

    /**
     * @brief 设置服务器端口
     * @param port 端口号
     */
    void setServerPort(int port);

    /**
     * @brief 获取服务器端口
     * @return 端口号
     */
    int serverPort() const;

    /**
     * @brief 设置服务器域名
     * @param domain 域名
     */
    void setServerDomain(const QString& domain);

    /**
     * @brief 获取服务器域名
     * @return 域名
     */
    QString serverDomain() const;

    // 连接配置
    /**
     * @brief 设置连接超时时间
     * @param timeout 超时时间（毫秒）
     */
    void setConnectionTimeout(int timeout);

    /**
     * @brief 获取连接超时时间
     * @return 超时时间（毫秒）
     */
    int connectionTimeout() const;

    /**
     * @brief 设置是否启用自动重连
     * @param enabled 是否启用
     */
    void setAutoReconnect(bool enabled);

    /**
     * @brief 获取是否启用自动重连
     * @return 是否启用
     */
    bool autoReconnect() const;

    /**
     * @brief 设置重连间隔
     * @param interval 重连间隔（毫秒）
     */
    void setReconnectInterval(int interval);

    /**
     * @brief 获取重连间隔
     * @return 重连间隔（毫秒）
     */
    int reconnectInterval() const;

    /**
     * @brief 设置最大重连次数
     * @param attempts 重连次数
     */
    void setMaxReconnectAttempts(int attempts);

    /**
     * @brief 获取最大重连次数
     * @return 重连次数
     */
    int maxReconnectAttempts() const;

    // 协议配置
    /**
     * @brief 设置是否启用WebRTC
     * @param enabled 是否启用
     */
    void setWebRTCEnabled(bool enabled);

    /**
     * @brief 获取是否启用WebRTC
     * @return 是否启用
     */
    bool webRTCEnabled() const;

    /**
     * @brief 设置是否启用WebSocket
     * @param enabled 是否启用
     */
    void setWebSocketEnabled(bool enabled);

    /**
     * @brief 获取是否启用WebSocket
     * @return 是否启用
     */
    bool webSocketEnabled() const;

    /**
     * @brief 设置是否仅使用HTTPS
     * @param httpsOnly 是否仅使用HTTPS
     */
    void setHttpsOnly(bool httpsOnly);

    /**
     * @brief 获取是否仅使用HTTPS
     * @return 是否仅使用HTTPS
     */
    bool httpsOnly() const;

    /**
     * @brief 设置启用的协议列表
     * @param protocols 协议列表
     */
    void setEnabledProtocols(const QList<Protocol>& protocols);

    /**
     * @brief 获取启用的协议列表
     * @return 协议列表
     */
    QList<Protocol> enabledProtocols() const;

    // STUN/TURN配置
    /**
     * @brief 设置STUN服务器列表
     * @param servers STUN服务器列表
     */
    void setStunServers(const QStringList& servers);

    /**
     * @brief 获取STUN服务器列表
     * @return STUN服务器列表
     */
    QStringList stunServers() const;

    /**
     * @brief 设置TURN服务器列表
     * @param servers TURN服务器列表
     */
    void setTurnServers(const QStringList& servers);

    /**
     * @brief 获取TURN服务器列表
     * @return TURN服务器列表
     */
    QStringList turnServers() const;

    /**
     * @brief 设置TURN服务器用户名
     * @param username 用户名
     */
    void setTurnUsername(const QString& username);

    /**
     * @brief 获取TURN服务器用户名
     * @return 用户名
     */
    QString turnUsername() const;

    /**
     * @brief 设置TURN服务器密码
     * @param password 密码
     */
    void setTurnPassword(const QString& password);

    /**
     * @brief 获取TURN服务器密码
     * @return 密码
     */
    QString turnPassword() const;

    // 性能配置
    /**
     * @brief 设置网络质量级别
     * @param level 质量级别
     */
    void setQualityLevel(QualityLevel level);

    /**
     * @brief 获取网络质量级别
     * @return 质量级别
     */
    QualityLevel qualityLevel() const;

    /**
     * @brief 设置带宽限制
     * @param bandwidth 带宽限制（kbps）
     */
    void setBandwidthLimit(int bandwidth);

    /**
     * @brief 获取带宽限制
     * @return 带宽限制（kbps）
     */
    int bandwidthLimit() const;

    /**
     * @brief 设置是否启用网络压缩
     * @param enabled 是否启用
     */
    void setCompressionEnabled(bool enabled);

    /**
     * @brief 获取是否启用网络压缩
     * @return 是否启用
     */
    bool compressionEnabled() const;

    // 配置管理
    /**
     * @brief 从配置映射加载配置
     * @param config 配置映射
     */
    void fromVariantMap(const QVariantMap& config);

    /**
     * @brief 转换为配置映射
     * @return 配置映射
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 验证配置有效性
     * @return 配置是否有效
     */
    bool validate() const;

    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults();

    /**
     * @brief 获取默认配置
     * @return 默认配置映射
     */
    static QVariantMap defaultConfiguration();

    /**
     * @brief 从文件加载配置
     * @param filePath 文件路径
     * @return 加载是否成功
     */
    bool loadFromFile(const QString& filePath);

    /**
     * @brief 保存配置到文件
     * @param filePath 文件路径
     * @return 保存是否成功
     */
    bool saveToFile(const QString& filePath) const;

public slots:
    /**
     * @brief 应用配置更改
     */
    void applyChanges();

    /**
     * @brief 取消配置更改
     */
    void cancelChanges();

signals:
    /**
     * @brief 服务器URL改变信号
     * @param url 新的URL
     */
    void serverUrlChanged(const QString& url);

    /**
     * @brief 服务器端口改变信号
     * @param port 新的端口
     */
    void serverPortChanged(int port);

    /**
     * @brief 连接超时改变信号
     * @param timeout 新的超时时间
     */
    void connectionTimeoutChanged(int timeout);

    /**
     * @brief 自动重连设置改变信号
     * @param enabled 是否启用
     */
    void autoReconnectChanged(bool enabled);

    /**
     * @brief 重连间隔改变信号
     * @param interval 新的间隔
     */
    void reconnectIntervalChanged(int interval);

    /**
     * @brief WebRTC启用状态改变信号
     * @param enabled 是否启用
     */
    void webRTCEnabledChanged(bool enabled);

    /**
     * @brief WebSocket启用状态改变信号
     * @param enabled 是否启用
     */
    void webSocketEnabledChanged(bool enabled);

    /**
     * @brief HTTPS专用模式改变信号
     * @param httpsOnly 是否仅使用HTTPS
     */
    void httpsOnlyChanged(bool httpsOnly);

    /**
     * @brief 配置改变信号
     */
    void configurationChanged();

    /**
     * @brief 配置验证失败信号
     * @param errors 错误列表
     */
    void validationFailed(const QStringList& errors);

private:
    /**
     * @brief 初始化默认值
     */
    void initializeDefaults();

    /**
     * @brief 验证URL格式
     * @param url URL字符串
     * @return 是否有效
     */
    bool isValidUrl(const QString& url) const;

    /**
     * @brief 验证端口号
     * @param port 端口号
     * @return 是否有效
     */
    bool isValidPort(int port) const;

    class Private;
    Private* d;
};

#endif // NETWORKCONFIG_H