#ifndef CONFERENCEMANAGER_H
#define CONFERENCEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QDateTime>
#include <QMap>
#include "XMPPClient.h"
#include "WebRTCEngine.h"
#include "JitsiError.h"

class AuthenticationManager;

/**
 * @brief 会议管理器类，管理会议生命周期和参与者状态
 * 
 * ConferenceManager是Jitsi Meet Qt应用程序的核心组件，负责：
 * - 会议的加入、离开和重连逻辑
 * - 参与者列表管理和状态同步
 * - XMPP客户端和WebRTC引擎的集成
 * - 会议事件的统一处理和分发
 */
class ConferenceManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 连接状态枚举
     */
    enum ConnectionState {
        Disconnected,    ///< 未连接
        Connecting,      ///< 连接中
        Connected,       ///< 已连接
        Reconnecting,    ///< 重连中
        Failed           ///< 连接失败
    };
    Q_ENUM(ConnectionState)

    /**
     * @brief 会议状态枚举
     */
    enum ConferenceState {
        Idle,           ///< 空闲状态
        Joining,        ///< 加入会议中
        InConference,   ///< 在会议中
        Leaving,        ///< 离开会议中
        Error           ///< 错误状态
    };
    Q_ENUM(ConferenceState)

    /**
     * @brief 参与者信息结构（扩展自XMPPClient::Participant）
     */
    struct ParticipantInfo {
        QString jid;                    ///< 参与者JID
        QString displayName;            ///< 显示名称
        QString role;                   ///< 角色
        bool audioMuted;                ///< 音频静音状态
        bool videoMuted;                ///< 视频静音状态
        QString status;                 ///< 状态信息
        QDateTime joinTime;             ///< 加入时间
        bool hasVideo;                  ///< 是否有视频流
        bool hasAudio;                  ///< 是否有音频流
        bool isScreenSharing;           ///< 是否在共享屏幕
        QString connectionQuality;      ///< 连接质量
        
        // 从XMPPClient::Participant转换
        static ParticipantInfo fromXMPPParticipant(const XMPPClient::Participant& xmppParticipant);
    };

    /**
     * @brief 会议信息结构
     */
    struct ConferenceInfo {
        QString roomName;               ///< 房间名称
        QString serverUrl;              ///< 服务器URL
        QString fullUrl;                ///< 完整URL
        QString displayName;            ///< 用户显示名称
        QDateTime joinTime;             ///< 加入时间
        int participantCount;           ///< 参与者数量
        bool isLocked;                  ///< 房间是否锁定
        bool isRecording;               ///< 是否在录制
        QString meetingId;              ///< 会议ID
    };

    explicit ConferenceManager(QObject *parent = nullptr);
    ~ConferenceManager();

    /**
     * @brief 加入会议
     * @param url 会议URL（支持多种格式）
     * @param displayName 用户显示名称
     */
    void joinConference(const QString& url, const QString& displayName = QString());

    /**
     * @brief 离开当前会议
     */
    void leaveConference();

    /**
     * @brief 重新连接到当前会议
     */
    void reconnectToConference();

    /**
     * @brief 发送聊天消息
     * @param message 消息内容
     */
    void sendChatMessage(const QString& message);

    /**
     * @brief 设置音频静音状态
     * @param muted 是否静音
     */
    void setAudioMuted(bool muted);

    /**
     * @brief 设置视频静音状态
     * @param muted 是否静音
     */
    void setVideoMuted(bool muted);

    /**
     * @brief 开始屏幕共享
     */
    void startScreenShare();

    /**
     * @brief 停止屏幕共享
     */
    void stopScreenShare();

    // Getters
    ConnectionState connectionState() const { return m_connectionState; }
    ConferenceState conferenceState() const { return m_conferenceState; }
    ConferenceInfo currentConference() const { return m_currentConference; }
    QList<ParticipantInfo> participants() const { return m_participants.values(); }
    ParticipantInfo localParticipant() const { return m_localParticipant; }
    bool isInConference() const { return m_conferenceState == InConference; }
    bool isConnected() const { return m_connectionState == Connected; }
    QString lastError() const { return m_lastError; }
    int participantCount() const { return m_participants.size(); }
    
    /**
     * @brief 获取XMPP客户端实例
     * @return XMPP客户端指针
     */
    XMPPClient* xmppClient() const { return m_xmppClient; }
    
    /**
     * @brief 获取WebRTC引擎实例
     * @return WebRTC引擎指针
     */
    WebRTCEngine* webRTCEngine() const { return m_webrtcEngine; }

