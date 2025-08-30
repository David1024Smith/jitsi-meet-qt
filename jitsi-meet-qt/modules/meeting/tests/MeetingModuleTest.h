#ifndef MEETINGMODULETEST_H
#define MEETINGMODULETEST_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include <QVariantMap>
#include <memory>

// Forward declarations
class MeetingModule;
class MeetingManager;
class LinkHandler;
class MeetingConfig;
class MeetingWidget;
class JoinDialog;
class CreateDialog;
class Meeting;
class Room;
class Invitation;
class URLHandler;
class ProtocolHandler;
class AuthHandler;

/**
 * @brief Meeting模块测试类
 * 
 * 提供完整的Meeting模块测试套件，包括：
 * - 链接解析和会议创建测试
 * - 会议加入和认证测试
 * - 会议UI组件交互测试
 * - 与现有ConferenceManager的兼容性测试
 */
class MeetingModuleTest : public QObject
{
    Q_OBJECT

public:
    explicit MeetingModuleTest(QObject* parent = nullptr);
    ~MeetingModuleTest();

private slots:
    // 测试框架生命周期
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 1. 模块基础测试
    void testModuleInitialization();
    void testModuleConfiguration();
    void testModuleLifecycle();
    void testModuleDependencies();

    // 2. 链接解析测试
    void testUrlParsing_data();
    void testUrlParsing();
    void testUrlValidation_data();
    void testUrlValidation();
    void testUrlNormalization_data();
    void testUrlNormalization();
    void testParameterExtraction_data();
    void testParameterExtraction();
    void testLinkTypeDetection_data();
    void testLinkTypeDetection();
    void testUrlBuilding_data();
    void testUrlBuilding();

    // 3. 会议创建测试
    void testMeetingCreation_data();
    void testMeetingCreation();
    void testMeetingCreationWithSettings();
    void testMeetingCreationValidation();
    void testMeetingCreationErrors();
    void testMeetingUrlGeneration();

    // 4. 会议加入测试
    void testMeetingJoin_data();
    void testMeetingJoin();
    void testMeetingJoinWithCredentials();
    void testMeetingJoinValidation();
    void testMeetingJoinErrors();
    void testMeetingJoinTimeout();
    void testMeetingReconnection();

    // 5. 认证测试
    void testAuthentication_data();
    void testAuthentication();
    void testAuthenticationFailure();
    void testAuthenticationTimeout();
    void testTokenValidation();
    void testPermissionChecking();

    // 6. 会议管理测试
    void testMeetingStateManagement();
    void testParticipantManagement();
    void testMeetingSettings();
    void testMeetingStatistics();
    void testMeetingInvitation();
    void testMeetingLeaving();

    // 7. 数据模型测试
    void testMeetingModel();
    void testRoomModel();
    void testInvitationModel();
    void testModelSerialization();
    void testModelValidation();

    // 8. 处理器测试
    void testURLHandler();
    void testProtocolHandler();
    void testAuthHandler();
    void testHandlerChaining();
    void testHandlerErrors();

    // 9. UI组件测试
    void testMeetingWidget();
    void testJoinDialog();
    void testCreateDialog();
    void testUIStateUpdates();
    void testUIInteractions();
    void testUIThemeSupport();

    // 10. 配置管理测试
    void testConfigurationLoad();
    void testConfigurationSave();
    void testConfigurationValidation();
    void testConfigurationDefaults();
    void testConfigurationMigration();

    // 11. 错误处理测试
    void testNetworkErrors();
    void testServerErrors();
    void testValidationErrors();
    void testRecoveryMechanisms();
    void testErrorPropagation();

    // 12. 性能测试
    void testModuleStartupTime();
    void testMemoryUsage();
    void testConcurrentOperations();
    void testLargeDataHandling();
    void testResourceCleanup();

    // 13. 集成测试
    void testModuleIntegration();
    void testNetworkIntegration();
    void testUIIntegration();
    void testConfigIntegration();

    // 14. 兼容性测试
    void testConferenceManagerCompatibility();
    void testLegacyAPICompatibility();
    void testBackwardCompatibility();
    void testVersionCompatibility();

    // 15. 端到端测试
    void testCompleteWorkflow();
    void testMeetingLifecycle();
    void testMultiUserScenario();
    void testErrorRecoveryWorkflow();

private:
    // 测试辅助方法
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    void createTestMeeting(const QString& name = "Test Meeting");
    void createTestRoom(const QString& name = "test-room");
    void createTestInvitation(const QString& email = "test@example.com");
    
    // 验证辅助方法
    bool verifyMeetingState(int expectedState);
    bool verifyUrlFormat(const QString& url);
    bool verifyMeetingInfo(const QVariantMap& info);
    bool verifyParticipantInfo(const QVariantMap& participant);
    
    // 模拟辅助方法
    void simulateNetworkDelay(int ms = 100);
    void simulateNetworkError();
    void simulateServerResponse(const QVariantMap& response);
    void simulateUserInteraction();
    
    // 数据生成辅助方法
    QString generateTestUrl(const QString& server = "meet.jit.si", 
                          const QString& room = "test-room");
    QVariantMap generateTestMeetingInfo();
    QVariantMap generateTestParticipant();
    QVariantMap generateTestSettings();
    
    // 等待辅助方法
    bool waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
    bool waitForState(int expectedState, int timeout = 5000);
    bool waitForConnection(int timeout = 10000);
    
    // 断言辅助方法
    void assertMeetingState(int expectedState);
    void assertUrlValid(const QString& url);
    void assertErrorOccurred(const QString& expectedError = QString());
    void assertSignalEmitted(QSignalSpy* spy, int expectedCount = 1);

private:
    // 测试对象
    std::unique_ptr<MeetingModule> m_meetingModule;
    std::unique_ptr<MeetingManager> m_meetingManager;
    std::unique_ptr<LinkHandler> m_linkHandler;
    std::unique_ptr<MeetingConfig> m_meetingConfig;
    std::unique_ptr<MeetingWidget> m_meetingWidget;
    std::unique_ptr<JoinDialog> m_joinDialog;
    std::unique_ptr<CreateDialog> m_createDialog;
    
    // 测试数据
    std::unique_ptr<Meeting> m_testMeeting;
    std::unique_ptr<Room> m_testRoom;
    std::unique_ptr<Invitation> m_testInvitation;
    
    // 处理器对象
    std::unique_ptr<URLHandler> m_urlHandler;
    std::unique_ptr<ProtocolHandler> m_protocolHandler;
    std::unique_ptr<AuthHandler> m_authHandler;
    
    // 信号监听器
    std::unique_ptr<QSignalSpy> m_stateChangedSpy;
    std::unique_ptr<QSignalSpy> m_meetingCreatedSpy;
    std::unique_ptr<QSignalSpy> m_meetingJoinedSpy;
    std::unique_ptr<QSignalSpy> m_meetingLeftSpy;
    std::unique_ptr<QSignalSpy> m_errorOccurredSpy;
    std::unique_ptr<QSignalSpy> m_urlParsedSpy;
    std::unique_ptr<QSignalSpy> m_urlValidatedSpy;
    
    // 测试配置
    QVariantMap m_testConfig;
    QString m_testServerUrl;
    QString m_testRoomName;
    QString m_testDisplayName;
    
    // 测试状态
    bool m_testEnvironmentReady;
    bool m_networkAvailable;
    int m_testTimeout;
    
    // 性能测试数据
    qint64 m_startupTime;
    qint64 m_memoryUsage;
    int m_operationCount;
};

#endif // MEETINGMODULETEST_H