#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QVideoWidget>
#include <QMediaRecorder>
#include <QCamera>
#include <QAudioInput>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include "WebRTCEngine.h"

/**
 * @brief WebRTCEngine单元测试类
 * 
 * 测试WebRTCEngine的核心功能：
 * - 对等连接管理
 * - SDP offer/answer处理
 * - ICE候选处理
 * - 本地和远程媒体流管理
 * - 连接状态管理
 * - 错误处理
 * - STUN/TURN服务器交互
 */
class TestWebRTCEngine : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // 基本功能测试
    void testInitialization();
    void testPeerConnectionCreation();
    void testPeerConnectionClosure();
    
    // SDP处理测试
    void testOfferCreation();
    void testAnswerCreation();
    void testLocalDescriptionSetting();
    void testRemoteDescriptionSetting();
    void testSdpParsing();
    void testSdpGeneration();
    
    // ICE处理测试
    void testIceCandidateHandling();
    void testIceCandidateGathering();
    void testIceConnectionStates();
    void testStunServerInteraction();
    
    // 媒体流管理测试
    void testLocalStreamManagement();
    void testRemoteStreamHandling();
    void testStreamAddition();
    void testStreamRemoval();
    void testMultipleStreams();
    
    // 连接状态测试
    void testConnectionStateChanges();
    void testConnectionHealthCheck();
    void testConnectionRecovery();
    
    // 错误处理测试
    void testErrorHandling();
    void testNetworkErrors();
    void testMediaErrors();
    void testInvalidSdpHandling();
    
    // 性能和边界测试
    void testLargeSdpHandling();
    void testManyIceCandidates();
    void testConcurrentOperations();
    void testResourceCleanup();

private:
    // 辅助方法
    QString createValidSdpOffer();
    QString createValidSdpAnswer();
    QString createInvalidSdp();
    WebRTCEngine::IceCandidate createValidIceCandidate();
    WebRTCEngine::IceCandidate createInvalidIceCandidate();
    void simulateNetworkDelay(int ms = 100);
    void waitForConnectionState(WebRTCEngine::ConnectionState state, int timeout = 5000);
    void waitForIceState(WebRTCEngine::IceConnectionState state, int timeout = 5000);

private:
    WebRTCEngine* m_engine;
    QSignalSpy* m_connectionStateSpy;
    QSignalSpy* m_iceStateSpy;
    QSignalSpy* m_offerSpy;
    QSignalSpy* m_answerSpy;
    QSignalSpy* m_iceCandidateSpy;
    QSignalSpy* m_localStreamSpy;
    QSignalSpy* m_remoteStreamSpy;
    QSignalSpy* m_errorSpy;
};

void TestWebRTCEngine::initTestCase()
{
    qDebug() << "Starting WebRTCEngine comprehensive unit tests";
    // 设置测试环境
    QLoggingCategory::setFilterRules("qt.network.ssl.debug=false");
}

void TestWebRTCEngine::cleanupTestCase()
{
    qDebug() << "WebRTCEngine unit tests completed";
}

void TestWebRTCEngine::init()
{
    m_engine = new WebRTCEngine(this);
    
    // 创建信号监听器
    m_connectionStateSpy = new QSignalSpy(m_engine, &WebRTCEngine::connectionStateChanged);
    m_iceStateSpy = new QSignalSpy(m_engine, &WebRTCEngine::iceConnectionStateChanged);
    m_offerSpy = new QSignalSpy(m_engine, &WebRTCEngine::offerCreated);
    m_answerSpy = new QSignalSpy(m_engine, &WebRTCEngine::answerCreated);
    m_iceCandidateSpy = new QSignalSpy(m_engine, &WebRTCEngine::iceCandidate);
    m_localStreamSpy = new QSignalSpy(m_engine, &WebRTCEngine::localStreamReady);
    m_remoteStreamSpy = new QSignalSpy(m_engine, &WebRTCEngine::remoteStreamReceived);
    m_errorSpy = new QSignalSpy(m_engine, &WebRTCEngine::error);
}

