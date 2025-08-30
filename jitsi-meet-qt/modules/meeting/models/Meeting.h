#ifndef MEETING_H
#define MEETING_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QVariantList>
#include <QUuid>

/**
 * @brief 会议数据模型类
 * 
 * 表示一个会议的完整信息，包括基本信息、参与者、设置等
 */
class Meeting : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged)
    Q_PROPERTY(QString roomName READ roomName WRITE setRoomName NOTIFY roomNameChanged)
    Q_PROPERTY(MeetingType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(MeetingStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QDateTime createdAt READ createdAt WRITE setCreatedAt NOTIFY createdAtChanged)
    Q_PROPERTY(QDateTime startedAt READ startedAt WRITE setStartedAt NOTIFY startedAtChanged)
    Q_PROPERTY(QDateTime endedAt READ endedAt WRITE setEndedAt NOTIFY endedAtChanged)
    Q_PROPERTY(QString creatorId READ creatorId WRITE setCreatorId NOTIFY creatorIdChanged)
    Q_PROPERTY(int participantCount READ participantCount NOTIFY participantCountChanged)
    Q_PROPERTY(int maxParticipants READ maxParticipants WRITE setMaxParticipants NOTIFY maxParticipantsChanged)
    Q_PROPERTY(bool isLocked READ isLocked WRITE setLocked NOTIFY lockedChanged)
    Q_PROPERTY(bool isRecording READ isRecording WRITE setRecording NOTIFY recordingChanged)

public:
    /**
     * @brief 会议类型枚举
     */
    enum MeetingType {
        PublicMeeting,   ///< 公开会议
        PrivateMeeting,  ///< 私人会议
        ScheduledMeeting, ///< 预定会议
        InstantMeeting   ///< 即时会议
    };
    Q_ENUM(MeetingType)

    /**
     * @brief 会议状态枚举
     */
    enum MeetingStatus {
        Created,         ///< 已创建
        Scheduled,       ///< 已安排
        Active,          ///< 进行中
        Paused,          ///< 暂停
        Ended,           ///< 已结束
        Cancelled        ///< 已取消
    };
    Q_ENUM(MeetingStatus)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit Meeting(QObject* parent = nullptr);

    /**
     * @brief 构造函数（带参数）
     * @param name 会议名称
     * @param url 会议URL
     * @param parent 父对象
     */
    explicit Meeting(const QString& name, const QString& url, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~Meeting();

    // 基本属性
    /**
     * @brief 获取会议ID
     * @return 会议ID
     */
    QString id() const;

    /**
     * @brief 设置会议ID
     * @param id 会议ID
     */
    void setId(const QString& id);

    /**
     * @brief 获取会议名称
     * @return 会议名称
     */
    QString name() const;

    /**
     * @brief 设置会议名称
     * @param name 会议名称
     */
    void setName(const QString& name);

    /**
     * @brief 获取会议URL
     * @return 会议URL
     */
    QString url() const;

    /**
     * @brief 设置会议URL
     * @param url 会议URL
     */
    void setUrl(const QString& url);

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
     * @brief 获取房间名称
     * @return 房间名称
     */
    QString roomName() const;

    /**
     * @brief 设置房间名称
     * @param roomName 房间名称
     */
    void setRoomName(const QString& roomName);

    /**
     * @brief 获取会议类型
     * @return 会议类型
     */
    MeetingType type() const;

    /**
     * @brief 设置会议类型
     * @param type 会议类型
     */
    void setType(MeetingType type);

    /**
     * @brief 获取会议状态
     * @return 会议状态
     */
    MeetingStatus status() const;

    /**
     * @brief 设置会议状态
     * @param status 会议状态
     */
    void setStatus(MeetingStatus status);

    // 时间属性
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
     * @brief 获取开始时间
     * @return 开始时间
     */
    QDateTime startedAt() const;

    /**
     * @brief 设置开始时间
     * @param startedAt 开始时间
     */
    void setStartedAt(const QDateTime& startedAt);

    /**
     * @brief 获取结束时间
     * @return 结束时间
     */
    QDateTime endedAt() const;

    /**
     * @brief 设置结束时间
     * @param endedAt 结束时间
     */
    void setEndedAt(const QDateTime& endedAt);

    /**
     * @brief 获取会议持续时间
     * @return 持续时间（秒）
     */
    qint64 duration() const;

    // 参与者相关
    /**
     * @brief 获取创建者ID
     * @return 创建者ID
     */
    QString creatorId() const;

    /**
     * @brief 设置创建者ID
     * @param creatorId 创建者ID
     */
    void setCreatorId(const QString& creatorId);

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
     */
    void addParticipant(const QString& participantId);

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

    // 会议设置
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
     * @brief 获取是否录制
     * @return 是否录制
     */
    bool isRecording() const;

    /**
     * @brief 设置录制状态
     * @param recording 是否录制
     */
    void setRecording(bool recording);

    /**
     * @brief 获取会议密码
     * @return 会议密码
     */
    QString password() const;

    /**
     * @brief 设置会议密码
     * @param password 会议密码
     */
    void setPassword(const QString& password);

    /**
     * @brief 获取会议描述
     * @return 会议描述
     */
    QString description() const;

    /**
     * @brief 设置会议描述
     * @param description 会议描述
     */
    void setDescription(const QString& description);

    /**
     * @brief 获取会议标签
     * @return 标签列表
     */
    QStringList tags() const;

    /**
     * @brief 设置会议标签
     * @param tags 标签列表
     */
    void setTags(const QStringList& tags);

    /**
     * @brief 添加标签
     * @param tag 标签
     */
    void addTag(const QString& tag);

    /**
     * @brief 移除标签
     * @param tag 标签
     */
    void removeTag(const QString& tag);

    // 会议设置
    /**
     * @brief 获取会议设置
     * @return 设置映射
     */
    QVariantMap settings() const;

    /**
     * @brief 设置会议设置
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

    // 序列化
    /**
     * @brief 转换为映射
     * @return 会议信息映射
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 从映射加载
     * @param map 会议信息映射
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
     * @brief 验证会议数据
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
     * @brief 生成新的会议ID
     * @return 会议ID
     */
    static QString generateMeetingId();

    /**
     * @brief 生成会议URL
     * @param server 服务器地址
     * @param roomName 房间名称
     * @return 会议URL
     */
    static QString generateMeetingUrl(const QString& server, const QString& roomName);

signals:
    // 属性改变信号
    void idChanged(const QString& id);
    void nameChanged(const QString& name);
    void urlChanged(const QString& url);
    void serverChanged(const QString& server);
    void roomNameChanged(const QString& roomName);
    void typeChanged(MeetingType type);
    void statusChanged(MeetingStatus status);
    void createdAtChanged(const QDateTime& createdAt);
    void startedAtChanged(const QDateTime& startedAt);
    void endedAtChanged(const QDateTime& endedAt);
    void creatorIdChanged(const QString& creatorId);
    void participantCountChanged(int count);
    void maxParticipantsChanged(int maxParticipants);
    void lockedChanged(bool locked);
    void recordingChanged(bool recording);

    // 参与者相关信号
    void participantAdded(const QString& participantId);
    void participantRemoved(const QString& participantId);

    // 设置相关信号
    void settingsChanged(const QVariantMap& settings);
    void settingChanged(const QString& key, const QVariant& value);

    // 统计信息信号
    void statisticsUpdated(const QVariantMap& statistics);

private:
    /**
     * @brief 更新参与者数量
     */
    void updateParticipantCount();

    /**
     * @brief 验证会议名称
     * @param name 会议名称
     * @return 是否有效
     */
    bool validateName(const QString& name) const;

    /**
     * @brief 验证会议URL
     * @param url 会议URL
     * @return 是否有效
     */
    bool validateUrl(const QString& url) const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // MEETING_H