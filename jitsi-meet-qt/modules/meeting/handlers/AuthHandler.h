#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QTimer>

/**
 * @brief 认证处理器类
 * 
 * 处理会议认证相关功能，包括用户认证、权限验证和会话管理
 */
class AuthHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 认证类型枚举
     */
    enum AuthType {
        GuestAuth,       ///< 访客认证
        PasswordAuth,    ///< 密码认证
        TokenAuth,       ///< 令牌认证
        JWTAuth,         ///< JWT认证
        SSOAuth,         ///< 单点登录认证
        OAuthAuth        ///< OAuth认证
    };
    Q_ENUM(AuthType)

    /**
     * @brief 认证状态枚举
     */
    enum AuthStatus {
        NotAuthenticated, ///< 未认证
        Authenticating,   ///< 认证中
        Authenticated,    ///< 已认证
        AuthFailed,       ///< 认证失败
        AuthExpired,      ///< 认证过期
        AuthRevoked       ///< 认证撤销
    };
    Q_ENUM(AuthStatus)

    /**
     * @brief 用户角色枚举
     */
    enum UserRole {
        Guest,           ///< 访客
        Participant,     ///< 参与者
        Moderator,       ///< 主持人
        Administrator    ///< 管理员
    };
    Q_ENUM(UserRole)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AuthHandler(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~AuthHandler();

    /**
     * @brief 进行认证
     * @param authType 认证类型
     * @param credentials 认证凭据
     * @return 认证是否成功启动
     */
    bool authenticate(AuthType authType, const QVariantMap& credentials);

    /**
     * @brief 注销认证
     * @return 注销是否成功
     */
    bool logout();

    /**
     * @brief 获取当前认证状态
     * @return 认证状态
     */
    AuthStatus currentStatus() const;

    /**
     * @brief 获取当前用户信息
     * @return 用户信息映射
     */
    QVariantMap getCurrentUser() const;

    /**
     * @brief 获取当前用户角色
     * @return 用户角色
     */
    UserRole getCurrentUserRole() const;

    /**
     * @brief 验证访问权限
     * @param resource 资源标识
     * @param action 操作类型
     * @return 是否有权限
     */
    bool checkPermission(const QString& resource, const QString& action);

    /**
     * @brief 刷新认证令牌
     * @return 刷新是否成功
     */
    bool refreshToken();

    /**
     * @brief 验证令牌有效性
     * @param token 令牌
     * @return 是否有效
     */
    bool validateToken(const QString& token);

    /**
     * @brief 获取认证令牌
     * @return 认证令牌
     */
    QString getAuthToken() const;

    /**
     * @brief 设置认证令牌
     * @param token 认证令牌
     */
    void setAuthToken(const QString& token);

    /**
     * @brief 获取会话信息
     * @return 会话信息映射
     */
    QVariantMap getSessionInfo() const;

    /**
     * @brief 设置认证服务器
     * @param serverUrl 服务器URL
     */
    void setAuthServer(const QString& serverUrl);

    /**
     * @brief 获取认证服务器
     * @return 服务器URL
     */
    QString authServer() const;

    /**
     * @brief 设置令牌过期时间
     * @param seconds 过期时间（秒）
     */
    void setTokenExpiration(int seconds);

    /**
     * @brief 获取令牌过期时间
     * @return 过期时间（秒）
     */
    int tokenExpiration() const;

    /**
     * @brief 启用自动刷新令牌
     * @param enabled 是否启用
     */
    void setAutoRefreshEnabled(bool enabled);

    /**
     * @brief 获取自动刷新令牌是否启用
     * @return 是否启用
     */
    bool isAutoRefreshEnabled() const;

    /**
     * @brief 进行访客认证
     * @param displayName 显示名称
     * @param email 邮箱（可选）
     * @return 认证是否成功启动
     */
    bool authenticateAsGuest(const QString& displayName, const QString& email = QString());

    /**
     * @brief 进行密码认证
     * @param username 用户名
     * @param password 密码
     * @return 认证是否成功启动
     */
    bool authenticateWithPassword(const QString& username, const QString& password);

    /**
     * @brief 进行令牌认证
     * @param token 认证令牌
     * @return 认证是否成功启动
     */
    bool authenticateWithToken(const QString& token);

    /**
     * @brief 进行JWT认证
     * @param jwt JWT令牌
     * @return 认证是否成功启动
     */
    bool authenticateWithJWT(const QString& jwt);

    /**
     * @brief 进行SSO认证
     * @param ssoProvider SSO提供商
     * @param redirectUrl 重定向URL
     * @return 认证是否成功启动
     */
    bool authenticateWithSSO(const QString& ssoProvider, const QString& redirectUrl = QString());

    /**
     * @brief 获取支持的认证类型
     * @return 认证类型列表
     */
    QList<AuthType> getSupportedAuthTypes() const;

    /**
     * @brief 检查认证是否过期
     * @return 是否过期
     */
    bool isAuthExpired() const;

    /**
     * @brief 获取认证剩余时间
     * @return 剩余时间（秒）
     */
    int getAuthTimeRemaining() const;

public slots:
    /**
     * @brief 检查认证状态
     */
    void checkAuthStatus();

    /**
     * @brief 处理认证过期
     */
    void handleAuthExpiration();

signals:
    /**
     * @brief 认证状态改变信号
     * @param status 新状态
     */
    void authStatusChanged(AuthStatus status);

    /**
     * @brief 认证成功信号
     * @param userInfo 用户信息
     */
    void authenticationSucceeded(const QVariantMap& userInfo);

    /**
     * @brief 认证失败信号
     * @param error 错误信息
     */
    void authenticationFailed(const QString& error);

    /**
     * @brief 认证过期信号
     */
    void authenticationExpired();

    /**
     * @brief 令牌刷新成功信号
     * @param newToken 新令牌
     */
    void tokenRefreshed(const QString& newToken);

    /**
     * @brief 令牌刷新失败信号
     * @param error 错误信息
     */
    void tokenRefreshFailed(const QString& error);

    /**
     * @brief 用户角色改变信号
     * @param role 新角色
     */
    void userRoleChanged(UserRole role);

    /**
     * @brief 权限改变信号
     * @param permissions 权限映射
     */
    void permissionsChanged(const QVariantMap& permissions);

    /**
     * @brief 注销完成信号
     */
    void loggedOut();

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief 处理网络回复
     */
    void handleNetworkReply();

    /**
     * @brief 处理令牌刷新定时器
     */
    void handleTokenRefreshTimer();

    /**
     * @brief 处理认证超时
     */
    void handleAuthTimeout();

private:
    /**
     * @brief 初始化网络管理器
     */
    void initializeNetworkManager();

    /**
     * @brief 设置认证状态
     * @param status 新状态
     */
    void setAuthStatus(AuthStatus status);

    /**
     * @brief 处理认证响应
     * @param response 响应数据
     */
    void processAuthResponse(const QVariantMap& response);

    /**
     * @brief 启动令牌刷新定时器
     */
    void startTokenRefreshTimer();

    /**
     * @brief 停止令牌刷新定时器
     */
    void stopTokenRefreshTimer();

    /**
     * @brief 解析JWT令牌
     * @param jwt JWT令牌
     * @return 解析结果
     */
    QVariantMap parseJWT(const QString& jwt) const;

    /**
     * @brief 验证JWT签名
     * @param jwt JWT令牌
     * @return 是否有效
     */
    bool validateJWTSignature(const QString& jwt);

    /**
     * @brief 构建认证请求
     * @param authType 认证类型
     * @param credentials 认证凭据
     * @return 请求数据
     */
    QVariantMap buildAuthRequest(AuthType authType, const QVariantMap& credentials);

    /**
     * @brief 存储认证信息
     * @param authInfo 认证信息
     */
    void storeAuthInfo(const QVariantMap& authInfo);

    /**
     * @brief 清除认证信息
     */
    void clearAuthInfo();

    /**
     * @brief 加载存储的认证信息
     * @return 认证信息
     */
    QVariantMap loadStoredAuthInfo();

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // AUTHHANDLER_H