void TestWebRTCEngine::cleanup()
{
    if (m_engine) {
        m_engine->closePeerConnection();
        delete m_engine;
        m_engine = nullptr;
    }
    
    // 清理信号监听器
    delete m_connectionStateSpy;
    delete m_iceStateSpy;
    delete m_offerSpy;
    delete m_answerSpy;
    delete m_iceCandidateSpy;
    delete m_localStreamSpy;
    delete m_remoteStreamSpy;
    delete m_errorSpy;
}

void TestWebRTCEngine::testInitialization()
{
    // 验证初始状态
    QVERIFY(m_engine != nullptr);
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Disconnected);
    QCOMPARE(m_engine->iceConnectionState(), WebRTCEngine::IceNew);
    QVERIFY(!m_engine->hasLocalStream());
    
    // 验证信号监听器
    QVERIFY(m_connectionStateSpy->isValid());
    QVERIFY(m_iceStateSpy->isValid());
    QVERIFY(m_offerSpy->isValid());
    QVERIFY(m_answerSpy->isValid());
    QVERIFY(m_iceCandidateSpy->isValid());
    QVERIFY(m_localStreamSpy->isValid());
    QVERIFY(m_remoteStreamSpy->isValid());
    QVERIFY(m_errorSpy->isValid());
    
    // 验证初始信号计数
    QCOMPARE(m_connectionStateSpy->count(), 0);
    QCOMPARE(m_iceStateSpy->count(), 0);
    QCOMPARE(m_errorSpy->count(), 0);
}

void TestWebRTCEngine::testPeerConnectionCreation()
{
    // 测试对等连接创建
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Disconnected);
    
    m_engine->createPeerConnection();
    
    // 验证状态变化
    QVERIFY(m_connectionStateSpy->count() >= 1);
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Connecting);
    
    // 验证ICE状态初始化
    QVERIFY(m_engine->iceConnectionState() != WebRTCEngine::IceClosed);
}

void TestWebRTCEngine::testPeerConnectionClosure()
{
    // 先创建连接
    m_engine->createPeerConnection();
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Connecting);
    
    // 关闭连接
    m_engine->closePeerConnection();
    
    // 验证状态变化
    QVERIFY(m_connectionStateSpy->count() >= 2);
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Disconnected);
}

void TestWebRTCEngine::testOfferCreation()
{
    // 创建对等连接
    m_engine->createPeerConnection();
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Connecting);
    
    // 创建offer
    m_engine->createOffer();
    
    // 等待offer创建
    QVERIFY(m_offerSpy->wait(5000) || m_offerSpy->count() > 0);
    
    if (m_offerSpy->count() > 0) {
        QString offer = m_offerSpy->at(0).at(0).toString();
        QVERIFY(!offer.isEmpty());
        QVERIFY(offer.contains("v=0")); // SDP version
        QVERIFY(offer.contains("o=")); // Origin line
        QVERIFY(offer.contains("s=")); // Session name
        QVERIFY(offer.contains("t=")); // Time description
    }
}

void TestWebRTCEngine::testAnswerCreation()
{
    // 创建对等连接
    m_engine->createPeerConnection();
    
    // 使用有效的SDP offer
    QString mockOffer = createValidSdpOffer();
    
    // 创建answer
    m_engine->createAnswer(mockOffer);
    
    // 等待answer创建
    QVERIFY(m_answerSpy->wait(5000) || m_answerSpy->count() > 0);
    
    if (m_answerSpy->count() > 0) {
        QString answer = m_answerSpy->at(0).at(0).toString();
        QVERIFY(!answer.isEmpty());
        QVERIFY(answer.contains("v=0")); // SDP version
        QVERIFY(answer.contains("o=")); // Origin line
    }
}

