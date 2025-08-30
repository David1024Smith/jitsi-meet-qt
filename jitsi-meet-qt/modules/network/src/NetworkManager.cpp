#include "../include/NetworkManager.h"
#include "../interfaces/INetworkManager.h"
#include "../config/NetworkConfig.h"
#include "NetworkManagerImpl.h"
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QHostInfo>
#include <QNetworkInterface>

class NetworkManager::Private
{
public:
    NetworkManagerImpl* impl;
    int reconnectInterval;
    QTimer* reconnectTimer;
    QTimer* networkCheckTimer;
    QMutex mutex;
    
    Private()
        : impl(nullptr)
        , reconnectInterval(5000)  // 5 seconds
        , reconnectTimer(nullptr)
        , networkCheckTimer(nullptr)
    {
    }
};

// 静态实例指针
static NetworkManager* s_instance = nullptr;
static QMutex s_instanceMutex;

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    // 创建实现类
    d->impl = new NetworkManagerImpl(this);
    
    // 连接信号
    connect(d->impl, &INetworkManager::connectionStateChanged,
            this, &NetworkManager::connectionStateChanged);
    connect(d->impl, &INetworkManager::networkQualityChanged,
            this, &NetworkManager::networkQualityChanged);
    connect(d->impl, &INetworkManager::dataReceived,
            this, &NetworkManager::dataReceived);
    connect(d->impl, &INetworkManager::dataSent,
            this, &NetworkManager::dataSent);
    connect(d->impl, &INetworkManager::errorOccurred,
            this, &NetworkManager::errorOccurred);
    connect(d->impl, &INetworkManager::connected,
            this, &NetworkManager::connected);
    connect(d->impl, &INetworkManager::disconnected,
            this, &NetworkManager::disconnected);
    connect(d->impl, &INetworkManager::reconnectStarted,
            this, &NetworkManager::reconnectStarted);
    connect(d->impl, &INetworkManager::networkStatsUpdated,
            this, &NetworkManager::networkStatsUpdated);
    
    // 初始化定时器
    d->reconnectTimer = new QTimer(this);
    d->reconnectTimer->setSingleShot(true);
    connect(d->reconnectTimer, &QTimer::timeout, this, &NetworkManager::handleReconnectTimer);
    
    d->networkCheckTimer = new QTimer(this);
    d->networkCheckTimer->setInterval(10000); // 10 seconds
    connect(d->networkCheckTimer, &QTimer::timeout, this, &NetworkManager::handleNetworkCheck);
}

NetworkManager::~NetworkManager()
{
    if (isConnected()) {
        disconnect();
    }
    delete d;
}

NetworkManager* NetworkManager::instance()
{
    QMutexLocker locker(&s_instanceMutex);
    if (!s_instance) {
        s_instance = new NetworkManager();
    }
    return s_instance;
}

bool NetworkManager::initialize(const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    
    qDebug() << "NetworkManager: Initializing with config:" << config;
    
    // 初始化实现类
    bool result = d->impl->initialize();
    
    // 设置配置
    if (!config.isEmpty()) {
        d->impl->setServerConfiguration(config);
    }
    
    // 启动网络状态检查定时器
    d->networkCheckTimer->start();
    
    qDebug() << "NetworkManager: Initialized successfully";
    return result;
}

NetworkManager::ConnectionState NetworkManager::connectionState() const
{
    QMutexLocker locker(&d->mutex);
    return static_cast<ConnectionState>(d->impl->connectionState());
}

NetworkManager::NetworkQuality NetworkManager::networkQuality() const
{
    QMutexLocker locker(&d->mutex);
    return static_cast<NetworkQuality>(d->impl->networkQuality());
}

bool NetworkManager::connectToServer(const QString& serverUrl)
{
    QMutexLocker locker(&d->mutex);
    return d->impl->connectToServer(serverUrl);
}

void NetworkManager::disconnect()
{
    QMutexLocker locker(&d->mutex);
    
    // 停止定时器
    if (d->reconnectTimer->isActive()) {
        d->reconnectTimer->stop();
    }
    
    d->impl->disconnect();
}

bool NetworkManager::isConnected() const
{
    QMutexLocker locker(&d->mutex);
    return d->impl->isConnected();
}

void NetworkManager::setServerConfiguration(const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    d->impl->setServerConfiguration(config);
}

QVariantMap NetworkManager::serverConfiguration() const
{
    QMutexLocker locker(&d->mutex);
    return d->impl->serverConfiguration();
}

void NetworkManager::setAutoReconnectEnabled(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    d->impl->setAutoReconnectEnabled(enabled);
    
    if (!enabled && d->reconnectTimer->isActive()) {
        d->reconnectTimer->stop();
    }
}

bool NetworkManager::isAutoReconnectEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->impl->isAutoReconnectEnabled();
}

void NetworkManager::setReconnectInterval(int interval)
{
    QMutexLocker locker(&d->mutex);
    
    if (interval > 0 && d->reconnectInterval != interval) {
        d->reconnectInterval = interval;
        d->reconnectTimer->setInterval(interval);
        qDebug() << "NetworkManager: Reconnect interval set to" << interval << "ms";
    }
}

int NetworkManager::reconnectInterval() const
{
    QMutexLocker locker(&d->mutex);
    return d->reconnectInterval;
}

int NetworkManager::networkLatency() const
{
    QMutexLocker locker(&d->mutex);
    return d->impl->networkLatency();
}

int NetworkManager::bandwidth() const
{
    QMutexLocker locker(&d->mutex);
    return d->impl->bandwidth();
}

void NetworkManager::reconnect()
{
    QMutexLocker locker(&d->mutex);
    d->impl->reconnect();
}

void NetworkManager::refreshNetworkStatus()
{
    QMutexLocker locker(&d->mutex);
    d->impl->refreshNetworkStatus();
}

void NetworkManager::handleConnectionTimeout()
{
    // 连接超时处理已由实现类处理
    qDebug() << "NetworkManager: Connection timeout handled by implementation";
}

void NetworkManager::handleReconnectTimer()
{
    QMutexLocker locker(&d->mutex);
    
    qDebug() << "NetworkManager: Attempting automatic reconnection";
    d->impl->reconnect();
}

void NetworkManager::handleNetworkCheck()
{
    QMutexLocker locker(&d->mutex);
    d->impl->refreshNetworkStatus();
}

void NetworkManager::handleNetworkError(const QString& error)
{
    Q_UNUSED(error)
    // 错误处理已由实现类处理
    qDebug() << "NetworkManager: Network error handled by implementation";
}

// 这些方法已由NetworkManagerImpl处理，不再需要