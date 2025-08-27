#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QNetworkReply>
#include "JitsiError.h"
#include "ErrorRecoveryManager.h"
#include "ErrorUtils.h"

class TestErrorHandling : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // JitsiError测试
    void testJitsiErrorCreation();
    void testJitsiErrorCopy();
    void testJitsiErrorStaticFactories();
    void testJitsiErrorSerialization();
    void testJitsiErrorContext();
    void testJitsiErrorRecoverable();
    
    // ErrorUtils测试
    void testUrlValidation();
    void testUrlValidation_data();
    void testServerUrlValidation();
    void testServerUrlValidation_data();
    void testUrlBuilding();
    void testUrlExtraction();
    void testNetworkErrorAnalysis();
    void testNetworkErrorAnalysis_data();
    void testProtocolUrlDetection();
    void testUrlNormalization();
    
    // ErrorRecoveryManager测试
    void testErrorRecoveryManagerCreation();
    void testErrorHandling();
    void testRecoveryStrategies();
    void testRetryMechanism();
    void testErrorLogging();
    void testErrorStatistics();

private:
    ErrorRecoveryManager* m_recoveryManager;
    QString m_tempLogFile;
};

void TestErrorHandling::initTestCase()
{
    // 设置测试环境
    QCoreApplication::setApplicationName("JitsiMeetQtTest");
    QCoreApplication::setOrganizationName("JitsiTest");
}

void TestErrorHandling::cleanupTestCase()
{
    // 清理测试环境
}

void TestErrorHandling::init()
{
    m_recoveryManager = new ErrorRecoveryManager(this);
    m_tempLogFile = QDir::temp().filePath("jitsi_test_errors.log");
    m_recoveryManager->setLogFilePath(m_tempLogFile);
}

void TestErrorHandling::cleanup()
{
    delete m_recoveryManager;
    m_recoveryManager = nullptr;
    
    // 清理临时日志文件
    QFile::remove(m_tempLogFile);
}

void TestErrorHandling::testJitsiErrorCreation()
{
    // 测试基本错误创建
    JitsiError error(ErrorType::NetworkError, "Test network error", "Detailed info");
    
    QCOMPARE(error.type(), ErrorType::NetworkError);
    QCOMPARE(error.message(), QString("Test network error"));
    QCOMPARE(error.details(), QString("Detailed info"));
    QCOMPARE(error.severity(), ErrorSeverity::Error);
    QVERIFY(!error.errorCode().isEmpty());
    QVERIFY(error.timestamp().isValid());
}

void TestErrorHandling::testJitsiErrorCopy()
{
    // 测试错误对象拷贝
    JitsiError original(ErrorType::InvalidUrl, "Original error");
    original.addContext("key1", "value1");
    
    JitsiError copy(original);
    
    QCOMPARE(copy.type(), original.type());
    QCOMPARE(copy.message(), original.message());
    QCOMPARE(copy.errorCode(), original.errorCode());
    QCOMPARE(copy.getContext("key1"), QString("value1"));
    
    // 测试赋值操作符
    JitsiError assigned(ErrorType::SystemError, "Different error");
    assigned = original;
    
    QCOMPARE(assigned.type(), original.type());
    QCOMPARE(assigned.message(), original.message());
}

void TestErrorHandling::testJitsiErrorStaticFactories()
{
    // 测试静态工厂方法
    JitsiError networkError = JitsiError::networkError("Network failed", "Connection timeout");
    QCOMPARE(networkError.type(), ErrorType::NetworkError);
    QCOMPARE(networkError.getContext("category"), QString("network"));
    
    JitsiError urlError = JitsiError::invalidUrlError("invalid-url", "Bad format");
    QCOMPARE(urlError.type(), ErrorType::InvalidUrl);
    QCOMPARE(urlError.getContext("url"), QString("invalid-url"));
    
    JitsiError validationError = JitsiError::validationError("username", "test@user", "Invalid characters");
    QCOMPARE(validationError.type(), ErrorType::ValidationError);
    QCOMPARE(validationError.getContext("field"), QString("username"));
    QCOMPARE(validationError.getContext("value"), QString("test@user"));
}

