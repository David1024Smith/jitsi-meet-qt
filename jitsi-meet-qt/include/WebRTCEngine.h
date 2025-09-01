#ifndef WEBRTCENGINE_H
#define WEBRTCENGINE_H

#include <QObject>
#include <QMap>
#include <QVariantMap>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QMutex>
#include <QAbstractSocket>
#include <memory>

class QNetworkAccessManager;
class QWebSocket;

/**
 * @brief WebRTC引擎
 * 
 * 负责管理WebRTC连接、媒体流和信令通信。
 */
class WebRTCEngine : public QObject
{
    Q_OBJECT

public:
    enum ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Reconnecting,
        Failed
    };
    Q_ENUM(ConnectionState)

    enum MediaType {
        Audio,
        Video,
        Screen,
        Data
    };
    Q_ENUM(MediaType)

    enum VideoQuality {
        Low,
        Medium,
        High,
        HD,
        FullHD
    };
    Q_ENUM(VideoQuality)

    struct MediaStreamInfo {
        QString streamId;
        QString participantId;
        MediaType type;
        bool enabled;
        QSize resolution;
        int bitrate;
        int framerate;
    };

    explicit WebRTCEngine(QObject *parent = nullptr);
    ~WebRTCEngine();

    /**
     * @brief 获取单例实例
     */
    static WebRTCEngine* instance();

    /**
     * @brief 初始化WebRTC引擎
     * @return 是否成功初始化
     */
    bool initialize();

    /**
     * @brief 关闭WebRTC引擎
     */
    void shutdown();

    /**
     * @brief 连接到会议
     * @param roomId 房间ID
     * @param displayName 显示名称
     * @param config 配置参数
     * @return 是否成功连接
     */
    bool connect(const QString& roomId, const QString& displayName, const QVariantMap& config = QVariantMap());

    /**
     * @brief 断开连接
     */
    void disconnect();

    /**
     * @brief 获取连接状态
     * @return 连接状态
     */
    ConnectionState connectionState() const;

    /**
     * @brief 获取房间ID
     * @return 房间ID
     */
    QString roomId() const;

    /**
     * @brief 获取显示名称
     * @return 显示名称
     */
    QString displayName() const;

    /**
     * @brief 设置显示名称
     * @param name 显示名称
     */
    void setDisplayName(const QString& name);

    /**
     * @brief 启用/禁用媒体
     * @param type 媒体类型
     * @param enabled 是否启用
     * @return 是否成功设置
     */
    bool setMediaEnabled(MediaType type, bool enabled);

    /**
     * @brief 检查媒体是否启用
     * @param type 媒体类型
     * @return 是否启用
     */
    bool isMediaEnabled(MediaType type) const;

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

    /**
     * @brief 获取参与者列表
     * @return 参与者ID列表
     */
    QStringList getParticipants() const;

    /**
     * @brief 获取参与者信息
     * @param participantId 参与者ID
     * @return 参与者信息
     */
    QVariantMap getParticipantInfo(const QString& participantId) const;

    /**
     * @brief 获取媒体流信息
     * @param streamId 流ID
     * @return 媒体流信息
     */
    MediaStreamInfo getStreamInfo(const QString& streamId) const;

    /**
     * @brief 获取参与者的媒体流
     * @param participantId 参与者ID
     * @param type 媒体类型
     * @return 流ID
     */
    QString getParticipantStream(const QString& participantId, MediaType type) const;

    /**
     * @brief 发送消息
     * @param message 消息内容
     * @param to 接收者ID，为空则发送给所有人
     * @return 是否成功发送
     */
    bool sendMessage(const QString& message, const QString& to = QString());

    /**
     * @brief 发送命令
     * @param command 命令名称
     * @param data 命令数据
     * @param to 接收者ID，为空则发送给所有人
     * @return 是否成功发送
     */
    bool sendCommand(const QString& command, const QVariantMap& data, const QString& to = QString());

    /**
     * @brief 设置ICE服务器
     * @param servers ICE服务器列表
     */
    void setIceServers(const QVariantList& servers);

    /**
     * @brief 获取ICE服务器
     * @return ICE服务器列表
     */
    QVariantList iceServers() const;

    /**
     * @brief 设置信令服务器
     * @param url 服务器URL
     */
    void setSignalingServer(const QString& url);

    /**
     * @brief 获取信令服务器
     * @return 服务器URL
     */
    QString signalingServer() const;

    /**
     * @brief 设置配置
     * @param config 配置参数
     */
    void setConfig(const QVariantMap& config);

    /**
     * @brief 获取配置
     * @return 配置参数
     */
    QVariantMap config() const;

    /**
     * @brief 获取统计信息
     * @return 统计信息
     */
    QVariantMap getStats() const;

    /**
     * @brief 重新协商连接
     * @return 是否成功开始重新协商
     */
    bool renegotiate();

    /**
     * @brief 重新连接
     * @return 是否成功开始重新连接
     */
    bool reconnect();

