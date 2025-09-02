#ifndef JITSIMEETAPI_H
#define JITSIMEETAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QSslConfiguration>
#include <QSslError>
#include <QAuthenticator>
#include <QMutex>
#include <QQueue>
#include <QHash>
#include <QDateTime>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;
QT_END_NAMESPACE

class ConfigurationManager;

/**
 * @brief Jitsi Meet API类 - 负责与Jitsi Meet服务器的通信
 * 
 * 这个类提供了与Jitsi Meet服务器进行通信的接口，包括：
 * - 服务器连接和认证
 * - 会议信息获取
 * - 用户状态管理
 * - 实时事件处理
 * - API请求管理
 */
class JitsiMeetAPI : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit JitsiMeetAPI(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~JitsiMeetAPI();
    
    /**
     * @brief 设置服务器URL
     * @param serverUrl 服务器URL
     */
    void setServerUrl(const QString& serverUrl);
    
    /**
     * @brief 获取服务器URL
     * @return 服务器URL
     */
    QString getServerUrl() const;
    
    /**
     * @brief 检查服务器连接状态
     * @return 是否已连接
     */
    bool isConnected() const;
    
    /**
     * @brief 连接到服务器
     * @param serverUrl 服务器URL（可选，使用已设置的URL）
     * @return 是否开始连接
     */
    bool connectToServer(const QString& serverUrl = QString());
    
    /**
     * @brief 断开服务器连接
     */
    void disconnectFromServer();
    
    /**
     * @brief 检查服务器可用性
     * @param serverUrl 服务器URL
     */
    void checkServerAvailability(const QString& serverUrl);
    
    /**
     * @brief 获取服务器信息
     */
    void getServerInfo();
    
    /**
     * @brief 获取会议信息
     * @param roomName 房间名称
     */
    void getRoomInfo(const QString& roomName);
    
    /**
     * @brief 创建会议房间
     * @param roomName 房间名称
     * @param options 房间选项
     */
    void createRoom(const QString& roomName, const QJsonObject& options = QJsonObject());
    
    /**
     * @brief 加入会议房间
     * @param roomName 房间名称
     * @param displayName 显示名称
     * @param password 密码（可选）
     */
    void joinRoom(const QString& roomName, const QString& displayName, const QString& password = QString());
    
    /**
     * @brief 离开会议房间
     * @param roomName 房间名称
     */
    void leaveRoom(const QString& roomName);
    
    /**
     * @brief 获取房间参与者列表
     * @param roomName 房间名称
     */
    void getRoomParticipants(const QString& roomName);
    
    /**
     * @brief 发送聊天消息
     * @param roomName 房间名称
     * @param message 消息内容
     */
    void sendChatMessage(const QString& roomName, const QString& message);
    
    /**
     * @brief 设置用户状态
     * @param roomName 房间名称
     * @param status 状态信息
     */
    void setUserStatus(const QString& roomName, const QJsonObject& status);
    
    /**
     * @brief 获取会议统计信息
     * @param roomName 房间名称
     */
    void getRoomStats(const QString& roomName);
    
    /**
     * @brief 设置认证信息
     * @param username 用户名
     * @param password 密码
     * @param token JWT令牌（可选）
     */
    void setAuthentication(const QString& username, const QString& password, const QString& token = QString());
    
    /**
     * @brief 清除认证信息
     */
    void clearAuthentication();
    
    /**
     * @brief 设置请求超时时间
     * @param timeout 超时时间（毫秒）
     */
    void setRequestTimeout(int timeout);
    
    /**
     * @brief 获取请求超时时间
     * @return 超时时间（毫秒）
     */
    int getRequestTimeout() const;
    
    /**
     * @brief 设置重试次数
     * @param retries 重试次数
     */
    void setMaxRetries(int retries);
    
    /**
     * @brief 获取重试次数
     * @return 重试次数
     */
    int getMaxRetries() const;
    
    /**
     * @brief 启用/禁用SSL验证
     * @param enabled 是否启用
     */
    void setSslVerificationEnabled(bool enabled);
    
    /**
     * @brief 检查是否启用SSL验证
     * @return 是否启用
     */
    bool isSslVerificationEnabled() const;

public slots:
    /**
     * @brief 处理网络回复完成
     */
    void onNetworkReplyFinished();
    
    /**
     * @brief 处理网络错误
     * @param error 网络错误
     */
    void onNetworkError(QNetworkReply::NetworkError error);
    
    /**
     * @brief 处理SSL错误
     * @param errors SSL错误列表
     */
    void onSslErrors(const QList<QSslError>& errors);
    
    /**
     * @brief 处理认证请求
     * @param authenticator 认证器
     */
    void onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);
    
    /**
     * @brief 处理请求超时
     */
    void onRequestTimeout();
    
    /**
     * @brief 处理重试定时器
     */
    void onRetryTimer();

