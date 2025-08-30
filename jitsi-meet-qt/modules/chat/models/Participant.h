#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QUrl>

/**
 * @brief 参与者数据模型
 * 
 * Participant表示聊天室中的一个参与者，包含参与者的
 * 所有相关信息如ID、名称、状态、权限等。
 */
class Participant : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(ParticipantStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(ParticipantRole role READ role WRITE setRole NOTIFY roleChanged)
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY onlineStatusChanged)
    Q_PROPERTY(bool isMuted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool isVideoEnabled READ isVideoEnabled WRITE setVideoEnabled NOTIFY videoEnabledChanged)
    Q_PROPERTY(QDateTime joinTime READ joinTime CONSTANT)
    Q_PROPERTY(QDateTime lastActivity READ lastActivity NOTIFY lastActivityChanged)

public:
    /**
     * @brief 参与者状态枚举
     */
    enum ParticipantStatus {
        Online,             ///< 在线
        Away,               ///< 离开
        Busy,               ///< 忙碌
        DoNotDisturb,       ///< 请勿打扰
        Invisible,          ///< 隐身
        Offline             ///< 离线
    };
    Q_ENUM(ParticipantStatus)

    /**
     * @brief 参与者角色枚举
     */
    enum ParticipantRole {
        Guest,              ///< 访客
        Member,             ///< 成员
        Moderator,          ///< 主持人
        Administrator,      ///< 管理员
        Owner               ///< 拥有者
    };
    Q_ENUM(ParticipantRole)

    /**
     * @brief 权限枚举
     */
    enum Permission {
        None = 0x00,        ///< 无权限
        Read = 0x01,        ///< 读取权限
        Write = 0x02,       ///< 写入权限
        Moderate = 0x04,    ///< 管理权限
        Invite = 0x08,      ///< 邀请权限
        Kick = 0x10,        ///< 踢出权限
        Ban = 0x20,         ///< 封禁权限
        ManageRoom = 0x40,  ///< 房间管理权限
        All = 0xFF          ///< 所有权限
    };
    Q_ENUM(Permission)
    Q_DECLARE_FLAGS(Permissions, Permission)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit Participant(QObject *parent = nullptr);

    /**
     * @brief 构造函数
     * @param id 参与者ID
     * @param name 参与者名称
     * @param parent 父对象
     */
    explicit Participant(const QString& id, const QString& name, QObject *parent = nullptr);

    /**
     * @brief 拷贝构造函数
     * @param other 其他参与者对象
     */
    Participant(const Participant& other);

    /**
     * @brief 赋值操作符
     * @param other 其他参与者对象
     * @return 当前对象引用
     */
    Participant& operator=(const Participant& other);

    /**
     * @brief 析构函数
     */
    ~Participant();

    /**
     * @brief 获取参与者ID
     * @return 参与者ID
     */
    QString id() const;

    /**
     * @brief 获取参与者名称
     * @return 参与者名称
     */
    QString name() const;

    /**
     * @brief 设置参与者名称
     * @param name 参与者名称
     */
    void setName(const QString& name);

    /**
     * @brief 获取显示名称
     * @return 显示名称
     */
    QString displayName() const;

    /**
     * @brief 设置显示名称
     * @param displayName 显示名称
     */
    void setDisplayName(const QString& displayName);

    /**
     * @brief 获取邮箱地址
     * @return 邮箱地址
     */
    QString email() const;

    /**
     * @brief 设置邮箱地址
     * @param email 邮箱地址
     */
    void setEmail(const QString& email);

    /**
     * @brief 获取参与者状态
     * @return 参与者状态
     */
    ParticipantStatus status() const;

    /**
     * @brief 设置参与者状态
     * @param status 参与者状态
     */
    void setStatus(ParticipantStatus status);

    /**
     * @brief 获取参与者角色
     * @return 参与者角色
     */
    ParticipantRole role() const;

    /**
     * @brief 设置参与者角色
     * @param role 参与者角色
     */
    void setRole(ParticipantRole role);

    /**
     * @brief 检查是否在线
     * @return 是否在线
     */
    bool isOnline() const;

    /**
     * @brief 检查是否静音
     * @return 是否静音
     */
    bool isMuted() const;

    /**
     * @brief 设置静音状态
     * @param muted 是否静音
     */
    void setMuted(bool muted);

    /**
     * @brief 检查视频是否启用
     * @return 视频是否启用
     */
    bool isVideoEnabled() const;

    /**
     * @brief 设置视频启用状态
     * @param enabled 是否启用视频
     */
    void setVideoEnabled(bool enabled);

    /**
     * @brief 获取加入时间
     * @return 加入时间
     */
    QDateTime joinTime() const;

    /**
     * @brief 获取最后活动时间
     * @return 最后活动时间
     */
    QDateTime lastActivity() const;

    /**
     * @brief 更新最后活动时间
     */
    void updateLastActivity();

    /**
     * @brief 获取头像URL
     * @return 头像URL
     */
    QUrl avatarUrl() const;

    /**
     * @brief 设置头像URL
     * @param url 头像URL
     */
    void setAvatarUrl(const QUrl& url);

    /**
     * @brief 获取状态消息
     * @return 状态消息
     */
    QString statusMessage() const;

    /**
     * @brief 设置状态消息
     * @param message 状态消息
     */
    void setStatusMessage(const QString& message);

    /**
     * @brief 获取权限
     * @return 权限标志
     */
    Permissions permissions() const;

    /**
     * @brief 设置权限
     * @param permissions 权限标志
     */
    void setPermissions(Permissions permissions);

    /**
     * @brief 检查是否有特定权限
     * @param permission 权限
     * @return 是否有权限
     */
    bool hasPermission(Permission permission) const;

    /**
     * @brief 添加权限
     * @param permission 权限
     */
    void addPermission(Permission permission);

    /**
     * @brief 移除权限
     * @param permission 权限
     */
    void removePermission(Permission permission);

    /**
     * @brief 获取客户端信息
     * @return 客户端信息
     */
    QString clientInfo() const;

    /**
     * @brief 设置客户端信息
     * @param clientInfo 客户端信息
     */
    void setClientInfo(const QString& clientInfo);

    /**
     * @brief 获取IP地址
     * @return IP地址
     */
    QString ipAddress() const;

    /**
     * @brief 设置IP地址
     * @param ipAddress IP地址
     */
    void setIpAddress(const QString& ipAddress);

    /**
     * @brief 获取地理位置
     * @return 地理位置信息
     */
    QVariantMap location() const;

    /**
     * @brief 设置地理位置
     * @param location 地理位置信息
     */
    void setLocation(const QVariantMap& location);

    /**
     * @brief 获取扩展属性
     * @param key 属性键
     * @param defaultValue 默认值
     * @return 属性值
     */
    QVariant property(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 设置扩展属性
     * @param key 属性键
     * @param value 属性值
     */
    void setProperty(const QString& key, const QVariant& value);

    /**
     * @brief 获取所有扩展属性
     * @return 属性映射
     */
    QVariantMap properties() const;

    /**
     * @brief 设置扩展属性
     * @param properties 属性映射
     */
    void setProperties(const QVariantMap& properties);

    /**
     * @brief 获取统计信息
     * @return 统计信息
     */
    QVariantMap statistics() const;

    /**
     * @brief 更新统计信息
     * @param stats 统计信息
     */
    void updateStatistics(const QVariantMap& stats);

    /**
     * @brief 转换为变体映射
     * @return 变体映射
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 从变体映射创建
     * @param map 变体映射
     * @return 参与者对象
     */
    static Participant* fromVariantMap(const QVariantMap& map, QObject* parent = nullptr);

    /**
     * @brief 转换为JSON字符串
     * @return JSON字符串
     */
    QString toJson() const;

    /**
     * @brief 从JSON字符串创建
     * @param json JSON字符串
     * @return 参与者对象
     */
    static Participant* fromJson(const QString& json, QObject* parent = nullptr);

    /**
     * @brief 克隆参与者
     * @param parent 父对象
     * @return 克隆的参与者对象
     */
    Participant* clone(QObject* parent = nullptr) const;

    /**
     * @brief 验证参与者数据
     * @return 验证是否通过
     */
    bool validate() const;

    /**
     * @brief 比较参与者
     * @param other 其他参与者
     * @return 是否相等
     */
    bool equals(const Participant* other) const;

    /**
     * @brief 获取在线时长（秒）
     * @return 在线时长
     */
    qint64 onlineDuration() const;

    /**
     * @brief 检查是否为管理员
     * @return 是否为管理员
     */
    bool isAdministrator() const;

    /**
     * @brief 检查是否为主持人
     * @return 是否为主持人
     */
    bool isModerator() const;

    /**
     * @brief 检查是否为拥有者
     * @return 是否为拥有者
     */
    bool isOwner() const;

    /**
     * @brief 检查是否为访客
     * @return 是否为访客
     */
    bool isGuest() const;

public slots:
    /**
     * @brief 设置在线状态
     */
    void setOnline();

    /**
     * @brief 设置离线状态
     */
    void setOffline();

    /**
     * @brief 切换静音状态
     */
    void toggleMute();

    /**
     * @brief 切换视频状态
     */
    void toggleVideo();

    /**
     * @brief 提升为主持人
     */
    void promoteToModerator();

    /**
     * @brief 降级为成员
     */
    void demoteToMember();

    /**
     * @brief 踢出参与者
     */
    void kick();

    /**
     * @brief 封禁参与者
     */
    void ban();

signals:
    /**
     * @brief 名称改变信号
     * @param name 新名称
     */
    void nameChanged(const QString& name);

    /**
     * @brief 显示名称改变信号
     * @param displayName 新显示名称
     */
    void displayNameChanged(const QString& displayName);

    /**
     * @brief 邮箱改变信号
     * @param email 新邮箱
     */
    void emailChanged(const QString& email);

    /**
     * @brief 状态改变信号
     * @param status 新状态
     */
    void statusChanged(ParticipantStatus status);

    /**
     * @brief 角色改变信号
     * @param role 新角色
     */
    void roleChanged(ParticipantRole role);

    /**
     * @brief 在线状态改变信号
     * @param online 是否在线
     */
    void onlineStatusChanged(bool online);

    /**
     * @brief 静音状态改变信号
     * @param muted 是否静音
     */
    void mutedChanged(bool muted);

    /**
     * @brief 视频启用状态改变信号
     * @param enabled 是否启用视频
     */
    void videoEnabledChanged(bool enabled);

    /**
     * @brief 最后活动时间改变信号
     * @param lastActivity 新的最后活动时间
     */
    void lastActivityChanged(const QDateTime& lastActivity);

    /**
     * @brief 权限改变信号
     * @param permissions 新权限
     */
    void permissionsChanged(Permissions permissions);

    /**
     * @brief 头像改变信号
     * @param avatarUrl 新头像URL
     */
    void avatarChanged(const QUrl& avatarUrl);

    /**
     * @brief 状态消息改变信号
     * @param message 新状态消息
     */
    void statusMessageChanged(const QString& message);

    /**
     * @brief 属性改变信号
     * @param key 属性键
     * @param value 新值
     */
    void propertyChanged(const QString& key, const QVariant& value);

    /**
     * @brief 统计信息更新信号
     * @param statistics 新统计信息
     */
    void statisticsUpdated(const QVariantMap& statistics);

private:
    /**
     * @brief 验证参与者ID
     * @param id 参与者ID
     * @return 是否有效
     */
    bool validateId(const QString& id) const;

    /**
     * @brief 验证参与者名称
     * @param name 参与者名称
     * @return 是否有效
     */
    bool validateName(const QString& name) const;

    /**
     * @brief 验证邮箱地址
     * @param email 邮箱地址
     * @return 是否有效
     */
    bool validateEmail(const QString& email) const;

private:
    class Private;
    Private* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Participant::Permissions)
Q_DECLARE_METATYPE(Participant*)
Q_DECLARE_METATYPE(Participant::ParticipantStatus)
Q_DECLARE_METATYPE(Participant::ParticipantRole)
Q_DECLARE_METATYPE(Participant::Permission)
Q_DECLARE_METATYPE(Participant::Permissions)

#endif // PARTICIPANT_H