signals:
    /**
     * @brief 连接状态改变信号
     * @param state 新的连接状态
     */
    void connectionStateChanged(ConnectionState state);

    /**
     * @brief 会议状态改变信号
     * @param state 新的会议状态
     */
    void conferenceStateChanged(ConferenceState state);

    /**
     * @brief 成功加入会议信号
     * @param conferenceInfo 会议信息
     */
    void conferenceJoined(const ConferenceInfo& conferenceInfo);

    /**
     * @brief 离开会议信号
     */
    void conferenceLeft();

    /**
     * @brief 参与者加入信号
     * @param participant 参与者信息
     */
    void participantJoined(const ParticipantInfo& participant);

    /**
     * @brief 参与者离开信号
     * @param jid 参与者JID
     */
    void participantLeft(const QString& jid);

    /**
     * @brief 参与者状态更新信号
     * @param participant 更新后的参与者信息
     */
    void participantUpdated(const ParticipantInfo& participant);

    /**
     * @brief 聊天消息接收信号
     * @param from 发送者JID
     * @param message 消息内容
     * @param timestamp 时间戳
     */
    void chatMessageReceived(const QString& from, const QString& message, const QDateTime& timestamp);

    /**
     * @brief 本地媒体状态改变信号
     * @param audioMuted 音频静音状态
     * @param videoMuted 视频静音状态
     */
    void localMediaStateChanged(bool audioMuted, bool videoMuted);

    /**
     * @brief 屏幕共享状态改变信号
     * @param isSharing 是否在共享
     * @param participantJid 共享者JID
     */
    void screenShareStateChanged(bool isSharing, const QString& participantJid);

    /**
     * @brief 会议信息更新信号
     * @param conferenceInfo 更新后的会议信息
     */
    void conferenceInfoUpdated(const ConferenceInfo& conferenceInfo);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const JitsiError& error);

    /**
     * @brief 重连开始信号
     * @param attempt 重连尝试次数
     */
    void reconnectionStarted(int attempt);

    /**
     * @brief 重连成功信号
     */
    void reconnectionSucceeded();

    /**
     * @brief 重连失败信号
     * @param error 失败原因
     */
    void reconnectionFailed(const QString& error);

private slots:
    // XMPP客户端事件处理
    void onXMPPConnectionStateChanged(XMPPClient::ConnectionState state);
    void onXMPPConnected();
    void onXMPPDisconnected();
    void onXMPPAuthenticated();
    void onXMPPRoomJoined();
    void onXMPPRoomLeft();
    void onXMPPParticipantJoined(const XMPPClient::Participant& participant);
    void onXMPPParticipantLeft(const QString& jid);
    void onXMPPParticipantUpdated(const XMPPClient::Participant& participant);
    void onXMPPChatMessageReceived(const QString& from, const QString& message, const QDateTime& timestamp);
    void onXMPPErrorOccurred(const QString& error);

    // WebRTC引擎事件处理
    void onWebRTCConnectionStateChanged(WebRTCEngine::ConnectionState state);
    void onWebRTCLocalStreamReady(QVideoWidget* videoWidget);
    void onWebRTCRemoteStreamReceived(const QString& participantId, QVideoWidget* videoWidget);
    void onWebRTCRemoteStreamRemoved(const QString& participantId);
    void onWebRTCIceCandidate(const WebRTCEngine::IceCandidate& candidate);
    void onWebRTCOfferCreated(const QString& sdp);
    void onWebRTCAnswerCreated(const QString& sdp);
    void onWebRTCError(const QString& message);

    // 重连机制
    void onReconnectTimer();
    void onConnectionHealthCheck();

