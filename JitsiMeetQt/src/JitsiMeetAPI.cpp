#include "JitsiMeetAPI.h"
#include "ConfigurationManager.h"
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
#include <QMutexLocker>
#include <QUuid>
#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QHttpMultiPart>
#include <QHttpPart>

/**
 * @brief 构造函数 - 初始化JitsiMeetAPI实例
 * @param parent 父对象
 */
JitsiMeetAPI::JitsiMeetAPI(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_serverUrl()
    , m_apiBasePath("/api/v1")
    , m_isConnected(false)
    , m_username()
    , m_password()
    , m_authToken()
    , m_jwtToken()
    , m_tokenExpiry()
    , m_requestTimeout(DEFAULT_REQUEST_TIMEOUT)
    , m_maxRetries(DEFAULT_MAX_RETRIES)
    , m_sslVerificationEnabled(true)
    , m_heartbeatInterval(DEFAULT_HEARTBEAT_INTERVAL)
    , m_connectionCheckInterval(DEFAULT_CONNECTION_CHECK_INTERVAL)
    , m_requestTimer(nullptr)
    , m_retryTimer(nullptr)
    , m_heartbeatTimer(nullptr)
    , m_connectionCheckTimer(nullptr)
    , m_cleanupTimer(nullptr)
    , m_totalRequests(0)
    , m_successfulRequests(0)
    , m_failedRequests(0)
    , m_configManager(nullptr)
{
    qDebug() << "JitsiMeetAPI: 初始化API客户端";
    
    // 获取配置管理器实例
    m_configManager = ConfigurationManager::instance();
    
    // 初始化网络管理器
    initializeNetworkManager();
    
    // 创建定时器
    m_requestTimer = new QTimer(this);
    m_requestTimer->setSingleShot(true);
    connect(m_requestTimer, &QTimer::timeout, this, &JitsiMeetAPI::onRequestTimeout);
    
    m_retryTimer = new QTimer(this);
    m_retryTimer->setSingleShot(true);
    connect(m_retryTimer, &QTimer::timeout, this, &JitsiMeetAPI::onRetryTimer);
    
    m_heartbeatTimer = new QTimer(this);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &JitsiMeetAPI::onHeartbeatTimer);
    
    m_connectionCheckTimer = new QTimer(this);
    connect(m_connectionCheckTimer, &QTimer::timeout, this, &JitsiMeetAPI::onConnectionCheckTimer);
    
    m_cleanupTimer = new QTimer(this);
    m_cleanupTimer->setInterval(REQUEST_CLEANUP_INTERVAL);
    connect(m_cleanupTimer, &QTimer::timeout, this, &JitsiMeetAPI::cleanupExpiredRequests);
    m_cleanupTimer->start();
    
    // 从配置加载设置
    if (m_configManager) {
        setServerUrl(m_configManager->getDefaultServerUrl());
        setRequestTimeout(m_configManager->getValue("api/requestTimeout", DEFAULT_REQUEST_TIMEOUT).toInt());
        setMaxRetries(m_configManager->getValue("api/maxRetries", DEFAULT_MAX_RETRIES).toInt());
        setSslVerificationEnabled(m_configManager->getValue("api/sslVerification", true).toBool());
    }
    
    qDebug() << "JitsiMeetAPI: 初始化完成";
}

/**
 * @brief 析构函数 - 清理资源
 */
JitsiMeetAPI::~JitsiMeetAPI()
{
    qDebug() << "JitsiMeetAPI: 销毁API客户端";
    
    // 断开服务器连接
    disconnectFromServer();
    
    // 停止所有定时器
    if (m_heartbeatTimer) {
        m_heartbeatTimer->stop();
    }
    if (m_connectionCheckTimer) {
        m_connectionCheckTimer->stop();
    }
    if (m_cleanupTimer) {
        m_cleanupTimer->stop();
    }
    
    // 取消所有待处理的请求
    QMutexLocker locker(&m_requestMutex);
    for (auto it = m_replyToRequestId.begin(); it != m_replyToRequestId.end(); ++it) {
        QNetworkReply* reply = it.key();
        if (reply) {
            reply->abort();
            reply->deleteLater();
        }
    }
    m_replyToRequestId.clear();
    m_pendingRequests.clear();
}

/**
 * @brief 初始化网络管理器
 */
