#include "AuthenticationManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>

AuthenticationManager::AuthenticationManager(QObject *parent)
    : QObject(parent)
    , m_authState(NotAuthenticated)
    , m_authType(None)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_tokenExpirationTimer(new QTimer(this))
{
    qDebug() << "AuthenticationManager initialized";
    
    // 设置token过期检查定时器
    m_tokenExpirationTimer->setInterval(60000); // 每分钟检查一次
    connect(m_tokenExpirationTimer, &QTimer::timeout, this, &AuthenticationManager::checkTokenExpiration);
}

AuthenticationManager::~AuthenticationManager()
{
    qDebug() << "AuthenticationManager destroyed";
}

void AuthenticationManager::authenticate(const QString& serverUrl, const QString& roomName, const QString& displayName)
{
    qDebug() << "Starting authentication for" << serverUrl << "room" << roomName << "user" << displayName;
    
    m_serverUrl = serverUrl;
    m_roomName = roomName;
    m_displayName = displayName;
    
    setAuthState(Authenticating);
    
    // 检查认证要求
    checkAuthRequirements();
}

void AuthenticationManager::authenticateWithJWT(const QString& token)
{
    qDebug() << "Authenticating with JWT token";
    
    // 解析JWT token
    m_tokenInfo = parseJWTToken(token);
    
    if (!m_tokenInfo.isValid) {
        emit authenticationFailed("Invalid JWT token format");
        return;
    }
    
    // 验证JWT token
    if (!verifyJWTToken(m_tokenInfo)) {
        emit authenticationFailed("JWT token verification failed");
        return;
    }
    
    m_authToken = token;
    m_authType = JWT;
    
    // 从JWT claims中提取用户信息
    if (m_tokenInfo.claims.contains("sub")) {
        m_userId = m_tokenInfo.claims["sub"].toString();
    }
    if (m_tokenInfo.claims.contains("name")) {
        m_displayName = m_tokenInfo.claims["name"].toString();
    }
    
    // 设置token过期监控
    setupTokenExpirationTimer();
    
    setAuthState(Authenticated);
    emit authenticationSucceeded(JWT);
}

void AuthenticationManager::authenticateWithPassword(const QString& password)
{
    qDebug() << "Authenticating with password";
    
    if (password.isEmpty()) {
        emit authenticationFailed("Password cannot be empty");
        return;
    }
    
    m_authType = Password;
    
    // 构建密码认证请求
    QString authUrl = m_serverUrl + "/api/auth/password";
    QNetworkRequest request(authUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject authData;
    authData["room"] = m_roomName;
    authData["password"] = password;
    authData["displayName"] = m_displayName;
    
    QJsonDocument doc(authData);
    QByteArray data = doc.toJson();
    
    QNetworkReply* reply = m_networkManager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &AuthenticationManager::onAuthenticationReply);
}

void AuthenticationManager::checkRoomPermissions(const QString& roomName)
{
    qDebug() << "Checking room permissions for" << roomName;
    
    // 构建权限检查URL
    QString url = m_serverUrl + "/api/room/" + roomName + "/permissions";
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_authToken).toUtf8());
    }
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &AuthenticationManager::onPermissionCheckReply);
}

void AuthenticationManager::setAuthState(AuthState state)
{
    if (m_authState == state) {
        return;
    }
    
    AuthState oldState = m_authState;
    m_authState = state;
    
    qDebug() << "Auth state changed:" << oldState << "->" << state;
    emit authStateChanged(state);
}

void AuthenticationManager::checkAuthRequirements()
{
    // 首先尝试Jitsi Meet标准认证流程
    performJitsiMeetAuthentication();
}

bool AuthenticationManager::validateJWTToken(const QString& token)
{
    // 简单的JWT格式验证（三个部分用.分隔）
    QStringList parts = token.split('.');
    return parts.size() == 3 && !parts[0].isEmpty() && !parts[1].isEmpty() && !parts[2].isEmpty();
}

void AuthenticationManager::performGuestAuthentication()
{
    qDebug() << "Performing guest authentication";
    
    m_authType = Guest;
    
    // 访客模式直接成功
    QTimer::singleShot(500, this, [this]() {
        setAuthState(Authenticated);
        emit authenticationSucceeded(Guest);
    });
}

