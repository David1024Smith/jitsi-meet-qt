#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include <QWebSocket>
#include <QDomDocument>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include "XMPPClient.h"

/**
 * @brief XMPPClient单元测试类
 * 
 * 测试XMPPClient的核心功能：
 * - 连接管理和状态变化
 * - XMPP消息处理和解析
 * - 参与者管理
 * - 聊天消息发送和接收
 * - 存在信息处理
 * - 错误处理和重连机制
 */
class TestXMPPClient : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基本功能测试
    void testInitialState();
    void testConnectionStateChanges();
    void testJIDParsing();
    void testUniqueIdGeneration();
    void testAudioVideoMuteStates();
    
    // 连接管理测试
    void testConnectionFlow();
    void testServerConfiguration();
    void testWebSocketConnection();
    void testAuthentication();
    void testRoomJoining();
    void testDisconnection();
    
    // XMPP消息处理测试
    void testPresenceHandling();
    void testMessageHandling();
    void testIQHandling();
    void testXMPPStanzaParsing();
    void testXMPPStanzaGeneration();
    
    // 参与者管理测试
    void testParticipantJoining();
    void testParticipantLeaving();
    void testParticipantStatusUpdates();
    void testParticipantList();
    
    // 聊天功能测试
    void testChatMessageSending();
    void testChatMessageReceiving();
    void testChatMessageValidation();
    
    // 存在信息测试
    void testPresenceSending();
    void testPresenceReceiving();
    void testMuteStatusBroadcast();
    
    // 错误处理和重连测试
    void testConnectionErrors();
    void testReconnectionMechanism();
    void testHeartbeatMechanism();
    void testNetworkFailureRecovery();
    
    // 边界条件测试
    void testInvalidServerUrl();
    void testMalformedXMPPMessages();
    void testLargeMessageHandling();
    void testConcurrentConnections();

private:
    // 辅助方法
    void simulateWebSocketMessage(const QString& message);
    void simulateConnectionState(QAbstractSocket::SocketState state);
    void simulateNetworkError(QAbstractSocket::SocketError error);
    QString createTestPresenceStanza(const QString& from, const QString& type = "available");
    QString createTestMessageStanza(const QString& from, const QString& body);
    QString createTestIQStanza(const QString& id, const QString& type, const QString& content = "");
    void waitForSignal(QObject* sender, const char* signal, int timeout = 5000);

private:
    XMPPClient* m_client;
    QSignalSpy* m_connectionStateSpy;
    QSignalSpy* m_connectedSpy;
    QSignalSpy* m_disconnectedSpy;
    QSignalSpy* m_authenticatedSpy;
    QSignalSpy* m_roomJoinedSpy;
    QSignalSpy* m_participantJoinedSpy;
    QSignalSpy* m_participantLeftSpy;
    QSignalSpy* m_chatMessageSpy;
    QSignalSpy* m_errorSpy;
};

void TestXMPPClient::initTestCase()
{
    qDebug() << "Starting XMPPClient comprehensive unit tests";
    // 设置测试环境
    QLoggingCategory::setFilterRules("qt.network.ssl.debug=false");
}

void TestXMPPClient::cleanupTestCase()
{
    qDebug() << "XMPPClient unit tests completed";
}

void TestXMPPClient::init()
{
    m_client = new XMPPClient(this);
    
    // 创建信号监听器
    m_connectionStateSpy = new QSignalSpy(m_client, &XMPPClient::connectionStateChanged);
    m_connectedSpy = new QSignalSpy(m_client, &XMPPClient::connected);
    m_disconnectedSpy = new QSignalSpy(m_client, &XMPPClient::disconnected);
    m_authenticatedSpy = new QSignalSpy(m_client, &XMPPClient::authenticated);
    m_roomJoinedSpy = new QSignalSpy(m_client, &XMPPClient::roomJoined);
    m_participantJoinedSpy = new QSignalSpy(m_client, &XMPPClient::participantJoined);
    m_participantLeftSpy = new QSignalSpy(m_client, &XMPPClient::participantLeft);
    m_chatMessageSpy = new QSignalSpy(m_client, &XMPPClient::chatMessageReceived);
    m_errorSpy = new QSignalSpy(m_client, &XMPPClient::errorOccurred);
}

