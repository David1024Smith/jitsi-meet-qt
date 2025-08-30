#ifndef MEETINGMANAGER_H
#define MEETINGMANAGER_H

#include "IMeetingManager.h"
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QTimer>
#include <memory>

class MeetingConfig;
class LinkHandler;

/**
 * @brief 会议管理器实现类
 * 
 * 实现IMeetingManager接口，提供完整的会议管理功能
 */
class MeetingManager : public IMeetingManager
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MeetingManager(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MeetingManager();

    // IMeetingManager接口实现
    bool initialize() override;
    MeetingState currentState() const override;
    bool createMeeting(const QString& meetingName, const QVariantMap& settings = QVariantMap()) override;
    bool joinMeeting(const QString& meetingUrl, 
                    const QString& displayName = QString(),
                    bool audioEnabled = true, 
                    bool videoEnabled = true) override;
    bool leaveMeeting() override;
    bool validateMeetingUrl(const QString& meetingUrl) override;
    QVariantMap getCurrentMeetingInfo() const override;
    void setConfiguration(const QVariantMap& config) override;
    QVariantMap getConfiguration() const override;
    QVariantList getParticipants() const override;
    bool inviteParticipant(const QString& email, const QString& message = QString()) override;

    /**
     * @brief 设置链接处理器
     * @param linkHandler 链接处理器指针
     */
    void setLinkHandler(LinkHandler* linkHandler);

    /**
     * @brief 获取链接处理器
     * @return 链接处理器指针
     */
    LinkHandler* linkHandler() const;

    /**
     * @brief 设置会议配置对象
     * @param config 配置对象指针
     */
    void setMeetingConfig(MeetingConfig* config);

    /**
     * @brief 获取会议配置对象
     * @return 配置对象指针
     */
    MeetingConfig* meetingConfig() const;

    /**
     * @brief 获取当前会议ID
     * @return 会议ID
     */
    QString getCurrentMeetingId() const;

    /**
     * @brief 获取当前会议URL
     * @return 会议URL
     */
    QString getCurrentMeetingUrl() const;

    /**
     * @brief 设置用户显示名称
     * @param displayName 显示名称
     */
    void setDisplayName(const QString& displayName);

    /**
     * @brief 获取用户显示名称
     * @return 显示名称
     */
    QString displayName() const;

    /**
     * @brief 设置音频状态
     * @param enabled 是否启用
     */
    void setAudioEnabled(bool enabled);

    /**
     * @brief 获取音频状态
     * @return 是否启用
     */
    bool isAudioEnabled() const;

    /**
     * @brief 设置视频状态
     * @param enabled 是否启用
     */
    void setVideoEnabled(bool enabled);

    /**
     * @brief 获取视频状态
     * @return 是否启用
     */
    bool isVideoEnabled() const;

    /**
     * @brief 获取连接质量
     * @return 连接质量 (0-100)
     */
    int getConnectionQuality() const;

    /**
     * @brief 获取会议统计信息
     * @return 统计信息映射
     */
    QVariantMap getMeetingStatistics() const;

    /**
     * @brief 重新连接会议
     * @return 重连是否成功
     */
    bool reconnect();

    /**
     * @brief 检查会议状态
     */
    void checkMeetingStatus();

public slots:
    /**
     * @brief 刷新参与者列表
     */
    void refreshParticipants();

    /**
     * @brief 更新会议设置
     * @param settings 新设置
     */
    void updateMeetingSettings(const QVariantMap& settings);

private slots:
    /**
     * @brief 处理连接超时
     */
    void handleConnectionTimeout();

    /**
     * @brief 处理状态检查定时器
     */
    void handleStatusCheck();

    /**
     * @brief 处理链接验证结果
     * @param url 链接
     * @param result 验证结果
     */
    void handleLinkValidation(const QString& url, int result);

private:
    /**
     * @brief 设置会议状态
     * @param state 新状态
     */
    void setState(MeetingState state);

    /**
     * @brief 初始化连接
     * @return 初始化是否成功
     */
    bool initializeConnection();

    /**
     * @brief 清理连接
     */
    void cleanupConnection();

    /**
     * @brief 处理会议加入逻辑
     * @param meetingUrl 会议URL
     * @param displayName 显示名称
     * @param audioEnabled 音频状态
     * @param videoEnabled 视频状态
     * @return 加入是否成功
     */
    bool performJoinMeeting(const QString& meetingUrl, 
                          const QString& displayName,
                          bool audioEnabled, 
                          bool videoEnabled);

    /**
     * @brief 处理会议创建逻辑
     * @param meetingName 会议名称
     * @param settings 会议设置
     * @return 创建是否成功
     */
    bool performCreateMeeting(const QString& meetingName, const QVariantMap& settings);

    /**
     * @brief 更新连接质量
     */
    void updateConnectionQuality();

    /**
     * @brief 发送心跳包
     */
    void sendHeartbeat();

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // MEETINGMANAGER_H