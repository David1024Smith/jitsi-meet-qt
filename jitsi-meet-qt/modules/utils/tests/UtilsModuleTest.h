#ifndef UTILSMODULETEST_H
#define UTILSMODULETEST_H

#include <QtTest/QtTest>
#include <QObject>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QElapsedTimer>
#include <QThread>
#include <QMutex>
#include <QRandomGenerator>

// Forward declarations
class Logger;
class FileManager;
class FileLogger;
class ConsoleLogger;
class NetworkLogger;
class ConfigFile;
class TempFile;
class FileWatcher;
class AESCrypto;
class RSACrypto;
class HashUtils;
class StringUtils;
class Validator;

/**
 * @brief Utils模块综合测试类
 * 
 * 完整测试Utils模块的所有组件，包括：
 * - 日志系统和文件处理测试
 * - 加密工具和字符串处理测试  
 * - 工具类性能和安全性测试
 * - 模块集成和错误处理测试
 */
class UtilsModuleTest : public QObject
{
    Q_OBJECT

public:
    explicit UtilsModuleTest(QObject* parent = nullptr);

private slots:
    // 测试初始化和清理
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // === 日志系统测试 ===
    void testLoggerSingleton();
    void testLoggerInitialization();
    void testLoggerLevels();
    void testLoggerFormat();
    void testLoggerMultipleLoggers();
    void testLoggerThreadSafety();
    void testLoggerPerformance();
    
    // FileLogger测试
    void testFileLoggerCreation();
    void testFileLoggerWriting();
    void testFileLoggerRotation();
    void testFileLoggerFlush();
    void testFileLoggerLargeFiles();
    void testFileLoggerConcurrency();
    
    // ConsoleLogger测试
    void testConsoleLoggerCreation();
    void testConsoleLoggerColors();
    void testConsoleLoggerStreams();
    void testConsoleLoggerFormatting();
    
    // NetworkLogger测试
    void testNetworkLoggerCreation();
    void testNetworkLoggerConfig();
    void testNetworkLoggerBatching();
    void testNetworkLoggerReconnection();
    void testNetworkLoggerSecurity();
    
    // === 文件处理测试 ===
    void testFileManagerSingleton();
    void testFileManagerInitialization();
    void testFileManagerBasicOperations();
    void testFileManagerDirectoryOperations();
    void testFileManagerCache();
    void testFileManagerHandlers();
    void testFileManagerPermissions();
    void testFileManagerLargeFiles();
    void testFileManagerConcurrency();
    
    // ConfigFile测试
    void testConfigFileCreation();
    void testConfigFileIniFormat();
    void testConfigFileJsonFormat();
    void testConfigFileXmlFormat();
    void testConfigFileGroups();
    void testConfigFileValidation();
    void testConfigFileAutoSave();
    void testConfigFileWatching();
    void testConfigFileEncryption();
    void testConfigFileBackup();
    
    // TempFile测试
    void testTempFileCreation();
    void testTempFileOperations();
    void testTempFileTypes();
    void testTempFileCleanup();
    void testTempFileStaticMethods();
    void testTempFileSecure();
    void testTempFilePermissions();
    
    // FileWatcher测试
    void testFileWatcherCreation();
    void testFileWatcherConfiguration();
    void testFileWatcherFiltering();
    void testFileWatcherEvents();
    void testFileWatcherBatching();
    void testFileWatcherRecursive();
    void testFileWatcherPerformance();
    void testFileWatcherStability();
    
    // === 加密工具测试 ===
    void testAESCryptoInitialization();
    void testAESEncryptionDecryption();
    void testAESKeyGeneration();
    void testAESModes();
    void testAESPadding();
    void testAESPerformance();
    void testAESSecurityFeatures();
    void testAESErrorHandling();
    
    void testRSACryptoInitialization();
    void testRSAKeyGeneration();
    void testRSAEncryptionDecryption();
    void testRSASignatureVerification();
    void testRSAKeyFormats();
    void testRSAPerformance();
    void testRSASecurityFeatures();
    void testRSAErrorHandling();
    
