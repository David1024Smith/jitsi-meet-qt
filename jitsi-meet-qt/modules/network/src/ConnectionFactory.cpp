#include "../include/ConnectionFactory.h"
#include "../interfaces/IConnectionHandler.h"
#include "../interfaces/IProtocolHandler.h"
#include "BaseConnectionHandler.h"
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QUuid>
#include <QUrl>
#include <functional>

// 前向声明具体实现类
class WebRTCConnectionHandler;
class HTTPConnectionHandler;
class WebSocketConnectionHandler;
class XMPPConnectionHandler;

class ConnectionFactory::Private
{
public:
    QMap<QString, std::function<QSharedPointer<IConnectionHandler>(const QVariantMap&)>> connectionCreators;
    QMap<QString, std::function<QSharedPointer<IProtocolHandler>(const QVariantMap&)>> protocolCreators;
    QMap<QString, QSharedPointer<IConnectionHandler>> activeConnections;
    QMutex mutex;
    
    Private() {
        // 初始化默认连接类型
        initializeDefaultCreators();
    }
    
    void initializeDefaultCreators() {
        // 注册默认连接类型创建函数
        connectionCreators["webrtc"] = [](const QVariantMap& config) -> QSharedPointer<IConnectionHandler> {
            auto handler = QSharedPointer<BaseConnectionHandler>::create(IConnectionHandler::WebRTC);
            handler->initialize(config);
            return handler;
        };
        
        connectionCreators["http"] = [](const QVariantMap& config) -> QSharedPointer<IConnectionHandler> {
            auto handler = QSharedPointer<BaseConnectionHandler>::create(IConnectionHandler::HTTP);
            handler->initialize(config);
            return handler;
        };
        
        connectionCreators["https"] = [](const QVariantMap& config) -> QSharedPointer<IConnectionHandler> {
            auto handler = QSharedPointer<BaseConnectionHandler>::create(IConnectionHandler::HTTP);
            handler->initialize(config);
            return handler;
        };
        
        connectionCreators["websocket"] = [](const QVariantMap& config) -> QSharedPointer<IConnectionHandler> {
            auto handler = QSharedPointer<BaseConnectionHandler>::create(IConnectionHandler::WebSocket);
            handler->initialize(config);
            return handler;
        };
        
        connectionCreators["websocket_secure"] = [](const QVariantMap& config) -> QSharedPointer<IConnectionHandler> {
            auto handler = QSharedPointer<BaseConnectionHandler>::create(IConnectionHandler::WebSocket);
            handler->initialize(config);
            return handler;
        };
        
        connectionCreators["xmpp"] = [](const QVariantMap& config) -> QSharedPointer<IConnectionHandler> {
            auto handler = QSharedPointer<BaseConnectionHandler>::create(IConnectionHandler::Custom);
            handler->initialize(config);
            return handler;
        };
    }
};

// 静态实例指针
static ConnectionFactory* s_instance = nullptr;
static QMutex s_instanceMutex;

ConnectionFactory::ConnectionFactory(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

ConnectionFactory::~ConnectionFactory()
{
    cleanupConnections();
    delete d;
}

ConnectionFactory* ConnectionFactory::instance()
{
    QMutexLocker locker(&s_instanceMutex);
    if (!s_instance) {
        s_instance = new ConnectionFactory();
    }
    return s_instance;
}

QSharedPointer<IConnectionHandler> ConnectionFactory::createConnection(
    ConnectionType type, 
    const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    
    QString typeName;
    switch (type) {
    case WebRTC:
        typeName = "webrtc";
        break;
    case HTTP:
        typeName = "http";
        break;
    case HTTPS:
        typeName = "https";
        break;
    case WebSocket:
        typeName = "websocket";
        break;
    case WebSocketSecure:
        typeName = "websocket_secure";
        break;
    case XMPP:
        typeName = "xmpp";
        break;
    case Custom:
        typeName = config.value("customType").toString();
        break;
    default:
        qWarning() << "ConnectionFactory: Unknown connection type:" << type;
        emit errorOccurred("Unknown connection type");
        return nullptr;
    }
    
    if (!d->connectionCreators.contains(typeName)) {
        qWarning() << "ConnectionFactory: No creator for connection type:" << typeName;
        emit errorOccurred(QString("No creator for connection type: %1").arg(typeName));
        return nullptr;
    }
    
    // 验证配置
    if (!validateConfiguration(type, config)) {
        qWarning() << "ConnectionFactory: Invalid configuration for type:" << typeName;
        emit errorOccurred("Invalid configuration");
        return nullptr;
    }
    
    try {
        // 创建连接
        auto creator = d->connectionCreators[typeName];
        QSharedPointer<IConnectionHandler> connection = creator(config);
        
        if (connection) {
            QString connectionId = generateConnectionId(type);
            d->activeConnections[connectionId] = connection;
            
            qDebug() << "ConnectionFactory: Created connection" << connectionId << "of type" << typeName;
            emit connectionCreated(type, connectionId);
            
            return connection;
        } else {
            qWarning() << "ConnectionFactory: Failed to create connection of type:" << typeName;
            emit errorOccurred(QString("Failed to create connection of type: %1").arg(typeName));
            return nullptr;
        }
        
    } catch (const std::exception& e) {
        qCritical() << "ConnectionFactory: Exception creating connection:" << e.what();
        emit errorOccurred(QString("Exception creating connection: %1").arg(e.what()));
        return nullptr;
    }
}

QSharedPointer<IProtocolHandler> ConnectionFactory::createProtocolHandler(
    const QString& protocol,
    const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->protocolCreators.contains(protocol)) {
        qWarning() << "ConnectionFactory: No creator for protocol:" << protocol;
        emit errorOccurred(QString("No creator for protocol: %1").arg(protocol));
        return nullptr;
    }
    
    try {
        auto creator = d->protocolCreators[protocol];
        QSharedPointer<IProtocolHandler> handler = creator(config);
        
        if (handler) {
            qDebug() << "ConnectionFactory: Created protocol handler for:" << protocol;
            return handler;
        } else {
            qWarning() << "ConnectionFactory: Failed to create protocol handler for:" << protocol;
            emit errorOccurred(QString("Failed to create protocol handler for: %1").arg(protocol));
            return nullptr;
        }
        
    } catch (const std::exception& e) {
        qCritical() << "ConnectionFactory: Exception creating protocol handler:" << e.what();
        emit errorOccurred(QString("Exception creating protocol handler: %1").arg(e.what()));
        return nullptr;
    }
}