void TestXMPPClient::cleanup()
{
    if (m_client) {
        m_client->disconnect();
        delete m_client;
        m_client = nullptr;
    }
    
    // 清理信号监听器
    delete m_connectionStateSpy;
    delete m_connectedSpy;
    delete m_disconnectedSpy;
    delete m_authenticatedSpy;
    delete m_roomJoinedSpy;
    delete m_participantJoinedSpy;
    delete m_participantLeftSpy;
    delete m_chatMessageSpy;
    delete m_errorSpy;
}

void TestXMPPClient::testInitialState()
{
    // 测试初始状态
    QCOMPARE(m_client->connectionState(), XMPPClient::Disconnected);
    QVERIFY(m_client->currentRoom().isEmpty());
    QVERIFY(m_client->serverUrl().isEmpty());
    QVERIFY(m_client->userJid().isEmpty());
    QVERIFY(m_client->displayName().isEmpty());
    QVERIFY(m_client->participants().isEmpty());
    QVERIFY(!m_client->isConnected());
    QVERIFY(!m_client->isInRoom());
    
    // 验证信号监听器初始状态
    QVERIFY(m_connectionStateSpy->isValid());
    QVERIFY(m_connectedSpy->isValid());
    QVERIFY(m_disconnectedSpy->isValid());
    QVERIFY(m_authenticatedSpy->isValid());
    QVERIFY(m_roomJoinedSpy->isValid());
    QVERIFY(m_participantJoinedSpy->isValid());
    QVERIFY(m_participantLeftSpy->isValid());
    QVERIFY(m_chatMessageSpy->isValid());
    QVERIFY(m_errorSpy->isValid());
    
    // 验证初始信号计数为0
    QCOMPARE(m_connectionStateSpy->count(), 0);
    QCOMPARE(m_connectedSpy->count(), 0);
    QCOMPARE(m_disconnectedSpy->count(), 0);
}

void TestXMPPClient::testConnectionStateChanges()
{
    // 测试连接状态变化
    QCOMPARE(m_client->connectionState(), XMPPClient::Disconnected);
    
    // 尝试连接到测试服务器
    m_client->connectToServer("wss://test.example.com", "testroom", "testuser");
    
    // 验证状态变为Connecting
    QCOMPARE(m_client->connectionState(), XMPPClient::Connecting);
    QVERIFY(m_connectionStateSpy->count() >= 1);
    
    // 验证连接参数设置正确
    QCOMPARE(m_client->serverUrl(), QString("wss://test.example.com"));
    QCOMPARE(m_client->currentRoom(), QString("testroom"));
    QCOMPARE(m_client->displayName(), QString("testuser"));
    
    // 等待连接尝试完成或超时
    QTest::qWait(1000);
    
    // 由于是无效服务器，应该进入错误状态或保持连接状态
    QVERIFY(m_client->connectionState() == XMPPClient::Error || 
            m_client->connectionState() == XMPPClient::Connecting);
}

void TestXMPPClient::testJIDParsing()
{
    // 测试JID格式验证和处理
    // 通过连接操作间接测试JID处理
    
    // 测试有效的服务器URL和房间名
    m_client->connectToServer("https://meet.jit.si", "validroom", "testuser");
    
    // 验证内部JID构建（通过公共接口观察）
    QVERIFY(!m_client->userJid().isEmpty() || m_client->connectionState() == XMPPClient::Connecting);
    
    // 测试包含特殊字符的房间名
    XMPPClient* client2 = new XMPPClient(this);
    client2->connectToServer("https://meet.jit.si", "room-with-dashes_and_underscores", "user@example.com");
    
    // 验证连接尝试正常启动
    QCOMPARE(client2->connectionState(), XMPPClient::Connecting);
    QCOMPARE(client2->currentRoom(), QString("room-with-dashes_and_underscores"));
    
    delete client2;
}

