#include "NetworkManagerImpl.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>
#include <QRandomGenerator>

class NetworkManagerImpl::Private
{
public:
    QNetworkAccessManager* networkManager;
    QTimer* connectionTimer;
    INetworkManager::ConnectionState connectionState;
    INetworkManager::NetworkQuality networkQuality;
    QString serverUrl;
    QVariantMap serverConfig;
    bool autoReconnectEnabled;
    int latency;
    int bandwidthValue;
    QString lastError;
    
    Private() 
        : networkManager(nullptr)
        , connectionTimer(nullptr)
        , connectionState(INetworkManager::ConnectionState::Disconnected)
        , networkQuality(INetworkManager::NetworkQuality::Unknown)
        , autoReconnectEnabled(true)
        , latency(0)
        , bandwidthValue(0)
    {
    }
};

NetworkManagerImpl::NetworkManagerImpl(QObject* parent)
    : INetworkManager(parent)
    , d(new Private)
{
    d->networkManager = new QNetworkAccessManager(this);
    d->connectionTimer = new QTimer(this);
    
    // 连接信号
    connect(d->connectionTimer, &QTimer::timeout,
            this, &NetworkManagerImpl::refreshNetworkStatus);
}

NetworkManagerImpl::~NetworkManagerImpl()
{
    delete d;
}

/**
 * @brief 初始化网络管理器
 * @return 初始化是否成功
 */
bool NetworkManagerImpl::initialize()
{
    qDebug() << "NetworkManagerImpl: 初始化网络管理器";
    
    d->connectionState = INetworkManager::ConnectionState::Disconnected;
    d->networkQuality = INetworkManager::NetworkQuality::Unknown;
    
    // 启动网络状态监控定时器
    d->connectionTimer->start(5000); // 每5秒检查一次
    
    // 注意：INetworkManager接口中没有initialized信号
    // 这里暂时注释掉，如果需要可以在子类中添加
    // emit initialized();
    return true;
}

/**
 * @brief 获取连接状态
 * @return 当前连接状态
 */
INetworkManager::ConnectionState NetworkManagerImpl::connectionState() const
{
    return d->connectionState;
}

/**
 * @brief 获取网络质量
 * @return 当前网络质量
 */
INetworkManager::NetworkQuality NetworkManagerImpl::networkQuality() const
{
    return d->networkQuality;
}

/**
 * @brief 连接到服务器
 * @param serverUrl 服务器URL
 * @return 连接是否成功启动
 */
bool NetworkManagerImpl::connectToServer(const QString& serverUrl)
{
    qDebug() << "NetworkManagerImpl: 连接到服务器:" << serverUrl;
    
    d->serverUrl = serverUrl;
    updateConnectionState(ConnectionState::Connecting);
    
    // 模拟连接过程
    QTimer::singleShot(1000, this, [this]() {
        updateConnectionState(INetworkManager::ConnectionState::Connected);
        emit connected();
        onConnectionEstablished();
    });
    
    return true;
}

/**
 * @brief 断开连接
 */
void NetworkManagerImpl::disconnect()
{
    qDebug() << "NetworkManagerImpl: 断开连接";
    
    updateConnectionState(INetworkManager::ConnectionState::Disconnected);
    
    // 停止定时器
    d->connectionTimer->stop();
    
    // 模拟断开过程
    QTimer::singleShot(500, this, [this]() {
        updateConnectionState(INetworkManager::ConnectionState::Disconnected);
        emit disconnected();
        onConnectionClosed();
    });
}

/**
 * @brief 检查是否已连接
 * @return 是否已连接
 */
bool NetworkManagerImpl::isConnected() const
{
    return d->connectionState == INetworkManager::ConnectionState::Connected;
}

/**
 * @brief 设置服务器配置
 * @param config 服务器配置
 */
void NetworkManagerImpl::setServerConfiguration(const QVariantMap& config)
{
    qDebug() << "NetworkManagerImpl: 设置服务器配置";
    d->serverConfig = config;
    // 注意：INetworkManager接口中没有serverConfigurationChanged信号
    // 这里暂时注释掉，如果需要可以在子类中添加
    // emit serverConfigurationChanged(config);
}

/**
 * @brief 获取服务器配置
 * @return 服务器配置
 */
QVariantMap NetworkManagerImpl::serverConfiguration() const
{
    return d->serverConfig;
}

/**
 * @brief 获取网络延迟
 * @return 网络延迟(毫秒)
 */
