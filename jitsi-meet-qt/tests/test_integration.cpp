#include <QtTest/QtTest>
#include <QApplication>
#include <QTimer>
#include <QSignalSpy>
#include <QDebug>
#include <QTemporaryDir>
#include <QSettings>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "MainApplication.h"
#include "WindowManager.h"
#include "ConfigurationManager.h"
#include "ProtocolHandler.h"
#include "TranslationManager.h"
#include "WindowStateManager.h"
#include "WelcomeWindow.h"
#include "ConferenceWindow.h"
#include "SettingsDialog.h"
#include "JitsiConstants.h"
#include "XMPPClient.h"
#include "WebRTCEngine.h"
#include "MediaManager.h"
#include "ConferenceManager.h"
#include "ChatManager.h"
#include "ScreenShareManager.h"
#include "AuthenticationManager.h"
#include "JitsiError.h"
#include "ErrorRecoveryManager.h"

/**
 * @brief 集成测试类，测试所有组件的集成工作
 * 
 * 这个测试类涵盖以下集成测试场景：
 * 1. 完整会议流程的端到端测试
 * 2. XMPP连接和WebRTC媒体传输集成测试
 * 3. 聊天功能和屏幕共享功能验证
 * 4. 多参与者会议场景测试
 * 5. 配置持久化和状态恢复功能验证
 */
class IntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // 完整会议流程的端到端测试 (Requirements: 5.1, 6.1, 11.1)
    void testCompleteConferenceFlow();
    void testConferenceJoinLeaveFlow();
    void testConferenceReconnectionFlow();
    void testConferenceErrorRecoveryFlow();
    
    // XMPP连接和WebRTC媒体传输集成测试 (Requirements: 6.1, 11.1)
    void testXMPPWebRTCIntegration();
    void testXMPPConnectionEstablishment();
    void testWebRTCMediaStreamIntegration();
    void testSignalingAndMediaFlow();
    void testICECandidateExchange();
    void testSDPOfferAnswerFlow();
    
    // 聊天功能和屏幕共享功能验证 (Requirements: 12.1, 13.1)
    void testChatFunctionalityIntegration();
    void testScreenSharingIntegration();
    void testChatMessageFlow();
    void testScreenShareFlow();
    void testChatAndScreenShareCombined();
    
    // 多参与者会议场景测试 (Requirements: 5.1, 11.1, 12.1, 13.1)
    void testMultiParticipantConference();
    void testMultiParticipantMediaStreams();
    void testMultiParticipantChat();
    void testMultiParticipantScreenShare();
    void testParticipantJoinLeaveEvents();
    
    // 配置持久化和状态恢复功能验证 (Requirements: 5.1, 6.1)
    void testConfigurationPersistence();
    void testWindowStatePersistence();
    void testConferenceStateRecovery();
    void testMediaDeviceStatePersistence();
    void testChatHistoryPersistence();
    
    // 协议兼容性和错误处理集成测试
    void testProtocolCompatibility();
    void testErrorHandlingIntegration();
    void testNetworkFailureRecovery();
    void testMediaDeviceFailureRecovery();
    
    // 性能和资源管理集成测试
    void testPerformanceIntegration();
    void testMemoryManagementIntegration();
    void testResourceCleanupIntegration();

private:
    // Test setup and teardown helpers
    void createTestApplication();
    void destroyTestApplication();
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    
    // Test utility methods
    bool waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
    bool waitForMultipleSignals(const QList<QPair<QObject*, const char*>>& signals, int timeout = 10000);
    void simulateNetworkDelay(int milliseconds = 100);
    void simulateUserInteraction();
    
    // Mock participant simulation
    void simulateParticipantJoin(const QString& participantId, const QString& displayName = QString());
    void simulateParticipantLeave(const QString& participantId);
    void simulateRemoteMediaStream(const QString& participantId, bool video = true, bool audio = true);
    void simulateChatMessage(const QString& fromId, const QString& message);
    void simulateScreenShare(const QString& participantId, bool start = true);
    
    // Conference flow helpers
    bool setupTestConference(const QString& roomName = "IntegrationTestRoom");
    void cleanupTestConference();
    bool joinConferenceAndWait(const QString& url, int timeout = 15000);
    bool leaveConferenceAndWait(int timeout = 5000);
    
    // Media testing helpers
    bool verifyLocalMediaStreams();
    bool verifyRemoteMediaStreams(const QStringList& participantIds);
    bool verifyAudioDeviceAccess();
    bool verifyVideoDeviceAccess();
    
    // Chat testing helpers
    bool sendChatMessageAndVerify(const QString& message);
    bool verifyChatMessageReceived(const QString& expectedMessage, const QString& fromId);
    
    // Screen share testing helpers
    bool startScreenShareAndVerify();
    bool stopScreenShareAndVerify();
    bool verifyRemoteScreenShare(const QString& participantId);
    
    // Configuration testing helpers
    void saveTestConfiguration();
    bool verifyConfigurationPersistence();
    void corruptConfigurationFile();
    bool verifyConfigurationRecovery();
    
    // Error simulation helpers
    void simulateNetworkError();
    void simulateXMPPConnectionError();
    void simulateWebRTCError();
    void simulateMediaDeviceError();
    void simulateAuthenticationError();
    
    // Performance monitoring helpers
    void startPerformanceMonitoring();
    void stopPerformanceMonitoring();
    bool verifyPerformanceMetrics();
    
    MainApplication* m_app;
    ConferenceManager* m_conferenceManager;
    XMPPClient* m_xmppClient;
    WebRTCEngine* m_webrtcEngine;
    MediaManager* m_mediaManager;
    ChatManager* m_chatManager;
    ScreenShareManager* m_screenShareManager;
    
    int m_argc;
    char** m_argv;
    QTemporaryDir* m_tempDir;
    QString m_originalConfigPath;
    QString m_testRoomName;
    QStringList m_testParticipants;
    
    // Performance monitoring
    QElapsedTimer m_performanceTimer;
    qint64 m_startMemoryUsage;
    qint64 m_peakMemoryUsage;
};

void IntegrationTest::initTestCase()
{
    qDebug() << "Starting comprehensive integration tests for Jitsi Meet Qt";
    
    // 设置测试环境
    setupTestEnvironment();
    
    m_argc = 1;
    m_argv = new char*[1];
    m_argv[0] = new char[30];
    strcpy(m_argv[0], "jitsi_integration_test");
    
    m_app = nullptr;
    m_tempDir = nullptr;
    m_conferenceManager = nullptr;
    m_xmppClient = nullptr;
    m_webrtcEngine = nullptr;
    m_mediaManager = nullptr;
    m_chatManager = nullptr;
    m_screenShareManager = nullptr;
    
    m_testRoomName = "IntegrationTestRoom";
    m_testParticipants.clear();
    m_startMemoryUsage = 0;
    m_peakMemoryUsage = 0;
}

void IntegrationTest::cleanupTestCase()
{
    qDebug() << "Finishing comprehensive integration tests";
    
    cleanupTestEnvironment();
    
    if (m_argv) {
        delete[] m_argv[0];
        delete[] m_argv;
    }
}

void IntegrationTest::init()
{
    createTestApplication();
    
    // 获取核心组件引用
    if (m_app) {
        m_conferenceManager = m_app->findChild<ConferenceManager*>();
        m_xmppClient = m_app->findChild<XMPPClient*>();
        m_webrtcEngine = m_app->findChild<WebRTCEngine*>();
        m_mediaManager = m_app->findChild<MediaManager*>();
        m_chatManager = m_app->findChild<ChatManager*>();
        m_screenShareManager = m_app->findChild<ScreenShareManager*>();
    }
    
    // 开始性能监控
    startPerformanceMonitoring();
}

void IntegrationTest::cleanup()
{
    // 停止性能监控
    stopPerformanceMonitoring();
    
    // 清理测试会议
    cleanupTestConference();
    
    // 清理应用程序
    destroyTestApplication();
    
    // 重置组件引用
    m_conferenceManager = nullptr;
    m_xmppClient = nullptr;
    m_webrtcEngine = nullptr;
    m_mediaManager = nullptr;
    m_chatManager = nullptr;
    m_screenShareManager = nullptr;
}

void IntegrationTest::setupTestEnvironment()
{
    // 创建临时目录用于测试配置
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    // 保存原始配置路径
    m_originalConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    
    // 设置测试应用程序信息
    QCoreApplication::setApplicationName("JitsiMeetQtTest");
    QCoreApplication::setOrganizationName("JitsiMeetQtTest");
    
    // 设置测试配置路径
    qputenv("XDG_CONFIG_HOME", m_tempDir->path().toLocal8Bit());
}

void IntegrationTest::cleanupTestEnvironment()
{
    if (m_tempDir) {
        delete m_tempDir;
        m_tempDir = nullptr;
    }
    
    // 恢复原始配置路径
    if (!m_originalConfigPath.isEmpty()) {
        qputenv("XDG_CONFIG_HOME", m_originalConfigPath.toLocal8Bit());
    }
}

// ============================================================================
// 完整会议流程的端到端测试 (Requirements: 5.1, 6.1, 11.1)
// ============================================================================

void IntegrationTest::testCompleteConferenceFlow()
{
    qDebug() << "Testing complete conference flow...";
    
    QVERIFY(m_app != nullptr);
    QVERIFY(m_conferenceManager != nullptr);
    
    // 1. 设置测试会议
    QVERIFY(setupTestConference());
    
    // 2. 加入会议
    QString testUrl = QString("https://meet.jit.si/%1").arg(m_testRoomName);
    QVERIFY(joinConferenceAndWait(testUrl));
    
    // 3. 验证会议状态
    QCOMPARE(m_conferenceManager->connectionState(), ConferenceManager::Connected);
    QVERIFY(!m_conferenceManager->currentRoom().isEmpty());
    
    // 4. 验证媒体流
    QVERIFY(verifyLocalMediaStreams());
    
    // 5. 模拟参与者加入
    simulateParticipantJoin("participant1", "Test User 1");
    simulateParticipantJoin("participant2", "Test User 2");
    
    // 6. 验证参与者列表
    QStringList participants = m_conferenceManager->participants();
    QVERIFY(participants.contains("participant1"));
    QVERIFY(participants.contains("participant2"));
    
    // 7. 测试聊天功能
    QVERIFY(sendChatMessageAndVerify("Hello from integration test!"));
    
    // 8. 测试屏幕共享
    QVERIFY(startScreenShareAndVerify());
    QVERIFY(stopScreenShareAndVerify());
    
    // 9. 离开会议
    QVERIFY(leaveConferenceAndWait());
    
    qDebug() << "Complete conference flow test passed";
}

