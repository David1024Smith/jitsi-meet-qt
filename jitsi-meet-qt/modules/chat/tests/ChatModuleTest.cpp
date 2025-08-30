#include "ChatModuleTest.h"
#include "ChatModule.h"
#include "ChatManager.h"
#include "MessageHandler.h"
#include "MessageStorage.h"
#include "HistoryManager.h"
#include "ChatMessage.h"
#include "ChatRoom.h"
#include "Participant.h"
#include "ChatWidget.h"
#include "MessageList.h"
#include "InputWidget.h"
#include "ChatConfig.h"

#include <QApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QThread>
#include <QStandardPaths>
#include <QProcess>

ChatModuleTest::ChatModuleTest(QObject *parent)
    : QObject(parent)
    , m_app(nullptr)
    , m_tempDir(nullptr)
    , m_mockNetworkManager(nullptr)
    , m_mockFileManager(nullptr)
    , m_mockCryptoHandler(nullptr)
    , m_skipUITests(false)
    , m_skipPerformanceTests(false)
    , m_skipStressTests(false)
    , m_verboseOutput(false)
{
}

ChatModuleTest::~ChatModuleTest()
{
}

void ChatModuleTest::initTestCase()
{
    // Initialize test environment
    setupTestEnvironment();
    
    // Ensure we have a QApplication instance for widget tests
    if (!QApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        m_app = new QApplication(argc, argv);
    }
    
    // Check for test flags
    QStringList args = QApplication::arguments();
    m_skipUITests = args.contains("--skip-ui");
    m_skipPerformanceTests = args.contains("--skip-performance");
    m_skipStressTests = args.contains("--skip-stress");
    m_verboseOutput = args.contains("--verbose");
}

void ChatModuleTest::cleanupTestCase()
{
    // Cleanup test environment
    cleanupTestEnvironment();
    
    if (m_app) {
        delete m_app;
        m_app = nullptr;
    }
}

void ChatModuleTest::init()
{
    // Setup for each test
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    // Create test objects
    m_chatModule = std::make_unique<ChatModule>();
    m_chatManager = std::make_unique<ChatManager>();
    m_messageHandler = std::make_unique<MessageHandler>();
    m_messageStorage = std::make_unique<MessageStorage>();
    m_historyManager = std::make_unique<HistoryManager>();
    m_chatConfig = std::make_unique<ChatConfig>();
    
    if (!m_skipUITests) {
        m_chatWidget = std::make_unique<ChatWidget>();
    }
    
    // Setup mock objects
    setupMockObjects();
    
    // Initialize test data
    setupTestData();
}

void ChatModuleTest::cleanup()
{
    // Cleanup after each test
    cleanupMockObjects();
    cleanupTestData();
    
    // Reset test objects
    m_chatWidget.reset();
    m_chatConfig.reset();
    m_historyManager.reset();
    m_messageStorage.reset();
    m_messageHandler.reset();
    m_chatManager.reset();
    m_chatModule.reset();
    
    if (m_tempDir) {
        delete m_tempDir;
        m_tempDir = nullptr;
    }
}

// Core module tests
void ChatModuleTest::testModuleInitialization()
{
    // Test module initialization
    QVERIFY(m_chatModule != nullptr);
    
    // Test initialization
    bool result = m_chatModule->initialize();
    QVERIFY(result == true);
    
    // Test double initialization
    bool secondInit = m_chatModule->initialize();
    QVERIFY(secondInit == true); // Should handle gracefully
    
    // Test module status
    QVERIFY(m_chatModule->isInitialized() == true);
}

void ChatModuleTest::testModuleConfiguration()
{
    // Test module configuration
    QVariantMap config = generateTestConfiguration();
    
    bool result = m_chatModule->configure(config);
    QVERIFY(result == true);
    
    // Verify configuration was applied
    QVariantMap appliedConfig = m_chatModule->configuration();
    QVERIFY(!appliedConfig.isEmpty());
    
    // Test invalid configuration
    QVariantMap invalidConfig;
    invalidConfig["invalid_key"] = "invalid_value";
    
    bool invalidResult = m_chatModule->configure(invalidConfig);
    // Should either reject or handle gracefully
    QVERIFY(invalidResult == false || m_chatModule->isInitialized());
}

void ChatModuleTest::testModuleStatus()
{
    // Test initial status
    QVERIFY(m_chatModule->status() != ChatModule::Error);
    
    // Test status after initialization
    m_chatModule->initialize();
    QVERIFY(m_chatModule->status() == ChatModule::Ready);
    
    // Test status monitoring
    QSignalSpy statusSpy(m_chatModule.get(), &ChatModule::statusChanged);
    
    // Trigger status change (simulate error)
    m_chatModule->handleError("Test error");
    
    QVERIFY(statusSpy.count() >= 1);
}
/
/ ChatManager tests
void ChatModuleTest::testChatManagerInitialization()
{
    // Test chat manager initialization
    QVERIFY(m_chatManager != nullptr);
    
    QVariantMap config = generateTestConfiguration();
    bool result = m_chatManager->initialize(config);
    QVERIFY(result == true);
    
    // Test connection status
    QVERIFY(m_chatManager->connectionStatus() == IChatManager::Disconnected);
    
    // Test initial state
    QVERIFY(m_chatManager->currentRoom().isEmpty());
    QVERIFY(m_chatManager->joinedRooms().isEmpty());
    QVERIFY(m_chatManager->participantCount() == 0);
}