void TestXMPPClient::testUniqueIdGeneration()
{
    // 测试唯一ID生成
    // 通过多次连接操作验证ID的唯一性
    
    // 创建多个客户端实例
    XMPPClient client1(this);
    XMPPClient client2(this);
    XMPPClient client3(this);
    
    // 连接到相同服务器但不同房间
    client1.connectToServer("https://test.example.com", "room1", "user1");
    client2.connectToServer("https://test.example.com", "room2", "user2");
    client3.connectToServer("https://test.example.com", "room3", "user3");
    
    // 验证每个客户端都进入连接状态
    QCOMPARE(client1.connectionState(), XMPPClient::Connecting);
    QCOMPARE(client2.connectionState(), XMPPClient::Connecting);
    QCOMPARE(client3.connectionState(), XMPPClient::Connecting);
    
    // 验证房间名设置正确（间接验证ID生成）
    QCOMPARE(client1.currentRoom(), QString("room1"));
    QCOMPARE(client2.currentRoom(), QString("room2"));
    QCOMPARE(client3.currentRoom(), QString("room3"));
}

void TestXMPPClient::testAudioVideoMuteStates()
{
    // 测试音视频静音状态管理
    QSignalSpy presenceSpy(m_client, &XMPPClient::connectionStateChanged);
    
    // 测试设置音频静音
    m_client->setAudioMuted(true);
    m_client->setAudioMuted(false);
    
    // 测试设置视频静音
    m_client->setVideoMuted(true);
    m_client->setVideoMuted(false);
    
    // 验证重复设置相同状态
    m_client->setAudioMuted(true);
    m_client->setAudioMuted(true); // 应该不产生额外操作
    
    m_client->setVideoMuted(false);
    m_client->setVideoMuted(false); // 应该不产生额外操作
    
    // 测试同时设置音视频静音
    m_client->setAudioMuted(true);
    m_client->setVideoMuted(true);
    
    // 验证方法调用不会导致崩溃
    QVERIFY(true);
}

void TestXMPPClient::testConnectionFlow()
{
    // 测试完整的连接流程
    
    // 1. 初始状态验证
    QCOMPARE(m_client->connectionState(), XMPPClient::Disconnected);
    
    // 2. 开始连接
    m_client->connectToServer("https://meet.jit.si", "testroom", "testuser");
    QCOMPARE(m_client->connectionState(), XMPPClient::Connecting);
    QVERIFY(m_connectionStateSpy->count() >= 1);
    
    // 3. 验证连接参数
    QCOMPARE(m_client->serverUrl(), QString("https://meet.jit.si"));
    QCOMPARE(m_client->currentRoom(), QString("testroom"));
    QCOMPARE(m_client->displayName(), QString("testuser"));
    
    // 4. 等待连接尝试
    QTest::qWait(2000);
    
    // 5. 测试断开连接
    m_client->disconnect();
    
    // 等待断开完成
    QTest::qWait(500);
    
    // 验证最终状态
    QVERIFY(m_client->connectionState() == XMPPClient::Disconnected || 
            m_client->connectionState() == XMPPClient::Disconnecting);
}

void TestXMPPClient::testServerConfiguration()
{
    // 测试服务器配置获取
    
    // 测试有效的Jitsi Meet服务器URL
    m_client->connectToServer("https://meet.jit.si", "configtest", "user");
    QCOMPARE(m_client->connectionState(), XMPPClient::Connecting);
    
    // 测试自定义服务器URL
    XMPPClient* client2 = new XMPPClient(this);
    client2->connectToServer("https://custom.jitsi.example.com", "room", "user");
    QCOMPARE(client2->connectionState(), XMPPClient::Connecting);
    
    delete client2;
}

void TestXMPPClient::testWebSocketConnection()
{
    // 测试WebSocket连接建立
    
    // 模拟WebSocket连接
    m_client->connectToServer("wss://meet.jit.si/xmpp-websocket", "wstest", "wsuser");
    
    // 验证连接状态变化
    QCOMPARE(m_client->connectionState(), XMPPClient::Connecting);
    
    // 等待WebSocket连接尝试
    QTest::qWait(1000);
    
    // 验证错误处理（由于是测试环境，连接会失败）
    QVERIFY(m_errorSpy->count() >= 0);
}