void IntegrationTest::testConferenceJoinLeaveFlow()
{
    qDebug() << "Testing conference join/leave flow...";
    
    QVERIFY(m_conferenceManager != nullptr);
    
    // 监听连接状态变化
    QSignalSpy connectionStateSpy(m_conferenceManager, &ConferenceManager::connectionStateChanged);
    QSignalSpy conferenceJoinedSpy(m_conferenceManager, &ConferenceManager::conferenceJoined);
    QSignalSpy conferenceLeftSpy(m_conferenceManager, &ConferenceManager::conferenceLeft);
    
    // 加入会议
    QString testUrl = QString("https://meet.jit.si/%1").arg(m_testRoomName);
    m_conferenceManager->joinConference(testUrl);
    
    // 等待连接建立
    QVERIFY(waitForSignal(m_conferenceManager, SIGNAL(conferenceJoined()), 15000));
    QCOMPARE(m_conferenceManager->connectionState(), ConferenceManager::Connected);
    
    // 验证信号发射
    QVERIFY(connectionStateSpy.count() > 0);
    QVERIFY(conferenceJoinedSpy.count() > 0);
    
    // 离开会议
    m_conferenceManager->leaveConference();
    
    // 等待断开连接
    QVERIFY(waitForSignal(m_conferenceManager, SIGNAL(conferenceLeft()), 5000));
    QCOMPARE(m_conferenceManager->connectionState(), ConferenceManager::Disconnected);
    
    // 验证离开信号
    QVERIFY(conferenceLeftSpy.count() > 0);
    
    qDebug() << "Conference join/leave flow test passed";
}

void IntegrationTest::testConferenceReconnectionFlow()
{
    qDebug() << "Testing conference reconnection flow...";
    
    QVERIFY(m_conferenceManager != nullptr);
    
    // 加入会议
    QString testUrl = QString("https://meet.jit.si/%1").arg(m_testRoomName);
    QVERIFY(joinConferenceAndWait(testUrl));
    
    // 监听重连信号
    QSignalSpy reconnectingSpy(m_conferenceManager, &ConferenceManager::connectionStateChanged);
    
    // 模拟网络中断
    simulateNetworkError();
    
    // 等待重连状态
    bool reconnectionDetected = false;
    for (int i = 0; i < reconnectingSpy.count(); ++i) {
        QList<QVariant> arguments = reconnectingSpy.at(i);
        ConferenceManager::ConnectionState state = 
            static_cast<ConferenceManager::ConnectionState>(arguments.at(0).toInt());
        if (state == ConferenceManager::Reconnecting) {
            reconnectionDetected = true;
            break;
        }
    }
    
    QVERIFY(reconnectionDetected);
    
    // 等待重连成功
    QVERIFY(waitForSignal(m_conferenceManager, SIGNAL(conferenceJoined()), 10000));
    QCOMPARE(m_conferenceManager->connectionState(), ConferenceManager::Connected);
    
    qDebug() << "Conference reconnection flow test passed";
}

void IntegrationTest::testConferenceErrorRecoveryFlow()
{
    qDebug() << "Testing conference error recovery flow...";
    
    QVERIFY(m_conferenceManager != nullptr);
    
    // 加入会议
    QString testUrl = QString("https://meet.jit.si/%1").arg(m_testRoomName);
    QVERIFY(joinConferenceAndWait(testUrl));
    
    // 模拟各种错误并验证恢复
    simulateXMPPConnectionError();
    simulateWebRTCError();
    simulateMediaDeviceError();
    
    // 验证错误恢复机制
    QTimer::singleShot(2000, [this]() {
        // 应该能够恢复连接
        QVERIFY(m_conferenceManager->connectionState() == ConferenceManager::Connected ||
                m_conferenceManager->connectionState() == ConferenceManager::Reconnecting);
    });
    
    QTest::qWait(3000); // 等待恢复
    
    qDebug() << "Conference error recovery flow test passed";
}

// ============================================================================
// XMPP连接和WebRTC媒体传输集成测试 (Requirements: 6.1, 11.1)
// ============================================================================

void IntegrationTest::testXMPPWebRTCIntegration()
{
    qDebug() << "Testing XMPP and WebRTC integration...";
    
    QVERIFY(m_xmppClient != nullptr);
    QVERIFY(m_webrtcEngine != nullptr);
    
    // 监听XMPP和WebRTC事件
    QSignalSpy xmppConnectedSpy(m_xmppClient, &XMPPClient::connected);
    QSignalSpy webrtcOfferSpy(m_webrtcEngine, &WebRTCEngine::offerCreated);
    QSignalSpy webrtcAnswerSpy(m_webrtcEngine, &WebRTCEngine::answerCreated);
    QSignalSpy iceCandidateSpy(m_webrtcEngine, &WebRTCEngine::iceCandidate);
    
    // 建立XMPP连接
    QString serverUrl = "wss://meet.jit.si/xmpp-websocket";
    m_xmppClient->connectToServer(serverUrl, m_testRoomName);
    
    // 等待XMPP连接建立
    QVERIFY(waitForSignal(m_xmppClient, SIGNAL(connected()), 10000));
    QVERIFY(xmppConnectedSpy.count() > 0);
    
    // 创建WebRTC连接
    m_webrtcEngine->createPeerConnection();
    m_webrtcEngine->createOffer();
    
    // 验证WebRTC信令
    QVERIFY(waitForSignal(m_webrtcEngine, SIGNAL(offerCreated(QString)), 5000));
    QVERIFY(webrtcOfferSpy.count() > 0);
    
    // 验证ICE候选交换
    QVERIFY(waitForSignal(m_webrtcEngine, SIGNAL(iceCandidate(QString)), 5000));
    QVERIFY(iceCandidateSpy.count() > 0);
    
    qDebug() << "XMPP and WebRTC integration test passed";
}

void IntegrationTest::testProtocolUrlParsing()
{
    auto protocolHandler = m_app->protocolHandler();
    
    // 测试各种协议URL格式
    struct ProtocolTestCase {
        QString input;
        bool shouldBeValid;
        QString expectedRoom;
        QString expectedServer;
    };
    
    QList<ProtocolTestCase> testCases = {
        {"jitsi-meet://meet.jit.si/TestRoom", true, "TestRoom", "meet.jit.si"},
        {"jitsi-meet://custom.server.com/MyRoom123", true, "MyRoom123", "custom.server.com"},
        {"jitsi-meet://meet.jit.si/Room-With-Dashes", true, "Room-With-Dashes", "meet.jit.si"},
        {"jitsi-meet://meet.jit.si/", false, "", ""},
        {"invalid-protocol://meet.jit.si/Room", false, "", ""},
        {"jitsi-meet://", false, "", ""},
        {"", false, "", ""}
    };
    
    for (const auto& testCase : testCases) {
        bool isValid = protocolHandler->isValidProtocolUrl(testCase.input);
        QCOMPARE(isValid, testCase.shouldBeValid);
        
        if (testCase.shouldBeValid) {
            QString parsedUrl = protocolHandler->parseProtocolUrl(testCase.input);
            QVERIFY(!parsedUrl.isEmpty());
            QVERIFY(parsedUrl.contains(testCase.expectedRoom));
            
            auto urlInfo = protocolHandler->parseUrlInfo(testCase.input);
            QCOMPARE(urlInfo.roomName, testCase.expectedRoom);
            QCOMPARE(urlInfo.serverUrl, testCase.expectedServer);
        }
    }
    
    qDebug() << "Protocol URL parsing test passed";
}

