#ifndef MEETINGCONFIG_H
#define MEETINGCONFIG_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QStringList>
#include <QSettings>

/**
 * @brief 会议配置管理类
 * 
 * 管理会议模块的所有配置选项，包括服务器设置、用户偏好和会议参数
 */
class MeetingConfig : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 音频质量枚举
     */
    enum AudioQuality {
        LowAudio,        ///< 低质量音频
        StandardAudio,   ///< 标准音频
        HighAudio        ///< 高质量音频
    };
    Q_ENUM(AudioQuality)

    /**
     * @brief 视频质量枚举
     */
    enum VideoQuality {
        LowVideo,        ///< 低质量视频
        StandardVideo,   ///< 标准视频
        HighVideo,       ///< 高质量视频
        UltraVideo       ///< 超高质量视频
    };
    Q_ENUM(VideoQuality)

    /**
     * @brief 认证方式枚举
     */
    enum AuthenticationMethod {
        GuestAuth,       ///< 访客认证
        PasswordAuth,    ///< 密码认证
        TokenAuth,       ///< 令牌认证
        SSOAuth          ///< 单点登录认证
    };
    Q_ENUM(AuthenticationMethod)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MeetingConfig(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MeetingConfig();

    /**
     * @brief 加载配置
     * @param configFile 配置文件路径（可选）
     * @return 加载是否成功
     */
    bool loadConfiguration(const QString& configFile = QString());

    /**
     * @brief 保存配置
     * @param configFile 配置文件路径（可选）
     * @return 保存是否成功
     */
    bool saveConfiguration(const QString& configFile = QString());

    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults();

    /**
     * @brief 验证配置
     * @return 配置是否有效
     */
    bool validateConfiguration() const;

    // 服务器设置
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
     * @brief 设置服务器列表
     * @param servers 服务器列表
     */
    void setServerList(const QStringList& servers);

    /**
     * @brief 获取服务器列表
     * @return 服务器列表
     */
    QStringList serverList() const;

    /**
     * @brief 添加服务器
     * @param server 服务器地址
     */
    void addServer(const QString& server);

    /**
     * @brief 移除服务器
     * @param server 服务器地址
     */
    void removeServer(const QString& server);

    // 用户设置
    /**
     * @brief 设置默认显示名称
     * @param name 显示名称
     */
    void setDefaultDisplayName(const QString& name);

    /**
     * @brief 获取默认显示名称
     * @return 显示名称
     */
    QString defaultDisplayName() const;

    /**
     * @brief 设置默认邮箱
     * @param email 邮箱地址
     */
    void setDefaultEmail(const QString& email);

    /**
     * @brief 获取默认邮箱
     * @return 邮箱地址
     */
    QString defaultEmail() const;

    // 会议设置
    /**
     * @brief 设置自动加入
     * @param autoJoin 是否自动加入
     */
    void setAutoJoin(bool autoJoin);

    /**
     * @brief 获取自动加入设置
     * @return 是否自动加入
     */
    bool autoJoin() const;

    /**
     * @brief 设置默认音频状态
     * @param enabled 是否启用
     */
    void setDefaultAudioEnabled(bool enabled);

    /**
     * @brief 获取默认音频状态
     * @return 是否启用
     */
    bool defaultAudioEnabled() const;

    /**
     * @brief 设置默认视频状态
     * @param enabled 是否启用
     */
    void setDefaultVideoEnabled(bool enabled);

    /**
     * @brief 获取默认视频状态
     * @return 是否启用
     */
    bool defaultVideoEnabled() const;

    /**
     * @brief 设置音频质量
     * @param quality 音频质量
     */
    void setAudioQuality(AudioQuality quality);

    /**
     * @brief 获取音频质量
     * @return 音频质量
     */
    AudioQuality audioQuality() const;

    /**
     * @brief 设置视频质量
     * @param quality 视频质量
     */
    void setVideoQuality(VideoQuality quality);

    /**
     * @brief 获取视频质量
     * @return 视频质量
     */
    VideoQuality videoQuality() const;

    // 认证设置
    /**
     * @brief 设置认证方式
     * @param method 认证方式
     */
    void setAuthenticationMethod(AuthenticationMethod method);

    /**
     * @brief 获取认证方式
     * @return 认证方式
     */
    AuthenticationMethod authenticationMethod() const;

    /**
     * @brief 设置是否记住认证信息
     * @param remember 是否记住
     */
    void setRememberAuthentication(bool remember);

    /**
     * @brief 获取是否记住认证信息
     * @return 是否记住
     */
    bool rememberAuthentication() const;

    // 网络设置
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
     * @brief 设置重连尝试次数
     * @param attempts 尝试次数
     */
    void setReconnectAttempts(int attempts);

    /**
     * @brief 获取重连尝试次数
     * @return 尝试次数
     */
    int reconnectAttempts() const;

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

    // 界面设置
    /**
     * @brief 设置是否显示加入对话框
     * @param show 是否显示
     */
    void setShowJoinDialog(bool show);

    /**
     * @brief 获取是否显示加入对话框
     * @return 是否显示
     */
    bool showJoinDialog() const;

    /**
     * @brief 设置是否最小化到系统托盘
     * @param minimize 是否最小化
     */
    void setMinimizeToTray(bool minimize);

    /**
     * @brief 获取是否最小化到系统托盘
     * @return 是否最小化
     */
    bool minimizeToTray() const;

    // 高级设置
    /**
     * @brief 设置调试模式
     * @param enabled 是否启用
     */
    void setDebugEnabled(bool enabled);

    /**
     * @brief 获取调试模式
     * @return 是否启用
     */
    bool debugEnabled() const;

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void setLogLevel(const QString& level);

    /**
     * @brief 获取日志级别
     * @return 日志级别
     */
    QString logLevel() const;

    /**
     * @brief 设置自定义配置
     * @param key 配置键
     * @param value 配置值
     */
    void setCustomSetting(const QString& key, const QVariant& value);

    /**
     * @brief 获取自定义配置
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    QVariant getCustomSetting(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 转换为映射
     * @return 配置映射
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 从映射加载
     * @param map 配置映射
     */
    void fromVariantMap(const QVariantMap& map);

signals:
    /**
     * @brief 配置改变信号
     * @param key 改变的配置键
     * @param value 新值
     */
    void configurationChanged(const QString& key, const QVariant& value);

    /**
     * @brief 服务器列表改变信号
     * @param servers 新的服务器列表
     */
    void serverListChanged(const QStringList& servers);

    /**
     * @brief 用户设置改变信号
     * @param settings 用户设置映射
     */
    void userSettingsChanged(const QVariantMap& settings);

private:
    /**
     * @brief 初始化默认值
     */
    void initializeDefaults();

    /**
     * @brief 发出配置改变信号
     * @param key 配置键
     * @param value 新值
     */
    void emitConfigurationChanged(const QString& key, const QVariant& value);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // MEETINGCONFIG_H