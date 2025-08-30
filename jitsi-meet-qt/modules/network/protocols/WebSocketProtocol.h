#ifndef WEBSOCKETPROTOCOL_H
#define WEBSOCKETPROTOCOL_H

#include "../interfaces/IProtocolHandler.h"
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QWebSocket>
#include <QTimer>
#include <QQueue>

/**
 * @brief WebSocket协议处理器
 * 
 * WebSocketProtocol实现了WebSocket协议的处理逻辑，包括连接管理、
 * 消息发送接收、心跳保持等功能。
 */
class WebSocketProtocol : public IProtocolHandler
{
    Q_OBJECT

public:
    /**
     * @brief WebSocket连接状态枚举
     */
    enum WebSocketState {
        Unconnected,        ///< 未连接
        HostLookup,         ///< 主机查找
        Connecting,         ///< 连接中
        Connected,          ///< 已连接
        Bound,              ///< 已绑定
        Listening,          ///< 监听中
        Closing,            ///< 关闭中
        Closed              ///< 已关闭
    };
    Q_ENUM(WebSocketState)

    /**
     * @brief 消息格式枚举
     */
    enum MessageFormat {
        TextMessage,        ///< 文本消息
        BinaryMessage,      ///< 二进制消息
        JsonMessage         ///< JSON消息
    };
    Q_ENUM(MessageFormat)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit WebSocketProtocol(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~WebSocketProtocol();

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
     * @brief 连接到WebSocket服务器
     * @param url 服务器URL
     * @return 连接是否成功启动
     */
    bool connectToServer(const QString& url);

    /**
     * @brief 断开WebSocket连接
     */
    void disconnectFromServer();

    /**
     * @brief 获取WebSocket连接状态
     * @return 连接状态
     */
    WebSocketState webSocketState() const;

    /**
     * @brief 发送文本消息
     * @param message 文本消息
     * @return 发送是否成功
     */
    bool sendTextMessage(const QString& message);

    /**
     * @brief 发送二进制消息
     * @param data 二进制数据
     * @return 发送是否成功
     */
    bool sendBinaryMessage(const QByteArray& data);

    /**
     * @brief 发送JSON消息
     * @param json JSON对象
     * @return 发送是否成功
     */
    bool sendJsonMessage(const QVariantMap& json);

    /**
     * @brief 设置服务器URL
     * @param url 服务器URL
     */
    void setServerUrl(const QString& url);

    /**
     * @brief 获取服务器URL
     * @return 服务器URL
     */
    QString serverUrl() const;

    /**
     * @brief 设置连接超时时间
     * @param timeout 超时时间（毫秒）
     */
    void setConnectionTimeout(int timeout);

    /**
     * @brief 获取连接超时时间
     * @return 超时时间（毫秒）
     */
    int connectionTimeout() const;

    /**
     * @brief 设置心跳间隔
     * @param interval 心跳间隔（毫秒）
     */
    void setHeartbeatInterval(int interval);

    /**
     * @brief 获取心跳间隔
     * @return 心跳间隔（毫秒）
     */
    int heartbeatInterval() const;

    /**
     * @brief 设置自动重连
     * @param enabled 是否启用自动重连
     */
    void setAutoReconnect(bool enabled);

    /**
     * @brief 获取是否启用自动重连
     * @return 是否启用自动重连
     */
    bool autoReconnect() const;

    /**
     * @brief 设置重连间隔
     * @param interval 重连间隔（毫秒）
     */
    void setReconnectInterval(int interval);

    /**
     * @brief 获取重连间隔
     * @return 重连间隔（毫秒）
     */
    int reconnectInterval() const;

    /**
     * @brief 设置最大重连次数
     * @param maxAttempts 最大重连次数
     */
    void setMaxReconnectAttempts(int maxAttempts);

    /**
     * @brief 获取最大重连次数
     * @return 最大重连次数
     */
    int maxReconnectAttempts() const;

    /**
     * @brief 设置消息格式
     * @param format 消息格式
     */
    void setMessageFormat(MessageFormat format);

    /**
     * @brief 获取消息格式
     * @return 消息格式
     */
    MessageFormat messageFormat() const;

    /**
     * @brief 获取连接延迟
     * @return 连接延迟（毫秒）
     */
    int connectionLatency() const;

    /**
     * @brief 获取发送队列大小
     * @return 队列大小
     */
    int sendQueueSize() const;

public slots:
    void reset() override;
    void refresh() override;

    /**
     * @brief 手动重连
     */
    void reconnect();

    /**
     * @brief 清空发送队列
     */
    void clearSendQueue();

    /**
     * @brief 刷新发送队列
     */
    void flushSendQueue();

signals:
    /**
     * @brief WebSocket状态改变信号
     * @param state 新的连接状态
     */
    void webSocketStateChanged(WebSocketState state);

    /**
     * @brief 连接建立信号
     */
    void connected();

    /**
     * @brief 连接断开信号
     */
    void disconnected();

    /**
     * @brief 文本消息接收信号
     * @param message 文本消息
     */
    void textMessageReceived(const QString& message);

    /**
     * @brief 二进制消息接收信号
     * @param data 二进制数据
     */
    void binaryMessageReceived(const QByteArray& data);

    /**
     * @brief JSON消息接收信号
     * @param json JSON对象
     */
    void jsonMessageReceived(const QVariantMap& json);

    /**
     * @brief 连接错误信号
     * @param error 错误信息
     */
    void connectionError(const QString& error);

    /**
     * @brief 重连开始信号
     * @param attempt 重连次数
     */
    void reconnectStarted(int attempt);

    /**
     * @brief 重连成功信号
     */
    void reconnectSucceeded();

    /**
     * @brief 重连失败信号
     * @param error 错误信息
     */
    void reconnectFailed(const QString& error);

    /**
     * @brief 消息发送失败信号
     * @param message 消息内容
     * @param error 错误信息
     */
    void messageSendFailed(const QByteArray& message, const QString& error);

private slots:
    /**
     * @brief 处理WebSocket连接
     */
    void handleConnected();

    /**
     * @brief 处理WebSocket断开
     */
    void handleDisconnected();

    /**
     * @brief 处理文本消息接收
     * @param message 文本消息
     */
    void handleTextMessageReceived(const QString& message);

    /**
     * @brief 处理二进制消息接收
     * @param data 二进制数据
     */
    void handleBinaryMessageReceived(const QByteArray& data);

    /**
     * @brief 处理WebSocket错误
     * @param error WebSocket错误
     */
    void handleWebSocketError(QAbstractSocket::SocketError error);

    /**
     * @brief 处理SSL错误
     * @param errors SSL错误列表
     */
    void handleSslErrors(const QList<QSslError>& errors);

    /**
     * @brief 处理心跳定时器
     */
    void handleHeartbeatTimer();

    /**
     * @brief 处理重连定时器
     */
    void handleReconnectTimer();

    /**
     * @brief 处理连接超时
     */
    void handleConnectionTimeout();

    /**
     * @brief 处理发送队列定时器
     */
    void handleSendQueueTimer();

private:
    /**
     * @brief 初始化WebSocket
     * @return 初始化是否成功
     */
    bool initializeWebSocket();

    /**
     * @brief 清理WebSocket
     */
    void cleanupWebSocket();

    /**
     * @brief 启动心跳定时器
     */
    void startHeartbeatTimer();

    /**
     * @brief 停止心跳定时器
     */
    void stopHeartbeatTimer();

    /**
     * @brief 启动重连定时器
     */
    void startReconnectTimer();

    /**
     * @brief 停止重连定时器
     */
    void stopReconnectTimer();

    /**
     * @brief 更新WebSocket状态
     * @param state 新状态
     */
    void updateWebSocketState(WebSocketState state);

    /**
     * @brief 处理消息队列
     */
    void processMessageQueue();

    /**
     * @brief 将消息加入发送队列
     * @param message 消息数据
     */
    void enqueueMessage(const QByteArray& message);

    /**
     * @brief 验证服务器URL
     * @param url 服务器URL
     * @return 是否有效
     */
    bool isValidServerUrl(const QString& url) const;

    /**
     * @brief 解析JSON消息
     * @param data 消息数据
     * @return JSON对象
     */
    QVariantMap parseJsonMessage(const QByteArray& data);

    /**
     * @brief 序列化JSON消息
     * @param json JSON对象
     * @return 消息数据
     */
    QByteArray serializeJsonMessage(const QVariantMap& json);

    /**
     * @brief 生成心跳消息
     * @return 心跳消息
     */
    QByteArray generateHeartbeatMessage();

    /**
     * @brief 检查是否为心跳响应
     * @param data 消息数据
     * @return 是否为心跳响应
     */
    bool isHeartbeatResponse(const QByteArray& data);

    class Private;
    Private* d;
};

#endif // WEBSOCKETPROTOCOL_H