int NetworkManagerImpl::networkLatency() const
{
    return d->latency;
}

/**
 * @brief 获取带宽
 * @return 带宽(kbps)
 */
int NetworkManagerImpl::bandwidth() const
{
    return d->bandwidthValue;
}

/**
 * @brief 设置自动重连是否启用
 * @param enabled 是否启用自动重连
 */
void NetworkManagerImpl::setAutoReconnectEnabled(bool enabled)
{
    qDebug() << "NetworkManagerImpl: 设置自动重连:" << enabled;
    d->autoReconnectEnabled = enabled;
    // 注意：INetworkManager接口中没有autoReconnectEnabledChanged信号
    // 这里暂时注释掉，如果需要可以在子类中添加
    // emit autoReconnectEnabledChanged(enabled);
}

/**
 * @brief 检查自动重连是否启用
 * @return 是否启用自动重连
 */
bool NetworkManagerImpl::isAutoReconnectEnabled() const
{
    return d->autoReconnectEnabled;
}

/**
 * @brief 重新连接
 */
void NetworkManagerImpl::reconnect()
{
    qDebug() << "NetworkManagerImpl: 重新连接";
    
    if (isConnected()) {
        disconnect();
    }
    
    // 延迟重连
    QTimer::singleShot(1000, this, [this]() {
        if (!d->serverUrl.isEmpty()) {
            connectToServer(d->serverUrl);
        }
    });
}

/**
 * @brief 刷新网络状态
 */
void NetworkManagerImpl::refreshNetworkStatus()
{
    // 模拟网络质量检测
    updateNetworkQuality();
    
    // 模拟延迟测量
    d->latency = 50 + (QRandomGenerator::global()->bounded(100)); // 50-150ms
    
    // 模拟带宽测量
    d->bandwidthValue = 1000 + (QRandomGenerator::global()->bounded(9000)); // 1-10 Mbps
    
    // 发出网络统计更新信号
    QVariantMap stats;
    stats["latency"] = d->latency;
    stats["bandwidth"] = d->bandwidthValue;
    emit networkStatsUpdated(stats);
}

/**
 * @brief 处理连接建立
 */
void NetworkManagerImpl::onConnectionEstablished()
{
    qDebug() << "NetworkManagerImpl: 连接已建立";
    updateNetworkQuality();
}

/**
 * @brief 处理连接关闭
 */
void NetworkManagerImpl::onConnectionClosed()
{
    qDebug() << "NetworkManagerImpl: 连接已关闭";
    d->networkQuality = INetworkManager::NetworkQuality::Unknown;
    d->latency = 0;
    d->bandwidthValue = 0;
}

/**
 * @brief 处理连接错误
 * @param error 错误信息
 */
void NetworkManagerImpl::onConnectionError(const QString& error)
{
    qDebug() << "NetworkManagerImpl: 连接错误:" << error;
    d->lastError = error;
    updateConnectionState(INetworkManager::ConnectionState::Error);
    emit errorOccurred(error);
    
    // 如果启用了自动重连，尝试重连
    if (d->autoReconnectEnabled) {
        QTimer::singleShot(5000, this, &NetworkManagerImpl::reconnect);
    }
}

/**
 * @brief 更新连接状态
 * @param state 新的连接状态
 */
void NetworkManagerImpl::updateConnectionState(INetworkManager::ConnectionState state)
{
    if (d->connectionState != state) {
        INetworkManager::ConnectionState oldState = d->connectionState;
        d->connectionState = state;
        
        qDebug() << "NetworkManagerImpl: 连接状态变更:" << static_cast<int>(oldState) 
                 << "->" << static_cast<int>(state);
        
        emit connectionStateChanged(state);
    }
}

/**
 * @brief 更新网络质量
 */
void NetworkManagerImpl::updateNetworkQuality()
{
    INetworkManager::NetworkQuality newQuality;
    
    if (!isConnected()) {
        newQuality = INetworkManager::NetworkQuality::Unknown;
    } else if (d->latency < 100) {
        newQuality = INetworkManager::NetworkQuality::Excellent;
    } else if (d->latency < 200) {
        newQuality = INetworkManager::NetworkQuality::Good;
    } else if (d->latency < 500) {
        newQuality = INetworkManager::NetworkQuality::Fair;
    } else {
        newQuality = INetworkManager::NetworkQuality::Poor;
    }
    
    if (d->networkQuality != newQuality) {
        d->networkQuality = newQuality;
        emit networkQualityChanged(newQuality);
    }
}