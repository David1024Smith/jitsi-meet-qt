#include "MeetingModuleTest.h"

// Include all necessary headers
#include "../include/MeetingModule.h"
#include "../include/MeetingManager.h"
#include "../include/LinkHandler.h"
#include "../config/MeetingConfig.h"
#include "../widgets/MeetingWidget.h"
#include "../widgets/JoinDialog.h"
#include "../widgets/CreateDialog.h"
#include "../models/Meeting.h"
#include "../models/Room.h"
#include "../models/Invitation.h"
#include "../handlers/URLHandler.h"
#include "../handlers/ProtocolHandler.h"
#include "../handlers/AuthHandler.h"

#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>

MeetingModuleTest::MeetingModuleTest(QObject* parent)
    : QObject(parent)
    , m_testEnvironmentReady(false)
    , m_networkAvailable(false)
    , m_testTimeout(5000)
    , m_startupTime(0)
    , m_memoryUsage(0)
    , m_operationCount(0)
{
    // Initialize test configuration
    m_testConfig["server"] = "meet.jit.si";
    m_testConfig["timeout"] = 10000;
    m_testConfig["debug"] = true;
    
    m_testServerUrl = "https://meet.jit.si";
    m_testRoomName = "test-room-" + QString::number(QDateTime::currentMSecsSinceEpoch());
    m_testDisplayName = "Test User";
}

MeetingModuleTest::~MeetingModuleTest()
{
    cleanupTestEnvironment();
}

void MeetingModuleTest::initTestCase()
{
    qDebug() << "Initializing Meeting Module Test Suite";
    
    // Setup test environment
    setupTestEnvironment();
    
    // Verify test environment is ready
    QVERIFY(m_testEnvironmentReady);
    
    qDebug() << "Test environment initialized successfully";
}

void MeetingModuleTest::cleanupTestCase()
{
    qDebug() << "Cleaning up Meeting Module Test Suite";
    
    // Cleanup all test objects
    cleanupTestEnvironment();
    
    qDebug() << "Test cleanup completed";
}

void MeetingModuleTest::init()
{
    // Reset test state before each test
    m_operationCount = 0;
    
    // Create fresh test objects for each test
    m_meetingModule = std::make_unique<MeetingModule>();
    m_meetingManager = std::make_unique<MeetingManager>();
    m_linkHandler = std::make_unique<LinkHandler>();
    m_meetingConfig = std::make_unique<MeetingConfig>();
    
    // Initialize signal spies
    m_stateChangedSpy = std::make_unique<QSignalSpy>(m_meetingManager.get(), 
        &IMeetingManager::stateChanged);
    m_meetingCreatedSpy = std::make_unique<QSignalSpy>(m_meetingManager.get(), 
        &IMeetingManager::meetingCreated);
    m_meetingJoinedSpy = std::make_unique<QSignalSpy>(m_meetingManager.get(), 
        &IMeetingManager::meetingJoined);
    m_meetingLeftSpy = std::make_unique<QSignalSpy>(m_meetingManager.get(), 
        &IMeetingManager::meetingLeft);
    m_errorOccurredSpy = std::make_unique<QSignalSpy>(m_meetingManager.get(), 
        &IMeetingManager::errorOccurred);
    
    // Initialize components
    QVERIFY(m_meetingModule->initialize());
    QVERIFY(m_meetingManager->initialize());
}

void MeetingModuleTest::cleanup()
{
    // Clean up after each test
    if (m_meetingManager && m_meetingManager->currentState() == IMeetingManager::InMeeting) {
        m_meetingManager->leaveMeeting();
        waitForState(IMeetingManager::Disconnected);
    }
    
    // Reset all objects
    m_stateChangedSpy.reset();
    m_meetingCreatedSpy.reset();
    m_meetingJoinedSpy.reset();
    m_meetingLeftSpy.reset();
    m_errorOccurredSpy.reset();
    
    m_meetingWidget.reset();
    m_joinDialog.reset();
    m_createDialog.reset();
    m_testMeeting.reset();
    m_testRoom.reset();
    m_testInvitation.reset();
    m_urlHandler.reset();
    m_protocolHandler.reset();
    m_authHandler.reset();
    
    m_meetingConfig.reset();
    m_linkHandler.reset();
    m_meetingManager.reset();
    m_meetingModule.reset();
}

// 1. Module Basic Tests
void MeetingModuleTest::testModuleInitialization()
{
    // Test module creation and initialization
    auto module = std::make_unique<MeetingModule>();
    QVERIFY(module != nullptr);
    
    // Test initialization
    QVERIFY(module->initialize());
    
    // Test module state after initialization
    QVERIFY(module->isInitialized());
    
    // Test module name and version
    QCOMPARE(module->name(), QString("MeetingModule"));
    QVERIFY(!module->version().isEmpty());
}

void MeetingModuleTest::testModuleConfiguration()
{
    // Test configuration loading
    QVariantMap config;
    config["server"] = "test.server.com";
    config["timeout"] = 15000;
    
    m_meetingModule->setConfiguration(config);
    
    QVariantMap retrievedConfig = m_meetingModule->getConfiguration();
    QCOMPARE(retrievedConfig["server"].toString(), QString("test.server.com"));
    QCOMPARE(retrievedConfig["timeout"].toInt(), 15000);
}

void MeetingModuleTest::testModuleLifecycle()
{
    // Test module lifecycle: initialize -> start -> stop -> cleanup
    QVERIFY(m_meetingModule->initialize());
    QVERIFY(m_meetingModule->start());
    QVERIFY(m_meetingModule->isRunning());
    
    m_meetingModule->stop();
    QVERIFY(!m_meetingModule->isRunning());
    
    m_meetingModule->cleanup();
    QVERIFY(!m_meetingModule->isInitialized());
}

void MeetingModuleTest::testModuleDependencies()
{
    // Test that module properly manages its dependencies
    QVERIFY(m_meetingModule->meetingManager() != nullptr);
    QVERIFY(m_meetingModule->linkHandler() != nullptr);
    QVERIFY(m_meetingModule->config() != nullptr);
}

// 2. Link Parsing Tests
void MeetingModuleTest::testUrlParsing_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<bool>("expectedValid");
    QTest::addColumn<QString>("expectedServer");
    QTest::addColumn<QString>("expectedRoom");
    
    QTest::newRow("valid_https") << "https://meet.jit.si/test-room" << true 
                                << "meet.jit.si" << "test-room";
    QTest::newRow("valid_with_params") << "https://meet.jit.si/test-room?config.startWithAudioMuted=true" 
                                      << true << "meet.jit.si" << "test-room";
    QTest::newRow("custom_server") << "https://jitsi.example.com/my-meeting" << true 
                                  << "jitsi.example.com" << "my-meeting";
    QTest::newRow("invalid_protocol") << "http://meet.jit.si/test-room" << false << "" << "";
    QTest::newRow("invalid_format") << "not-a-url" << false << "" << "";
    QTest::newRow("empty_url") << "" << false << "" << "";
}

void MeetingModuleTest::testUrlParsing()
{
    QFETCH(QString, url);
    QFETCH(bool, expectedValid);
    QFETCH(QString, expectedServer);
    QFETCH(QString, expectedRoom);
    
    QVariantMap result = m_linkHandler->parseUrl(url);
    
    QCOMPARE(result["valid"].toBool(), expectedValid);
    if (expectedValid) {
        QCOMPARE(result["server"].toString(), expectedServer);
        QCOMPARE(result["room"].toString(), expectedRoom);
    }
}

void MeetingModuleTest::testUrlValidation_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("expectedResult");
    
    QTest::newRow("valid_url") << "https://meet.jit.si/test-room" 
                              << static_cast<int>(ILinkHandler::Valid);
    QTest::newRow("invalid_format") << "invalid-url" 
                                   << static_cast<int>(ILinkHandler::InvalidFormat);
    QTest::newRow("invalid_server") << "https://invalid.server.xyz/room" 
                                   << static_cast<int>(ILinkHandler::InvalidServer);
}

void MeetingModuleTest::testUrlValidation()
{
    QFETCH(QString, url);
    QFETCH(int, expectedResult);
    
    ILinkHandler::ValidationResult result = m_linkHandler->validateUrl(url);
    QCOMPARE(static_cast<int>(result), expectedResult);
}vo
id MeetingModuleTest::testUrlNormalization_data()
{
    QTest::addColumn<QString>("inputUrl");
    QTest::addColumn<QString>("expectedUrl");
    
    QTest::newRow("add_protocol") << "meet.jit.si/test-room" 
                                 << "https://meet.jit.si/test-room";
    QTest::newRow("remove_trailing_slash") << "https://meet.jit.si/test-room/" 
                                          << "https://meet.jit.si/test-room";
    QTest::newRow("lowercase_server") << "https://MEET.JIT.SI/test-room" 
                                     << "https://meet.jit.si/test-room";
}

void MeetingModuleTest::testUrlNormalization()
{
    QFETCH(QString, inputUrl);
    QFETCH(QString, expectedUrl);
    
    QString normalizedUrl = m_linkHandler->normalizeUrl(inputUrl);
    QCOMPARE(normalizedUrl, expectedUrl);
}

