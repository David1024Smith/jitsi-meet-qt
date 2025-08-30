#ifndef INVITATION_H
#define INVITATION_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QStringList>

/**
 * @brief 邀请数据模型类
 * 
 * 表示一个会议邀请的信息，包括邀请详情、接收者信息和状态跟踪
 */
class Invitation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString meetingId READ meetingId WRITE setMeetingId NOTIFY meetingIdChanged)
    Q_PROPERTY(QString senderId READ senderId WRITE setSenderId NOTIFY senderIdChanged)
    Q_PROPERTY(QString recipientId READ recipientId WRITE setRecipientId NOTIFY recipientIdChanged)
    Q_PROPERTY(QString recipientEmail READ recipientEmail WRITE setRecipientEmail NOTIFY recipientEmailChanged)
    Q_PROPERTY(InvitationType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(InvitationStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QDateTime createdAt READ createdAt WRITE setCreatedAt NOTIFY createdAtChanged)
    Q_PROPERTY(QDateTime sentAt READ sentAt WRITE setSentAt NOTIFY sentAtChanged)
    Q_PROPERTY(QDateTime respondedAt READ respondedAt WRITE setRespondedAt NOTIFY respondedAtChanged)
    Q_PROPERTY(QDateTime expiresAt READ expiresAt WRITE setExpiresAt NOTIFY expiresAtChanged)
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)

