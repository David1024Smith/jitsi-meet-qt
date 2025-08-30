#ifndef ROOM_H
#define ROOM_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QVariantList>

/**
 * @brief 房间数据模型类
 * 
 * 表示一个会议房间的信息，包括房间设置、参与者管理和状态信息
 */
class Room : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged)
    Q_PROPERTY(RoomType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(RoomStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QDateTime createdAt READ createdAt WRITE setCreatedAt NOTIFY createdAtChanged)
    Q_PROPERTY(QString ownerId READ ownerId WRITE setOwnerId NOTIFY ownerIdChanged)
    Q_PROPERTY(int participantCount READ participantCount NOTIFY participantCountChanged)
    Q_PROPERTY(int maxParticipants READ maxParticipants WRITE setMaxParticipants NOTIFY maxParticipantsChanged)
    Q_PROPERTY(bool isLocked READ isLocked WRITE setLocked NOTIFY lockedChanged)
    Q_PROPERTY(bool isPublic READ isPublic WRITE setPublic NOTIFY publicChanged)
    Q_PROPERTY(bool allowGuests READ allowGuests WRITE setAllowGuests NOTIFY allowGuestsChanged)

public:
    /**
     * @brief 房间类型枚举
     */
    enum RoomType {
        PublicRoom,      ///< 公开房间
        PrivateRoom,     ///< 私人房间
        PasswordRoom,    ///< 密码房间
        InviteOnlyRoom,  ///< 仅邀请房间
        TemporaryRoom    ///< 临时房间
    };
    Q_ENUM(RoomType)

    /**
     * @brief 房间状态枚举
     */
    enum RoomStatus {
        Inactive,        ///< 未激活
        Active,          ///< 激活
        Locked,          ///< 锁定
        Full,            ///< 已满
        Suspended,       ///< 暂停
        Closed           ///< 已关闭
    };
    Q_ENUM(RoomStatus)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit Room(QObject* parent = nullptr);

    /**
     * @brief 构造函数（带参数）
     * @param name 房间名称
     * @param server 服务器地址
     * @param parent 父对象
     */
    explicit Room(const QString& name, const QString& server, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~Room();

    // 基本属性
    /**
     * @brief 获取房间ID
     * @return 房间ID
     */
    QString id() const;

    /**
     * @brief 设置房间ID
     * @param id 房间ID
     */
    void setId(const QString& id);

    /**
     * @brief 获取房间名称
     * @return 房间名称
     */
    QString name() const;

    /**
     * @brief 设置房间名称
     * @param name 房间名称
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
     * @brief 获取服务器地址
     * @return 服务器地址
     */
    QString server() const;

    /**
     * @brief 设置服务器地址
     * @param server 服务器地址
     */
    void setServer(const QString& server);

    /**
     * @brief 获取房间类型
     * @return 房间类型
     */
    RoomType type() const;

    /**
     * @brief 设置房间类型
     * @param type 房间类型
     */
    void setType(RoomType type);

    /**
     * @brief 获取房间状态
     * @return 房间状态
     */
    RoomStatus status() const;

    /**
     * @brief 设置房间状态
     * @param status 房间状态
     */
    void setStatus(RoomStatus status);

    /**
     * @brief 获取创建时间
     * @return 创建时间
     */
    QDateTime createdAt() const;

    /**
     * @brief 设置创建时间
     * @param createdAt 创建时间
     */
    void setCreatedAt(const QDateTime& createdAt);

    /**
     * @brief 获取房间所有者ID
     * @return 所有者ID
     */
    QString ownerId() const;

    /**
     * @brief 设置房间所有者ID
     * @param ownerId 所有者ID
     */
    void setOwnerId(const QString& ownerId);

    // 参与者管理
    /**
     * @brief 获取参与者数量
     * @return 参与者数量
     */
    int participantCount() const;

    /**
     * @brief 获取最大参与者数量
     * @return 最大参与者数量
     */
    int maxParticipants() const;

    /**
     * @brief 设置最大参与者数量
     * @param maxParticipants 最大参与者数量
     */
    void setMaxParticipants(int maxParticipants);

    /**
     * @brief 获取参与者列表
     * @return 参与者ID列表
     */
    QStringList participants() const;

    /**
     * @brief 添加参与者
     * @param participantId 参与者ID
     * @param role 参与者角色
     */
    void addParticipant(const QString& participantId, const QString& role = "participant");

    /**
     * @brief 移除参与者
     * @param participantId 参与者ID
     */
    void removeParticipant(const QString& participantId);

    /**
     * @brief 检查是否包含参与者
     * @param participantId 参与者ID
     * @return 是否包含
     */
    bool hasParticipant(const QString& participantId) const;

    /**
     * @brief 获取参与者角色
     * @param participantId 参与者ID
     * @return 参与者角色
     */
    QString getParticipantRole(const QString& participantId) const;

    /**
     * @brief 设置参与者角色
     * @param participantId 参与者ID
     * @param role 参与者角色
     */
    void setParticipantRole(const QString& participantId, const QString& role);

    /**
     * @brief 获取主持人列表
     * @return 主持人ID列表
     */
    QStringList moderators() const;

    /**
     * @brief 检查是否为主持人
     * @param participantId 参与者ID
     * @return 是否为主持人
     */
    bool isModerator(const QString& participantId) const;

    // 房间设置
    /**
     * @brief 获取是否锁定
     * @return 是否锁定
     */
    bool isLocked() const;

    /**
     * @brief 设置锁定状态
     * @param locked 是否锁定
     */
    void setLocked(bool locked);

    /**
     * @brief 获取是否公开
     * @return 是否公开
     */
    bool isPublic() const;

    /**
     * @brief 设置公开状态
     * @param isPublic 是否公开
     */
    void setPublic(bool isPublic);

    /**
     * @brief 获取是否允许访客
     * @return 是否允许访客
     */
    bool allowGuests() const;

    /**
     * @brief 设置是否允许访客
     * @param allowGuests 是否允许访客
     */
    void setAllowGuests(bool allowGuests);

    /**
     * @brief 获取房间密码
     * @return 房间密码
     */
    QString password() const;

    /**
     * @brief 设置房间密码
     * @param password 房间密码
     */
    void setPassword(const QString& password);

    /**
     * @brief 检查是否需要密码
     * @return 是否需要密码
     */
    bool requiresPassword() const;

    /**
     * @brief 验证密码
     * @param password 密码
     * @return 密码是否正确
     */
    bool validatePassword(const QString& password) const;

    /**
     * @brief 获取房间描述
     * @return 房间描述
     */
    QString description() const;

    /**
     * @brief 设置房间描述
     * @param description 房间描述
     */
    void setDescription(const QString& description);

    /**
     * @brief 获取房间主题
     * @return 房间主题
     */
    QString subject() const;

    /**
     * @brief 设置房间主题
     * @param subject 房间主题
     */
    void setSubject(const QString& subject);

    // 房间配置
    /**
     * @brief 获取房间配置
     * @return 配置映射
     */
    QVariantMap configuration() const;

    /**
     * @brief 设置房间配置
     * @param config 配置映射
     */
    void setConfiguration(const QVariantMap& config);

    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    QVariant getConfigValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 配置值
     */
    void setConfigValue(const QString& key, const QVariant& value);

    // 权限管理
    /**
     * @brief 获取权限设置
     * @return 权限映射
     */
    QVariantMap permissions() const;

    /**
     * @brief 设置权限设置
     * @param permissions 权限映射
     */
    void setPermissions(const QVariantMap& permissions);

    /**
     * @brief 检查权限
     * @param participantId 参与者ID
     * @param permission 权限名称
     * @return 是否有权限
     */
    bool hasPermission(const QString& participantId, const QString& permission) const;

    /**
     * @brief 授予权限
     * @param participantId 参与者ID
     * @param permission 权限名称
     */
    void grantPermission(const QString& participantId, const QString& permission);

    /**
     * @brief 撤销权限
     * @param participantId 参与者ID
     * @param permission 权限名称
     */
    void revokePermission(const QString& participantId, const QString& permission);

    // 统计信息
    /**
     * @brief 获取统计信息
     * @return 统计信息映射
     */
    QVariantMap statistics() const;

    /**
     * @brief 更新统计信息
     * @param stats 统计信息
     */
    void updateStatistics(const QVariantMap& stats);

    /**
     * @brief 获取房间使用时长
     * @return 使用时长（秒）
     */
    qint64 getUsageDuration() const;

    // 序列化
    /**
     * @brief 转换为映射
     * @return 房间信息映射
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 从映射加载
     * @param map 房间信息映射
     */
    void fromVariantMap(const QVariantMap& map);

    /**
     * @brief 转换为JSON字符串
     * @return JSON字符串
     */
    QString toJson() const;

    /**
     * @brief 从JSON字符串加载
     * @param json JSON字符串
     * @return 加载是否成功
     */
    bool fromJson(const QString& json);

    // 验证
    /**
     * @brief 验证房间数据
     * @return 是否有效
     */
    bool isValid() const;

    /**
     * @brief 获取验证错误
     * @return 错误列表
     */
    QStringList validationErrors() const;

    // 工具方法
    /**
     * @brief 生成房间ID
     * @return 房间ID
     */
    static QString generateRoomId();

    /**
     * @brief 生成房间URL
     * @param server 服务器地址
     * @param roomName 房间名称
     * @return 房间URL
     */
    static QString generateRoomUrl(const QString& server, const QString& roomName);

    /**
     * @brief 验证房间名称
     * @param name 房间名称
     * @return 是否有效
     */
    static bool validateRoomName(const QString& name);

signals:
    // 属性改变信号
    void idChanged(const QString& id);
    void nameChanged(const QString& name);
    void displayNameChanged(const QString& displayName);
    void serverChanged(const QString& server);
    void typeChanged(RoomType type);
    void statusChanged(RoomStatus status);
    void createdAtChanged(const QDateTime& createdAt);
    void ownerIdChanged(const QString& ownerId);
    void participantCountChanged(int count);
    void maxParticipantsChanged(int maxParticipants);
    void lockedChanged(bool locked);
    void publicChanged(bool isPublic);
    void allowGuestsChanged(bool allowGuests);

    // 参与者相关信号
    void participantAdded(const QString& participantId, const QString& role);
    void participantRemoved(const QString& participantId);
    void participantRoleChanged(const QString& participantId, const QString& role);

    // 配置相关信号
    void configurationChanged(const QVariantMap& configuration);
    void configValueChanged(const QString& key, const QVariant& value);

    // 权限相关信号
    void permissionsChanged(const QVariantMap& permissions);
    void permissionGranted(const QString& participantId, const QString& permission);
    void permissionRevoked(const QString& participantId, const QString& permission);

    // 统计信息信号
    void statisticsUpdated(const QVariantMap& statistics);

private:
    /**
     * @brief 更新参与者数量
     */
    void updateParticipantCount();

    /**
     * @brief 初始化默认配置
     */
    void initializeDefaultConfiguration();

    /**
     * @brief 初始化默认权限
     */
    void initializeDefaultPermissions();

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // ROOM_H