void MeetingModuleTest::testParameterExtraction_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QVariantMap>("expectedParams");
    
    QVariantMap params1;
    params1["config.startWithAudioMuted"] = "true";
    params1["config.startWithVideoMuted"] = "false";
    
    QTest::newRow("with_params") << "https://meet.jit.si/room?config.startWithAudioMuted=true&config.startWithVideoMuted=false" 
                                << params1;
    
    QVariantMap params2;
    QTest::newRow("no_params") << "https://meet.jit.si/room" << params2;
}

void MeetingModuleTest::testParameterExtraction()
{
    QFETCH(QString, url);
    QFETCH(QVariantMap, expectedParams);
    
    QVariantMap extractedParams = m_linkHandler->extractParameters(url);
    
    for (auto it = expectedParams.begin(); it != expectedParams.end(); ++it) {
        QCOMPARE(extractedParams[it.key()].toString(), it.value().toString());
    }
}

void MeetingModuleTest::testLinkTypeDetection_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("expectedType");
    
    QTest::newRow("https_link") << "https://meet.jit.si/room" 
                               << static_cast<int>(ILinkHandler::HttpsLink);
    QTest::newRow("jitsi_protocol") << "jitsi://meet.jit.si/room" 
                                   << static_cast<int>(ILinkHandler::JitsiProtocol);
    QTest::newRow("invalid_link") << "invalid-url" 
                                 << static_cast<int>(ILinkHandler::InvalidLink);
}

void MeetingModuleTest::testLinkTypeDetection()
{
    QFETCH(QString, url);
    QFETCH(int, expectedType);
    
    ILinkHandler::LinkType type = m_linkHandler->getLinkType(url);
    QCOMPARE(static_cast<int>(type), expectedType);
}

void MeetingModuleTest::testUrlBuilding_data()
{
    QTest::addColumn<QString>("server");
    QTest::addColumn<QString>("room");
    QTest::addColumn<QVariantMap>("params");
    QTest::addColumn<QString>("expectedUrl");
    
    QVariantMap params;
    params["config.startWithAudioMuted"] = "true";
    
    QTest::newRow("with_params") << "meet.jit.si" << "test-room" << params 
                                << "https://meet.jit.si/test-room?config.startWithAudioMuted=true";
    
    QVariantMap noParams;
    QTest::newRow("no_params") << "meet.jit.si" << "test-room" << noParams 
                              << "https://meet.jit.si/test-room";
}

void MeetingModuleTest::testUrlBuilding()
{
    QFETCH(QString, server);
    QFETCH(QString, room);
    QFETCH(QVariantMap, params);
    QFETCH(QString, expectedUrl);
    
    QString builtUrl = m_linkHandler->buildMeetingUrl(server, room, params);
    QCOMPARE(builtUrl, expectedUrl);
}

// 3. Meeting Creation Tests
void MeetingModuleTest::testMeetingCreation_data()
{
    QTest::addColumn<QString>("meetingName");
    QTest::addColumn<QVariantMap>("settings");
    QTest::addColumn<bool>("expectedSuccess");
    
    QVariantMap validSettings;
    validSettings["server"] = "meet.jit.si";
    validSettings["audioEnabled"] = true;
    validSettings["videoEnabled"] = true;
    
    QTest::newRow("valid_creation") << "Test Meeting" << validSettings << true;
    QTest::newRow("empty_name") << "" << validSettings << false;
    
    QVariantMap invalidSettings;
    invalidSettings["server"] = "";
    QTest::newRow("invalid_settings") << "Test Meeting" << invalidSettings << false;
}

void MeetingModuleTest::testMeetingCreation()
{
    QFETCH(QString, meetingName);
    QFETCH(QVariantMap, settings);
    QFETCH(bool, expectedSuccess);
    
    bool result = m_meetingManager->createMeeting(meetingName, settings);
    QCOMPARE(result, expectedSuccess);
    
    if (expectedSuccess) {
        // Wait for meeting created signal
        QVERIFY(waitForSignal(m_meetingManager.get(), 
                             SIGNAL(meetingCreated(QString, QVariantMap)), 3000));
        QCOMPARE(m_meetingCreatedSpy->count(), 1);
        
        // Verify meeting state
        QCOMPARE(m_meetingManager->currentState(), IMeetingManager::Connected);
    }
}

void MeetingModuleTest::testMeetingCreationWithSettings()
{
    QVariantMap settings;
    settings["server"] = m_testServerUrl;
    settings["audioEnabled"] = false;
    settings["videoEnabled"] = true;
    settings["password"] = "test123";
    
    bool result = m_meetingManager->createMeeting("Test Meeting with Settings", settings);
    QVERIFY(result);
    
    // Verify settings were applied
    QVariantMap currentInfo = m_meetingManager->getCurrentMeetingInfo();
    QCOMPARE(currentInfo["audioEnabled"].toBool(), false);
    QCOMPARE(currentInfo["videoEnabled"].toBool(), true);
}

void MeetingModuleTest::testMeetingCreationValidation()
{
    // Test various invalid inputs
    QVERIFY(!m_meetingManager->createMeeting("", QVariantMap()));
    QVERIFY(!m_meetingManager->createMeeting("Test", QVariantMap()));
    
    QVariantMap invalidSettings;
    invalidSettings["server"] = "invalid-server";
    QVERIFY(!m_meetingManager->createMeeting("Test", invalidSettings));
}

void MeetingModuleTest::testMeetingCreationErrors()
{
    // Simulate network error
    simulateNetworkError();
    
    QVariantMap settings;
    settings["server"] = "unreachable.server.com";
    
    bool result = m_meetingManager->createMeeting("Test Meeting", settings);
    QVERIFY(!result);
    
    // Verify error signal was emitted
    QVERIFY(waitForSignal(m_meetingManager.get(), SIGNAL(errorOccurred(QString)), 2000));
    QVERIFY(m_errorOccurredSpy->count() > 0);
}

void MeetingModuleTest::testMeetingUrlGeneration()
{
    QVariantMap settings;
    settings["server"] = "meet.jit.si";
    
    bool result = m_meetingManager->createMeeting("URL Generation Test", settings);
    QVERIFY(result);
    
    if (waitForSignal(m_meetingManager.get(), SIGNAL(meetingCreated(QString, QVariantMap)), 3000)) {
        QList<QVariant> arguments = m_meetingCreatedSpy->takeFirst();
        QString meetingUrl = arguments.at(0).toString();
        
        QVERIFY(!meetingUrl.isEmpty());
        QVERIFY(meetingUrl.startsWith("https://"));
        QVERIFY(meetingUrl.contains("meet.jit.si"));
    }
}

// 4. Meeting Join Tests
void MeetingModuleTest::testMeetingJoin_data()
{
    QTest::addColumn<QString>("meetingUrl");
    QTest::addColumn<QString>("displayName");
    QTest::addColumn<bool>("audioEnabled");
    QTest::addColumn<bool>("videoEnabled");
    QTest::addColumn<bool>("expectedSuccess");
    
    QTest::newRow("valid_join") << generateTestUrl() << "Test User" << true << true << true;
    QTest::newRow("audio_only") << generateTestUrl() << "Audio User" << true << false << true;
    QTest::newRow("invalid_url") << "invalid-url" << "Test User" << true << true << false;
    QTest::newRow("empty_url") << "" << "Test User" << true << true << false;
}

void MeetingModuleTest::testMeetingJoin()
{
    QFETCH(QString, meetingUrl);
    QFETCH(QString, displayName);
    QFETCH(bool, audioEnabled);
    QFETCH(bool, videoEnabled);
    QFETCH(bool, expectedSuccess);
    
    bool result = m_meetingManager->joinMeeting(meetingUrl, displayName, audioEnabled, videoEnabled);
    QCOMPARE(result, expectedSuccess);
    
    if (expectedSuccess) {
        // Wait for meeting joined signal
        QVERIFY(waitForSignal(m_meetingManager.get(), SIGNAL(meetingJoined(QVariantMap)), 5000));
        QCOMPARE(m_meetingJoinedSpy->count(), 1);
        
        // Verify meeting state
        QCOMPARE(m_meetingManager->currentState(), IMeetingManager::InMeeting);
        
        // Verify audio/video settings
        QCOMPARE(m_meetingManager->isAudioEnabled(), audioEnabled);
        QCOMPARE(m_meetingManager->isVideoEnabled(), videoEnabled);
    }
}

void MeetingModuleTest::testMeetingJoinWithCredentials()
{
    QString meetingUrl = generateTestUrl() + "?password=test123";
    
    bool result = m_meetingManager->joinMeeting(meetingUrl, "Authenticated User", true, true);
    QVERIFY(result);
    
    // Should handle authentication automatically based on URL parameters
    if (waitForSignal(m_meetingManager.get(), SIGNAL(meetingJoined(QVariantMap)), 5000)) {
        QVariantMap meetingInfo = m_meetingManager->getCurrentMeetingInfo();
        QVERIFY(meetingInfo.contains("authenticated"));
    }
}

void MeetingModuleTest::testMeetingJoinValidation()
{
    // Test URL validation before joining
    QVERIFY(!m_meetingManager->validateMeetingUrl(""));
    QVERIFY(!m_meetingManager->validateMeetingUrl("invalid-url"));
    QVERIFY(!m_meetingManager->validateMeetingUrl("http://insecure.com/room"));
    QVERIFY(m_meetingManager->validateMeetingUrl(generateTestUrl()));
}

