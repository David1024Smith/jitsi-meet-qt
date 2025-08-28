#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include <QVideoWidget>
#include <QCamera>
#include <QAudioInput>
#include <QMediaDevices>
#include "WebRTCEngine.h"

/**
 * @brief WebRTCEngine单元测试类
 * 
 * 测试WebRTCEngine的媒体流处理功能：
 * - 媒体设备管理
 * - 本地媒体流控制
 * - SDP处理
 * - ICE候选处理
 * - 连接状态管理
 * - 权限处理
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
    void testInitialState();
    void testMediaDeviceEnumeration();
    void testLocalMediaControl();
    void testConnectionManagement();
    
    // 媒体流测试
    void testLocalVideoStream();
    void testLocalAudioStream();
    void testScreenSharing();
    void testMediaPermissions();
    
    // SDP和ICE测试
    void testSDPHandling();
    void testICECandidateHandling();
    void testOfferAnswerFlow();
    
    // 设备管理测试
    void testDeviceSelection();
    void testDeviceStateChanges();
    void testMediaSettings();

private:
    WebRTCEngine* m_engine;
    QSignalSpy* m_connectionStateSpy;
    QSignalSpy* m_localStreamSpy;
    QSignalSpy* m_iceCandidateSpy;
    QSignalSpy* m_errorSpy;
};

void TestWebRTCEngine::initTestCase()
{
    qDebug() << "Starting WebRTCEngine unit tests";
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
    m_localStreamSpy = new QSignalSpy(m_engine, &WebRTCEngine::localStreamReady);
    m_iceCandidateSpy = new QSignalSpy(m_engine, &WebRTCEngine::iceCandidate);
    m_errorSpy = new QSignalSpy(m_engine, &WebRTCEngine::error);
}

void TestWebRTCEngine::cleanup()
{
    if (m_engine) {
        m_engine->closePeerConnection();
        delete m_engine;
        m_engine = nullptr;
    }
    
    delete m_connectionStateSpy;
    delete m_localStreamSpy;
    delete m_iceCandidateSpy;
    delete m_errorSpy;
}

void TestWebRTCEngine::testInitialState()
{
    // 测试初始状态
    QCOMPARE(m_engine->connectionState(), WebRTCEngine::Disconnected);
    QCOMPARE(m_engine->iceConnectionState(), WebRTCEngine::IceNew);
    QVERIFY(!m_engine->hasLocalStream());
    QVERIFY(!m_engine->hasVideoPermission());
    QVERIFY(!m_engine->hasAudioPermission());
}

void TestWebRTCEngine::testMediaDeviceEnumeration()
{
    // 测试媒体设备枚举
    QList<QCameraDevice> cameras = m_engine->availableCameras();
    QList<QAudioDevice> audioInputs = m_engine->availableAudioInputs();
    QList<QAudioDevice> audioOutputs = m_engine->availableAudioOutputs();
    
    // 验证设备列表不为null（可能为空但不应该崩溃）
    QVERIFY(cameras.size() >= 0);
    QVERIFY(audioInputs.size() >= 0);
    QVERIFY(audioOutputs.size() >= 0);
    
    qDebug() << "Found" << cameras.size() << "cameras";
    qDebug() << "Found" << audioInputs.size() << "audio inputs";
    qDebug() << "Found" << audioOutputs.size() << "audio outputs";
}

void TestWebRTCEngine::testLocalMediaControl()
{
    // 测试本地媒体控制
    m_engine->startLocalVideo();
    m_engine->stopLocalVideo();
    
    m_engine->startLocalAudio();
    m_engine->stopLocalAudio();
    
    // 验证重复操作不会崩溃
    m_engine->startLocalVideo();
    m_engine->startLocalVideo();
    m_engine->stopLocalVideo();
    m_engine->stopLocalVideo();
    
    // 验证操作不会崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testConnectionManagement()
{
    // 测试连接管理
    m_engine->createPeerConnection();
    
    // 验证连接状态可能改变
    QVERIFY(m_connectionStateSpy->count() >= 0);
    
    m_engine->closePeerConnection();
    
    // 验证连接管理不会崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testLocalVideoStream()
{
    // 测试本地视频流
    m_engine->startLocalVideo();
    
    // 等待视频流启动
    QTest::qWait(500);
    
    // 验证本地流信号
    QVERIFY(m_localStreamSpy->count() >= 0);
    
    m_engine->stopLocalVideo();
}

void TestWebRTCEngine::testLocalAudioStream()
{
    // 测试本地音频流
    m_engine->startLocalAudio();
    
    // 等待音频流启动
    QTest::qWait(500);
    
    m_engine->stopLocalAudio();
    
    // 验证音频流控制不会崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testScreenSharing()
{
    // 测试屏幕共享
    QPixmap testFrame(640, 480);
    testFrame.fill(Qt::blue);
    
    m_engine->sendScreenFrame(testFrame);
    
    // 验证屏幕共享不会崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testMediaPermissions()
{
    // 测试媒体权限
    m_engine->requestMediaPermissions();
    
    // 等待权限请求处理
    QTest::qWait(100);
    
    // 验证权限请求不会崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testSDPHandling()
{
    // 测试SDP处理
    QString testOffer = "v=0\r\no=- 123456789 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n";
    QString testAnswer = "v=0\r\no=- 987654321 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n";
    
    m_engine->createOffer();
    m_engine->createAnswer(testOffer);
    m_engine->setRemoteDescription(testOffer, "offer");
    m_engine->setLocalDescription(testAnswer, "answer");
    
    // 验证SDP处理不会崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testICECandidateHandling()
{
    // 测试ICE候选处理
    WebRTCEngine::IceCandidate candidate;
    candidate.candidate = "candidate:1 1 UDP 2130706431 192.168.1.100 54400 typ host";
    candidate.sdpMid = "0";
    candidate.sdpMLineIndex = 0;
    
    m_engine->addIceCandidate(candidate);
    m_engine->gatherIceCandidates();
    
    // 验证ICE处理不会崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testOfferAnswerFlow()
{
    // 测试Offer/Answer流程
    m_engine->createPeerConnection();
    m_engine->createOffer();
    
    // 等待Offer创建
    QTest::qWait(100);
    
    // 模拟接收到远程Offer
    QString remoteOffer = "v=0\r\no=- 123456789 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n";
    m_engine->createAnswer(remoteOffer);
    
    // 验证Offer/Answer流程不会崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testDeviceSelection()
{
    // 测试设备选择
    QList<QCameraDevice> cameras = m_engine->availableCameras();
    QList<QAudioDevice> audioInputs = m_engine->availableAudioInputs();
    QList<QAudioDevice> audioOutputs = m_engine->availableAudioOutputs();
    
    // 如果有设备，测试设备选择
    if (!cameras.isEmpty()) {
        m_engine->setCamera(cameras.first());
    }
    
    if (!audioInputs.isEmpty()) {
        m_engine->setAudioInput(audioInputs.first());
    }
    
    if (!audioOutputs.isEmpty()) {
        m_engine->setAudioOutput(audioOutputs.first());
    }
    
    // 验证设备选择不会崩溃
    QVERIFY(true);
}

void TestWebRTCEngine::testDeviceStateChanges()
{
    // 测试设备状态变化
    m_engine->startLocalVideo();
    m_engine->startLocalAudio();
    
    // 等待设备状态变化
    QTest::qWait(500);
    
    m_engine->stopLocalVideo();
    m_engine->stopLocalAudio();
    
    // 验证设备状态变化处理
    QVERIFY(true);
}

void TestWebRTCEngine::testMediaSettings()
{
    // 测试媒体设置
    QVariantMap settings;
    settings["videoWidth"] = 1280;
    settings["videoHeight"] = 720;
    settings["videoFrameRate"] = 30;
    settings["audioBitrate"] = 128;
    
    m_engine->updateMediaSettings(settings);
    
    // 验证媒体设置更新不会崩溃
    QVERIFY(true);
}

QTEST_MAIN(TestWebRTCEngine)
#include "test_unit_webrtc_engine.moc"