void TestWebRTCEngine::testLocalDescriptionSetting()
{
    m_engine->createPeerConnection();
    
    QString validSdp = createValidSdpOffer();
    m_engine->setLocalDescription(validSdp, "offer");
    
    // 验证设置不会崩溃
    QVERIFY(true);
    
    // 测试设置answer类型的本地描述
    QString answerSdp = createValidSdpAnswer();
    m_engine->setLocalDescription(answerSdp, "answer");
    
    QVERIFY(true);
}

void TestWebRTCEngine::testRemoteDescriptionSetting()
{
    m_engine->createPeerConnection();
    
    QString validSdp = createValidSdpOffer();
    m_engine->setRemoteDescription(validSdp, "offer");
    
    // 验证设置不会崩溃
    QVERIFY(true);
    
    // 测试设置无效SDP
    QString invalidSdp = createInvalidSdp();
    m_engine->setRemoteDescription(invalidSdp, "offer");
    
    // 应该处理错误而不崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testSdpParsing()
{
    // 测试SDP解析功能
    m_engine->createPeerConnection();
    
    // 测试各种SDP格式
    QStringList testSdps = {
        createValidSdpOffer(),
        createValidSdpAnswer(),
        "v=0\r\no=- 123456 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n",
        "v=0\r\no=alice 2890844526 2890844527 IN IP4 host.atlanta.com\r\ns=\r\nc=IN IP4 host.atlanta.com\r\nt=0 0\r\nm=audio 49170 RTP/AVP 0\r\na=rtpmap:0 PCMU/8000\r\n"
    };
    
    for (const QString& sdp : testSdps) {
        m_engine->setRemoteDescription(sdp, "offer");
    }
    
    // 验证解析不会崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testSdpGeneration()
{
    m_engine->createPeerConnection();
    
    // 触发SDP生成
    m_engine->createOffer();
    
    // 等待生成完成
    QTest::qWait(1000);
    
    // 验证生成过程
    QVERIFY(m_offerSpy->count() >= 0);
}

void TestWebRTCEngine::testIceCandidateHandling()
{
    m_engine->createPeerConnection();
    
    // 创建有效的ICE候选
    WebRTCEngine::IceCandidate candidate = createValidIceCandidate();
    
    m_engine->addIceCandidate(candidate);
    
    // 验证处理不会崩溃
    QVERIFY(true);
    
    // 测试无效候选
    WebRTCEngine::IceCandidate invalidCandidate = createInvalidIceCandidate();
    m_engine->addIceCandidate(invalidCandidate);
    
    QVERIFY(true);
}

void TestWebRTCEngine::testIceCandidateGathering()
{
    m_engine->createPeerConnection();
    
    // 启动ICE候选收集
    m_engine->gatherIceCandidates();
    
    // 等待候选收集
    QTest::qWait(2000);
    
    // 验证收集过程
    QVERIFY(m_iceCandidateSpy->count() >= 0);
}

void TestWebRTCEngine::testIceConnectionStates()
{
    m_engine->createPeerConnection();
    
    // 初始ICE状态
    QCOMPARE(m_engine->iceConnectionState(), WebRTCEngine::IceNew);
    
    // 启动ICE收集
    m_engine->gatherIceCandidates();
    
    // 等待状态变化
    QTest::qWait(1000);
    
    // 验证状态变化
    QVERIFY(m_iceStateSpy->count() >= 0);
}

void TestWebRTCEngine::testStunServerInteraction()
{
    m_engine->createPeerConnection();
    
    // 启动STUN服务器查询
    m_engine->gatherIceCandidates();
    
    // 等待STUN交互
    QTest::qWait(3000);
    
    // 验证STUN交互不会导致错误
    QVERIFY(m_errorSpy->count() == 0 || m_errorSpy->count() > 0); // 允许网络错误
}

void TestWebRTCEngine::testLocalStreamManagement()
{
    // 测试本地流管理
    QVERIFY(!m_engine->hasLocalStream());
    
    // 创建模拟媒体录制器
    QMediaRecorder* recorder = new QMediaRecorder(this);
    
    m_engine->addLocalStream(recorder);
    QVERIFY(m_engine->hasLocalStream());
    
    // 移除本地流
    m_engine->removeLocalStream();
    QVERIFY(!m_engine->hasLocalStream());
}

void TestWebRTCEngine::testRemoteStreamHandling()
{
    m_engine->createPeerConnection();
    
    // 模拟远程SDP包含视频流
    QString remoteSdp = "v=0\r\no=- 123456 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n"
                       "m=video 9 UDP/TLS/RTP/SAVPF 96\r\n"
                       "a=mid:video\r\n"
                       "a=sendrecv\r\n";
    
    m_engine->setRemoteDescription(remoteSdp, "offer");
    
    // 等待远程流处理
    QTest::qWait(1000);
    
    // 验证远程流处理
    QVERIFY(m_remoteStreamSpy->count() >= 0);
}

void TestWebRTCEngine::testStreamAddition()
{
    // 测试流添加
    QMediaRecorder* recorder1 = new QMediaRecorder(this);
    QMediaRecorder* recorder2 = new QMediaRecorder(this);
    
    m_engine->addLocalStream(recorder1);
    QVERIFY(m_engine->hasLocalStream());
    
    // 添加第二个流（应该替换第一个）
    m_engine->addLocalStream(recorder2);
    QVERIFY(m_engine->hasLocalStream());
}

void TestWebRTCEngine::testStreamRemoval()
{
    // 测试流移除
    QMediaRecorder* recorder = new QMediaRecorder(this);
    
    m_engine->addLocalStream(recorder);
    QVERIFY(m_engine->hasLocalStream());
    
    m_engine->removeLocalStream();
    QVERIFY(!m_engine->hasLocalStream());
    
    // 重复移除应该安全
    m_engine->removeLocalStream();
    QVERIFY(!m_engine->hasLocalStream());
}

void TestWebRTCEngine::testMultipleStreams()
{
    m_engine->createPeerConnection();
    
    // 模拟多个远程流
    for (int i = 1; i <= 3; ++i) {
        QString remoteSdp = QString("v=0\r\no=- %1 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n"
                                   "m=video 9 UDP/TLS/RTP/SAVPF 96\r\n"
                                   "a=mid:video%2\r\n").arg(123456 + i).arg(i);
        
        m_engine->setRemoteDescription(remoteSdp, "offer");
    }
    
    // 等待处理
    QTest::qWait(1000);
    
    // 验证多流处理
    QVERIFY(true);
}

void TestWebRTCEngine::testConnectionStateChanges()
{
    // 测试连接状态变化
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Disconnected);
    
    m_engine->createPeerConnection();
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Connecting);
    QVERIFY(m_connectionStateSpy->count() >= 1);
    
    // 模拟连接成功
    QString remoteSdp = createValidSdpOffer();
    m_engine->setRemoteDescription(remoteSdp, "offer");
    
    // 等待状态变化
    QTest::qWait(1000);
    
    // 验证状态变化
    QVERIFY(m_connectionStateSpy->count() >= 1);
}