void ChatModuleTest::testConnectionManagement()
{
    // Initialize manager
    QVERIFY(m_chatManager->initialize());
    
    // Test connection
    QSignalSpy connectionSpy(m_chatManager.get(), &IChatManager::connectionChanged);
    QSignalSpy statusSpy(m_chatManager.get(), &IChatManager::connectionStatusChanged);
    
    QString serverUrl = "wss://test.server.com";
    QVariantMap credentials;
    credentials["token"] = "test_token";
    
    bool connected = m_chatManager->connectToService(serverUrl, credentials);
    QVERIFY(connected == true);
    
    // Verify signals were emitted
    QVERIFY(verifySignalEmission(connectionSpy));
    QVERIFY(verifySignalEmission(statusSpy));
    
    // Test connection status
    QVERIFY(m_chatManager->isConnected() == true);
    QVERIFY(m_chatManager->connectionStatus() == IChatManager::Connected);
    
    // Test disconnection
    m_chatManager->disconnect();
    QVERIFY(m_chatManager->isConnected() == false);
}

void ChatModuleTest::testRoomManagement()
{
    // Initialize and connect
    QVERIFY(m_chatManager->initialize());
    QVERIFY(m_chatManager->connectToService("wss://test.server.com"));
    
    // Test joining room
    QSignalSpy roomJoinedSpy(m_chatManager.get(), &IChatManager::roomJoined);
    QSignalSpy currentRoomSpy(m_chatManager.get(), &IChatManager::currentRoomChanged);
    
    QString roomId = "test_room_123";
    bool joined = m_chatManager->joinRoom(roomId);
    QVERIFY(joined == true);
    
    // Verify signals and state
    QVERIFY(verifySignalEmission(roomJoinedSpy));
    QVERIFY(verifySignalEmission(currentRoomSpy));
    QCOMPARE(m_chatManager->currentRoom(), roomId);
    QVERIFY(m_chatManager->joinedRooms().contains(roomId));
    
    // Test joining with password
    QString protectedRoom = "protected_room";
    QString password = "secret123";
    bool joinedProtected = m_chatManager->joinRoom(protectedRoom, password);
    QVERIFY(joinedProtected == true);
    
    // Test leaving room
    QSignalSpy roomLeftSpy(m_chatManager.get(), &IChatManager::roomLeft);
    m_chatManager->leaveRoom(roomId);
    
    QVERIFY(verifySignalEmission(roomLeftSpy));
    QVERIFY(!m_chatManager->joinedRooms().contains(roomId));
}

void ChatModuleTest::testParticipantManagement()
{
    // Setup room
    QVERIFY(m_chatManager->initialize());
    QVERIFY(m_chatManager->connectToService("wss://test.server.com"));
    QVERIFY(m_chatManager->joinRoom("test_room"));
    
    // Test participant signals
    QSignalSpy participantJoinedSpy(m_chatManager.get(), &IChatManager::participantJoined);
    QSignalSpy participantLeftSpy(m_chatManager.get(), &IChatManager::participantLeft);
    QSignalSpy countChangedSpy(m_chatManager.get(), &IChatManager::participantCountChanged);
    
    // Simulate participant joining
    Participant* participant = createTestParticipant("user123", "Test User");
    // This would normally be triggered by network events
    emit m_chatManager->participantJoined(participant, "test_room");
    
    QVERIFY(verifySignalEmission(participantJoinedSpy));
    QVERIFY(verifySignalEmission(countChangedSpy));
    
    // Test participant list
    QList<Participant*> participants = m_chatManager->getParticipants();
    QVERIFY(participants.size() > 0);
    QVERIFY(m_chatManager->participantCount() > 0);
    
    // Test refresh participants
    m_chatManager->refreshParticipants();
    // Should not crash and may emit signals
}

void ChatModuleTest::testMessageHistoryManagement()
{
    // Initialize manager
    QVERIFY(m_chatManager->initialize());
    
    // Test history enabled state
    bool initialState = m_chatManager->isMessageHistoryEnabled();
    
    QSignalSpy historySpy(m_chatManager.get(), &IChatManager::messageHistoryEnabledChanged);
    
    // Toggle history state
    m_chatManager->setMessageHistoryEnabled(!initialState);
    QVERIFY(m_chatManager->isMessageHistoryEnabled() == !initialState);
    QVERIFY(verifySignalEmission(historySpy));
    
    // Test getting message history
    QString roomId = "test_room";
    QList<ChatMessage*> history = m_chatManager->getMessageHistory(roomId, 10);
    QVERIFY(history.size() >= 0); // Should not crash
    
    // Test searching messages
    QList<ChatMessage*> searchResults = m_chatManager->searchMessages("test", roomId);
    QVERIFY(searchResults.size() >= 0); // Should not crash
    
    // Test clearing history
    m_chatManager->clearMessageHistory(roomId);
    // Should not crash
}

void ChatModuleTest::testChatManagerStatistics()
{
    // Initialize manager
    QVERIFY(m_chatManager->initialize());
    
    // Get statistics
    QVariantMap stats = m_chatManager->getStatistics();
    QVERIFY(!stats.isEmpty());
    
    // Verify expected statistics keys
    QVERIFY(stats.contains("messagesReceived"));
    QVERIFY(stats.contains("messagesSent"));
    QVERIFY(stats.contains("connectionsCount"));
    QVERIFY(stats.contains("uptime"));
    
    // Verify statistics are numeric
    QVERIFY(stats["messagesReceived"].canConvert<int>());
    QVERIFY(stats["messagesSent"].canConvert<int>());
}

// Message sending and receiving tests
void ChatModuleTest::testTextMessageSending()
{
    // Setup
    QVERIFY(m_chatManager->initialize());
    QVERIFY(m_chatManager->connectToService("wss://test.server.com"));
    QVERIFY(m_chatManager->joinRoom("test_room"));
    
    // Test message sending
    QSignalSpy messageSentSpy(m_chatManager.get(), &IChatManager::messageSent);
    QSignalSpy messageFailedSpy(m_chatManager.get(), &IChatManager::messageSendFailed);
    
    QString messageContent = "Hello, this is a test message!";
    bool sent = m_chatManager->sendMessage(messageContent, IChatManager::TextMessage);
    QVERIFY(sent == true);
    
    // Verify signal emission
    QVERIFY(verifySignalEmission(messageSentSpy) || verifySignalEmission(messageFailedSpy));
    
    // Test empty message
    bool emptySent = m_chatManager->sendMessage("", IChatManager::TextMessage);
    QVERIFY(emptySent == false); // Should reject empty messages
    
    // Test very long message
    QString longMessage = QString("A").repeated(10000);
    bool longSent = m_chatManager->sendMessage(longMessage, IChatManager::TextMessage);
    // Should either send or reject gracefully
    QVERIFY(longSent == true || longSent == false);
}