signals:
    /**
     * @brief 服务器连接成功信号
     * @param serverUrl 服务器URL
     */
    void serverConnected(const QString& serverUrl);
    
    /**
     * @brief 服务器连接失败信号
     * @param serverUrl 服务器URL
     * @param error 错误信息
     */
    void serverConnectionFailed(const QString& serverUrl, const QString& error);
    
    /**
     * @brief 服务器断开连接信号
     * @param serverUrl 服务器URL
     */
    void serverDisconnected(const QString& serverUrl);
    
    /**
     * @brief 服务器可用性检查完成信号
     * @param serverUrl 服务器URL
     * @param available 是否可用
     * @param responseTime 响应时间（毫秒）
     */
    void serverAvailabilityChecked(const QString& serverUrl, bool available, int responseTime);
    
    /**
     * @brief 服务器信息获取完成信号
     * @param serverInfo 服务器信息
     */
    void serverInfoReceived(const QJsonObject& serverInfo);
    
    /**
     * @brief 房间信息获取完成信号
     * @param roomName 房间名称
     * @param roomInfo 房间信息
     */
    void roomInfoReceived(const QString& roomName, const QJsonObject& roomInfo);
    
    /**
     * @brief 房间创建完成信号
     * @param roomName 房间名称
     * @param success 是否成功
     */
    void roomCreated(const QString& roomName, bool success);
    
    /**
     * @brief 房间加入完成信号
     * @param roomName 房间名称
     * @param success 是否成功
     */
    void roomJoined(const QString& roomName, bool success);
    
    /**
     * @brief 房间离开完成信号
     * @param roomName 房间名称
     */
    void roomLeft(const QString& roomName);
    
    /**
     * @brief 参与者列表更新信号
     * @param roomName 房间名称
     * @param participants 参与者列表
     */
    void participantsUpdated(const QString& roomName, const QJsonArray& participants);
    
    /**
     * @brief 聊天消息发送完成信号
     * @param roomName 房间名称
     * @param success 是否成功
     */
    void chatMessageSent(const QString& roomName, bool success);
    
    /**
     * @brief 聊天消息接收信号
     * @param roomName 房间名称
     * @param senderId 发送者ID
     * @param message 消息内容
     * @param timestamp 时间戳
     */
    void chatMessageReceived(const QString& roomName, const QString& senderId, const QString& message, qint64 timestamp);
    
    /**
     * @brief 用户状态更新完成信号
     * @param roomName 房间名称
     * @param success 是否成功
     */
    void userStatusUpdated(const QString& roomName, bool success);
    
    /**
     * @brief 房间统计信息更新信号
     * @param roomName 房间名称
     * @param stats 统计信息
     */
    void roomStatsUpdated(const QString& roomName, const QJsonObject& stats);
    
    /**
     * @brief API错误信号
     * @param operation 操作名称
     * @param error 错误信息
     * @param details 错误详情
     */
    void apiError(const QString& operation, const QString& error, const QJsonObject& details = QJsonObject());
    
    /**
     * @brief 认证失败信号
     * @param serverUrl 服务器URL
     * @param reason 失败原因
     */
    void authenticationFailed(const QString& serverUrl, const QString& reason);

private slots:
    /**
     * @brief 处理心跳定时器
     */
    void onHeartbeatTimer();
    
    /**
     * @brief 处理连接检查定时器
     */
    void onConnectionCheckTimer();