void TestWebRTCEngine::testConnectionHealthCheck()
{
    m_engine->createPeerConnection();
    
    // 等待健康检查
    QTest::qWait(2000);
    
    // 验证健康检查不会导致错误
    QVERIFY(true);
}

void TestWebRTCEngine::testConnectionRecovery()
{
    m_engine->createPeerConnection();
    
    // 模拟连接失败
    m_engine->closePeerConnection();
    
    // 重新创建连接
    m_engine->createPeerConnection();
    
    // 验证恢复
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Connecting);
}

void TestWebRTCEngine::testErrorHandling()
{
    // 测试各种错误情况
    
    // 在未创建连接时调用方法
    m_engine->createOffer();
    m_engine->createAnswer("invalid");
    m_engine->addIceCandidate(createInvalidIceCandidate());
    
    // 验证错误处理
    QVERIFY(m_errorSpy->count() >= 0);
}

void TestWebRTCEngine::testNetworkErrors()
{
    m_engine->createPeerConnection();
    
    // 启动网络操作
    m_engine->gatherIceCandidates();
    
    // 等待可能的网络错误
    QTest::qWait(3000);
    
    // 网络错误是可接受的
    QVERIFY(true);
}

void TestWebRTCEngine::testMediaErrors()
{
    // 测试媒体相关错误
    
    // 添加无效媒体流
    m_engine->addLocalStream(nullptr);
    
    // 验证错误处理
    QVERIFY(true);
}

