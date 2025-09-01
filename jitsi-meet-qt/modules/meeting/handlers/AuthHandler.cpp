#include "AuthHandler.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QDebug>

class AuthHandler::Private
{
public:
    AuthStatus currentStatus;
    QVariantMap currentUser;
    UserRole currentRole;
    QString authToken;
    QString authServer;
    int tokenExpiration;
    bool autoRefreshEnabled;
    
    QNetworkAccessManager* networkManager;
    QTimer* refreshTimer;
    QTimer* timeoutTimer;
    
    QVariantMap sessionInfo;
    QVariantMap permissions;
    
    Private()
        : currentStatus(NotAuthenticated)
        , currentRole(Guest)
        , tokenExpiration(3600) // 1 hour default
        , autoRefreshEnabled(true)
        , networkManager(nullptr)
        , refreshTimer(nullptr)
        , timeoutTimer(nullptr)
    {
    }
};

AuthHandler::AuthHandler(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    initializeNetworkManager();
    
    d->refreshTimer = new QTimer(this);
    d->refreshTimer->setSingleShot(true);
    connect(d->refreshTimer, &QTimer::timeout, this, &AuthHandler::handleTokenRefreshTimer);
    
    d->timeoutTimer = new QTimer(this);
    d->timeoutTimer->setSingleShot(true);
    connect(d->timeoutTimer, &QTimer::timeout, this, &AuthHandler::handleAuthTimeout);
    
    // Load stored auth info
    QVariantMap storedAuth = loadStoredAuthInfo();
    if (!storedAuth.isEmpty()) {
        QString token = storedAuth.value("token").toString();
        if (!token.isEmpty() && validateToken(token)) {
            d->authToken = token;
            d->currentUser = storedAuth.value("user").toMap();
            d->currentRole = static_cast<UserRole>(storedAuth.value("role", Guest).toInt());
            setAuthStatus(Authenticated);
        }
    }
}

AuthHandler::~AuthHandler() = default;

bool AuthHandler::authenticate(AuthType authType, const QVariantMap& credentials)
{
    if (d->currentStatus == Authenticating) {
        emit errorOccurred("Authentication already in progress");
        return false;
    }
    
    setAuthStatus(Authenticating);
    
    // Start timeout timer
    d->timeoutTimer->start(30000); // 30 seconds timeout
    
    switch (authType) {
    case GuestAuth:
        return authenticateAsGuest(
            credentials.value("displayName").toString(),
            credentials.value("email").toString()
        );
    case PasswordAuth:
        return authenticateWithPassword(
            credentials.value("username").toString(),
            credentials.value("password").toString()
        );
    case TokenAuth:
        return authenticateWithToken(credentials.value("token").toString());
    case JWTAuth:
        return authenticateWithJWT(credentials.value("jwt").toString());
    case SSOAuth:
        return authenticateWithSSO(
            credentials.value("provider").toString(),
            credentials.value("redirectUrl").toString()
        );
    case OAuthAuth:
        // OAuth implementation would be more complex
        emit errorOccurred("OAuth authentication not yet implemented");
        setAuthStatus(AuthFailed);
        return false;
    default:
        emit errorOccurred("Unsupported authentication type");
        setAuthStatus(AuthFailed);
        return false;
    }
}

bool AuthHandler::logout()
{
    if (d->currentStatus == NotAuthenticated) {
        return true;
    }
    
    // Stop timers
    stopTokenRefreshTimer();
    d->timeoutTimer->stop();
    
    // Clear auth info
    clearAuthInfo();
    
    // Reset state
    d->authToken.clear();
    d->currentUser.clear();
    d->currentRole = Guest;
    d->sessionInfo.clear();
    d->permissions.clear();
    
    setAuthStatus(NotAuthenticated);
    emit loggedOut();
    
    return true;
}

AuthHandler::AuthStatus AuthHandler::currentStatus() const
{
    return d->currentStatus;
}

QVariantMap AuthHandler::getCurrentUser() const
{
    return d->currentUser;
}

AuthHandler::UserRole AuthHandler::getCurrentUserRole() const
{
    return d->currentRole;
}

bool AuthHandler::checkPermission(const QString& resource, const QString& action)
{
    if (d->currentStatus != Authenticated) {
        return false;
    }
    
    // Check role-based permissions
    switch (d->currentRole) {
    case Administrator:
        return true; // Admin has all permissions
    case Moderator:
        // Moderators have most permissions except admin functions
        if (resource == "admin") {
            return false;
        }
        return true;
    case Participant:
        // Participants have basic permissions
        if (resource == "meeting" && (action == "join" || action == "leave" || action == "speak")) {
            return true;
        }
        if (resource == "chat" && (action == "send" || action == "receive")) {
            return true;
        }
        return false;
    case Guest:
        // Guests have limited permissions
        if (resource == "meeting" && action == "join") {
            return true;
        }
        return false;
    default:
        return false;
    }
}