bool ConnectionFactory::registerConnectionType(
    const QString& typeName,
    std::function<QSharedPointer<IConnectionHandler>(const QVariantMap&)> creator)
{
    QMutexLocker locker(&d->mutex);
    
    if (typeName.isEmpty() || !creator) {
        qWarning() << "ConnectionFactory: Invalid parameters for connection type registration";
        return false;
    }
    
    d->connectionCreators[typeName] = creator;
    qDebug() << "ConnectionFactory: Registered connection type:" << typeName;
    return true;
}

bool ConnectionFactory::registerProtocolHandler(
    const QString& protocolName,
    std::function<QSharedPointer<IProtocolHandler>(const QVariantMap&)> creator)
{
    QMutexLocker locker(&d->mutex);
    
    if (protocolName.isEmpty() || !creator) {
        qWarning() << "ConnectionFactory: Invalid parameters for protocol handler registration";
        return false;
    }
    
    d->protocolCreators[protocolName] = creator;
    qDebug() << "ConnectionFactory: Registered protocol handler:" << protocolName;
    return true;
}

QStringList ConnectionFactory::supportedConnectionTypes() const
{
    QMutexLocker locker(&d->mutex);
    return d->connectionCreators.keys();
}

QStringList ConnectionFactory::supportedProtocols() const
{
    QMutexLocker locker(&d->mutex);
    return d->protocolCreators.keys();
}

bool ConnectionFactory::isConnectionTypeSupported(ConnectionType type) const
{
    QString typeName;
    switch (type) {
    case WebRTC:
        typeName = "webrtc";
        break;
    case HTTP:
        typeName = "http";
        break;
    case HTTPS:
        typeName = "https";
        break;
    case WebSocket:
        typeName = "websocket";
        break;
    case WebSocketSecure:
        typeName = "websocket_secure";
        break;
    case XMPP:
        typeName = "xmpp";
        break;
    case Custom:
        return true; // Custom types are always "supported" if registered
    default:
        return false;
    }
    
    QMutexLocker locker(&d->mutex);
    return d->connectionCreators.contains(typeName);
}

bool ConnectionFactory::isProtocolSupported(const QString& protocol) const
{
    QMutexLocker locker(&d->mutex);
    return d->protocolCreators.contains(protocol);
}

QVariantMap ConnectionFactory::getDefaultConfiguration(ConnectionType type) const
{
    QVariantMap config;
    
    switch (type) {
    case WebRTC:
        config["iceServers"] = QVariantList();
        config["enableAudio"] = true;
        config["enableVideo"] = true;
        break;
    case HTTP:
    case HTTPS:
        config["timeout"] = 30000;
        config["followRedirects"] = true;
        config["maxRedirects"] = 5;
        break;
    case WebSocket:
    case WebSocketSecure:
        config["timeout"] = 30000;
        config["pingInterval"] = 30000;
        config["pongTimeout"] = 10000;
        break;
    case XMPP:
        config["resource"] = "jitsi-meet-qt";
        config["priority"] = 1;
        config["keepAlive"] = true;
        break;
    case Custom:
        // Custom types should provide their own defaults
        break;
    }
    
    return config;
}