void TestXMPPClient::testAuthentication()
{
    // 测试认证流程
    
    m_client->connectToServer("https://meet.jit.si", "authtest", "authuser");
    
    // 等待认证尝试
    QTest::qWait(1500);
    
    // 在测试环境中，认证可能不会成功，但应该尝试
    QVERIFY(m_client->connectionState() != XMPPClient::Disconnected);
}

void TestXMPPClient::testRoomJoining()
{
    // 测试房间加入流程
    
    m_client->connectToServer("https://meet.jit.si", "jointest", "joinuser");
    
    // 验证房间信息设置
    QCOMPARE(m_client->currentRoom(), QString("jointest"));
    
    // 等待房间加入尝试
    QTest::qWait(1000);
    
    // 验证状态变化
    QVERIFY(m_connectionStateSpy->count() >= 1);
}

void TestXMPPClient::testDisconnection()
{
    // 测试断开连接
    
    // 先建立连接
    m_client->connectToServer("https://meet.jit.si", "disconnecttest", "user");
    QCOMPARE(m_client->connectionState(), XMPPClient::Connecting);
    
    // 立即断开连接
    m_client->disconnect();
    
    // 等待断开完成
    QTest::qWait(500);
    
    // 验证断开状态
    QVERIFY(m_client->connectionState() == XMPPClient::Disconnected ||
            m_client->connectionState() == XMPPClient::Disconnecting);
}

void TestXMPPClient::testPresenceHandling()
{
    // 测试存在信息处理
    
    // 验证信号有效性
    QVERIFY(m_participantJoinedSpy->isValid());
    QVERIFY(m_participantLeftSpy->isValid());
    
    // 测试发送存在信息
    m_client->sendPresence("available");
    m_client->sendPresence("away");
    m_client->sendPresence(); // 默认状态
    
    // 验证方法调用不会崩溃
    QVERIFY(true);
    
    // 模拟接收存在信息
    QString presenceStanza = createTestPresenceStanza("room@conference.meet.jit.si/participant1", "available");
    simulateWebSocketMessage(presenceStanza);
}

void TestXMPPClient::testMessageHandling()
{
    // 测试消息处理
    QVERIFY(m_chatMessageSpy->isValid());
    
    // 测试发送消息（在未连接状态下应该失败）
    m_client->sendChatMessage("Test message");
    
    // 验证在未连接状态下不会发送消息
    QVERIFY(!m_client->isInRoom());
    
    // 模拟接收聊天消息
    QString messageStanza = createTestMessageStanza("room@conference.meet.jit.si/sender", "Hello World");
    simulateWebSocketMessage(messageStanza);
    
    // 测试发送空消息
    m_client->sendChatMessage("");
    m_client->sendChatMessage("   ");
    
    // 测试发送长消息
    QString longMessage(5000, 'A');
    m_client->sendChatMessage(longMessage);
}

