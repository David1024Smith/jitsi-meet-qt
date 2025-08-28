#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QApplication>
#include "ProtocolHandler.h"
#include "JitsiConstants.h"

class TestProtocolHandler : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // 协议URL解析测试
    void testParseProtocolUrl_data();
    void testParseProtocolUrl();
    
    // URL验证测试
    void testIsValidProtocolUrl_data();
    void testIsValidProtocolUrl();
    
    // 协议注册测试
    void testRegisterProtocol();
    void testUnregisterProtocol();
    
    // 房间信息提取测试
    void testExtractRoomInfo_data();
    void testExtractRoomInfo();
    
    // 边界条件测试
    void testEmptyUrl();
    void testInvalidUrls();
    void testSpecialCharacters();
    
    // 信号测试
    void testProtocolUrlReceivedSignal();

private:
    ProtocolHandler* m_protocolHandler;
    QApplication* m_app;
};

void TestProtocolHandler::initTestCase()
{
    // 创建应用程序实例（如果不存在）
    if (!QApplication::instance()) {
        int argc = 1;
        char* argv[] = {"test"};
        m_app = new QApplication(argc, argv);
    } else {
        m_app = nullptr;
    }
}

void TestProtocolHandler::cleanupTestCase()
{
    if (m_app) {
        delete m_app;
        m_app = nullptr;
    }
}

void TestProtocolHandler::init()
{
    m_protocolHandler = new ProtocolHandler();
}

void TestProtocolHandler::cleanup()
{
    delete m_protocolHandler;
    m_protocolHandler = nullptr;
}

void TestProtocolHandler::testParseProtocolUrl_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");
    
    // 基本房间名
    QTest::newRow("simple room") 
        << "jitsi-meet://test-room" 
        << "https://meet.jit.si/test-room";
    
    // 带服务器的房间
    QTest::newRow("custom server") 
        << "jitsi-meet://example.com/my-room" 
        << "https://example.com/my-room";
    
    // 完整的HTTPS URL
    QTest::newRow("full https url") 
        << "jitsi-meet://https://custom.server.com/room-name" 
        << "https://custom.server.com/room-name";
    
    // 完整的HTTP URL
    QTest::newRow("full http url") 
        << "jitsi-meet://http://localhost:8080/test" 
        << "http://localhost:8080/test";
    
    // 复杂房间名
    QTest::newRow("complex room name") 
        << "jitsi-meet://my-company.meeting.room_123" 
        << "https://meet.jit.si/my-company.meeting.room_123";
    
    // 带路径的服务器
    QTest::newRow("server with path") 
        << "jitsi-meet://server.com/path/to/room" 
        << "https://server.com/path/to/room";
}

void TestProtocolHandler::testParseProtocolUrl()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    
    QString result = m_protocolHandler->parseProtocolUrl(input);
    QCOMPARE(result, expected);
}

void TestProtocolHandler::testIsValidProtocolUrl_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<bool>("expected");
    
    // 有效的URL
    QTest::newRow("valid simple") << "jitsi-meet://test-room" << true;
    QTest::newRow("valid with server") << "jitsi-meet://example.com/room" << true;
    QTest::newRow("valid with https") << "jitsi-meet://https://server.com/room" << true;
    QTest::newRow("valid complex") << "jitsi-meet://my-room_123.test" << true;
    
    // 无效的URL
    QTest::newRow("empty") << "" << false;
    QTest::newRow("no protocol") << "test-room" << false;
    QTest::newRow("wrong protocol") << "http://test-room" << false;
    QTest::newRow("no room name") << "jitsi-meet://" << false;
    QTest::newRow("invalid characters") << "jitsi-meet://room with spaces" << false;
    QTest::newRow("invalid characters 2") << "jitsi-meet://room@#$%" << false;
}

