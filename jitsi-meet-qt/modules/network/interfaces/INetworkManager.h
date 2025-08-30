#ifndef INETWORKMANAGER_H
#define INETWORKMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantMap>

/**
 * @brief 网络管理器接口
 * 
 * INetworkManager定义了网络管理器的标准接口，提供网络连接管理、
 * 状态监控和配置管理的抽象方法。
 */
class INetworkManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 连接状态枚举
     */
    enum ConnectionState {
        Disconnected,       ///< 已断开
        Connecting,         ///< 连接中
        Connected,          ///< 已连接
        Reconnecting,       ///< 重连中
        Error               ///< 连接错误
    };
    Q_ENUM(ConnectionState)

    /**
     * @brief 网络质量枚举
     */
    enum NetworkQuality {
        Unknown,            ///< 未知
        Poor,               ///< 差
        Fair,               ///< 一般
        Good,               ///< 良好
        Excellent           ///< 优秀
    };
    Q_ENUM(NetworkQuality)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit INetworkManager(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * @brief 虚析构函数
     */
    virtual ~INetworkManager() = default;

    /**
     * @brief 初始化网络管理器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 获取连接状态
     * @return 当前连接状态
     */
    virtual ConnectionState connectionState() const = 0;

    /**
     * @brief 获取网络质量
     * @return 当前网络质量
     */
    virtual NetworkQuality networkQuality() const = 0;

    /**
     * @brief 连接到服务器
     * @param serverUrl 服务器URL
     * @return 连接是否成功启动
     */
    virtual bool connectToServer(const QString& serverUrl) = 0;

    /**
     * @brief 断开连接
     */
    virtual void disconnect() = 0;

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief 设置服务器配置
     * @param config 服务器配置
     */
    virtual void setServerConfiguration(const QVariantMap& config) = 0;

    /**
     * @brief 获取服务器配置
     * @return 服务器配置
     */
    virtual QVariantMap serverConfiguration() const = 0;

    /**
     * @brief 获取网络延迟
     * @return 网络延迟（毫秒）
     */
    virtual int networkLatency() const = 0;

    /**
     * @brief 获取带宽信息
     * @return 带宽信息（kbps）
     */
    virtual int bandwidth() const = 0;

    /**
     * @brief 启用自动重连
     * @param enabled 是否启用
     */
    virtual void setAutoReconnectEnabled(bool enabled) = 0;

    /**
     * @brief 检查是否启用自动重连
     * @return 是否启用自动重连
     */
    virtual bool isAutoReconnectEnabled() const = 0;

public slots:
    /**
     * @brief 手动触发重连
     */
    virtual void reconnect() = 0;

    /**
     * @brief 刷新网络状态
     */
    virtual void refreshNetworkStatus() = 0;

signals:
    /**
     * @brief 连接状态改变信号
     * @param state 新的连接状态
     */
    void connectionStateChanged(ConnectionState state);

    /**
     * @brief 网络质量改变信号
     * @param quality 新的网络质量
     */
    void networkQualityChanged(NetworkQuality quality);

    /**
     * @brief 数据接收信号
     * @param data 接收到的数据
     */
    void dataReceived(const QByteArray& data);

    /**
     * @brief 数据发送信号
     * @param data 发送的数据
     */
    void dataSent(const QByteArray& data);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 连接建立信号
     */
    void connected();

    /**
     * @brief 连接断开信号
     */
    void disconnected();

    /**
     * @brief 重连开始信号
     */
    void reconnectStarted();

    /**
     * @brief 网络统计更新信号
     * @param stats 网络统计信息
     */
    void networkStatsUpdated(const QVariantMap& stats);
};

#endif // INETWORKMANAGER_H