void JitsiMeetAPI::initializeNetworkManager()
{
    qDebug() << "JitsiMeetAPI: 初始化网络管理器";
    
    m_networkManager = new QNetworkAccessManager(this);
    
    // 连接信号
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &JitsiMeetAPI::onNetworkReplyFinished);
    connect(m_networkManager, &QNetworkAccessManager::authenticationRequired,
            this, &JitsiMeetAPI::onAuthenticationRequired);
    // SSL错误处理将在具体的QNetworkReply中处理
    
    // 设置SSL配置
    setupSslConfiguration();
}

/**
 * @brief 设置SSL配置
 */
void JitsiMeetAPI::setupSslConfiguration()
{
    m_sslConfig = QSslConfiguration::defaultConfiguration();
    
    if (!m_sslVerificationEnabled) {
        m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        qDebug() << "JitsiMeetAPI: SSL验证已禁用";
    } else {
        m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
        qDebug() << "JitsiMeetAPI: SSL验证已启用";
    }
    
    m_sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
}

/**
 * @brief 设置服务器URL
 * @param serverUrl 服务器URL
 */
void JitsiMeetAPI::setServerUrl(const QString& serverUrl)
{
    if (m_serverUrl != serverUrl) {
        qDebug() << "JitsiMeetAPI: 设置服务器URL:" << serverUrl;
        m_serverUrl = serverUrl;
        
        // 如果当前已连接，需要重新连接
        if (m_isConnected) {
            disconnectFromServer();
            connectToServer();
        }
    }
}

/**
 * @brief 获取服务器URL
 * @return 服务器URL
 */
QString JitsiMeetAPI::getServerUrl() const
{
    return m_serverUrl;
}

/**
 * @brief 检查服务器连接状态
 * @return 是否已连接
 */
bool JitsiMeetAPI::isConnected() const
{
    return m_isConnected;
}

/**
 * @brief 连接到服务器
 * @param serverUrl 服务器URL（可选，使用已设置的URL）
 * @return 是否开始连接
 */
bool JitsiMeetAPI::connectToServer(const QString& serverUrl)
{
    if (!serverUrl.isEmpty()) {
        setServerUrl(serverUrl);
    }
    
    if (m_serverUrl.isEmpty()) {
        qWarning() << "JitsiMeetAPI: 服务器URL为空，无法连接";
        emit serverConnectionFailed(m_serverUrl, "服务器URL为空");
        return false;
    }
    
    qDebug() << "JitsiMeetAPI: 开始连接到服务器:" << m_serverUrl;
    
    // 检查服务器可用性
    checkServerAvailability(m_serverUrl);
    
    return true;
}

/**
 * @brief 断开服务器连接
 */
void JitsiMeetAPI::disconnectFromServer()
{
    if (m_isConnected) {
        qDebug() << "JitsiMeetAPI: 断开服务器连接:" << m_serverUrl;
        
        m_isConnected = false;
        
        // 停止心跳和连接检查
        if (m_heartbeatTimer) {
            m_heartbeatTimer->stop();
        }
        if (m_connectionCheckTimer) {
            m_connectionCheckTimer->stop();
        }
        
        // 清除认证信息
        clearAuthentication();
        
        emit serverDisconnected(m_serverUrl);
    }
}

/**
 * @brief 检查服务器可用性
 * @param serverUrl 服务器URL
 */
void JitsiMeetAPI::checkServerAvailability(const QString& serverUrl)
{
    qDebug() << "JitsiMeetAPI: 检查服务器可用性:" << serverUrl;
    
    QDateTime startTime = QDateTime::currentDateTime();
    
    QJsonObject context;
    context["operation"] = "checkAvailability";
    context["serverUrl"] = serverUrl;
    context["startTime"] = startTime.toString(Qt::ISODate);
    
    sendApiRequest("checkServerAvailability", "/health", "GET", QJsonObject(), context);
}

/**
 * @brief 获取服务器信息
 */
void JitsiMeetAPI::getServerInfo()
{
    qDebug() << "JitsiMeetAPI: 获取服务器信息";
    
    QJsonObject context;
    context["operation"] = "getServerInfo";
    
    sendApiRequest("getServerInfo", "/info", "GET", QJsonObject(), context);
}

/**
 * @brief 获取会议信息
 * @param roomName 房间名称
 */