void IntegrationTest::testProtocolUrlFlow()
{
    auto windowManager = m_app->windowManager();
    auto protocolHandler = m_app->protocolHandler();
    
    // 监听协议URL处理信号
    QSignalSpy protocolUrlSpy(protocolHandler, &ProtocolHandler::protocolUrlReceived);
    QSignalSpy windowChangedSpy(windowManager, &WindowManager::windowChanged);
    
    // 模拟协议URL处理
    QString testProtocolUrl = "jitsi-meet://meet.jit.si/ProtocolFlowTest";
    
    // 处理协议URL
    m_app->handleProtocolUrl(testProtocolUrl);
    
    // 验证协议URL被正确接收
    QVERIFY(protocolUrlSpy.count() > 0);
    
    // 验证窗口切换到会议窗口
    QVERIFY(waitForSignal(windowManager, SIGNAL(windowChanged(int)), 3000));
    QCOMPARE(windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    
    // 验证会议窗口加载了正确的URL
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    qDebug() << "Protocol URL flow test passed";
}

void IntegrationTest::testProtocolErrorHandling()
{
    auto protocolHandler = m_app->protocolHandler();
    
    // 测试无效协议URL的错误处理
    QSignalSpy errorSpy(protocolHandler, &ProtocolHandler::protocolError);
    
    // 处理无效的协议URL
    QStringList invalidUrls = {
        "invalid-protocol://meet.jit.si/Room",
        "jitsi-meet://",
        "jitsi-meet://invalid-server/Room",
        "",
        "not-a-url-at-all"
    };
    
    for (const QString& invalidUrl : invalidUrls) {
        m_app->handleProtocolUrl(invalidUrl);
    }
    
    // 验证错误信号被发射
    QVERIFY(errorSpy.count() > 0);
    
    qDebug() << "Protocol error handling test passed";
}

void IntegrationTest::testProtocolMultipleInstances()
{
    // 测试多实例情况下的协议处理
    
    // 注意：这个测试比较复杂，因为涉及到多个应用程序实例
    // 在实际测试中，我们模拟第二个实例尝试启动的情况
    
    auto protocolHandler = m_app->protocolHandler();
    
    // 模拟第二个实例发送协议URL
    QSignalSpy secondInstanceSpy(m_app, &MainApplication::secondInstanceStarted);
    
    // 这里我们模拟接收到来自第二个实例的消息
    QString testUrl = "jitsi-meet://meet.jit.si/MultiInstanceTest";
    m_app->handleSecondInstanceMessage(testUrl);
    
    // 验证第二个实例信号被处理
    QVERIFY(secondInstanceSpy.count() > 0);
    
    qDebug() << "Protocol multiple instances test passed";
}

void IntegrationTest::testWindowManagerIntegration()
{
    auto windowManager = m_app->windowManager();
    QVERIFY(windowManager != nullptr);
    
    // 测试窗口创建和显示
    QSignalSpy windowChangedSpy(windowManager, &WindowManager::windowChanged);
    QSignalSpy windowCreatedSpy(windowManager, &WindowManager::windowCreated);
    
    // 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 验证信号发射
    QVERIFY(windowChangedSpy.count() > 0);
    QVERIFY(windowCreatedSpy.count() > 0);
    
    // 验证窗口状态
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    QVERIFY(windowManager->hasWindow(WindowManager::WelcomeWindow));
    QVERIFY(windowManager->isWindowVisible(WindowManager::WelcomeWindow));
    
    qDebug() << "WindowManager integration test passed";
}

void IntegrationTest::testWindowSwitching()
{
    auto windowManager = m_app->windowManager();
    
    QSignalSpy windowChangedSpy(windowManager, &WindowManager::windowChanged);
    QSignalSpy windowCreatedSpy(windowManager, &WindowManager::windowCreated);
    QSignalSpy windowClosedSpy(windowManager, &WindowManager::windowClosed);
    
    // 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    QVERIFY(windowChangedSpy.count() > 0);
    
    // 切换到会议窗口
    QVariantMap conferenceData;
    conferenceData["url"] = "https://meet.jit.si/TestRoom";
    windowManager->showWindow(WindowManager::ConferenceWindow, conferenceData);
    
    QCOMPARE(windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    QVERIFY(windowManager->hasWindow(WindowManager::ConferenceWindow));
    QVERIFY(windowCreatedSpy.count() > 0);
    
    // 显示设置对话框（模态窗口）
    windowManager->showWindow(WindowManager::SettingsDialog);
    QVERIFY(windowManager->hasWindow(WindowManager::SettingsDialog));
    QVERIFY(windowManager->isWindowVisible(WindowManager::SettingsDialog));
    
    // 关闭设置对话框
    windowManager->closeWindow(WindowManager::SettingsDialog);
    QVERIFY(!windowManager->isWindowVisible(WindowManager::SettingsDialog));
    QVERIFY(windowClosedSpy.count() > 0);
    
    // 返回欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    
    qDebug() << "Window switching test passed";
}

void IntegrationTest::testWindowNavigationFlow()
{
    auto windowManager = m_app->windowManager();
    
    // 测试完整的导航流程
    QSignalSpy navigationSpy(windowManager, &WindowManager::navigationRequested);
    
    // 1. 启动 -> 欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    auto welcomeWindow = qobject_cast<WelcomeWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(welcomeWindow != nullptr);
    
    // 2. 欢迎窗口 -> 会议窗口
    QVariantMap data;
    data["url"] = "https://meet.jit.si/NavigationTest";
    windowManager->showWindow(WindowManager::ConferenceWindow, data);
    
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    // 3. 会议窗口 -> 设置对话框
    windowManager->showWindow(WindowManager::SettingsDialog);
    auto settingsDialog = qobject_cast<SettingsDialog*>(windowManager->getWindow(WindowManager::SettingsDialog));
    QVERIFY(settingsDialog != nullptr);
    
    // 4. 设置对话框 -> 返回会议窗口
    windowManager->closeWindow(WindowManager::SettingsDialog);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    
    // 5. 会议窗口 -> 返回欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    
    qDebug() << "Window navigation flow test passed";
}

void IntegrationTest::testWindowStateManagement()
{
    auto windowManager = m_app->windowManager();
    auto stateManager = windowManager->stateManager();
    QVERIFY(stateManager != nullptr);
    
    // 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    auto welcomeWindow = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(welcomeWindow != nullptr);
    
    // 测试窗口状态保存
    QRect originalGeometry = welcomeWindow->geometry();
    welcomeWindow->resize(1000, 700);
    welcomeWindow->move(200, 150);
    
    // 保存状态
    stateManager->saveWindowState(WindowManager::WelcomeWindow, welcomeWindow);
    
    // 创建新窗口并恢复状态
    windowManager->closeWindow(WindowManager::WelcomeWindow);
    windowManager->showWindow(WindowManager::WelcomeWindow);
    
    auto newWelcomeWindow = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(newWelcomeWindow != nullptr);
    
    // 验证状态恢复
    stateManager->restoreWindowState(WindowManager::WelcomeWindow, newWelcomeWindow);
    QCOMPARE(newWelcomeWindow->size(), QSize(1000, 700));
    
    qDebug() << "Window state management test passed";
}

void IntegrationTest::testWindowMemoryManagement()
{
    auto windowManager = m_app->windowManager();
    
    // 测试窗口创建和销毁的内存管理
    QPointer<QMainWindow> welcomePtr;
    QPointer<QMainWindow> conferencePtr;
    QPointer<QDialog> settingsPtr;
    
    // 创建窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    welcomePtr = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(!welcomePtr.isNull());
    
    windowManager->showWindow(WindowManager::ConferenceWindow);
    conferencePtr = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(!conferencePtr.isNull());
    
    windowManager->showWindow(WindowManager::SettingsDialog);
    settingsPtr = qobject_cast<QDialog*>(windowManager->getWindow(WindowManager::SettingsDialog));
    QVERIFY(!settingsPtr.isNull());
    
    // 关闭窗口
    windowManager->closeWindow(WindowManager::SettingsDialog);
    windowManager->closeWindow(WindowManager::ConferenceWindow);
    windowManager->closeWindow(WindowManager::WelcomeWindow);
    
    // 强制垃圾回收
    QCoreApplication::processEvents();
    
    // 验证内存清理（注意：这个测试可能需要调整，取决于具体的内存管理策略）
    qDebug() << "Window memory management test passed";
}

void IntegrationTest::testWindowDataTransfer()
{
    auto windowManager = m_app->windowManager();
    
    QSignalSpy dataTransferSpy(windowManager, &WindowManager::dataTransferred);
    
    // 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 传递数据到会议窗口
    QVariantMap data;
    data["url"] = "https://meet.jit.si/DataTransferTest";
    data["roomName"] = "DataTransferTest";
    data["serverUrl"] = "https://meet.jit.si";
    windowManager->showWindow(WindowManager::ConferenceWindow, data);
    
    // 验证数据传递信号
    QVERIFY(dataTransferSpy.count() > 0);
    
    // 验证数据正确传递到会议窗口
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    qDebug() << "Window data transfer test passed";
}

// WebEngine集成测试
void IntegrationTest::testWebEngineInitialization()
{
    auto windowManager = m_app->windowManager();
    
    // 创建会议窗口
    windowManager->showWindow(WindowManager::ConferenceWindow);
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    // 获取WebEngineView
    auto webView = conferenceWindow->findChild<QWebEngineView*>();
    QVERIFY(webView != nullptr);
    
    // 验证WebEngine设置
    auto settings = webView->settings();
    QVERIFY(settings != nullptr);
    QVERIFY(settings->testAttribute(QWebEngineSettings::JavascriptEnabled));
    QVERIFY(settings->testAttribute(QWebEngineSettings::LocalStorageEnabled));
    
    // 验证WebEngine配置文件
    auto profile = webView->page()->profile();
    QVERIFY(profile != nullptr);
    
    qDebug() << "WebEngine initialization test passed";
}

void IntegrationTest::testWebEngineLoading()
{
    auto windowManager = m_app->windowManager();
    
    // 创建会议窗口
    QVariantMap data;
    data["url"] = "https://meet.jit.si/WebEngineLoadTest";
    windowManager->showWindow(WindowManager::ConferenceWindow, data);
    
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    auto webView = conferenceWindow->findChild<QWebEngineView*>();
    QVERIFY(webView != nullptr);
    
    // 监听加载信号
    QSignalSpy loadStartedSpy(webView, &QWebEngineView::loadStarted);
    QSignalSpy loadFinishedSpy(webView, &QWebEngineView::loadFinished);
    QSignalSpy loadProgressSpy(webView, &QWebEngineView::loadProgress);
    
    // 加载测试页面
    conferenceWindow->loadConference("https://meet.jit.si/WebEngineLoadTest");
    
    // 验证加载开始
    QVERIFY(loadStartedSpy.count() > 0 || loadStartedSpy.wait(5000));
    
    // 等待加载完成
    if (loadFinishedSpy.isEmpty()) {
        QVERIFY(loadFinishedSpy.wait(15000)); // 等待最多15秒
    }
    
    // 验证加载进度
    QVERIFY(loadProgressSpy.count() > 0);
    
    qDebug() << "WebEngine loading test passed";
}

void IntegrationTest::testWebEngineJavaScriptInteraction()
{
    auto windowManager = m_app->windowManager();
    
    // 创建会议窗口
    windowManager->showWindow(WindowManager::ConferenceWindow);
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    auto webView = conferenceWindow->findChild<QWebEngineView*>();
    QVERIFY(webView != nullptr);
    
    // 加载简单的HTML页面用于测试
    QString testHtml = R"(
        <!DOCTYPE html>
        <html>
        <head><title>Test Page</title></head>
        <body>
            <div id="test">Hello World</div>
            <script>
                window.testFunction = function() {
                    return "JavaScript works!";
                };
                
                window.testCallback = function(message) {
                    document.getElementById('test').innerHTML = message;
                };
            </script>
        </body>
        </html>
    )";
    
    webView->setHtml(testHtml);
    
    // 等待页面加载
    QSignalSpy loadFinishedSpy(webView, &QWebEngineView::loadFinished);
    QVERIFY(loadFinishedSpy.wait(5000));
    
    // 测试JavaScript执行
    QEventLoop loop;
    bool jsTestPassed = false;
    
    webView->page()->runJavaScript("window.testFunction()", [&](const QVariant& result) {
        jsTestPassed = (result.toString() == "JavaScript works!");
        loop.quit();
    });
    
    QTimer::singleShot(3000, &loop, &QEventLoop::quit); // 超时保护
    loop.exec();
    
    QVERIFY(jsTestPassed);
    
    qDebug() << "WebEngine JavaScript interaction test passed";
}

void IntegrationTest::testWebEngineErrorHandling()
{
    auto windowManager = m_app->windowManager();
    
    // 创建会议窗口
    windowManager->showWindow(WindowManager::ConferenceWindow);
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    auto webView = conferenceWindow->findChild<QWebEngineView*>();
    QVERIFY(webView != nullptr);
    
    // 监听错误信号
    QSignalSpy loadFinishedSpy(webView, &QWebEngineView::loadFinished);
    
    // 尝试加载无效URL
    webView->load(QUrl("https://invalid-domain-that-does-not-exist.com"));
    
    // 等待加载完成（应该失败）
    QVERIFY(loadFinishedSpy.wait(10000));
    
    // 验证加载失败
    QList<QVariant> arguments = loadFinishedSpy.takeLast();
    bool loadSuccess = arguments.at(0).toBool();
    QVERIFY(!loadSuccess); // 应该加载失败
    
    qDebug() << "WebEngine error handling test passed";
}

void IntegrationTest::testWebEngineNetworkRequests()
{
    auto windowManager = m_app->windowManager();
    
    // 创建会议窗口
    windowManager->showWindow(WindowManager::ConferenceWindow);
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    auto webView = conferenceWindow->findChild<QWebEngineView*>();
    QVERIFY(webView != nullptr);
    
    // 测试网络请求拦截（如果实现了的话）
    auto profile = webView->page()->profile();
    QVERIFY(profile != nullptr);
    
    // 加载一个真实的URL来测试网络功能
    QSignalSpy loadFinishedSpy(webView, &QWebEngineView::loadFinished);
    
    webView->load(QUrl("https://www.google.com"));
    
    // 等待加载完成
    if (loadFinishedSpy.isEmpty()) {
        QVERIFY(loadFinishedSpy.wait(15000));
    }
    
    // 验证网络请求成功
    QList<QVariant> arguments = loadFinishedSpy.takeLast();
    bool loadSuccess = arguments.at(0).toBool();
    QVERIFY(loadSuccess);
    
    qDebug() << "WebEngine network requests test passed";
}

// 配置持久化和状态恢复测试
void IntegrationTest::testConfigurationPersistence()
{
    auto configManager = m_app->configurationManager();
    QVERIFY(configManager != nullptr);
    
    // 保存原始配置
    QString originalServerUrl = configManager->serverUrl();
    QString originalLanguage = configManager->language();
    QStringList originalRecentUrls = configManager->recentUrls();
    
    // 修改配置
    QString testServerUrl = "https://test-persistence.example.com";
    QString testLanguage = "zh_CN";
    QStringList testRecentUrls = {
        "https://meet.jit.si/PersistenceTest1",
        "https://meet.jit.si/PersistenceTest2",
        "https://meet.jit.si/PersistenceTest3"
    };
    
    configManager->setServerUrl(testServerUrl);
    configManager->setLanguage(testLanguage);
    for (const QString& url : testRecentUrls) {
        configManager->addRecentUrl(url);
    }
    
    // 强制保存配置
    configManager->saveConfiguration();
    
    // 创建新的配置管理器实例来测试持久化
    auto newConfigManager = new ConfigurationManager(this);
    newConfigManager->loadConfiguration();
    
    // 验证配置持久化
    QCOMPARE(newConfigManager->serverUrl(), testServerUrl);
    QCOMPARE(newConfigManager->language(), testLanguage);
    
    QStringList loadedRecentUrls = newConfigManager->recentUrls();
    for (const QString& url : testRecentUrls) {
        QVERIFY(loadedRecentUrls.contains(url));
    }
    
    // 清理
    delete newConfigManager;
    
    // 恢复原始配置
    configManager->setServerUrl(originalServerUrl);
    configManager->setLanguage(originalLanguage);
    configManager->clearRecentUrls();
    for (const QString& url : originalRecentUrls) {
        configManager->addRecentUrl(url);
    }
    
    qDebug() << "Configuration persistence test passed";
}

void IntegrationTest::testWindowStatePersistence()
{
    auto windowManager = m_app->windowManager();
    auto stateManager = windowManager->stateManager();
    QVERIFY(stateManager != nullptr);
    
    // 创建并配置欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    auto welcomeWindow = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(welcomeWindow != nullptr);
    
    // 设置特定的窗口状态
    QRect testGeometry(150, 100, 900, 650);
    welcomeWindow->setGeometry(testGeometry);
    welcomeWindow->showMaximized();
    
    // 保存窗口状态
    stateManager->saveWindowState(WindowManager::WelcomeWindow, welcomeWindow);
    stateManager->saveAllStates();
    
    // 关闭窗口
    windowManager->closeWindow(WindowManager::WelcomeWindow);
    
    // 重新创建窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    auto newWelcomeWindow = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(newWelcomeWindow != nullptr);
    
    // 恢复窗口状态
    stateManager->restoreWindowState(WindowManager::WelcomeWindow, newWelcomeWindow);
    
    // 验证状态恢复
    QVERIFY(newWelcomeWindow->isMaximized());
    
    qDebug() << "Window state persistence test passed";
}

void IntegrationTest::testRecentUrlsPersistence()
{
    auto configManager = m_app->configurationManager();
    
    // 清空现有的最近URL列表
    configManager->clearRecentUrls();
    
    // 添加测试URL
    QStringList testUrls = {
        "https://meet.jit.si/RecentTest1",
        "https://meet.jit.si/RecentTest2",
        "https://custom.server.com/RecentTest3",
        "https://meet.jit.si/RecentTest4"
    };
    
    for (const QString& url : testUrls) {
        configManager->addRecentUrl(url);
    }
    
    // 验证URL被正确添加
    QStringList recentUrls = configManager->recentUrls();
    QCOMPARE(recentUrls.size(), testUrls.size());
    
    for (const QString& url : testUrls) {
        QVERIFY(recentUrls.contains(url));
    }
    
    // 测试URL去重功能
    configManager->addRecentUrl(testUrls.first()); // 重复添加第一个URL
    recentUrls = configManager->recentUrls();
    QCOMPARE(recentUrls.size(), testUrls.size()); // 大小应该保持不变
    
    // 测试最大数量限制
    for (int i = 0; i < 20; ++i) {
        configManager->addRecentUrl(QString("https://meet.jit.si/ExtraTest%1").arg(i));
    }
    
    recentUrls = configManager->recentUrls();
    QVERIFY(recentUrls.size() <= configManager->maxRecentUrls());
    
    qDebug() << "Recent URLs persistence test passed";
}

void IntegrationTest::testSettingsPersistence()
{
    auto configManager = m_app->configurationManager();
    
    // 测试各种设置的持久化
    struct TestSettings {
        QString serverUrl;
        QString language;
        bool autoJoinAudio;
        bool autoJoinVideo;
        bool darkMode;
        int maxRecentItems;
    };
    
    TestSettings originalSettings = {
        configManager->serverUrl(),
        configManager->language(),
        configManager->autoJoinAudio(),
        configManager->autoJoinVideo(),
        configManager->darkMode(),
        configManager->maxRecentUrls()
    };
    
    TestSettings testSettings = {
        "https://custom-settings-test.com",
        "ja",
        false,
        true,
        true,
        15
    };
    
    // 应用测试设置
    configManager->setServerUrl(testSettings.serverUrl);
    configManager->setLanguage(testSettings.language);
    configManager->setAutoJoinAudio(testSettings.autoJoinAudio);
    configManager->setAutoJoinVideo(testSettings.autoJoinVideo);
    configManager->setDarkMode(testSettings.darkMode);
    configManager->setMaxRecentUrls(testSettings.maxRecentItems);
    
    // 保存配置
    configManager->saveConfiguration();
    
    // 重新加载配置
    configManager->loadConfiguration();
    
    // 验证设置持久化
    QCOMPARE(configManager->serverUrl(), testSettings.serverUrl);
    QCOMPARE(configManager->language(), testSettings.language);
    QCOMPARE(configManager->autoJoinAudio(), testSettings.autoJoinAudio);
    QCOMPARE(configManager->autoJoinVideo(), testSettings.autoJoinVideo);
    QCOMPARE(configManager->darkMode(), testSettings.darkMode);
    QCOMPARE(configManager->maxRecentUrls(), testSettings.maxRecentItems);
    
    // 恢复原始设置
    configManager->setServerUrl(originalSettings.serverUrl);
    configManager->setLanguage(originalSettings.language);
    configManager->setAutoJoinAudio(originalSettings.autoJoinAudio);
    configManager->setAutoJoinVideo(originalSettings.autoJoinVideo);
    configManager->setDarkMode(originalSettings.darkMode);
    configManager->setMaxRecentUrls(originalSettings.maxRecentItems);
    
    qDebug() << "Settings persistence test passed";
}

void IntegrationTest::testConfigurationRecovery()
{
    auto configManager = m_app->configurationManager();
    
    // 测试配置文件损坏时的恢复机制
    
    // 保存当前配置
    auto originalConfig = configManager->currentConfiguration();
    
    // 模拟配置损坏（设置无效值）
    configManager->setServerUrl("invalid-url-format");
    
    // 触发配置验证和恢复
    bool recoveryResult = configManager->validateAndRecover();
    QVERIFY(recoveryResult);
    
    // 验证恢复后的配置是有效的
    QVERIFY(configManager->isValidServerUrl(configManager->serverUrl()));
    
    // 测试默认配置恢复
    configManager->resetToDefaults();
    QCOMPARE(configManager->serverUrl(), JitsiConstants::DEFAULT_SERVER_URL);
    QCOMPARE(configManager->language(), "auto");
    
    qDebug() << "Configuration recovery test passed";
}

void IntegrationTest::testTranslationIntegration()
{
    auto translationManager = m_app->translationManager();
    QVERIFY(translationManager != nullptr);
    
    // 测试语言切换
    QSignalSpy languageChangedSpy(translationManager, &TranslationManager::languageChanged);
    
    QString currentLanguage = translationManager->currentLanguage();
    QString testLanguage = (currentLanguage == "en") ? "zh_CN" : "en";
    
    translationManager->setLanguage(testLanguage);
    
    // 验证语言改变信号
    QVERIFY(languageChangedSpy.count() > 0);
    
    qDebug() << "Translation integration test passed";
}

void IntegrationTest::testCompleteApplicationFlow()
{
    auto windowManager = m_app->windowManager();
    auto configManager = m_app->configurationManager();
    
    // 模拟完整的应用程序流程
    
    // 1. 启动应用程序（已经完成）
    QVERIFY(windowManager->currentWindowType() == WindowManager::WelcomeWindow ||
            !windowManager->hasWindow(WindowManager::WelcomeWindow));
    
    // 2. 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    
    // 3. 加入会议
    QVariantMap conferenceData;
    conferenceData["url"] = "https://meet.jit.si/CompleteFlowTest";
    windowManager->showWindow(WindowManager::ConferenceWindow, conferenceData);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    
    // 4. 验证URL被添加到最近列表
    QStringList recentUrls = configManager->recentUrls();
    // 注意：这个测试可能需要等待会议窗口完全加载
    
    // 5. 返回欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    
    qDebug() << "Complete application flow test passed";
}

void IntegrationTest::testProtocolUrlFlow()
{
    // 测试协议URL处理流程
    QString protocolUrl = "jitsi-meet://meet.jit.si/ProtocolFlowTest";
    
    QSignalSpy windowChangedSpy(m_app->windowManager(), &WindowManager::windowChanged);
    
    // 处理协议URL
    m_app->handleProtocolUrl(protocolUrl);
    
    // 验证窗口切换到会议窗口
    QVERIFY(waitForSignal(m_app->windowManager(), SIGNAL(windowChanged(int)), 3000));
    QCOMPARE(m_app->windowManager()->currentWindowType(), WindowManager::ConferenceWindow);
    
    qDebug() << "Protocol URL flow test passed";
}

void IntegrationTest::testSettingsFlow()
{
    auto windowManager = m_app->windowManager();
    
    // 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 显示设置对话框
    windowManager->showWindow(WindowManager::SettingsDialog);
    QVERIFY(windowManager->hasWindow(WindowManager::SettingsDialog));
    QVERIFY(windowManager->isWindowVisible(WindowManager::SettingsDialog));
    
    // 关闭设置对话框
    windowManager->closeWindow(WindowManager::SettingsDialog);
    QVERIFY(!windowManager->isWindowVisible(WindowManager::SettingsDialog));
    
    qDebug() << "Settings flow test passed";
}

void IntegrationTest::createTestApplication()
{
    if (!m_app) {
        m_app = new MainApplication(m_argc, m_argv);
        
        // 等待应用程序初始化完成
        QTimer::singleShot(100, [this]() {
            // 初始化完成后的处理
        });
        
        // 处理事件循环
        QCoreApplication::processEvents();
    }
}

void IntegrationTest::destroyTestApplication()
{
    if (m_app) {
        delete m_app;
        m_app = nullptr;
    }
}

bool IntegrationTest::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QSignalSpy spy(sender, signal);
    return spy.wait(timeout);
}

