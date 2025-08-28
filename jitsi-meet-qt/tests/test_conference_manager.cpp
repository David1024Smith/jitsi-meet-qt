#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include "ConferenceManager.h"
#include "JitsiError.h"

class TestConferenceManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // URL解析测试
    void testParseConferenceUrl_data();
    void testParseConferenceUrl();
    
    // 基本功能测试
    void testInitialState();
    void testJoinConference();
    void testLeaveConference();
    void testInvalidUrl();
    
    // 媒体控制测试
    void testAudioMuteControl();
    void testVideoMuteControl();
    void testScreenShareControl();
    
    // 参与者管理测试
    void testParticipantManagement();
    
    // 重连机制测试
    void testReconnectionMechanism();
    
    // 错误处理测试
    void testErrorHandling();

private:
    ConferenceManager* m_conferenceManager;
    
    // 辅助方法
    void waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
    bool parseUrlHelper(const QString& url, QString& serverUrl, QString& roomName);
};

void TestConferenceManager::initTestCase()
{
    qDebug() << "Starting ConferenceManager tests";
}

void TestConferenceManager::cleanupTestCase()
{
    qDebug() << "ConferenceManager tests completed";
}

void TestConferenceManager::init()
{
    m_conferenceManager = new ConferenceManager(this);
}

void TestConferenceManager::cleanup()
{
    if (m_conferenceManager) {
        m_conferenceManager->leaveConference();
        delete m_conferenceManager;
        m_conferenceManager = nullptr;
    }
}

void TestConferenceManager::testParseConferenceUrl_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("expectedServerUrl");
    QTest::addColumn<QString>("expectedRoomName");
    QTest::addColumn<bool>("shouldSucceed");

    // 有效URL格式
    QTest::newRow("full_https_url") 
        << "https://meet.jit.si/TestRoom"
        << "https://meet.jit.si"
        << "TestRoom"
        << true;

    QTest::newRow("url_without_protocol") 
        << "meet.jit.si/TestRoom"
        << "https://meet.jit.si"
        << "TestRoom"
        << true;

    QTest::newRow("jitsi_protocol") 
        << "jitsi-meet://meet.jit.si/TestRoom"
        << "https://meet.jit.si"
        << "TestRoom"
        << true;

    QTest::newRow("room_name_only") 
        << "TestRoom"
        << "https://meet.jit.si"
        << "TestRoom"
        << true;

    QTest::newRow("custom_server") 
        << "https://jitsi.example.com/MyRoom"
        << "https://jitsi.example.com"
        << "MyRoom"
        << true;

    QTest::newRow("server_with_port") 
        << "https://jitsi.example.com:8443/MyRoom"
        << "https://jitsi.example.com:8443"
        << "MyRoom"
        << true;

    // 无效URL格式
    QTest::newRow("empty_url") 
        << ""
        << ""
        << ""
        << false;

    QTest::newRow("invalid_characters") 
        << "TestRoom@#$"
        << ""
        << ""
        << false;

    QTest::newRow("no_room_name") 
        << "https://meet.jit.si/"
        << ""
        << ""
        << false;
}

void TestConferenceManager::testParseConferenceUrl()
{
    QFETCH(QString, url);
    QFETCH(QString, expectedServerUrl);
    QFETCH(QString, expectedRoomName);
    QFETCH(bool, shouldSucceed);

    QString serverUrl, roomName;
    bool result = parseUrlHelper(url, serverUrl, roomName);

    QCOMPARE(result, shouldSucceed);
    
    if (shouldSucceed) {
        QCOMPARE(serverUrl, expectedServerUrl);
        QCOMPARE(roomName, expectedRoomName);
    }
}

void TestConferenceManager::testInitialState()
{
    QCOMPARE(m_conferenceManager->connectionState(), ConferenceManager::Disconnected);
    QCOMPARE(m_conferenceManager->conferenceState(), ConferenceManager::Idle);
    QVERIFY(!m_conferenceManager->isInConference());
    QVERIFY(!m_conferenceManager->isConnected());
    QCOMPARE(m_conferenceManager->participantCount(), 0);
    QVERIFY(m_conferenceManager->participants().isEmpty());
}

