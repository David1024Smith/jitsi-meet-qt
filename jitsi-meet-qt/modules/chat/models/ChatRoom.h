#ifndef CHATROOM_H
#define CHATROOM_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <QUrl>

// Forward declarations
class ChatMessage;
class Participant;

/**
 * @brief 聊天房间数据模型
 * 
 * ChatRoom表示一个聊天房间，包含房间的所有相关信息
 * 如房间ID、名称、参与者、设置等。
 */
class ChatRoom : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(RoomType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(RoomStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(int participantCount READ participantCount NOTIFY participantCountChanged)
    Q_PROPERTY(int maxParticipants READ maxParticipants WRITE setMaxParticipants NOTIFY maxParticipantsChanged)
    Q_PROPERTY(bool isPrivate READ isPrivate WRITE setPrivate NOTIFY privateChanged)
    Q_PROPERTY(bool hasPassword READ hasPassword NOTIFY passwordChanged)
    Q_PROPERTY(QDateTime createdTime READ createdTime CONSTANT)
    Q_PROPERTY(QDateTime lastActivity READ lastActivity NOTIFY lastActivityChanged)

public:
    /**
     * @brief 房间类型枚举
     */
    enum RoomType {
        PublicRoom,         ///< 公开房间
        PrivateRoom,        ///< 私有房间
        DirectMessage,      ///< 直接消息
        GroupChat,          ///< 群聊
        ConferenceRoom,     ///< 会议房间
        TemporaryRoom       ///< 临时房间
    };
    Q_ENUM(RoomType)

    /**
     * @brief 房间状态枚举
     */
    enum RoomStatus {
        Active,             ///< 活跃
        Inactive,           ///< 不活跃
        Archived,           ///< 已归档
        Locked,             ///< 已锁定
        Suspended,          ///< 已暂停
        Deleted             ///< 已删除
    };
    Q_ENUM(RoomStatus)

    /**
     * @brief 权限级别枚举
     */
    enum PermissionLevel {
        None = 0,           ///< 无权限
        Read = 1,           ///< 只读
        Write = 2,          ///< 读写
        Moderate = 4,       ///< 管理
        Admin = 8,          ///< 管理员
        Owner = 16          ///< 拥有者
    };
    Q_ENUM(PermissionLevel)
    Q_DECLARE_FLAGS(Permissions, PermissionLevel)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ChatRoom(QObject *parent = nullptr);

    /**
     * @brief 构造函数
     * @param id 房间ID
     * @param name 房间名称
     * @param type 房间类型
     * @param parent 父对象
     */
    explicit ChatRoom(const QString& id, const QString& name, RoomType type = PublicRoom, QObject *parent = nullptr);

    /**
     * @brief 拷贝构造函数
     * @param other 其他房间对象
     */
    ChatRoom(const ChatRoom& other);

    /**
     * @brief 赋值操作符
     * @param other 其他房间对象
     * @return 当前对象引用
     */
    ChatRoom& operator=(const ChatRoom& other);

    /**
     * @brief 析构函数
     */
    ~ChatRoom();

    /**
     * @brief 获取房间ID
     * @return 房间ID
     */
    QString id() const;

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
    QDateTime createdTime() const;

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
     * @brief 检查是否为私有房间
     * @return 是否为私有房间
     */
    bool isPrivate() const;

    /**
     * @brief 设置私有房间状态
     * @param isPrivate 是否为私有房间
     */
    void setPrivate(bool isPrivate);

    /**
     * @brief 检查是否有密码
     * @return 是否有密码
     */
    bool hasPassword() const;

    /**
     * @brief 设置房间密码
     * @param password 密码
     */
    void setPassword(const QString& password);

    /**
     * @brief 验证密码
     * @param password 密码
     * @return 密码是否正确
     */
    bool validatePassword(const QString& password) const;

    /**
     * @brief 清除密码
     */
    void clearPassword();

    /**
     * @brief 获取房间主题
     * @return 房间主题
     */
    QString topic() const;

    /**
     * @brief 设置房间主题
     * @param topic 房间主题
     */
    void setTopic(const QString& topic);

    /**
     * @brief 获取房间头像URL
     * @return 头像URL
     */
    QUrl avatarUrl() const;

    /**
     * @brief 设置房间头像URL
     * @param url 头像URL
     */
    void setAvatarUrl(const QUrl& url);

    /**
     * @brief 获取参与者列表
     * @return 参与者列表
     */
    QList<Participant*> participants() const;

    /**
     * @brief 添加参与者
     * @param participant 参与者对象
     * @return 添加是否成功
     */
    bool addParticipant(Participant* participant);

    /**
     * @brief 移除参与者
     * @param participantId 参与者ID
     * @return 移除是否成功
     */
    bool removeParticipant(const QString& participantId);

    /**
     * @brief 获取参与者
     * @param participantId 参与者ID
     * @return 参与者对象，如果不存在则返回nullptr
     */
    Participant* getParticipant(const QString& participantId) const;

    /**
     * @brief 检查参与者是否存在
     * @param participantId 参与者ID
     * @return 是否存在
     */
    bool hasParticipant(const QString& participantId) const;

    /**
     * @brief 获取参与者ID列表
     * @return 参与者ID列表
     */
    QStringList participantIds() const;

    /**
     * @brief 获取管理员列表
     * @return 管理员ID列表
     */
    QStringList administrators() const;

    /**
     * @brief 添加管理员
     * @param participantId 参与者ID
     * @return 添加是否成功
     */
    bool addAdministrator(const QString& participantId);

    /**
     * @brief 移除管理员
     * @param participantId 参与者ID
     * @return 移除是否成功
     */
    bool removeAdministrator(const QString& participantId);

    /**
     * @brief 检查是否为管理员
     * @param participantId 参与者ID
     * @return 是否为管理员
     */
    bool isAdministrator(const QString& participantId) const;

    /**
     * @brief 获取房间拥有者
     * @return 拥有者ID
     */
    QString owner() const;

    /**
     * @brief 设置房间拥有者
     * @param ownerId 拥有者ID
     */
    void setOwner(const QString& ownerId);

    /**
     * @brief 获取用户权限
     * @param participantId 参与者ID
     * @return 权限级别
     */
    Permissions getUserPermissions(const QString& participantId) const;

    /**
     * @brief 设置用户权限
     * @param participantId 参与者ID
     * @param permissions 权限级别
     */
    void setUserPermissions(const QString& participantId, Permissions permissions);

    /**
     * @brief 获取房间设置
     * @return 设置映射
     */
    QVariantMap settings() const;

    /**
     * @brief 设置房间设置
     * @param settings 设置映射
     */
    void setSettings(const QVariantMap& settings);

    /**
     * @brief 获取设置值
     * @param key 设置键
     * @param defaultValue 默认值
     * @return 设置值
     */
    QVariant getSetting(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 设置设置值
     * @param key 设置键
     * @param value 设置值
     */
    void setSetting(const QString& key, const QVariant& value);

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
     * @brief 转换为变体映射
     * @return 变体映射
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 从变体映射创建
     * @param map 变体映射
     * @return 房间对象
     */
    static ChatRoom* fromVariantMap(const QVariantMap& map, QObject* parent = nullptr);

    /**
     * @brief 转换为JSON字符串
     * @return JSON字符串
     */
    QString toJson() const;

    /**
     * @brief 从JSON字符串创建
     * @param json JSON字符串
     * @return 房间对象
     */
    static ChatRoom* fromJson(const QString& json, QObject* parent = nullptr);

    /**
     * @brief 克隆房间
     * @param parent 父对象
     * @return 克隆的房间对象
     */
    ChatRoom* clone(QObject* parent = nullptr) const;

    /**
     * @brief 验证房间数据
     * @return 验证是否通过
     */
    bool validate() const;

    /**
     * @brief 比较房间
     * @param other 其他房间
     * @return 是否相等
     */
    bool equals(const ChatRoom* other) const;

public slots:
    /**
     * @brief 激活房间
     */
    void activate();

    /**
     * @brief 停用房间
     */
    void deactivate();

    /**
     * @brief 归档房间
     */
    void archive();

    /**
     * @brief 锁定房间
     */
    void lock();

    /**
     * @brief 解锁房间
     */
    void unlock();

    /**
     * @brief 清除所有参与者
     */
    void clearParticipants();

signals:
    /**
     * @brief 名称改变信号
     * @param name 新名称
     */
    void nameChanged(const QString& name);

    /**
     * @brief 描述改变信号
     * @param description 新描述
     */
    void descriptionChanged(const QString& description);

    /**
     * @brief 类型改变信号
     * @param type 新类型
     */
    void typeChanged(RoomType type);

    /**
     * @brief 状态改变信号
     * @param status 新状态
     */
    void statusChanged(RoomStatus status);

    /**
     * @brief 参与者数量改变信号
     * @param count 新数量
     */
    void participantCountChanged(int count);

    /**
     * @brief 最大参与者数量改变信号
     * @param maxParticipants 新的最大数量
     */
    void maxParticipantsChanged(int maxParticipants);

    /**
     * @brief 私有状态改变信号
     * @param isPrivate 是否为私有
     */
    void privateChanged(bool isPrivate);

    /**
     * @brief 密码状态改变信号
     * @param hasPassword 是否有密码
     */
    void passwordChanged(bool hasPassword);

    /**
     * @brief 最后活动时间改变信号
     * @param lastActivity 新的最后活动时间
     */
    void lastActivityChanged(const QDateTime& lastActivity);

    /**
     * @brief 参与者加入信号
     * @param participant 参与者对象
     */
    void participantJoined(Participant* participant);

    /**
     * @brief 参与者离开信号
     * @param participantId 参与者ID
     */
    void participantLeft(const QString& participantId);

    /**
     * @brief 管理员添加信号
     * @param participantId 参与者ID
     */
    void administratorAdded(const QString& participantId);

    /**
     * @brief 管理员移除信号
     * @param participantId 参与者ID
     */
    void administratorRemoved(const QString& participantId);

    /**
     * @brief 拥有者改变信号
     * @param ownerId 新拥有者ID
     */
    void ownerChanged(const QString& ownerId);

    /**
     * @brief 权限改变信号
     * @param participantId 参与者ID
     * @param permissions 新权限
     */
    void permissionsChanged(const QString& participantId, Permissions permissions);

    /**
     * @brief 设置改变信号
     * @param key 设置键
     * @param value 新值
     */
    void settingChanged(const QString& key, const QVariant& value);

    /**
     * @brief 属性改变信号
     * @param key 属性键
     * @param value 新值
     */
    void propertyChanged(const QString& key, const QVariant& value);

private:
    /**
     * @brief 验证房间ID
     * @param id 房间ID
     * @return 是否有效
     */
    bool validateId(const QString& id) const;

    /**
     * @brief 验证房间名称
     * @param name 房间名称
     * @return 是否有效
     */
    bool validateName(const QString& name) const;

private:
    class Private;
    Private* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ChatRoom::Permissions)
Q_DECLARE_METATYPE(ChatRoom*)
Q_DECLARE_METATYPE(ChatRoom::RoomType)
Q_DECLARE_METATYPE(ChatRoom::RoomStatus)
Q_DECLARE_METATYPE(ChatRoom::PermissionLevel)
Q_DECLARE_METATYPE(ChatRoom::Permissions)

#endif // CHATROOM_H