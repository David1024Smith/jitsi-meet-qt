#include "NetworkManagerImpl.h"
#include "../include/ConnectionFactory.h"
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

class NetworkManagerImpl::Private
{
public:
    INetworkManager::ConnectionState connectionState;
    INetworkManager::NetworkQuality networkQuality;
    QVariantMap serverConfig;
    bool autoReconnectEnabled;
    int networkLatency;
    int bandwidth;
    QString currentServerUrl;
    
    QSharedPointer<IConnectionHandler> currentConnection;
    ConnectionFactory* connectionFactory;
    QMutex mutex;
    
    Private()
        : connectionState(INetworkManager::Disconnected)
        , networkQuality(INetworkManager::Unknown)
        , autoReconnectEnabled(true)
        , networkLatency(0)
        , bandwidth(0)
        , connectionFactory(nullptr)
    {
    }
};

NetworkManagerImpl::NetworkManagerImpl(QObject *parent)
    : INetworkManager(parent)
    , d(new Private)
{
    d->connectionFactory = ConnectionFactory::instance();
}

NetworkManagerImpl::~NetworkManagerImpl()
{
    if (isConnected()) {
        disconnect();
    }
    delete d;
}

bool NetworkManagerImpl::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    qDebug() << "NetworkManagerImpl: Initializing";
    
    // 初始化连接工厂
    if (!d->connectionFactory) {
        d->connectionFactory = ConnectionFactory::instance();
    }
    
    qDebug() << "NetworkManagerImpl: Initialized successfully";
    return true;
}

INetworkManager::ConnectionState NetworkManagerImpl::connectionState() const
{
    QMutexLocker locker(&d->mutex);
    return d->connectionState;
}

INetworkManager::NetworkQuality NetworkManagerImpl::networkQuality() const
{
    QMutexLocker locker(&d->mutex);
    return d->networkQuality;
}

bool NetworkManagerImpl::connectToServer(const QString& serverUrl)
{
    QMutexLocker locker(&d->mutex);
    
    QString url = serverUrl.isEmpty() ? d->serverConfig.value("serverUrl").toString() : serverUrl;
    
    if (url.isEmpty()) {
        qWarning() << "NetworkManagerImpl: No server URL provided";
        emit errorOccurred("No server URL provided");
        return false;
    }
    
    if (d->connectionState == Connected || d->connectionState == Connecting) {
        qWarning() << "NetworkManagerImpl: Already connected or connecting";
        return false;
    }
    
    qDebug() << "NetworkManagerImpl: Connecting to server:" << url;
    
    d->currentServerUrl = url;
    updateConnectionState(Connecting);
    
    // 创建连接
    QVariantMap config = d->serverConfig;
    config["serverUrl"] = url;
    
    // 根据URL协议选择连接类型
    ConnectionFactory::ConnectionType connType = ConnectionFactory::HTTP;
    if (url.startsWith("ws://") || url.startsWith("wss://")) {
        connType = ConnectionFactory::WebSocket;
    } else if (url.startsWith("https://")) {
        connType = ConnectionFactory::HTTPS;
    }
    
    d->currentConnection = d->connectionFactory->createConnection(connType, config);
    
    if (!d->currentConnection) {
        updateConnectionState(Error);
        emit errorOccurred("Failed to create connection");
        return false;
    }
    
    // 连接信号
    connect(d->currentConnection.data(), &IConnectionHandler::connectionEstablished,
            this, &NetworkManagerImpl::onConnectionEstablished);
    connect(d->currentConnection.data(), &IConnectionHandler::connectionClosed,
            this, &NetworkManagerImpl::onConnectionClosed);
    connect(d->currentConnection.data(), &IConnectionHandler::connectionError,
            this, &NetworkManagerImpl::onConnectionError);
    connect(d->currentConnection.data(), &IConnectionHandler::dataReceived,
            this, &NetworkManagerImpl::dataReceived);
    
    // 建立连接
    bool result = d->currentConnection->establishConnection(url);
    
    if (!result) {
        updateConnectionState(Error);
        emit errorOccurred("Failed to establish connection");
        d->currentConnection.reset();
    }
    
    return result;
}

void NetworkManagerImpl::disconnect()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->connectionState == Disconnected) {
        return;
    }
    
    qDebug() << "NetworkManagerImpl: Disconnecting...";
    
    if (d->currentConnection) {
        d->currentConnection->closeConnection();
        d->currentConnection.reset();
    }
    
    updateConnectionState(Disconnected);
}

bool NetworkManagerImpl::isConnected() const
{
    QMutexLocker locker(&d->mutex);
    return d->connectionState == Connected;
}

void NetworkManagerImpl::setServerConfiguration(const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->serverConfig != config) {
        d->serverConfig = config;
        qDebug() << "NetworkManagerImpl: Server configuration updated";
    }
}