private:
    /**
     * @brief 设置连接状态
     * @param state 新状态
     */
    void setConnectionState(ConnectionState state);

    /**
     * @brief 设置会议状态
     * @param state 新状态
     */
    void setConferenceState(ConferenceState state);

    /**
     * @brief 解析会议URL
     * @param url 输入URL
     * @param serverUrl 输出服务器URL
     * @param roomName 输出房间名称
     * @return 解析是否成功
     */
    bool parseConferenceUrl(const QString& url, QString& serverUrl, QString& roomName);

    /**
     * @brief 初始化组件
     */
    void initializeComponents();

    /**
     * @brief 清理资源
     */
    void cleanup();

    /**
     * @brief 建立XMPP连接
     */
    void establishXMPPConnection();

    /**
     * @brief 建立WebRTC连接
     */
    void establishWebRTCConnection();

    /**
     * @brief 同步参与者状态
     */
    void synchronizeParticipants();

    /**
     * @brief 更新会议信息
     */
    void updateConferenceInfo();

    /**
     * @brief 处理认证
     */
    void handleAuthentication();

    /**
     * @brief 启动重连机制
     */
    void startReconnection();

    /**
     * @brief 停止重连机制
     */
    void stopReconnection();

    /**
     * @brief 检查连接健康状态
     */
    void checkConnectionHealth();

    /**
     * @brief 发出错误信号
     * @param type 错误类型
     * @param message 错误消息
     * @param details 错误详情
     */
    void emitError(::ErrorType type, const QString& message, const QString& details = QString());

    /**
     * @brief 更新本地参与者信息
     */
    void updateLocalParticipant();

    /**
     * @brief 处理媒体流事件
     * @param participantJid 参与者JID
     * @param hasVideo 是否有视频
     * @param hasAudio 是否有音频
     */
    void handleMediaStreamEvent(const QString& participantJid, bool hasVideo, bool hasAudio);

    // 核心组件
    XMPPClient* m_xmppClient;                   ///< XMPP客户端
    WebRTCEngine* m_webrtcEngine;               ///< WebRTC引擎
    AuthenticationManager* m_authManager;       ///< 认证管理器

    // 状态管理
    ConnectionState m_connectionState;          ///< 连接状态
    ConferenceState m_conferenceState;          ///< 会议状态
    ConferenceInfo m_currentConference;         ///< 当前会议信息
    ParticipantInfo m_localParticipant;         ///< 本地参与者信息

    // 参与者管理
    QMap<QString, ParticipantInfo> m_participants; ///< 参与者列表（JID -> 参与者信息）

    // 重连机制
    QTimer* m_reconnectTimer;                   ///< 重连定时器
    QTimer* m_healthCheckTimer;                 ///< 健康检查定时器
    int m_reconnectAttempts;                    ///< 重连尝试次数
    QString m_lastError;                        ///< 最后一次错误信息

    // 配置常量
    static const int MAX_RECONNECT_ATTEMPTS = 5;     ///< 最大重连次数
    static const int RECONNECT_INTERVAL = 3000;      ///< 重连间隔（毫秒）
    static const int HEALTH_CHECK_INTERVAL = 10000;  ///< 健康检查间隔（毫秒）

    // 媒体状态
    bool m_localAudioMuted;                     ///< 本地音频静音状态
    bool m_localVideoMuted;                     ///< 本地视频静音状态
    bool m_isScreenSharing;                     ///< 是否在共享屏幕
    QString m_screenSharingParticipant;         ///< 当前屏幕共享者JID
};

#endif // CONFERENCEMANAGER_H