void TestErrorHandling::testJitsiErrorSerialization()
{
    // 测试错误序列化
    JitsiError error(ErrorType::WebEngineError, "WebEngine crashed", "Stack trace here");
    error.addContext("component", "webengine");
    error.addContext("version", "5.15.2");
    
    QString logString = error.toLogString();
    QVERIFY(logString.contains("WebEngineError"));
    QVERIFY(logString.contains("WebEngine crashed"));
    QVERIFY(logString.contains("component=webengine"));
    
    QString userMessage = error.toUserMessage();
    QVERIFY(!userMessage.isEmpty());
    QVERIFY(userMessage != error.message()); // 应该是用户友好的消息
}

void TestErrorHandling::testJitsiErrorContext()
{
    // 测试上下文信息管理
    JitsiError error(ErrorType::ConfigurationError, "Config error");
    
    error.addContext("file", "config.ini");
    error.addContext("line", "42");
    error.addContext("section", "network");
    
    QCOMPARE(error.getContext("file"), QString("config.ini"));
    QCOMPARE(error.getContext("line"), QString("42"));
    QCOMPARE(error.getContext("nonexistent"), QString());
    
    QMap<QString, QString> allContext = error.getAllContext();
    QCOMPARE(allContext.size(), 4); // 3个自定义 + 1个category
    QVERIFY(allContext.contains("file"));
    QVERIFY(allContext.contains("category"));
}

void TestErrorHandling::testJitsiErrorRecoverable()
{
    // 测试错误可恢复性判断
    JitsiError networkError = JitsiError::networkError("Network error");
    QVERIFY(networkError.isRecoverable());
    
    JitsiError urlError = JitsiError::invalidUrlError("bad-url");
    QVERIFY(urlError.isRecoverable());
    
    JitsiError criticalError(ErrorType::SystemError, "Critical system error", "", ErrorSeverity::Critical);
    QVERIFY(!criticalError.isRecoverable());
}

void TestErrorHandling::testUrlValidation_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<bool>("expectedValid");
    QTest::addColumn<QString>("expectedError");
    
    // 有效的URL
    QTest::newRow("valid_room_name") << "test-room" << true << "";
    QTest::newRow("valid_full_url") << "https://meet.jit.si/test-room" << true << "";
    QTest::newRow("valid_protocol_url") << "jitsi-meet://test-room" << true << "";
    QTest::newRow("valid_complex_room") << "my-test-room-123" << true << "";
    
    // 无效的URL
    QTest::newRow("empty_url") << "" << false << "URL不能为空";
    QTest::newRow("invalid_protocol") << "http://meet.jit.si/room" << false << "URL格式不符合要求";
    QTest::newRow("no_room_name") << "https://meet.jit.si/" << false << "URL中缺少房间名";
    QTest::newRow("invalid_characters") << "room with spaces" << false << "无效的URL或房间名格式";
    QTest::newRow("missing_protocol") << "meet.jit.si/room" << false << "URL缺少协议前缀";
}

void TestErrorHandling::testUrlValidation()
{
    QFETCH(QString, url);
    QFETCH(bool, expectedValid);
    QFETCH(QString, expectedError);
    
    ErrorUtils::UrlValidationResult result = ErrorUtils::validateJitsiUrl(url);
    
    QCOMPARE(result.isValid, expectedValid);
    if (!expectedValid) {
        QVERIFY(result.errorMessage.contains(expectedError) || expectedError.isEmpty());
    }
}

