#ifndef CHATCONFIG_H
#define CHATCONFIG_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QUrl>
#include <QSize>

/**
 * @brief 聊天配置类
 * 
 * ChatConfig管理聊天模块的所有配置参数，
 * 包括服务器设置、消息设置、界面设置等。
 */
class ChatConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(bool useSSL READ useSSL WRITE setUseSSL NOTIFY useSSLChanged)
    Q_PROPERTY(int maxMessageLength READ maxMessageLength WRITE setMaxMessageLength NOTIFY maxMessageLengthChanged)
    Q_PROPERTY(bool historyEnabled READ isHistoryEnabled WRITE setHistoryEnabled NOTIFY historyEnabledChanged)
    Q_PROPERTY(int historyLimit READ historyLimit WRITE setHistoryLimit NOTIFY historyLimitChanged)
    Q_PROPERTY(bool notificationsEnabled READ areNotificationsEnabled WRITE setNotificationsEnabled NOTIFY notificationsEnabledChanged)
    Q_PROPERTY(bool soundEnabled READ isSoundEnabled WRITE setSoundEnabled NOTIFY soundEnabledChanged)
    Q_PROPERTY(bool autoReconnect READ isAutoReconnectEnabled WRITE setAutoReconnectEnabled NOTIFY autoReconnectChanged)