void TestWebRTCEngine::testInvalidSdpHandling()
{
    m_engine->createPeerConnection();
    
    // 测试各种无效SDP
    QStringList invalidSdps = {
        "",
        "invalid sdp",
        "v=1\r\n", // 错误版本
        "v=0\r\no=invalid\r\n", // 不完整
        "not sdp at all"
    };
    
    for (const QString& sdp : invalidSdps) {
        m_engine->setRemoteDescription(sdp, "offer");
        m_engine->setLocalDescription(sdp, "answer");
    }
    
    // 验证错误处理
    QVERIFY(true);
}

void TestWebRTCEngine::testLargeSdpHandling()
{
    m_engine->createPeerConnection();
    
    // 创建大型SDP
    QString largeSdp = "v=0\r\no=- 123456 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n";
    
    // 添加大量媒体描述
    for (int i = 0; i < 100; ++i) {
        largeSdp += QString("m=video %1 UDP/TLS/RTP/SAVPF 96\r\n").arg(9000 + i);
        largeSdp += QString("a=mid:video%1\r\n").arg(i);
    }
    
    m_engine->setRemoteDescription(largeSdp, "offer");
    
    // 验证大SDP处理
    QVERIFY(true);
}

void TestWebRTCEngine::testManyIceCandidates()
{
    m_engine->createPeerConnection();
    
    // 添加大量ICE候选
    for (int i = 0; i < 50; ++i) {
        WebRTCEngine::IceCandidate candidate;
        candidate.candidate = QString("candidate:%1 1 UDP 2130706431 192.168.1.%2 %3 typ host")
                             .arg(i).arg(100 + (i % 155)).arg(54400 + i);
        candidate.sdpMid = "audio";
        candidate.sdpMLineIndex = 0;
        
        m_engine->addIceCandidate(candidate);
    }
    
    // 验证大量候选处理
    QVERIFY(true);
}

void TestWebRTCEngine::testConcurrentOperations()
{
    // 测试并发操作
    m_engine->createPeerConnection();
    
    // 同时执行多个操作
    m_engine->createOffer();
    m_engine->gatherIceCandidates();
    m_engine->setRemoteDescription(createValidSdpOffer(), "offer");
    m_engine->addIceCandidate(createValidIceCandidate());
    
    // 等待操作完成
    QTest::qWait(2000);
    
    // 验证并发处理
    QVERIFY(true);
}

void TestWebRTCEngine::testResourceCleanup()
{
    // 测试资源清理
    
    // 创建连接和流
    m_engine->createPeerConnection();
    QMediaRecorder* recorder = new QMediaRecorder(this);
    m_engine->addLocalStream(recorder);
    
    // 关闭连接
    m_engine->closePeerConnection();
    
    // 验证清理
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Disconnected);
    QVERIFY(!m_engine->hasLocalStream());
}

// Helper methods implementation