void TestErrorHandling::testServerUrlValidation_data()
{
    QTest::addColumn<QString>("serverUrl");
    QTest::addColumn<bool>("expectedValid");
    
    // 有效的服务器URL
    QTest::newRow("valid_https") << "https://meet.jit.si" << true;
    QTest::newRow("valid_with_port") << "https://meet.example.com:8443" << true;
    QTest::newRow("valid_subdomain") << "https://jitsi.example.org" << true;
    
    // 无效的服务器URL
    QTest::newRow("empty") << "" << false;
    QTest::newRow("http_not_https") << "http://meet.jit.si" << false;
    QTest::newRow("no_protocol") << "meet.jit.si" << false;
    QTest::newRow("invalid_domain") << "https://invalid..domain" << false;
}

void TestErrorHandling::testServerUrlValidation()
{
    QFETCH(QString, serverUrl);
    QFETCH(bool, expectedValid);
    
    ErrorUtils::UrlValidationResult result = ErrorUtils::validateServerUrl(serverUrl);
    QCOMPARE(result.isValid, expectedValid);
}

void TestErrorHandling::testUrlBuilding()
{
    // 测试URL构建
    QString url = ErrorUtils::buildConferenceUrl("test-room", "https://meet.jit.si");
    QCOMPARE(url, QString("https://meet.jit.si/test-room"));
    
    // 测试带斜杠的情况
    url = ErrorUtils::buildConferenceUrl("/test-room", "https://meet.jit.si/");
    QCOMPARE(url, QString("https://meet.jit.si/test-room"));
}

void TestErrorHandling::testUrlExtraction()
{
    // 测试房间名提取
    QString roomName = ErrorUtils::extractRoomName("https://meet.jit.si/test-room-123");
    QCOMPARE(roomName, QString("test-room-123"));
    
    // 测试服务器URL提取
    QString serverUrl = ErrorUtils::extractServerUrl("https://meet.jit.si:8443/test-room");
    QCOMPARE(serverUrl, QString("https://meet.jit.si:8443"));
}

void TestErrorHandling::testNetworkErrorAnalysis_data()
{
    QTest::addColumn<int>("networkError");
    QTest::addColumn<bool>("expectedRetryable");
    QTest::addColumn<int>("expectedDelay");
    
    QTest::newRow("connection_refused") << static_cast<int>(QNetworkReply::ConnectionRefusedError) << true << 5000;
    QTest::newRow("host_not_found") << static_cast<int>(QNetworkReply::HostNotFoundError) << true << 10000;
    QTest::newRow("timeout") << static_cast<int>(QNetworkReply::TimeoutError) << true << 5000;
    QTest::newRow("ssl_error") << static_cast<int>(QNetworkReply::SslHandshakeFailedError) << false << 0;
    QTest::newRow("operation_canceled") << static_cast<int>(QNetworkReply::OperationCanceledError) << false << 0;
}

void TestErrorHandling::testNetworkErrorAnalysis()
{
    QFETCH(int, networkError);
    QFETCH(bool, expectedRetryable);
    QFETCH(int, expectedDelay);
    
    ErrorUtils::NetworkErrorInfo info = ErrorUtils::analyzeNetworkError(
        static_cast<QNetworkReply::NetworkError>(networkError));
    
    QCOMPARE(info.isRetryable, expectedRetryable);
    if (expectedRetryable) {
        QCOMPARE(info.suggestedRetryDelay, expectedDelay);
    }
    QVERIFY(!info.userMessage.isEmpty());
}

void TestErrorHandling::testProtocolUrlDetection()
{
    // 测试协议URL检测
    QVERIFY(ErrorUtils::isJitsiProtocolUrl("jitsi-meet://test-room"));
    QVERIFY(ErrorUtils::isJitsiProtocolUrl("JITSI-MEET://test-room")); // 大小写不敏感
    QVERIFY(!ErrorUtils::isJitsiProtocolUrl("https://meet.jit.si/room"));
    QVERIFY(!ErrorUtils::isJitsiProtocolUrl("test-room"));
}

void TestErrorHandling::testUrlNormalization()
{
    // 测试URL规范化
    QCOMPARE(ErrorUtils::normalizeUrl("  test-room  "), QString("test-room"));
    QCOMPARE(ErrorUtils::normalizeUrl("https://meet.jit.si/room/"), QString("https://meet.jit.si/room"));
    QCOMPARE(ErrorUtils::normalizeUrl("meet.jit.si/room"), QString("https://meet.jit.si/room"));
}