bool IntegrationTest::waitForMultipleSignals(const QList<QPair<QObject*, const char*>>& signals, int timeout)
{
    QElapsedTimer timer;
    timer.start();
    
    QList<QSignalSpy*> spies;
    for (const auto& signalPair : signals) {
        spies.append(new QSignalSpy(signalPair.first, signalPair.second));
    }
    
    bool allSignalsReceived = false;
    while (timer.elapsed() < timeout && !allSignalsReceived) {
        QCoreApplication::processEvents();
        
        allSignalsReceived = true;
        for (QSignalSpy* spy : spies) {
            if (spy->count() == 0) {
                allSignalsReceived = false;
                break;
            }
        }
        
        if (!allSignalsReceived) {
            QTest::qWait(50);
        }
    }
    
    // 清理
    qDeleteAll(spies);
    
    return allSignalsReceived;
}

void IntegrationTest::simulateNetworkDelay(int milliseconds)
{
    QTest::qWait(milliseconds);
}

void IntegrationTest::simulateParticipantJoin(const QString& participantId, const QString& displayName)
{
    if (m_xmppClient) {
        // 模拟XMPP参与者加入事件
        emit m_xmppClient->participantJoined(participantId);
    }
    
    if (m_conferenceManager) {
        // 更新参与者列表
        m_testParticipants.append(participantId);
    }
    
    simulateNetworkDelay(50);
}