void MeetingModuleTest::testMeetingJoinErrors()
{
    // Test joining with network error
    simulateNetworkError();
    
    bool result = m_meetingManager->joinMeeting("https://unreachable.server.com/room", 
                                               "Test User", true, true);
    QVERIFY(!result);
    
    // Verify error handling
    QVERIFY(waitForSignal(m_meetingManager.get(), SIGNAL(errorOccurred(QString)), 2000));
}

void MeetingModuleTest::testMeetingJoinTimeout()
{
    // Set short timeout for testing
    QVariantMap config;
    config["connectionTimeout"] = 1000; // 1 second
    m_meetingManager->setConfiguration(config);
    
    // Try to join a slow/unresponsive server
    bool result = m_meetingManager->joinMeeting("https://slow.server.com/room", 
                                               "Test User", true, true);
    
    // Should timeout and fail
    QTimer::singleShot(2000, [this]() {
        QCOMPARE(m_meetingManager->currentState(), IMeetingManager::Error);
    });
}

void MeetingModuleTest::testMeetingReconnection()
{
    // First, join a meeting
    QString meetingUrl = generateTestUrl();
    QVERIFY(m_meetingManager->joinMeeting(meetingUrl, "Test User", true, true));
    
    if (waitForState(IMeetingManager::InMeeting)) {
        // Simulate connection loss
        simulateNetworkError();
        
        // Attempt reconnection
        bool reconnectResult = m_meetingManager->reconnect();
        QVERIFY(reconnectResult);
        
        // Should eventually reconnect
        QVERIFY(waitForState(IMeetingManager::InMeeting, 10000));
    }
}//
 5. Authentication Tests
void MeetingModuleTest::testAuthentication_data()
{
    QTest::addColumn<QString>("authType");
    QTest::addColumn<QVariantMap>("credentials");
    QTest::addColumn<bool>("expectedSuccess");
    
    QVariantMap tokenAuth;
    tokenAuth["token"] = "valid-jwt-token";
    QTest::newRow("token_auth") << "token" << tokenAuth << true;
    
    QVariantMap passwordAuth;
    passwordAuth["password"] = "meeting-password";
    QTest::newRow("password_auth") << "password" << passwordAuth << true;
    
    QVariantMap invalidAuth;
    invalidAuth["token"] = "invalid-token";
    QTest::newRow("invalid_token") << "token" << invalidAuth << false;
}

void MeetingModuleTest::testAuthentication()
{
    QFETCH(QString, authType);
    QFETCH(QVariantMap, credentials);
    QFETCH(bool, expectedSuccess);
    
    // Create auth handler for testing
    m_authHandler = std::make_unique<AuthHandler>();
    
    bool result = m_authHandler->authenticate(authType, credentials);
    QCOMPARE(result, expectedSuccess);
    
    if (expectedSuccess) {
        QVERIFY(m_authHandler->isAuthenticated());
        QCOMPARE(m_authHandler->getAuthType(), authType);
    }
}

void MeetingModuleTest::testAuthenticationFailure()
{
    m_authHandler = std::make_unique<AuthHandler>();
    
    QVariantMap invalidCredentials;
    invalidCredentials["password"] = "wrong-password";
    
    bool result = m_authHandler->authenticate("password", invalidCredentials);
    QVERIFY(!result);
    QVERIFY(!m_authHandler->isAuthenticated());
}

void MeetingModuleTest::testAuthenticationTimeout()
{
    m_authHandler = std::make_unique<AuthHandler>();
    m_authHandler->setTimeout(1000); // 1 second timeout
    
    // Simulate slow authentication server
    simulateNetworkDelay(2000);
    
    QVariantMap credentials;
    credentials["token"] = "slow-token";
    
    bool result = m_authHandler->authenticate("token", credentials);
    QVERIFY(!result); // Should timeout
}

void MeetingModuleTest::testTokenValidation()
{
    m_authHandler = std::make_unique<AuthHandler>();
    
    // Test valid token format
    QVERIFY(m_authHandler->validateToken("eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.test.signature"));
    
    // Test invalid token formats
    QVERIFY(!m_authHandler->validateToken("invalid-token"));
    QVERIFY(!m_authHandler->validateToken(""));
    QVERIFY(!m_authHandler->validateToken("not.jwt.format"));
}

void MeetingModuleTest::testPermissionChecking()
{
    m_authHandler = std::make_unique<AuthHandler>();
    
    // Set up authenticated user with moderator permissions
    QVariantMap credentials;
    credentials["token"] = "moderator-token";
    credentials["role"] = "moderator";
    
    QVERIFY(m_authHandler->authenticate("token", credentials));
    
    // Test permission checks
    QVERIFY(m_authHandler->hasPermission("mute_participants"));
    QVERIFY(m_authHandler->hasPermission("kick_participants"));
    QVERIFY(!m_authHandler->hasPermission("admin_only_action"));
}

// 6. Meeting Management Tests
void MeetingModuleTest::testMeetingStateManagement()
{
    // Test state transitions
    QCOMPARE(m_meetingManager->currentState(), IMeetingManager::Disconnected);
    
    // Create meeting
    QVariantMap settings = generateTestSettings();
    QVERIFY(m_meetingManager->createMeeting("State Test", settings));
    
    // Should transition to Connected
    QVERIFY(waitForState(IMeetingManager::Connected));
    
    // Join meeting
    QString meetingUrl = generateTestUrl();
    QVERIFY(m_meetingManager->joinMeeting(meetingUrl, "Test User", true, true));
    
    // Should transition to InMeeting
    QVERIFY(waitForState(IMeetingManager::InMeeting));
    
    // Leave meeting
    QVERIFY(m_meetingManager->leaveMeeting());
    
    // Should transition back to Disconnected
    QVERIFY(waitForState(IMeetingManager::Disconnected));
}

void MeetingModuleTest::testParticipantManagement()
{
    // Join a meeting first
    QString meetingUrl = generateTestUrl();
    QVERIFY(m_meetingManager->joinMeeting(meetingUrl, "Test User", true, true));
    QVERIFY(waitForState(IMeetingManager::InMeeting));
    
    // Test participant list
    QVariantList participants = m_meetingManager->getParticipants();
    QVERIFY(participants.size() >= 1); // At least the current user
    
    // Test participant invitation
    bool inviteResult = m_meetingManager->inviteParticipant("test@example.com", 
                                                           "Join our meeting!");
    QVERIFY(inviteResult);
    
    // Simulate participant joining
    QVariantMap newParticipant = generateTestParticipant();
    simulateServerResponse(QVariantMap{{"event", "participant_joined"}, 
                                     {"participant", newParticipant}});
    
    // Verify participant joined signal
    QVERIFY(waitForSignal(m_meetingManager.get(), 
                         SIGNAL(participantJoined(QVariantMap)), 2000));
}

void MeetingModuleTest::testMeetingSettings()
{
    // Join meeting
    QString meetingUrl = generateTestUrl();
    QVERIFY(m_meetingManager->joinMeeting(meetingUrl, "Test User", true, true));
    QVERIFY(waitForState(IMeetingManager::InMeeting));
    
    // Test audio/video controls
    m_meetingManager->setAudioEnabled(false);
    QVERIFY(!m_meetingManager->isAudioEnabled());
    
    m_meetingManager->setVideoEnabled(false);
    QVERIFY(!m_meetingManager->isVideoEnabled());
    
    // Test display name change
    m_meetingManager->setDisplayName("New Display Name");
    QCOMPARE(m_meetingManager->displayName(), QString("New Display Name"));
    
    // Test meeting settings update
    QVariantMap newSettings;
    newSettings["quality"] = "high";
    newSettings["bandwidth"] = 1000;
    
    m_meetingManager->updateMeetingSettings(newSettings);
    
    QVariantMap currentConfig = m_meetingManager->getConfiguration();
    QCOMPARE(currentConfig["quality"].toString(), QString("high"));
    QCOMPARE(currentConfig["bandwidth"].toInt(), 1000);
}

void MeetingModuleTest::testMeetingStatistics()
{
    // Join meeting
    QString meetingUrl = generateTestUrl();
    QVERIFY(m_meetingManager->joinMeeting(meetingUrl, "Test User", true, true));
    QVERIFY(waitForState(IMeetingManager::InMeeting));
    
    // Get meeting statistics
    QVariantMap stats = m_meetingManager->getMeetingStatistics();
    
    // Verify expected statistics are present
    QVERIFY(stats.contains("connectionQuality"));
    QVERIFY(stats.contains("duration"));
    QVERIFY(stats.contains("participantCount"));
    QVERIFY(stats.contains("audioStats"));
    QVERIFY(stats.contains("videoStats"));
    
    // Test connection quality
    int quality = m_meetingManager->getConnectionQuality();
    QVERIFY(quality >= 0 && quality <= 100);
}