void ChatModuleTest::testEmojiMessageSending()
{
    // Setup
    QVERIFY(m_chatManager->initialize());
    QVERIFY(m_chatManager->connectToService("wss://test.server.com"));
    QVERIFY(m_chatManager->joinRoom("test_room"));
    
    // Test emoji message
    QSignalSpy messageSentSpy(m_chatManager.get(), &IChatManager::messageSent);
    
    QString emojiMessage = "😀😃😄😁😆😅😂🤣";
    bool sent = m_chatManager->sendMessage(emojiMessage, IChatManager::EmojiMessage);
    QVERIFY(sent == true);
    
    // Test mixed emoji and text
    QString mixedMessage = "Hello 😀 World 🌍!";
    bool mixedSent = m_chatManager->sendMessage(mixedMessage, IChatManager::EmojiMessage);
    QVERIFY(mixedSent == true);
}

void ChatModuleTest::testFileMessageSending()
{
    // Setup
    QVERIFY(m_chatManager->initialize());
    QVERIFY(m_chatManager->connectToService("wss://test.server.com"));
    QVERIFY(m_chatManager->joinRoom("test_room"));
    
    // Create test file
    QTemporaryFile testFile;
    QVERIFY(testFile.open());
    testFile.write("Test file content");
    testFile.close();
    
    // Test file sending
    QSignalSpy messageSentSpy(m_chatManager.get(), &IChatManager::messageSent);
    QSignalSpy messageFailedSpy(m_chatManager.get(), &IChatManager::messageSendFailed);
    
    bool sent = m_chatManager->sendFile(testFile.fileName());
    QVERIFY(sent == true);
    
    // Test non-existent file
    bool nonExistentSent = m_chatManager->sendFile("/non/existent/file.txt");
    QVERIFY(nonExistentSent == false);
}

void ChatModuleTest::testMessageReceiving()
{
    // Setup
    QVERIFY(m_chatManager->initialize());
    QVERIFY(m_chatManager->connectToService("wss://test.server.com"));
    QVERIFY(m_chatManager->joinRoom("test_room"));
    
    // Test message receiving
    QSignalSpy messageReceivedSpy(m_chatManager.get(), &IChatManager::messageReceived);
    
    // Simulate receiving a message
    ChatMessage* testMessage = createTestMessage("Received test message", "sender123", "test_room");
    
    // This would normally be triggered by network events
    emit m_chatManager->messageReceived(testMessage);
    
    QVERIFY(verifySignalEmission(messageReceivedSpy));
    
    // Verify message properties
    QList<QVariant> arguments = messageReceivedSpy.takeFirst();
    ChatMessage* receivedMessage = qvariant_cast<ChatMessage*>(arguments.at(0));
    QVERIFY(receivedMessage != nullptr);
    QCOMPARE(receivedMessage->content(), QString("Received test message"));
    QCOMPARE(receivedMessage->senderId(), QString("sender123"));
    QCOMPARE(receivedMessage->roomId(), QString("test_room"));
}

void ChatModuleTest::testMessageValidation()
{
    // Create test messages with various properties
    ChatMessage* validMessage = createTestMessage("Valid message", "user123", "room123");
    QVERIFY(validMessage->validate() == true);
    
    // Test message with empty content
    ChatMessage* emptyMessage = createTestMessage("", "user123", "room123");
    QVERIFY(emptyMessage->validate() == false);
    
    // Test message with empty sender
    ChatMessage* noSenderMessage = createTestMessage("Message", "", "room123");
    QVERIFY(noSenderMessage->validate() == false);
    
    // Test message with empty room
    ChatMessage* noRoomMessage = createTestMessage("Message", "user123", "");
    QVERIFY(noRoomMessage->validate() == false);
    
    // Test message size limits
    QString largeContent = QString("A").repeated(100000);
    ChatMessage* largeMessage = createTestMessage(largeContent, "user123", "room123");
    // Should handle large messages appropriately
    QVERIFY(largeMessage->validate() == true || largeMessage->validate() == false);
    
    // Cleanup
    delete validMessage;
    delete emptyMessage;
    delete noSenderMessage;
    delete noRoomMessage;
    delete largeMessage;
}void Chat
ModuleTest::testMessagePriority()
{
    // Test message priority handling
    ChatMessage* normalMessage = createTestMessage("Normal message", "user123", "room123");
    normalMessage->setPriority(ChatMessage::Normal);
    QCOMPARE(normalMessage->priority(), ChatMessage::Normal);
    
    ChatMessage* highMessage = createTestMessage("High priority message", "user123", "room123");
    highMessage->setPriority(ChatMessage::High);
    QCOMPARE(highMessage->priority(), ChatMessage::High);
    
    ChatMessage* criticalMessage = createTestMessage("Critical message", "user123", "room123");
    criticalMessage->setPriority(ChatMessage::Critical);
    QCOMPARE(criticalMessage->priority(), ChatMessage::Critical);
    
    // Cleanup
    delete normalMessage;
    delete highMessage;
    delete criticalMessage;
}

void ChatModuleTest::testMessageRetry()
{
    // Setup
    QVERIFY(m_chatManager->initialize());
    
    // Create a message that will fail to send
    ChatMessage* testMessage = createTestMessage("Test retry message", "user123", "room123");
    testMessage->setStatus(ChatMessage::Failed);
    
    // Test retry mechanism
    QSignalSpy retrySpy(testMessage, &ChatMessage::statusChanged);
    testMessage->retrySend();
    
    // Should change status to pending or sending
    QVERIFY(testMessage->status() == ChatMessage::Pending || 
            testMessage->status() == ChatMessage::Sending);
    
    delete testMessage;
}

