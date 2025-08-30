#include "BaseConnectionHandler.h"
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QUuid>

class BaseConnectionHandler::Private
{
public:
    ConnectionStatus status;
    ConnectionType type;
    QString connectionId;
    QString remoteEndpoint;
    QString localEndpoint;
    int connectionTimeout;
    QVariantMap properties;
    QVariantMap config;
    QTimer* timeoutTimer;
    QMutex mutex;
    
    Private(ConnectionType connType)
        : status(Inactive)
        , type(connType)
        , connectionTimeout(30000)  // 30 seconds default
        , timeoutTimer(nullptr)
    {
        connectionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
};

BaseConnectionHandler::BaseConnectionHandler(ConnectionType type, QObject *parent)
    : IConnectionHandler(parent)
    , d(new Private(type))
{
    d->timeoutTimer = new QTimer(this);
    d->timeoutTimer->setSingleShot(true);
    connect(d->timeoutTimer, &QTimer::timeout, this, &BaseConnectionHandler::connectionTimeout);
}

BaseConnectionHandler::~BaseConnectionHandler()
{
    if (isConnected()) {
        closeConnection();
    }
    delete d;
}

bool BaseConnectionHandler::initialize(const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    
    d->config = config;
    
    // 设置连接超时
    if (config.contains("timeout")) {
        setConnectionTimeout(config["timeout"].toInt());
    }
    
    // 设置端点信息
    if (config.contains("remoteEndpoint")) {
        d->remoteEndpoint = config["remoteEndpoint"].toString();
    }
    
    if (config.contains("localEndpoint")) {
        d->localEndpoint = config["localEndpoint"].toString();
    }
    
    qDebug() << "BaseConnectionHandler: Initialized with config:" << config;
    return true;
}

bool BaseConnectionHandler::establishConnection(const QString& endpoint)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status == Connected || d->status == Connecting) {
        qWarning() << "BaseConnectionHandler: Already connected or connecting";
        return false;
    }
    
    if (endpoint.isEmpty()) {
        qWarning() << "BaseConnectionHandler: Empty endpoint provided";
        return false;
    }
    
    d->remoteEndpoint = endpoint;
    updateConnectionStatus(Connecting);
    
    // 启动超时定时器
    d->timeoutTimer->start(d->connectionTimeout);
    
    qDebug() << "BaseConnectionHandler: Establishing connection to" << endpoint;
    
    // 执行实际连接逻辑（子类实现）
    bool result = doEstablishConnection(endpoint);
    
    if (!result) {
        d->timeoutTimer->stop();
        updateConnectionStatus(Error);
    }
    
    return result;
}

void BaseConnectionHandler::closeConnection()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status == Disconnected || d->status == Inactive) {
        return;
    }
    
    qDebug() << "BaseConnectionHandler: Closing connection";
    
    updateConnectionStatus(Disconnecting);
    
    // 停止超时定时器
    d->timeoutTimer->stop();
    
    // 执行实际断开逻辑（子类实现）
    doCloseConnection();
    
    updateConnectionStatus(Disconnected);
}

bool BaseConnectionHandler::isConnected() const
{
    QMutexLocker locker(&d->mutex);
    return d->status == Connected;
}

IConnectionHandler::ConnectionStatus BaseConnectionHandler::connectionStatus() const
{
    QMutexLocker locker(&d->mutex);
    return d->status;
}

IConnectionHandler::ConnectionType BaseConnectionHandler::connectionType() const
{
    return d->type;
}

bool BaseConnectionHandler::sendData(const QByteArray& data)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Connected) {
        qWarning() << "BaseConnectionHandler: Cannot send data, not connected";
        return false;
    }
    
    if (data.isEmpty()) {
        qWarning() << "BaseConnectionHandler: Cannot send empty data";
        return false;
    }
    
    // 执行实际发送逻辑（子类实现）
    bool result = doSendData(data);
    
    if (result) {
        emit dataSent(data.size());
    }
    
    return result;
}

bool BaseConnectionHandler::sendText(const QString& text)
{
    return sendData(text.toUtf8());
}

