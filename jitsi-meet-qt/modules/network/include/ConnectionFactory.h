#ifndef CONNECTIONFACTORY_H
#define CONNECTIONFACTORY_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QSharedPointer>

// 前向声明
class IConnectionHandler;
class IProtocolHandler;

/**
 * @brief 连接工厂类
 * 
 * ConnectionFactory负责创建和管理各种类型的网络连接。
 * 它使用工厂模式来创建不同协议的连接处理器。
 */
class ConnectionFactory : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 连接类型枚举
     */
    enum ConnectionType {
        WebRTC,             ///< WebRTC连接
        HTTP,               ///< HTTP连接
        HTTPS,              ///< HTTPS连接
        WebSocket,          ///< WebSocket连接
        WebSocketSecure,    ///< 安全WebSocket连接
        XMPP,               ///< XMPP连接
        Custom              ///< 自定义连接
    };
    Q_ENUM(ConnectionType)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ConnectionFactory(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ConnectionFactory();

    /**
     * @brief 获取连接工厂单例实例
     * @return ConnectionFactory实例指针
     */
    static ConnectionFactory* instance();

    /**
     * @brief 创建连接处理器
     * @param type 连接类型
     * @param config 连接配置
     * @return 连接处理器指针
     */
    QSharedPointer<IConnectionHandler> createConnection(
        ConnectionType type, 
        const QVariantMap& config = QVariantMap()
    );

    /**
     * @brief 创建协议处理器
     * @param protocol 协议名称
     * @param config 协议配置
     * @return 协议处理器指针
     */
    QSharedPointer<IProtocolHandler> createProtocolHandler(
        const QString& protocol,
        const QVariantMap& config = QVariantMap()
    );

    /**
     * @brief 注册自定义连接类型
     * @param typeName 类型名称
     * @param creator 创建函数
     * @return 注册是否成功
     */
    bool registerConnectionType(
        const QString& typeName,
        std::function<QSharedPointer<IConnectionHandler>(const QVariantMap&)> creator
    );

    /**
     * @brief 注册自定义协议处理器
     * @param protocolName 协议名称
     * @param creator 创建函数
     * @return 注册是否成功
     */
    bool registerProtocolHandler(
        const QString& protocolName,
        std::function<QSharedPointer<IProtocolHandler>(const QVariantMap&)> creator
    );

    /**
     * @brief 获取支持的连接类型列表
     * @return 连接类型列表
     */
    QStringList supportedConnectionTypes() const;

    /**
     * @brief 获取支持的协议列表
     * @return 协议列表
     */
    QStringList supportedProtocols() const;

    /**
     * @brief 检查连接类型是否支持
     * @param type 连接类型
     * @return 是否支持
     */
    bool isConnectionTypeSupported(ConnectionType type) const;

    /**
     * @brief 检查协议是否支持
     * @param protocol 协议名称
     * @return 是否支持
     */
    bool isProtocolSupported(const QString& protocol) const;

    /**
     * @brief 获取连接类型的默认配置
     * @param type 连接类型
     * @return 默认配置
     */
    QVariantMap getDefaultConfiguration(ConnectionType type) const;

    /**
     * @brief 验证连接配置
     * @param type 连接类型
     * @param config 配置参数
     * @return 配置是否有效
     */
    bool validateConfiguration(ConnectionType type, const QVariantMap& config) const;

public slots:
    /**
     * @brief 清理所有连接
     */
    void cleanupConnections();

    /**
     * @brief 重置工厂设置
     */
    void reset();

signals:
    /**
     * @brief 连接创建信号
     * @param type 连接类型
     * @param connectionId 连接ID
     */
    void connectionCreated(ConnectionType type, const QString& connectionId);

    /**
     * @brief 连接销毁信号
     * @param connectionId 连接ID
     */
    void connectionDestroyed(const QString& connectionId);

    /**
     * @brief 工厂错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private:
    /**
     * @brief 创建WebRTC连接
     * @param config 配置参数
     * @return 连接处理器
     */
    QSharedPointer<IConnectionHandler> createWebRTCConnection(const QVariantMap& config);

    /**
     * @brief 创建HTTP连接
     * @param config 配置参数
     * @return 连接处理器
     */
    QSharedPointer<IConnectionHandler> createHTTPConnection(const QVariantMap& config);

    /**
     * @brief 创建WebSocket连接
     * @param config 配置参数
     * @return 连接处理器
     */
    QSharedPointer<IConnectionHandler> createWebSocketConnection(const QVariantMap& config);

    /**
     * @brief 创建XMPP连接
     * @param config 配置参数
     * @return 连接处理器
     */
    QSharedPointer<IConnectionHandler> createXMPPConnection(const QVariantMap& config);

    /**
     * @brief 初始化默认连接类型
     */
    void initializeDefaultTypes();

    /**
     * @brief 生成连接ID
     * @param type 连接类型
     * @return 连接ID
     */
    QString generateConnectionId(ConnectionType type);

    class Private;
    Private* d;
};

#endif // CONNECTIONFACTORY_H