signals:
    /**
     * @brief 连接状态变化信号
     * @param state 连接状态
     */
    void connectionStateChanged(ConnectionState state);

    /**
     * @brief 参与者加入信号
     * @param participantId 参与者ID
     * @param info 参与者信息
     */
    void participantJoined(const QString& participantId, const QVariantMap& info);

    /**
     * @brief 参与者离开信号
     * @param participantId 参与者ID
     */
    void participantLeft(const QString& participantId);

    /**
     * @brief 参与者更新信号
     * @param participantId 参与者ID
     * @param info 更新的信息
     */
    void participantUpdated(const QString& participantId, const QVariantMap& info);

    /**
     * @brief 媒体流添加信号
     * @param streamId 流ID
     * @param info 媒体流信息
     */
    void streamAdded(const QString& streamId, const MediaStreamInfo& info);

    /**
     * @brief 媒体流移除信号
     * @param streamId 流ID
     */
    void streamRemoved(const QString& streamId);

    /**
     * @brief 媒体流更新信号
     * @param streamId 流ID
     * @param info 更新的信息
     */
    void streamUpdated(const QString& streamId, const MediaStreamInfo& info);

    /**
     * @brief 消息接收信号
     * @param message 消息内容
     * @param from 发送者ID
     */
    void messageReceived(const QString& message, const QString& from);

    /**
     * @brief 命令接收信号
     * @param command 命令名称
     * @param data 命令数据
     * @param from 发送者ID
     */
    void commandReceived(const QString& command, const QVariantMap& data, const QString& from);

    /**
     * @brief 错误发生信号
     * @param errorCode 错误代码
     * @param errorMessage 错误消息
     */
    void errorOccurred(int errorCode, const QString& errorMessage);

    /**
     * @brief 统计信息更新信号
     * @param stats 统计信息
     */
    void statsUpdated(const QVariantMap& stats);

private slots:
    void onSignalingConnected();
    void onSignalingDisconnected();
    void onSignalingError(QAbstractSocket::SocketError error);
    void onSignalingMessageReceived(const QString& message);
    void onIceConnectionStateChanged(int state);
    void onIceCandidateGathered(const QVariantMap& candidate);
    void onNegotiationNeeded();
    void onDataChannelOpened(const QString& label);
    void onDataChannelClosed(const QString& label);
    void onDataChannelMessage(const QString& label, const QByteArray& message);

private:
    void setupSignaling();
    void setupPeerConnection();
    void setupDataChannels();
    void createOffer();
    void createAnswer(const QVariantMap& offer);
    void processSignalingMessage(const QVariantMap& message);
    void updateConnectionState(ConnectionState state);
    void cleanupPeerConnection();
    void cleanupSignaling();
    void cleanupDataChannels();
    void processIceCandidate(const QVariantMap& candidate);
    void gatherStats();
    void updateParticipantInfo(const QString& participantId, const QVariantMap& info);
    void updateStreamInfo(const QString& streamId, const MediaStreamInfo& info);
    void sendSignalingMessage(const QVariantMap& message);
    void handleRemoteDescription(const QVariantMap& description);
    void handleRemoteCandidate(const QVariantMap& candidate);
    void handleParticipantEvent(const QVariantMap& event);
    void handleStreamEvent(const QVariantMap& event);
    void handleCommandMessage(const QVariantMap& message);

    static WebRTCEngine* s_instance;
    QMutex m_mutex;
    ConnectionState m_connectionState;
    QString m_roomId;
    QString m_displayName;
    QString m_signalingServer;
    QVariantList m_iceServers;
    QVariantMap m_config;
    VideoQuality m_videoQuality;

    QMap<MediaType, bool> m_mediaEnabled;
    QMap<QString, QVariantMap> m_participants;
    QMap<QString, MediaStreamInfo> m_streams;
    QMap<QString, QString> m_participantStreams;

    QNetworkAccessManager* m_networkManager;
    QWebSocket* m_signalingSocket;
    bool m_initialized;
};

Q_DECLARE_METATYPE(WebRTCEngine::MediaStreamInfo)

#endif // WEBRTCENGINE_H