bool AuthHandler::refreshToken()
{
    if (d->authToken.isEmpty() || d->authServer.isEmpty()) {
        emit tokenRefreshFailed("No token or server configured");
        return false;
    }
    
    QNetworkRequest request;
    request.setUrl(QUrl(d->authServer + "/auth/refresh"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(d->authToken).toUtf8());
    
    QJsonObject requestData;
    requestData["token"] = d->authToken;
    
    QNetworkReply* reply = d->networkManager->post(request, QJsonDocument(requestData).toJson());
    connect(reply, &QNetworkReply::finished, this, &AuthHandler::handleNetworkReply);
    
    return true;
}

bool AuthHandler::validateToken(const QString& token)
{
    if (token.isEmpty()) {
        return false;
    }
    
    // Basic token validation
    if (token.startsWith("eyJ")) {
        // JWT token
        return validateJWTSignature(token);
    }
    
    // For other token types, we might need to validate with server
    return !token.isEmpty() && token.length() > 10;
}

QString AuthHandler::getAuthToken() const
{
    return d->authToken;
}

void AuthHandler::setAuthToken(const QString& token)
{
    d->authToken = token;
    
    if (!token.isEmpty() && d->autoRefreshEnabled) {
        startTokenRefreshTimer();
    }
}

QVariantMap AuthHandler::getSessionInfo() const
{
    return d->sessionInfo;
}

void AuthHandler::setAuthServer(const QString& serverUrl)
{
    d->authServer = serverUrl;
}

QString AuthHandler::authServer() const
{
    return d->authServer;
}

void AuthHandler::setTokenExpiration(int seconds)
{
    d->tokenExpiration = seconds;
}

int AuthHandler::tokenExpiration() const
{
    return d->tokenExpiration;
}

void AuthHandler::setAutoRefreshEnabled(bool enabled)
{
    d->autoRefreshEnabled = enabled;
    
    if (enabled && !d->authToken.isEmpty()) {
        startTokenRefreshTimer();
    } else {
        stopTokenRefreshTimer();
    }
}

bool AuthHandler::isAutoRefreshEnabled() const
{
    return d->autoRefreshEnabled;
}

bool AuthHandler::authenticateAsGuest(const QString& displayName, const QString& email)
{
    if (displayName.isEmpty()) {
        emit authenticationFailed("Display name is required for guest authentication");
        setAuthStatus(AuthFailed);
        return false;
    }
    
    // Generate a guest token
    QString guestToken = QString("guest_%1_%2")
                        .arg(QDateTime::currentSecsSinceEpoch())
                        .arg(QRandomGenerator::global()->generate());
    
    QVariantMap userInfo;
    userInfo["id"] = guestToken;
    userInfo["displayName"] = displayName;
    userInfo["email"] = email;
    userInfo["type"] = "guest";
    
    d->authToken = guestToken;
    d->currentUser = userInfo;
    d->currentRole = Guest;
    
    // Store auth info
    QVariantMap authInfo;
    authInfo["token"] = d->authToken;
    authInfo["user"] = d->currentUser;
    authInfo["role"] = static_cast<int>(d->currentRole);
    storeAuthInfo(authInfo);
    
    setAuthStatus(Authenticated);
    emit authenticationSucceeded(userInfo);
    
    return true;
}

bool AuthHandler::authenticateWithPassword(const QString& username, const QString& password)
{
    if (username.isEmpty() || password.isEmpty()) {
        emit authenticationFailed("Username and password are required");
        setAuthStatus(AuthFailed);
        return false;
    }
    
    if (d->authServer.isEmpty()) {
        emit authenticationFailed("Authentication server not configured");
        setAuthStatus(AuthFailed);
        return false;
    }
    
    QNetworkRequest request;
    request.setUrl(QUrl(d->authServer + "/auth/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject requestData;
    requestData["username"] = username;
    requestData["password"] = password;
    
    QNetworkReply* reply = d->networkManager->post(request, QJsonDocument(requestData).toJson());
    connect(reply, &QNetworkReply::finished, this, &AuthHandler::handleNetworkReply);
    
    return true;
}

bool AuthHandler::authenticateWithToken(const QString& token)
{
    if (!validateToken(token)) {
        emit authenticationFailed("Invalid token");
        setAuthStatus(AuthFailed);
        return false;
    }
    
    d->authToken = token;
    
    // For token auth, we might need to fetch user info from server
    if (!d->authServer.isEmpty()) {
        QNetworkRequest request;
        request.setUrl(QUrl(d->authServer + "/auth/userinfo"));
        request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
        
        QNetworkReply* reply = d->networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, &AuthHandler::handleNetworkReply);
    } else {
        // Assume token is valid and create basic user info
        QVariantMap userInfo;
        userInfo["id"] = "token_user";
        userInfo["displayName"] = "Token User";
        userInfo["type"] = "token";
        
        d->currentUser = userInfo;
        d->currentRole = Participant;
        
        setAuthStatus(Authenticated);
        emit authenticationSucceeded(userInfo);
    }
    
    return true;
}

bool AuthHandler::authenticateWithJWT(const QString& jwt)
{
    if (!validateJWTSignature(jwt)) {
        emit authenticationFailed("Invalid JWT token");
        setAuthStatus(AuthFailed);
        return false;
    }
    
    QVariantMap jwtData = parseJWT(jwt);
    if (jwtData.isEmpty()) {
        emit authenticationFailed("Failed to parse JWT token");
        setAuthStatus(AuthFailed);
        return false;
    }
    
    d->authToken = jwt;
    
    // Extract user info from JWT
    QVariantMap userInfo;
    userInfo["id"] = jwtData.value("sub").toString();
    userInfo["displayName"] = jwtData.value("name", jwtData.value("sub")).toString();
    userInfo["email"] = jwtData.value("email").toString();
    userInfo["type"] = "jwt";
    
    // Determine role from JWT claims
    QString role = jwtData.value("role", "participant").toString().toLower();
    if (role == "admin" || role == "administrator") {
        d->currentRole = Administrator;
    } else if (role == "moderator" || role == "mod") {
        d->currentRole = Moderator;
    } else {
        d->currentRole = Participant;
    }
    
    d->currentUser = userInfo;
    
    // Store auth info
    QVariantMap authInfo;
    authInfo["token"] = d->authToken;
    authInfo["user"] = d->currentUser;
    authInfo["role"] = static_cast<int>(d->currentRole);
    storeAuthInfo(authInfo);
    
    setAuthStatus(Authenticated);
    emit authenticationSucceeded(userInfo);
    
    if (d->autoRefreshEnabled) {
        startTokenRefreshTimer();
    }
    
    return true;
}

bool AuthHandler::authenticateWithSSO(const QString& ssoProvider, const QString& redirectUrl)
{
    Q_UNUSED(ssoProvider)
    Q_UNUSED(redirectUrl)
    
    // SSO authentication would require opening a web browser
    // and handling the callback - this is a simplified implementation
    emit authenticationFailed("SSO authentication not yet fully implemented");
    setAuthStatus(AuthFailed);
    return false;
}

QList<AuthHandler::AuthType> AuthHandler::getSupportedAuthTypes() const
{
    return {GuestAuth, PasswordAuth, TokenAuth, JWTAuth};
}

bool AuthHandler::isAuthExpired() const
{
    if (d->authToken.isEmpty()) {
        return true;
    }
    
    // For JWT tokens, check expiration
    if (d->authToken.startsWith("eyJ")) {
        QVariantMap jwtData = parseJWT(d->authToken);
        qint64 exp = jwtData.value("exp").toLongLong();
        if (exp > 0) {
            return QDateTime::currentSecsSinceEpoch() >= exp;
        }
    }
    
    return false;
}

int AuthHandler::getAuthTimeRemaining() const
{
    if (d->authToken.isEmpty()) {
        return 0;
    }
    
    // For JWT tokens, calculate remaining time
    if (d->authToken.startsWith("eyJ")) {
        QVariantMap jwtData = parseJWT(d->authToken);
        qint64 exp = jwtData.value("exp").toLongLong();
        if (exp > 0) {
            qint64 remaining = exp - QDateTime::currentSecsSinceEpoch();
            return qMax(0LL, remaining);
        }
    }
    
    return d->tokenExpiration;
}

void AuthHandler::checkAuthStatus()
{
    if (isAuthExpired()) {
        handleAuthExpiration();
    }
}

void AuthHandler::handleAuthExpiration()
{
    if (d->autoRefreshEnabled && !d->authToken.isEmpty()) {
        refreshToken();
    } else {
        setAuthStatus(AuthExpired);
        emit authenticationExpired();
    }
}

void AuthHandler::handleNetworkReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    reply->deleteLater();
    d->timeoutTimer->stop();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString error = QString("Network error: %1").arg(reply->errorString());
        emit authenticationFailed(error);
        setAuthStatus(AuthFailed);
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        emit authenticationFailed("Invalid response format");
        setAuthStatus(AuthFailed);
        return;
    }
    
    QVariantMap response = doc.object().toVariantMap();
    processAuthResponse(response);
}