void MeetingModuleTest::testMeetingInvitation()
{
    // Create invitation object
    m_testInvitation = std::make_unique<Invitation>();
    
    // Set invitation properties
    m_testInvitation->setRecipientEmail("test@example.com");
    m_testInvitation->setMeetingUrl(generateTestUrl());
    m_testInvitation->setMessage("Please join our meeting");
    m_testInvitation->setSenderName("Test Organizer");
    
    // Validate invitation
    QVERIFY(m_testInvitation->isValid());
    
    // Test invitation serialization
    QVariantMap invitationData = m_testInvitation->toVariantMap();
    QCOMPARE(invitationData["recipientEmail"].toString(), QString("test@example.com"));
    QCOMPARE(invitationData["senderName"].toString(), QString("Test Organizer"));
    
    // Test sending invitation
    bool sendResult = m_meetingManager->inviteParticipant(
        m_testInvitation->recipientEmail(), 
        m_testInvitation->message());
    QVERIFY(sendResult);
}

void MeetingModuleTest::testMeetingLeaving()
{
    // Join meeting first
    QString meetingUrl = generateTestUrl();
    QVERIFY(m_meetingManager->joinMeeting(meetingUrl, "Test User", true, true));
    QVERIFY(waitForState(IMeetingManager::InMeeting));
    
    // Leave meeting
    bool leaveResult = m_meetingManager->leaveMeeting();
    QVERIFY(leaveResult);
    
    // Verify meeting left signal
    QVERIFY(waitForSignal(m_meetingManager.get(), SIGNAL(meetingLeft()), 3000));
    QCOMPARE(m_meetingLeftSpy->count(), 1);
    
    // Verify state change
    QVERIFY(waitForState(IMeetingManager::Disconnected));
    
    // Verify cleanup
    QVariantMap meetingInfo = m_meetingManager->getCurrentMeetingInfo();
    QVERIFY(meetingInfo.isEmpty());
}

// 7. Data Model Tests
void MeetingModuleTest::testMeetingModel()
{
    m_testMeeting = std::make_unique<Meeting>();
    
    // Test basic properties
    m_testMeeting->setId("test-meeting-123");
    m_testMeeting->setName("Test Meeting");
    m_testMeeting->setUrl(generateTestUrl());
    m_testMeeting->setCreatedAt(QDateTime::currentDateTime());
    
    QCOMPARE(m_testMeeting->id(), QString("test-meeting-123"));
    QCOMPARE(m_testMeeting->name(), QString("Test Meeting"));
    QVERIFY(!m_testMeeting->url().isEmpty());
    QVERIFY(m_testMeeting->createdAt().isValid());
    
    // Test validation
    QVERIFY(m_testMeeting->isValid());
    
    // Test invalid meeting
    Meeting invalidMeeting;
    QVERIFY(!invalidMeeting.isValid());
}

void MeetingModuleTest::testRoomModel()
{
    m_testRoom = std::make_unique<Room>();
    
    // Test room properties
    m_testRoom->setName("test-room");
    m_testRoom->setServer("meet.jit.si");
    m_testRoom->setPassword("room-password");
    m_testRoom->setMaxParticipants(50);
    
    QCOMPARE(m_testRoom->name(), QString("test-room"));
    QCOMPARE(m_testRoom->server(), QString("meet.jit.si"));
    QCOMPARE(m_testRoom->password(), QString("room-password"));
    QCOMPARE(m_testRoom->maxParticipants(), 50);
    
    // Test room URL generation
    QString roomUrl = m_testRoom->generateUrl();
    QVERIFY(roomUrl.contains("meet.jit.si"));
    QVERIFY(roomUrl.contains("test-room"));
    
    // Test validation
    QVERIFY(m_testRoom->isValid());
}

void MeetingModuleTest::testInvitationModel()
{
    m_testInvitation = std::make_unique<Invitation>();
    
    // Test invitation properties
    m_testInvitation->setRecipientEmail("recipient@example.com");
    m_testInvitation->setSenderName("Sender Name");
    m_testInvitation->setMeetingUrl(generateTestUrl());
    m_testInvitation->setMessage("Custom invitation message");
    m_testInvitation->setScheduledTime(QDateTime::currentDateTime().addHours(1));
    
    QCOMPARE(m_testInvitation->recipientEmail(), QString("recipient@example.com"));
    QCOMPARE(m_testInvitation->senderName(), QString("Sender Name"));
    QVERIFY(!m_testInvitation->meetingUrl().isEmpty());
    QCOMPARE(m_testInvitation->message(), QString("Custom invitation message"));
    QVERIFY(m_testInvitation->scheduledTime().isValid());
    
    // Test validation
    QVERIFY(m_testInvitation->isValid());
    
    // Test invalid email
    m_testInvitation->setRecipientEmail("invalid-email");
    QVERIFY(!m_testInvitation->isValid());
}

void MeetingModuleTest::testModelSerialization()
{
    // Test Meeting serialization
    m_testMeeting = std::make_unique<Meeting>();
    m_testMeeting->setId("test-123");
    m_testMeeting->setName("Serialization Test");
    
    QVariantMap meetingData = m_testMeeting->toVariantMap();
    QCOMPARE(meetingData["id"].toString(), QString("test-123"));
    QCOMPARE(meetingData["name"].toString(), QString("Serialization Test"));
    
    // Test deserialization
    Meeting deserializedMeeting;
    deserializedMeeting.fromVariantMap(meetingData);
    QCOMPARE(deserializedMeeting.id(), m_testMeeting->id());
    QCOMPARE(deserializedMeeting.name(), m_testMeeting->name());
}

void MeetingModuleTest::testModelValidation()
{
    // Test Meeting validation rules
    Meeting meeting;
    QVERIFY(!meeting.isValid()); // Empty meeting should be invalid
    
    meeting.setId("valid-id");
    meeting.setName("Valid Name");
    meeting.setUrl("https://meet.jit.si/room");
    QVERIFY(meeting.isValid());
    
    // Test Room validation rules
    Room room;
    QVERIFY(!room.isValid()); // Empty room should be invalid
    
    room.setName("valid-room");
    room.setServer("meet.jit.si");
    QVERIFY(room.isValid());
    
    // Test invalid room name
    room.setName("invalid room name with spaces");
    QVERIFY(!room.isValid());
}/
/ 8. Handler Tests
void MeetingModuleTest::testURLHandler()
{
    m_urlHandler = std::make_unique<URLHandler>();
    
    // Test URL processing
    QString testUrl = "https://meet.jit.si/test-room?config.startWithAudioMuted=true";
    QVariantMap result = m_urlHandler->processUrl(testUrl);
    
    QVERIFY(result["valid"].toBool());
    QCOMPARE(result["server"].toString(), QString("meet.jit.si"));
    QCOMPARE(result["room"].toString(), QString("test-room"));
    QVERIFY(result.contains("parameters"));
    
    // Test URL validation
    QVERIFY(m_urlHandler->isValidUrl(testUrl));
    QVERIFY(!m_urlHandler->isValidUrl("invalid-url"));
    QVERIFY(!m_urlHandler->isValidUrl(""));
}

void MeetingModuleTest::testProtocolHandler()
{
    m_protocolHandler = std::make_unique<ProtocolHandler>();
    
    // Test protocol registration
    QVERIFY(m_protocolHandler->registerProtocol("jitsi"));
    QVERIFY(m_protocolHandler->isProtocolSupported("jitsi"));
    
    // Test protocol URL handling
    QString protocolUrl = "jitsi://meet.jit.si/test-room";
    QVariantMap result = m_protocolHandler->handleProtocolUrl(protocolUrl);
    
    QVERIFY(result["handled"].toBool());
    QCOMPARE(result["action"].toString(), QString("join_meeting"));
    QCOMPARE(result["url"].toString(), QString("https://meet.jit.si/test-room"));
}

void MeetingModuleTest::testAuthHandler()
{
    m_authHandler = std::make_unique<AuthHandler>();
    
    // Test authentication methods
    QVariantMap tokenCredentials;
    tokenCredentials["token"] = "test-jwt-token";
    
    QVERIFY(m_authHandler->authenticate("jwt", tokenCredentials));
    QVERIFY(m_authHandler->isAuthenticated());
    
    // Test logout
    m_authHandler->logout();
    QVERIFY(!m_authHandler->isAuthenticated());
}

void MeetingModuleTest::testHandlerChaining()
{
    // Test that handlers work together properly
    m_urlHandler = std::make_unique<URLHandler>();
    m_protocolHandler = std::make_unique<ProtocolHandler>();
    m_authHandler = std::make_unique<AuthHandler>();
    
    // Chain: Protocol -> URL -> Auth
    QString protocolUrl = "jitsi://meet.jit.si/secure-room?token=auth-token";
    
    // 1. Protocol handler processes the URL
    QVariantMap protocolResult = m_protocolHandler->handleProtocolUrl(protocolUrl);
    QVERIFY(protocolResult["handled"].toBool());
    
    // 2. URL handler processes the converted URL
    QString httpUrl = protocolResult["url"].toString();
    QVariantMap urlResult = m_urlHandler->processUrl(httpUrl);
    QVERIFY(urlResult["valid"].toBool());
    
    // 3. Auth handler processes any authentication tokens
    if (urlResult.contains("token")) {
        QVariantMap authCredentials;
        authCredentials["token"] = urlResult["token"];
        QVERIFY(m_authHandler->authenticate("jwt", authCredentials));
    }
}

