#ifndef NETWORKMODULETEST_H
#define NETWORKMODULETEST_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// 包含网络模块相关头文件
#include "../interfaces/INetworkManager.h"
#include "../interfaces/IConnectionHandler.h"
#include "../interfaces/IProtocolHandler.h"
#include "../include/NetworkManager.h"
#include "../include/ConnectionFactory.h"
#include "../protocols/WebRTCProtocol.h"
#include "../protocols/HTTPProtocol.h"
#include "../protocols/WebSocketProtocol.h"
#include "../utils/NetworkQualityMonitor.h"
#include "../config/NetworkConfig.h"

/**
 * @brief 网络模块测试类
 * 
 * NetworkModuleTest提供网络模块的完整测试套件，包括：
 * - 连接建立和断开测试
 * - 网络质量和延迟测试
 * - 协议处理器测试
 * - 与现有网络组件的兼容性测试
 */
class NetworkModuleTest : public QObject
{
    Q_OBJECT

public:
    explicit NetworkModuleTest(QObject *parent = nullptr);
    ~NetworkModuleTest();

private slots:
    // 测试框架生命周期
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基础功能测试
    void testNetworkManagerInitialization();
    void testNetworkManagerSingleton();
    void testConnectionStateManagement();
    void testServerConfiguration();
    void testAutoReconnectFeature();

    // 连接建立和断开测试
    void testConnectionEstablishment();
    void testConnectionEstablishment_data();
    void testConnectionDisconnection();
    void testConnectionTimeout();
    void testConnectionRetry();
    void testMultipleConnections();
    void testConnectionFailureHandling();

    // 网络质量和延迟测试
    void testNetworkQualityMonitoring();
    void testLatencyMeasurement();
    void testBandwidthMeasurement();
    void testPacketLossDetection();
    void testQualityThresholds();
    void testQualityHistoryTracking();
    void testNetworkDiagnostics();

    // 协议处理器测试
    void testWebRTCProtocolHandler();
    void testHTTPProtocolHandler();
    void testWebSocketProtocolHandler();
    void testProtocolMessageEncoding();
    void testProtocolMessageDecoding();
    void testProtocolHeartbeat();
    void testProtocolErrorHandling();
    void testProtocolFeatureSupport();

    // 连接工厂测试
    void testConnectionFactory();
    void testConnectionCreation();
    void testConnectionPooling();
    void testConnectionTypeSelection();

    // 配置管理测试
    void testNetworkConfiguration();
    void testConfigurationValidation();
    void testConfigurationPersistence();
    void testConfigurationDefaults();

    // 错误处理和恢复测试
    void testNetworkErrorHandling();
    void testConnectionRecovery();
    void testProtocolErrorRecovery();
    void testTimeoutHandling();

    // 性能测试
    void testConnectionPerformance();
    void testDataTransmissionPerformance();
    void testMemoryUsage();
    void testCPUUsage();

    // 兼容性测试
    void testLegacyNetworkManagerCompatibility();
    void testExistingComponentIntegration();
    void testAPIBackwardCompatibility();
    void testConfigurationMigration();

    // 并发和线程安全测试
    void testConcurrentConnections();
    void testThreadSafety();
    void testSignalSlotConnections();

    // 边界条件测试
    void testInvalidServerURL();
    void testNetworkUnavailable();
    void testLargeDataTransmission();
    void testRapidConnectionCycles();

private:
    // 测试辅助方法
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    bool waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
    QVariantMap createTestConfiguration();
    QString getTestServerURL();
    void simulateNetworkConditions(const QString& condition);
    void verifyConnectionState(INetworkManager::ConnectionState expectedState);
    void verifyNetworkQuality(INetworkManager::NetworkQuality expectedQuality);

    // 模拟对象创建
    class MockNetworkManager;
    class MockConnectionHandler;
    class MockProtocolHandler;
    
    MockNetworkManager* createMockNetworkManager();
    MockConnectionHandler* createMockConnectionHandler();
    MockProtocolHandler* createMockProtocolHandler();

    // 测试数据生成
    QByteArray generateTestData(int size);
    QVariantMap generateTestMessage(const QString& type);
    QStringList generateTestServerList();