QString BaseConnectionHandler::connectionId() const
{
    return d->connectionId;
}

QString BaseConnectionHandler::remoteEndpoint() const
{
    QMutexLocker locker(&d->mutex);
    return d->remoteEndpoint;
}

QString BaseConnectionHandler::localEndpoint() const
{
    QMutexLocker locker(&d->mutex);
    return d->localEndpoint;
}

void BaseConnectionHandler::setConnectionTimeout(int timeout)
{
    QMutexLocker locker(&d->mutex);
    
    if (timeout > 0) {
        d->connectionTimeout = timeout;
        qDebug() << "BaseConnectionHandler: Connection timeout set to" << timeout << "ms";
    }
}

int BaseConnectionHandler::connectionTimeout() const
{
    QMutexLocker locker(&d->mutex);
    return d->connectionTimeout;
}

QVariantMap BaseConnectionHandler::connectionStats() const
{
    QMutexLocker locker(&d->mutex);
    
    QVariantMap stats;
    stats["connectionId"] = d->connectionId;
    stats["status"] = static_cast<int>(d->status);
    stats["type"] = static_cast<int>(d->type);
    stats["remoteEndpoint"] = d->remoteEndpoint;
    stats["localEndpoint"] = d->localEndpoint;
    stats["timeout"] = d->connectionTimeout;
    
    return stats;
}

void BaseConnectionHandler::setProperty(const QString& key, const QVariant& value)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->properties.value(key) != value) {
        d->properties[key] = value;
        qDebug() << "BaseConnectionHandler: Property" << key << "set to" << value;
    }
}

QVariant BaseConnectionHandler::property(const QString& key) const
{
    QMutexLocker locker(&d->mutex);
    return d->properties.value(key);
}

void BaseConnectionHandler::reconnect()
{
    QMutexLocker locker(&d->mutex);
    
    QString endpoint = d->remoteEndpoint;
    locker.unlock();
    
    closeConnection();
    
    if (!endpoint.isEmpty()) {
        establishConnection(endpoint);
    } else {
        qWarning() << "BaseConnectionHandler: No endpoint for reconnection";
        emit connectionError("No endpoint for reconnection");
    }
}

void BaseConnectionHandler::refreshStatus()
{
    // 刷新连接状态（子类可以重写）
    QVariantMap stats = connectionStats();
    emit statsUpdated(stats);
}

void BaseConnectionHandler::updateConnectionStatus(ConnectionStatus status)
{
    if (d->status != status) {
        ConnectionStatus oldStatus = d->status;
        d->status = status;
        
        qDebug() << "BaseConnectionHandler: Status changed from" << oldStatus << "to" << status;
        
        emit connectionStatusChanged(status);
        
        // 发送特定状态信号
        switch (status) {
        case Connected:
            d->timeoutTimer->stop();
            emit connectionEstablished();
            break;
        case Disconnected:
            emit connectionClosed();
            break;
        case Error:
            d->timeoutTimer->stop();
            emit connectionError("Connection error occurred");
            break;
        default:
            break;
        }
    }
}

void BaseConnectionHandler::handleDataReceived(const QByteArray& data)
{
    emit dataReceived(data);
    
    // 尝试解析为文本
    QString text = QString::fromUtf8(data);
    if (!text.isEmpty()) {
        emit textReceived(text);
    }
}

bool BaseConnectionHandler::doEstablishConnection(const QString& endpoint)
{
    Q_UNUSED(endpoint)
    
    // 基类默认实现：模拟连接成功
    QTimer::singleShot(100, this, [this]() {
        updateConnectionStatus(Connected);
    });
    
    return true;
}

void BaseConnectionHandler::doCloseConnection()
{
    // 基类默认实现：无需特殊处理
    qDebug() << "BaseConnectionHandler: Default close connection implementation";
}

bool BaseConnectionHandler::doSendData(const QByteArray& data)
{
    Q_UNUSED(data)
    
    // 基类默认实现：假设发送成功
    qDebug() << "BaseConnectionHandler: Default send data implementation, size:" << data.size();
    return true;
}