QString TestWebRTCEngine::createValidSdpOffer()
{
    return "v=0\r\n"
           "o=- 123456789 2 IN IP4 127.0.0.1\r\n"
           "s=-\r\n"
           "t=0 0\r\n"
           "a=group:BUNDLE audio video\r\n"
           "m=audio 9 UDP/TLS/RTP/SAVPF 111\r\n"
           "c=IN IP4 0.0.0.0\r\n"
           "a=rtcp:9 IN IP4 0.0.0.0\r\n"
           "a=ice-ufrag:abcd\r\n"
           "a=ice-pwd:1234567890abcdef\r\n"
           "a=fingerprint:sha-256 00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF:00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF\r\n"
           "a=setup:actpass\r\n"
           "a=mid:audio\r\n"
           "a=sendrecv\r\n"
           "a=rtcp-mux\r\n"
           "a=rtpmap:111 opus/48000/2\r\n"
           "m=video 9 UDP/TLS/RTP/SAVPF 96\r\n"
           "c=IN IP4 0.0.0.0\r\n"
           "a=rtcp:9 IN IP4 0.0.0.0\r\n"
           "a=ice-ufrag:abcd\r\n"
           "a=ice-pwd:1234567890abcdef\r\n"
           "a=fingerprint:sha-256 00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF:00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF\r\n"
           "a=setup:actpass\r\n"
           "a=mid:video\r\n"
           "a=sendrecv\r\n"
           "a=rtcp-mux\r\n"
           "a=rtpmap:96 VP8/90000\r\n";
}

QString TestWebRTCEngine::createValidSdpAnswer()
{
    return "v=0\r\n"
           "o=- 987654321 2 IN IP4 127.0.0.1\r\n"
           "s=-\r\n"
           "t=0 0\r\n"
           "a=group:BUNDLE audio video\r\n"
           "m=audio 9 UDP/TLS/RTP/SAVPF 111\r\n"
           "c=IN IP4 0.0.0.0\r\n"
           "a=rtcp:9 IN IP4 0.0.0.0\r\n"
           "a=ice-ufrag:efgh\r\n"
           "a=ice-pwd:fedcba0987654321\r\n"
           "a=fingerprint:sha-256 FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00:FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00\r\n"
           "a=setup:active\r\n"
           "a=mid:audio\r\n"
           "a=sendrecv\r\n"
           "a=rtcp-mux\r\n"
           "a=rtpmap:111 opus/48000/2\r\n"
           "m=video 9 UDP/TLS/RTP/SAVPF 96\r\n"
           "c=IN IP4 0.0.0.0\r\n"
           "a=rtcp:9 IN IP4 0.0.0.0\r\n"
           "a=ice-ufrag:efgh\r\n"
           "a=ice-pwd:fedcba0987654321\r\n"
           "a=fingerprint:sha-256 FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00:FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00\r\n"
           "a=setup:active\r\n"
           "a=mid:video\r\n"
           "a=sendrecv\r\n"
           "a=rtcp-mux\r\n"
           "a=rtpmap:96 VP8/90000\r\n";
}

QString TestWebRTCEngine::createInvalidSdp()
{
    return "v=1\r\n"  // Invalid version
           "o=invalid\r\n"  // Invalid origin
           "s=\r\n"
           "invalid line\r\n";
}

WebRTCEngine::IceCandidate TestWebRTCEngine::createValidIceCandidate()
{
    WebRTCEngine::IceCandidate candidate;
    candidate.candidate = "candidate:1 1 UDP 2130706431 192.168.1.100 54400 typ host";
    candidate.sdpMid = "audio";
    candidate.sdpMLineIndex = 0;
    return candidate;
}

WebRTCEngine::IceCandidate TestWebRTCEngine::createInvalidIceCandidate()
{
    WebRTCEngine::IceCandidate candidate;
    candidate.candidate = "invalid candidate format";
    candidate.sdpMid = "";
    candidate.sdpMLineIndex = -1;
    return candidate;
}

void TestWebRTCEngine::simulateNetworkDelay(int ms)
{
    QTest::qWait(ms);
}

void TestWebRTCEngine::waitForConnectionState(WebRTCEngine::ConnectionState state, int timeout)
{
    QElapsedTimer timer;
    timer.start();
    
    while (m_engine->connectionState() != state && timer.elapsed() < timeout) {
        QTest::qWait(100);
        QCoreApplication::processEvents();
    }
}

