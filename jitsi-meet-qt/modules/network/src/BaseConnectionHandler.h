#ifndef BASECONNECTIONHANDLER_H
#define BASECONNECTIONHANDLER_H

#include "../interfaces/IConnectionHandler.h"
#include <QObject>

/**
 * @brief 基础连接处理器类
 * 
 * BaseConnectionHandler提供了IConnectionHandler接口的基础实现，
 * 包含通用的连接管理逻辑。具体的连接类型可以继承此类并实现
 * 特定的连接逻辑。
 */
class BaseConnectionHandler : public IConnectionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param type 连接类型
     * @param parent 父对象
     */
    explicit BaseConnectionHandler(ConnectionType type, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~BaseConnectionHandler() override;

    // IConnectionHandler接口实现
    bool initialize(const QVariantMap& config = QVariantMap()) override;
    bool establishConnection(const QString& endpoint) override;
    void closeConnection() override;
    bool isConnected() const override;
    ConnectionStatus connectionStatus() const override;
    ConnectionType connectionType() const override;
    bool sendData(const QByteArray& data) override;
    bool sendText(const QString& text) override;
    QString connectionId() const override;
    QString remoteEndpoint() const override;
    QString localEndpoint() const override;
    void setConnectionTimeout(int timeout) override;
    int connectionTimeout() const override;
    QVariantMap connectionStats() const override;
    void setProperty(const QString& key, const QVariant& value) override;
    QVariant property(const QString& key) const override;

public slots:
    void reconnect() override;
    void refreshStatus() override;

protected:
    /**
     * @brief 更新连接状态
     * @param status 新的连接状态
     */
    void updateConnectionStatus(ConnectionStatus status);

    /**
     * @brief 处理接收到的数据
     * @param data 接收到的数据
     */
    void handleDataReceived(const QByteArray& data);

    /**
     * @brief 执行实际的连接建立操作（子类实现）
     * @param endpoint 连接端点
     * @return 连接是否成功
     */
    virtual bool doEstablishConnection(const QString& endpoint);

    /**
     * @brief 执行实际的连接关闭操作（子类实现）
     */
    virtual void doCloseConnection();

    /**
     * @brief 执行实际的数据发送操作（子类实现）
     * @param data 要发送的数据
     * @return 发送是否成功
     */
    virtual bool doSendData(const QByteArray& data);

private:
    class Private;
    Private* d;
};

#endif // BASECONNECTIONHANDLER_H