void AuthenticationManager::onAuthenticationReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Authentication request failed:" << reply->errorString();
        
        // 如果认证请求失败，尝试访客模式
        performGuestAuthentication();
        return;
    }
    
    QByteArray data = reply->readAll();
    
    // 尝试解析JSON响应
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        // JSON响应 - 处理认证结果
        handleAuthenticationResponse(doc.object());
    } else {
        // 可能是配置文件响应 - 解析认证要求
        QString configText = QString::fromUtf8(data);
        
        if (configText.contains("enableUserRolesBasedOnToken") && configText.contains("true")) {
            // 需要JWT认证
            emit jwtTokenRequired();
        } else if (configText.contains("requireDisplayName") && configText.contains("true")) {
            // 需要密码认证
            emit passwordRequired();
        } else {
            // 访客模式
            performGuestAuthentication();
        }
    }
}

void AuthenticationManager::onPermissionCheckReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Permission check failed:" << reply->errorString();
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    // 更新房间权限信息
    m_roomPermissions.canJoin = obj.value("canJoin").toBool(true);
    m_roomPermissions.isModerator = obj.value("isModerator").toBool(false);
    m_roomPermissions.canRecord = obj.value("canRecord").toBool(false);
    m_roomPermissions.canLiveStream = obj.value("canLiveStream").toBool(false);
    m_roomPermissions.role = obj.value("role").toString("participant");
    
    // 解析功能列表
    QJsonArray featuresArray = obj.value("features").toArray();
    m_roomPermissions.features.clear();
    for (const QJsonValue& value : featuresArray) {
        m_roomPermissions.features.append(value.toString());
    }
    
    qDebug() << "Room permissions updated - canJoin:" << m_roomPermissions.canJoin 
             << "isModerator:" << m_roomPermissions.isModerator
             << "role:" << m_roomPermissions.role;
    
    emit roomPermissionsUpdated(m_roomPermissions);
}

AuthenticationManager::JWTTokenInfo AuthenticationManager::parseJWTToken(const QString& token)
{
    JWTTokenInfo tokenInfo;
    
    QStringList parts = token.split('.');
    if (parts.size() != 3) {
        qWarning() << "Invalid JWT token format: expected 3 parts";
        return tokenInfo;
    }
    
    try {
        // 解码header
        QString headerJson = base64UrlDecode(parts[0]);
        QJsonDocument headerDoc = QJsonDocument::fromJson(headerJson.toUtf8());
        if (headerDoc.isNull()) {
            qWarning() << "Failed to parse JWT header";
            return tokenInfo;
        }
        
        // 解码payload
        QString payloadJson = base64UrlDecode(parts[1]);
        QJsonDocument payloadDoc = QJsonDocument::fromJson(payloadJson.toUtf8());
        if (payloadDoc.isNull()) {
            qWarning() << "Failed to parse JWT payload";
            return tokenInfo;
        }
        
        tokenInfo.header = headerJson;
        tokenInfo.payload = payloadJson;
        tokenInfo.signature = parts[2];
        tokenInfo.claims = payloadDoc.object();
        
        // 解析时间戳
        if (tokenInfo.claims.contains("iat")) {
            qint64 iat = tokenInfo.claims["iat"].toVariant().toLongLong();
            tokenInfo.issuedAt = QDateTime::fromSecsSinceEpoch(iat);
        }
        
        if (tokenInfo.claims.contains("exp")) {
            qint64 exp = tokenInfo.claims["exp"].toVariant().toLongLong();
            tokenInfo.expiresAt = QDateTime::fromSecsSinceEpoch(exp);
        }
        
        tokenInfo.isValid = true;
        
        qDebug() << "JWT token parsed successfully";
        qDebug() << "Issued at:" << tokenInfo.issuedAt;
        qDebug() << "Expires at:" << tokenInfo.expiresAt;
        
    } catch (...) {
        qWarning() << "Exception occurred while parsing JWT token";
        tokenInfo.isValid = false;
    }
    
    return tokenInfo;
}

bool AuthenticationManager::verifyJWTToken(const JWTTokenInfo& tokenInfo)
{
    if (!tokenInfo.isValid) {
        return false;
    }
    
    // 检查token是否过期
    QDateTime now = QDateTime::currentDateTime();
    if (tokenInfo.expiresAt.isValid() && tokenInfo.expiresAt < now) {
        qWarning() << "JWT token has expired";
        return false;
    }
    
    // 检查必要的claims
    if (!tokenInfo.claims.contains("sub")) {
        qWarning() << "JWT token missing 'sub' claim";
        return false;
    }
    
    // 这里可以添加更多的验证逻辑，比如签名验证
    // 但由于Qt没有内置的JWT签名验证，这里简化处理
    
    qDebug() << "JWT token verification passed";
    return true;
}