void JitsiMeetAPI::getRoomInfo(const QString& roomName)
{
    qDebug() << "JitsiMeetAPI: 获取房间信息:" << roomName;
    
    QJsonObject context;
    context["operation"] = "getRoomInfo";
    context["roomName"] = roomName;
    
    QString endpoint = QString("/rooms/%1").arg(roomName);
    sendApiRequest("getRoomInfo", endpoint, "GET", QJsonObject(), context);
}

/**
 * @brief 创建会议房间
 * @param roomName 房间名称
 * @param options 房间选项
 */
void JitsiMeetAPI::createRoom(const QString& roomName, const QJsonObject& options)
{
    qDebug() << "JitsiMeetAPI: 创建房间:" << roomName;
    
    QJsonObject data;
    data["name"] = roomName;
    data["options"] = options;
    
    QJsonObject context;
    context["operation"] = "createRoom";
    context["roomName"] = roomName;
    
    sendApiRequest("createRoom", "/rooms", "POST", data, context);
}

/**
 * @brief 加入会议房间
 * @param roomName 房间名称
 * @param displayName 显示名称
 * @param password 密码（可选）
 */
void JitsiMeetAPI::joinRoom(const QString& roomName, const QString& displayName, const QString& password)
{
    qDebug() << "JitsiMeetAPI: 加入房间:" << roomName << "显示名称:" << displayName;
    
    QJsonObject data;
    data["displayName"] = displayName;
    if (!password.isEmpty()) {
        data["password"] = password;
    }
    
    QJsonObject context;
    context["operation"] = "joinRoom";
    context["roomName"] = roomName;
    context["displayName"] = displayName;
    
    QString endpoint = QString("/rooms/%1/join").arg(roomName);
    sendApiRequest("joinRoom", endpoint, "POST", data, context);
}

/**
 * @brief 离开会议房间
 * @param roomName 房间名称
 */
void JitsiMeetAPI::leaveRoom(const QString& roomName)
{
    qDebug() << "JitsiMeetAPI: 离开房间:" << roomName;
    
    QJsonObject context;
    context["operation"] = "leaveRoom";
    context["roomName"] = roomName;
    
    QString endpoint = QString("/rooms/%1/leave").arg(roomName);
    sendApiRequest("leaveRoom", endpoint, "POST", QJsonObject(), context);
}

/**
 * @brief 获取房间参与者列表
 * @param roomName 房间名称
 */
void JitsiMeetAPI::getRoomParticipants(const QString& roomName)
{
    qDebug() << "JitsiMeetAPI: 获取房间参与者:" << roomName;
    
    QJsonObject context;
    context["operation"] = "getRoomParticipants";
    context["roomName"] = roomName;
    
    QString endpoint = QString("/rooms/%1/participants").arg(roomName);
    sendApiRequest("getRoomParticipants", endpoint, "GET", QJsonObject(), context);
}

/**
 * @brief 发送聊天消息
 * @param roomName 房间名称
 * @param message 消息内容
 */
