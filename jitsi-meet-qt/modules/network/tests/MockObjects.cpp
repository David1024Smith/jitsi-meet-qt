#include "NetworkModuleTest.h"
#include <QTimer>
#include <QUuid>

// MockNetworkManager 实现
NetworkModuleTest::MockNetworkManager::MockNetworkManager(QObject* parent)
    : INetworkManager(parent)
    , m_connectionState(Disconnected)
    , m_networkQuality(Unknown)
    , m_latency(0)
    , m_bandwidth(0)
    , m_autoReconnect(false)
    , m_initialized(false)
{
}

bool NetworkModuleTest::MockNetworkManager::initialize()
{
    m_initialized = true;
    return true;
}

INetworkManager::ConnectionState NetworkModuleTest::MockNetworkManager::connectionState() const
{
    return m_connectionState;
}

INetworkManager::NetworkQuality NetworkModuleTest::MockNetworkManager::networkQuality() const
{
    return m_networkQuality;
}

bool NetworkModuleTest::MockNetworkManager::connectToServer(const QString& serverUrl)
{
    if (serverUrl.isEmpty() || serverUrl.startsWith("invalid")) {
        setConnectionState(Error);
        emit errorOccurred("Invalid server URL");
        return false;
    }
    
    setConnectionState(Connecting);
    
    // 模拟异步连接
    QTimer::singleShot(100, this, [this]() {
        setConnectionState(Connected);
        emit connected();
    });
    
    return true;
}

void NetworkModuleTest::MockNetworkManager::disconnect()
{
    setConnectionState(Disconnected);
    emit disconnected();
}

bool NetworkModuleTest::MockNetworkManager::isConnected() const
{
    return m_connectionState == Connected;
}

void NetworkModuleTest::MockNetworkManager::setServerConfiguration(const QVariantMap& config)
{
    m_serverConfig = config;
}

QVariantMap NetworkModuleTest::MockNetworkManager::serverConfiguration() const
{
    return m_serverConfig;
}

int NetworkModuleTest::MockNetworkManager::networkLatency() const
{
    return m_latency;
}

int NetworkModuleTest::MockNetworkManager::bandwidth() const
{
    return m_bandwidth;
}

void NetworkModuleTest::MockNetworkManager::setAutoReconnectEnabled(bool enabled)
{
    m_autoReconnect = enabled;
}

bool NetworkModuleTest::MockNetworkManager::isAutoReconnectEnabled() const
{
    return m_autoReconnect;
}

void NetworkModuleTest::MockNetworkManager::reconnect()
{
    if (m_autoReconnect) {
        emit reconnectStarted();
        setConnectionState(Reconnecting);
        
        QTimer::singleShot(200, this, [this]() {
            setConnectionState(Connected);
            emit connected();
        });
    }
}

void NetworkModuleTest::MockNetworkManager::refreshNetworkStatus()
{
    // 模拟网络状态刷新
    emit networkStatsUpdated(QVariantMap());
}

void NetworkModuleTest::MockNetworkManager::setConnectionState(ConnectionState state)
{
    if (m_connectionState != state) {
        m_connectionState = state;
        emit connectionStateChanged(state);
    }
}

void NetworkModuleTest::MockNetworkManager::setNetworkQuality(NetworkQuality quality)
{
    if (m_networkQuality != quality) {
        m_networkQuality = quality;
        emit networkQualityChanged(quality);
    }
}

void NetworkModuleTest::MockNetworkManager::setLatency(int latency)
{
    m_latency = latency;
}

void NetworkModuleTest::MockNetworkManager::setBandwidth(int bandwidth)
{
    m_bandwidth = bandwidth;
}

void NetworkModuleTest::MockNetworkManager::simulateConnectionError(const QString& error)
{
    setConnectionState(Error);
    emit errorOccurred(error);
}

void NetworkModuleTest::MockNetworkManager::simulateDataReceived(const QByteArray& data)
{
    emit dataReceived(data);
}

// MockConnectionHandler 实现
NetworkModuleTest::MockConnectionHandler::MockConnectionHandler(QObject* parent)
    : IConnectionHandler(parent)
    , m_status(Inactive)
    , m_type(TCP)
    , m_connectionId(QUuid::createUuid().toString())
    , m_timeout(5000)
{
}

