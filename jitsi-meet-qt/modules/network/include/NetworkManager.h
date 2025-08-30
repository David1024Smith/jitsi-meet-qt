#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QTimer>

/**
 * @brief 网络管理器类
 * 
 * NetworkManager提供高级网络管理功能，包括连接管理、状态监控、
 * 自动重连等功能。它是网络模块的主要接口类。
 */
class NetworkManager : public QObject
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
    explicit NetworkManager(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~NetworkManager();

    /**
     * @brief 获取网络管理器单例实例
     * @return NetworkManager实例指针
     */
    static NetworkManager* instance();

    /**
     * @brief 初始化网络管理器
     * @param config 配置参数
     * @return 初始化是否成功
     */
    bool initialize(const QVariantMap& config = QVariantMap());

    /**
     * @brief 获取连接状态
     * @return 当前连接状态
     */
    ConnectionState connectionState() const;

    /**
     * @brief 获取网络质量
     * @return 当前网络质量
     */
    NetworkQuality networkQuality() const;

    /**
     * @brief 连接到服务器
     * @param serverUrl 服务器URL
     * @return 连接是否成功启动
     */
    bool connectToServer(const QString& serverUrl = QString());

    /**
     * @brief 断开连接
     */
    void disconnect();

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 设置服务器配置
     * @param config 服务器配置
     */
    void setServerConfiguration(const QVariantMap& config);

    /**
     * @brief 获取服务器配置
     * @return 服务器配置
     */
    QVariantMap serverConfiguration() const;

    /**
     * @brief 启用自动重连
     * @param enabled 是否启用
     */
    void setAutoReconnectEnabled(bool enabled);

    /**
     * @brief 检查是否启用自动重连
     * @return 是否启用自动重连
     */
    bool isAutoReconnectEnabled() const;

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
     * @brief 获取网络延迟
     * @return 网络延迟（毫秒）
     */
    int networkLatency() const;

    /**
     * @brief 获取带宽信息
     * @return 带宽信息（kbps）
     */
    int bandwidth() const;

public slots:
    /**
     * @brief 手动触发重连
     */
    void reconnect();

    /**
     * @brief 刷新网络状态
     */
    void refreshNetworkStatus();

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

private slots:
    /**
     * @brief 处理连接超时
     */
    void handleConnectionTimeout();

    /**
     * @brief 处理重连定时器
     */
    void handleReconnectTimer();

    /**
     * @brief 处理网络状态检查
     */
    void handleNetworkCheck();

    /**
     * @brief 处理网络错误
     * @param error 错误信息
     */
    void handleNetworkError(const QString& error);

private:
    /**
     * @brief 执行实际的连接操作
     * @param url 连接URL
     * @return 连接是否成功
     */
    bool doConnect(const QString& url);

    /**
     * @brief 执行实际的断开操作
     */
    void doDisconnect();

    /**
     * @brief 更新连接状态
     * @param state 新状态
     */
    void updateConnectionState(ConnectionState state);

    /**
     * @brief 更新网络质量
     */
    void updateNetworkQuality();

    /**
     * @brief 启动重连定时器
     */
    void startReconnectTimer();

    /**
     * @brief 停止重连定时器
     */
    void stopReconnectTimer();

    class Private;
    Private* d;
};

#endif // NETWORKMANAGER_H