    // 性能测量
    struct PerformanceMetrics {
        qint64 connectionTime;
        qint64 dataTransferTime;
        qint64 memoryUsage;
        double cpuUsage;
    };
    
    PerformanceMetrics measurePerformance(std::function<void()> testFunction);
    void logPerformanceMetrics(const PerformanceMetrics& metrics, const QString& testName);

private:
    NetworkManager* m_networkManager;
    NetworkQualityMonitor* m_qualityMonitor;
    NetworkConfig* m_networkConfig;
    ConnectionFactory* m_connectionFactory;
    
    WebRTCProtocol* m_webrtcProtocol;
    HTTPProtocol* m_httpProtocol;
    WebSocketProtocol* m_websocketProtocol;
    
    QTimer* m_testTimer;
    QNetworkAccessManager* m_testNetworkManager;
    
    QString m_testServerURL;
    int m_testTimeout;
    bool m_testEnvironmentReady;
    
    // 测试统计
    int m_totalTests;
    int m_passedTests;
    int m_failedTests;
    QStringList m_failedTestNames;
};

/**
 * @brief 模拟网络管理器类
 */
class NetworkModuleTest::MockNetworkManager : public INetworkManager
{
    Q_OBJECT

public:
    explicit MockNetworkManager(QObject* parent = nullptr);
    
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

public:
    // 测试辅助方法
    void setConnectionState(ConnectionState state);
    void setNetworkQuality(NetworkQuality quality);
    void setLatency(int latency);
    void setBandwidth(int bandwidth);
    void simulateConnectionError(const QString& error);
    void simulateDataReceived(const QByteArray& data);

private:
    ConnectionState m_connectionState;
    NetworkQuality m_networkQuality;
    QVariantMap m_serverConfig;
    int m_latency;
    int m_bandwidth;
    bool m_autoReconnect;
    bool m_initialized;
};

/**
 * @brief 模拟连接处理器类
 */
class NetworkModuleTest::MockConnectionHandler : public IConnectionHandler
{
    Q_OBJECT

public:
    explicit MockConnectionHandler(QObject* parent = nullptr);
    
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

public:
    // 测试辅助方法
    void setConnectionStatus(ConnectionStatus status);
    void setConnectionType(ConnectionType type);
    void simulateDataReceived(const QByteArray& data);
    void simulateConnectionError(const QString& error);

private:
    ConnectionStatus m_status;
    ConnectionType m_type;
    QString m_connectionId;
    QString m_remoteEndpoint;
    QString m_localEndpoint;
    int m_timeout;
    QVariantMap m_properties;
    QVariantMap m_stats;
};

/**
 * @brief 模拟协议处理器类
 */
class NetworkModuleTest::MockProtocolHandler : public IProtocolHandler
{
    Q_OBJECT

public:
    explicit MockProtocolHandler(QObject* parent = nullptr);
    
    // IProtocolHandler接口实现
    bool initialize(const QVariantMap& config = QVariantMap()) override;
    bool start() override;
    void stop() override;
    ProtocolStatus protocolStatus() const override;
    QString protocolName() const override;
    QString protocolVersion() const override;
    QByteArray encodeMessage(MessageType type, const QVariantMap& data) override;
    bool decodeMessage(const QByteArray& rawData, MessageType& type, QVariantMap& data) override;
    bool handleReceivedData(const QByteArray& data) override;
    bool sendMessage(MessageType type, const QVariantMap& data) override;
    bool sendHeartbeat() override;
    bool supportsFeature(const QString& feature) const override;
    QStringList supportedFeatures() const override;
    void setParameter(const QString& key, const QVariant& value) override;
    QVariant parameter(const QString& key) const override;
    QVariantMap protocolStats() const override;

public slots:
    void reset() override;
    void refresh() override;

public:
    // 测试辅助方法
    void setProtocolStatus(ProtocolStatus status);
    void addSupportedFeature(const QString& feature);
    void simulateMessageReceived(MessageType type, const QVariantMap& data);
    void simulateProtocolError(const QString& error);

private:
    ProtocolStatus m_status;
    QString m_protocolName;
    QString m_protocolVersion;
    QStringList m_supportedFeatures;
    QVariantMap m_parameters;
    QVariantMap m_stats;
};

#endif // NETWORKMODULETEST_H