bool ConnectionFactory::validateConfiguration(ConnectionType type, const QVariantMap& config) const
{
    switch (type) {
    case WebRTC:
        // WebRTC配置验证
        if (config.contains("iceServers")) {
            QVariantList iceServers = config["iceServers"].toList();
            // 验证ICE服务器配置
        }
        break;
        
    case HTTP:
    case HTTPS:
        // HTTP配置验证
        if (config.contains("timeout")) {
            int timeout = config["timeout"].toInt();
            if (timeout <= 0) {
                qWarning() << "ConnectionFactory: Invalid timeout value:" << timeout;
                return false;
            }
        }
        break;
        
    case WebSocket:
    case WebSocketSecure:
        // WebSocket配置验证
        if (config.contains("url")) {
            QUrl url(config["url"].toString());
            if (!url.isValid()) {
                qWarning() << "ConnectionFactory: Invalid WebSocket URL";
                return false;
            }
        }
        break;
        
    case XMPP:
        // XMPP配置验证
        if (config.contains("server")) {
            QString server = config["server"].toString();
            if (server.isEmpty()) {
                qWarning() << "ConnectionFactory: Empty XMPP server";
                return false;
            }
        }
        break;
        
    case Custom:
        // Custom types should implement their own validation
        break;
    }
    
    return true;
}

void ConnectionFactory::cleanupConnections()
{
    QMutexLocker locker(&d->mutex);
    
    qDebug() << "ConnectionFactory: Cleaning up" << d->activeConnections.size() << "connections";
    
    for (auto it = d->activeConnections.begin(); it != d->activeConnections.end(); ++it) {
        QString connectionId = it.key();
        QSharedPointer<IConnectionHandler> connection = it.value();
        
        if (connection && connection->isConnected()) {
            connection->closeConnection();
        }
        
        emit connectionDestroyed(connectionId);
    }
    
    d->activeConnections.clear();
    qDebug() << "ConnectionFactory: All connections cleaned up";
}

void ConnectionFactory::reset()
{
    QMutexLocker locker(&d->mutex);
    
    qDebug() << "ConnectionFactory: Resetting factory";
    
    // 清理所有连接
    cleanupConnections();
    
    // 重置创建器到默认状态
    d->connectionCreators.clear();
    d->protocolCreators.clear();
    d->initializeDefaultCreators();
    
    qDebug() << "ConnectionFactory: Factory reset completed";
}

QSharedPointer<IConnectionHandler> ConnectionFactory::createWebRTCConnection(const QVariantMap& config)
{
    qDebug() << "ConnectionFactory: Creating WebRTC connection";
    auto handler = QSharedPointer<BaseConnectionHandler>::create(IConnectionHandler::WebRTC);
    handler->initialize(config);
    return handler;
}

QSharedPointer<IConnectionHandler> ConnectionFactory::createHTTPConnection(const QVariantMap& config)
{
    qDebug() << "ConnectionFactory: Creating HTTP connection";
    auto handler = QSharedPointer<BaseConnectionHandler>::create(IConnectionHandler::HTTP);
    handler->initialize(config);
    return handler;
}

QSharedPointer<IConnectionHandler> ConnectionFactory::createWebSocketConnection(const QVariantMap& config)
{
    qDebug() << "ConnectionFactory: Creating WebSocket connection";
    auto handler = QSharedPointer<BaseConnectionHandler>::create(IConnectionHandler::WebSocket);
    handler->initialize(config);
    return handler;
}

QSharedPointer<IConnectionHandler> ConnectionFactory::createXMPPConnection(const QVariantMap& config)
{
    qDebug() << "ConnectionFactory: Creating XMPP connection";
    auto handler = QSharedPointer<BaseConnectionHandler>::create(IConnectionHandler::Custom);
    handler->initialize(config);
    return handler;
}

void ConnectionFactory::initializeDefaultTypes()
{
    // 这个方法在Private构造函数中已经调用
    // 这里保留作为公共接口，以防需要重新初始化
    d->initializeDefaultCreators();
}

QString ConnectionFactory::generateConnectionId(ConnectionType type)
{
    QString prefix;
    switch (type) {
    case WebRTC:
        prefix = "webrtc";
        break;
    case HTTP:
        prefix = "http";
        break;
    case HTTPS:
        prefix = "https";
        break;
    case WebSocket:
        prefix = "ws";
        break;
    case WebSocketSecure:
        prefix = "wss";
        break;
    case XMPP:
        prefix = "xmpp";
        break;
    case Custom:
        prefix = "custom";
        break;
    }
    
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return QString("%1_%2").arg(prefix, uuid);
}