void JitsiMeetAPI::sendChatMessage(const QString& roomName, const QString& message)
{
    qDebug() << "JitsiMeetAPI: 发送聊天消息到房间:" << roomName;
    
    QJsonObject data;
    data["message"] = message;
    data["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    QJsonObject context;
    context["operation"] = "sendChatMessage";
    context["roomName"] = roomName;
    
    QString endpoint = QString("/rooms/%1/chat").arg(roomName);
    sendApiRequest("sendChatMessage", endpoint, "POST", data, context);
}

/**
 * @brief 设置用户状态
 * @param roomName 房间名称
 * @param status 状态信息
 */
void JitsiMeetAPI::setUserStatus(const QString& roomName, const QJsonObject& status)
{
    qDebug() << "JitsiMeetAPI: 设置用户状态:" << roomName;
    
    QJsonObject context;
    context["operation"] = "setUserStatus";
    context["roomName"] = roomName;
    
    QString endpoint = QString("/rooms/%1/status").arg(roomName);
    sendApiRequest("setUserStatus", endpoint, "PUT", status, context);
}

/**
 * @brief 获取会议统计信息
 * @param roomName 房间名称
 */
void JitsiMeetAPI::getRoomStats(const QString& roomName)
{
    qDebug() << "JitsiMeetAPI: 获取房间统计:" << roomName;
    
    QJsonObject context;
    context["operation"] = "getRoomStats";
    context["roomName"] = roomName;
    
    QString endpoint = QString("/rooms/%1/stats").arg(roomName);
    sendApiRequest("getRoomStats", endpoint, "GET", QJsonObject(), context);
}

/**
 * @brief 设置认证信息
 * @param username 用户名
 * @param password 密码
 * @param token JWT令牌（可选）
 */
void JitsiMeetAPI::setAuthentication(const QString& username, const QString& password, const QString& token)
{
    qDebug() << "JitsiMeetAPI: 设置认证信息，用户名:" << username;
    
    m_username = username;
    m_password = password;
    m_jwtToken = token;
    
    if (!token.isEmpty()) {
        // 解析JWT令牌获取过期时间（简化实现）
        m_tokenExpiry = QDateTime::currentDateTime().addSecs(3600); // 默认1小时
    }
}

/**
 * @brief 清除认证信息
 */
void JitsiMeetAPI::clearAuthentication()
{
    qDebug() << "JitsiMeetAPI: 清除认证信息";
    
    m_username.clear();
    m_password.clear();
    m_authToken.clear();
    m_jwtToken.clear();
    m_tokenExpiry = QDateTime();
}

/**
 * @brief 设置请求超时时间
 * @param timeout 超时时间（毫秒）
 */
void JitsiMeetAPI::setRequestTimeout(int timeout)
{
    m_requestTimeout = qMax(1000, timeout); // 最小1秒
    qDebug() << "JitsiMeetAPI: 设置请求超时:" << m_requestTimeout << "毫秒";
}

/**
 * @brief 获取请求超时时间
 * @return 超时时间（毫秒）
 */
int JitsiMeetAPI::getRequestTimeout() const
{
    return m_requestTimeout;
}

/**
 * @brief 设置重试次数
 * @param retries 重试次数
 */
void JitsiMeetAPI::setMaxRetries(int retries)
{
    m_maxRetries = qMax(0, retries);
    qDebug() << "JitsiMeetAPI: 设置最大重试次数:" << m_maxRetries;
}

/**
 * @brief 获取重试次数
 * @return 重试次数
 */
int JitsiMeetAPI::getMaxRetries() const
{
    return m_maxRetries;
}

/**
 * @brief 启用/禁用SSL验证
 * @param enabled 是否启用
 */
void JitsiMeetAPI::setSslVerificationEnabled(bool enabled)
{
    if (m_sslVerificationEnabled != enabled) {
        m_sslVerificationEnabled = enabled;
        setupSslConfiguration();
        qDebug() << "JitsiMeetAPI: SSL验证" << (enabled ? "已启用" : "已禁用");
    }
}

/**
 * @brief 检查是否启用SSL验证
 * @return 是否启用
 */
bool JitsiMeetAPI::isSslVerificationEnabled() const
{
    return m_sslVerificationEnabled;
}

/**
 * @brief 创建网络请求
 * @param url 请求URL
 * @param method HTTP方法
 * @param headers 请求头
 * @return 网络请求对象
 */
QNetworkRequest JitsiMeetAPI::createRequest(const QUrl& url, const QString& method, const QJsonObject& headers) const
{
    QNetworkRequest request(url);
    
    // 设置SSL配置
    request.setSslConfiguration(m_sslConfig);
    
    // 设置基本头部
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "JitsiMeetQt/1.0");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8");
    
    // 添加自定义头部
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
    }
    
    // 添加认证头部
    addAuthenticationHeaders(request);
    
    return request;
}

/**
 * @brief 发送API请求
 * @param operation 操作名称
 * @param endpoint API端点
 * @param method HTTP方法
 * @param data 请求数据
 * @param context 上下文信息
 * @return 请求ID
 */
