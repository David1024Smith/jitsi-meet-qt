#ifndef XMPPCLIENT_H
#define XMPPCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QDomDocument>
#include <QDomElement>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QMap>
#include <QStringList>

/**
 * @brief XMPP客户端类，处理与Jitsi Meet服务器的XMPP over WebSocket通信
 * 
 * 该类实现了Jitsi Meet所需的XMPP协议功能，包括：
 * - WebSocket连接管理
 * - XMPP消息解析和构建
 * - 会议室加入和离开流程
 * - 参与者状态管理
 * - 事件通知机制
 */
class XMPPClient : public QObject
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
        Authenticating,  ///< 认证中
        Authenticated,   ///< 已认证
        JoiningRoom,     ///< 加入房间中
        InRoom,          ///< 在房间中
        Disconnecting,   ///< 断开连接中
        Error            ///< 错误状态
    };
    Q_ENUM(ConnectionState)

    /**
     * @brief 参与者信息结构
     */
    struct Participant {
        QString jid;           ///< 参与者JID
        QString displayName;   ///< 显示名称
        QString role;          ///< 角色（moderator, participant等）
        bool audioMuted;       ///< 音频静音状态
        bool videoMuted;       ///< 视频静音状态
        QString status;        ///< 状态信息
        QDateTime joinTime;    ///< 加入时间
    };

    explicit XMPPClient(QObject *parent = nullptr);
    ~XMPPClient();

    /**
     * @brief 连接到Jitsi Meet服务器
     * @param serverUrl 服务器URL（如：https://meet.jit.si）
     * @param roomName 房间名称
     * @param displayName 用户显示名称
     */
    void connectToServer(const QString& serverUrl, const QString& roomName, const QString& displayName = QString());

    /**
     * @brief 断开连接
     */
    void disconnect();

    /**
     * @brief 发送聊天消息
     * @param message 消息内容
     */
    void sendChatMessage(const QString& message);

    /**
     * @brief 发送存在信息（状态更新）
     * @param status 状态信息
     */
    void sendPresence(const QString& status = QString());

    /**
     * @brief 更新音频静音状态
     * @param muted 是否静音
     */
    void setAudioMuted(bool muted);

    /**
     * @brief 更新视频静音状态
     * @param muted 是否静音
     */
    void setVideoMuted(bool muted);

    /**
     * @brief 离开当前房间
     */
    void leaveRoom();

    // Getters
    ConnectionState connectionState() const { return m_connectionState; }
    QString currentRoom() const { return m_roomName; }
    QString serverUrl() const { return m_serverUrl; }
    QString userJid() const { return m_userJid; }
    QString displayName() const { return m_displayName; }
    QList<Participant> participants() const { return m_participants.values(); }
    bool isConnected() const { return m_connectionState == Connected || m_connectionState == Authenticated || m_connectionState == InRoom; }
    bool isInRoom() const { return m_connectionState == InRoom; }

signals:
    /**
     * @brief 连接状态改变信号
     * @param state 新的连接状态
     */
    void connectionStateChanged(ConnectionState state);

    /**
     * @brief 连接成功信号
     */
    void connected();

    /**
     * @brief 断开连接信号
     */
    void disconnected();

    /**
     * @brief 认证成功信号
     */
    void authenticated();

    /**
     * @brief 加入房间成功信号
     */
    void roomJoined();

    /**
     * @brief 离开房间信号
     */
    void roomLeft();

    /**
     * @brief 参与者加入信号
     * @param participant 参与者信息
     */
    void participantJoined(const Participant& participant);

    /**
     * @brief 参与者离开信号
     * @param jid 参与者JID
     */
    void participantLeft(const QString& jid);

    /**
     * @brief 参与者状态更新信号
     * @param participant 更新后的参与者信息
     */
    void participantUpdated(const Participant& participant);

    /**
     * @brief 聊天消息接收信号
     * @param from 发送者JID
     * @param message 消息内容
     * @param timestamp 时间戳
     */
    void chatMessageReceived(const QString& from, const QString& message, const QDateTime& timestamp);

    /**
     * @brief 错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief WebSocket连接成功处理
     */
    void onWebSocketConnected();

    /**
     * @brief WebSocket断开连接处理
     */
    void onWebSocketDisconnected();

    /**
     * @brief WebSocket消息接收处理
     * @param message 接收到的消息
     */
    void onWebSocketMessageReceived(const QString& message);

    /**
     * @brief WebSocket错误处理
     * @param error 错误类型
     */
    void onWebSocketError(QAbstractSocket::SocketError error);

    /**
     * @brief 心跳定时器处理
     */
    void onHeartbeatTimer();

    /**
     * @brief 重连定时器处理
     */
    void onReconnectTimer();

    /**
     * @brief 配置获取完成处理
     */
    void onConfigurationReceived();