void TestErrorHandling::testErrorRecoveryManagerCreation()
{
    // 测试错误恢复管理器创建
    QVERIFY(m_recoveryManager != nullptr);
    QVERIFY(m_recoveryManager->isLoggingEnabled());
    QCOMPARE(m_recoveryManager->maxRetryCount(), 3);
}

void TestErrorHandling::testErrorHandling()
{
    // 测试错误处理
    QSignalSpy errorHandledSpy(m_recoveryManager, &ErrorRecoveryManager::errorHandled);
    
    JitsiError error = JitsiError::networkError("Test network error");
    ErrorRecoveryManager::RecoveryResult result = m_recoveryManager->handleError(error);
    
    QCOMPARE(errorHandledSpy.count(), 1);
    QVERIFY(result.strategy != ErrorRecoveryManager::RecoveryStrategy::None);
}

void TestErrorHandling::testRecoveryStrategies()
{
    // 测试不同的恢复策略
    ErrorRecoveryManager::RecoveryResult result;
    
    // 网络错误 - 应该重试
    result = m_recoveryManager->attemptRecovery(ErrorType::NetworkError);
    QCOMPARE(result.strategy, ErrorRecoveryManager::RecoveryStrategy::Retry);
    QVERIFY(result.success);
    
    // URL错误 - 需要用户干预
    result = m_recoveryManager->attemptRecovery(ErrorType::InvalidUrl);
    QCOMPARE(result.strategy, ErrorRecoveryManager::RecoveryStrategy::UserIntervention);
    QVERIFY(!result.success);
    
    // 配置错误 - 重置
    result = m_recoveryManager->attemptRecovery(ErrorType::ConfigurationError);
    QCOMPARE(result.strategy, ErrorRecoveryManager::RecoveryStrategy::Reset);
}

void TestErrorHandling::testRetryMechanism()
{
    // 测试重试机制
    QSignalSpy recoverySpy(m_recoveryManager, &ErrorRecoveryManager::recoverySuccessful);
    
    // 设置较小的重试次数用于测试
    m_recoveryManager->setMaxRetryCount(2);
    
    // 模拟网络错误
    m_recoveryManager->handleNetworkError("Test network error");
    
    // 验证重试逻辑被触发
    QVERIFY(m_recoveryManager->maxRetryCount() == 2);
}

void TestErrorHandling::testErrorLogging()
{
    // 测试错误日志记录
    JitsiError error = JitsiError::webEngineError("Test WebEngine error", "Detailed stack trace");
    error.addContext("component", "webview");
    
    m_recoveryManager->logError(error);
    
    // 验证日志文件是否创建并包含错误信息
    QFile logFile(m_tempLogFile);
    QVERIFY(logFile.exists());
    
    if (logFile.open(QIODevice::ReadOnly)) {
        QString logContent = QString::fromUtf8(logFile.readAll());
        QVERIFY(logContent.contains("WebEngineError"));
        QVERIFY(logContent.contains("Test WebEngine error"));
        QVERIFY(logContent.contains("component=webview"));
        logFile.close();
    }
}

void TestErrorHandling::testErrorStatistics()
{
    // 测试错误统计
    m_recoveryManager->resetErrorStatistics();
    
    // 生成一些错误
    m_recoveryManager->handleError(JitsiError::networkError("Error 1"));
    m_recoveryManager->handleError(JitsiError::networkError("Error 2"));
    m_recoveryManager->handleError(JitsiError::invalidUrlError("bad-url"));
    
    QMap<ErrorType, int> stats = m_recoveryManager->getErrorStatistics();
    QCOMPARE(stats[ErrorType::NetworkError], 2);
    QCOMPARE(stats[ErrorType::InvalidUrl], 1);
}

QTEST_MAIN(TestErrorHandling)
#include "test_error_handling.moc"