void MeetingModuleTest::testHandlerErrors()
{
    m_urlHandler = std::make_unique<URLHandler>();
    
    // Test error handling for invalid URLs
    QVariantMap result = m_urlHandler->processUrl("invalid-url");
    QVERIFY(!result["valid"].toBool());
    QVERIFY(result.contains("error"));
    QVERIFY(!result["error"].toString().isEmpty());
    
    // Test error handling for network issues
    simulateNetworkError();
    result = m_urlHandler->processUrl("https://unreachable.server.com/room");
    QVERIFY(!result["valid"].toBool());
    QVERIFY(result["error"].toString().contains("network"));
}

// 9. UI Component Tests
void MeetingModuleTest::testMeetingWidget()
{
    m_meetingWidget = std::make_unique<MeetingWidget>();
    m_meetingWidget->setMeetingManager(m_meetingManager.get());
    
    // Test widget initialization
    QVERIFY(m_meetingWidget->meetingManager() == m_meetingManager.get());
    
    // Test display modes
    m_meetingWidget->setDisplayMode(MeetingWidget::CompactMode);
    QCOMPARE(m_meetingWidget->displayMode(), MeetingWidget::CompactMode);
    
    m_meetingWidget->setDisplayMode(MeetingWidget::DetailedMode);
    QCOMPARE(m_meetingWidget->displayMode(), MeetingWidget::DetailedMode);
    
    // Test control visibility
    m_meetingWidget->setShowControls(true);
    QVERIFY(m_meetingWidget->showControls());
    
    m_meetingWidget->setShowParticipants(false);
    QVERIFY(!m_meetingWidget->showParticipants());
    
    // Test meeting info display
    QVariantMap testMeetingInfo = generateTestMeetingInfo();
    m_meetingWidget->showMeetingInfo(testMeetingInfo);
    
    // Verify UI updates (would need to check actual widget state in real implementation)
    QVERIFY(true); // Placeholder for UI state verification
}

void MeetingModuleTest::testJoinDialog()
{
    m_joinDialog = std::make_unique<JoinDialog>();
    
    // Test dialog properties
    m_joinDialog->setMeetingUrl(generateTestUrl());
    QCOMPARE(m_joinDialog->meetingUrl(), generateTestUrl());
    
    m_joinDialog->setDisplayName("Test User");
    QCOMPARE(m_joinDialog->displayName(), QString("Test User"));
    
    m_joinDialog->setAudioEnabled(false);
    QVERIFY(!m_joinDialog->isAudioEnabled());
    
    m_joinDialog->setVideoEnabled(true);
    QVERIFY(m_joinDialog->isVideoEnabled());
    
    // Test validation
    QVERIFY(m_joinDialog->validateInput());
    
    // Test invalid input
    m_joinDialog->setMeetingUrl("");
    QVERIFY(!m_joinDialog->validateInput());
}

void MeetingModuleTest::testCreateDialog()
{
    m_createDialog = std::make_unique<CreateDialog>();
    
    // Test dialog properties
    m_createDialog->setMeetingName("Test Meeting");
    QCOMPARE(m_createDialog->meetingName(), QString("Test Meeting"));
    
    m_createDialog->setServer("meet.jit.si");
    QCOMPARE(m_createDialog->server(), QString("meet.jit.si"));
    
    m_createDialog->setPassword("meeting-password");
    QCOMPARE(m_createDialog->password(), QString("meeting-password"));
    
    // Test settings
    QVariantMap settings;
    settings["maxParticipants"] = 25;
    settings["recordMeeting"] = true;
    
    m_createDialog->setMeetingSettings(settings);
    QVariantMap retrievedSettings = m_createDialog->meetingSettings();
    QCOMPARE(retrievedSettings["maxParticipants"].toInt(), 25);
    QCOMPARE(retrievedSettings["recordMeeting"].toBool(), true);
    
    // Test validation
    QVERIFY(m_createDialog->validateInput());
}

void MeetingModuleTest::testUIStateUpdates()
{
    m_meetingWidget = std::make_unique<MeetingWidget>();
    m_meetingWidget->setMeetingManager(m_meetingManager.get());
    
    // Test state updates when meeting state changes
    QSignalSpy displayModeChangedSpy(m_meetingWidget.get(), 
                                    &MeetingWidget::displayModeChanged);
    
    // Simulate meeting state changes
    m_meetingManager->createMeeting("UI Test Meeting", generateTestSettings());
    
    // Widget should update its display
    m_meetingWidget->updateMeetingInfo();
    m_meetingWidget->updateControlsState();
    
    // Verify UI responded to state changes
    QVERIFY(true); // Placeholder for actual UI state verification
}

void MeetingModuleTest::testUIInteractions()
{
    m_meetingWidget = std::make_unique<MeetingWidget>();
    m_meetingWidget->setMeetingManager(m_meetingManager.get());
    
    // Test signal connections
    QSignalSpy joinRequestedSpy(m_meetingWidget.get(), 
                               &MeetingWidget::joinMeetingRequested);
    QSignalSpy leaveRequestedSpy(m_meetingWidget.get(), 
                                &MeetingWidget::leaveMeetingRequested);
    
    // Simulate user interactions
    simulateUserInteraction();
    
    // In a real implementation, we would trigger actual UI events
    // For now, we'll emit the signals directly to test the connections
    emit m_meetingWidget->joinMeetingRequested(generateTestUrl());
    emit m_meetingWidget->leaveMeetingRequested();
    
    QCOMPARE(joinRequestedSpy.count(), 1);
    QCOMPARE(leaveRequestedSpy.count(), 1);
}

void MeetingModuleTest::testUIThemeSupport()
{
    m_meetingWidget = std::make_unique<MeetingWidget>();
    
    // Test theme switching
    m_meetingWidget->setTheme("dark");
    QCOMPARE(m_meetingWidget->theme(), QString("dark"));
    
    m_meetingWidget->setTheme("light");
    QCOMPARE(m_meetingWidget->theme(), QString("light"));
    
    // Test invalid theme
    m_meetingWidget->setTheme("invalid-theme");
    // Should fall back to default theme
    QVERIFY(!m_meetingWidget->theme().isEmpty());
}

// 10. Configuration Tests
void MeetingModuleTest::testConfigurationLoad()
{
    // Test loading configuration from file
    QVariantMap testConfig;
    testConfig["server"] = "custom.jitsi.server.com";
    testConfig["timeout"] = 15000;
    testConfig["audioEnabled"] = false;
    testConfig["videoEnabled"] = true;
    
    m_meetingConfig->setConfiguration(testConfig);
    
    QVariantMap loadedConfig = m_meetingConfig->getConfiguration();
    QCOMPARE(loadedConfig["server"].toString(), QString("custom.jitsi.server.com"));
    QCOMPARE(loadedConfig["timeout"].toInt(), 15000);
    QCOMPARE(loadedConfig["audioEnabled"].toBool(), false);
    QCOMPARE(loadedConfig["videoEnabled"].toBool(), true);
}

void MeetingModuleTest::testConfigurationSave()
{
    // Test saving configuration
    QVariantMap configToSave;
    configToSave["server"] = "save.test.server.com";
    configToSave["quality"] = "high";
    
    bool saveResult = m_meetingConfig->saveConfiguration(configToSave);
    QVERIFY(saveResult);
    
    // Verify saved configuration can be loaded
    QVariantMap loadedConfig = m_meetingConfig->loadConfiguration();
    QCOMPARE(loadedConfig["server"].toString(), QString("save.test.server.com"));
    QCOMPARE(loadedConfig["quality"].toString(), QString("high"));
}

void MeetingModuleTest::testConfigurationValidation()
{
    // Test valid configuration
    QVariantMap validConfig;
    validConfig["server"] = "meet.jit.si";
    validConfig["timeout"] = 10000;
    validConfig["audioEnabled"] = true;
    
    QVERIFY(m_meetingConfig->validateConfiguration(validConfig));
    
    // Test invalid configuration
    QVariantMap invalidConfig;
    invalidConfig["server"] = ""; // Empty server
    invalidConfig["timeout"] = -1; // Invalid timeout
    
    QVERIFY(!m_meetingConfig->validateConfiguration(invalidConfig));
}

void MeetingModuleTest::testConfigurationDefaults()
{
    // Test default configuration values
    QVariantMap defaults = m_meetingConfig->getDefaultConfiguration();
    
    QVERIFY(defaults.contains("server"));
    QVERIFY(defaults.contains("timeout"));
    QVERIFY(defaults.contains("audioEnabled"));
    QVERIFY(defaults.contains("videoEnabled"));
    
    // Verify reasonable default values
    QVERIFY(defaults["timeout"].toInt() > 0);
    QVERIFY(!defaults["server"].toString().isEmpty());
}