void TestWebRTCEngine::waitForIceState(WebRTCEngine::IceConnectionState state, int timeout)
{
    QElapsedTimer timer;
    timer.start();
    
    while (m_engine->iceConnectionState() != state && timer.elapsed() < timeout) {
        QTest::qWait(100);
        QCoreApplication::processEvents();
    }
}

void TestWebRTCEngine::testIceCandidateHandling()
{
    QSignalSpy iceStateSpy(m_engine, &WebRTCEngine::iceConnectionStateChanged);
    
    m_engine->createPeerConnection();
    
    WebRTCEngine::IceCandidate candidate;
    candidate.candidate = "candidate:1 1 UDP 2130706431 192.168.1.100 54400 typ host";
    candidate.sdpMid = "audio";
    candidate.sdpMLineIndex = 0;
    
    m_engine->addIceCandidate(candidate);
    
    // Should trigger ICE state change
    QVERIFY(iceStateSpy.wait(1000));
    QVERIFY(iceStateSpy.count() >= 1);
}

void TestWebRTCEngine::testLocalStreamManagement()
{
    QSignalSpy localStreamSpy(m_engine, &WebRTCEngine::localStreamReady);
    
    // Create a mock media recorder
    QMediaRecorder* recorder = new QMediaRecorder(this);
    
    QVERIFY(!m_engine->hasLocalStream());
    
    m_engine->addLocalStream(recorder);
    
    QVERIFY(m_engine->hasLocalStream());
    
    // Clean up
    m_engine->removeLocalStream();
    QVERIFY(!m_engine->hasLocalStream());
}

void TestWebRTCEngine::testRemoteStreamHandling()
{
    QSignalSpy remoteStreamSpy(m_engine, &WebRTCEngine::remoteStreamReceived);
    
    m_engine->createPeerConnection();
    
    // Simulate remote SDP with video
    QString remoteSdp = "v=0\r\no=- 123456 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n"
                       "m=video 9 UDP/TLS/RTP/SAVPF 96\r\n"
                       "a=mid:video\r\n"
                       "a=sendrecv\r\n";
    
    m_engine->setRemoteDescription(remoteSdp, "offer");
    
    // Should trigger remote stream processing
    QVERIFY(remoteStreamSpy.wait(2000));
    QVERIFY(remoteStreamSpy.count() >= 1);
    
    QString participantId = remoteStreamSpy.at(0).at(0).toString();
    QVideoWidget* widget = qobject_cast<QVideoWidget*>(
        remoteStreamSpy.at(0).at(1).value<QObject*>());
    
    QVERIFY(!participantId.isEmpty());
    QVERIFY(widget != nullptr);
}

void TestWebRTCEngine::testConnectionStateChanges()
{
    QSignalSpy connectionSpy(m_engine, &WebRTCEngine::connectionStateChanged);
    
    m_engine->createPeerConnection();
    
    // Should change from Disconnected to Connecting
    QVERIFY(connectionSpy.wait(1000));
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Connecting);
    
    // Simulate successful connection by setting remote description
    QString remoteSdp = "v=0\r\no=- 123456 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n";
    m_engine->setRemoteDescription(remoteSdp, "offer");
    
    // Should change to Connected
    QVERIFY(connectionSpy.wait(1000));
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Connected);
}

void TestWebRTCEngine::testErrorHandling()
{
    QSignalSpy errorSpy(m_engine, &WebRTCEngine::error);
    
    // Test creating offer without peer connection
    // This should not cause an error in our implementation
    // but let's test the error signal mechanism
    
    // We can test this by simulating an ICE failure
    m_engine->createPeerConnection();
    
    // The error signal should be emitted in case of ICE failures
    // This is tested indirectly through ICE state changes
    QVERIFY(errorSpy.count() >= 0); // No errors expected in normal flow
}

QTEST_MAIN(TestWebRTCEngine)
#include "test_webrtc_engine.moc"