private:
    /**
     * @brief 设置连接状态
     * @param state 新状态
     */
    void setConnectionState(ConnectionState state);

    /**
     * @brief 获取服务器配置
     */
    void fetchServerConfiguration();

    /**
     * @brief 建立WebSocket连接
     */
    void establishWebSocketConnection();

    /**
     * @brief 发送XMPP节
     * @param stanza XMPP节内容
     */
    void sendXMPPStanza(const QString& stanza);

    /**
     * @brief 处理接收到的XMPP消息
     * @param doc XML文档
     */
    void processXMPPMessage(const QDomDocument& doc);

    /**
     * @brief 处理存在信息
     * @param element presence元素
     */
    void handlePresence(const QDomElement& element);

    /**
     * @brief 处理消息
     * @param element message元素
     */
    void handleMessage(const QDomElement& element);

    /**
     * @brief 处理IQ（Info/Query）
     * @param element iq元素
     */
    void handleIQ(const QDomElement& element);

    /**
     * @brief 发送初始存在信息
     */
    void sendInitialPresence();

    /**
     * @brief 加入MUC房间
     */
    void joinMUCRoom();

    /**
     * @brief 生成唯一ID
     * @return 唯一ID字符串
     */
    QString generateUniqueId();

    /**
     * @brief 构建完整JID
     * @param node 节点名
     * @param domain 域名
     * @param resource 资源名
     * @return 完整JID
     */
    QString buildJID(const QString& node, const QString& domain, const QString& resource = QString());

    /**
     * @brief 解析JID
     * @param jid 完整JID
     * @param node 输出节点名
     * @param domain 输出域名
     * @param resource 输出资源名
     */
    void parseJID(const QString& jid, QString& node, QString& domain, QString& resource);

    /**
     * @brief 启动重连机制
     */
    void startReconnection();

    /**
     * @brief 停止重连机制
     */
    void stopReconnection();

    /**
     * @brief 重置连接状态
     */
    void resetConnection();

    // 网络组件
    QWebSocket* m_webSocket;                    ///< WebSocket连接
    QNetworkAccessManager* m_networkManager;    ///< 网络访问管理器
    QTimer* m_heartbeatTimer;                   ///< 心跳定时器
    QTimer* m_reconnectTimer;                   ///< 重连定时器

    // 连接信息
    ConnectionState m_connectionState;          ///< 当前连接状态
    QString m_serverUrl;                        ///< 服务器URL
    QString m_roomName;                         ///< 房间名称
    QString m_displayName;                      ///< 用户显示名称
    QString m_userJid;                          ///< 用户JID
    QString m_roomJid;                          ///< 房间JID
    QString m_websocketUrl;                     ///< WebSocket URL
    QString m_domain;                           ///< XMPP域名
    QString m_mucDomain;                        ///< MUC域名
    QString m_focusJid;                         ///< Focus组件JID

    // 会话信息
    QMap<QString, Participant> m_participants;  ///< 参与者列表
    QString m_sessionId;                        ///< 会话ID
    bool m_audioMuted;                          ///< 本地音频静音状态
    bool m_videoMuted;                          ///< 本地视频静音状态

    // 重连机制
    int m_reconnectAttempts;                    ///< 重连尝试次数
    static const int MAX_RECONNECT_ATTEMPTS = 5; ///< 最大重连次数
    static const int RECONNECT_INTERVAL = 3000;  ///< 重连间隔（毫秒）
    static const int HEARTBEAT_INTERVAL = 30000; ///< 心跳间隔（毫秒）

    // 服务器配置
    QJsonObject m_serverConfig;                 ///< 服务器配置信息
};

#endif // XMPPCLIENT_H