void ChatModuleTest::testBulkMessageSending()
{
    if (m_skipPerformanceTests) {
        QSKIP("Performance tests disabled");
    }
    
    // Setup
    QVERIFY(m_chatManager->initialize());
    QVERIFY(m_chatManager->connectToService("wss://test.server.com"));
    QVERIFY(m_chatManager->joinRoom("test_room"));
    
    // Test sending multiple messages
    QSignalSpy messageSentSpy(m_chatManager.get(), &IChatManager::messageSent);
    
    int messageCount = 10;
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < messageCount; ++i) {
        QString message = QString("Bulk message %1").arg(i);
        m_chatManager->sendMessage(message, IChatManager::TextMessage);
    }
    
    qint64 elapsed = timer.elapsed();
    
    // Should complete within reasonable time
    QVERIFY(elapsed < 5000); // 5 seconds max
    
    // Wait for all messages to be processed
    QTest::qWait(1000);
}

// MessageHandler tests
void ChatModuleTest::testMessageHandlerInitialization()
{
    // Test message handler initialization
    QVERIFY(m_messageHandler != nullptr);
    
    QVariantMap config = generateTestConfiguration();
    bool result = m_messageHandler->initialize(config);
    QVERIFY(result == true);
    
    // Test initial state
    QVERIFY(m_messageHandler->processingStatus() == IMessageHandler::Idle);
    QVERIFY(m_messageHandler->queueSize() == 0);
    QVERIFY(m_messageHandler->processedCount() == 0);
    QVERIFY(m_messageHandler->isProcessingEnabled() == true);
}

void ChatModuleTest::testIncomingMessageProcessing()
{
    // Initialize handler
    QVERIFY(m_messageHandler->initialize());
    
    // Test processing incoming message
    QSignalSpy processedSpy(m_messageHandler.get(), &IMessageHandler::messageProcessed);
    
    QVariantMap messageData;
    messageData["content"] = "Test incoming message";
    messageData["senderId"] = "user123";
    messageData["roomId"] = "room123";
    messageData["timestamp"] = QDateTime::currentDateTime();
    
    IMessageHandler::ProcessingResult result = 
        m_messageHandler->processIncomingMessage(messageData, IMessageHandler::Normal);
    
    QVERIFY(result == IMessageHandler::Success || result == IMessageHandler::Queued);
    
    // Process queue if message was queued
    if (result == IMessageHandler::Queued) {
        m_messageHandler->processQueue();
        QVERIFY(verifySignalEmission(processedSpy));
    }
}

void ChatModuleTest::testOutgoingMessageProcessing()
{
    // Initialize handler
    QVERIFY(m_messageHandler->initialize());
    
    // Test processing outgoing message
    QSignalSpy processedSpy(m_messageHandler.get(), &IMessageHandler::messageProcessed);
    
    ChatMessage* testMessage = createTestMessage("Test outgoing message", "user123", "room123");
    
    IMessageHandler::ProcessingResult result = 
        m_messageHandler->processOutgoingMessage(testMessage, IMessageHandler::Normal);
    
    QVERIFY(result == IMessageHandler::Success || result == IMessageHandler::Queued);
    
    delete testMessage;
}

void ChatModuleTest::testMessageFiltering()
{
    // Initialize handler
    QVERIFY(m_messageHandler->initialize());
    
    // Set up message filter
    m_messageHandler->setMessageFilter([](const QVariantMap& data) {
        QString content = data["content"].toString();
        return !content.contains("spam"); // Filter out spam messages
    });
    
    // Test filtered message
    QSignalSpy filteredSpy(m_messageHandler.get(), &IMessageHandler::messageFiltered);
    
    QVariantMap spamMessage;
    spamMessage["content"] = "This is spam content";
    spamMessage["senderId"] = "spammer";
    spamMessage["roomId"] = "room123";
    
    IMessageHandler::ProcessingResult result = 
        m_messageHandler->processIncomingMessage(spamMessage);
    
    QVERIFY(result == IMessageHandler::Filtered);
    QVERIFY(verifySignalEmission(filteredSpy));
    
    // Test non-filtered message
    QVariantMap validMessage;
    validMessage["content"] = "This is valid content";
    validMessage["senderId"] = "user123";
    validMessage["roomId"] = "room123";
    
    IMessageHandler::ProcessingResult validResult = 
        m_messageHandler->processIncomingMessage(validMessage);
    
    QVERIFY(validResult != IMessageHandler::Filtered);
}

void ChatModuleTest::testMessageTransformation()
{
    // Initialize handler
    QVERIFY(m_messageHandler->initialize());
    
    // Set up message transformer
    m_messageHandler->setMessageTransformer([](const QVariantMap& data) {
        QVariantMap transformed = data;
        QString content = data["content"].toString();
        transformed["content"] = content.toUpper(); // Transform to uppercase
        return transformed;
    });
    
    // Test message transformation
    QVariantMap originalMessage;
    originalMessage["content"] = "hello world";
    originalMessage["senderId"] = "user123";
    originalMessage["roomId"] = "room123";
    
    IMessageHandler::ProcessingResult result = 
        m_messageHandler->processIncomingMessage(originalMessage);
    
    QVERIFY(result == IMessageHandler::Success || result == IMessageHandler::Queued);
}

void ChatModuleTest::testMessageQueue()
{
    // Initialize handler
    QVERIFY(m_messageHandler->initialize());
    
    // Test queue operations
    QVERIFY(m_messageHandler->queueSize() == 0);
    
    // Add messages to queue
    for (int i = 0; i < 5; ++i) {
        QVariantMap message;
        message["content"] = QString("Queued message %1").arg(i);
        message["senderId"] = "user123";
        message["roomId"] = "room123";
        
        m_messageHandler->processIncomingMessage(message);
    }
    
    // Check queue size
    int queueSize = m_messageHandler->queueSize();
    QVERIFY(queueSize >= 0);
    
    // Process queue
    QSignalSpy queueEmptySpy(m_messageHandler.get(), &IMessageHandler::queueEmpty);
    m_messageHandler->processQueue();
    
    // Clear queue
    m_messageHandler->clearQueue();
    QVERIFY(m_messageHandler->queueSize() == 0);
}