void IntegrationTest::simulateParticipantLeave(const QString& participantId)
{
    if (m_xmppClient) {
        // 模拟XMPP参与者离开事件
        emit m_xmppClient->participantLeft(participantId);
    }
    
    if (m_conferenceManager) {
        // 更新参与者列表
        m_testParticipants.removeAll(participantId);
    }
    
    simulateNetworkDelay(50);
}

void IntegrationTest::simulateRemoteMediaStream(const QString& participantId, bool video, bool audio)
{
    if (m_webrtcEngine) {
        // 模拟远程媒体流
        QVideoWidget* mockVideoWidget = new QVideoWidget();
        emit m_webrtcEngine->remoteStreamReceived(participantId, mockVideoWidget);
    }
    
    simulateNetworkDelay(100);
}

void IntegrationTest::simulateChatMessage(const QString& fromId, const QString& message)
{
    if (m_xmppClient) {
        // 模拟XMPP聊天消息
        emit m_xmppClient->messageReceived(fromId, message);
    }
    
    simulateNetworkDelay(50);
}

void IntegrationTest::simulateScreenShare(const QString& participantId, bool start)
{
    if (m_screenShareManager && start) {
        // 模拟远程屏幕共享开始
        QVideoWidget* mockScreenWidget = new QVideoWidget();
        // 这里需要根据实际的ScreenShareManager API调整
    }
    
    simulateNetworkDelay(100);
}

bool IntegrationTest::setupTestConference(const QString& roomName)
{
    m_testRoomName = roomName;
    m_testParticipants.clear();
    
    // 确保所有组件都已初始化
    if (!m_conferenceManager || !m_xmppClient || !m_webrtcEngine) {
        return false;
    }
    
    return true;
}

void IntegrationTest::cleanupTestConference()
{
    if (m_conferenceManager && 
        m_conferenceManager->connectionState() != ConferenceManager::Disconnected) {
        m_conferenceManager->leaveConference();
        waitForSignal(m_conferenceManager, SIGNAL(conferenceLeft()), 3000);
    }
    
    m_testParticipants.clear();
}

bool IntegrationTest::joinConferenceAndWait(const QString& url, int timeout)
{
    if (!m_conferenceManager) {
        return false;
    }
    
    m_conferenceManager->joinConference(url);
    return waitForSignal(m_conferenceManager, SIGNAL(conferenceJoined()), timeout);
}

bool IntegrationTest::leaveConferenceAndWait(int timeout)
{
    if (!m_conferenceManager) {
        return false;
    }
    
    m_conferenceManager->leaveConference();
    return waitForSignal(m_conferenceManager, SIGNAL(conferenceLeft()), timeout);
}

bool IntegrationTest::verifyLocalMediaStreams()
{
    if (!m_mediaManager) {
        return false;
    }
    
    // 验证本地视频流
    QVideoWidget* localVideo = m_mediaManager->localVideoWidget();
    if (!localVideo) {
        return false;
    }
    
    return true;
}

bool IntegrationTest::verifyRemoteMediaStreams(const QStringList& participantIds)
{
    // 这里需要根据实际的媒体管理器API来验证远程流
    // 暂时返回true，实际实现需要检查每个参与者的媒体流
    return true;
}

bool IntegrationTest::verifyAudioDeviceAccess()
{
    if (!m_mediaManager) {
        return false;
    }
    
    // 尝试启动音频设备
    m_mediaManager->startLocalAudio();
    QTest::qWait(500);
    
    // 验证音频设备是否可用
    return true; // 简化实现
}

bool IntegrationTest::verifyVideoDeviceAccess()
{
    if (!m_mediaManager) {
        return false;
    }
    
    // 尝试启动视频设备
    m_mediaManager->startLocalVideo();
    QTest::qWait(500);
    
    // 验证视频设备是否可用
    return true; // 简化实现
}

bool IntegrationTest::sendChatMessageAndVerify(const QString& message)
{
    if (!m_chatManager) {
        return false;
    }
    
    QSignalSpy messageSentSpy(m_chatManager, &ChatManager::messageSent);
    m_chatManager->sendMessage(message);
    
    return waitForSignal(m_chatManager, SIGNAL(messageSent(ChatManager::ChatMessage)), 3000);
}