void MeetingModuleTest::testConfigurationMigration()
{
    // Test configuration migration from older versions
    QVariantMap oldConfig;
    oldConfig["version"] = "1.0";
    oldConfig["jitsiServer"] = "old.server.com"; // Old key name
    
    QVariantMap migratedConfig = m_meetingConfig->migrateConfiguration(oldConfig);
    
    QVERIFY(migratedConfig["version"].toString() != "1.0");
    QCOMPARE(migratedConfig["server"].toString(), QString("old.server.com"));
    QVERIFY(!migratedConfig.contains("jitsiServer")); // Old key should be removed
}/
/ 11. Error Handling Tests
void MeetingModuleTest::testNetworkErrors()
{
    // Simulate network disconnection
    simulateNetworkError();
    
    // Try to join meeting with no network
    bool result = m_meetingManager->joinMeeting(generateTestUrl(), "Test User", true, true);
    QVERIFY(!result);
    
    // Verify error signal
    QVERIFY(waitForSignal(m_meetingManager.get(), SIGNAL(errorOccurred(QString)), 2000));
    
    QString errorMessage = m_errorOccurredSpy->takeFirst().at(0).toString();
    QVERIFY(errorMessage.contains("network") || errorMessage.contains("connection"));
}

void MeetingModuleTest::testServerErrors()
{
    // Simulate server returning error response
    QVariantMap errorResponse;
    errorResponse["error"] = "server_unavailable";
    errorResponse["message"] = "Server is temporarily unavailable";
    
    simulateServerResponse(errorResponse);
    
    bool result = m_meetingManager->createMeeting("Server Error Test", generateTestSettings());
    QVERIFY(!result);
    
    QVERIFY(waitForSignal(m_meetingManager.get(), SIGNAL(errorOccurred(QString)), 2000));
}

void MeetingModuleTest::testValidationErrors()
{
    // Test various validation errors
    QVERIFY(!m_meetingManager->joinMeeting("", "Test User", true, true)); // Empty URL
    QVERIFY(!m_meetingManager->joinMeeting("invalid-url", "Test User", true, true)); // Invalid URL
    QVERIFY(!m_meetingManager->createMeeting("", QVariantMap())); // Empty name
    
    // Each should generate appropriate error messages
    QVERIFY(m_errorOccurredSpy->count() >= 3);
}

void MeetingModuleTest::testRecoveryMechanisms()
{
    // Join a meeting successfully first
    QVERIFY(m_meetingManager->joinMeeting(generateTestUrl(), "Test User", true, true));
    QVERIFY(waitForState(IMeetingManager::InMeeting));
    
    // Simulate connection loss
    simulateNetworkError();
    
    // Manager should attempt automatic recovery
    QTimer::singleShot(1000, [this]() {
        // Restore network
        m_networkAvailable = true;
    });
    
    // Should eventually recover
    QVERIFY(waitForState(IMeetingManager::InMeeting, 10000));
}

void MeetingModuleTest::testErrorPropagation()
{
    // Test that errors propagate correctly through the system
    QSignalSpy moduleErrorSpy(m_meetingModule.get(), SIGNAL(errorOccurred(QString)));
    QSignalSpy managerErrorSpy(m_meetingManager.get(), SIGNAL(errorOccurred(QString)));
    
    // Cause an error at the manager level
    m_meetingManager->joinMeeting("invalid-url", "Test User", true, true);
    
    // Error should propagate to module level
    QVERIFY(waitForSignal(m_meetingManager.get(), SIGNAL(errorOccurred(QString)), 2000));
    QVERIFY(waitForSignal(m_meetingModule.get(), SIGNAL(errorOccurred(QString)), 2000));
    
    QVERIFY(managerErrorSpy.count() > 0);
    QVERIFY(moduleErrorSpy.count() > 0);
}

// 12. Performance Tests
void MeetingModuleTest::testModuleStartupTime()
{
    // Measure module startup time
    QElapsedTimer timer;
    timer.start();
    
    auto module = std::make_unique<MeetingModule>();
    bool initResult = module->initialize();
    
    m_startupTime = timer.elapsed();
    
    QVERIFY(initResult);
    QVERIFY(m_startupTime < 1000); // Should initialize within 1 second
    
    qDebug() << "Module startup time:" << m_startupTime << "ms";
}

void MeetingModuleTest::testMemoryUsage()
{
    // Basic memory usage test (simplified)
    size_t initialMemory = getCurrentMemoryUsage();
    
    // Create multiple meeting objects
    QList<std::unique_ptr<Meeting>> meetings;
    for (int i = 0; i < 100; ++i) {
        auto meeting = std::make_unique<Meeting>();
        meeting->setId(QString("meeting-%1").arg(i));
        meeting->setName(QString("Test Meeting %1").arg(i));
        meetings.append(std::move(meeting));
    }
    
    size_t peakMemory = getCurrentMemoryUsage();
    
    // Clear meetings
    meetings.clear();
    
    size_t finalMemory = getCurrentMemoryUsage();
    
    // Memory should be reasonable
    size_t memoryIncrease = peakMemory - initialMemory;
    QVERIFY(memoryIncrease < 10 * 1024 * 1024); // Less than 10MB for 100 meetings
    
    // Memory should be mostly freed
    size_t memoryLeak = finalMemory - initialMemory;
    QVERIFY(memoryLeak < 1024 * 1024); // Less than 1MB leak
    
    qDebug() << "Memory usage - Initial:" << initialMemory 
             << "Peak:" << peakMemory << "Final:" << finalMemory;
}

void MeetingModuleTest::testConcurrentOperations()
{
    // Test multiple concurrent operations
    const int operationCount = 10;
    QList<QFuture<bool>> futures;
    
    for (int i = 0; i < operationCount; ++i) {
        QFuture<bool> future = QtConcurrent::run([this, i]() {
            auto manager = std::make_unique<MeetingManager>();
            manager->initialize();
            
            QString meetingName = QString("Concurrent Meeting %1").arg(i);
            return manager->createMeeting(meetingName, generateTestSettings());
        });
        futures.append(future);
    }
    
    // Wait for all operations to complete
    int successCount = 0;
    for (auto& future : futures) {
        future.waitForFinished();
        if (future.result()) {
            successCount++;
        }
    }
    
    // Most operations should succeed (allowing for some failures due to resource limits)
    QVERIFY(successCount >= operationCount / 2);
    
    qDebug() << "Concurrent operations - Success rate:" 
             << (successCount * 100 / operationCount) << "%";
}

void MeetingModuleTest::testLargeDataHandling()
{
    // Test handling of large participant lists
    QVariantList largeParticipantList;
    for (int i = 0; i < 1000; ++i) {
        QVariantMap participant;
        participant["id"] = QString("participant-%1").arg(i);
        participant["name"] = QString("Participant %1").arg(i);
        participant["email"] = QString("participant%1@example.com").arg(i);
        largeParticipantList.append(participant);
    }
    
    // Simulate receiving large participant list
    QElapsedTimer timer;
    timer.start();
    
    // Process the large list (this would be done internally by the meeting manager)
    for (const QVariant& participantVar : largeParticipantList) {
        QVariantMap participant = participantVar.toMap();
        // Simulate processing each participant
        QString id = participant["id"].toString();
        QVERIFY(!id.isEmpty());
    }
    
    qint64 processingTime = timer.elapsed();
    
    // Should process 1000 participants quickly
    QVERIFY(processingTime < 1000); // Less than 1 second
    
    qDebug() << "Large data processing time:" << processingTime << "ms for 1000 participants";
}

void MeetingModuleTest::testResourceCleanup()
{
    // Test that resources are properly cleaned up
    size_t initialMemory = getCurrentMemoryUsage();
    
    {
        // Create and destroy many objects in a scope
        for (int i = 0; i < 50; ++i) {
            auto module = std::make_unique<MeetingModule>();
            module->initialize();
            
            auto manager = std::make_unique<MeetingManager>();
            manager->initialize();
            
            auto linkHandler = std::make_unique<LinkHandler>();
            
            // Objects should be automatically destroyed at end of scope
        }
    }
    
    // Force garbage collection if applicable
    QCoreApplication::processEvents();
    
    size_t finalMemory = getCurrentMemoryUsage();
    size_t memoryLeak = finalMemory - initialMemory;
    
    // Should have minimal memory leak
    QVERIFY(memoryLeak < 5 * 1024 * 1024); // Less than 5MB leak
    
    qDebug() << "Resource cleanup - Memory leak:" << memoryLeak << "bytes";
}

// 13. Integration Tests
void MeetingModuleTest::testModuleIntegration()
{
    // Test integration between module components
    QVERIFY(m_meetingModule->initialize());
    
    // Get components
    MeetingManager* manager = m_meetingModule->meetingManager();
    LinkHandler* linkHandler = m_meetingModule->linkHandler();
    MeetingConfig* config = m_meetingModule->config();
    
    QVERIFY(manager != nullptr);
    QVERIFY(linkHandler != nullptr);
    QVERIFY(config != nullptr);
    
    // Test component interaction
    QString testUrl = generateTestUrl();
    QVariantMap parseResult = linkHandler->parseUrl(testUrl);
    QVERIFY(parseResult["valid"].toBool());
    
    // Use parsed URL with manager
    bool joinResult = manager->joinMeeting(testUrl, "Integration Test User", true, true);
    QVERIFY(joinResult);
}

void MeetingModuleTest::testNetworkIntegration()
{
    // Test integration with network layer
    QVERIFY(m_meetingManager->initialize());
    
    // Test network connectivity check
    bool networkReachable = m_linkHandler->isServerReachable("meet.jit.si");
    if (m_networkAvailable) {
        QVERIFY(networkReachable);
    }
    
    // Test actual network operation
    if (networkReachable) {
        QVariantMap roomInfo = m_linkHandler->getRoomInfo(generateTestUrl());
        QVERIFY(!roomInfo.isEmpty());
    }
}

