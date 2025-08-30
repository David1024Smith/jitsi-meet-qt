#ifndef CHATMODULETEST_H
#define CHATMODULETEST_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>

// Forward declarations
class ChatModule;
class ChatManager;
class MessageHandler;
class MessageStorage;
class HistoryManager;
class ChatMessage;
class ChatRoom;
class Participant;
class ChatWidget;
class MessageList;
class InputWidget;
class ChatConfig;

/**
 * @brief 聊天模块测试类
 * 
 * ChatModuleTest提供聊天模块的完整测试套件，包括：
 * - 消息发送和接收测试
 * - 消息存储和历史管理测试
 * - 聊天UI组件交互测试
 * - 与现有ChatManager的兼容性测试
 */
class ChatModuleTest : public QObject
{
    Q_OBJECT

public:
    explicit ChatModuleTest(QObject *parent = nullptr);
    ~ChatModuleTest();

private slots:
    // Test framework setup/teardown
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Core module tests
    void testModuleInitialization();
    void testModuleConfiguration();
    void testModuleStatus();

    // ChatManager tests
    void testChatManagerInitialization();
    void testConnectionManagement();
    void testRoomManagement();
    void testParticipantManagement();
    void testMessageHistoryManagement();
    void testChatManagerStatistics();

    // Message sending and receiving tests
    void testTextMessageSending();
    void testEmojiMessageSending();
    void testFileMessageSending();
    void testMessageReceiving();
    void testMessageValidation();
    void testMessagePriority();
    void testMessageRetry();
    void testBulkMessageSending();

    // MessageHandler tests
    void testMessageHandlerInitialization();
    void testIncomingMessageProcessing();
    void testOutgoingMessageProcessing();
    void testMessageFiltering();
    void testMessageTransformation();
    void testMessageQueue();
    void testMessageHandlerStatistics();

    // Message storage and history tests
    void testMessageStorageInitialization();
    void testMessagePersistence();
    void testMessageRetrieval();
    void testMessageSearch();
    void testMessageDeletion();
    void testMessageUpdate();
    void testStorageStatistics();
    void testStorageBackup();
    void testStorageRestore();

    // History management tests
    void testHistoryManagerInitialization();
    void testHistoryRetrieval();
    void testHistorySearch();
    void testHistoryCleanup();
    void testHistoryExport();
    void testHistoryImport();

    // Chat UI component tests
    void testChatWidgetInitialization();
    void testChatWidgetConfiguration();
    void testChatWidgetThemes();
    void testChatWidgetInteraction();
    void testMessageListDisplay();
    void testInputWidgetFunctionality();
    void testUIComponentSignals();
    void testUIComponentValidation();

    // Data model tests
    void testChatMessageModel();
    void testChatRoomModel();
    void testParticipantModel();
    void testModelSerialization();
    void testModelValidation();

    // Integration tests
    void testModuleIntegration();
    void testComponentCommunication();
    void testEndToEndWorkflow();
    void testConcurrentOperations();

    // Compatibility tests
    void testLegacyChatManagerCompatibility();
    void testExistingAPICompatibility();
    void testConfigurationMigration();
    void testDataMigration();

    // Performance tests
    void testMessageThroughput();
    void testStoragePerformance();
    void testUIPerformance();
    void testMemoryUsage();
    void testConcurrentUsers();

    // Error handling tests
    void testConnectionErrors();
    void testStorageErrors();
    void testMessageErrors();
    void testUIErrors();
    void testRecoveryMechanisms();

    // Security tests
    void testMessageValidation_Security();
    void testInputSanitization();
    void testFileUploadSecurity();
    void testDataEncryption();

    // Stress tests
    void testHighMessageVolume();
    void testLongRunningSession();
    void testResourceExhaustion();
    void testNetworkInstability();

private:
    // Test setup helpers
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    void setupTestData();
    void cleanupTestData();
    void createTestMessages();
    void createTestParticipants();
    void createTestRooms();

    // Mock and stub helpers
    void setupMockObjects();
    void cleanupMockObjects();
    ChatMessage* createTestMessage(const QString& content = "Test message",
                                   const QString& senderId = "test_user",
                                   const QString& roomId = "test_room");
    Participant* createTestParticipant(const QString& id = "test_participant",
                                       const QString& name = "Test User");
    ChatRoom* createTestRoom(const QString& id = "test_room",
                             const QString& name = "Test Room");

    // Verification helpers
    bool verifyMessageDelivery(const QString& messageId, int timeoutMs = 5000);
    bool verifyStoragePersistence(const QString& messageId);
    bool verifyUIUpdate(const QString& expectedContent);
    bool verifySignalEmission(QSignalSpy& spy, int expectedCount = 1, int timeoutMs = 5000);

    // Performance measurement helpers
    void measureMessageThroughput(int messageCount, int& messagesPerSecond);
    void measureStoragePerformance(int operationCount, qint64& avgTimeMs);
    void measureMemoryUsage(qint64& beforeMB, qint64& afterMB, qint64& peakMB);

    // Test data generators
    QStringList generateTestMessages(int count);
    QList<ChatMessage*> generateTestMessageObjects(int count);
    QVariantMap generateTestConfiguration();
    QByteArray generateTestFileData(int sizeKB = 100);

    // Utility methods
    void waitForSignal(QObject* sender, const char* signal, int timeoutMs = 5000);
    void simulateNetworkDelay(int delayMs = 100);
    void simulateUserInput(const QString& text);
    void simulateFileUpload(const QString& filePath);

private:
    // Test environment
    QApplication* m_app;
    QTemporaryDir* m_tempDir;
    QString m_testDataPath;

    // Test objects
    std::unique_ptr<ChatModule> m_chatModule;
    std::unique_ptr<ChatManager> m_chatManager;
    std::unique_ptr<MessageHandler> m_messageHandler;
    std::unique_ptr<MessageStorage> m_messageStorage;
    std::unique_ptr<HistoryManager> m_historyManager;
    std::unique_ptr<ChatWidget> m_chatWidget;
    std::unique_ptr<ChatConfig> m_chatConfig;

    // Test data
    QList<ChatMessage*> m_testMessages;
    QList<Participant*> m_testParticipants;
    QList<ChatRoom*> m_testRooms;
    QVariantMap m_testConfiguration;
    QStringList m_testMessageContents;

    // Mock objects
    QObject* m_mockNetworkManager;
    QObject* m_mockFileManager;
    QObject* m_mockCryptoHandler;

    // Performance tracking
    QElapsedTimer m_performanceTimer;
    QMap<QString, qint64> m_performanceMetrics;

    // Test flags
    bool m_skipUITests;
    bool m_skipPerformanceTests;
    bool m_skipStressTests;
    bool m_verboseOutput;
};

#endif // CHATMODULETEST_H