#ifndef WEBRTCPROTOCOL_H
#define WEBRTCPROTOCOL_H

#include "../interfaces/IProtocolHandler.h"
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QTimer>

/**
 * @brief WebRTC协议处理器
 * 
 * WebRTCProtocol实现了WebRTC协议的处理逻辑，包括信令处理、
 * 媒体流管理、ICE连接建立等功能。
 */
class WebRTCProtocol : public IProtocolHandler
{
    Q_OBJECT

public:
    /**
     * @brief WebRTC连接状态枚举
     */
    enum WebRTCState {
        New,                ///< 新建状态
        Connecting,         ///< 连接中
        Connected,          ///< 已连接
        Disconnected,       ///< 已断开
        Failed,             ///< 连接失败
        Closed              ///< 已关闭
    };
    Q_ENUM(WebRTCState)

    /**
     * @brief ICE连接状态枚举
     */
    enum ICEConnectionState {
        ICENew,             ///< ICE新建
        ICEChecking,        ///< ICE检查中
        ICEConnected,       ///< ICE已连接
        ICECompleted,       ///< ICE完成
        ICEFailed,          ///< ICE失败
        ICEDisconnected,    ///< ICE断开
        ICEClosed           ///< ICE关闭
    };
    Q_ENUM(ICEConnectionState)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit WebRTCProtocol(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~WebRTCProtocol();

    // IProtocolHandler接口实现
    bool initialize(const QVariantMap& config = QVariantMap()) override;
    bool start() override;
    void stop() override;
    ProtocolStatus protocolStatus() const override;
    QString protocolName() const override;
    QString protocolVersion() const override;

    QByteArray encodeMessage(MessageType type, const QVariantMap& data) override;
    bool decodeMessage(const QByteArray& rawData, MessageType& type, QVariantMap& data) override;
    bool handleReceivedData(const QByteArray& data) override;
    bool sendMessage(MessageType type, const QVariantMap& data) override;
    bool sendHeartbeat() override;

    bool supportsFeature(const QString& feature) const override;
    QStringList supportedFeatures() const override;
    void setParameter(const QString& key, const QVariant& value) override;
    QVariant parameter(const QString& key) const override;
    QVariantMap protocolStats() const override;

    /**
     * @brief 获取WebRTC连接状态
     * @return WebRTC连接状态
     */
    WebRTCState webRTCState() const;

    /**
     * @brief 获取ICE连接状态
     * @return ICE连接状态
     */
    ICEConnectionState iceConnectionState() const;

    /**
     * @brief 设置STUN服务器列表
     * @param servers STUN服务器列表
     */
    void setStunServers(const QStringList& servers);

    /**
     * @brief 获取STUN服务器列表
     * @return STUN服务器列表
     */
    QStringList stunServers() const;

    /**
     * @brief 设置TURN服务器列表
     * @param servers TURN服务器列表
     */
    void setTurnServers(const QStringList& servers);

    /**
     * @brief 获取TURN服务器列表
     * @return TURN服务器列表
     */
    QStringList turnServers() const;

    /**
     * @brief 创建Offer
     * @return Offer SDP
     */
    QString createOffer();

    /**
     * @brief 创建Answer
     * @param offer Offer SDP
     * @return Answer SDP
     */
    QString createAnswer(const QString& offer);

    /**
     * @brief 设置远程描述
     * @param sdp 远程SDP
     * @return 设置是否成功
     */
    bool setRemoteDescription(const QString& sdp);

    /**
     * @brief 设置本地描述
     * @param sdp 本地SDP
     * @return 设置是否成功
     */
    bool setLocalDescription(const QString& sdp);

    /**
     * @brief 添加ICE候选
     * @param candidate ICE候选信息
     * @return 添加是否成功
     */
    bool addIceCandidate(const QVariantMap& candidate);

    /**
     * @brief 获取本地ICE候选列表
     * @return ICE候选列表
     */
    QVariantList getLocalIceCandidates() const;

public slots:
    void reset() override;
    void refresh() override;

    /**
     * @brief 开始ICE收集
     */
    void startIceGathering();

    /**
     * @brief 停止ICE收集
     */
    void stopIceGathering();

signals:
    /**
     * @brief WebRTC状态改变信号
     * @param state 新的WebRTC状态
     */
    void webRTCStateChanged(WebRTCState state);

    /**
     * @brief ICE连接状态改变信号
     * @param state 新的ICE连接状态
     */
    void iceConnectionStateChanged(ICEConnectionState state);

    /**
     * @brief ICE候选生成信号
     * @param candidate ICE候选信息
     */
    void iceCandidateGenerated(const QVariantMap& candidate);

    /**
     * @brief 本地SDP生成信号
     * @param sdp 本地SDP
     */
    void localDescriptionGenerated(const QString& sdp);

    /**
     * @brief 远程SDP接收信号
     * @param sdp 远程SDP
     */
    void remoteDescriptionReceived(const QString& sdp);

    /**
     * @brief 媒体流添加信号
     * @param streamId 流ID
     */
    void mediaStreamAdded(const QString& streamId);

    /**
     * @brief 媒体流移除信号
     * @param streamId 流ID
     */
    void mediaStreamRemoved(const QString& streamId);

    /**
     * @brief 数据通道打开信号
     * @param channelId 通道ID
     */
    void dataChannelOpened(const QString& channelId);

    /**
     * @brief 数据通道关闭信号
     * @param channelId 通道ID
     */
    void dataChannelClosed(const QString& channelId);

    /**
     * @brief 数据通道消息信号
     * @param channelId 通道ID
     * @param message 消息内容
     */
    void dataChannelMessage(const QString& channelId, const QByteArray& message);

private slots:
    /**
     * @brief 处理心跳定时器
     */
    void handleHeartbeatTimer();

    /**
     * @brief 处理ICE收集超时
     */
    void handleIceGatheringTimeout();

    /**
     * @brief 处理连接超时
     */
    void handleConnectionTimeout();

private:
    /**
     * @brief 初始化WebRTC引擎
     * @return 初始化是否成功
     */
    bool initializeWebRTCEngine();

    /**
     * @brief 清理WebRTC引擎
     */
    void cleanupWebRTCEngine();

    /**
     * @brief 配置ICE服务器
     */
    void configureIceServers();

    /**
     * @brief 处理信令消息
     * @param message 信令消息
     * @return 处理是否成功
     */
    bool handleSignalingMessage(const QVariantMap& message);

    /**
     * @brief 发送信令消息
     * @param message 信令消息
     * @return 发送是否成功
     */
    bool sendSignalingMessage(const QVariantMap& message);

    /**
     * @brief 更新WebRTC状态
     * @param state 新状态
     */
    void updateWebRTCState(WebRTCState state);

    /**
     * @brief 更新ICE连接状态
     * @param state 新状态
     */
    void updateIceConnectionState(ICEConnectionState state);

    /**
     * @brief 生成会话ID
     * @return 会话ID
     */
    QString generateSessionId();

    /**
     * @brief 验证SDP格式
     * @param sdp SDP字符串
     * @return 是否有效
     */
    bool isValidSDP(const QString& sdp);

    /**
     * @brief 解析ICE候选
     * @param candidateString 候选字符串
     * @return 候选信息映射
     */
    QVariantMap parseIceCandidate(const QString& candidateString);

    class Private;
    Private* d;
};

#endif // WEBRTCPROTOCOL_H