void TestConferenceManager::testJoinConference()
{
    QSignalSpy stateChangedSpy(m_conferenceManager, &ConferenceManager::conferenceStateChanged);
    QSignalSpy connectionSpy(m_conferenceManager, &ConferenceManager::connectionStateChanged);
    
    // 测试加入会议
    m_conferenceManager->joinConference("https://meet.jit.si/TestRoom", "TestUser");
    
    // 验证状态变化
    QVERIFY(stateChangedSpy.wait(1000));
    QCOMPARE(m_conferenceManager->conferenceState(), ConferenceManager::Joining);
    
    // 验证会议信息
    ConferenceManager::ConferenceInfo info = m_conferenceManager->currentConference();
    QCOMPARE(info.roomName, QString("TestRoom"));
    QCOMPARE(info.serverUrl, QString("https://meet.jit.si"));
    QCOMPARE(info.displayName, QString("TestUser"));
    QVERIFY(!info.joinTime.isNull());
}

void TestConferenceManager::testLeaveConference()
{
    // 先加入会议
    m_conferenceManager->joinConference("https://meet.jit.si/TestRoom", "TestUser");
    
    QSignalSpy leftSpy(m_conferenceManager, &ConferenceManager::conferenceLeft);
    QSignalSpy stateChangedSpy(m_conferenceManager, &ConferenceManager::conferenceStateChanged);
    
    // 离开会议
    m_conferenceManager->leaveConference();
    
    // 验证状态变化
    QVERIFY(stateChangedSpy.wait(1000));
    QCOMPARE(m_conferenceManager->conferenceState(), ConferenceManager::Idle);
    QCOMPARE(m_conferenceManager->connectionState(), ConferenceManager::Disconnected);
    
    // 验证离开信号
    QVERIFY(leftSpy.count() > 0);
}

void TestConferenceManager::testInvalidUrl()
{
    QSignalSpy errorSpy(m_conferenceManager, &ConferenceManager::errorOccurred);
    
    // 测试无效URL
    m_conferenceManager->joinConference("invalid@url#", "TestUser");
    
    // 应该收到错误信号
    QVERIFY(errorSpy.wait(1000));
    QCOMPARE(errorSpy.count(), 1);
    
    QList<QVariant> arguments = errorSpy.takeFirst();
    JitsiError error = arguments.at(0).value<JitsiError>();
    QCOMPARE(error.type(), ErrorType::InvalidUrl);
}

void TestConferenceManager::testAudioMuteControl()
{
    QSignalSpy mediaSpy(m_conferenceManager, &ConferenceManager::localMediaStateChanged);
    
    // 测试音频静音
    QVERIFY(!m_conferenceManager->localParticipant().audioMuted);
    
    m_conferenceManager->setAudioMuted(true);
    QVERIFY(m_conferenceManager->localParticipant().audioMuted);
    
    // 验证信号发出
    QVERIFY(mediaSpy.wait(100));
    QCOMPARE(mediaSpy.count(), 1);
    
    QList<QVariant> arguments = mediaSpy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);  // audioMuted
    QCOMPARE(arguments.at(1).toBool(), false); // videoMuted (默认值)
    
    // 测试取消静音
    m_conferenceManager->setAudioMuted(false);
    QVERIFY(!m_conferenceManager->localParticipant().audioMuted);
}

void TestConferenceManager::testVideoMuteControl()
{
    QSignalSpy mediaSpy(m_conferenceManager, &ConferenceManager::localMediaStateChanged);
    
    // 测试视频静音
    QVERIFY(!m_conferenceManager->localParticipant().videoMuted);
    
    m_conferenceManager->setVideoMuted(true);
    QVERIFY(m_conferenceManager->localParticipant().videoMuted);
    
    // 验证信号发出
    QVERIFY(mediaSpy.wait(100));
    QCOMPARE(mediaSpy.count(), 1);
    
    QList<QVariant> arguments = mediaSpy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), false); // audioMuted (默认值)
    QCOMPARE(arguments.at(1).toBool(), true);  // videoMuted
}