QVariantMap NetworkManagerImpl::serverConfiguration() const
{
    QMutexLocker locker(&d->mutex);
    return d->serverConfig;
}

int NetworkManagerImpl::networkLatency() const
{
    QMutexLocker locker(&d->mutex);
    return d->networkLatency;
}

int NetworkManagerImpl::bandwidth() const
{
    QMutexLocker locker(&d->mutex);
    return d->bandwidth;
}

void NetworkManagerImpl::setAutoReconnectEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->autoReconnectEnabled != enabled) {
        d->autoReconnectEnabled = enabled;
        qDebug() << "NetworkManagerImpl: Auto-reconnect" << (enabled ? "enabled" : "disabled");
    }
}

bool NetworkManagerImpl::isAutoReconnectEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->autoReconnectEnabled;
}

void NetworkManagerImpl::reconnect()
{
    QMutexLocker locker(&d->mutex);
    
    qDebug() << "NetworkManagerImpl: Manual reconnect triggered";
    
    QString url = d->currentServerUrl;
    locker.unlock();
    
    if (isConnected()) {
        disconnect();
    }
    
    if (!url.isEmpty()) {
        emit reconnectStarted();
        connectToServer(url);
    } else {
        qWarning() << "NetworkManagerImpl: No server URL for reconnection";
        emit errorOccurred("No server URL for reconnection");
    }
}

void NetworkManagerImpl::refreshNetworkStatus()
{
    // 更新网络质量
    updateNetworkQuality();
    
    // 发送网络统计更新
    QVariantMap stats;
    stats["latency"] = d->networkLatency;
    stats["bandwidth"] = d->bandwidth;
    stats["quality"] = static_cast<int>(d->networkQuality);
    stats["connectionState"] = static_cast<int>(d->connectionState);
    
    if (d->currentConnection) {
        QVariantMap connStats = d->currentConnection->connectionStats();
        stats.unite(connStats);
    }
    
    emit networkStatsUpdated(stats);
}

void NetworkManagerImpl::onConnectionEstablished()
{
    QMutexLocker locker(&d->mutex);
    
    qDebug() << "NetworkManagerImpl: Connection established";
    updateConnectionState(Connected);
}

void NetworkManagerImpl::onConnectionClosed()
{
    QMutexLocker locker(&d->mutex);
    
    qDebug() << "NetworkManagerImpl: Connection closed";
    updateConnectionState(Disconnected);
    
    // 如果启用自动重连，尝试重连
    if (d->autoReconnectEnabled && !d->currentServerUrl.isEmpty()) {
        locker.unlock();
        QTimer::singleShot(5000, this, &NetworkManagerImpl::reconnect); // 5秒后重连
    }
}

void NetworkManagerImpl::onConnectionError(const QString& error)
{
    QMutexLocker locker(&d->mutex);
    
    qWarning() << "NetworkManagerImpl: Connection error:" << error;
    updateConnectionState(Error);
    emit errorOccurred(error);
    
    // 如果启用自动重连，尝试重连
    if (d->autoReconnectEnabled && !d->currentServerUrl.isEmpty()) {
        locker.unlock();
        QTimer::singleShot(5000, this, &NetworkManagerImpl::reconnect); // 5秒后重连
    }
}

void NetworkManagerImpl::updateConnectionState(INetworkManager::ConnectionState state)
{
    if (d->connectionState != state) {
        INetworkManager::ConnectionState oldState = d->connectionState;
        d->connectionState = state;
        
        qDebug() << "NetworkManagerImpl: Connection state changed from" << oldState << "to" << state;
        
        emit connectionStateChanged(state);
        
        // 发送特定状态信号
        switch (state) {
        case Connected:
            emit connected();
            break;
        case Disconnected:
            emit disconnected();
            break;
        case Reconnecting:
            emit reconnectStarted();
            break;
        default:
            break;
        }
    }
}

void NetworkManagerImpl::updateNetworkQuality()
{
    // 实现网络质量检测逻辑
    INetworkManager::NetworkQuality newQuality = Unknown;
    
    if (d->connectionState != Connected) {
        newQuality = Unknown;
    } else {
        // 基于延迟判断网络质量
        if (d->networkLatency == 0) {
            newQuality = Unknown;
        } else if (d->networkLatency < 50) {
            newQuality = Excellent;
        } else if (d->networkLatency < 100) {
            newQuality = Good;
        } else if (d->networkLatency < 200) {
            newQuality = Fair;
        } else {
            newQuality = Poor;
        }
    }
    
    if (d->networkQuality != newQuality) {
        d->networkQuality = newQuality;
        emit networkQualityChanged(newQuality);
    }
}