bool NetworkModuleTest::MockConnectionHandler::initialize(const QVariantMap& config)
{
    if (config.contains("timeout")) {
        m_timeout = config["timeout"].toInt();
    }
    return true;
}

bool NetworkModuleTest::MockConnectionHandler::establishConnection(const QString& endpoint)
{
    m_remoteEndpoint = endpoint;
    
    if (endpoint.isEmpty() || endpoint.startsWith("invalid")) {
        setConnectionStatus(Error);
        emit connectionError("Invalid endpoint");
        return false;
    }
    
    setConnectionStatus(Connecting);
    
    // 模拟异步连接
    QTimer::singleShot(50, this, [this]() {
        setConnectionStatus(Connected);
        emit connectionEstablished();
    });
    
    return true;
}

void NetworkModuleTest::MockConnectionHandler::closeConnection()
{
    setConnectionStatus(Disconnecting);
    
    QTimer::singleShot(10, this, [this]() {
        setConnectionStatus(Disconnected);
        emit connectionClosed();
    });
}

bool NetworkModuleTest::MockConnectionHandler::isConnected() const
{
    return m_status == Connected;
}

IConnectionHandler::ConnectionStatus NetworkModuleTest::MockConnectionHandler::connectionStatus() const
{
    return m_status;
}

IConnectionHandler::ConnectionType NetworkModuleTest::MockConnectionHandler::connectionType() const
{
    return m_type;
}

bool NetworkModuleTest::MockConnectionHandler::sendData(const QByteArray& data)
{
    if (!isConnected()) {
        return false;
    }
    
    // 模拟异步发送
    QTimer::singleShot(10, this, [this, data]() {
        emit dataSent(data.size());
    });
    
    return true;
}

bool NetworkModuleTest::MockConnectionHandler::sendText(const QString& text)
{
    return sendData(text.toUtf8());
}

QString NetworkModuleTest::MockConnectionHandler::connectionId() const
{
    return m_connectionId;
}

QString NetworkModuleTest::MockConnectionHandler::remoteEndpoint() const
{
    return m_remoteEndpoint;
}

QString NetworkModuleTest::MockConnectionHandler::localEndpoint() const
{
    return m_localEndpoint;
}

void NetworkModuleTest::MockConnectionHandler::setConnectionTimeout(int timeout)
{
    m_timeout = timeout;
}

int NetworkModuleTest::MockConnectionHandler::connectionTimeout() const
{
    return m_timeout;
}

QVariantMap NetworkModuleTest::MockConnectionHandler::connectionStats() const
{
    return m_stats;
}

void NetworkModuleTest::MockConnectionHandler::setProperty(const QString& key, const QVariant& value)
{
    m_properties[key] = value;
}

QVariant NetworkModuleTest::MockConnectionHandler::property(const QString& key) const
{
    return m_properties.value(key);
}

void NetworkModuleTest::MockConnectionHandler::reconnect()
{
    closeConnection();
    
    QTimer::singleShot(100, this, [this]() {
        establishConnection(m_remoteEndpoint);
    });
}

void NetworkModuleTest::MockConnectionHandler::refreshStatus()
{
    emit statsUpdated(m_stats);
}

void NetworkModuleTest::MockConnectionHandler::setConnectionStatus(ConnectionStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit connectionStatusChanged(status);
    }
}

void NetworkModuleTest::MockConnectionHandler::setConnectionType(ConnectionType type)
{
    m_type = type;
}

void NetworkModuleTest::MockConnectionHandler::simulateDataReceived(const QByteArray& data)
{
    emit dataReceived(data);
    emit textReceived(QString::fromUtf8(data));
}

void NetworkModuleTest::MockConnectionHandler::simulateConnectionError(const QString& error)
{
    setConnectionStatus(Error);
    emit connectionError(error);
}

// MockProtocolHandler 实现
NetworkModuleTest::MockProtocolHandler::MockProtocolHandler(QObject* parent)
    : IProtocolHandler(parent)
    , m_status(Inactive)
    , m_protocolName("MockProtocol")
    , m_protocolVersion("1.0")
{
    m_supportedFeatures << "mock_feature1" << "mock_feature2";
}