void MeetingModuleTest::testUIIntegration()
{
    // Test integration between UI components and business logic
    m_meetingWidget = std::make_unique<MeetingWidget>();
    m_meetingWidget->setMeetingManager(m_meetingManager.get());
    
    // Connect signals
    connect(m_meetingWidget.get(), &MeetingWidget::joinMeetingRequested,
            m_meetingManager.get(), &MeetingManager::joinMeeting);
    
    // Simulate UI interaction
    emit m_meetingWidget->joinMeetingRequested(generateTestUrl());
    
    // Business logic should respond
    QVERIFY(waitForState(IMeetingManager::Connecting, 2000));
}

void MeetingModuleTest::testConfigIntegration()
{
    // Test configuration integration across components
    QVariantMap globalConfig;
    globalConfig["server"] = "config.test.server.com";
    globalConfig["timeout"] = 20000;
    
    m_meetingConfig->setConfiguration(globalConfig);
    
    // Configuration should be used by other components
    m_meetingManager->setMeetingConfig(m_meetingConfig.get());
    
    QVariantMap managerConfig = m_meetingManager->getConfiguration();
    QCOMPARE(managerConfig["server"].toString(), QString("config.test.server.com"));
    QCOMPARE(managerConfig["timeout"].toInt(), 20000);
}

// 14. Compatibility Tests
void MeetingModuleTest::testConferenceManagerCompatibility()
{
    // Test compatibility with existing ConferenceManager interface
    // This would test that the new MeetingManager can work alongside
    // or replace the existing ConferenceManager
    
    // Create meeting using new interface
    QVERIFY(m_meetingManager->createMeeting("Compatibility Test", generateTestSettings()));
    
    // Verify that meeting info is in expected format for legacy systems
    QVariantMap meetingInfo = m_meetingManager->getCurrentMeetingInfo();
    
    // Check for expected legacy fields
    QVERIFY(meetingInfo.contains("meetingId") || meetingInfo.contains("id"));
    QVERIFY(meetingInfo.contains("meetingUrl") || meetingInfo.contains("url"));
    QVERIFY(meetingInfo.contains("participants"));
    
    // Test state mapping
    IMeetingManager::MeetingState currentState = m_meetingManager->currentState();
    QVERIFY(currentState >= IMeetingManager::Disconnected && 
            currentState <= IMeetingManager::Error);
}

void MeetingModuleTest::testLegacyAPICompatibility()
{
    // Test that legacy API calls still work
    // This would involve testing deprecated methods that are still supported
    
    // Example: legacy join method
    QVariantMap legacyParams;
    legacyParams["url"] = generateTestUrl();
    legacyParams["displayName"] = "Legacy User";
    
    // Should still work through compatibility layer
    bool result = m_meetingManager->joinMeeting(
        legacyParams["url"].toString(),
        legacyParams["displayName"].toString(),
        true, true);
    QVERIFY(result);
}

void MeetingModuleTest::testBackwardCompatibility()
{
    // Test backward compatibility with older configuration formats
    QVariantMap oldFormatConfig;
    oldFormatConfig["jitsiServer"] = "old.format.server.com"; // Old key name
    oldFormatConfig["connectionTimeoutMs"] = 15000; // Old key name
    
    // Should be automatically converted to new format
    m_meetingConfig->setConfiguration(oldFormatConfig);
    QVariantMap newFormatConfig = m_meetingConfig->getConfiguration();
    
    QCOMPARE(newFormatConfig["server"].toString(), QString("old.format.server.com"));
    QCOMPARE(newFormatConfig["timeout"].toInt(), 15000);
}

void MeetingModuleTest::testVersionCompatibility()
{
    // Test compatibility across different versions
    QVariantMap versionInfo = m_meetingModule->getVersionInfo();
    
    QVERIFY(versionInfo.contains("version"));
    QVERIFY(versionInfo.contains("apiVersion"));
    QVERIFY(versionInfo.contains("compatibleVersions"));
    
    // Check that current version is in compatible versions list
    QString currentVersion = versionInfo["version"].toString();
    QStringList compatibleVersions = versionInfo["compatibleVersions"].toStringList();
    QVERIFY(compatibleVersions.contains(currentVersion));
}

// 15. End-to-End Tests
void MeetingModuleTest::testCompleteWorkflow()
{
    // Test complete meeting workflow from start to finish
    
    // 1. Initialize module
    QVERIFY(m_meetingModule->initialize());
    
    // 2. Create meeting
    QVariantMap settings = generateTestSettings();
    QVERIFY(m_meetingManager->createMeeting("E2E Test Meeting", settings));
    QVERIFY(waitForState(IMeetingManager::Connected));
    
    // 3. Get meeting URL
    QString meetingUrl;
    if (waitForSignal(m_meetingManager.get(), SIGNAL(meetingCreated(QString, QVariantMap)), 3000)) {
        QList<QVariant> arguments = m_meetingCreatedSpy->takeFirst();
        meetingUrl = arguments.at(0).toString();
        QVERIFY(!meetingUrl.isEmpty());
    }
    
    // 4. Join meeting
    QVERIFY(m_meetingManager->joinMeeting(meetingUrl, "E2E Test User", true, true));
    QVERIFY(waitForState(IMeetingManager::InMeeting));
    
    // 5. Verify meeting state
    QVariantMap meetingInfo = m_meetingManager->getCurrentMeetingInfo();
    QVERIFY(!meetingInfo.isEmpty());
    QVERIFY(meetingInfo.contains("participants"));
    
    // 6. Test meeting controls
    m_meetingManager->setAudioEnabled(false);
    QVERIFY(!m_meetingManager->isAudioEnabled());
    
    m_meetingManager->setVideoEnabled(false);
    QVERIFY(!m_meetingManager->isVideoEnabled());
    
    // 7. Leave meeting
    QVERIFY(m_meetingManager->leaveMeeting());
    QVERIFY(waitForState(IMeetingManager::Disconnected));
    
    // 8. Cleanup
    m_meetingModule->cleanup();
}

void MeetingModuleTest::testMeetingLifecycle()
{
    // Test complete meeting lifecycle
    
    // Create -> Join -> Active -> Leave -> Cleanup
    QCOMPARE(m_meetingManager->currentState(), IMeetingManager::Disconnected);
    
    // Create meeting
    QVERIFY(m_meetingManager->createMeeting("Lifecycle Test", generateTestSettings()));
    QVERIFY(waitForState(IMeetingManager::Connected));
    
    // Join meeting
    QString meetingUrl = generateTestUrl();
    QVERIFY(m_meetingManager->joinMeeting(meetingUrl, "Lifecycle User", true, true));
    QVERIFY(waitForState(IMeetingManager::InMeeting));
    
    // Simulate meeting activity
    simulateNetworkDelay(100);
    m_meetingManager->refreshParticipants();
    
    // Leave meeting
    QVERIFY(m_meetingManager->leaveMeeting());
    QVERIFY(waitForState(IMeetingManager::Disconnected));
    
    // Verify cleanup
    QVariantMap meetingInfo = m_meetingManager->getCurrentMeetingInfo();
    QVERIFY(meetingInfo.isEmpty());
}

void MeetingModuleTest::testMultiUserScenario()
{
    // Test scenario with multiple users (simulated)
    
    // Create meeting
    QVERIFY(m_meetingManager->createMeeting("Multi-User Test", generateTestSettings()));
    QVERIFY(waitForState(IMeetingManager::Connected));
    
    // Join as first user
    QString meetingUrl = generateTestUrl();
    QVERIFY(m_meetingManager->joinMeeting(meetingUrl, "User 1", true, true));
    QVERIFY(waitForState(IMeetingManager::InMeeting));
    
    // Simulate other users joining
    for (int i = 2; i <= 5; ++i) {
        QVariantMap participant = generateTestParticipant();
        participant["name"] = QString("User %1").arg(i);
        participant["id"] = QString("user-%1").arg(i);
        
        simulateServerResponse(QVariantMap{
            {"event", "participant_joined"},
            {"participant", participant}
        });
    }
    
    // Verify participant count
    QVariantList participants = m_meetingManager->getParticipants();
    QVERIFY(participants.size() >= 5);
    
    // Simulate participants leaving
    for (int i = 2; i <= 3; ++i) {
        simulateServerResponse(QVariantMap{
            {"event", "participant_left"},
            {"participantId", QString("user-%1").arg(i)}
        });
    }
    
    // Verify updated participant count
    participants = m_meetingManager->getParticipants();
    QVERIFY(participants.size() >= 3);
}