QString JitsiMeetAPI::sendApiRequest(const QString& operation, const QString& endpoint, 
                                   const QString& method, const QJsonObject& data,
                                   const QJsonObject& context)
{
    QString requestId = generateRequestId();
    
    qDebug() << "JitsiMeetAPI: 发送API请求" << operation << endpoint << method << "ID:" << requestId;
    
    // 构建URL
    QUrl url = buildApiUrl(endpoint);
    if (!url.isValid()) {
        qWarning() << "JitsiMeetAPI: 无效的API URL:" << url.toString();
        emit apiError(operation, "无效的API URL", QJsonObject());
        return QString();
    }
    
    // 创建请求
    QNetworkRequest request = createRequest(url, method);
    
    // 准备请求数据
    QByteArray requestData;
    if (!data.isEmpty()) {
        QJsonDocument doc(data);
        requestData = doc.toJson(QJsonDocument::Compact);
    }
    
    // 创建API请求结构
    ApiRequest apiRequest;
    apiRequest.id = requestId;
    apiRequest.operation = operation;
    apiRequest.request = request;
    apiRequest.data = requestData;
    apiRequest.method = method.toUpper();
    apiRequest.timestamp = QDateTime::currentDateTime();
    apiRequest.retryCount = 0;
    apiRequest.context = context;
    
    // 检查待处理请求数量
    QMutexLocker locker(&m_requestMutex);
    if (m_pendingRequests.size() >= MAX_PENDING_REQUESTS) {
        qWarning() << "JitsiMeetAPI: 待处理请求过多，拒绝新请求";
        emit apiError(operation, "待处理请求过多", QJsonObject());
        return QString();
    }
    
    // 存储请求
    m_pendingRequests[requestId] = apiRequest;
    locker.unlock();
    
    // 发送网络请求
    QNetworkReply* reply = nullptr;
    if (method == "GET") {
        reply = m_networkManager->get(request);
    } else if (method == "POST") {
        reply = m_networkManager->post(request, requestData);
    } else if (method == "PUT") {
        reply = m_networkManager->put(request, requestData);
    } else if (method == "DELETE") {
        reply = m_networkManager->deleteResource(request);
    } else {
        qWarning() << "JitsiMeetAPI: 不支持的HTTP方法:" << method;
        QMutexLocker locker2(&m_requestMutex);
        m_pendingRequests.remove(requestId);
        emit apiError(operation, "不支持的HTTP方法", QJsonObject());
        return QString();
    }
    
    if (reply) {
        // 建立回复到请求ID的映射
        QMutexLocker locker2(&m_requestMutex);
        m_replyToRequestId[reply] = requestId;
        
        // 连接错误信号
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
                this, &JitsiMeetAPI::onNetworkError);
        
        // 启动超时定时器
        m_requestTimer->start(m_requestTimeout);
        
        m_totalRequests++;
        
        logApiCall(operation, endpoint, method, false, 0);
    } else {
        qWarning() << "JitsiMeetAPI: 创建网络请求失败";
        QMutexLocker locker2(&m_requestMutex);
        m_pendingRequests.remove(requestId);
        emit apiError(operation, "创建网络请求失败", QJsonObject());
        return QString();
    }
    
    return requestId;
}

/**
 * @brief 构建API URL
 * @param endpoint API端点
 * @return 完整URL
 */
QUrl JitsiMeetAPI::buildApiUrl(const QString& endpoint) const
{
    if (m_serverUrl.isEmpty()) {
        return QUrl();
    }
    
    QString urlString = m_serverUrl;
    if (!urlString.endsWith('/')) {
        urlString += '/';
    }
    
    // 移除端点开头的斜杠
    QString cleanEndpoint = endpoint;
    if (cleanEndpoint.startsWith('/')) {
        cleanEndpoint = cleanEndpoint.mid(1);
    }
    
    // 对于健康检查等特殊端点，不添加API基础路径
    if (endpoint == "/health" || endpoint == "/info") {
        urlString += cleanEndpoint;
    } else {
        urlString += m_apiBasePath.mid(1) + "/" + cleanEndpoint;
    }
    
    return QUrl(urlString);
}

/**
 * @brief 添加认证头部
 * @param request 网络请求
 */
void JitsiMeetAPI::addAuthenticationHeaders(QNetworkRequest& request) const
{
    if (!m_jwtToken.isEmpty() && m_tokenExpiry > QDateTime::currentDateTime()) {
        // 使用JWT令牌
        request.setRawHeader("Authorization", ("Bearer " + m_jwtToken).toUtf8());
    } else if (!m_username.isEmpty() && !m_password.isEmpty()) {
        // 使用基本认证
        QString credentials = m_username + ":" + m_password;
        QByteArray encodedCredentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + encodedCredentials);
    } else if (!m_authToken.isEmpty()) {
        // 使用自定义认证令牌
        request.setRawHeader("X-Auth-Token", m_authToken.toUtf8());
    }
}

/**
 * @brief 生成请求ID
 * @return 唯一请求ID
 */
QString JitsiMeetAPI::generateRequestId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

/**
 * @brief 处理网络回复完成
 */