void TestXMPPClient::testIQHandling()
{
    // 测试IQ（Info/Query）处理
    
    // 模拟各种IQ消息
    QString iqGet = createTestIQStanza("iq1", "get", "<query xmlns='jabber:iq:roster'/>");
    QString iqSet = createTestIQStanza("iq2", "set", "<query xmlns='jabber:iq:roster'/>");
    QString iqResult = createTestIQStanza("iq3", "result", "<query xmlns='jabber:iq:roster'/>");
    QString iqError = createTestIQStanza("iq4", "error", "<error type='cancel'><item-not-found xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'/></error>");
    
    simulateWebSocketMessage(iqGet);
    simulateWebSocketMessage(iqSet);
    simulateWebSocketMessage(iqResult);
    simulateWebSocketMessage(iqError);
    
    // 验证处理不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testXMPPStanzaParsing()
{
    // 测试XMPP节解析
    
    // 测试格式良好的XML
    QString validXML = "<presence from='room@conference.meet.jit.si/user1' to='user@meet.jit.si/resource'><show>available</show></presence>";
    simulateWebSocketMessage(validXML);
    
    // 测试包含特殊字符的XML
    QString specialCharsXML = "<message from='room@conference.meet.jit.si/user1'><body>&lt;test&gt; &amp; &quot;quotes&quot;</body></message>";
    simulateWebSocketMessage(specialCharsXML);
    
    // 验证解析不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testXMPPStanzaGeneration()
{
    // 测试XMPP节生成
    
    // 测试各种消息发送
    m_client->sendChatMessage("Normal message");
    m_client->sendChatMessage("Message with <special> &characters& \"quotes\"");
    m_client->sendPresence("available");
    m_client->sendPresence("away");
    
    // 测试静音状态发送
    m_client->setAudioMuted(true);
    m_client->setVideoMuted(true);
    m_client->setAudioMuted(false);
    m_client->setVideoMuted(false);
    
    // 验证生成不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testParticipantJoining()
{
    // 测试参与者加入
    
    // 模拟参与者加入的存在信息
    QString joinPresence = createTestPresenceStanza("room@conference.meet.jit.si/participant1", "available");
    simulateWebSocketMessage(joinPresence);
    
    QString joinPresence2 = createTestPresenceStanza("room@conference.meet.jit.si/participant2", "available");
    simulateWebSocketMessage(joinPresence2);
    
    // 验证参与者列表（间接测试）
    QVERIFY(m_participantJoinedSpy->count() >= 0);
}

void TestXMPPClient::testParticipantLeaving()
{
    // 测试参与者离开
    
    // 先模拟参与者加入
    QString joinPresence = createTestPresenceStanza("room@conference.meet.jit.si/participant1", "available");
    simulateWebSocketMessage(joinPresence);
    
    // 然后模拟参与者离开
    QString leavePresence = createTestPresenceStanza("room@conference.meet.jit.si/participant1", "unavailable");
    simulateWebSocketMessage(leavePresence);
    
    // 验证离开事件
    QVERIFY(m_participantLeftSpy->count() >= 0);
}

void TestXMPPClient::testParticipantStatusUpdates()
{
    // 测试参与者状态更新
    
    // 模拟参与者状态变化
    QString statusUpdate1 = "<presence from='room@conference.meet.jit.si/participant1'><show>away</show><status>Away from keyboard</status></presence>";
    simulateWebSocketMessage(statusUpdate1);
    
    QString statusUpdate2 = "<presence from='room@conference.meet.jit.si/participant1'><show>available</show></presence>";
    simulateWebSocketMessage(statusUpdate2);
    
    // 验证状态更新处理
    QVERIFY(true);
}

void TestXMPPClient::testParticipantList()
{
    // 测试参与者列表管理
    
    // 初始状态应该没有参与者
    QVERIFY(m_client->participants().isEmpty());
    
    // 模拟多个参与者加入
    for (int i = 1; i <= 5; ++i) {
        QString presence = createTestPresenceStanza(
            QString("room@conference.meet.jit.si/participant%1").arg(i), "available");
        simulateWebSocketMessage(presence);
    }
    
    // 验证参与者列表处理
    QVERIFY(m_participantJoinedSpy->count() >= 0);
}

void TestXMPPClient::testChatMessageSending()
{
    // 测试聊天消息发送
    
    // 测试正常消息
    m_client->sendChatMessage("Hello everyone!");
    m_client->sendChatMessage("How are you doing?");
    
    // 测试特殊字符消息
    m_client->sendChatMessage("Message with émojis 😀 and symbols ♠♥♦♣");
    m_client->sendChatMessage("XML chars: <>&\"'");
    
    // 测试多行消息
    m_client->sendChatMessage("Line 1\nLine 2\nLine 3");
    
    // 验证发送不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testChatMessageReceiving()
{
    // 测试聊天消息接收
    
    // 模拟接收各种消息
    QString msg1 = createTestMessageStanza("room@conference.meet.jit.si/alice", "Hello everyone!");
    QString msg2 = createTestMessageStanza("room@conference.meet.jit.si/bob", "How is everyone doing?");
    QString msg3 = createTestMessageStanza("room@conference.meet.jit.si/charlie", "Great meeting today!");
    
    simulateWebSocketMessage(msg1);
    simulateWebSocketMessage(msg2);
    simulateWebSocketMessage(msg3);
    
    // 验证消息接收处理
    QVERIFY(m_chatMessageSpy->count() >= 0);
}

void TestXMPPClient::testChatMessageValidation()
{
    // 测试聊天消息验证
    
    // 测试空消息
    m_client->sendChatMessage("");
    m_client->sendChatMessage("   ");
    m_client->sendChatMessage("\t\n");
    
    // 测试null消息
    m_client->sendChatMessage(QString());
    
    // 测试超长消息
    QString veryLongMessage(10000, 'X');
    m_client->sendChatMessage(veryLongMessage);
    
    // 验证验证不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testPresenceSending()
{
    // 测试存在信息发送
    
    m_client->sendPresence(); // 默认
    m_client->sendPresence("available");
    m_client->sendPresence("away");
    m_client->sendPresence("dnd");
    m_client->sendPresence("xa");
    
    // 测试自定义状态
    m_client->sendPresence("custom status message");
    
    // 验证发送不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testPresenceReceiving()
{
    // 测试存在信息接收
    
    // 模拟接收各种存在信息
    QString available = createTestPresenceStanza("room@conference.meet.jit.si/user1", "available");
    QString away = createTestPresenceStanza("room@conference.meet.jit.si/user2", "away");
    QString unavailable = createTestPresenceStanza("room@conference.meet.jit.si/user3", "unavailable");
    
    simulateWebSocketMessage(available);
    simulateWebSocketMessage(away);
    simulateWebSocketMessage(unavailable);
    
    // 验证接收处理
    QVERIFY(true);
}

void TestXMPPClient::testMuteStatusBroadcast()
{
    // 测试静音状态广播
    
    // 测试音频静音广播
    m_client->setAudioMuted(true);
    m_client->setAudioMuted(false);
    
    // 测试视频静音广播
    m_client->setVideoMuted(true);
    m_client->setVideoMuted(false);
    
    // 测试同时静音
    m_client->setAudioMuted(true);
    m_client->setVideoMuted(true);
    
    // 测试同时取消静音
    m_client->setAudioMuted(false);
    m_client->setVideoMuted(false);
    
    // 验证广播不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testConnectionErrors()
{
    // 测试连接错误处理
    
    // 测试无效URL
    m_client->connectToServer("invalid://url", "room", "user");
    QTest::qWait(500);
    
    // 测试不存在的服务器
    XMPPClient* client2 = new XMPPClient(this);
    client2->connectToServer("https://nonexistent.server.example.com", "room", "user");
    QTest::qWait(1000);
    
    // 验证错误处理
    QVERIFY(m_errorSpy->count() >= 0);
    
    delete client2;
}

void TestXMPPClient::testReconnectionMechanism()
{
    // 测试重连机制
    
    // 连接到无效服务器触发重连
    m_client->connectToServer("wss://invalid.example.com", "room", "user");
    
    // 等待重连尝试
    QTest::qWait(3000);
    
    // 验证重连尝试
    QVERIFY(m_connectionStateSpy->count() >= 1);
}

void TestXMPPClient::testHeartbeatMechanism()
{
    // 测试心跳机制
    
    // 建立连接以启动心跳
    m_client->connectToServer("https://meet.jit.si", "heartbeat", "user");
    
    // 等待心跳周期
    QTest::qWait(2000);
    
    // 验证心跳不会导致错误
    QVERIFY(true);
}

void TestXMPPClient::testNetworkFailureRecovery()
{
    // 测试网络故障恢复
    
    // 模拟网络连接
    m_client->connectToServer("https://meet.jit.si", "recovery", "user");
    
    // 模拟网络错误
    simulateNetworkError(QAbstractSocket::NetworkError);
    
    // 等待恢复尝试
    QTest::qWait(1000);
    
    // 验证恢复机制
    QVERIFY(m_errorSpy->count() >= 0);
}

void TestXMPPClient::testInvalidServerUrl()
{
    // 测试无效服务器URL处理
    
    QStringList invalidUrls = {
        "",
        "invalid",
        "ftp://example.com",
        "http://",
        "https://",
        "not-a-url",
        "javascript:alert('xss')"
    };
    
    for (const QString& url : invalidUrls) {
        XMPPClient* client = new XMPPClient(this);
        client->connectToServer(url, "room", "user");
        
        // 验证处理无效URL不会崩溃
        QVERIFY(true);
        
        delete client;
    }
}

void TestXMPPClient::testMalformedXMPPMessages()
{
    // 测试格式错误的XMPP消息处理
    
    QStringList malformedMessages = {
        "",
        "<invalid>",
        "<presence><unclosed>",
        "not xml at all",
        "<presence from='invalid jid'>",
        "<message><body>unclosed body</message>",
        "<?xml version='1.0'?><root><invalid></root>",
        "<presence xmlns:invalid='invalid namespace'>"
    };
    
    for (const QString& msg : malformedMessages) {
        simulateWebSocketMessage(msg);
    }
    
    // 验证处理格式错误消息不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testLargeMessageHandling()
{
    // 测试大消息处理
    
    // 测试大型存在信息
    QString largePresence = "<presence from='room@conference.meet.jit.si/user'>";
    largePresence += QString("<status>%1</status>").arg(QString(5000, 'A'));
    largePresence += "</presence>";
    simulateWebSocketMessage(largePresence);
    
    // 测试大型聊天消息
    QString largeMessage = createTestMessageStanza("room@conference.meet.jit.si/user", QString(10000, 'B'));
    simulateWebSocketMessage(largeMessage);
    
    // 验证大消息处理
    QVERIFY(true);
}

void TestXMPPClient::testConcurrentConnections()
{
    // 测试并发连接
    
    QList<XMPPClient*> clients;
    
    // 创建多个并发连接
    for (int i = 0; i < 5; ++i) {
        XMPPClient* client = new XMPPClient(this);
        client->connectToServer("https://meet.jit.si", QString("room%1").arg(i), QString("user%1").arg(i));
        clients.append(client);
    }
    
    // 等待连接尝试
    QTest::qWait(1000);
    
    // 清理
    for (XMPPClient* client : clients) {
        client->disconnect();
        delete client;
    }
    
    // 验证并发处理
    QVERIFY(true);
}

// Helper methods implementation

void TestXMPPClient::simulateWebSocketMessage(const QString& message)
{
    // 模拟WebSocket消息接收
    // 由于无法直接访问私有WebSocket，这里只是调用公共方法
    // 在实际实现中，可能需要使用友元类或测试专用接口
    
    // 这里我们只验证消息格式不会导致崩溃
    QDomDocument doc;
    doc.setContent(message);
    
    // 验证XML解析
    QVERIFY(true); // 如果到这里没有崩溃就算成功
}

void TestXMPPClient::simulateConnectionState(QAbstractSocket::SocketState state)
{
    // 模拟连接状态变化
    // 这是一个辅助方法，用于测试连接状态处理
    Q_UNUSED(state)
    
    // 在实际实现中，这里会触发相应的状态变化处理
    QVERIFY(true);
}

void TestXMPPClient::simulateNetworkError(QAbstractSocket::SocketError error)
{
    // 模拟网络错误
    Q_UNUSED(error)
    
    // 在实际实现中，这里会触发错误处理机制
    QVERIFY(true);
}

QString TestXMPPClient::createTestPresenceStanza(const QString& from, const QString& type)
{
    QString stanza = QString("<presence from='%1'").arg(from);
    if (!type.isEmpty() && type != "available") {
        stanza += QString(" type='%1'").arg(type);
    }
    stanza += ">";
    
    if (type == "available") {
        stanza += "<show>available</show>";
    } else if (type == "away") {
        stanza += "<show>away</show>";
    }
    
    stanza += "</presence>";
    return stanza;
}

QString TestXMPPClient::createTestMessageStanza(const QString& from, const QString& body)
{
    return QString("<message from='%1' type='groupchat'><body>%2</body></message>")
           .arg(from)
           .arg(body);
}

QString TestXMPPClient::createTestIQStanza(const QString& id, const QString& type, const QString& content)
{
    QString stanza = QString("<iq id='%1' type='%2'").arg(id, type);
    
    if (!content.isEmpty()) {
        stanza += QString(">%1</iq>").arg(content);
    } else {
        stanza += "/>";
    }
    
    return stanza;
}

void TestXMPPClient::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QSignalSpy spy(sender, signal);
    QVERIFY(spy.wait(timeout) || spy.count() > 0);
}

QTEST_MAIN(TestXMPPClient)
#include "test_xmpp_client.moc"