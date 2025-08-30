#ifndef NETWORKMANAGERIMPL_H
#define NETWORKMANAGERIMPL_H

#include "../interfaces/INetworkManager.h"
#include "../interfaces/IConnectionHandler.h"
#include <QObject>
#include <QTimer>

/**
 * @brief 网络管理器实现类
 * 
 * NetworkManagerImpl是INetworkManager接口的具体实现，
 * 提供完整的网络管理功能，包括连接管理、状态监控和自动重连。
 */
class NetworkManagerImpl : public INetworkManager
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit NetworkManagerImpl(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~NetworkManagerImpl() override;

    // INetworkManager接口实现
    bool initialize() override;
    ConnectionState connectionState() const override;
    NetworkQuality networkQuality() const override;
    bool connectToServer(const QString& serverUrl) override;
    void disconnect() override;
    bool isConnected() const override;
    void setServerConfiguration(const QVariantMap& config) override;
    QVariantMap serverConfiguration() const override;
    int networkLatency() const override;
    int bandwidth() const override;
    void setAutoReconnectEnabled(bool enabled) override;
    bool isAutoReconnectEnabled() const override;

public slots:
    void reconnect() override;
    void refreshNetworkStatus() override;

private slots:
    /**
     * @brief 处理连接建立
     */
    void onConnectionEstablished();

    /**
     * @brief 处理连接关闭
     */
    void onConnectionClosed();

    /**
     * @brief 处理连接错误
     * @param error 错误信息
     */
    void onConnectionError(const QString& error);

private:
    /**
     * @brief 更新连接状态
     * @param state 新的连接状态
     */
    void updateConnectionState(ConnectionState state);

    /**
     * @brief 更新网络质量
     */
    void updateNetworkQuality();

    class Private;
    Private* d;
};

#endif // NETWORKMANAGERIMPL_H