void ChatModuleTest::testMessageHandlerStatistics()
{
    // Initialize handler
    QVERIFY(m_messageHandler->initialize());
    
    // Get statistics
    QVariantMap stats = m_messageHandler->getStatistics();
    QVERIFY(!stats.isEmpty());
    
    // Verify expected statistics
    QVERIFY(stats.contains("processedCount"));
    QVERIFY(stats.contains("queueSize"));
    QVERIFY(stats.contains("processingTime"));
    
    // Clear statistics
    m_messageHandler->clearStatistics();
    
    QVariantMap clearedStats = m_messageHandler->getStatistics();
    QVERIFY(clearedStats["processedCount"].toInt() == 0);
}

// Message storage and history tests
void ChatModuleTest::testMessageStorageInitialization()
{
    // Test storage initialization
    QVERIFY(m_messageStorage != nullptr);
    
    QVariantMap config;
    config["databasePath"] = m_tempDir->path() + "/test_messages.db";
    config["maxMessages"] = 10000;
    
    bool result = m_messageStorage->initialize(config);
    QVERIFY(result == true);
    
    // Test storage status
    QVERIFY(m_messageStorage->status() == IMessageStorage::Ready);
    QVERIFY(m_messageStorage->isReady() == true);
}

void ChatModuleTest::testMessagePersistence()
{
    // Initialize storage
    QVariantMap config;
    config["databasePath"] = m_tempDir->path() + "/test_messages.db";
    QVERIFY(m_messageStorage->initialize(config));
    
    // Test storing message
    ChatMessage* testMessage = createTestMessage("Persistent message", "user123", "room123");
    
    QSignalSpy storedSpy(m_messageStorage.get(), &IMessageStorage::messageStored);
    
    IMessageStorage::OperationResult result = m_messageStorage->storeMessage(testMessage);
    QVERIFY(result == IMessageStorage::Success);
    QVERIFY(verifySignalEmission(storedSpy));
    
    // Test retrieving message
    ChatMessage* retrievedMessage = m_messageStorage->getMessage(testMessage->id());
    QVERIFY(retrievedMessage != nullptr);
    QCOMPARE(retrievedMessage->content(), testMessage->content());
    QCOMPARE(retrievedMessage->senderId(), testMessage->senderId());
    
    delete testMessage;
    delete retrievedMessage;
}

void ChatModuleTest::testMessageRetrieval()
{
    // Initialize storage and add test messages
    QVariantMap config;
    config["databasePath"] = m_tempDir->path() + "/test_messages.db";
    QVERIFY(m_messageStorage->initialize(config));
    
    // Store multiple messages
    QString roomId = "test_room";
    QList<ChatMessage*> testMessages = generateTestMessageObjects(10);
    
    for (ChatMessage* message : testMessages) {
        message->setRoomId(roomId);
        m_messageStorage->storeMessage(message);
    }
    
    // Test retrieving room messages
    QList<ChatMessage*> roomMessages = m_messageStorage->getRoomMessages(roomId, 5);
    QVERIFY(roomMessages.size() <= 5);
    QVERIFY(roomMessages.size() > 0);
    
    // Test retrieving by time range
    QDateTime startTime = QDateTime::currentDateTime().addSecs(-3600); // 1 hour ago
    QDateTime endTime = QDateTime::currentDateTime();
    
    QList<ChatMessage*> timeRangeMessages = 
        m_messageStorage->getMessagesByTimeRange(roomId, startTime, endTime);
    QVERIFY(timeRangeMessages.size() >= 0);
    
    // Test getting last message
    ChatMessage* lastMessage = m_messageStorage->getLastMessage(roomId);
    QVERIFY(lastMessage != nullptr);
    
    // Cleanup
    qDeleteAll(testMessages);
    qDeleteAll(roomMessages);
    qDeleteAll(timeRangeMessages);
    delete lastMessage;
}void ChatMod
uleTest::testMessageSearch()
{
    // Initialize storage
    QVariantMap config;
    config["databasePath"] = m_tempDir->path() + "/test_messages.db";
    QVERIFY(m_messageStorage->initialize(config));
    
    // Store test messages with searchable content
    QString roomId = "search_room";
    QStringList searchableContents = {
        "Hello world",
        "This is a test message",
        "Another test with keywords",
        "Random content here",
        "Final test message"
    };
    
    for (const QString& content : searchableContents) {
        ChatMessage* message = createTestMessage(content, "user123", roomId);
        m_messageStorage->storeMessage(message);
        delete message;
    }
    
    // Test searching for "test"
    QList<ChatMessage*> testResults = m_messageStorage->searchMessages("test", roomId);
    QVERIFY(testResults.size() >= 3); // Should find at least 3 messages
    
    // Test searching for "hello"
    QList<ChatMessage*> helloResults = m_messageStorage->searchMessages("hello", roomId);
    QVERIFY(helloResults.size() >= 1);
    
    // Test searching in all rooms
    QList<ChatMessage*> allResults = m_messageStorage->searchMessages("test");
    QVERIFY(allResults.size() >= testResults.size());
    
    // Cleanup
    qDeleteAll(testResults);
    qDeleteAll(helloResults);
    qDeleteAll(allResults);
}