void TestProtocolHandler::testIsValidProtocolUrl()
{
    QFETCH(QString, url);
    QFETCH(bool, expected);
    
    bool result = m_protocolHandler->isValidProtocolUrl(url);
    QCOMPARE(result, expected);
}

void TestProtocolHandler::testRegisterProtocol()
{
    // 测试协议注册
    bool result = m_protocolHandler->registerProtocol();
    
#ifdef Q_OS_WIN
    // 在Windows上应该成功
    QVERIFY(result);
#else
    // 在其他平台上可能不支持
    Q_UNUSED(result);
#endif
}

void TestProtocolHandler::testUnregisterProtocol()
{
    // 先注册
    m_protocolHandler->registerProtocol();
    
    // 然后注销（不应该崩溃）
    m_protocolHandler->unregisterProtocol();
    
    // 再次注销（不应该崩溃）
    m_protocolHandler->unregisterProtocol();
    
    QVERIFY(true); // 如果没有崩溃就算成功
}

void TestProtocolHandler::testExtractRoomInfo_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("expected");
    
    QTest::newRow("simple room") << "jitsi-meet://test-room" << "test-room";
    QTest::newRow("server and room") << "jitsi-meet://server.com/room" << "server.com/room";
    QTest::newRow("full url") << "jitsi-meet://https://server.com/room" << "https://server.com/room";
    QTest::newRow("no protocol") << "test-room" << "";
    QTest::newRow("empty") << "" << "";
}

void TestProtocolHandler::testExtractRoomInfo()
{
    QFETCH(QString, url);
    QFETCH(QString, expected);
    
    // 使用反射或友元函数来测试私有方法
    // 这里我们通过parseProtocolUrl间接测试extractRoomInfo
    if (url.startsWith(JitsiConstants::PROTOCOL_PREFIX)) {
        QString result = m_protocolHandler->parseProtocolUrl(url);
        QVERIFY(!result.isEmpty() || expected.isEmpty());
    } else {
        QString result = m_protocolHandler->parseProtocolUrl(url);
        QVERIFY(result.isEmpty());
    }
}

void TestProtocolHandler::testEmptyUrl()
{
    QString result = m_protocolHandler->parseProtocolUrl("");
    QVERIFY(result.isEmpty());
    
    bool valid = m_protocolHandler->isValidProtocolUrl("");
    QVERIFY(!valid);
}

void TestProtocolHandler::testInvalidUrls()
{
    QStringList invalidUrls = {
        "http://example.com",
        "https://example.com",
        "ftp://example.com",
        "jitsi-meet://",
        "jitsi-meet:// ",
        "jitsi-meet://room with spaces",
        "jitsi-meet://room@invalid",
        "jitsi-meet://room#invalid"
    };
    
    for (const QString& url : invalidUrls) {
        bool valid = m_protocolHandler->isValidProtocolUrl(url);
        QVERIFY2(!valid, qPrintable(QString("URL should be invalid: %1").arg(url)));
    }
}

void TestProtocolHandler::testSpecialCharacters()
{
    // 测试允许的特殊字符
    QStringList validUrls = {
        "jitsi-meet://room-name",
        "jitsi-meet://room_name",
        "jitsi-meet://room.name",
        "jitsi-meet://server.com/room-name",
        "jitsi-meet://room123",
        "jitsi-meet://123room"
    };
    
    for (const QString& url : validUrls) {
        bool valid = m_protocolHandler->isValidProtocolUrl(url);
        QVERIFY2(valid, qPrintable(QString("URL should be valid: %1").arg(url)));
    }
}

void TestProtocolHandler::testProtocolUrlReceivedSignal()
{
    QSignalSpy spy(m_protocolHandler, &ProtocolHandler::protocolUrlReceived);
    
    // 模拟信号发射
    QString testUrl = "jitsi-meet://test-room";
    emit m_protocolHandler->protocolUrlReceived(testUrl);
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), testUrl);
}

QTEST_MAIN(TestProtocolHandler)
#include "test_protocol_handler.moc"