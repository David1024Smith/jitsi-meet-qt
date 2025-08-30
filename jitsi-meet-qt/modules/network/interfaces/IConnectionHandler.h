#ifndef ICONNECTIONHANDLER_H
#define ICONNECTIONHANDLER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QByteArray>

/**
 * @brief 连接处理器接口
 * 
 * IConnectionHandler定义了网络连接处理的标准接口，提供连接建立、
 * 数据传输和连接管理的抽象方法。
 */
class IConnectionHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 连接状态枚举
     */
    enum ConnectionStatus {
        Inactive,           ///< 未激活
        Connecting,         ///< 连接中
        Connected,          ///< 已连接
        Disconnecting,      ///< 断开中
        Disconnected,       ///< 已断开
        Error               ///< 错误状态
    };
    Q_ENUM(ConnectionStatus)

    /**
     * @brief 连接类型枚举
     */
    enum ConnectionType {
        TCP,                ///< TCP连接
        UDP,                ///< UDP连接
        WebSocket,          ///< WebSocket连接
        WebRTC,             ///< WebRTC连接
        HTTP,               ///< HTTP连接
        Custom              ///< 自定义连接
    };
    Q_ENUM(ConnectionType)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit IConnectionHandler(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * @brief 虚析构函数
     */
    virtual ~IConnectionHandler() = default;

    /**
     * @brief 初始化连接处理器
     * @param config 配置参数
     * @return 初始化是否成功
     */
    virtual bool initialize(const QVariantMap& config = QVariantMap()) = 0;

    /**
     * @brief 建立连接
     * @param endpoint 连接端点
     * @return 连接是否成功启动
     */
    virtual bool establishConnection(const QString& endpoint) = 0;

    /**
     * @brief 关闭连接
     */
    virtual void closeConnection() = 0;

    /**
     * @brief 检查连接状态
     * @return 是否已连接
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief 获取连接状态
     * @return 当前连接状态
     */
    virtual ConnectionStatus connectionStatus() const = 0;

    /**
     * @brief 获取连接类型
     * @return 连接类型
     */
    virtual ConnectionType connectionType() const = 0;

    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @return 发送是否成功
     */
    virtual bool sendData(const QByteArray& data) = 0;

    /**
     * @brief 发送文本数据
     * @param text 要发送的文本
     * @return 发送是否成功
     */
    virtual bool sendText(const QString& text) = 0;

    /**
     * @brief 获取连接ID
     * @return 连接唯一标识符
     */
    virtual QString connectionId() const = 0;

    /**
     * @brief 获取远程端点信息
     * @return 远程端点地址
     */
    virtual QString remoteEndpoint() const = 0;

    /**
     * @brief 获取本地端点信息
     * @return 本地端点地址
     */
    virtual QString localEndpoint() const = 0;

    /**
     * @brief 设置连接超时
     * @param timeout 超时时间（毫秒）
     */
    virtual void setConnectionTimeout(int timeout) = 0;

    /**
     * @brief 获取连接超时
     * @return 超时时间（毫秒）
     */
    virtual int connectionTimeout() const = 0;

    /**
     * @brief 获取连接统计信息
     * @return 统计信息映射
     */
    virtual QVariantMap connectionStats() const = 0;

    /**
     * @brief 设置连接属性
     * @param key 属性键
     * @param value 属性值
     */
    virtual void setProperty(const QString& key, const QVariant& value) = 0;

    /**
     * @brief 获取连接属性
     * @param key 属性键
     * @return 属性值
     */
    virtual QVariant property(const QString& key) const = 0;

public slots:
    /**
     * @brief 重新连接
     */
    virtual void reconnect() = 0;

    /**
     * @brief 刷新连接状态
     */
    virtual void refreshStatus() = 0;

signals:
    /**
     * @brief 连接状态改变信号
     * @param status 新的连接状态
     */
    void connectionStatusChanged(ConnectionStatus status);

    /**
     * @brief 连接建立信号
     */
    void connectionEstablished();

    /**
     * @brief 连接关闭信号
     */
    void connectionClosed();

    /**
     * @brief 数据接收信号
     * @param data 接收到的数据
     */
    void dataReceived(const QByteArray& data);

    /**
     * @brief 文本接收信号
     * @param text 接收到的文本
     */
    void textReceived(const QString& text);

    /**
     * @brief 数据发送完成信号
     * @param bytesWritten 发送的字节数
     */
    void dataSent(qint64 bytesWritten);

    /**
     * @brief 连接错误信号
     * @param error 错误信息
     */
    void connectionError(const QString& error);

    /**
     * @brief 连接超时信号
     */
    void connectionTimeout();

    /**
     * @brief 连接统计更新信号
     * @param stats 统计信息
     */
    void statsUpdated(const QVariantMap& stats);
};

#endif // ICONNECTIONHANDLER_H