    void testHashUtilsBasic();
    void testHashUtilsFile();
    void testHashUtilsHMAC();
    void testHashUtilsPassword();
    void testHashUtilsPerformance();
    void testHashUtilsSecurity();
    void testHashUtilsCollisions();
    
    // === 字符串处理测试 ===
    void testStringUtilsBasic();
    void testStringUtilsCase();
    void testStringUtilsValidation();
    void testStringUtilsEncoding();
    void testStringUtilsFormatting();
    void testStringUtilsRegex();
    void testStringUtilsLocalization();
    void testStringUtilsPerformance();
    void testStringUtilsUnicode();
    void testStringUtilsSecurity();
    
    void testValidatorBasic();
    void testValidatorEmail();
    void testValidatorPassword();
    void testValidatorURL();
    void testValidatorPhone();
    void testValidatorCreditCard();
    void testValidatorCustomRules();
    void testValidatorPerformance();
    void testValidatorSecurity();
    
    // === 性能测试 ===
    void benchmarkLoggingPerformance();
    void benchmarkFileOperationsPerformance();
    void benchmarkCryptoPerformance();
    void benchmarkStringProcessingPerformance();
    void benchmarkMemoryUsage();
    void benchmarkConcurrentOperations();
    
    // === 安全性测试 ===
    void testCryptoSecurityFeatures();
    void testFileSecurityFeatures();
    void testStringSecurityFeatures();
    void testMemorySecurityFeatures();
    void testInputValidationSecurity();
    void testErrorHandlingSecurity();
    
    // === 集成测试 ===
    void testModuleIntegration();
    void testErrorPropagation();
    void testResourceManagement();
    void testConfigurationManagement();
    void testThreadSafety();
    void testStressTest();
    
    // === 边界条件测试 ===
    void testLargeDataHandling();
    void testEmptyInputHandling();
    void testInvalidInputHandling();
    void testResourceExhaustion();
    void testNetworkFailures();
    void testFileSystemErrors();

private:
    // 辅助方法
    void createTestFile(const QString& path, const QString& content = "test");
    void createTestDirectory(const QString& path);
    void removeTestPath(const QString& path);
    QString getTestFilePath(const QString& name);
    QString getTestDirPath(const QString& name);
    
    // 性能测试辅助方法
    void measureExecutionTime(const QString& testName, std::function<void()> testFunction);
    void measureMemoryUsage(const QString& testName, std::function<void()> testFunction);
    qint64 getCurrentMemoryUsage();
    
    // 安全测试辅助方法
    bool testBufferOverflow(const QString& input);
    bool testSQLInjection(const QString& input);
    bool testXSSAttack(const QString& input);
    
    // 并发测试辅助方法
    void runConcurrentTest(int threadCount, std::function<void()> testFunction);
    
    // 数据生成辅助方法
    QString generateRandomString(int length);
    QByteArray generateRandomData(int size);
    QString generateLargeString(int size);
    
    // 验证辅助方法
    bool verifyFileIntegrity(const QString& filePath, const QByteArray& expectedData);
    bool verifyEncryptionQuality(const QByteArray& original, const QByteArray& encrypted);
    
private:
    // 测试数据
    QString m_testDir;
    QStringList m_createdFiles;
    QStringList m_createdDirs;
    
    // 性能测试数据
    QMap<QString, qint64> m_performanceResults;
    QMap<QString, qint64> m_memoryResults;
    
    // 测试配置
    static const int PERFORMANCE_ITERATIONS = 1000;
    static const int STRESS_TEST_DURATION = 5000; // 5 seconds
    static const int LARGE_DATA_SIZE = 1024 * 1024; // 1MB
    static const int CONCURRENT_THREADS = 10;
    
    // 测试状态
    QMutex m_testMutex;
    QElapsedTimer m_testTimer;
    bool m_performanceTestsEnabled;
    bool m_securityTestsEnabled;
    bool m_stressTestsEnabled;
};

#endif // UTILSMODULETEST_H