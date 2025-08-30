#ifndef IMEETINGMANAGER_H
#define IMEETINGMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QUrl>

/**
 * @brief 会议管理器接口
 * 
 * 定义会议管理的核心功能，包括会议创建、加入、离开和状态管理
 */
class IMeetingManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 会议状态枚举
     */
    enum MeetingState {
        Disconnected,    ///< 未连接
        Connecting,      ///< 连接中
        Connected,       ///< 已连接
        InMeeting,       ///< 会议中
        Leaving,         ///< 离开中
        Error            ///< 错误状态
    };
    Q_ENUM(MeetingState)

    /**
     * @brief 会议类型枚举
     */
    enum MeetingType {
        PublicMeeting,   ///< 公开会议
        PrivateMeeting,  ///< 私人会议
        ScheduledMeeting ///< 预定会议
    };
    Q_ENUM(MeetingType)

    /**
     * @brief 参与者角色枚举
     */
    enum ParticipantRole {
        Guest,           ///< 访客
        Participant,     ///< 参与者
        Moderator,       ///< 主持人
        Owner            ///< 所有者
    };
    Q_ENUM(ParticipantRole)

    explicit IMeetingManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IMeetingManager() = default;

    /**
     * @brief 初始化会议管理器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 获取当前会议状态
     * @return 会议状态
     */
    virtual MeetingState currentState() const = 0;

    /**
     * @brief 创建新会议
     * @param meetingName 会议名称
     * @param settings 会议设置
     * @return 创建是否成功
     */
    virtual bool createMeeting(const QString& meetingName, const QVariantMap& settings = QVariantMap()) = 0;

    /**
     * @brief 加入会议
     * @param meetingUrl 会议链接
     * @param displayName 显示名称
     * @param audioEnabled 是否启用音频
     * @param videoEnabled 是否启用视频
     * @return 加入是否成功
     */
    virtual bool joinMeeting(const QString& meetingUrl, 
                           const QString& displayName = QString(),
                           bool audioEnabled = true, 
                           bool videoEnabled = true) = 0;

    /**
     * @brief 离开当前会议
     * @return 离开是否成功
     */
    virtual bool leaveMeeting() = 0;

    /**
     * @brief 验证会议URL
     * @param meetingUrl 会议链接
     * @return URL是否有效
     */
    virtual bool validateMeetingUrl(const QString& meetingUrl) = 0;

    /**
     * @brief 获取当前会议信息
     * @return 会议信息映射
     */
    virtual QVariantMap getCurrentMeetingInfo() const = 0;

    /**
     * @brief 设置会议配置
     * @param config 配置映射
     */
    virtual void setConfiguration(const QVariantMap& config) = 0;

    /**
     * @brief 获取会议配置
     * @return 配置映射
     */
    virtual QVariantMap getConfiguration() const = 0;

    /**
     * @brief 获取参与者列表
     * @return 参与者信息列表
     */
    virtual QVariantList getParticipants() const = 0;

    /**
     * @brief 邀请参与者
     * @param email 邮箱地址
     * @param message 邀请消息
     * @return 邀请是否成功
     */
    virtual bool inviteParticipant(const QString& email, const QString& message = QString()) = 0;

signals:
    /**
     * @brief 会议状态改变信号
     * @param state 新状态
     */
    void stateChanged(MeetingState state);

    /**
     * @brief 会议创建成功信号
     * @param meetingUrl 会议链接
     * @param meetingInfo 会议信息
     */
    void meetingCreated(const QString& meetingUrl, const QVariantMap& meetingInfo);

    /**
     * @brief 会议加入成功信号
     * @param meetingInfo 会议信息
     */
    void meetingJoined(const QVariantMap& meetingInfo);

    /**
     * @brief 会议离开信号
     */
    void meetingLeft();

    /**
     * @brief 参与者加入信号
     * @param participantInfo 参与者信息
     */
    void participantJoined(const QVariantMap& participantInfo);

    /**
     * @brief 参与者离开信号
     * @param participantId 参与者ID
     */
    void participantLeft(const QString& participantId);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 连接质量改变信号
     * @param quality 连接质量 (0-100)
     */
    void connectionQualityChanged(int quality);
};

#endif // IMEETINGMANAGER_H