void JitsiMeetAPI::onNetworkReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // 停止超时定时器
    m_requestTimer->stop();
    
    // 获取请求ID
    QMutexLocker locker(&m_requestMutex);
    QString requestId = m_replyToRequestId.value(reply);
    if (requestId.isEmpty()) {
        qWarning() << "JitsiMeetAPI: 找不到请求ID";
        reply->deleteLater();
        return;
    }
    
    // 获取原始请求
    ApiRequest apiRequest = m_pendingRequests.value(requestId);
    if (apiRequest.id.isEmpty()) {
        qWarning() << "JitsiMeetAPI: 找不到原始请求";
        m_replyToRequestId.remove(reply);
        reply->deleteLater();
        return;
    }
    
    // 移除映射
    m_replyToRequestId.remove(reply);
    m_pendingRequests.remove(requestId);
    locker.unlock();
    
    // 处理响应
    handleApiResponse(reply, apiRequest);
    
    // 清理回复对象
    reply->deleteLater();
}

/**
 * @brief 处理API响应
 * @param reply 网络回复
 * @param request 原始请求
 */
void JitsiMeetAPI::handleApiResponse(QNetworkReply* reply, const ApiRequest& request)
{
    QString operation = request.operation;
    QJsonObject context = request.context;
    
    // 计算响应时间
    int responseTime = request.timestamp.msecsTo(QDateTime::currentDateTime());
    
    // 检查HTTP状态码
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray responseData = reply->readAll();
    
    qDebug() << "JitsiMeetAPI: 收到响应" << operation << "状态码:" << statusCode << "响应时间:" << responseTime << "ms";
    
    // 解析JSON响应
    QString parseError;
    QJsonObject responseJson = parseJsonResponse(responseData, &parseError);
    
    bool success = (reply->error() == QNetworkReply::NoError && statusCode >= 200 && statusCode < 300);
    
    if (success) {
        m_successfulRequests++;
        m_lastSuccessfulRequest = QDateTime::currentDateTime();
        
        // 处理特定操作的成功响应
        if (operation == "checkServerAvailability") {
            QString serverUrl = context["serverUrl"].toString();
            emit serverAvailabilityChecked(serverUrl, true, responseTime);
            
            // 如果这是连接检查，设置连接状态
            if (!m_isConnected) {
                m_isConnected = true;
                emit serverConnected(serverUrl);
                
                // 启动心跳和连接检查
                m_heartbeatTimer->start(m_heartbeatInterval);
                m_connectionCheckTimer->start(m_connectionCheckInterval);
            }
        } else if (operation == "getServerInfo") {
            emit serverInfoReceived(responseJson);
        } else if (operation == "getRoomInfo") {
            QString roomName = context["roomName"].toString();
            emit roomInfoReceived(roomName, responseJson);
        } else if (operation == "createRoom") {
            QString roomName = context["roomName"].toString();
            emit roomCreated(roomName, true);
        } else if (operation == "joinRoom") {
            QString roomName = context["roomName"].toString();
            emit roomJoined(roomName, true);
        } else if (operation == "leaveRoom") {
            QString roomName = context["roomName"].toString();
            emit roomLeft(roomName);
        } else if (operation == "getRoomParticipants") {
            QString roomName = context["roomName"].toString();
            QJsonArray participants = responseJson["participants"].toArray();
            emit participantsUpdated(roomName, participants);
        } else if (operation == "sendChatMessage") {
            QString roomName = context["roomName"].toString();
            emit chatMessageSent(roomName, true);
        } else if (operation == "setUserStatus") {
            QString roomName = context["roomName"].toString();
            emit userStatusUpdated(roomName, true);
        } else if (operation == "getRoomStats") {
            QString roomName = context["roomName"].toString();
            emit roomStatsUpdated(roomName, responseJson);
        }
    } else {
        m_failedRequests++;
        m_lastFailedRequest = QDateTime::currentDateTime();
        
        QString errorMessage;
        if (reply->error() != QNetworkReply::NoError) {
            errorMessage = reply->errorString();
        } else if (!parseError.isEmpty()) {
            errorMessage = "JSON解析错误: " + parseError;
        } else {
            errorMessage = QString("HTTP错误: %1").arg(statusCode);
        }
        
        qWarning() << "JitsiMeetAPI: API请求失败" << operation << errorMessage;
        
        // 检查是否需要重试
        if (request.retryCount < m_maxRetries) {
            qDebug() << "JitsiMeetAPI: 准备重试请求" << operation << "重试次数:" << (request.retryCount + 1);
            
            // 重新添加到待处理请求（增加重试计数）
            ApiRequest retryRequest = request;
            retryRequest.retryCount++;
            
            QMutexLocker locker(&m_requestMutex);
            m_pendingRequests[request.id] = retryRequest;
            locker.unlock();
            
            // 延迟重试
            m_retryTimer->start(1000 * (request.retryCount + 1)); // 递增延迟
        } else {
            // 处理特定操作的失败响应
            if (operation == "checkServerAvailability") {
                QString serverUrl = context["serverUrl"].toString();
                emit serverAvailabilityChecked(serverUrl, false, responseTime);
                emit serverConnectionFailed(serverUrl, errorMessage);
            } else if (operation == "createRoom") {
                QString roomName = context["roomName"].toString();
                emit roomCreated(roomName, false);
            } else if (operation == "joinRoom") {
                QString roomName = context["roomName"].toString();
                emit roomJoined(roomName, false);
            } else if (operation == "sendChatMessage") {
                QString roomName = context["roomName"].toString();
                emit chatMessageSent(roomName, false);
            } else if (operation == "setUserStatus") {
                QString roomName = context["roomName"].toString();
                emit userStatusUpdated(roomName, false);
            }
            
            // 发送通用错误信号
            emit apiError(operation, errorMessage, responseJson);
        }
    }
    
    // 记录API调用
    logApiCall(operation, request.request.url().path(), request.method, success, responseTime);
}

