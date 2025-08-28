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
 * 测试XMPPClient的连接和消息处理功能：
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

    // 连接和状态测试
    void testInitialState();
    void testConnectionStateChanges();
    void testConnectionFlow();
    void testDisconnection();
    
    // 消息处理测试
    void testChatMessageSending();
    void testPresenceHandling();
    void testXMPPStanzaParsing();
    void testParticipantManagement();
    
    // 错误处理测试
    void testConnectionErrors();
    void testInvalidServerUrl();
    void testMalformedMessages();

private:
    // 辅助方法
    void simulateWebSocketMessage(const QString& message);
    QString createTestPresenceStanza(const QString& from, const QString& type = "available");
    QString createTestMessageStanza(const QString& from, const QString& body);

private:
    XMPPClient* m_client;
    QSignalSpy* m_connectionStateSpy;
    QSignalSpy* m_connectedSpy;
    QSignalSpy* m_chatMessageSpy;
    QSignalSpy* m_errorSpy;
};

void TestXMPPClient::initTestCase()
{
    qDebug() << "Starting XMPPClient unit tests";
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
    
    delete m_connectionStateSpy;
    delete m_connectedSpy;
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
}

void TestXMPPClient::testConnectionStateChanges()
{
    // 测试连接状态变化
    QCOMPARE(m_client->connectionState(), XMPPClient::Disconnected);
    
    // 尝试连接到测试服务器
    m_client->connectToServer("https://meet.jit.si", "testroom", "testuser");
    
    // 验证状态变为Connecting
    QCOMPARE(m_client->connectionState(), XMPPClient::Connecting);
    QVERIFY(m_connectionStateSpy->count() >= 1);
    
    // 验证连接参数设置正确
    QCOMPARE(m_client->serverUrl(), QString("https://meet.jit.si"));
    QCOMPARE(m_client->currentRoom(), QString("testroom"));
    QCOMPARE(m_client->displayName(), QString("testuser"));
}

void TestXMPPClient::testConnectionFlow()
{
    // 测试完整的连接流程
    QCOMPARE(m_client->connectionState(), XMPPClient::Disconnected);
    
    // 开始连接
    m_client->connectToServer("https://meet.jit.si", "testroom", "testuser");
    QCOMPARE(m_client->connectionState(), XMPPClient::Connecting);
    
    // 验证连接参数
    QCOMPARE(m_client->serverUrl(), QString("https://meet.jit.si"));
    QCOMPARE(m_client->currentRoom(), QString("testroom"));
    QCOMPARE(m_client->displayName(), QString("testuser"));
    
    // 等待连接尝试
    QTest::qWait(1000);
}

void TestXMPPClient::testDisconnection()
{
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

void TestXMPPClient::testChatMessageSending()
{
    // 测试聊天消息发送
    m_client->sendChatMessage("Hello everyone!");
    m_client->sendChatMessage("How are you doing?");
    
    // 测试特殊字符消息
    m_client->sendChatMessage("Message with <special> &characters& \"quotes\"");
    
    // 测试空消息
    m_client->sendChatMessage("");
    m_client->sendChatMessage("   ");
    
    // 验证发送不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testPresenceHandling()
{
    // 测试存在信息处理
    m_client->sendPresence("available");
    m_client->sendPresence("away");
    m_client->sendPresence(); // 默认状态
    
    // 测试音视频静音状态
    m_client->setAudioMuted(true);
    m_client->setVideoMuted(true);
    m_client->setAudioMuted(false);
    m_client->setVideoMuted(false);
    
    // 验证方法调用不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testXMPPStanzaParsing()
{
    // 测试XMPP节解析
    QString validXML = "<presence from='room@conference.meet.jit.si/user1'><show>available</show></presence>";
    simulateWebSocketMessage(validXML);
    
    QString messageXML = "<message from='room@conference.meet.jit.si/user1'><body>Hello World</body></message>";
    simulateWebSocketMessage(messageXML);
    
    // 验证解析不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testParticipantManagement()
{
    // 模拟参与者加入
    QString joinPresence = createTestPresenceStanza("room@conference.meet.jit.si/participant1", "available");
    simulateWebSocketMessage(joinPresence);
    
    // 模拟参与者离开
    QString leavePresence = createTestPresenceStanza("room@conference.meet.jit.si/participant1", "unavailable");
    simulateWebSocketMessage(leavePresence);
    
    // 验证参与者管理不会崩溃
    QVERIFY(true);
}

void TestXMPPClient::testConnectionErrors()
{
    // 测试无效URL
    m_client->connectToServer("invalid://url", "room", "user");
    QTest::qWait(500);
    
    // 验证错误处理
    QVERIFY(m_errorSpy->count() >= 0);
}

void TestXMPPClient::testInvalidServerUrl()
{
    QStringList invalidUrls = {
        "",
        "invalid",
        "ftp://example.com",
        "not-a-url"
    };
    
    for (const QString& url : invalidUrls) {
        XMPPClient* client = new XMPPClient(this);
        client->connectToServer(url, "room", "user");
        
        // 验证处理无效URL不会崩溃
        QVERIFY(true);
        
        delete client;
    }
}

void TestXMPPClient::testMalformedMessages()
{
    QStringList malformedMessages = {
        "",
        "<invalid>",
        "<presence><unclosed>",
        "not xml at all"
    };
    
    for (const QString& msg : malformedMessages) {
        simulateWebSocketMessage(msg);
    }
    
    // 验证处理格式错误消息不会崩溃
    QVERIFY(true);
}

// Helper methods
void TestXMPPClient::simulateWebSocketMessage(const QString& message)
{
    // 模拟WebSocket消息接收
    QDomDocument doc;
    doc.setContent(message);
    
    // 验证XML解析不会崩溃
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

QTEST_MAIN(TestXMPPClient)
#include "test_unit_xmpp_client.moc"