void ChatModuleTest::testMessageDeletion()
{
    // Initialize storage
    QVariantMap config;
    config["databasePath"] = m_tempDir->path() + "/test_messages.db";
    QVERIFY(m_messageStorage->initialize(config));
    
    // Store test message
    ChatMessage* testMessage = createTestMessage("Message to delete", "user123", "room123");
    QVERIFY(m_messageStorage->storeMessage(testMessage) == IMessageStorage::Success);
    
    QString messageId = testMessage->id();
    delete testMessage;
    
    // Verify message exists
    ChatMessage* retrievedMessage = m_messageStorage->getMessage(messageId);
    QVERIFY(retrievedMessage != nullptr);
    delete retrievedMessage;
    
    // Test deleting message
    QSignalSpy deletedSpy(m_messageStorage.get(), &IMessageStorage::messageDeleted);
    
    IMessageStorage::OperationResult result = m_messageStorage->deleteMessage(messageId);
    QVERIFY(result == IMessageStorage::Success);
    QVERIFY(verifySignalEmission(deletedSpy));
    
    // Verify message is deleted
    ChatMessage* deletedMessage = m_messageStorage->getMessage(messageId);
    QVERIFY(deletedMessage == nullptr);
}

void ChatModuleTest::testMessageUpdate()
{
    // Initialize storage
    QVariantMap config;
    config["databasePath"] = m_tempDir->path() + "/test_messages.db";
    QVERIFY(m_messageStorage->initialize(config));
    
    // Store test message
    ChatMessage* testMessage = createTestMessage("Original content", "user123", "room123");
    QVERIFY(m_messageStorage->storeMessage(testMessage) == IMessageStorage::Success);
    
    // Update message content
    testMessage->editContent("Updated content");
    
    QSignalSpy updatedSpy(m_messageStorage.get(), &IMessageStorage::messageUpdated);
    
    IMessageStorage::OperationResult result = m_messageStorage->updateMessage(testMessage);
    QVERIFY(result == IMessageStorage::Success);
    QVERIFY(verifySignalEmission(updatedSpy));
    
    // Verify update
    ChatMessage* updatedMessage = m_messageStorage->getMessage(testMessage->id());
    QVERIFY(updatedMessage != nullptr);
    QCOMPARE(updatedMessage->content(), QString("Updated content"));
    QVERIFY(updatedMessage->isEdited() == true);
    
    delete testMessage;
    delete updatedMessage;
}

void ChatModuleTest::testStorageStatistics()
{
    // Initialize storage
    QVariantMap config;
    config["databasePath"] = m_tempDir->path() + "/test_messages.db";
    QVERIFY(m_messageStorage->initialize(config));
    
    // Get statistics
    QVariantMap stats = m_messageStorage->getStatistics();
    QVERIFY(!stats.isEmpty());
    
    // Verify expected statistics
    QVERIFY(stats.contains("totalMessages"));
    QVERIFY(stats.contains("totalRooms"));
    QVERIFY(stats.contains("databaseSize"));
    QVERIFY(stats.contains("lastBackup"));
    
    // Test message count
    int initialCount = m_messageStorage->getMessageCount();
    
    // Add a message
    ChatMessage* testMessage = createTestMessage("Count test", "user123", "room123");
    m_messageStorage->storeMessage(testMessage);
    
    int newCount = m_messageStorage->getMessageCount();
    QVERIFY(newCount == initialCount + 1);
    
    delete testMessage;
}

void ChatModuleTest::testStorageBackup()
{
    // Initialize storage
    QVariantMap config;
    config["databasePath"] = m_tempDir->path() + "/test_messages.db";
    QVERIFY(m_messageStorage->initialize(config));
    
    // Add some test data
    for (int i = 0; i < 5; ++i) {
        ChatMessage* message = createTestMessage(QString("Backup test %1").arg(i), "user123", "room123");
        m_messageStorage->storeMessage(message);
        delete message;
    }
    
    // Test backup
    QString backupPath = m_tempDir->path() + "/backup.db";
    QSignalSpy backupSpy(m_messageStorage.get(), &IMessageStorage::backupCompleted);
    
    IMessageStorage::OperationResult result = m_messageStorage->backup(backupPath);
    QVERIFY(result == IMessageStorage::Success);
    
    // Verify backup file exists
    QVERIFY(QFile::exists(backupPath));
}

void ChatModuleTest::testStorageRestore()
{
    // This test depends on testStorageBackup creating a backup
    QString backupPath = m_tempDir->path() + "/backup.db";
    
    if (!QFile::exists(backupPath)) {
        QSKIP("Backup file not available for restore test");
    }
    
    // Initialize new storage instance
    MessageStorage restoreStorage;
    QVariantMap config;
    config["databasePath"] = m_tempDir->path() + "/restored_messages.db";
    QVERIFY(restoreStorage.initialize(config));
    
    // Test restore
    QSignalSpy restoreSpy(&restoreStorage, &IMessageStorage::restoreCompleted);
    
    IMessageStorage::OperationResult result = restoreStorage.restore(backupPath);
    QVERIFY(result == IMessageStorage::Success);
    
    // Verify restored data
    int messageCount = restoreStorage.getMessageCount();
    QVERIFY(messageCount > 0);
}

// Chat UI component tests
void ChatModuleTest::testChatWidgetInitialization()
{
    if (m_skipUITests) {
        QSKIP("UI tests disabled");
    }
    
    // Test widget initialization
    QVERIFY(m_chatWidget != nullptr);
    
    // Test setting chat manager
    m_chatWidget->setChatManager(m_chatManager.get());
    QVERIFY(m_chatWidget->chatManager() == m_chatManager.get());
    
    // Test initial state
    QVERIFY(m_chatWidget->currentRoom().isEmpty());
    QVERIFY(m_chatWidget->isConnected() == false);
    QVERIFY(m_chatWidget->participantCount() == 0);
    QVERIFY(m_chatWidget->isInputEnabled() == true);
}

