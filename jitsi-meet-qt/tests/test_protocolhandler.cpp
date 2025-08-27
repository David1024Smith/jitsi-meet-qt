#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
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
    
    // 测试协议URL验证
    void testIsValidProtocolUrl_data();
    void testIsValidProtocolUrl();
    
    // 测试协议URL解析
    void testParseProtocolUrl_data();
    void testParseProtocolUrl();
    
    // 测试协议注册
    void testRegisterProtocol();
    void testUnregisterProtocol();
    
    // 测试边界情况
    void testEmptyUrl();
    void testInvalidUrls();
    void testSpecialCharacters();

private:
    ProtocolHandler* m_handler;
};

void TestProtocolHandler::initTestCase()
{
    // 测试开始前的全局初始化
}

void TestProtocolHandler::cleanupTestCase()
{
    // 测试结束后的全局清理
}

void TestProtocolHandler::init()
{
    // 每个测试前的初始化
    m_handler = new ProtocolHandler();
}

void TestProtocolHandler::cleanup()
{
    // 每个测试后的清理
    delete m_handler;
    m_handler = nullptr;
}

void TestProtocolHandler::testIsValidProtocolUrl_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<bool>("expected");
    
    // 有效的协议URL
    QTest::newRow("simple room") << "jitsi-meet://test-room" << true;
    QTest::newRow("room with server") << "jitsi-meet://meet.example.com/test-room" << true;
    QTest::newRow("room with numbers") << "jitsi-meet://room123" << true;
    QTest::newRow("room with underscores") << "jitsi-meet://test_room_123" << true;
    QTest::newRow("room with dots") << "jitsi-meet://test.room" << true;
    QTest::newRow("full https url") << "jitsi-meet://https://meet.jit.si/test-room" << true;
    
    // 无效的协议URL
    QTest::newRow("empty") << "" << false;
    QTest::newRow("wrong protocol") << "http://test-room" << false;
    QTest::newRow("no room name") << "jitsi-meet://" << false;
    QTest::newRow("invalid characters") << "jitsi-meet://test room" << false;
    QTest::newRow("special chars") << "jitsi-meet://test@room" << false;
}

void TestProtocolHandler::testIsValidProtocolUrl()
{
    QFETCH(QString, url);
    QFETCH(bool, expected);
    
    bool result = m_handler->isValidProtocolUrl(url);
    QCOMPARE(result, expected);
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
    QTest::newRow("room with server") 
        << "jitsi-meet://meet.example.com/test-room" 
        << "https://meet.example.com/test-room";
    
    // 完整的HTTPS URL
    QTest::newRow("full https url") 
        << "jitsi-meet://https://custom.server.com/my-room" 
        << "https://custom.server.com/my-room";
    
    // 复杂房间名
    QTest::newRow("complex room name") 
        << "jitsi-meet://my_test.room-123" 
        << "https://meet.jit.si/my_test.room-123";
    
    // 无效URL应该返回空字符串
    QTest::newRow("invalid url") 
        << "invalid://test" 
        << "";
    
    QTest::newRow("empty url") 
        << "" 
        << "";
}

void TestProtocolHandler::testParseProtocolUrl()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    
    QString result = m_handler->parseProtocolUrl(input);
    QCOMPARE(result, expected);
}

void TestProtocolHandler::testRegisterProtocol()
{
    // 测试协议注册
    bool result = m_handler->registerProtocol();
    
#ifdef Q_OS_WIN
    // 在Windows上应该成功注册
    QVERIFY(result);
#else
    // 在其他平台上可能不支持
    Q_UNUSED(result);
#endif
}

void TestProtocolHandler::testUnregisterProtocol()
{
    // 先注册协议
    m_handler->registerProtocol();
    
    // 然后注销协议（不应该崩溃）
    m_handler->unregisterProtocol();
    
    // 再次注销（不应该崩溃）
    m_handler->unregisterProtocol();
}

void TestProtocolHandler::testEmptyUrl()
{
    QString result = m_handler->parseProtocolUrl("");
    QVERIFY(result.isEmpty());
    
    bool valid = m_handler->isValidProtocolUrl("");
    QVERIFY(!valid);
}

void TestProtocolHandler::testInvalidUrls()
{
    QStringList invalidUrls = {
        "http://test.com",
        "ftp://test.com",
        "jitsi-meet://",
        "jitsi-meet://test room",  // 空格
        "jitsi-meet://test@room",  // @符号
        "jitsi-meet://test#room",  // #符号
        "jitsi-meet://test?room",  // ?符号
    };
    
    for (const QString& url : invalidUrls) {
        bool valid = m_handler->isValidProtocolUrl(url);
        QVERIFY2(!valid, qPrintable(QString("URL should be invalid: %1").arg(url)));
        
        QString parsed = m_handler->parseProtocolUrl(url);
        QVERIFY2(parsed.isEmpty(), qPrintable(QString("Parsed result should be empty for: %1").arg(url)));
    }
}

void TestProtocolHandler::testSpecialCharacters()
{
    // 测试允许的特殊字符
    QStringList validUrls = {
        "jitsi-meet://test-room",      // 连字符
        "jitsi-meet://test_room",      // 下划线
        "jitsi-meet://test.room",      // 点号
        "jitsi-meet://test123",        // 数字
        "jitsi-meet://server.com/room", // 斜杠
    };
    
    for (const QString& url : validUrls) {
        bool valid = m_handler->isValidProtocolUrl(url);
        QVERIFY2(valid, qPrintable(QString("URL should be valid: %1").arg(url)));
        
        QString parsed = m_handler->parseProtocolUrl(url);
        QVERIFY2(!parsed.isEmpty(), qPrintable(QString("Parsed result should not be empty for: %1").arg(url)));
    }
}

QTEST_MAIN(TestProtocolHandler)
#include "test_protocolhandler.moc"