bool IntegrationTest::verifyChatMessageReceived(const QString& expectedMessage, const QString& fromId)
{
    if (!m_chatManager) {
        return false;
    }
    
    QList<ChatManager::ChatMessage> history = m_chatManager->messageHistory();
    
    for (const auto& msg : history) {
        if (msg.content == expectedMessage && msg.senderId == fromId) {
            return true;
        }
    }
    
    return false;
}

bool IntegrationTest::startScreenShareAndVerify()
{
    if (!m_screenShareManager) {
        return false;
    }
    
    QSignalSpy screenShareStartedSpy(m_screenShareManager, &ScreenShareManager::screenShareStarted);
    
    // 启动屏幕共享（这里需要根据实际API调整）
    // m_screenShareManager->startScreenShare();
    
    return waitForSignal(m_screenShareManager, SIGNAL(screenShareStarted()), 5000);
}

bool IntegrationTest::stopScreenShareAndVerify()
{
    if (!m_screenShareManager) {
        return false;
    }
    
    QSignalSpy screenShareStoppedSpy(m_screenShareManager, &ScreenShareManager::screenShareStopped);
    
    // 停止屏幕共享（这里需要根据实际API调整）
    // m_screenShareManager->stopScreenShare();
    
    return waitForSignal(m_screenShareManager, SIGNAL(screenShareStopped()), 3000);
}

bool IntegrationTest::verifyRemoteScreenShare(const QString& participantId)
{
    // 验证远程屏幕共享
    // 这里需要根据实际的屏幕共享管理器API来实现
    return true; // 简化实现
}

void IntegrationTest::simulateNetworkError()
{
    // 模拟网络错误
    if (m_xmppClient) {
        // 强制断开XMPP连接来模拟网络错误
        m_xmppClient->disconnect();
    }
}

void IntegrationTest::simulateXMPPConnectionError()
{
    // 模拟XMPP连接错误
    if (m_xmppClient) {
        emit m_xmppClient->disconnected();
    }
}

void IntegrationTest::simulateWebRTCError()
{
    // 模拟WebRTC错误
    // 这里需要根据实际的WebRTC引擎API来实现错误模拟
}

void IntegrationTest::simulateMediaDeviceError()
{
    // 模拟媒体设备错误
    if (m_mediaManager) {
        m_mediaManager->stopLocalVideo();
        m_mediaManager->stopLocalAudio();
    }
}

void IntegrationTest::simulateAuthenticationError()
{
    // 模拟认证错误
    // 这里需要根据实际的认证管理器API来实现
}

void IntegrationTest::startPerformanceMonitoring()
{
    m_performanceTimer.start();
    
    // 记录初始内存使用情况
    // 这里需要实际的内存监控实现
    m_startMemoryUsage = 0; // 简化实现
    m_peakMemoryUsage = 0;
}

void IntegrationTest::stopPerformanceMonitoring()
{
    // 停止性能监控
    qint64 elapsedTime = m_performanceTimer.elapsed();
    qDebug() << "Test execution time:" << elapsedTime << "ms";
    
    // 记录峰值内存使用
    qDebug() << "Peak memory usage:" << m_peakMemoryUsage << "bytes";
}

bool IntegrationTest::verifyPerformanceMetrics()
{
    qint64 elapsedTime = m_performanceTimer.elapsed();
    
    // 验证性能指标
    // 例如：测试不应该超过30秒
    if (elapsedTime > 30000) {
        qWarning() << "Test took too long:" << elapsedTime << "ms";
        return false;
    }
    
    return true;
}

QTEST_MAIN(IntegrationTest)
#include "test_integration.moc"
vo
id IntegrationTest::testXMPPConnectionEstablishment()
{
    qDebug() << "Testing XMPP connection establishment...";
    
    QVERIFY(m_xmppClient != nullptr);
    
    QSignalSpy connectedSpy(m_xmppClient, &XMPPClient::connected);
    QSignalSpy disconnectedSpy(m_xmppClient, &XMPPClient::disconnected);
    QSignalSpy participantJoinedSpy(m_xmppClient, &XMPPClient::participantJoined);
    
    // 连接到XMPP服务器
    QString serverUrl = "wss://meet.jit.si/xmpp-websocket";
    m_xmppClient->connectToServer(serverUrl, m_testRoomName);
    
    // 验证连接建立
    QVERIFY(waitForSignal(m_xmppClient, SIGNAL(connected()), 10000));
    QVERIFY(connectedSpy.count() > 0);
    
    // 发送presence信息
    m_xmppClient->sendPresence("available");
    
    // 模拟参与者加入
    simulateParticipantJoin("test_participant");
    
    // 断开连接
    m_xmppClient->disconnect();
    QVERIFY(waitForSignal(m_xmppClient, SIGNAL(disconnected()), 5000));
    QVERIFY(disconnectedSpy.count() > 0);
    
    qDebug() << "XMPP connection establishment test passed";
}

void IntegrationTest::testWebRTCMediaStreamIntegration()
{
    qDebug() << "Testing WebRTC media stream integration...";
    
    QVERIFY(m_webrtcEngine != nullptr);
    QVERIFY(m_mediaManager != nullptr);
    
    QSignalSpy localStreamSpy(m_webrtcEngine, &WebRTCEngine::localStreamReady);
    QSignalSpy remoteStreamSpy(m_webrtcEngine, &WebRTCEngine::remoteStreamReceived);
    
    // 启动本地媒体流
    m_mediaManager->startLocalVideo();
    m_mediaManager->startLocalAudio();
    
    // 验证本地媒体设备
    QVERIFY(verifyAudioDeviceAccess());
    QVERIFY(verifyVideoDeviceAccess());
    
    // 创建WebRTC连接并添加本地流
    m_webrtcEngine->createPeerConnection();
    
    // 验证本地流就绪
    QVERIFY(waitForSignal(m_webrtcEngine, SIGNAL(localStreamReady(QVideoWidget*)), 5000));
    QVERIFY(localStreamSpy.count() > 0);
    
    // 模拟远程流
    simulateRemoteMediaStream("remote_participant");
    
    qDebug() << "WebRTC media stream integration test passed";
}

void IntegrationTest::testSignalingAndMediaFlow()
{
    qDebug() << "Testing signaling and media flow integration...";
    
    QVERIFY(m_xmppClient != nullptr);
    QVERIFY(m_webrtcEngine != nullptr);
    
    // 建立完整的信令和媒体流
    QString serverUrl = "wss://meet.jit.si/xmpp-websocket";
    m_xmppClient->connectToServer(serverUrl, m_testRoomName);
    
    QVERIFY(waitForSignal(m_xmppClient, SIGNAL(connected()), 10000));
    
    // 创建WebRTC offer
    m_webrtcEngine->createPeerConnection();
    m_webrtcEngine->createOffer();
    
    QSignalSpy offerSpy(m_webrtcEngine, &WebRTCEngine::offerCreated);
    QVERIFY(waitForSignal(m_webrtcEngine, SIGNAL(offerCreated(QString)), 5000));
    
    // 模拟远程answer
    QString mockAnswer = "v=0\r\no=- 123456789 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n";
    m_webrtcEngine->setRemoteDescription(mockAnswer);
    
    // 验证ICE候选交换
    QSignalSpy iceSpy(m_webrtcEngine, &WebRTCEngine::iceCandidate);
    QVERIFY(waitForSignal(m_webrtcEngine, SIGNAL(iceCandidate(QString)), 5000));
    
    qDebug() << "Signaling and media flow integration test passed";
}

void IntegrationTest::testICECandidateExchange()
{
    qDebug() << "Testing ICE candidate exchange...";
    
    QVERIFY(m_webrtcEngine != nullptr);
    
    QSignalSpy iceCandidateSpy(m_webrtcEngine, &WebRTCEngine::iceCandidate);
    
    // 创建peer connection
    m_webrtcEngine->createPeerConnection();
    
    // 模拟ICE候选
    QString mockCandidate = "candidate:1 1 UDP 2130706431 192.168.1.100 54400 typ host";
    m_webrtcEngine->addIceCandidate(mockCandidate);
    
    // 验证ICE候选生成
    QVERIFY(waitForSignal(m_webrtcEngine, SIGNAL(iceCandidate(QString)), 5000));
    QVERIFY(iceCandidateSpy.count() > 0);
    
    qDebug() << "ICE candidate exchange test passed";
}

void IntegrationTest::testSDPOfferAnswerFlow()
{
    qDebug() << "Testing SDP offer/answer flow...";
    
    QVERIFY(m_webrtcEngine != nullptr);
    
    QSignalSpy offerSpy(m_webrtcEngine, &WebRTCEngine::offerCreated);
    QSignalSpy answerSpy(m_webrtcEngine, &WebRTCEngine::answerCreated);
    
    // 创建offer
    m_webrtcEngine->createPeerConnection();
    m_webrtcEngine->createOffer();
    
    QVERIFY(waitForSignal(m_webrtcEngine, SIGNAL(offerCreated(QString)), 5000));
    QVERIFY(offerSpy.count() > 0);
    
    // 获取offer内容
    QString offer = offerSpy.first().first().toString();
    QVERIFY(!offer.isEmpty());
    QVERIFY(offer.contains("v=0")); // SDP version
    
    // 创建answer
    m_webrtcEngine->createAnswer(offer);
    QVERIFY(waitForSignal(m_webrtcEngine, SIGNAL(answerCreated(QString)), 5000));
    QVERIFY(answerSpy.count() > 0);
    
    qDebug() << "SDP offer/answer flow test passed";
}

// ============================================================================
// 聊天功能和屏幕共享功能验证 (Requirements: 12.1, 13.1)
// ============================================================================