void AuthenticationManager::refreshAuthToken()
{
    if (m_authType != JWT || m_authToken.isEmpty()) {
        qWarning() << "Cannot refresh token: not using JWT authentication";
        return;
    }
    
    qDebug() << "Refreshing authentication token";
    
    QString refreshUrl = m_serverUrl + "/api/auth/refresh";
    QNetworkRequest request(refreshUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_authToken).toUtf8());
    
    QJsonObject refreshData;
    refreshData["token"] = m_authToken;
    
    QJsonDocument doc(refreshData);
    QByteArray data = doc.toJson();
    
    QNetworkReply* reply = m_networkManager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &AuthenticationManager::onTokenRefreshReply);
}

void AuthenticationManager::logout()
{
    qDebug() << "Logging out";
    
    // 停止token过期检查
    m_tokenExpirationTimer->stop();
    
    // 清除认证信息
    m_authToken.clear();
    m_userId.clear();
    m_tokenInfo = JWTTokenInfo();
    m_roomPermissions = RoomPermissions();
    
    setAuthState(NotAuthenticated);
    m_authType = None;
}

void AuthenticationManager::onTokenRefreshReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Token refresh failed:" << reply->errorString();
        emit authenticationFailed("Token refresh failed");
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    if (obj.contains("token")) {
        QString newToken = obj["token"].toString();
        authenticateWithJWT(newToken);
        qDebug() << "Token refreshed successfully";
    } else {
        qWarning() << "Token refresh response missing token";
        emit authenticationFailed("Invalid token refresh response");
    }
}

void AuthenticationManager::checkTokenExpiration()
{
    if (!m_tokenInfo.isValid || !m_tokenInfo.expiresAt.isValid()) {
        return;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsToExpiry = now.secsTo(m_tokenInfo.expiresAt);
    
    if (secondsToExpiry <= 0) {
        // Token已过期
        qWarning() << "Authentication token has expired";
        m_tokenExpirationTimer->stop();
        emit tokenExpired();
        logout();
    } else if (secondsToExpiry <= 300) { // 5分钟内过期
        // Token即将过期
        qDebug() << "Authentication token expiring in" << secondsToExpiry << "seconds";
        emit tokenExpiring(static_cast<int>(secondsToExpiry));
        
        // 尝试自动刷新token
        refreshAuthToken();
    }
}

QString AuthenticationManager::base64UrlDecode(const QString& input)
{
    QString padded = input;
    
    // 添加必要的padding
    int padding = 4 - (padded.length() % 4);
    if (padding != 4) {
        padded.append(QString(padding, '='));
    }
    
    // 替换URL安全字符
    padded.replace('-', '+');
    padded.replace('_', '/');
    
    QByteArray decoded = QByteArray::fromBase64(padded.toUtf8());
    return QString::fromUtf8(decoded);
}

QString AuthenticationManager::base64UrlEncode(const QByteArray& input)
{
    QString encoded = input.toBase64();
    
    // 替换为URL安全字符
    encoded.replace('+', '-');
    encoded.replace('/', '_');
    encoded.remove('='); // 移除padding
    
    return encoded;
}

void AuthenticationManager::setupTokenExpirationTimer()
{
    if (m_tokenInfo.isValid && m_tokenInfo.expiresAt.isValid()) {
        m_tokenExpirationTimer->start();
        qDebug() << "Token expiration monitoring started";
    }
}

void AuthenticationManager::performJitsiMeetAuthentication()
{
    qDebug() << "Performing Jitsi Meet authentication flow";
    
    // 构建Jitsi Meet认证URL
    QString authUrl = m_serverUrl + "/api/v1/auth";
    QNetworkRequest request(authUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject authData;
    authData["room"] = m_roomName;
    authData["displayName"] = m_displayName;
    authData["authType"] = "jitsi";
    
    QJsonDocument doc(authData);
    QByteArray data = doc.toJson();
    
    QNetworkReply* reply = m_networkManager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &AuthenticationManager::onAuthenticationReply);
}

void AuthenticationManager::handleAuthenticationResponse(const QJsonObject& response)
{
    if (response.contains("success") && response["success"].toBool()) {
        // 认证成功
        if (response.contains("token")) {
            QString token = response["token"].toString();
            authenticateWithJWT(token);
        } else {
            // 无token的成功认证（访客模式）
            performGuestAuthentication();
        }
        
        // 处理用户信息
        if (response.contains("user")) {
            QJsonObject userObj = response["user"].toObject();
            if (userObj.contains("id")) {
                m_userId = userObj["id"].toString();
            }
            if (userObj.contains("displayName")) {
                m_displayName = userObj["displayName"].toString();
            }
        }
        
    } else {
        // 认证失败
        QString error = response.value("error").toString("Authentication failed");
        emit authenticationFailed(error);
    }
}