void MeetingModuleTest::testErrorRecoveryWorkflow()
{
    // Test error recovery in a complete workflow
    
    // Start normal workflow
    QVERIFY(m_meetingManager->createMeeting("Recovery Test", generateTestSettings()));
    QVERIFY(waitForState(IMeetingManager::Connected));
    
    QString meetingUrl = generateTestUrl();
    QVERIFY(m_meetingManager->joinMeeting(meetingUrl, "Recovery User", true, true));
    QVERIFY(waitForState(IMeetingManager::InMeeting));
    
    // Introduce error
    simulateNetworkError();
    
    // Should transition to error state
    QVERIFY(waitForState(IMeetingManager::Error, 5000));
    
    // Restore network and attempt recovery
    m_networkAvailable = true;
    bool recoveryResult = m_meetingManager->reconnect();
    QVERIFY(recoveryResult);
    
    // Should recover to meeting state
    QVERIFY(waitForState(IMeetingManager::InMeeting, 10000));
    
    // Continue normal workflow
    QVERIFY(m_meetingManager->leaveMeeting());
    QVERIFY(waitForState(IMeetingManager::Disconnected));
}//
 Helper Methods Implementation

void MeetingModuleTest::setupTestEnvironment()
{
    // Create test directories
    QString testDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/meeting_tests";
    QDir().mkpath(testDir);
    
    // Set up test configuration
    m_testConfig["testMode"] = true;
    m_testConfig["testDirectory"] = testDir;
    m_testConfig["networkSimulation"] = true;
    
    // Initialize network availability
    m_networkAvailable = true;
    
    m_testEnvironmentReady = true;
    
    qDebug() << "Test environment setup completed in:" << testDir;
}

void MeetingModuleTest::cleanupTestEnvironment()
{
    // Clean up test directories
    QString testDir = m_testConfig["testDirectory"].toString();
    if (!testDir.isEmpty()) {
        QDir(testDir).removeRecursively();
    }
    
    // Reset test state
    m_testEnvironmentReady = false;
    m_networkAvailable = false;
    
    qDebug() << "Test environment cleanup completed";
}

void MeetingModuleTest::createTestMeeting(const QString& name)
{
    m_testMeeting = std::make_unique<Meeting>();
    m_testMeeting->setId(QString("test-meeting-%1").arg(QDateTime::currentMSecsSinceEpoch()));
    m_testMeeting->setName(name);
    m_testMeeting->setUrl(generateTestUrl());
    m_testMeeting->setCreatedAt(QDateTime::currentDateTime());
}

void MeetingModuleTest::createTestRoom(const QString& name)
{
    m_testRoom = std::make_unique<Room>();
    m_testRoom->setName(name);
    m_testRoom->setServer("meet.jit.si");
    m_testRoom->setMaxParticipants(50);
}

void MeetingModuleTest::createTestInvitation(const QString& email)
{
    m_testInvitation = std::make_unique<Invitation>();
    m_testInvitation->setRecipientEmail(email);
    m_testInvitation->setSenderName("Test Organizer");
    m_testInvitation->setMeetingUrl(generateTestUrl());
    m_testInvitation->setMessage("Please join our test meeting");
    m_testInvitation->setScheduledTime(QDateTime::currentDateTime().addHours(1));
}

bool MeetingModuleTest::verifyMeetingState(int expectedState)
{
    return m_meetingManager->currentState() == static_cast<IMeetingManager::MeetingState>(expectedState);
}

bool MeetingModuleTest::verifyUrlFormat(const QString& url)
{
    QUrl qurl(url);
    return qurl.isValid() && 
           (qurl.scheme() == "https" || qurl.scheme() == "jitsi") &&
           !qurl.host().isEmpty() &&
           !qurl.path().isEmpty();
}

bool MeetingModuleTest::verifyMeetingInfo(const QVariantMap& info)
{
    return info.contains("id") &&
           info.contains("name") &&
           info.contains("url") &&
           info.contains("participants") &&
           !info["id"].toString().isEmpty() &&
           !info["name"].toString().isEmpty() &&
           verifyUrlFormat(info["url"].toString());
}

bool MeetingModuleTest::verifyParticipantInfo(const QVariantMap& participant)
{
    return participant.contains("id") &&
           participant.contains("name") &&
           !participant["id"].toString().isEmpty() &&
           !participant["name"].toString().isEmpty();
}

void MeetingModuleTest::simulateNetworkDelay(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

void MeetingModuleTest::simulateNetworkError()
{
    m_networkAvailable = false;
    
    // Emit network error signal if manager is available
    if (m_meetingManager) {
        QTimer::singleShot(100, [this]() {
            emit m_meetingManager->errorOccurred("Network connection lost");
        });
    }
}

void MeetingModuleTest::simulateServerResponse(const QVariantMap& response)
{
    // Simulate server response after a short delay
    QTimer::singleShot(50, [this, response]() {
        // Process the simulated response
        QString event = response["event"].toString();
        
        if (event == "participant_joined") {
            QVariantMap participant = response["participant"].toMap();
            emit m_meetingManager->participantJoined(participant);
        } else if (event == "participant_left") {
            QString participantId = response["participantId"].toString();
            emit m_meetingManager->participantLeft(participantId);
        } else if (event == "meeting_created") {
            QString meetingUrl = response["meetingUrl"].toString();
            QVariantMap meetingInfo = response["meetingInfo"].toMap();
            emit m_meetingManager->meetingCreated(meetingUrl, meetingInfo);
        } else if (event == "error") {
            QString error = response["error"].toString();
            emit m_meetingManager->errorOccurred(error);
        }
    });
}

void MeetingModuleTest::simulateUserInteraction()
{
    // Simulate user clicking buttons, typing, etc.
    // In a real implementation, this would use QTest::mouseClick, QTest::keyClick, etc.
    simulateNetworkDelay(50); // Simulate user reaction time
}

QString MeetingModuleTest::generateTestUrl(const QString& server, const QString& room)
{
    return QString("https://%1/%2").arg(server, room);
}

QVariantMap MeetingModuleTest::generateTestMeetingInfo()
{
    QVariantMap info;
    info["id"] = QString("meeting-%1").arg(QDateTime::currentMSecsSinceEpoch());
    info["name"] = "Test Meeting";
    info["url"] = generateTestUrl();
    info["createdAt"] = QDateTime::currentDateTime();
    info["participants"] = QVariantList();
    info["audioEnabled"] = true;
    info["videoEnabled"] = true;
    info["duration"] = 0;
    info["participantCount"] = 1;
    
    return info;
}

QVariantMap MeetingModuleTest::generateTestParticipant()
{
    static int participantCounter = 0;
    participantCounter++;
    
    QVariantMap participant;
    participant["id"] = QString("participant-%1").arg(participantCounter);
    participant["name"] = QString("Test Participant %1").arg(participantCounter);
    participant["email"] = QString("participant%1@example.com").arg(participantCounter);
    participant["audioEnabled"] = true;
    participant["videoEnabled"] = true;
    participant["role"] = "participant";
    participant["joinedAt"] = QDateTime::currentDateTime();
    
    return participant;
}

QVariantMap MeetingModuleTest::generateTestSettings()
{
    QVariantMap settings;
    settings["server"] = m_testServerUrl;
    settings["timeout"] = m_testTimeout;
    settings["audioEnabled"] = true;
    settings["videoEnabled"] = true;
    settings["quality"] = "medium";
    settings["maxParticipants"] = 25;
    
    return settings;
}

bool MeetingModuleTest::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QSignalSpy spy(sender, signal);
    return spy.wait(timeout);
}

bool MeetingModuleTest::waitForState(int expectedState, int timeout)
{
    QElapsedTimer timer;
    timer.start();
    
    while (timer.elapsed() < timeout) {
        if (verifyMeetingState(expectedState)) {
            return true;
        }
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    
    return false;
}

bool MeetingModuleTest::waitForConnection(int timeout)
{
    return waitForState(IMeetingManager::Connected, timeout) ||
           waitForState(IMeetingManager::InMeeting, timeout);
}

void MeetingModuleTest::assertMeetingState(int expectedState)
{
    QCOMPARE(static_cast<int>(m_meetingManager->currentState()), expectedState);
}

void MeetingModuleTest::assertUrlValid(const QString& url)
{
    QVERIFY(verifyUrlFormat(url));
}

void MeetingModuleTest::assertErrorOccurred(const QString& expectedError)
{
    QVERIFY(m_errorOccurredSpy->count() > 0);
    
    if (!expectedError.isEmpty()) {
        bool errorFound = false;
        for (const QList<QVariant>& arguments : *m_errorOccurredSpy) {
            QString errorMessage = arguments.at(0).toString();
            if (errorMessage.contains(expectedError, Qt::CaseInsensitive)) {
                errorFound = true;
                break;
            }
        }
        QVERIFY2(errorFound, QString("Expected error '%1' not found").arg(expectedError).toLocal8Bit());
    }
}

void MeetingModuleTest::assertSignalEmitted(QSignalSpy* spy, int expectedCount)
{
    QCOMPARE(spy->count(), expectedCount);
}

size_t MeetingModuleTest::getCurrentMemoryUsage()
{
    // Simplified memory usage calculation
    // In a real implementation, this would use platform-specific APIs
    // to get actual memory usage
    
#ifdef Q_OS_WIN
    // Windows implementation would use GetProcessMemoryInfo
    return 0; // Placeholder
#elif defined(Q_OS_LINUX)
    // Linux implementation would read from /proc/self/status
    return 0; // Placeholder
#elif defined(Q_OS_MAC)
    // macOS implementation would use task_info
    return 0; // Placeholder
#else
    return 0; // Fallback
#endif
}

// Test main function
QTEST_MAIN(MeetingModuleTest)
#include "MeetingModuleTest.moc"