public:
    /**
     * @brief 消息过滤级别枚举
     */
    enum MessageFilterLevel {
        NoFilter,           ///< 无过滤
        BasicFilter,        ///< 基础过滤
        ModerateFilter,     ///< 中等过滤
        StrictFilter        ///< 严格过滤
    };
    Q_ENUM(MessageFilterLevel)

    /**
     * @brief 通知类型枚举
     */
    enum NotificationType {
        None = 0x00,        ///< 无通知
        Sound = 0x01,       ///< 声音通知
        Visual = 0x02,      ///< 视觉通知
        Desktop = 0x04,     ///< 桌面通知
        All = 0xFF          ///< 所有通知
    };
    Q_ENUM(NotificationType)
    Q_DECLARE_FLAGS(NotificationTypes, NotificationType)

    /**
     * @brief 连接模式枚举
     */
    enum ConnectionMode {
        DirectConnection,   ///< 直接连接
        ProxyConnection,    ///< 代理连接
        AutoDetect          ///< 自动检测
    };
    Q_ENUM(ConnectionMode)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ChatConfig(QObject *parent = nullptr);

    /**
     * @brief 拷贝构造函数
     * @param other 其他配置对象
     */
    ChatConfig(const ChatConfig& other);

    /**
     * @brief 赋值操作符
     * @param other 其他配置对象
     * @return 当前对象引用
     */
    ChatConfig& operator=(const ChatConfig& other);

    /**
     * @brief 析构函数
     */
    ~ChatConfig();

    // 服务器配置
    /**
     * @brief 获取服务器URL
     * @return 服务器URL
     */
    QString serverUrl() const;

    /**
     * @brief 设置服务器URL
     * @param url 服务器URL
     */
    void setServerUrl(const QString& url);

    /**
     * @brief 获取服务器端口
     * @return 服务器端口
     */
    int serverPort() const;

    /**
     * @brief 设置服务器端口
     * @param port 服务器端口
     */
    void setServerPort(int port);

    /**
     * @brief 检查是否使用SSL
     * @return 是否使用SSL
     */
    bool useSSL() const;

    /**
     * @brief 设置是否使用SSL
     * @param useSSL 是否使用SSL
     */
    void setUseSSL(bool useSSL);

    /**
     * @brief 获取连接模式
     * @return 连接模式
     */
    ConnectionMode connectionMode() const;

    /**
     * @brief 设置连接模式
     * @param mode 连接模式
     */
    void setConnectionMode(ConnectionMode mode);

    /**
     * @brief 获取代理设置
     * @return 代理设置
     */
    QVariantMap proxySettings() const;

    /**
     * @brief 设置代理设置
     * @param settings 代理设置
     */
    void setProxySettings(const QVariantMap& settings);

    // 消息配置
    /**
     * @brief 获取最大消息长度
     * @return 最大消息长度
     */
    int maxMessageLength() const;

    /**
     * @brief 设置最大消息长度
     * @param length 最大消息长度
     */
    void setMaxMessageLength(int length);

    /**
     * @brief 获取消息过滤级别
     * @return 过滤级别
     */
    MessageFilterLevel messageFilterLevel() const;

    /**
     * @brief 设置消息过滤级别
     * @param level 过滤级别
     */
    void setMessageFilterLevel(MessageFilterLevel level);

    /**
     * @brief 获取过滤关键词列表
     * @return 关键词列表
     */
    QStringList filterKeywords() const;

    /**
     * @brief 设置过滤关键词列表
     * @param keywords 关键词列表
     */
    void setFilterKeywords(const QStringList& keywords);

    /**
     * @brief 添加过滤关键词
     * @param keyword 关键词
     */
    void addFilterKeyword(const QString& keyword);

    /**
     * @brief 移除过滤关键词
     * @param keyword 关键词
     */
    void removeFilterKeyword(const QString& keyword);

    /**
     * @brief 检查是否启用表情符号
     * @return 是否启用表情符号
     */
    bool isEmojiEnabled() const;

    /**
     * @brief 设置表情符号启用状态
     * @param enabled 是否启用
     */
    void setEmojiEnabled(bool enabled);

    /**
     * @brief 检查是否启用文件分享
     * @return 是否启用文件分享
     */
    bool isFileShareEnabled() const;

    /**
     * @brief 设置文件分享启用状态
     * @param enabled 是否启用
     */
    void setFileShareEnabled(bool enabled);

    /**
     * @brief 获取最大文件大小（字节）
     * @return 最大文件大小
     */
    qint64 maxFileSize() const;

    /**
     * @brief 设置最大文件大小（字节）
     * @param size 最大文件大小
     */
    void setMaxFileSize(qint64 size);

    /**
     * @brief 获取允许的文件类型
     * @return 文件类型列表
     */
    QStringList allowedFileTypes() const;

    /**
     * @brief 设置允许的文件类型
     * @param types 文件类型列表
     */
    void setAllowedFileTypes(const QStringList& types);

    // 历史记录配置
    /**
     * @brief 检查是否启用历史记录
     * @return 是否启用历史记录
     */
    bool isHistoryEnabled() const;

    /**
     * @brief 设置历史记录启用状态
     * @param enabled 是否启用
     */
    void setHistoryEnabled(bool enabled);

    /**
     * @brief 获取历史记录限制
     * @return 历史记录数量限制
     */
    int historyLimit() const;

    /**
     * @brief 设置历史记录限制
     * @param limit 历史记录数量限制
     */
    void setHistoryLimit(int limit);

    /**
     * @brief 获取历史记录保留天数
     * @return 保留天数
     */
    int historyRetentionDays() const;

    /**
     * @brief 设置历史记录保留天数
     * @param days 保留天数
     */
    void setHistoryRetentionDays(int days);

    /**
     * @brief 检查是否启用历史记录搜索
     * @return 是否启用搜索
     */
    bool isHistorySearchEnabled() const;

    /**
     * @brief 设置历史记录搜索启用状态
     * @param enabled 是否启用
     */
    void setHistorySearchEnabled(bool enabled);

    // 通知配置
    /**
     * @brief 检查是否启用通知
     * @return 是否启用通知
     */
    bool areNotificationsEnabled() const;

    /**
     * @brief 设置通知启用状态
     * @param enabled 是否启用
     */
    void setNotificationsEnabled(bool enabled);

    /**
     * @brief 获取通知类型
     * @return 通知类型标志
     */
    NotificationTypes notificationTypes() const;

    /**
     * @brief 设置通知类型
     * @param types 通知类型标志
     */
    void setNotificationTypes(NotificationTypes types);

    /**
     * @brief 检查是否启用声音
     * @return 是否启用声音
     */
    bool isSoundEnabled() const;

    /**
     * @brief 设置声音启用状态
     * @param enabled 是否启用
     */
    void setSoundEnabled(bool enabled);

    /**
     * @brief 获取通知声音文件路径
     * @return 声音文件路径
     */
    QString notificationSoundPath() const;

    /**
     * @brief 设置通知声音文件路径
     * @param path 声音文件路径
     */
    void setNotificationSoundPath(const QString& path);

    /**
     * @brief 获取通知显示时间（毫秒）
     * @return 显示时间
     */
    int notificationDisplayTime() const;

    /**
     * @brief 设置通知显示时间（毫秒）
     * @param time 显示时间
     */
    void setNotificationDisplayTime(int time);

    // 连接配置
    /**
     * @brief 检查是否启用自动重连
     * @return 是否启用自动重连
     */
    bool isAutoReconnectEnabled() const;

    /**
     * @brief 设置自动重连启用状态
     * @param enabled 是否启用
     */
    void setAutoReconnectEnabled(bool enabled);

    /**
     * @brief 获取重连间隔（秒）
     * @return 重连间隔
     */
    int reconnectInterval() const;

    /**
     * @brief 设置重连间隔（秒）
     * @param interval 重连间隔
     */
    void setReconnectInterval(int interval);

    /**
     * @brief 获取最大重连次数
     * @return 最大重连次数
     */
    int maxReconnectAttempts() const;

    /**
     * @brief 设置最大重连次数
     * @param attempts 最大重连次数
     */
    void setMaxReconnectAttempts(int attempts);

    /**
     * @brief 获取连接超时时间（秒）
     * @return 连接超时时间
     */
    int connectionTimeout() const;

    /**
     * @brief 设置连接超时时间（秒）
     * @param timeout 连接超时时间
     */
    void setConnectionTimeout(int timeout);

    // 界面配置
    /**
     * @brief 获取聊天窗口大小
     * @return 窗口大小
     */
    QSize chatWindowSize() const;

    /**
     * @brief 设置聊天窗口大小
     * @param size 窗口大小
     */
    void setChatWindowSize(const QSize& size);

    /**
     * @brief 获取字体大小
     * @return 字体大小
     */
    int fontSize() const;

    /**
     * @brief 设置字体大小
     * @param size 字体大小
     */
    void setFontSize(int size);

    /**
     * @brief 获取主题名称
     * @return 主题名称
     */
    QString themeName() const;

    /**
     * @brief 设置主题名称
     * @param theme 主题名称
     */
    void setThemeName(const QString& theme);

    /**
     * @brief 检查是否显示时间戳
     * @return 是否显示时间戳
     */
    bool showTimestamps() const;

    /**
     * @brief 设置时间戳显示状态
     * @param show 是否显示
     */
    void setShowTimestamps(bool show);

    /**
     * @brief 检查是否显示头像
     * @return 是否显示头像
     */
    bool showAvatars() const;

    /**
     * @brief 设置头像显示状态
     * @param show 是否显示
     */
    void setShowAvatars(bool show);

    // 扩展配置
    /**
     * @brief 获取自定义配置
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    QVariant customSetting(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 设置自定义配置
     * @param key 配置键
     * @param value 配置值
     */
    void setCustomSetting(const QString& key, const QVariant& value);

    /**
     * @brief 获取所有自定义配置
     * @return 配置映射
     */
    QVariantMap customSettings() const;

    /**
     * @brief 设置自定义配置
     * @param settings 配置映射
     */
    void setCustomSettings(const QVariantMap& settings);

    // 配置管理
    /**
     * @brief 加载配置
     * @param filePath 配置文件路径
     * @return 加载是否成功
     */
    bool loadFromFile(const QString& filePath);

    /**
     * @brief 保存配置
     * @param filePath 配置文件路径
     * @return 保存是否成功
     */
    bool saveToFile(const QString& filePath) const;

    /**
     * @brief 转换为变体映射
     * @return 变体映射
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 从变体映射加载
     * @param map 变体映射
     */
    void fromVariantMap(const QVariantMap& map);

    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults();

    /**
     * @brief 验证配置
     * @return 验证是否通过
     */
    bool validate() const;

    /**
     * @brief 克隆配置
     * @param parent 父对象
     * @return 克隆的配置对象
     */
    ChatConfig* clone(QObject* parent = nullptr) const;

    /**
     * @brief 比较配置
     * @param other 其他配置
     * @return 是否相等
     */
    bool equals(const ChatConfig* other) const;

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
     * @param url 新URL
     */
    void serverUrlChanged(const QString& url);

    /**
     * @brief 服务器端口改变信号
     * @param port 新端口
     */
    void serverPortChanged(int port);

    /**
     * @brief SSL使用状态改变信号
     * @param useSSL 是否使用SSL
     */
    void useSSLChanged(bool useSSL);

    /**
     * @brief 最大消息长度改变信号
     * @param length 新长度
     */
    void maxMessageLengthChanged(int length);

    /**
     * @brief 历史记录启用状态改变信号
     * @param enabled 是否启用
     */
    void historyEnabledChanged(bool enabled);

    /**
     * @brief 历史记录限制改变信号
     * @param limit 新限制
     */
    void historyLimitChanged(int limit);

    /**
     * @brief 通知启用状态改变信号
     * @param enabled 是否启用
     */
    void notificationsEnabledChanged(bool enabled);

    /**
     * @brief 声音启用状态改变信号
     * @param enabled 是否启用
     */
    void soundEnabledChanged(bool enabled);

    /**
     * @brief 自动重连状态改变信号
     * @param enabled 是否启用
     */
    void autoReconnectChanged(bool enabled);

    /**
     * @brief 配置改变信号
     */
    void configurationChanged();

    /**
     * @brief 自定义设置改变信号
     * @param key 设置键
     * @param value 新值
     */
    void customSettingChanged(const QString& key, const QVariant& value);

private:
    /**
     * @brief 验证服务器URL
     * @param url 服务器URL
     * @return 是否有效
     */
    bool validateServerUrl(const QString& url) const;

    /**
     * @brief 验证端口号
     * @param port 端口号
     * @return 是否有效
     */
    bool validatePort(int port) const;

    /**
     * @brief 初始化默认值
     */
    void initializeDefaults();

private:
    class Private;
    Private* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ChatConfig::NotificationTypes)
Q_DECLARE_METATYPE(ChatConfig*)
Q_DECLARE_METATYPE(ChatConfig::MessageFilterLevel)
Q_DECLARE_METATYPE(ChatConfig::NotificationType)
Q_DECLARE_METATYPE(ChatConfig::NotificationTypes)
Q_DECLARE_METATYPE(ChatConfig::ConnectionMode)

#endif // CHATCONFIG_H