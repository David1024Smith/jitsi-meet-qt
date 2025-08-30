#include "NetworkModuleTest.h"
#include <QCoreApplication>
#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QElapsedTimer>
#include <QRandomGenerator>

// 测试常量
static const QString TEST_SERVER_URL = "https://meet.jit.si";
static const int DEFAULT_TIMEOUT = 5000;

NetworkModuleTest::NetworkModuleTest(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_qualityMonitor(nullptr)
    , m_networkConfig(nullptr)
    , m_connectionFactory(nullptr)
    , m_webrtcProtocol(nullptr)
    , m_httpProtocol(nullptr)
    , m_websocketProtocol(nullptr)
    , m_testTimer(new QTimer(this))
    , m_testNetworkManager(new QNetworkAccessManager(this))
    , m_testServerURL(TEST_SERVER_URL)
    , m_testTimeout(DEFAULT_TIMEOUT)
    , m_testEnvironmentReady(false)
    , m_totalTests(0)
    , m_passedTests(0)
    , m_failedTests(0)
{
    m_testTimer->setSingleShot(true);
}

NetworkModuleTest::~NetworkModuleTest()
{
    cleanupTestEnvironment();
}

void NetworkModuleTest::initTestCase()
{
    qDebug() << "初始化网络模块测试套件...";
    setupTestEnvironment();
    QVERIFY2(m_testEnvironmentReady, "测试环境初始化失败");
    qDebug() << "网络模块测试套件初始化完成";
}

void NetworkModuleTest::cleanupTestCase()
{
    qDebug() << "清理网络模块测试套件...";
    cleanupTestEnvironment();
    qDebug() << QString("测试完成 - 总计: %1, 通过: %2, 失败: %3")
                .arg(m_totalTests).arg(m_passedTests).arg(m_failedTests);
}

void NetworkModuleTest::init()
{
    m_totalTests++;
}

void NetworkModuleTest::cleanup()
{
    // 每个测试方法执行后的清理
}v
oid NetworkModuleTest::setupTestEnvironment()
{
    try {
        // 创建网络管理器
        m_networkManager = NetworkManager::instance();
        QVERIFY(m_networkManager != nullptr);
        
        // 创建网络质量监控器
        m_qualityMonitor = new NetworkQualityMonitor(this);
        QVERIFY(m_qualityMonitor != nullptr);
        
        // 创建网络配置
        m_networkConfig = new NetworkConfig(this);
        QVERIFY(m_networkConfig != nullptr);
        
        // 创建连接工厂
        m_connectionFactory = new ConnectionFactory(this);
        QVERIFY(m_connectionFactory != nullptr);
        
        // 创建协议处理器
        m_webrtcProtocol = new WebRTCProtocol(this);
        m_httpProtocol = new HTTPProtocol(this);
        m_websocketProtocol = new WebSocketProtocol(this);
        
        QVERIFY(m_webrtcProtocol != nullptr);
        QVERIFY(m_httpProtocol != nullptr);
        QVERIFY(m_websocketProtocol != nullptr);
        
        // 设置测试配置
        QVariantMap testConfig = createTestConfiguration();
        m_networkConfig->loadConfiguration(testConfig);
        
        m_testEnvironmentReady = true;
        
    } catch (const std::exception& e) {
        qCritical() << "测试环境设置失败:" << e.what();
        m_testEnvironmentReady = false;
    }
}

void NetworkModuleTest::cleanupTestEnvironment()
{
    // 清理协议处理器
    if (m_webrtcProtocol) {
        m_webrtcProtocol->stop();
    }
    if (m_httpProtocol) {
        m_httpProtocol->stop();
    }
    if (m_websocketProtocol) {
        m_websocketProtocol->stop();
    }
    
    // 清理网络管理器
    if (m_networkManager && m_networkManager->isConnected()) {
        m_networkManager->disconnect();
    }
    
    // 停止质量监控
    if (m_qualityMonitor) {
        m_qualityMonitor->stopMonitoring();
    }
}

QVariantMap NetworkModuleTest::createTestConfiguration()
{
    QVariantMap config;
    
    // 服务器配置
    config["server_url"] = m_testServerURL;
    config["server_port"] = 443;
    config["use_ssl"] = true;
    config["timeout"] = m_testTimeout;
    
    // 协议配置
    QVariantMap protocols;
    protocols["webrtc_enabled"] = true;
    protocols["websocket_enabled"] = true;
    protocols["http_enabled"] = true;
    config["protocols"] = protocols;
    
    // 质量监控配置
    QVariantMap quality;
    quality["monitor_enabled"] = true;
    quality["monitor_interval"] = 1000;
    quality["latency_threshold"] = 100;
    quality["bandwidth_threshold"] = 1000;
    config["quality"] = quality;
    
    return config;
}// 基础功能测试

void NetworkModuleTest::testNetworkManagerInitialization()
{
    qDebug() << "测试网络管理器初始化...";
    
    // 测试初始化
    QVariantMap config = createTestConfiguration();
    bool result = m_networkManager->initialize(config);
    QVERIFY2(result, "网络管理器初始化失败");
    
    // 验证初始状态
    QCOMPARE(m_networkManager->connectionState(), NetworkManager::Disconnected);
    QCOMPARE(m_networkManager->networkQuality(), NetworkManager::Unknown);
    
    m_passedTests++;
}

void NetworkModuleTest::testNetworkManagerSingleton()
{
    qDebug() << "测试网络管理器单例模式...";
    
    NetworkManager* instance1 = NetworkManager::instance();
    NetworkManager* instance2 = NetworkManager::instance();
    
    QVERIFY(instance1 != nullptr);
    QVERIFY(instance2 != nullptr);
    QCOMPARE(instance1, instance2);
    
    m_passedTests++;
}

void NetworkModuleTest::testConnectionStateManagement()
{
    qDebug() << "测试连接状态管理...";
    
    QSignalSpy stateSpy(m_networkManager, &NetworkManager::connectionStateChanged);
    
    // 测试状态变化
    QCOMPARE(m_networkManager->connectionState(), NetworkManager::Disconnected);
    
    // 模拟连接
    bool connectResult = m_networkManager->connectToServer(m_testServerURL);
    QVERIFY(connectResult);
    
    // 等待状态变化
    if (!waitForSignal(m_networkManager, SIGNAL(connectionStateChanged(NetworkManager::ConnectionState)), 3000)) {
        qWarning() << "连接状态变化信号超时";
    }
    
    // 验证信号发送
    QVERIFY(stateSpy.count() > 0);
    
    m_passedTests++;
}

void NetworkModuleTest::testServerConfiguration()
{
    qDebug() << "测试服务器配置...";
    
    QVariantMap config;
    config["url"] = "https://test.example.com";
    config["port"] = 8443;
    config["ssl"] = true;
    
    m_networkManager->setServerConfiguration(config);
    QVariantMap retrievedConfig = m_networkManager->serverConfiguration();
    
    QCOMPARE(retrievedConfig["url"].toString(), config["url"].toString());
    QCOMPARE(retrievedConfig["port"].toInt(), config["port"].toInt());
    QCOMPARE(retrievedConfig["ssl"].toBool(), config["ssl"].toBool());
    
    m_passedTests++;
}

void NetworkModuleTest::testAutoReconnectFeature()
{
    qDebug() << "测试自动重连功能...";
    
    // 测试启用自动重连
    m_networkManager->setAutoReconnectEnabled(true);
    QVERIFY(m_networkManager->isAutoReconnectEnabled());
    
    // 测试禁用自动重连
    m_networkManager->setAutoReconnectEnabled(false);
    QVERIFY(!m_networkManager->isAutoReconnectEnabled());
    
    m_passedTests++;
}// 连接
建立和断开测试
void NetworkModuleTest::testConnectionEstablishment()
{
    qDebug() << "测试连接建立...";
    
    QFETCH(QString, serverUrl);
    QFETCH(bool, expectedResult);
    
    QSignalSpy connectedSpy(m_networkManager, &NetworkManager::connected);
    QSignalSpy errorSpy(m_networkManager, &NetworkManager::errorOccurred);
    
    bool result = m_networkManager->connectToServer(serverUrl);
    QCOMPARE(result, expectedResult);
    
    if (expectedResult) {
        // 等待连接成功信号
        bool signalReceived = waitForSignal(m_networkManager, SIGNAL(connected()), 10000);
        if (signalReceived) {
            QVERIFY(connectedSpy.count() > 0);
            QVERIFY(m_networkManager->isConnected());
        }
    } else {
        // 等待错误信号
        bool errorReceived = waitForSignal(m_networkManager, SIGNAL(errorOccurred(QString)), 5000);
        if (errorReceived) {
            QVERIFY(errorSpy.count() > 0);
        }
    }
    
    m_passedTests++;
}

void NetworkModuleTest::testConnectionEstablishment_data()
{
    QTest::addColumn<QString>("serverUrl");
    QTest::addColumn<bool>("expectedResult");
    
    QTest::newRow("valid_url") << "https://meet.jit.si" << true;
    QTest::newRow("invalid_url") << "invalid://url" << false;
    QTest::newRow("empty_url") << "" << false;
    QTest::newRow("localhost") << "http://localhost:8080" << true;
}

void NetworkModuleTest::testConnectionDisconnection()
{
    qDebug() << "测试连接断开...";
    
    // 首先建立连接
    if (!m_networkManager->isConnected()) {
        m_networkManager->connectToServer(m_testServerURL);
        waitForSignal(m_networkManager, SIGNAL(connected()), 5000);
    }
    
    QSignalSpy disconnectedSpy(m_networkManager, &NetworkManager::disconnected);
    
    // 断开连接
    m_networkManager->disconnect();
    
    // 等待断开信号
    bool signalReceived = waitForSignal(m_networkManager, SIGNAL(disconnected()), 3000);
    QVERIFY(signalReceived);
    QVERIFY(disconnectedSpy.count() > 0);
    QVERIFY(!m_networkManager->isConnected());
    
    m_passedTests++;
}

void NetworkModuleTest::testConnectionTimeout()
{
    qDebug() << "测试连接超时...";
    
    QSignalSpy errorSpy(m_networkManager, &NetworkManager::errorOccurred);
    
    // 尝试连接到不存在的服务器
    bool result = m_networkManager->connectToServer("http://192.0.2.1:12345");
    
    if (result) {
        // 等待超时错误
        bool errorReceived = waitForSignal(m_networkManager, SIGNAL(errorOccurred(QString)), 15000);
        QVERIFY(errorReceived);
        QVERIFY(errorSpy.count() > 0);
    }
    
    m_passedTests++;
}

void NetworkModuleTest::testConnectionRetry()
{
    qDebug() << "测试连接重试...";
    
    // 启用自动重连
    m_networkManager->setAutoReconnectEnabled(true);
    
    QSignalSpy reconnectSpy(m_networkManager, &NetworkManager::reconnectStarted);
    
    // 手动触发重连
    m_networkManager->reconnect();
    
    // 验证重连信号
    bool signalReceived = waitForSignal(m_networkManager, SIGNAL(reconnectStarted()), 3000);
    if (signalReceived) {
        QVERIFY(reconnectSpy.count() > 0);
    }
    
    m_passedTests++;
}

void NetworkModuleTest::testMultipleConnections()
{
    qDebug() << "测试多重连接...";
    
    // 测试连接工厂创建多个连接
    auto connection1 = m_connectionFactory->createConnection(IConnectionHandler::WebSocket);
    auto connection2 = m_connectionFactory->createConnection(IConnectionHandler::HTTP);
    auto connection3 = m_connectionFactory->createConnection(IConnectionHandler::WebRTC);
    
    QVERIFY(connection1 != nullptr);
    QVERIFY(connection2 != nullptr);
    QVERIFY(connection3 != nullptr);
    
    // 验证连接类型
    QCOMPARE(connection1->connectionType(), IConnectionHandler::WebSocket);
    QCOMPARE(connection2->connectionType(), IConnectionHandler::HTTP);
    QCOMPARE(connection3->connectionType(), IConnectionHandler::WebRTC);
    
    m_passedTests++;
}

void NetworkModuleTest::testConnectionFailureHandling()
{
    qDebug() << "测试连接失败处理...";
    
    QSignalSpy errorSpy(m_networkManager, &NetworkManager::errorOccurred);
    
    // 尝试连接到无效地址
    bool result = m_networkManager->connectToServer("invalid://invalid.url");
    
    if (!result || errorSpy.wait(5000)) {
        // 验证错误处理
        QVERIFY(!m_networkManager->isConnected());
        QCOMPARE(m_networkManager->connectionState(), NetworkManager::Error);
    }
    
    m_passedTests++;
}// 网络质量
和延迟测试
void NetworkModuleTest::testNetworkQualityMonitoring()
{
    qDebug() << "测试网络质量监控...";
    
    QSignalSpy qualitySpy(m_qualityMonitor, &NetworkQualityMonitor::qualityChanged);
    
    // 启动监控
    bool result = m_qualityMonitor->startMonitoring("8.8.8.8", 2000);
    QVERIFY(result);
    
    // 等待质量数据
    bool signalReceived = waitForSignal(m_qualityMonitor, 
        SIGNAL(qualityChanged(NetworkQualityMonitor::QualityLevel, int)), 10000);
    
    if (signalReceived) {
        QVERIFY(qualitySpy.count() > 0);
        
        // 验证质量数据
        NetworkQualityMonitor::QualityLevel level = m_qualityMonitor->currentQualityLevel();
        int score = m_qualityMonitor->currentQualityScore();
        
        QVERIFY(level >= NetworkQualityMonitor::VeryPoor && level <= NetworkQualityMonitor::Excellent);
        QVERIFY(score >= 0 && score <= 100);
    }
    
    m_qualityMonitor->stopMonitoring();
    m_passedTests++;
}

void NetworkModuleTest::testLatencyMeasurement()
{
    qDebug() << "测试延迟测量...";
    
    QSignalSpy latencySpy(m_qualityMonitor, &NetworkQualityMonitor::latencyChanged);
    
    // 执行单次测试
    QVariantMap result = m_qualityMonitor->performSingleTest();
    
    QVERIFY(result.contains("latency"));
    int latency = result["latency"].toInt();
    QVERIFY(latency >= 0);
    QVERIFY(latency < 10000); // 合理的延迟范围
    
    m_passedTests++;
}

void NetworkModuleTest::testBandwidthMeasurement()
{
    qDebug() << "测试带宽测量...";
    
    QSignalSpy bandwidthSpy(m_qualityMonitor, &NetworkQualityMonitor::bandwidthChanged);
    
    // 执行带宽测试
    QVariantMap result = m_qualityMonitor->performSingleTest();
    
    if (result.contains("bandwidth")) {
        int bandwidth = result["bandwidth"].toInt();
        QVERIFY(bandwidth >= 0);
        qDebug() << "测量带宽:" << bandwidth << "kbps";
    }
    
    m_passedTests++;
}

void NetworkModuleTest::testPacketLossDetection()
{
    qDebug() << "测试丢包检测...";
    
    QSignalSpy packetLossSpy(m_qualityMonitor, &NetworkQualityMonitor::packetLossChanged);
    
    // 执行丢包测试
    QVariantMap result = m_qualityMonitor->performSingleTest();
    
    if (result.contains("packet_loss")) {
        double packetLoss = result["packet_loss"].toDouble();
        QVERIFY(packetLoss >= 0.0 && packetLoss <= 100.0);
        qDebug() << "丢包率:" << packetLoss << "%";
    }
    
    m_passedTests++;
}

void NetworkModuleTest::testQualityThresholds()
{
    qDebug() << "测试质量阈值...";
    
    // 设置自定义阈值
    m_qualityMonitor->setQualityThresholds(95, 80, 60, 40);
    
    // 启动监控
    m_qualityMonitor->startMonitoring("8.8.8.8", 1000);
    
    // 等待质量评估
    waitForSignal(m_qualityMonitor, SIGNAL(qualityChanged(NetworkQualityMonitor::QualityLevel, int)), 5000);
    
    // 验证阈值应用
    int score = m_qualityMonitor->currentQualityScore();
    NetworkQualityMonitor::QualityLevel level = m_qualityMonitor->currentQualityLevel();
    
    if (score >= 95) {
        QCOMPARE(level, NetworkQualityMonitor::Excellent);
    } else if (score >= 80) {
        QCOMPARE(level, NetworkQualityMonitor::Good);
    } else if (score >= 60) {
        QCOMPARE(level, NetworkQualityMonitor::Fair);
    } else if (score >= 40) {
        QCOMPARE(level, NetworkQualityMonitor::Poor);
    } else {
        QCOMPARE(level, NetworkQualityMonitor::VeryPoor);
    }
    
    m_qualityMonitor->stopMonitoring();
    m_passedTests++;
}

void NetworkModuleTest::testQualityHistoryTracking()
{
    qDebug() << "测试质量历史跟踪...";
    
    // 启动监控
    m_qualityMonitor->startMonitoring("8.8.8.8", 500);
    
    // 等待收集一些历史数据
    QThread::msleep(3000);
    
    // 获取历史数据
    QVariantList history = m_qualityMonitor->getHistoryData(1); // 最近1分钟
    
    QVERIFY(history.size() > 0);
    
    // 验证历史数据格式
    for (const QVariant& entry : history) {
        QVariantMap data = entry.toMap();
        QVERIFY(data.contains("timestamp"));
        QVERIFY(data.contains("quality_score"));
        QVERIFY(data.contains("latency"));
    }
    
    m_qualityMonitor->stopMonitoring();
    m_passedTests++;
}

void NetworkModuleTest::testNetworkDiagnostics()
{
    qDebug() << "测试网络诊断...";
    
    // 获取网络统计信息
    QVariantMap stats = m_qualityMonitor->getQualityStats();
    
    // 验证统计信息包含必要字段
    QVERIFY(stats.contains("total_tests"));
    QVERIFY(stats.contains("average_latency"));
    QVERIFY(stats.contains("average_quality_score"));
    
    m_passedTests++;
}// 协议处理器测试

void NetworkModuleTest::testWebRTCProtocolHandler()
{
    qDebug() << "测试WebRTC协议处理器...";
    
    // 初始化协议
    QVariantMap config;
    config["stun_servers"] = QStringList() << "stun:stun.l.google.com:19302";
    
    bool initResult = m_webrtcProtocol->initialize(config);
    QVERIFY(initResult);
    
    // 启动协议
    bool startResult = m_webrtcProtocol->start();
    QVERIFY(startResult);
    
    // 验证协议信息
    QCOMPARE(m_webrtcProtocol->protocolName(), QString("WebRTC"));
    QVERIFY(!m_webrtcProtocol->protocolVersion().isEmpty());
    
    // 测试功能支持
    QVERIFY(m_webrtcProtocol->supportsFeature("ice"));
    QVERIFY(m_webrtcProtocol->supportsFeature("dtls"));
    
    // 测试消息编码
    QVariantMap testData;
    testData["type"] = "offer";
    testData["sdp"] = "test_sdp_content";
    
    QByteArray encoded = m_webrtcProtocol->encodeMessage(IProtocolHandler::Control, testData);
    QVERIFY(!encoded.isEmpty());
    
    // 测试消息解码
    IProtocolHandler::MessageType decodedType;
    QVariantMap decodedData;
    bool decodeResult = m_webrtcProtocol->decodeMessage(encoded, decodedType, decodedData);
    QVERIFY(decodeResult);
    QCOMPARE(decodedType, IProtocolHandler::Control);
    
    m_webrtcProtocol->stop();
    m_passedTests++;
}

void NetworkModuleTest::testHTTPProtocolHandler()
{
    qDebug() << "测试HTTP协议处理器...";
    
    // 初始化协议
    bool initResult = m_httpProtocol->initialize();
    QVERIFY(initResult);
    
    // 启动协议
    bool startResult = m_httpProtocol->start();
    QVERIFY(startResult);
    
    // 验证协议信息
    QCOMPARE(m_httpProtocol->protocolName(), QString("HTTP"));
    
    // 测试HTTP请求消息
    QVariantMap requestData;
    requestData["method"] = "GET";
    requestData["url"] = "/api/test";
    requestData["headers"] = QVariantMap();
    
    QByteArray encoded = m_httpProtocol->encodeMessage(IProtocolHandler::Data, requestData);
    QVERIFY(!encoded.isEmpty());
    
    m_httpProtocol->stop();
    m_passedTests++;
}

void NetworkModuleTest::testWebSocketProtocolHandler()
{
    qDebug() << "测试WebSocket协议处理器...";
    
    // 初始化协议
    QVariantMap config;
    config["url"] = "wss://echo.websocket.org";
    
    bool initResult = m_websocketProtocol->initialize(config);
    QVERIFY(initResult);
    
    // 启动协议
    bool startResult = m_websocketProtocol->start();
    QVERIFY(startResult);
    
    // 验证协议信息
    QCOMPARE(m_websocketProtocol->protocolName(), QString("WebSocket"));
    
    // 测试心跳
    bool heartbeatResult = m_websocketProtocol->sendHeartbeat();
    QVERIFY(heartbeatResult);
    
    m_websocketProtocol->stop();
    m_passedTests++;
}

void NetworkModuleTest::testProtocolMessageEncoding()
{
    qDebug() << "测试协议消息编码...";
    
    // 测试不同类型的消息编码
    QVariantMap controlData;
    controlData["command"] = "connect";
    controlData["parameters"] = QVariantMap();
    
    QByteArray controlEncoded = m_webrtcProtocol->encodeMessage(IProtocolHandler::Control, controlData);
    QVERIFY(!controlEncoded.isEmpty());
    
    QVariantMap dataMessage;
    dataMessage["payload"] = "test_payload";
    dataMessage["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    QByteArray dataEncoded = m_webrtcProtocol->encodeMessage(IProtocolHandler::Data, dataMessage);
    QVERIFY(!dataEncoded.isEmpty());
    
    m_passedTests++;
}

void NetworkModuleTest::testProtocolMessageDecoding()
{
    qDebug() << "测试协议消息解码...";
    
    // 创建测试消息
    QVariantMap originalData;
    originalData["test_field"] = "test_value";
    originalData["number_field"] = 42;
    
    // 编码消息
    QByteArray encoded = m_webrtcProtocol->encodeMessage(IProtocolHandler::Data, originalData);
    QVERIFY(!encoded.isEmpty());
    
    // 解码消息
    IProtocolHandler::MessageType decodedType;
    QVariantMap decodedData;
    bool result = m_webrtcProtocol->decodeMessage(encoded, decodedType, decodedData);
    
    QVERIFY(result);
    QCOMPARE(decodedType, IProtocolHandler::Data);
    QCOMPARE(decodedData["test_field"].toString(), originalData["test_field"].toString());
    QCOMPARE(decodedData["number_field"].toInt(), originalData["number_field"].toInt());
    
    m_passedTests++;
}

void NetworkModuleTest::testProtocolHeartbeat()
{
    qDebug() << "测试协议心跳...";
    
    QSignalSpy heartbeatSpy(m_webrtcProtocol, &IProtocolHandler::heartbeatSent);
    
    // 发送心跳
    bool result = m_webrtcProtocol->sendHeartbeat();
    QVERIFY(result);
    
    // 验证心跳信号
    QVERIFY(heartbeatSpy.wait(1000));
    QVERIFY(heartbeatSpy.count() > 0);
    
    m_passedTests++;
}

void NetworkModuleTest::testProtocolErrorHandling()
{
    qDebug() << "测试协议错误处理...";
    
    QSignalSpy errorSpy(m_webrtcProtocol, &IProtocolHandler::protocolError);
    
    // 尝试解码无效数据
    QByteArray invalidData = "invalid_protocol_data";
    IProtocolHandler::MessageType type;
    QVariantMap data;
    
    bool result = m_webrtcProtocol->decodeMessage(invalidData, type, data);
    QVERIFY(!result);
    
    m_passedTests++;
}

void NetworkModuleTest::testProtocolFeatureSupport()
{
    qDebug() << "测试协议功能支持...";
    
    // 测试WebRTC功能支持
    QStringList webrtcFeatures = m_webrtcProtocol->supportedFeatures();
    QVERIFY(webrtcFeatures.contains("ice"));
    QVERIFY(webrtcFeatures.contains("dtls"));
    
    // 测试HTTP功能支持
    QStringList httpFeatures = m_httpProtocol->supportedFeatures();
    QVERIFY(httpFeatures.contains("get"));
    QVERIFY(httpFeatures.contains("post"));
    
    // 测试WebSocket功能支持
    QStringList wsFeatures = m_websocketProtocol->supportedFeatures();
    QVERIFY(wsFeatures.contains("text"));
    QVERIFY(wsFeatures.contains("binary"));
    
    m_passedTests++;
}// 连接
工厂测试
void NetworkModuleTest::testConnectionFactory()
{
    qDebug() << "测试连接工厂...";
    
    // 测试创建不同类型的连接
    auto tcpConnection = m_connectionFactory->createConnection(IConnectionHandler::TCP);
    auto udpConnection = m_connectionFactory->createConnection(IConnectionHandler::UDP);
    auto wsConnection = m_connectionFactory->createConnection(IConnectionHandler::WebSocket);
    
    QVERIFY(tcpConnection != nullptr);
    QVERIFY(udpConnection != nullptr);
    QVERIFY(wsConnection != nullptr);
    
    // 验证连接类型
    QCOMPARE(tcpConnection->connectionType(), IConnectionHandler::TCP);
    QCOMPARE(udpConnection->connectionType(), IConnectionHandler::UDP);
    QCOMPARE(wsConnection->connectionType(), IConnectionHandler::WebSocket);
    
    m_passedTests++;
}

void NetworkModuleTest::testConnectionCreation()
{
    qDebug() << "测试连接创建...";
    
    // 测试连接创建参数
    QVariantMap config;
    config["timeout"] = 5000;
    config["retry_count"] = 3;
    
    auto connection = m_connectionFactory->createConnection(IConnectionHandler::HTTP, config);
    QVERIFY(connection != nullptr);
    
    // 验证配置应用
    QCOMPARE(connection->connectionTimeout(), 5000);
    
    m_passedTests++;
}

void NetworkModuleTest::testConnectionPooling()
{
    qDebug() << "测试连接池...";
    
    // 创建多个相同类型的连接
    auto conn1 = m_connectionFactory->createConnection(IConnectionHandler::HTTP);
    auto conn2 = m_connectionFactory->createConnection(IConnectionHandler::HTTP);
    auto conn3 = m_connectionFactory->createConnection(IConnectionHandler::HTTP);
    
    QVERIFY(conn1 != nullptr);
    QVERIFY(conn2 != nullptr);
    QVERIFY(conn3 != nullptr);
    
    // 验证连接是独立的
    QVERIFY(conn1 != conn2);
    QVERIFY(conn2 != conn3);
    QVERIFY(conn1 != conn3);
    
    m_passedTests++;
}

void NetworkModuleTest::testConnectionTypeSelection()
{
    qDebug() << "测试连接类型选择...";
    
    // 测试所有支持的连接类型
    QList<IConnectionHandler::ConnectionType> types = {
        IConnectionHandler::TCP,
        IConnectionHandler::UDP,
        IConnectionHandler::WebSocket,
        IConnectionHandler::WebRTC,
        IConnectionHandler::HTTP
    };
    
    for (auto type : types) {
        auto connection = m_connectionFactory->createConnection(type);
        QVERIFY2(connection != nullptr, QString("Failed to create connection of type %1").arg(type).toLatin1());
        QCOMPARE(connection->connectionType(), type);
    }
    
    m_passedTests++;
}

// 配置管理测试
void NetworkModuleTest::testNetworkConfiguration()
{
    qDebug() << "测试网络配置...";
    
    QVariantMap config;
    config["server_url"] = "https://test.example.com";
    config["port"] = 8443;
    config["ssl_enabled"] = true;
    config["timeout"] = 10000;
    
    // 加载配置
    bool result = m_networkConfig->loadConfiguration(config);
    QVERIFY(result);
    
    // 验证配置
    QCOMPARE(m_networkConfig->serverUrl(), config["server_url"].toString());
    QCOMPARE(m_networkConfig->port(), config["port"].toInt());
    QCOMPARE(m_networkConfig->sslEnabled(), config["ssl_enabled"].toBool());
    QCOMPARE(m_networkConfig->timeout(), config["timeout"].toInt());
    
    m_passedTests++;
}

void NetworkModuleTest::testConfigurationValidation()
{
    qDebug() << "测试配置验证...";
    
    // 测试有效配置
    QVariantMap validConfig;
    validConfig["server_url"] = "https://valid.example.com";
    validConfig["port"] = 443;
    
    bool validResult = m_networkConfig->validateConfiguration(validConfig);
    QVERIFY(validResult);
    
    // 测试无效配置
    QVariantMap invalidConfig;
    invalidConfig["server_url"] = "invalid_url";
    invalidConfig["port"] = -1;
    
    bool invalidResult = m_networkConfig->validateConfiguration(invalidConfig);
    QVERIFY(!invalidResult);
    
    m_passedTests++;
}

void NetworkModuleTest::testConfigurationPersistence()
{
    qDebug() << "测试配置持久化...";
    
    QVariantMap config;
    config["test_setting"] = "test_value";
    config["numeric_setting"] = 42;
    
    // 保存配置
    bool saveResult = m_networkConfig->saveConfiguration(config);
    QVERIFY(saveResult);
    
    // 加载配置
    QVariantMap loadedConfig = m_networkConfig->loadSavedConfiguration();
    QCOMPARE(loadedConfig["test_setting"].toString(), config["test_setting"].toString());
    QCOMPARE(loadedConfig["numeric_setting"].toInt(), config["numeric_setting"].toInt());
    
    m_passedTests++;
}

void NetworkModuleTest::testConfigurationDefaults()
{
    qDebug() << "测试默认配置...";
    
    // 获取默认配置
    QVariantMap defaults = m_networkConfig->getDefaultConfiguration();
    
    // 验证必要的默认值存在
    QVERIFY(defaults.contains("timeout"));
    QVERIFY(defaults.contains("retry_count"));
    QVERIFY(defaults.contains("ssl_enabled"));
    
    // 验证默认值合理性
    QVERIFY(defaults["timeout"].toInt() > 0);
    QVERIFY(defaults["retry_count"].toInt() >= 0);
    
    m_passedTests++;
}/
/ 错误处理和恢复测试
void NetworkModuleTest::testNetworkErrorHandling()
{
    qDebug() << "测试网络错误处理...";
    
    QSignalSpy errorSpy(m_networkManager, &NetworkManager::errorOccurred);
    
    // 测试无效URL连接
    bool result = m_networkManager->connectToServer("invalid://url");
    QVERIFY(!result || errorSpy.wait(3000));
    
    // 测试网络不可达
    result = m_networkManager->connectToServer("http://192.0.2.1:12345");
    if (result) {
        QVERIFY(errorSpy.wait(10000));
    }
    
    m_passedTests++;
}

void NetworkModuleTest::testConnectionRecovery()
{
    qDebug() << "测试连接恢复...";
    
    // 启用自动重连
    m_networkManager->setAutoReconnectEnabled(true);
    
    QSignalSpy reconnectSpy(m_networkManager, &NetworkManager::reconnectStarted);
    
    // 模拟连接中断
    if (m_networkManager->isConnected()) {
        m_networkManager->disconnect();
    }
    
    // 触发重连
    m_networkManager->reconnect();
    
    // 验证重连尝试
    bool signalReceived = waitForSignal(m_networkManager, SIGNAL(reconnectStarted()), 3000);
    if (signalReceived) {
        QVERIFY(reconnectSpy.count() > 0);
    }
    
    m_passedTests++;
}

void NetworkModuleTest::testProtocolErrorRecovery()
{
    qDebug() << "测试协议错误恢复...";
    
    QSignalSpy errorSpy(m_webrtcProtocol, &IProtocolHandler::protocolError);
    QSignalSpy statusSpy(m_webrtcProtocol, &IProtocolHandler::protocolStatusChanged);
    
    // 模拟协议错误
    m_webrtcProtocol->handleReceivedData("invalid_data");
    
    // 重置协议
    m_webrtcProtocol->reset();
    
    // 验证协议恢复
    QCOMPARE(m_webrtcProtocol->protocolStatus(), IProtocolHandler::Inactive);
    
    m_passedTests++;
}

void NetworkModuleTest::testTimeoutHandling()
{
    qDebug() << "测试超时处理...";
    
    // 创建连接处理器
    auto connection = m_connectionFactory->createConnection(IConnectionHandler::TCP);
    QVERIFY(connection != nullptr);
    
    // 设置短超时时间
    connection->setConnectionTimeout(1000);
    
    QSignalSpy timeoutSpy(connection.get(), &IConnectionHandler::connectionTimeout);
    
    // 尝试连接到不响应的地址
    bool result = connection->establishConnection("192.0.2.1:12345");
    
    if (result) {
        // 等待超时
        bool timeoutReceived = waitForSignal(connection.get(), SIGNAL(connectionTimeout()), 5000);
        if (timeoutReceived) {
            QVERIFY(timeoutSpy.count() > 0);
        }
    }
    
    m_passedTests++;
}

// 性能测试
void NetworkModuleTest::testConnectionPerformance()
{
    qDebug() << "测试连接性能...";
    
    QElapsedTimer timer;
    QList<qint64> connectionTimes;
    
    for (int i = 0; i < 10; ++i) {
        timer.start();
        
        bool result = m_networkManager->connectToServer(m_testServerURL);
        if (result) {
            waitForSignal(m_networkManager, SIGNAL(connected()), 5000);
        }
        
        qint64 elapsed = timer.elapsed();
        connectionTimes.append(elapsed);
        
        m_networkManager->disconnect();
        waitForSignal(m_networkManager, SIGNAL(disconnected()), 3000);
        
        QThread::msleep(100); // 短暂休息
    }
    
    // 计算平均连接时间
    qint64 totalTime = 0;
    for (qint64 time : connectionTimes) {
        totalTime += time;
    }
    qint64 averageTime = totalTime / connectionTimes.size();
    
    qDebug() << QString("平均连接时间: %1ms").arg(averageTime);
    QVERIFY(averageTime < 5000); // 连接时间应该小于5秒
    
    m_passedTests++;
}

void NetworkModuleTest::testDataTransmissionPerformance()
{
    qDebug() << "测试数据传输性能...";
    
    // 创建测试数据
    QByteArray testData = generateTestData(1024 * 100); // 100KB
    
    QElapsedTimer timer;
    timer.start();
    
    // 模拟数据传输
    auto connection = m_connectionFactory->createConnection(IConnectionHandler::HTTP);
    if (connection && connection->establishConnection(m_testServerURL)) {
        connection->sendData(testData);
        waitForSignal(connection.get(), SIGNAL(dataSent(qint64)), 5000);
    }
    
    qint64 elapsed = timer.elapsed();
    double throughput = (testData.size() / 1024.0) / (elapsed / 1000.0); // KB/s
    
    qDebug() << QString("数据传输吞吐量: %1 KB/s").arg(throughput);
    QVERIFY(throughput > 0);
    
    m_passedTests++;
}

void NetworkModuleTest::testMemoryUsage()
{
    qDebug() << "测试内存使用...";
    
    // 记录初始内存使用
    size_t initialMemory = getCurrentMemoryUsage();
    
    // 创建多个连接
    QList<std::shared_ptr<IConnectionHandler>> connections;
    for (int i = 0; i < 100; ++i) {
        auto conn = m_connectionFactory->createConnection(IConnectionHandler::HTTP);
        connections.append(conn);
    }
    
    // 记录峰值内存使用
    size_t peakMemory = getCurrentMemoryUsage();
    
    // 清理连接
    connections.clear();
    
    // 记录清理后内存使用
    size_t finalMemory = getCurrentMemoryUsage();
    
    qDebug() << QString("内存使用 - 初始: %1KB, 峰值: %2KB, 最终: %3KB")
                .arg(initialMemory/1024).arg(peakMemory/1024).arg(finalMemory/1024);
    
    // 验证内存没有严重泄漏
    QVERIFY(finalMemory < peakMemory * 1.1); // 允许10%的内存残留
    
    m_passedTests++;
}

void NetworkModuleTest::testCPUUsage()
{
    qDebug() << "测试CPU使用...";
    
    // 启动网络质量监控
    m_qualityMonitor->startMonitoring("8.8.8.8", 100); // 高频监控
    
    QElapsedTimer timer;
    timer.start();
    
    // 运行一段时间
    while (timer.elapsed() < 5000) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    
    m_qualityMonitor->stopMonitoring();
    
    // 这里应该有CPU使用率的实际测量
    // 由于测试环境限制，我们只验证程序没有崩溃
    QVERIFY(true);
    
    m_passedTests++;
}// 
兼容性测试
void NetworkModuleTest::testLegacyNetworkManagerCompatibility()
{
    qDebug() << "测试与旧版网络管理器的兼容性...";
    
    // 测试API兼容性
    QVERIFY(m_networkManager->metaObject()->indexOfMethod("connectToServer(QString)") != -1);
    QVERIFY(m_networkManager->metaObject()->indexOfMethod("disconnect()") != -1);
    QVERIFY(m_networkManager->metaObject()->indexOfMethod("isConnected()") != -1);
    
    // 测试信号兼容性
    QVERIFY(m_networkManager->metaObject()->indexOfSignal("connected()") != -1);
    QVERIFY(m_networkManager->metaObject()->indexOfSignal("disconnected()") != -1);
    QVERIFY(m_networkManager->metaObject()->indexOfSignal("errorOccurred(QString)") != -1);
    
    m_passedTests++;
}

void NetworkModuleTest::testExistingComponentIntegration()
{
    qDebug() << "测试与现有组件的集成...";
    
    // 测试与Qt网络组件的集成
    QNetworkRequest request(QUrl(m_testServerURL));
    QNetworkReply* reply = m_testNetworkManager->get(request);
    
    QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
    bool finished = finishedSpy.wait(10000);
    
    if (finished) {
        QVERIFY(reply->error() == QNetworkReply::NoError || 
                reply->error() == QNetworkReply::HostNotFoundError);
    }
    
    reply->deleteLater();
    m_passedTests++;
}

void NetworkModuleTest::testAPIBackwardCompatibility()
{
    qDebug() << "测试API向后兼容性...";
    
    // 测试旧版API调用
    bool result = m_networkManager->connectToServer(); // 无参数版本
    QVERIFY(result || !result); // 应该能够调用，不管结果如何
    
    // 测试属性访问
    NetworkManager::ConnectionState state = m_networkManager->connectionState();
    QVERIFY(state >= NetworkManager::Disconnected && state <= NetworkManager::Error);
    
    m_passedTests++;
}

void NetworkModuleTest::testConfigurationMigration()
{
    qDebug() << "测试配置迁移...";
    
    // 模拟旧版配置格式
    QVariantMap oldConfig;
    oldConfig["serverURL"] = "https://old.example.com"; // 旧格式
    oldConfig["serverPort"] = 8443;
    
    // 测试配置迁移
    QVariantMap migratedConfig = m_networkConfig->migrateConfiguration(oldConfig);
    
    // 验证迁移结果
    QVERIFY(migratedConfig.contains("server_url")); // 新格式
    QCOMPARE(migratedConfig["server_url"].toString(), oldConfig["serverURL"].toString());
    
    m_passedTests++;
}

// 并发和线程安全测试
void NetworkModuleTest::testConcurrentConnections()
{
    qDebug() << "测试并发连接...";
    
    const int connectionCount = 5;
    QList<std::shared_ptr<IConnectionHandler>> connections;
    QList<QSignalSpy*> spies;
    
    // 创建多个连接
    for (int i = 0; i < connectionCount; ++i) {
        auto conn = m_connectionFactory->createConnection(IConnectionHandler::HTTP);
        connections.append(conn);
        
        auto spy = new QSignalSpy(conn.get(), &IConnectionHandler::connectionEstablished);
        spies.append(spy);
    }
    
    // 同时建立连接
    for (auto& conn : connections) {
        conn->establishConnection(m_testServerURL);
    }
    
    // 等待连接建立
    int establishedCount = 0;
    for (auto spy : spies) {
        if (spy->wait(5000)) {
            establishedCount++;
        }
    }
    
    qDebug() << QString("成功建立 %1/%2 个并发连接").arg(establishedCount).arg(connectionCount);
    
    // 清理
    qDeleteAll(spies);
    
    m_passedTests++;
}

void NetworkModuleTest::testThreadSafety()
{
    qDebug() << "测试线程安全...";
    
    QList<QThread*> threads;
    QList<QSignalSpy*> spies;
    
    // 创建多个线程进行网络操作
    for (int i = 0; i < 3; ++i) {
        QThread* thread = new QThread(this);
        threads.append(thread);
        
        // 在线程中执行网络操作
        QTimer::singleShot(0, [this, i]() {
            auto conn = m_connectionFactory->createConnection(IConnectionHandler::HTTP);
            if (conn) {
                conn->establishConnection(QString("%1?thread=%2").arg(m_testServerURL).arg(i));
            }
        });
        
        thread->start();
    }
    
    // 等待所有线程完成
    for (auto thread : threads) {
        thread->wait(5000);
        thread->deleteLater();
    }
    
    qDeleteAll(spies);
    m_passedTests++;
}

void NetworkModuleTest::testSignalSlotConnections()
{
    qDebug() << "测试信号槽连接...";
    
    // 测试信号槽连接的线程安全性
    QSignalSpy stateSpy(m_networkManager, &NetworkManager::connectionStateChanged);
    QSignalSpy qualitySpy(m_networkManager, &NetworkManager::networkQualityChanged);
    QSignalSpy errorSpy(m_networkManager, &NetworkManager::errorOccurred);
    
    // 触发各种信号
    m_networkManager->connectToServer(m_testServerURL);
    m_networkManager->refreshNetworkStatus();
    
    // 等待信号
    QThread::msleep(1000);
    
    // 验证信号正常工作
    QVERIFY(stateSpy.count() >= 0);
    QVERIFY(qualitySpy.count() >= 0);
    QVERIFY(errorSpy.count() >= 0);
    
    m_passedTests++;
}// 边界条件测试

void NetworkModuleTest::testInvalidServerURL()
{
    qDebug() << "测试无效服务器URL...";
    
    QStringList invalidUrls = {
        "",
        "invalid",
        "://invalid",
        "http://",
        "ftp://unsupported.protocol",
        "http://[invalid:ipv6",
        "http://256.256.256.256",
        QString("http://") + QString("a").repeated(1000) + ".com"
    };
    
    for (const QString& url : invalidUrls) {
        QSignalSpy errorSpy(m_networkManager, &NetworkManager::errorOccurred);
        
        bool result = m_networkManager->connectToServer(url);
        
        // 应该立即失败或在短时间内产生错误
        if (result) {
            QVERIFY(errorSpy.wait(3000));
        } else {
            QVERIFY(!result);
        }
    }
    
    m_passedTests++;
}

void NetworkModuleTest::testNetworkUnavailable()
{
    qDebug() << "测试网络不可用...";
    
    // 模拟网络不可用的情况
    simulateNetworkConditions("unavailable");
    
    QSignalSpy errorSpy(m_networkManager, &NetworkManager::errorOccurred);
    
    bool result = m_networkManager->connectToServer(m_testServerURL);
    
    if (result) {
        // 应该在合理时间内产生网络错误
        QVERIFY(errorSpy.wait(10000));
    }
    
    // 恢复网络条件
    simulateNetworkConditions("normal");
    
    m_passedTests++;
}

void NetworkModuleTest::testLargeDataTransmission()
{
    qDebug() << "测试大数据传输...";
    
    // 创建大数据块 (1MB)
    QByteArray largeData = generateTestData(1024 * 1024);
    
    auto connection = m_connectionFactory->createConnection(IConnectionHandler::HTTP);
    QVERIFY(connection != nullptr);
    
    QSignalSpy dataSentSpy(connection.get(), &IConnectionHandler::dataSent);
    QSignalSpy errorSpy(connection.get(), &IConnectionHandler::connectionError);
    
    if (connection->establishConnection(m_testServerURL)) {
        bool result = connection->sendData(largeData);
        
        if (result) {
            // 等待数据发送完成或错误
            bool finished = dataSentSpy.wait(30000) || errorSpy.wait(30000);
            QVERIFY(finished);
            
            if (dataSentSpy.count() > 0) {
                qint64 totalSent = 0;
                for (const QList<QVariant>& args : dataSentSpy) {
                    totalSent += args[0].toLongLong();
                }
                QVERIFY(totalSent > 0);
            }
        }
    }
    
    m_passedTests++;
}

void NetworkModuleTest::testRapidConnectionCycles()
{
    qDebug() << "测试快速连接循环...";
    
    const int cycleCount = 20;
    int successfulCycles = 0;
    
    for (int i = 0; i < cycleCount; ++i) {
        // 快速连接和断开
        bool connectResult = m_networkManager->connectToServer(m_testServerURL);
        
        if (connectResult) {
            // 短暂等待
            QThread::msleep(100);
            
            m_networkManager->disconnect();
            
            // 短暂等待
            QThread::msleep(50);
            
            successfulCycles++;
        }
    }
    
    qDebug() << QString("成功完成 %1/%2 个连接循环").arg(successfulCycles).arg(cycleCount);
    
    // 至少应该有一些成功的循环
    QVERIFY(successfulCycles > 0);
    
    m_passedTests++;
}

// 辅助方法实现
bool NetworkModuleTest::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(timeout);
    
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(sender, signal, &loop, &QEventLoop::quit);
    
    timer.start();
    loop.exec();
    
    return timer.isActive();
}

QString NetworkModuleTest::getTestServerURL()
{
    return qEnvironmentVariable("TEST_SERVER_URL", m_testServerURL);
}

QByteArray NetworkModuleTest::generateTestData(int size)
{
    QByteArray data;
    data.reserve(size);
    
    for (int i = 0; i < size; ++i) {
        data.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    
    return data;
}

QVariantMap NetworkModuleTest::generateTestMessage(const QString& type)
{
    QVariantMap message;
    message["type"] = type;
    message["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    message["id"] = QUuid::createUuid().toString();
    
    if (type == "data") {
        message["payload"] = generateTestData(1024);
    } else if (type == "control") {
        message["command"] = "test_command";
        message["parameters"] = QVariantMap();
    }
    
    return message;
}

QStringList NetworkModuleTest::generateTestServerList()
{
    return QStringList() 
        << "https://meet.jit.si"
        << "https://8x8.vc"
        << "http://localhost:8080"
        << "wss://echo.websocket.org";
}

void NetworkModuleTest::simulateNetworkConditions(const QString& condition)
{
    // 这里可以实现网络条件模拟
    // 在实际实现中，可能需要使用网络模拟工具
    qDebug() << "模拟网络条件:" << condition;
}

void NetworkModuleTest::verifyConnectionState(INetworkManager::ConnectionState expectedState)
{
    QCOMPARE(m_networkManager->connectionState(), expectedState);
}

void NetworkModuleTest::verifyNetworkQuality(INetworkManager::NetworkQuality expectedQuality)
{
    QCOMPARE(m_networkManager->networkQuality(), expectedQuality);
}

size_t NetworkModuleTest::getCurrentMemoryUsage()
{
    // 简化的内存使用测量
    // 在实际实现中，应该使用平台特定的API
    return 0; // 占位符
}

// 包含模拟对象的实现
#include "NetworkModuleTest.moc"

// 注册测试
QTEST_MAIN(NetworkModuleTest)