bool NetworkModuleTest::MockProtocolHandler::initialize(const QVariantMap& config)
{
    Q_UNUSED(config)
    m_status = Initializing;
    
    QTimer::singleShot(10, this, [this]() {
        setProtocolStatus(Active);
    });
    
    return true;
}

bool NetworkModuleTest::MockProtocolHandler::start()
{
    if (m_status == Inactive) {
        return false;
    }
    
    setProtocolStatus(Active);
    emit protocolStarted();
    return true;
}

void NetworkModuleTest::MockProtocolHandler::stop()
{
    setProtocolStatus(Shutdown);
    emit protocolStopped();
}

IProtocolHandler::ProtocolStatus NetworkModuleTest::MockProtocolHandler::protocolStatus() const
{
    return m_status;
}

QString NetworkModuleTest::MockProtocolHandler::protocolName() const
{
    return m_protocolName;
}

QString NetworkModuleTest::MockProtocolHandler::protocolVersion() const
{
    return m_protocolVersion;
}

QByteArray NetworkModuleTest::MockProtocolHandler::encodeMessage(MessageType type, const QVariantMap& data)
{
    QVariantMap message;
    message["type"] = static_cast<int>(type);
    message["data"] = data;
    
    QJsonDocument doc = QJsonDocument::fromVariant(message);
    return doc.toJson(QJsonDocument::Compact);
}

bool NetworkModuleTest::MockProtocolHandler::decodeMessage(const QByteArray& rawData, MessageType& type, QVariantMap& data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    QVariantMap message = doc.toVariant().toMap();
    type = static_cast<MessageType>(message["type"].toInt());
    data = message["data"].toMap();
    
    return true;
}

bool NetworkModuleTest::MockProtocolHandler::handleReceivedData(const QByteArray& data)
{
    MessageType type;
    QVariantMap messageData;
    
    if (decodeMessage(data, type, messageData)) {
        emit messageReceived(type, messageData);
        return true;
    }
    
    emit protocolError("Failed to decode message");
    return false;
}

bool NetworkModuleTest::MockProtocolHandler::sendMessage(MessageType type, const QVariantMap& data)
{
    if (m_status != Active) {
        return false;
    }
    
    QByteArray encoded = encodeMessage(type, data);
    
    // 模拟异步发送
    QTimer::singleShot(5, this, [this, type, data]() {
        emit messageSent(type, data);
    });
    
    return true;
}

bool NetworkModuleTest::MockProtocolHandler::sendHeartbeat()
{
    if (m_status != Active) {
        return false;
    }
    
    QTimer::singleShot(5, this, [this]() {
        emit heartbeatSent();
    });
    
    return true;
}

bool NetworkModuleTest::MockProtocolHandler::supportsFeature(const QString& feature) const
{
    return m_supportedFeatures.contains(feature);
}

QStringList NetworkModuleTest::MockProtocolHandler::supportedFeatures() const
{
    return m_supportedFeatures;
}

void NetworkModuleTest::MockProtocolHandler::setParameter(const QString& key, const QVariant& value)
{
    m_parameters[key] = value;
}

QVariant NetworkModuleTest::MockProtocolHandler::parameter(const QString& key) const
{
    return m_parameters.value(key);
}

QVariantMap NetworkModuleTest::MockProtocolHandler::protocolStats() const
{
    return m_stats;
}

void NetworkModuleTest::MockProtocolHandler::reset()
{
    setProtocolStatus(Inactive);
    m_parameters.clear();
    m_stats.clear();
}

void NetworkModuleTest::MockProtocolHandler::refresh()
{
    emit statsUpdated(m_stats);
}

void NetworkModuleTest::MockProtocolHandler::setProtocolStatus(ProtocolStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit protocolStatusChanged(status);
    }
}

void NetworkModuleTest::MockProtocolHandler::addSupportedFeature(const QString& feature)
{
    if (!m_supportedFeatures.contains(feature)) {
        m_supportedFeatures.append(feature);
    }
}

void NetworkModuleTest::MockProtocolHandler::simulateMessageReceived(MessageType type, const QVariantMap& data)
{
    emit messageReceived(type, data);
}

void NetworkModuleTest::MockProtocolHandler::simulateProtocolError(const QString& error)
{
    setProtocolStatus(Error);
    emit protocolError(error);
}