private:
    /**
     * @brief API请求结构体
     */
    struct ApiRequest {
        QString id;                 ///< 请求ID
        QString operation;          ///< 操作名称
        QNetworkRequest request;    ///< 网络请求
        QByteArray data;           ///< 请求数据
        QString method;            ///< HTTP方法
        QDateTime timestamp;       ///< 请求时间戳
        int retryCount;            ///< 重试次数
        QJsonObject context;       ///< 上下文信息
    };
    
    /**
     * @brief 初始化网络管理器
     */
    void initializeNetworkManager();
    
    /**
     * @brief 设置SSL配置
     */
    void setupSslConfiguration();
    
    /**
     * @brief 创建网络请求
     * @param url 请求URL
     * @param method HTTP方法
     * @param data 请求数据
     * @param headers 请求头
     * @return 网络请求对象
     */
    QNetworkRequest createRequest(const QUrl& url, const QString& method = "GET", 
                                 const QJsonObject& headers = QJsonObject()) const;
    
    /**
     * @brief 发送API请求
     * @param operation 操作名称
     * @param endpoint API端点
     * @param method HTTP方法
     * @param data 请求数据
     * @param context 上下文信息
     * @return 请求ID
     */
    QString sendApiRequest(const QString& operation, const QString& endpoint, 
                          const QString& method = "GET", const QJsonObject& data = QJsonObject(),
                          const QJsonObject& context = QJsonObject());
    
    /**
     * @brief 重试API请求
     * @param requestId 请求ID
     */
    void retryApiRequest(const QString& requestId);
    
    /**
     * @brief 处理API响应
     * @param reply 网络回复
     * @param request 原始请求
     */
    void handleApiResponse(QNetworkReply* reply, const ApiRequest& request);
    
    /**
     * @brief 解析JSON响应
     * @param data 响应数据
     * @param error 错误信息输出
     * @return 解析结果
     */
    QJsonObject parseJsonResponse(const QByteArray& data, QString* error = nullptr) const;
    
    /**
     * @brief 构建API URL
     * @param endpoint API端点
     * @return 完整URL
     */
    QUrl buildApiUrl(const QString& endpoint) const;
    
    /**
     * @brief 添加认证头
     * @param request 网络请求
     */
    void addAuthenticationHeaders(QNetworkRequest& request) const;
    
    /**
     * @brief 生成请求ID
     * @return 唯一请求ID
     */
    QString generateRequestId() const;
    
    /**
     * @brief 清理过期请求
     */
    void cleanupExpiredRequests();
    
    /**
     * @brief 检查服务器连接
     */
    void checkServerConnection();
    
    /**
     * @brief 发送心跳请求
     */
    void sendHeartbeat();
    
    /**
     * @brief 处理连接状态变化
     * @param connected 是否连接
     */
    void handleConnectionStateChange(bool connected);
    
    /**
     * @brief 记录API调用
     * @param operation 操作名称
     * @param endpoint API端点
     * @param method HTTP方法
     * @param success 是否成功
     * @param responseTime 响应时间
     */
    void logApiCall(const QString& operation, const QString& endpoint, 
                   const QString& method, bool success, int responseTime) const;

private:
    // 网络管理
    QNetworkAccessManager* m_networkManager;   ///< 网络访问管理器
    QSslConfiguration m_sslConfig;             ///< SSL配置
    
    // 服务器信息
    QString m_serverUrl;                       ///< 服务器URL
    QString m_apiBasePath;                     ///< API基础路径
    bool m_isConnected;                        ///< 是否已连接
    
    // 认证信息
    QString m_username;                        ///< 用户名
    QString m_password;                        ///< 密码
    QString m_authToken;                       ///< 认证令牌
    QString m_jwtToken;                        ///< JWT令牌
    QDateTime m_tokenExpiry;                   ///< 令牌过期时间
    
    // 请求管理
    QHash<QString, ApiRequest> m_pendingRequests; ///< 待处理请求
    QHash<QNetworkReply*, QString> m_replyToRequestId; ///< 回复到请求ID映射
    QMutex m_requestMutex;                     ///< 请求互斥锁
    
    // 配置参数
    int m_requestTimeout;                      ///< 请求超时时间（毫秒）
    int m_maxRetries;                          ///< 最大重试次数
    bool m_sslVerificationEnabled;             ///< 是否启用SSL验证
    int m_heartbeatInterval;                   ///< 心跳间隔（毫秒）
    int m_connectionCheckInterval;             ///< 连接检查间隔（毫秒）
    
    // 定时器
    QTimer* m_requestTimer;                    ///< 请求超时定时器
    QTimer* m_retryTimer;                      ///< 重试定时器
    QTimer* m_heartbeatTimer;                  ///< 心跳定时器
    QTimer* m_connectionCheckTimer;            ///< 连接检查定时器
    QTimer* m_cleanupTimer;                    ///< 清理定时器
    
    // 统计信息
    int m_totalRequests;                       ///< 总请求数
    int m_successfulRequests;                  ///< 成功请求数
    int m_failedRequests;                      ///< 失败请求数
    QDateTime m_lastSuccessfulRequest;         ///< 最后成功请求时间
    QDateTime m_lastFailedRequest;             ///< 最后失败请求时间
    
    // 配置管理器
    ConfigurationManager* m_configManager;    ///< 配置管理器
    
    // 常量
    static const int DEFAULT_REQUEST_TIMEOUT = 30000;      ///< 默认请求超时（30秒）
    static const int DEFAULT_MAX_RETRIES = 3;              ///< 默认最大重试次数
    static const int DEFAULT_HEARTBEAT_INTERVAL = 60000;   ///< 默认心跳间隔（60秒）
    static const int DEFAULT_CONNECTION_CHECK_INTERVAL = 30000; ///< 默认连接检查间隔（30秒）
    static const int REQUEST_CLEANUP_INTERVAL = 300000;    ///< 请求清理间隔（5分钟）
    static const int MAX_PENDING_REQUESTS = 100;           ///< 最大待处理请求数
};

#endif // JITSIMEETAPI_H