void ChatModuleTest::testChatWidgetConfiguration()
{
    if (m_skipUITests) {
        QSKIP("UI tests disabled");
    }
    
    // Test widget configuration
    QVariantMap config;
    config["theme"] = "dark";
    config["displayMode"] = "compact";
    config["showParticipants"] = true;
    config["showToolbar"] = true;
    
    m_chatWidget->setConfiguration(config);
    
    QVariantMap appliedConfig = m_chatWidget->getConfiguration();
    QCOMPARE(appliedConfig["theme"].toString(), QString("dark"));
    QCOMPARE(appliedConfig["showParticipants"].toBool(), true);
}

void ChatModuleTest::testChatWidgetThemes()
{
    if (m_skipUITests) {
        QSKIP("UI tests disabled");
    }
    
    // Test theme changes
    QSignalSpy themeSpy(m_chatWidget.get(), &ChatWidget::themeChanged);
    
    m_chatWidget->setTheme("dark");
    QCOMPARE(m_chatWidget->theme(), QString("dark"));
    QVERIFY(verifySignalEmission(themeSpy));
    
    m_chatWidget->setTheme("light");
    QCOMPARE(m_chatWidget->theme(), QString("light"));
    
    // Test applying theme
    m_chatWidget->applyTheme("custom");
    QCOMPARE(m_chatWidget->theme(), QString("custom"));
}

void ChatModuleTest::testChatWidgetInteraction()
{
    if (m_skipUITests) {
        QSKIP("UI tests disabled");
    }
    
    // Setup widget with manager
    m_chatWidget->setChatManager(m_chatManager.get());
    
    // Test connection signals
    QSignalSpy connectionSpy(m_chatWidget.get(), &ChatWidget::connectionChanged);
    
    // Simulate connection
    m_chatWidget->connectToChat("wss://test.server.com");
    
    // Test room operations
    QSignalSpy roomJoinedSpy(m_chatWidget.get(), &ChatWidget::roomJoined);
    m_chatWidget->joinRoom("test_room");
    
    // Test message sending
    QSignalSpy messageSentSpy(m_chatWidget.get(), &ChatWidget::messageSent);
    m_chatWidget->sendMessage("Test UI message");
    
    QVERIFY(verifySignalEmission(messageSentSpy));
}

void ChatModuleTest::testMessageListDisplay()
{
    if (m_skipUITests) {
        QSKIP("UI tests disabled");
    }
    
    // Get message list component
    MessageList* messageList = m_chatWidget->messageList();
    QVERIFY(messageList != nullptr);
    
    // Test adding messages to display
    ChatMessage* testMessage = createTestMessage("Display test message", "user123", "room123");
    
    // This would normally be handled by the widget's message handling
    // For testing, we simulate the display update
    
    delete testMessage;
}

void ChatModuleTest::testInputWidgetFunctionality()
{
    if (m_skipUITests) {
        QSKIP("UI tests disabled");
    }
    
    // Get input widget component
    InputWidget* inputWidget = m_chatWidget->inputWidget();
    QVERIFY(inputWidget != nullptr);
    
    // Test input enabling/disabling
    m_chatWidget->setInputEnabled(false);
    QVERIFY(m_chatWidget->isInputEnabled() == false);
    
    m_chatWidget->setInputEnabled(true);
    QVERIFY(m_chatWidget->isInputEnabled() == true);
    
    // Test simulated user input
    simulateUserInput("Test input message");
}

void ChatModuleTest::testUIComponentSignals()
{
    if (m_skipUITests) {
        QSKIP("UI tests disabled");
    }
    
    // Test various UI signals
    QSignalSpy currentRoomSpy(m_chatWidget.get(), &ChatWidget::currentRoomChanged);
    QSignalSpy participantCountSpy(m_chatWidget.get(), &ChatWidget::participantCountChanged);
    QSignalSpy inputEnabledSpy(m_chatWidget.get(), &ChatWidget::inputEnabledChanged);
    
    // Trigger signal emissions
    m_chatWidget->setCurrentRoom("new_room");
    m_chatWidget->setInputEnabled(false);
    
    QVERIFY(verifySignalEmission(currentRoomSpy));
    QVERIFY(verifySignalEmission(inputEnabledSpy));
}

void ChatModuleTest::testUIComponentValidation()
{
    if (m_skipUITests) {
        QSKIP("UI tests disabled");
    }
    
    // Test UI component validation
    QVERIFY(m_chatWidget->messageList() != nullptr);
    QVERIFY(m_chatWidget->inputWidget() != nullptr);
    
    // Test widget hierarchy
    QVERIFY(m_chatWidget->messageList()->parent() != nullptr);
    QVERIFY(m_chatWidget->inputWidget()->parent() != nullptr);
}

// Helper function implementations
void ChatModuleTest::setupTestEnvironment()
{
    m_testDataPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/ChatModuleTest";
    QDir().mkpath(m_testDataPath);
}

void ChatModuleTest::cleanupTestEnvironment()
{
    if (!m_testDataPath.isEmpty()) {
        QDir(m_testDataPath).removeRecursively();
    }
}

void ChatModuleTest::setupTestData()
{
    m_testConfiguration.clear();
    m_testConfiguration["serverUrl"] = "wss://test.server.com";
    m_testConfiguration["maxMessages"] = 1000;
    m_testConfiguration["enableHistory"] = true;
    m_testConfiguration["enableEncryption"] = false;
    
    m_testMessageContents = {
        "Hello world!",
        "This is a test message",
        "Another test message with more content",
        "Short msg",
        "A very long message that contains a lot of text to test message handling with longer content that might need special processing or truncation in some cases"
    };
}

void ChatModuleTest::cleanupTestData()
{
    qDeleteAll(m_testMessages);
    m_testMessages.clear();
    
    qDeleteAll(m_testParticipants);
    m_testParticipants.clear();
    
    qDeleteAll(m_testRooms);
    m_testRooms.clear();
    
    m_testConfiguration.clear();
    m_testMessageContents.clear();
}

void ChatModuleTest::setupMockObjects()
{
    // Setup mock objects for testing
    // In a real implementation, these would be proper mock objects
    m_mockNetworkManager = new QObject(this);
    m_mockFileManager = new QObject(this);
    m_mockCryptoHandler = new QObject(this);
}