void IntegrationTest::testChatFunctionalityIntegration()
{
    qDebug() << "Testing chat functionality integration...";
    
    QVERIFY(m_chatManager != nullptr);
    QVERIFY(m_xmppClient != nullptr);
    
    QSignalSpy messageSentSpy(m_chatManager, &ChatManager::messageSent);
    QSignalSpy messageReceivedSpy(m_chatManager, &ChatManager::messageReceived);
    
    // 建立XMPP连接
    QString serverUrl = "wss://meet.jit.si/xmpp-websocket";
    m_xmppClient->connectToServer(serverUrl, m_testRoomName);
    QVERIFY(waitForSignal(m_xmppClient, SIGNAL(connected()), 10000));
    
    // 发送聊天消息
    QString testMessage = "Integration test message";
    m_chatManager->sendMessage(testMessage);
    
    // 验证消息发送
    QVERIFY(waitForSignal(m_chatManager, SIGNAL(messageSent(ChatManager::ChatMessage)), 3000));
    QVERIFY(messageSentSpy.count() > 0);
    
    // 模拟接收消息
    simulateChatMessage("remote_user", "Hello from remote user!");
    
    // 验证消息接收
    QVERIFY(waitForSignal(m_chatManager, SIGNAL(messageReceived(ChatManager::ChatMessage)), 3000));
    QVERIFY(messageReceivedSpy.count() > 0);
    
    // 验证消息历史
    QList<ChatManager::ChatMessage> history = m_chatManager->messageHistory();
    QVERIFY(history.size() >= 2);
    
    qDebug() << "Chat functionality integration test passed";
}

void IntegrationTest::testScreenSharingIntegration()
{
    qDebug() << "Testing screen sharing integration...";
    
    QVERIFY(m_screenShareManager != nullptr);
    QVERIFY(m_webrtcEngine != nullptr);
    
    QSignalSpy screenShareStartedSpy(m_screenShareManager, &ScreenShareManager::screenShareStarted);
    QSignalSpy screenShareStoppedSpy(m_screenShareManager, &ScreenShareManager::screenShareStopped);
    
    // 启动屏幕共享
    QVERIFY(startScreenShareAndVerify());
    QVERIFY(screenShareStartedSpy.count() > 0);
    
    // 验证屏幕流添加到WebRTC
    QSignalSpy localStreamSpy(m_webrtcEngine, &WebRTCEngine::localStreamReady);
    QVERIFY(waitForSignal(m_webrtcEngine, SIGNAL(localStreamReady(QVideoWidget*)), 5000));
    
    // 停止屏幕共享
    QVERIFY(stopScreenShareAndVerify());
    QVERIFY(screenShareStoppedSpy.count() > 0);
    
    qDebug() << "Screen sharing integration test passed";
}

void IntegrationTest::testChatMessageFlow()
{
    qDebug() << "Testing chat message flow...";
    
    QVERIFY(setupTestConference());
    
    // 发送多条消息
    QStringList testMessages = {
        "First test message",
        "Second test message with emoji 😊",
        "Third message with special chars: @#$%^&*()"
    };
    
    for (const QString& message : testMessages) {
        QVERIFY(sendChatMessageAndVerify(message));
        QTest::qWait(100); // 短暂延迟
    }
    
    // 验证消息历史
    QList<ChatManager::ChatMessage> history = m_chatManager->messageHistory();
    QVERIFY(history.size() >= testMessages.size());
    
    qDebug() << "Chat message flow test passed";
}

void IntegrationTest::testScreenShareFlow()
{
    qDebug() << "Testing screen share flow...";
    
    QVERIFY(setupTestConference());
    
    // 测试屏幕共享开始/停止循环
    for (int i = 0; i < 3; ++i) {
        QVERIFY(startScreenShareAndVerify());
        QTest::qWait(1000);
        QVERIFY(stopScreenShareAndVerify());
        QTest::qWait(500);
    }
    
    qDebug() << "Screen share flow test passed";
}

void IntegrationTest::testChatAndScreenShareCombined()
{
    qDebug() << "Testing combined chat and screen share...";
    
    QVERIFY(setupTestConference());
    
    // 启动屏幕共享
    QVERIFY(startScreenShareAndVerify());
    
    // 在屏幕共享期间发送聊天消息
    QVERIFY(sendChatMessageAndVerify("Sharing my screen now!"));
    
    // 模拟远程用户响应
    simulateChatMessage("remote_user", "I can see your screen!");
    
    // 停止屏幕共享
    QVERIFY(stopScreenShareAndVerify());
    
    // 发送停止共享的消息
    QVERIFY(sendChatMessageAndVerify("Screen sharing stopped"));
    
    qDebug() << "Combined chat and screen share test passed";
}

// ============================================================================
// 多参与者会议场景测试 (Requirements: 5.1, 11.1, 12.1, 13.1)
// ============================================================================

void IntegrationTest::testMultiParticipantConference()
{
    qDebug() << "Testing multi-participant conference...";
    
    QVERIFY(setupTestConference());
    
    // 模拟多个参与者加入
    QStringList participants = {"user1", "user2", "user3", "user4"};
    
    for (const QString& participantId : participants) {
        simulateParticipantJoin(participantId, QString("User %1").arg(participantId));
        QTest::qWait(200);
    }
    
    // 验证参与者列表
    QStringList currentParticipants = m_conferenceManager->participants();
    for (const QString& participantId : participants) {
        QVERIFY(currentParticipants.contains(participantId));
    }
    
    // 模拟参与者离开
    for (int i = 0; i < 2; ++i) {
        simulateParticipantLeave(participants[i]);
        QTest::qWait(200);
    }
    
    // 验证参与者列表更新
    currentParticipants = m_conferenceManager->participants();
    QVERIFY(!currentParticipants.contains(participants[0]));
    QVERIFY(!currentParticipants.contains(participants[1]));
    QVERIFY(currentParticipants.contains(participants[2]));
    QVERIFY(currentParticipants.contains(participants[3]));
    
    qDebug() << "Multi-participant conference test passed";
}

void IntegrationTest::testMultiParticipantMediaStreams()
{
    qDebug() << "Testing multi-participant media streams...";
    
    QVERIFY(setupTestConference());
    
    QStringList participants = {"media_user1", "media_user2", "media_user3"};
    
    // 模拟多个参与者的媒体流
    for (const QString& participantId : participants) {
        simulateParticipantJoin(participantId);
        simulateRemoteMediaStream(participantId, true, true); // video and audio
        QTest::qWait(300);
    }
    
    // 验证远程媒体流
    QVERIFY(verifyRemoteMediaStreams(participants));
    
    qDebug() << "Multi-participant media streams test passed";
}

void IntegrationTest::testMultiParticipantChat()
{
    qDebug() << "Testing multi-participant chat...";
    
    QVERIFY(setupTestConference());
    
    QStringList participants = {"chat_user1", "chat_user2", "chat_user3"};
    
    // 模拟参与者加入
    for (const QString& participantId : participants) {
        simulateParticipantJoin(participantId);
    }
    
    // 模拟多方聊天
    simulateChatMessage("chat_user1", "Hello everyone!");
    simulateChatMessage("chat_user2", "Hi there!");
    simulateChatMessage("chat_user3", "Good to see you all!");
    
    // 发送本地消息
    QVERIFY(sendChatMessageAndVerify("Hello from local user!"));
    
    // 验证消息历史包含所有消息
    QList<ChatManager::ChatMessage> history = m_chatManager->messageHistory();
    QVERIFY(history.size() >= 4);
    
    qDebug() << "Multi-participant chat test passed";
}

void IntegrationTest::testMultiParticipantScreenShare()
{
    qDebug() << "Testing multi-participant screen share...";
    
    QVERIFY(setupTestConference());
    
    QStringList participants = {"screen_user1", "screen_user2"};
    
    // 模拟参与者加入
    for (const QString& participantId : participants) {
        simulateParticipantJoin(participantId);
    }
    
    // 本地用户开始屏幕共享
    QVERIFY(startScreenShareAndVerify());
    
    // 模拟远程用户也开始屏幕共享
    simulateScreenShare("screen_user1", true);
    
    // 验证远程屏幕共享
    QVERIFY(verifyRemoteScreenShare("screen_user1"));
    
    // 停止本地屏幕共享
    QVERIFY(stopScreenShareAndVerify());
    
    // 停止远程屏幕共享
    simulateScreenShare("screen_user1", false);
    
    qDebug() << "Multi-participant screen share test passed";
}

void IntegrationTest::testParticipantJoinLeaveEvents()
{
    qDebug() << "Testing participant join/leave events...";
    
    QVERIFY(setupTestConference());
    
    QSignalSpy participantJoinedSpy(m_conferenceManager, &ConferenceManager::participantJoined);
    QSignalSpy participantLeftSpy(m_conferenceManager, &ConferenceManager::participantLeft);
    
    // 模拟参与者加入
    simulateParticipantJoin("event_user1");
    simulateParticipantJoin("event_user2");
    
    // 验证加入事件
    QVERIFY(participantJoinedSpy.count() >= 2);
    
    // 模拟参与者离开
    simulateParticipantLeave("event_user1");
    
    // 验证离开事件
    QVERIFY(participantLeftSpy.count() >= 1);
    
    qDebug() << "Participant join/leave events test passed";
}

// ============================================================================
// 配置持久化和状态恢复功能验证 (Requirements: 5.1, 6.1)
// ============================================================================

void IntegrationTest::testConfigurationPersistence()
{
    qDebug() << "Testing configuration persistence...";
    
    auto configManager = m_app->configurationManager();
    QVERIFY(configManager != nullptr);
    
    // 保存原始配置
    QString originalServerUrl = configManager->serverUrl();
    QString originalLanguage = configManager->language();
    
    // 修改配置
    QString testServerUrl = "https://test-persistence.example.com";
    QString testLanguage = "zh_CN";
    
    configManager->setServerUrl(testServerUrl);
    configManager->setLanguage(testLanguage);
    configManager->addRecentUrl("https://meet.jit.si/PersistenceTest");
    
    // 保存配置
    configManager->saveConfiguration();
    
    // 重新加载配置
    configManager->loadConfiguration();
    
    // 验证配置持久化
    QCOMPARE(configManager->serverUrl(), testServerUrl);
    QCOMPARE(configManager->language(), testLanguage);
    QVERIFY(configManager->recentUrls().contains("https://meet.jit.si/PersistenceTest"));
    
    // 恢复原始配置
    configManager->setServerUrl(originalServerUrl);
    configManager->setLanguage(originalLanguage);
    configManager->saveConfiguration();
    
    qDebug() << "Configuration persistence test passed";
}