public:
    /**
     * @brief 邀请类型枚举
     */
    enum InvitationType {
        EmailInvitation,     ///< 邮件邀请
        LinkInvitation,      ///< 链接邀请
        DirectInvitation,    ///< 直接邀请
        ScheduledInvitation  ///< 预定邀请
    };
    Q_ENUM(InvitationType)

    /**
     * @brief 邀请状态枚举
     */
    enum InvitationStatus {
        Pending,         ///< 待处理
        Sent,            ///< 已发送
        Delivered,       ///< 已送达
        Opened,          ///< 已打开
        Accepted,        ///< 已接受
        Declined,        ///< 已拒绝
        Expired,         ///< 已过期
        Cancelled        ///< 已取消
    };
    Q_ENUM(InvitationStatus)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit Invitation(QObject* parent = nullptr);

    /**
     * @brief 构造函数（带参数）
     * @param meetingId 会议ID
     * @param recipientEmail 接收者邮箱
     * @param parent 父对象
     */
    explicit Invitation(const QString& meetingId, const QString& recipientEmail, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~Invitation();

    // 基本属性
    /**
     * @brief 获取邀请ID
     * @return 邀请ID
     */
    QString id() const;

    /**
     * @brief 设置邀请ID
     * @param id 邀请ID
     */
    void setId(const QString& id);

    /**
     * @brief 获取会议ID
     * @return 会议ID
     */
    QString meetingId() const;

    /**
     * @brief 设置会议ID
     * @param meetingId 会议ID
     */
    void setMeetingId(const QString& meetingId);

    /**
     * @brief 获取发送者ID
     * @return 发送者ID
     */
    QString senderId() const;

    /**
     * @brief 设置发送者ID
     * @param senderId 发送者ID
     */
    void setSenderId(const QString& senderId);

    /**
     * @brief 获取接收者ID
     * @return 接收者ID
     */
    QString recipientId() const;

    /**
     * @brief 设置接收者ID
     * @param recipientId 接收者ID
     */
    void setRecipientId(const QString& recipientId);

    /**
     * @brief 获取接收者邮箱
     * @return 接收者邮箱
     */
    QString recipientEmail() const;

    /**
     * @brief 设置接收者邮箱
     * @param recipientEmail 接收者邮箱
     */
    void setRecipientEmail(const QString& recipientEmail);

    /**
     * @brief 获取邀请类型
     * @return 邀请类型
     */
    InvitationType type() const;

    /**
     * @brief 设置邀请类型
     * @param type 邀请类型
     */
    void setType(InvitationType type);

    /**
     * @brief 获取邀请状态
     * @return 邀请状态
     */
    InvitationStatus status() const;

    /**
     * @brief 设置邀请状态
     * @param status 邀请状态
     */
    void setStatus(InvitationStatus status);

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
     * @brief 获取发送时间
     * @return 发送时间
     */
    QDateTime sentAt() const;

    /**
     * @brief 设置发送时间
     * @param sentAt 发送时间
     */
    void setSentAt(const QDateTime& sentAt);

    /**
     * @brief 获取响应时间
     * @return 响应时间
     */
    QDateTime respondedAt() const;

    /**
     * @brief 设置响应时间
     * @param respondedAt 响应时间
     */
    void setRespondedAt(const QDateTime& respondedAt);

    /**
     * @brief 获取过期时间
     * @return 过期时间
     */
    QDateTime expiresAt() const;

    /**
     * @brief 设置过期时间
     * @param expiresAt 过期时间
     */
    void setExpiresAt(const QDateTime& expiresAt);

    /**
     * @brief 检查是否已过期
     * @return 是否已过期
     */
    bool isExpired() const;

    /**
     * @brief 获取剩余有效时间
     * @return 剩余时间（秒）
     */
    qint64 timeRemaining() const;

    // 邀请内容
    /**
     * @brief 获取邀请消息
     * @return 邀请消息
     */
    QString message() const;

    /**
     * @brief 设置邀请消息
     * @param message 邀请消息
     */
    void setMessage(const QString& message);

    /**
     * @brief 获取邀请主题
     * @return 邀请主题
     */
    QString subject() const;

    /**
     * @brief 设置邀请主题
     * @param subject 邀请主题
     */
    void setSubject(const QString& subject);

    /**
     * @brief 获取会议链接
     * @return 会议链接
     */
    QString meetingUrl() const;

    /**
     * @brief 设置会议链接
     * @param meetingUrl 会议链接
     */
    void setMeetingUrl(const QString& meetingUrl);

    /**
     * @brief 获取邀请链接
     * @return 邀请链接
     */
    QString invitationUrl() const;

    /**
     * @brief 设置邀请链接
     * @param invitationUrl 邀请链接
     */
    void setInvitationUrl(const QString& invitationUrl);

    // 发送者信息
    /**
     * @brief 获取发送者姓名
     * @return 发送者姓名
     */
    QString senderName() const;

    /**
     * @brief 设置发送者姓名
     * @param senderName 发送者姓名
     */
    void setSenderName(const QString& senderName);

    /**
     * @brief 获取发送者邮箱
     * @return 发送者邮箱
     */
    QString senderEmail() const;

    /**
     * @brief 设置发送者邮箱
     * @param senderEmail 发送者邮箱
     */
    void setSenderEmail(const QString& senderEmail);

    // 接收者信息
    /**
     * @brief 获取接收者姓名
     * @return 接收者姓名
     */
    QString recipientName() const;

    /**
     * @brief 设置接收者姓名
     * @param recipientName 接收者姓名
     */
    void setRecipientName(const QString& recipientName);

    // 邀请设置
    /**
     * @brief 获取邀请设置
     * @return 设置映射
     */
    QVariantMap settings() const;

    /**
     * @brief 设置邀请设置
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

    // 提醒设置
    /**
     * @brief 获取提醒时间列表
     * @return 提醒时间列表（分钟）
     */
    QList<int> reminderTimes() const;

    /**
     * @brief 设置提醒时间列表
     * @param reminderTimes 提醒时间列表（分钟）
     */
    void setReminderTimes(const QList<int>& reminderTimes);

    /**
     * @brief 添加提醒时间
     * @param minutes 提醒时间（分钟）
     */
    void addReminderTime(int minutes);

    /**
     * @brief 移除提醒时间
     * @param minutes 提醒时间（分钟）
     */
    void removeReminderTime(int minutes);

    /**
     * @brief 获取是否启用提醒
     * @return 是否启用提醒
     */
    bool isReminderEnabled() const;

    /**
     * @brief 设置是否启用提醒
     * @param enabled 是否启用提醒
     */
    void setReminderEnabled(bool enabled);

    // 跟踪信息
    /**
     * @brief 获取跟踪信息
     * @return 跟踪信息映射
     */
    QVariantMap trackingInfo() const;

    /**
     * @brief 更新跟踪信息
     * @param info 跟踪信息
     */
    void updateTrackingInfo(const QVariantMap& info);

    /**
     * @brief 记录事件
     * @param event 事件名称
     * @param data 事件数据
     */
    void recordEvent(const QString& event, const QVariantMap& data = QVariantMap());

    /**
     * @brief 获取事件历史
     * @return 事件历史列表
     */
    QVariantList eventHistory() const;

    // 操作方法
    /**
     * @brief 发送邀请
     * @return 发送是否成功
     */
    bool send();

    /**
     * @brief 重新发送邀请
     * @return 重新发送是否成功
     */
    bool resend();

    /**
     * @brief 取消邀请
     * @return 取消是否成功
     */
    bool cancel();

    /**
     * @brief 接受邀请
     * @param response 响应信息
     * @return 接受是否成功
     */
    bool accept(const QString& response = QString());

    /**
     * @brief 拒绝邀请
     * @param reason 拒绝原因
     * @return 拒绝是否成功
     */
    bool decline(const QString& reason = QString());

    // 序列化
    /**
     * @brief 转换为映射
     * @return 邀请信息映射
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 从映射加载
     * @param map 邀请信息映射
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
     * @brief 验证邀请数据
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
     * @brief 生成邀请ID
     * @return 邀请ID
     */
    static QString generateInvitationId();

    /**
     * @brief 生成邀请链接
     * @param meetingUrl 会议链接
     * @param invitationId 邀请ID
     * @return 邀请链接
     */
    static QString generateInvitationUrl(const QString& meetingUrl, const QString& invitationId);

    /**
     * @brief 验证邮箱地址
     * @param email 邮箱地址
     * @return 是否有效
     */
    static bool validateEmail(const QString& email);

    /**
     * @brief 格式化邀请消息
     * @param template 消息模板
     * @param variables 变量映射
     * @return 格式化后的消息
     */
    static QString formatMessage(const QString& templateStr, const QVariantMap& variables);

signals:
    // 属性改变信号
    void idChanged(const QString& id);
    void meetingIdChanged(const QString& meetingId);
    void senderIdChanged(const QString& senderId);
    void recipientIdChanged(const QString& recipientId);
    void recipientEmailChanged(const QString& recipientEmail);
    void typeChanged(InvitationType type);
    void statusChanged(InvitationStatus status);
    void createdAtChanged(const QDateTime& createdAt);
    void sentAtChanged(const QDateTime& sentAt);
    void respondedAtChanged(const QDateTime& respondedAt);
    void expiresAtChanged(const QDateTime& expiresAt);
    void messageChanged(const QString& message);

    // 状态相关信号
    void invitationSent();
    void invitationDelivered();
    void invitationOpened();
    void invitationAccepted(const QString& response);
    void invitationDeclined(const QString& reason);
    void invitationExpired();
    void invitationCancelled();

    // 设置相关信号
    void settingsChanged(const QVariantMap& settings);
    void settingChanged(const QString& key, const QVariant& value);

    // 跟踪相关信号
    void trackingInfoUpdated(const QVariantMap& info);
    void eventRecorded(const QString& event, const QVariantMap& data);

private:
    /**
     * @brief 初始化默认设置
     */
    void initializeDefaultSettings();

    /**
     * @brief 验证邮箱格式
     * @param email 邮箱地址
     * @return 是否有效
     */
    bool validateEmailFormat(const QString& email) const;

    /**
     * @brief 生成默认主题
     * @return 默认主题
     */
    QString generateDefaultSubject() const;

    /**
     * @brief 生成默认消息
     * @return 默认消息
     */
    QString generateDefaultMessage() const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // INVITATION_H