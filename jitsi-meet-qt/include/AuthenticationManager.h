#ifndef AUTHENTICATIONMANAGER_H
#define AUTHENTICATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QDateTime>
#include <QCryptographicHash>
#include <QUrlQuery>

/**
 * @brief 认证管理器类，处理Jitsi Meet的用户认证
 * 
 * 该类负责处理JWT token验证、密码认证和会议室权限检查
 */
class AuthenticationManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 认证状态枚举
     */
    enum AuthState {
        NotAuthenticated,   ///< 未认证
        Authenticating,     ///< 认证中
        Authenticated,      ///< 已认证
        Failed              ///< 认证失败
    };
    Q_ENUM(AuthState)

    /**
     * @brief 认证类型枚举
     */
    enum AuthType {
        None,               ///< 无需认证
        JWT,                ///< JWT token认证
        Password,           ///< 密码认证
        Guest               ///< 访客模式
    };
    Q_ENUM(AuthType)

    /**
     * @brief JWT token信息结构
     */
    struct JWTTokenInfo {
        QString header;         ///< JWT头部
        QString payload;        ///< JWT载荷
        QString signature;      ///< JWT签名
        QJsonObject claims;     ///< JWT声明
        QDateTime issuedAt;     ///< 签发时间
        QDateTime expiresAt;    ///< 过期时间
        bool isValid;           ///< 是否有效
        
        JWTTokenInfo() : isValid(false) {}
    };

    /**
     * @brief 房间权限信息结构
     */
    struct RoomPermissions {
        bool canJoin;           ///< 是否可以加入
        bool isModerator;       ///< 是否为主持人
        bool canRecord;         ///< 是否可以录制
        bool canLiveStream;     ///< 是否可以直播
        QString role;           ///< 用户角色
        QStringList features;   ///< 可用功能列表
        
        RoomPermissions() : canJoin(true), isModerator(false), canRecord(false), canLiveStream(false) {}
    };

    explicit AuthenticationManager(QObject *parent = nullptr);
    ~AuthenticationManager();

    /**
     * @brief 开始认证流程
     * @param serverUrl 服务器URL
     * @param roomName 房间名称
     * @param displayName 用户显示名称
     */
    void authenticate(const QString& serverUrl, const QString& roomName, const QString& displayName);

    /**
     * @brief 使用JWT token认证
     * @param token JWT token
     */
    void authenticateWithJWT(const QString& token);

    /**
     * @brief 使用密码认证
     * @param password 密码
     */
    void authenticateWithPassword(const QString& password);

    /**
     * @brief 检查房间权限
     * @param roomName 房间名称
     */
    void checkRoomPermissions(const QString& roomName);

    /**
     * @brief 解析JWT token
     * @param token JWT token字符串
     * @return JWT token信息
     */
    JWTTokenInfo parseJWTToken(const QString& token);

    /**
     * @brief 验证JWT token
     * @param tokenInfo JWT token信息
     * @return 是否有效
     */
    bool verifyJWTToken(const JWTTokenInfo& tokenInfo);

    /**
     * @brief 刷新认证token
     */
    void refreshAuthToken();

    /**
     * @brief 注销认证
     */
    void logout();

    // Getters
    AuthState authState() const { return m_authState; }
    AuthType authType() const { return m_authType; }
    QString authToken() const { return m_authToken; }
    bool isAuthenticated() const { return m_authState == Authenticated; }
    QString userDisplayName() const { return m_displayName; }
    QString userId() const { return m_userId; }
    RoomPermissions roomPermissions() const { return m_roomPermissions; }
    JWTTokenInfo currentTokenInfo() const { return m_tokenInfo; }

signals:
    /**
     * @brief 认证状态改变信号
     * @param state 新的认证状态
     */
    void authStateChanged(AuthState state);

    /**
     * @brief 认证成功信号
     * @param authType 认证类型
     */
    void authenticationSucceeded(AuthType authType);

    /**
     * @brief 认证失败信号
     * @param error 错误信息
     */
    void authenticationFailed(const QString& error);

    /**
     * @brief 需要密码认证信号
     */
    void passwordRequired();

    /**
     * @brief 需要JWT token信号
     */
    void jwtTokenRequired();

    /**
     * @brief 房间权限更新信号
     * @param permissions 房间权限信息
     */
    void roomPermissionsUpdated(const RoomPermissions& permissions);

    /**
     * @brief 认证token即将过期信号
     * @param expiresIn 剩余时间（秒）
     */
    void tokenExpiring(int expiresIn);

    /**
     * @brief 认证token已过期信号
     */
    void tokenExpired();

private slots:
    void onAuthenticationReply();
    void onPermissionCheckReply();
    void onTokenRefreshReply();
    void checkTokenExpiration();

private:
    void setAuthState(AuthState state);
    void checkAuthRequirements();
    bool validateJWTToken(const QString& token);
    void performGuestAuthentication();
    QString base64UrlDecode(const QString& input);
    QString base64UrlEncode(const QByteArray& input);
    void setupTokenExpirationTimer();
    void performJitsiMeetAuthentication();
    void handleAuthenticationResponse(const QJsonObject& response);

    AuthState m_authState;
    AuthType m_authType;
    QString m_serverUrl;
    QString m_roomName;
    QString m_displayName;
    QString m_userId;
    QString m_authToken;
    JWTTokenInfo m_tokenInfo;
    RoomPermissions m_roomPermissions;
    QNetworkAccessManager* m_networkManager;
    QTimer* m_tokenExpirationTimer;
};

#endif // AUTHENTICATIONMANAGER_H