void AuthHandler::handleTokenRefreshTimer()
{
    refreshToken();
}

void AuthHandler::handleAuthTimeout()
{
    emit authenticationFailed("Authentication timeout");
    setAuthStatus(AuthFailed);
}

void AuthHandler::initializeNetworkManager()
{
    d->networkManager = new QNetworkAccessManager(this);
    d->networkManager->setTransferTimeout(30000); // 30 seconds
}

void AuthHandler::setAuthStatus(AuthStatus status)
{
    if (d->currentStatus != status) {
        d->currentStatus = status;
        emit authStatusChanged(status);
    }
}

void AuthHandler::processAuthResponse(const QVariantMap& response)
{
    if (!response.value("success", false).toBool()) {
        QString error = response.value("error", "Authentication failed").toString();
        emit authenticationFailed(error);
        setAuthStatus(AuthFailed);
        return;
    }
    
    // Extract token
    QString token = response.value("token").toString();
    if (token.isEmpty()) {
        emit authenticationFailed("No token received");
        setAuthStatus(AuthFailed);
        return;
    }
    
    d->authToken = token;
    
    // Extract user info
    QVariantMap userInfo = response.value("user").toMap();
    if (userInfo.isEmpty()) {
        // Create basic user info
        userInfo["id"] = "authenticated_user";
        userInfo["displayName"] = "Authenticated User";
    }
    
    d->currentUser = userInfo;
    
    // Extract role
    QString roleStr = response.value("role", "participant").toString().toLower();
    if (roleStr == "admin" || roleStr == "administrator") {
        d->currentRole = Administrator;
    } else if (roleStr == "moderator" || roleStr == "mod") {
        d->currentRole = Moderator;
    } else {
        d->currentRole = Participant;
    }
    
    // Store auth info
    QVariantMap authInfo;
    authInfo["token"] = d->authToken;
    authInfo["user"] = d->currentUser;
    authInfo["role"] = static_cast<int>(d->currentRole);
    storeAuthInfo(authInfo);
    
    setAuthStatus(Authenticated);
    emit authenticationSucceeded(userInfo);
    
    if (d->autoRefreshEnabled) {
        startTokenRefreshTimer();
    }
}