/**
 * @brief 解析JSON响应
 * @param data 响应数据
 * @param error 错误信息输出
 * @return 解析结果
 */
QJsonObject JitsiMeetAPI::parseJsonResponse(const QByteArray& data, QString* error) const
{
    if (data.isEmpty()) {
        if (error) {
            *error = "响应数据为空";
        }
        return QJsonObject();
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        if (error) {
            *error = parseError.errorString();
        }
        return QJsonObject();
    }
    
    if (error) {
        error->clear();
    }
    
    return doc.object();
}

/**
 * @brief 处理网络错误
 * @param error 网络错误
 */
void JitsiMeetAPI::onNetworkError(QNetworkReply::NetworkError error)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    qWarning() << "JitsiMeetAPI: 网络错误" << error << reply->errorString();
    
    // 错误将在onNetworkReplyFinished中处理
}

/**
 * @brief 处理SSL错误
 * @param errors SSL错误列表
 */
void JitsiMeetAPI::onSslErrors(const QList<QSslError>& errors)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    for (const QSslError& error : errors) {
        qWarning() << "JitsiMeetAPI: SSL错误" << error.errorString();
    }
    
    if (!m_sslVerificationEnabled) {
        // 如果禁用了SSL验证，忽略SSL错误
        reply->ignoreSslErrors();
    }
}

/**
 * @brief 处理认证请求
 * @param authenticator 认证器
 */
void JitsiMeetAPI::onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    Q_UNUSED(reply)
    
    if (!m_username.isEmpty() && !m_password.isEmpty()) {
        qDebug() << "JitsiMeetAPI: 提供认证信息";
        authenticator->setUser(m_username);
        authenticator->setPassword(m_password);
    } else {
        qWarning() << "JitsiMeetAPI: 需要认证但未提供凭据";
        emit authenticationFailed(m_serverUrl, "需要认证但未提供凭据");
    }
}

/**
 * @brief 处理请求超时
 */
void JitsiMeetAPI::onRequestTimeout()
{
    qWarning() << "JitsiMeetAPI: 请求超时";
    
    // 超时将在网络回复中作为错误处理
}

/**
 * @brief 处理重试定时器
 */
void JitsiMeetAPI::onRetryTimer()
{
    qDebug() << "JitsiMeetAPI: 执行重试";
    
    // 查找需要重试的请求
    QMutexLocker locker(&m_requestMutex);
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
        const ApiRequest& request = it.value();
        if (request.retryCount > 0) {
            // 重新发送请求
            retryApiRequest(request.id);
            break; // 一次只重试一个请求
        }
    }
}

/**
 * @brief 重试API请求
 * @param requestId 请求ID
 */