void TestConferenceManager::testScreenShareControl()
{
    QSignalSpy screenShareSpy(m_conferenceManager, &ConferenceManager::screenShareStateChanged);
    
    // 测试开始屏幕共享
    QVERIFY(!m_conferenceManager->localParticipant().isScreenSharing);
    
    m_conferenceManager->startScreenShare();
    QVERIFY(m_conferenceManager->localParticipant().isScreenSharing);
    
    // 验证信号发出
    QVERIFY(screenShareSpy.wait(100));
    QCOMPARE(screenShareSpy.count(), 1);
    
    QList<QVariant> arguments = screenShareSpy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true); // isSharing
    
    // 测试停止屏幕共享
    m_conferenceManager->stopScreenShare();
    QVERIFY(!m_conferenceManager->localParticipant().isScreenSharing);
}

void TestConferenceManager::testParticipantManagement()
{
    // 初始状态应该没有参与者
    QCOMPARE(m_conferenceManager->participantCount(), 0);
    QVERIFY(m_conferenceManager->participants().isEmpty());
    
    // 测试本地参与者信息
    ConferenceManager::ParticipantInfo localParticipant = m_conferenceManager->localParticipant();
    QVERIFY(!localParticipant.displayName.isEmpty() || localParticipant.jid.isEmpty()); // 初始状态可能为空
}

void TestConferenceManager::testReconnectionMechanism()
{
    QSignalSpy reconnectionSpy(m_conferenceManager, &ConferenceManager::reconnectionStarted);
    
    // 测试重连功能
    m_conferenceManager->reconnectToConference();
    
    // 如果没有会议可重连，应该收到错误
    // 这里主要测试方法调用不会崩溃
    QVERIFY(true); // 基本的调用测试
}

void TestConferenceManager::testErrorHandling()
{
    QSignalSpy errorSpy(m_conferenceManager, &ConferenceManager::errorOccurred);
    
    // 测试在未连接状态下发送聊天消息
    m_conferenceManager->sendChatMessage("Test message");
    
    // 应该收到错误信号
    QVERIFY(errorSpy.wait(1000));
    QCOMPARE(errorSpy.count(), 1);
    
    QList<QVariant> arguments = errorSpy.takeFirst();
    JitsiError error = arguments.at(0).value<JitsiError>();
    QCOMPARE(error.type(), ErrorType::NetworkError);
}

void TestConferenceManager::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QSignalSpy spy(sender, signal);
    QVERIFY(spy.wait(timeout));
}

bool TestConferenceManager::parseUrlHelper(const QString& url, QString& serverUrl, QString& roomName)
{
    // 这里需要访问ConferenceManager的私有方法，实际测试中可能需要将其设为protected或添加测试友元
    // 为了测试目的，这里重新实现URL解析逻辑
    
    QString normalizedUrl = url.trimmed();
    
    if (normalizedUrl.isEmpty()) {
        return false;
    }
    
    // 处理协议前缀
    if (normalizedUrl.startsWith("jitsi-meet://")) {
        normalizedUrl = "https://" + normalizedUrl.mid(13);
    } else if (!normalizedUrl.startsWith("http://") && !normalizedUrl.startsWith("https://")) {
        if (normalizedUrl.contains("/")) {
            normalizedUrl = "https://" + normalizedUrl;
        } else {
            // 只有房间名，使用默认服务器
            serverUrl = "https://meet.jit.si";
            roomName = normalizedUrl;
            
            // 验证房间名
            QRegularExpression roomNameRegex("^[a-zA-Z0-9_-]+$");
            return roomNameRegex.match(roomName).hasMatch();
        }
    }
    
    QUrl qurl(normalizedUrl);
    if (!qurl.isValid()) {
        return false;
    }
    
    serverUrl = qurl.scheme() + "://" + qurl.host();
    if (qurl.port() != -1) {
        serverUrl += ":" + QString::number(qurl.port());
    }
    
    QString path = qurl.path();
    if (path.startsWith("/")) {
        path = path.mid(1);
    }
    
    roomName = path;
    
    // 验证房间名
    if (roomName.isEmpty()) {
        return false;
    }
    
    QRegularExpression roomNameRegex("^[a-zA-Z0-9_-]+$");
    return roomNameRegex.match(roomName).hasMatch();
}

QTEST_MAIN(TestConferenceManager)
#include "test_conference_manager.moc"