void AuthHandler::startTokenRefreshTimer()
{
    if (!d->refreshTimer) {
        return;
    }
    
    int refreshInterval = d->tokenExpiration * 1000 * 0.8; // Refresh at 80% of expiration
    d->refreshTimer->start(refreshInterval);
}

void AuthHandler::stopTokenRefreshTimer()
{
    if (d->refreshTimer) {
        d->refreshTimer->stop();
    }
}

QVariantMap AuthHandler::parseJWT(const QString& jwt) const
{
    QStringList parts = jwt.split('.');
    if (parts.size() != 3) {
        return QVariantMap();
    }
    
    // Decode payload (second part)
    QByteArray payload = QByteArray::fromBase64(parts[1].toUtf8(), QByteArray::Base64UrlEncoding);
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        return QVariantMap();
    }
    
    return doc.object().toVariantMap();
}

bool AuthHandler::validateJWTSignature(const QString& jwt)
{
    // Basic JWT format validation
    QStringList parts = jwt.split('.');
    if (parts.size() != 3) {
        return false;
    }
    
    // For now, just check if it's properly formatted
    // Real signature validation would require the secret key
    return !parts[0].isEmpty() && !parts[1].isEmpty() && !parts[2].isEmpty();
}

QVariantMap AuthHandler::buildAuthRequest(AuthType authType, const QVariantMap& credentials)
{
    QVariantMap request;
    request["type"] = static_cast<int>(authType);
    request["credentials"] = credentials;
    request["timestamp"] = QDateTime::currentSecsSinceEpoch();
    
    return request;
}

void AuthHandler::storeAuthInfo(const QVariantMap& authInfo)
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    
    QSettings settings(configDir + "/auth.ini", QSettings::IniFormat);
    settings.setValue("token", authInfo.value("token"));
    settings.setValue("user", authInfo.value("user"));
    settings.setValue("role", authInfo.value("role"));
    settings.setValue("timestamp", QDateTime::currentSecsSinceEpoch());
}

void AuthHandler::clearAuthInfo()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QSettings settings(configDir + "/auth.ini", QSettings::IniFormat);
    settings.clear();
}

QVariantMap AuthHandler::loadStoredAuthInfo()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QSettings settings(configDir + "/auth.ini", QSettings::IniFormat);
    
    QVariantMap authInfo;
    authInfo["token"] = settings.value("token");
    authInfo["user"] = settings.value("user");
    authInfo["role"] = settings.value("role");
    authInfo["timestamp"] = settings.value("timestamp");
    
    // Check if stored auth is not too old (24 hours)
    qint64 timestamp = authInfo.value("timestamp").toLongLong();
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    if (currentTime - timestamp > 86400) { // 24 hours
        return QVariantMap();
    }
    
    return authInfo;
}