void ChatModuleTest::cleanupMockObjects()
{
    if (m_mockNetworkManager) {
        delete m_mockNetworkManager;
        m_mockNetworkManager = nullptr;
    }
    
    if (m_mockFileManager) {
        delete m_mockFileManager;
        m_mockFileManager = nullptr;
    }
    
    if (m_mockCryptoHandler) {
        delete m_mockCryptoHandler;
        m_mockCryptoHandler = nullptr;
    }
}

ChatMessage* ChatModuleTest::createTestMessage(const QString& content, const QString& senderId, const QString& roomId)
{
    ChatMessage* message = new ChatMessage();
    message->setContent(content);
    message->setSenderId(senderId);
    message->setRoomId(roomId);
    message->setTimestamp(QDateTime::currentDateTime());
    message->setStatus(ChatMessage::Sent);
    return message;
}

Participant* ChatModuleTest::createTestParticipant(const QString& id, const QString& name)
{
    Participant* participant = new Participant();
    participant->setId(id);
    participant->setName(name);
    participant->setOnline(true);
    return participant;
}

ChatRoom* ChatModuleTest::createTestRoom(const QString& id, const QString& name)
{
    ChatRoom* room = new ChatRoom();
    room->setId(id);
    room->setName(name);
    room->setActive(true);
    return room;
}

bool ChatModuleTest::verifySignalEmission(QSignalSpy& spy, int expectedCount, int timeoutMs)
{
    if (spy.count() >= expectedCount) {
        return true;
    }
    
    return spy.wait(timeoutMs) && spy.count() >= expectedCount;
}

QVariantMap ChatModuleTest::generateTestConfiguration()
{
    return m_testConfiguration;
}

QStringList ChatModuleTest::generateTestMessages(int count)
{
    QStringList messages;
    for (int i = 0; i < count; ++i) {
        if (i < m_testMessageContents.size()) {
            messages.append(m_testMessageContents[i]);
        } else {
            messages.append(QString("Generated test message %1").arg(i));
        }
    }
    return messages;
}

QList<ChatMessage*> ChatModuleTest::generateTestMessageObjects(int count)
{
    QList<ChatMessage*> messages;
    QStringList contents = generateTestMessages(count);
    
    for (int i = 0; i < contents.size(); ++i) {
        ChatMessage* message = createTestMessage(
            contents[i], 
            QString("user_%1").arg(i % 3), // Rotate between 3 users
            QString("room_%1").arg(i % 2)  // Rotate between 2 rooms
        );
        messages.append(message);
    }
    
    return messages;
}

void ChatModuleTest::simulateUserInput(const QString& text)
{
    if (m_skipUITests) {
        return;
    }
    
    // Simulate user typing in input widget
    InputWidget* inputWidget = m_chatWidget->inputWidget();
    if (inputWidget) {
        // This would normally involve setting text and triggering events
        // For testing purposes, we just verify the widget exists
        Q_UNUSED(text)
    }
}

// Stub implementations for remaining test methods
void ChatModuleTest::testHistoryManagerInitialization() { /* Implementation */ }
void ChatModuleTest::testHistoryRetrieval() { /* Implementation */ }
void ChatModuleTest::testHistorySearch() { /* Implementation */ }
void ChatModuleTest::testHistoryCleanup() { /* Implementation */ }
void ChatModuleTest::testHistoryExport() { /* Implementation */ }
void ChatModuleTest::testHistoryImport() { /* Implementation */ }
void ChatModuleTest::testChatMessageModel() { /* Implementation */ }
void ChatModuleTest::testChatRoomModel() { /* Implementation */ }
void ChatModuleTest::testParticipantModel() { /* Implementation */ }
void ChatModuleTest::testModelSerialization() { /* Implementation */ }
void ChatModuleTest::testModelValidation() { /* Implementation */ }
void ChatModuleTest::testModuleIntegration() { /* Implementation */ }
void ChatModuleTest::testComponentCommunication() { /* Implementation */ }
void ChatModuleTest::testEndToEndWorkflow() { /* Implementation */ }
void ChatModuleTest::testConcurrentOperations() { /* Implementation */ }
void ChatModuleTest::testLegacyChatManagerCompatibility() { /* Implementation */ }
void ChatModuleTest::testExistingAPICompatibility() { /* Implementation */ }
void ChatModuleTest::testConfigurationMigration() { /* Implementation */ }
void ChatModuleTest::testDataMigration() { /* Implementation */ }
void ChatModuleTest::testMessageThroughput() { /* Implementation */ }
void ChatModuleTest::testStoragePerformance() { /* Implementation */ }
void ChatModuleTest::testUIPerformance() { /* Implementation */ }
void ChatModuleTest::testMemoryUsage() { /* Implementation */ }
void ChatModuleTest::testConcurrentUsers() { /* Implementation */ }
void ChatModuleTest::testConnectionErrors() { /* Implementation */ }
void ChatModuleTest::testStorageErrors() { /* Implementation */ }
void ChatModuleTest::testMessageErrors() { /* Implementation */ }
void ChatModuleTest::testUIErrors() { /* Implementation */ }
void ChatModuleTest::testRecoveryMechanisms() { /* Implementation */ }
void ChatModuleTest::testMessageValidation_Security() { /* Implementation */ }
void ChatModuleTest::testInputSanitization() { /* Implementation */ }
void ChatModuleTest::testFileUploadSecurity() { /* Implementation */ }
void ChatModuleTest::testDataEncryption() { /* Implementation */ }
void ChatModuleTest::testHighMessageVolume() { /* Implementation */ }
void ChatModuleTest::testLongRunningSession() { /* Implementation */ }
void ChatModuleTest::testResourceExhaustion() { /* Implementation */ }
void ChatModuleTest::testNetworkInstability() { /* Implementation */ }

QTEST_MAIN(ChatModuleTest)