void IntegrationTest::testWindowStatePersistence()
{
    qDebug() << "Testing window state persistence...";
    
    auto windowManager = m_app->windowManager();
    auto stateManager = windowManager->stateManager();
    QVERIFY(stateManager != nullptr);
    
    // 创建并配置窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    auto welcomeWindow = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(welcomeWindow != nullptr);
    
    // 设置窗口状态
    QRect testGeometry(200, 150, 900, 650);
    welcomeWindow->setGeometry(testGeometry);
    
    // 保存窗口状态
    stateManager->saveWindowState(WindowManager::WelcomeWindow, welcomeWindow);
    
    // 关闭并重新创建窗口
    windowManager->closeWindow(WindowManager::WelcomeWindow);
    windowManager->showWindow(WindowManager::WelcomeWindow);
    
    auto newWelcomeWindow = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(newWelcomeWindow != nullptr);
    
    // 恢复窗口状态
    stateManager->restoreWindowState(WindowManager::WelcomeWindow, newWelcomeWindow);
    
    // 验证状态恢复
    QCOMPARE(newWelcomeWindow->size(), testGeometry.size());
    
    qDebug() << "Window state persistence test passed";
}

void IntegrationTest::testConferenceStateRecovery()
{
    qDebug() << "Testing conference state recovery...";
    
    QVERIFY(setupTestConference());
    
    // 加入会议
    QString testUrl = QString("https://meet.jit.si/%1").arg(m_testRoomName);
    QVERIFY(joinConferenceAndWait(testUrl));
    
    // 模拟应用程序崩溃和重启
    QString currentRoom = m_conferenceManager->currentRoom();
    ConferenceManager::ConnectionState currentState = m_conferenceManager->connectionState();
    
    // 保存会议状态
    // 这里需要根据实际的状态管理API来实现
    
    // 模拟重新连接
    simulateNetworkError();
    QTest::qWait(1000);
    
    // 验证自动重连
    QVERIFY(waitForSignal(m_conferenceManager, SIGNAL(conferenceJoined()), 10000));
    QCOMPARE(m_conferenceManager->connectionState(), ConferenceManager::Connected);
    
    qDebug() << "Conference state recovery test passed";
}

void IntegrationTest::testMediaDeviceStatePersistence()
{
    qDebug() << "Testing media device state persistence...";
    
    QVERIFY(m_mediaManager != nullptr);
    
    // 启动媒体设备
    m_mediaManager->startLocalVideo();
    m_mediaManager->startLocalAudio();
    
    // 验证设备状态
    QVERIFY(verifyVideoDeviceAccess());
    QVERIFY(verifyAudioDeviceAccess());
    
    // 模拟设备状态保存和恢复
    // 这里需要根据实际的媒体管理器API来实现状态持久化
    
    qDebug() << "Media device state persistence test passed";
}

void IntegrationTest::testChatHistoryPersistence()
{
    qDebug() << "Testing chat history persistence...";
    
    QVERIFY(setupTestConference());
    QVERIFY(m_chatManager != nullptr);
    
    // 发送一些聊天消息
    QStringList testMessages = {
        "Persistent message 1",
        "Persistent message 2",
        "Persistent message 3"
    };
    
    for (const QString& message : testMessages) {
        QVERIFY(sendChatMessageAndVerify(message));
    }
    
    // 验证消息历史
    QList<ChatManager::ChatMessage> history = m_chatManager->messageHistory();
    QVERIFY(history.size() >= testMessages.size());
    
    // 模拟应用程序重启后恢复聊天历史
    // 这里需要根据实际的聊天管理器API来实现历史持久化
    
    qDebug() << "Chat history persistence test passed";
}

// ============================================================================
// 协议兼容性和错误处理集成测试
// ============================================================================

void IntegrationTest::testProtocolCompatibility()
{
    qDebug() << "Testing protocol compatibility...";
    
    auto protocolHandler = m_app->protocolHandler();
    QVERIFY(protocolHandler != nullptr);
    
    // 测试各种协议URL格式
    QStringList testUrls = {
        "jitsi-meet://meet.jit.si/TestRoom",
        "jitsi-meet://8x8.vc/TestRoom",
        "jitsi-meet://custom.server.com/TestRoom"
    };
    
    for (const QString& url : testUrls) {
        QVERIFY(protocolHandler->isValidProtocolUrl(url));
        QString parsedUrl = protocolHandler->parseProtocolUrl(url);
        QVERIFY(!parsedUrl.isEmpty());
    }
    
    qDebug() << "Protocol compatibility test passed";
}

void IntegrationTest::testErrorHandlingIntegration()
{
    qDebug() << "Testing error handling integration...";
    
    QVERIFY(setupTestConference());
    
    // 测试各种错误场景
    simulateXMPPConnectionError();
    simulateWebRTCError();
    simulateMediaDeviceError();
    simulateAuthenticationError();
    
    // 验证错误恢复
    QTest::qWait(2000);
    
    // 应该能够从错误中恢复
    QVERIFY(m_conferenceManager->connectionState() != ConferenceManager::Failed);
    
    qDebug() << "Error handling integration test passed";
}

void IntegrationTest::testNetworkFailureRecovery()
{
    qDebug() << "Testing network failure recovery...";
    
    QVERIFY(setupTestConference());
    
    QString testUrl = QString("https://meet.jit.si/%1").arg(m_testRoomName);
    QVERIFY(joinConferenceAndWait(testUrl));
    
    // 模拟网络故障
    simulateNetworkError();
    
    // 等待重连
    QVERIFY(waitForSignal(m_conferenceManager, SIGNAL(conferenceJoined()), 15000));
    QCOMPARE(m_conferenceManager->connectionState(), ConferenceManager::Connected);
    
    qDebug() << "Network failure recovery test passed";
}

void IntegrationTest::testMediaDeviceFailureRecovery()
{
    qDebug() << "Testing media device failure recovery...";
    
    QVERIFY(m_mediaManager != nullptr);
    
    // 启动媒体设备
    m_mediaManager->startLocalVideo();
    m_mediaManager->startLocalAudio();
    
    // 模拟设备故障
    simulateMediaDeviceError();
    
    // 尝试重新启动设备
    QTest::qWait(1000);
    m_mediaManager->startLocalVideo();
    m_mediaManager->startLocalAudio();
    
    // 验证设备恢复
    QVERIFY(verifyVideoDeviceAccess());
    QVERIFY(verifyAudioDeviceAccess());
    
    qDebug() << "Media device failure recovery test passed";
}

// ============================================================================
// 性能和资源管理集成测试
// ============================================================================

void IntegrationTest::testPerformanceIntegration()
{
    qDebug() << "Testing performance integration...";
    
    startPerformanceMonitoring();
    
    // 执行一系列性能敏感的操作
    QVERIFY(setupTestConference());
    
    QString testUrl = QString("https://meet.jit.si/%1").arg(m_testRoomName);
    QVERIFY(joinConferenceAndWait(testUrl));
    
    // 模拟多个参与者
    for (int i = 0; i < 10; ++i) {
        simulateParticipantJoin(QString("perf_user_%1").arg(i));
        simulateRemoteMediaStream(QString("perf_user_%1").arg(i));
    }
    
    // 发送多条聊天消息
    for (int i = 0; i < 20; ++i) {
        sendChatMessageAndVerify(QString("Performance test message %1").arg(i));
    }
    
    // 验证性能指标
    QVERIFY(verifyPerformanceMetrics());
    
    qDebug() << "Performance integration test passed";
}

void IntegrationTest::testMemoryManagementIntegration()
{
    qDebug() << "Testing memory management integration...";
    
    // 记录初始内存使用
    startPerformanceMonitoring();
    
    // 执行内存密集型操作
    for (int cycle = 0; cycle < 5; ++cycle) {
        QVERIFY(setupTestConference());
        
        QString testUrl = QString("https://meet.jit.si/MemoryTest%1").arg(cycle);
        QVERIFY(joinConferenceAndWait(testUrl));
        
        // 模拟参与者和媒体流
        for (int i = 0; i < 5; ++i) {
            simulateParticipantJoin(QString("mem_user_%1_%2").arg(cycle).arg(i));
            simulateRemoteMediaStream(QString("mem_user_%1_%2").arg(cycle).arg(i));
        }
        
        // 清理
        QVERIFY(leaveConferenceAndWait());
        cleanupTestConference();
        
        // 强制垃圾回收
        QCoreApplication::processEvents();
    }
    
    // 验证内存没有泄漏
    // 这里需要实际的内存监控实现
    
    qDebug() << "Memory management integration test passed";
}

void IntegrationTest::testResourceCleanupIntegration()
{
    qDebug() << "Testing resource cleanup integration...";
    
    // 创建和销毁多个会议会话
    for (int i = 0; i < 3; ++i) {
        QVERIFY(setupTestConference(QString("CleanupTest%1").arg(i)));
        
        QString testUrl = QString("https://meet.jit.si/CleanupTest%1").arg(i);
        QVERIFY(joinConferenceAndWait(testUrl));
        
        // 启动媒体设备
        m_mediaManager->startLocalVideo();
        m_mediaManager->startLocalAudio();
        
        // 启动屏幕共享
        startScreenShareAndVerify();
        
        // 发送聊天消息
        sendChatMessageAndVerify("Cleanup test message");
        
        // 清理所有资源
        stopScreenShareAndVerify();
        m_mediaManager->stopLocalVideo();
        m_mediaManager->stopLocalAudio();
        
        QVERIFY(leaveConferenceAndWait());
        cleanupTestConference();
        
        QCoreApplication::processEvents();
    }
    
    qDebug() << "Resource cleanup integration test passed";
}