void JitsiMeetAPI::retryApiRequest(const QString& requestId)
{
    QMutexLocker locker(&m_requestMutex);
    ApiRequest request = m_pendingRequests.value(requestId);
    if (request.id.isEmpty()) {
        return;
    }
    
    // 移除旧请求
    m_pendingRequests.remove(requestId);
    locker.unlock();
    
    qDebug() << "JitsiMeetAPI: 重试请求" << request.operation << "第" << request.retryCount << "次";
    
    // 发送网络请求
    QNetworkReply* reply = nullptr;
    if (request.method == "GET") {
        reply = m_networkManager->get(request.request);
    } else if (request.method == "POST") {
        reply = m_networkManager->post(request.request, request.data);
    } else if (request.method == "PUT") {
        reply = m_networkManager->put(request.request, request.data);
    } else if (request.method == "DELETE") {
        reply = m_networkManager->deleteResource(request.request);
    }
    
    if (reply) {
        // 重新建立映射
        QMutexLocker locker2(&m_requestMutex);
        m_replyToRequestId[reply] = requestId;
        m_pendingRequests[requestId] = request;
        
        // 连接信号
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
                this, &JitsiMeetAPI::onNetworkError);
        connect(reply, &QNetworkReply::sslErrors, this, &JitsiMeetAPI::onSslErrors);
        
        // 重新启动超时定时器
        m_requestTimer->start(m_requestTimeout);
    }
}

/**
 * @brief 处理心跳定时器
 */
void JitsiMeetAPI::onHeartbeatTimer()
{
    if (m_isConnected) {
        sendHeartbeat();
    }
}

/**
 * @brief 处理连接检查定时器
 */
void JitsiMeetAPI::onConnectionCheckTimer()
{
    if (m_isConnected) {
        checkServerConnection();
    }
}

/**
 * @brief 发送心跳请求
 */
void JitsiMeetAPI::sendHeartbeat()
{
    qDebug() << "JitsiMeetAPI: 发送心跳";
    
    QJsonObject context;
    context["operation"] = "heartbeat";
    
    sendApiRequest("heartbeat", "/ping", "GET", QJsonObject(), context);
}

/**
 * @brief 检查服务器连接
 */
void JitsiMeetAPI::checkServerConnection()
{
    qDebug() << "JitsiMeetAPI: 检查服务器连接";
    
    checkServerAvailability(m_serverUrl);
}

/**
 * @brief 处理连接状态变化
 * @param connected 是否连接
 */
void JitsiMeetAPI::handleConnectionStateChange(bool connected)
{
    if (m_isConnected != connected) {
        m_isConnected = connected;
        
        if (connected) {
            qDebug() << "JitsiMeetAPI: 连接已建立";
            emit serverConnected(m_serverUrl);
        } else {
            qDebug() << "JitsiMeetAPI: 连接已断开";
            emit serverDisconnected(m_serverUrl);
        }
    }
}

/**
 * @brief 清理过期请求
 */
void JitsiMeetAPI::cleanupExpiredRequests()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime expireTime = now.addSecs(-300); // 5分钟前
    
    QMutexLocker locker(&m_requestMutex);
    
    QStringList expiredRequests;
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
        if (it.value().timestamp < expireTime) {
            expiredRequests.append(it.key());
        }
    }
    
    for (const QString& requestId : expiredRequests) {
        qDebug() << "JitsiMeetAPI: 清理过期请求" << requestId;
        
        // 查找并取消对应的网络回复
        for (auto replyIt = m_replyToRequestId.begin(); replyIt != m_replyToRequestId.end(); ++replyIt) {
            if (replyIt.value() == requestId) {
                QNetworkReply* reply = replyIt.key();
                if (reply) {
                    reply->abort();
                    reply->deleteLater();
                }
                m_replyToRequestId.erase(replyIt);
                break;
            }
        }
        
        m_pendingRequests.remove(requestId);
    }
    
    if (!expiredRequests.isEmpty()) {
        qDebug() << "JitsiMeetAPI: 清理了" << expiredRequests.size() << "个过期请求";
    }
}

/**
 * @brief 记录API调用
 * @param operation 操作名称
 * @param endpoint API端点
 * @param method HTTP方法
 * @param success 是否成功
 * @param responseTime 响应时间
 */
void JitsiMeetAPI::logApiCall(const QString& operation, const QString& endpoint, 
                             const QString& method, bool success, int responseTime) const
{
    QString status = success ? "成功" : "失败";
    qDebug() << QString("JitsiMeetAPI: [%1] %2 %3 %4 - %5 (%6ms)")
                .arg(status)
                .arg(method)
                .arg(endpoint)
                .arg(operation)
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg(responseTime);
}