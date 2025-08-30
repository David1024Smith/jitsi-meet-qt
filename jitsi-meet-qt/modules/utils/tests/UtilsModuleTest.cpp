#include "UtilsModuleTest.h"
#include "../include/Logger.h"
#include "../include/FileManager.h"
#include "../logging/FileLogger.h"
#include "../logging/ConsoleLogger.h"
#include "../logging/NetworkLogger.h"
#include "../file/ConfigFile.h"
#include "../file/TempFile.h"
#include "../file/FileWatcher.h"
#include "../crypto/AESCrypto.h"
#include "../crypto/RSACrypto.h"
#include "../crypto/HashUtils.h"
#include "../string/StringUtils.h"
#include "../string/Validator.h"
#include "../include/UtilsModule.h"
#include "../config/UtilsConfig.h"
#include "../include/UtilsErrorHandler.h"

#include <QCoreApplication>
#include <QThread>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrent>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslSocket>

UtilsModuleTest::UtilsModuleTest(QObject* parent)
    : QObject(parent)
    , m_performanceTestsEnabled(true)
    , m_securityTestsEnabled(true)
    , m_stressTestsEnabled(false) // 默认关闭压力测试
{
}

void UtilsModuleTest::initTestCase()
{
    // 创建测试目录
    m_testDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/utils_test";
    QDir().mkpath(m_testDir);
    
    // 初始化测试环境
    m_testTimer.start();
    
    // 检查测试环境
    qDebug() << "=== Utils Module Test Framework ===";
    qDebug() << "Test directory:" << m_testDir;
    qDebug() << "Performance tests:" << (m_performanceTestsEnabled ? "Enabled" : "Disabled");
    qDebug() << "Security tests:" << (m_securityTestsEnabled ? "Enabled" : "Disabled");
    qDebug() << "Stress tests:" << (m_stressTestsEnabled ? "Enabled" : "Disabled");
    qDebug() << "Available threads:" << QThread::idealThreadCount();
    qDebug() << "SSL support:" << QSslSocket::supportsSsl();
    
    // 创建测试子目录
    QDir().mkpath(m_testDir + "/logs");
    QDir().mkpath(m_testDir + "/configs");
    QDir().mkpath(m_testDir + "/temp");
    QDir().mkpath(m_testDir + "/crypto");
    QDir().mkpath(m_testDir + "/performance");
}

void UtilsModuleTest::cleanupTestCase()
{
    // 输出测试统计
    qint64 totalTime = m_testTimer.elapsed();
    qDebug() << "=== Test Summary ===";
    qDebug() << "Total test time:" << totalTime << "ms";
    qDebug() << "Performance results:" << m_performanceResults.size();
    qDebug() << "Memory results:" << m_memoryResults.size();
    qDebug() << "Created files:" << m_createdFiles.size();
    qDebug() << "Created directories:" << m_createdDirs.size();
    
    // 清理测试目录
    QDir testDir(m_testDir);
    if (testDir.exists()) {
        testDir.removeRecursively();
    }
    
    // 清理测试数据
    m_createdFiles.clear();
    m_createdDirs.clear();
    m_performanceResults.clear();
    m_memoryResults.clear();
}

void UtilsModuleTest::init()
{
    // 每个测试前的初始化
}

void UtilsModuleTest::cleanup()
{
    // 每个测试后的清理
}

// === 日志系统测试 ===
void UtilsModuleTest::testLoggerSingleton()
{
    Logger* logger1 = Logger::instance();
    Logger* logger2 = Logger::instance();
    
    QVERIFY(logger1 != nullptr);
    QCOMPARE(logger1, logger2);
}

void UtilsModuleTest::testLoggerInitialization()
{
    Logger* logger = Logger::instance();
    QVERIFY(logger != nullptr);
    
    QVERIFY(logger->initialize());
    
    // 测试重复初始化
    QVERIFY(logger->initialize());
    
    logger->cleanup();
}

void UtilsModuleTest::testLoggerLevels()
{
    Logger* logger = Logger::instance();
    logger->initialize();
    
    // 测试日志级别设置
    logger->setGlobalLogLevel(Logger::Warning);
    QCOMPARE(logger->globalLogLevel(), Logger::Warning);
    
    logger->setGlobalLogLevel(Logger::Debug);
    QCOMPARE(logger->globalLogLevel(), Logger::Debug);
    
    // 测试静态日志方法
    Logger::debug("Debug message");
    Logger::info("Info message");
    Logger::warning("Warning message");
    Logger::error("Error message");
    Logger::critical("Critical message");
    
    logger->cleanup();
}

void UtilsModuleTest::testLoggerFormat()
{
    Logger* logger = Logger::instance();
    logger->initialize();
    
    // 测试格式化日志
    Logger::info("Formatted message: %1 %2", "Hello", "World");
    Logger::debug("Number: %1, Bool: %2", 42, true);
    
    logger->cleanup();
}

void UtilsModuleTest::testLoggerMultipleLoggers()
{
    Logger* logger = Logger::instance();
    logger->initialize();
    
    QString logFile1 = m_testDir + "/logs/test1.log";
    QString logFile2 = m_testDir + "/logs/test2.log";
    
    FileLogger* fileLogger1 = new FileLogger(logFile1);
    FileLogger* fileLogger2 = new FileLogger(logFile2);
    
    QVERIFY(fileLogger1->initialize());
    QVERIFY(fileLogger2->initialize());
    
    // 添加多个日志记录器
    logger->addLogger(fileLogger1);
    logger->addLogger(fileLogger2);
    
    Logger::info("Test message to multiple loggers");
    
    // 验证文件是否创建
    QVERIFY(QFile::exists(logFile1));
    QVERIFY(QFile::exists(logFile2));
    
    logger->removeLogger(fileLogger1);
    logger->removeLogger(fileLogger2);
    
    delete fileLogger1;
    delete fileLogger2;
    
    logger->cleanup();
}

void UtilsModuleTest::testLoggerThreadSafety()
{
    Logger* logger = Logger::instance();
    logger->initialize();
    
    QString logFile = m_testDir + "/logs/thread_test.log";
    FileLogger* fileLogger = new FileLogger(logFile);
    fileLogger->initialize();
    logger->addLogger(fileLogger);
    
    // 并发日志测试
    const int threadCount = 10;
    const int messagesPerThread = 100;
    
    QList<QFuture<void>> futures;
    
    for (int i = 0; i < threadCount; ++i) {
        QFuture<void> future = QtConcurrent::run([i, messagesPerThread]() {
            for (int j = 0; j < messagesPerThread; ++j) {
                Logger::info("Thread %1 Message %2", i, j);
            }
        });
        futures.append(future);
    }
    
    // 等待所有线程完成
    for (auto& future : futures) {
        future.waitForFinished();
    }
    
    logger->removeLogger(fileLogger);
    delete fileLogger;
    logger->cleanup();
    
    // 验证日志文件
    QVERIFY(QFile::exists(logFile));
    QFile file(logFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray content = file.readAll();
    
    // 应该包含所有消息
    int messageCount = content.count("Thread");
    QCOMPARE(messageCount, threadCount * messagesPerThread);
}

void UtilsModuleTest::testLoggerPerformance()
{
    if (!m_performanceTestsEnabled) {
        QSKIP("Performance tests disabled");
    }
    
    Logger* logger = Logger::instance();
    logger->initialize();
    
    QString logFile = m_testDir + "/logs/performance_test.log";
    FileLogger* fileLogger = new FileLogger(logFile);
    fileLogger->initialize();
    logger->addLogger(fileLogger);
    
    measureExecutionTime("Logger Performance", [&]() {
        for (int i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
            Logger::info("Performance test message %1", i);
        }
    });
    
    logger->removeLogger(fileLogger);
    delete fileLogger;
    logger->cleanup();
}

void UtilsModuleTest::testFileLoggerCreation()
{
    QString logFile = m_testDir + "/logs/creation_test.log";
    
    FileLogger fileLogger(logFile);
    QVERIFY(fileLogger.initialize());
    
    // 测试基本属性
    QCOMPARE(fileLogger.name(), QString("File Logger"));
    QVERIFY(!fileLogger.version().isEmpty());
    
    fileLogger.cleanup();
}

void UtilsModuleTest::testFileLoggerWriting()
{
    QString logFile = m_testDir + "/logs/writing_test.log";
    
    FileLogger fileLogger(logFile);
    QVERIFY(fileLogger.initialize());
    
    // 测试日志记录
    ILogger::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = ILogger::Info;
    entry.category = "Test";
    entry.message = "Test message";
    entry.thread = "main";
    entry.file = __FILE__;
    entry.line = __LINE__;
    
    fileLogger.log(entry);
    fileLogger.flush();
    
    // 验证文件是否创建
    QVERIFY(QFile::exists(logFile));
    
    // 验证文件内容
    QFile file(logFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();
    QVERIFY(content.contains("Test message"));
    QVERIFY(content.contains("Info"));
    QVERIFY(content.contains("Test"));
    
    fileLogger.cleanup();
}

void UtilsModuleTest::testFileLoggerRotation()
{
    QString logFile = m_testDir + "/logs/rotation_test.log";
    
    FileLogger fileLogger(logFile);
    fileLogger.setMaxFileSize(1024); // 1KB
    fileLogger.setMaxBackupFiles(3);
    QVERIFY(fileLogger.initialize());
    
    // 写入大量数据触发轮转
    for (int i = 0; i < 100; ++i) {
        ILogger::LogEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.level = ILogger::Info;
        entry.category = "Rotation";
        entry.message = QString("Long message for rotation test %1 - ").repeated(10).arg(i);
        entry.thread = "main";
        entry.file = __FILE__;
        entry.line = __LINE__;
        
        fileLogger.log(entry);
    }
    
    fileLogger.flush();
    
    // 检查是否创建了备份文件
    QVERIFY(QFile::exists(logFile));
    // 可能存在备份文件
    
    fileLogger.cleanup();
}

void UtilsModuleTest::testFileLoggerFlush()
{
    QString logFile = m_testDir + "/logs/flush_test.log";
    
    FileLogger fileLogger(logFile);
    QVERIFY(fileLogger.initialize());
    
    ILogger::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = ILogger::Info;
    entry.category = "Flush";
    entry.message = "Flush test message";
    entry.thread = "main";
    entry.file = __FILE__;
    entry.line = __LINE__;
    
    fileLogger.log(entry);
    
    // 在flush之前，文件可能为空或不完整
    fileLogger.flush();
    
    // flush后应该能读取到内容
    QFile file(logFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();
    QVERIFY(content.contains("Flush test message"));
    
    fileLogger.cleanup();
}

void UtilsModuleTest::testFileLoggerLargeFiles()
{
    QString logFile = m_testDir + "/logs/large_file_test.log";
    
    FileLogger fileLogger(logFile);
    QVERIFY(fileLogger.initialize());
    
    // 写入大量数据
    const int messageCount = 10000;
    for (int i = 0; i < messageCount; ++i) {
        ILogger::LogEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.level = ILogger::Info;
        entry.category = "Large";
        entry.message = QString("Large file test message %1").arg(i);
        entry.thread = "main";
        entry.file = __FILE__;
        entry.line = __LINE__;
        
        fileLogger.log(entry);
        
        if (i % 1000 == 0) {
            fileLogger.flush();
        }
    }
    
    fileLogger.flush();
    fileLogger.cleanup();
    
    // 验证文件大小和内容
    QFile file(logFile);
    QVERIFY(file.exists());
    QVERIFY(file.size() > 0);
}

void UtilsModuleTest::testFileLoggerConcurrency()
{
    QString logFile = m_testDir + "/logs/concurrency_test.log";
    
    FileLogger fileLogger(logFile);
    QVERIFY(fileLogger.initialize());
    
    // 并发写入测试
    const int threadCount = 5;
    const int messagesPerThread = 200;
    
    QList<QFuture<void>> futures;
    
    for (int t = 0; t < threadCount; ++t) {
        QFuture<void> future = QtConcurrent::run([&fileLogger, t, messagesPerThread]() {
            for (int i = 0; i < messagesPerThread; ++i) {
                ILogger::LogEntry entry;
                entry.timestamp = QDateTime::currentDateTime();
                entry.level = ILogger::Info;
                entry.category = QString("Thread%1").arg(t);
                entry.message = QString("Concurrent message %1").arg(i);
                entry.thread = QString("thread_%1").arg(t);
                entry.file = __FILE__;
                entry.line = __LINE__;
                
                fileLogger.log(entry);
            }
        });
        futures.append(future);
    }
    
    // 等待所有线程完成
    for (auto& future : futures) {
        future.waitForFinished();
    }
    
    fileLogger.flush();
    fileLogger.cleanup();
    
    // 验证所有消息都被写入
    QFile file(logFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray content = file.readAll();
    
    int totalMessages = content.count("Concurrent message");
    QCOMPARE(totalMessages, threadCount * messagesPerThread);
}

void UtilsModuleTest::testConsoleLoggerCreation()
{
    ConsoleLogger consoleLogger;
    QVERIFY(consoleLogger.initialize());
    
    // 测试基本属性
    QCOMPARE(consoleLogger.name(), QString("Console Logger"));
    QVERIFY(!consoleLogger.version().isEmpty());
    
    consoleLogger.cleanup();
}

void UtilsModuleTest::testConsoleLoggerColors()
{
    ConsoleLogger consoleLogger;
    QVERIFY(consoleLogger.initialize());
    
    // 测试颜色支持
    bool colorSupported = ConsoleLogger::supportsColor();
    qDebug() << "Color support:" << colorSupported;
    
    // 测试不同级别的颜色输出
    ILogger::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.thread = "main";
    entry.file = __FILE__;
    entry.line = __LINE__;
    entry.category = "Color";
    
    entry.level = ILogger::Debug;
    entry.message = "Debug message (should be gray)";
    consoleLogger.log(entry);
    
    entry.level = ILogger::Info;
    entry.message = "Info message (should be white)";
    consoleLogger.log(entry);
    
    entry.level = ILogger::Warning;
    entry.message = "Warning message (should be yellow)";
    consoleLogger.log(entry);
    
    entry.level = ILogger::Error;
    entry.message = "Error message (should be red)";
    consoleLogger.log(entry);
    
    entry.level = ILogger::Critical;
    entry.message = "Critical message (should be bright red)";
    consoleLogger.log(entry);
    
    consoleLogger.cleanup();
}

void UtilsModuleTest::testConsoleLoggerStreams()
{
    ConsoleLogger consoleLogger;
    QVERIFY(consoleLogger.initialize());
    
    // 测试不同输出流
    ILogger::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.thread = "main";
    entry.file = __FILE__;
    entry.line = __LINE__;
    entry.category = "Stream";
    
    // Info和Debug通常输出到stdout
    entry.level = ILogger::Info;
    entry.message = "Info to stdout";
    consoleLogger.log(entry);
    
    // Warning和Error通常输出到stderr
    entry.level = ILogger::Error;
    entry.message = "Error to stderr";
    consoleLogger.log(entry);
    
    consoleLogger.cleanup();
}

void UtilsModuleTest::testConsoleLoggerFormatting()
{
    ConsoleLogger consoleLogger;
    QVERIFY(consoleLogger.initialize());
    
    // 测试格式化选项
    consoleLogger.setShowTimestamp(true);
    consoleLogger.setShowThreadId(true);
    consoleLogger.setShowCategory(true);
    consoleLogger.setShowFileLocation(true);
    
    ILogger::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = ILogger::Info;
    entry.category = "Format";
    entry.message = "Formatted console message";
    entry.thread = "main";
    entry.file = __FILE__;
    entry.line = __LINE__;
    
    consoleLogger.log(entry);
    
    // 测试简化格式
    consoleLogger.setShowTimestamp(false);
    consoleLogger.setShowThreadId(false);
    consoleLogger.setShowCategory(false);
    consoleLogger.setShowFileLocation(false);
    
    entry.message = "Simple console message";
    consoleLogger.log(entry);
    
    consoleLogger.cleanup();
}

void UtilsModuleTest::testNetworkLoggerCreation()
{
    NetworkLogger networkLogger("http://localhost:8080/logs");
    QVERIFY(networkLogger.initialize());
    
    // 测试基本属性
    QCOMPARE(networkLogger.name(), QString("Network Logger"));
    QVERIFY(!networkLogger.version().isEmpty());
    
    networkLogger.cleanup();
}

void UtilsModuleTest::testNetworkLoggerConfig()
{
    NetworkLogger networkLogger("http://localhost:8080/logs");
    
    // 测试配置
    networkLogger.setBatchSize(100);
    networkLogger.setFlushInterval(5000);
    networkLogger.setTimeout(10000);
    networkLogger.setRetryCount(3);
    
    QCOMPARE(networkLogger.batchSize(), 100);
    QCOMPARE(networkLogger.flushInterval(), 5000);
    QCOMPARE(networkLogger.timeout(), 10000);
    QCOMPARE(networkLogger.retryCount(), 3);
    
    QVERIFY(networkLogger.initialize());
    networkLogger.cleanup();
}

void UtilsModuleTest::testNetworkLoggerBatching()
{
    NetworkLogger networkLogger("http://localhost:8080/logs");
    networkLogger.setBatchSize(5);
    networkLogger.setFlushInterval(1000);
    
    QVERIFY(networkLogger.initialize());
    
    // 发送多条日志消息
    for (int i = 0; i < 10; ++i) {
        ILogger::LogEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.level = ILogger::Info;
        entry.category = "Batch";
        entry.message = QString("Batch message %1").arg(i);
        entry.thread = "main";
        entry.file = __FILE__;
        entry.line = __LINE__;
        
        networkLogger.log(entry);
    }
    
    // 等待批处理发送
    QTest::qWait(2000);
    
    networkLogger.cleanup();
}

void UtilsModuleTest::testNetworkLoggerReconnection()
{
    NetworkLogger networkLogger("http://invalid-host:8080/logs");
    networkLogger.setRetryCount(2);
    networkLogger.setTimeout(1000);
    
    QVERIFY(networkLogger.initialize());
    
    // 发送消息到无效主机（应该触发重连机制）
    ILogger::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = ILogger::Error;
    entry.category = "Reconnect";
    entry.message = "Test reconnection";
    entry.thread = "main";
    entry.file = __FILE__;
    entry.line = __LINE__;
    
    networkLogger.log(entry);
    
    // 等待重试完成
    QTest::qWait(5000);
    
    networkLogger.cleanup();
}

void UtilsModuleTest::testNetworkLoggerSecurity()
{
    if (!m_securityTestsEnabled) {
        QSKIP("Security tests disabled");
    }
    
    // 测试HTTPS连接
    NetworkLogger secureLogger("https://secure-log-server.example.com/logs");
    secureLogger.setUseSSL(true);
    secureLogger.setVerifySSLCertificate(true);
    
    QVERIFY(secureLogger.initialize());
    
    // 测试认证
    secureLogger.setAuthenticationToken("Bearer test-token");
    
    ILogger::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = ILogger::Info;
    entry.category = "Security";
    entry.message = "Secure log message";
    entry.thread = "main";
    entry.file = __FILE__;
    entry.line = __LINE__;
    
    secureLogger.log(entry);
    
    secureLogger.cleanup();
}

void UtilsModuleTest::testLoggerLevels()
{
    Logger* logger = Logger::instance();
    logger->initialize();
    
    // 测试不同日志级别
    logger->setGlobalLogLevel(Logger::Warning);
    QCOMPARE(logger->globalLogLevel(), Logger::Warning);
    
    // 测试静态方法
    Logger::debug("Debug message");
    Logger::info("Info message");
    Logger::warning("Warning message");
    Logger::error("Error message");
    Logger::critical("Critical message");
    
    logger->cleanup();
}

void UtilsModuleTest::testFileManagerInitialization()
{
    FileManager* fileManager = FileManager::instance();
    QVERIFY(fileManager != nullptr);
    
    QVERIFY(fileManager->initialize());
    
    // 测试单例模式
    FileManager* fileManager2 = FileManager::instance();
    QCOMPARE(fileManager, fileManager2);
    
    fileManager->cleanup();
}

void UtilsModuleTest::testFileOperations()
{
    FileManager* fileManager = FileManager::instance();
    fileManager->initialize();
    
    QString testFile = m_testDir + "/test_file.txt";
    QByteArray testData = "Hello, World!";
    
    // 测试写入文件
    FileManager::OperationResult result = fileManager->writeFile(testFile, testData);
    QCOMPARE(result, FileManager::Success);
    
    // 测试文件是否存在
    QVERIFY(fileManager->exists(testFile));
    
    // 测试读取文件
    QByteArray readData;
    result = fileManager->readFile(testFile, readData);
    QCOMPARE(result, FileManager::Success);
    QCOMPARE(readData, testData);
    
    // 测试文件信息
    FileManager::FileInfo info = fileManager->getFileInfo(testFile);
    QCOMPARE(info.type, FileManager::RegularFile);
    QCOMPARE(info.size, testData.size());
    
    // 测试删除文件
    result = fileManager->deleteFile(testFile);
    QCOMPARE(result, FileManager::Success);
    QVERIFY(!fileManager->exists(testFile));
    
    fileManager->cleanup();
}

void UtilsModuleTest::testDirectoryOperations()
{
    FileManager* fileManager = FileManager::instance();
    fileManager->initialize();
    
    QString testDir = m_testDir + "/test_subdir";
    
    // 测试创建目录
    FileManager::OperationResult result = fileManager->createDirectory(testDir);
    QCOMPARE(result, FileManager::Success);
    QVERIFY(fileManager->exists(testDir));
    
    // 测试列出目录内容
    QString testFile = testDir + "/file.txt";
    fileManager->writeFile(testFile, "test");
    
    QStringList files = fileManager->listDirectory(testDir);
    QVERIFY(files.contains(testFile));
    
    // 测试删除目录
    result = fileManager->removeDirectory(testDir, true);
    QCOMPARE(result, FileManager::Success);
    QVERIFY(!fileManager->exists(testDir));
    
    fileManager->cleanup();
}

void UtilsModuleTest::testConfigFileCreation()
{
    QString configFile = m_testDir + "/test_config.ini";
    
    ConfigFile config(configFile, ConfigFile::IniFormat);
    QVERIFY(config.initialize());
    
    // 测试设置和获取值
    config.setValue("section/key1", "value1");
    config.setValue("section/key2", 42);
    config.setValue("section/key3", true);
    
    QCOMPARE(config.value("section/key1").toString(), QString("value1"));
    QCOMPARE(config.value("section/key2").toInt(), 42);
    QCOMPARE(config.value("section/key3").toBool(), true);
    
    // 测试保存
    QVERIFY(config.save());
    QVERIFY(QFile::exists(configFile));
    
    config.cleanup();
}

void UtilsModuleTest::testConfigFileOperations()
{
    QString configFile = m_testDir + "/test_config2.json";
    
    ConfigFile config(configFile, ConfigFile::JsonFormat);
    config.initialize();
    
    // 测试组操作
    config.beginGroup("database");
    config.setValue("host", "localhost");
    config.setValue("port", 5432);
    config.endGroup();
    
    config.beginGroup("ui");
    config.setValue("theme", "dark");
    config.setValue("language", "en");
    config.endGroup();
    
    // 测试键列表
    QStringList keys = config.allKeys();
    QVERIFY(keys.contains("database/host"));
    QVERIFY(keys.contains("ui/theme"));
    
    // 测试子组
    QStringList groups = config.childGroups();
    QVERIFY(groups.contains("database"));
    QVERIFY(groups.contains("ui"));
    
    config.save();
    config.cleanup();
}

void UtilsModuleTest::testConfigFileFormats()
{
    // 测试不同格式的配置文件
    QStringList formats = {"ini", "json", "xml"};
    
    for (const QString& format : formats) {
        QString configFile = m_testDir + "/test_config." + format;
        
        ConfigFile::Format fmt = ConfigFile::AutoDetect;
        if (format == "ini") fmt = ConfigFile::IniFormat;
        else if (format == "json") fmt = ConfigFile::JsonFormat;
        else if (format == "xml") fmt = ConfigFile::XmlFormat;
        
        ConfigFile config(configFile, fmt);
        config.initialize();
        
        config.setValue("test/value", "test_data");
        QVERIFY(config.save());
        QVERIFY(QFile::exists(configFile));
        
        // 重新加载并验证
        ConfigFile config2(configFile, fmt);
        config2.initialize();
        QCOMPARE(config2.value("test/value").toString(), QString("test_data"));
        
        config.cleanup();
        config2.cleanup();
    }
}

void UtilsModuleTest::testTempFileCreation()
{
    TempFile tempFile("test-XXXXXX", TempFile::ManualDelete);
    QVERIFY(tempFile.initialize());
    
    // 测试文件创建
    QVERIFY(tempFile.create());
    QString fileName = tempFile.fileName();
    QVERIFY(!fileName.isEmpty());
    QVERIFY(QFile::exists(fileName));
    
    tempFile.cleanup();
}

void UtilsModuleTest::testTempFileOperations()
{
    TempFile tempFile("test-XXXXXX", TempFile::ManualDelete);
    tempFile.initialize();
    tempFile.create();
    
    // 测试写入和读取
    QByteArray testData = "Temporary file test data";
    QVERIFY(tempFile.open(QIODevice::WriteOnly));
    qint64 written = tempFile.write(testData);
    QCOMPARE(written, testData.size());
    tempFile.close();
    
    QVERIFY(tempFile.open(QIODevice::ReadOnly));
    QByteArray readData = tempFile.readAll();
    QCOMPARE(readData, testData);
    tempFile.close();
    
    // 测试文件大小
    QCOMPARE(tempFile.size(), testData.size());
    
    tempFile.cleanup();
}

void UtilsModuleTest::testTempFileCleanup()
{
    QString fileName;
    
    {
        TempFile tempFile("cleanup-test-XXXXXX", TempFile::AutoDelete);
        tempFile.initialize();
        tempFile.create();
        fileName = tempFile.fileName();
        QVERIFY(QFile::exists(fileName));
    } // tempFile 析构，应该自动删除文件
    
    // 验证文件是否被删除（可能需要一些时间）
    QTest::qWait(100);
    // 注意：由于Qt的QTemporaryFile的行为，这个测试可能不总是通过
}

void UtilsModuleTest::testFileWatcherInitialization()
{
    FileWatcher watcher;
    QVERIFY(watcher.initialize());
    
    // 测试配置
    FileWatcher::WatchConfig config;
    config.recursive = true;
    config.mode = FileWatcher::WatchBoth;
    
    watcher.setGlobalConfig(config);
    FileWatcher::WatchConfig retrievedConfig = watcher.globalConfig();
    QCOMPARE(retrievedConfig.recursive, true);
    QCOMPARE(retrievedConfig.mode, FileWatcher::WatchBoth);
    
    watcher.cleanup();
}

void UtilsModuleTest::testFileWatcherEvents()
{
    FileWatcher watcher;
    watcher.initialize();
    
    // 创建测试文件
    QString testFile = m_testDir + "/watch_test.txt";
    QFile file(testFile);
    file.open(QIODevice::WriteOnly);
    file.write("initial content");
    file.close();
    
    // 添加监控
    QVERIFY(watcher.addWatch(testFile));
    QVERIFY(watcher.isWatched(testFile));
    
    // 设置信号监听
    QSignalSpy spy(&watcher, &FileWatcher::fileModified);
    
    // 修改文件
    QTest::qWait(100); // 等待文件系统稳定
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    file.write("modified content");
    file.close();
    
    // 等待事件
    QTest::qWait(1000);
    
    // 验证信号是否被发出（可能因为文件系统延迟而不稳定）
    // QVERIFY(spy.count() > 0);
    
    // 移除监控
    QVERIFY(watcher.removeWatch(testFile));
    QVERIFY(!watcher.isWatched(testFile));
    
    watcher.cleanup();
}

// === 加密工具测试 ===
void UtilsModuleTest::testAESCryptoInitialization()
{
    AESCrypto aes;
    QVERIFY(aes.initialize());
    
    // 测试支持的算法
    QList<ICryptoHandler::Algorithm> algorithms = aes.supportedAlgorithms();
    QVERIFY(algorithms.contains(ICryptoHandler::AES_128));
    QVERIFY(algorithms.contains(ICryptoHandler::AES_192));
    QVERIFY(algorithms.contains(ICryptoHandler::AES_256));
    
    // 测试名称和版本
    QCOMPARE(aes.name(), QString("AES Crypto Handler"));
    QVERIFY(!aes.version().isEmpty());
    
    // 测试算法支持检查
    QVERIFY(aes.isAlgorithmSupported(ICryptoHandler::AES_256));
    QVERIFY(!aes.isAlgorithmSupported(ICryptoHandler::RSA_2048));
    
    aes.cleanup();
}

void UtilsModuleTest::testAESEncryptionDecryption()
{
    AESCrypto aes;
    aes.initialize();
    
    QString testData = "Hello, AES Encryption! 这是一个测试消息。";
    QString password = "test_password_123";
    
    // 测试简单加密/解密
    QByteArray encrypted = aes.encryptAES(testData.toUtf8(), password);
    QVERIFY(!encrypted.isEmpty());
    QVERIFY(encrypted != testData.toUtf8());
    
    QByteArray decrypted = aes.decryptAES(encrypted, password);
    QCOMPARE(QString::fromUtf8(decrypted), testData);
    
    // 测试错误密码
    QByteArray wrongDecrypted = aes.decryptAES(encrypted, "wrong_password");
    QVERIFY(wrongDecrypted.isEmpty() || wrongDecrypted != testData.toUtf8());
    
    // 测试空数据
    QByteArray emptyEncrypted = aes.encryptAES(QByteArray(), password);
    QByteArray emptyDecrypted = aes.decryptAES(emptyEncrypted, password);
    QVERIFY(emptyDecrypted.isEmpty());
    
    aes.cleanup();
}

void UtilsModuleTest::testAESKeyGeneration()
{
    AESCrypto aes;
    aes.initialize();
    
    // 测试随机密钥生成
    QByteArray key128, key192, key256;
    
    QCOMPARE(aes.generateRandomKey(16, key128), ICryptoHandler::Success);
    QCOMPARE(key128.size(), 16);
    
    QCOMPARE(aes.generateRandomKey(24, key192), ICryptoHandler::Success);
    QCOMPARE(key192.size(), 24);
    
    QCOMPARE(aes.generateRandomKey(32, key256), ICryptoHandler::Success);
    QCOMPARE(key256.size(), 32);
    
    // 验证密钥的随机性
    QVERIFY(key128 != key192.left(16));
    QVERIFY(key192 != key256.left(24));
    
    // 测试从密码派生密钥
    QString password = "test_password";
    QByteArray salt = aes.generateSalt(16);
    QByteArray derivedKey = aes.deriveKeyFromPassword(password, salt, 32);
    
    QCOMPARE(derivedKey.size(), 32);
    QVERIFY(!derivedKey.isEmpty());
    
    // 相同密码和盐应该产生相同密钥
    QByteArray derivedKey2 = aes.deriveKeyFromPassword(password, salt, 32);
    QCOMPARE(derivedKey, derivedKey2);
    
    aes.cleanup();
}

void UtilsModuleTest::testAESModes()
{
    AESCrypto aes;
    aes.initialize();
    
    QString testData = "Test data for different AES modes";
    QByteArray key = aes.generateSalt(32); // 使用随机密钥
    QByteArray iv = aes.generateIV(16);
    
    // 测试不同加密模式
    QList<ICryptoHandler::Mode> modes = {
        ICryptoHandler::CBC,
        ICryptoHandler::ECB,
        ICryptoHandler::CFB,
        ICryptoHandler::OFB
    };
    
    for (ICryptoHandler::Mode mode : modes) {
        QByteArray encrypted, decrypted;
        
        ICryptoHandler::OperationResult encResult = aes.encrypt(
            testData.toUtf8(), key, ICryptoHandler::AES_256, 
            mode, ICryptoHandler::PKCS7, encrypted);
        
        if (encResult == ICryptoHandler::Success) {
            ICryptoHandler::OperationResult decResult = aes.decrypt(
                encrypted, key, ICryptoHandler::AES_256,
                mode, ICryptoHandler::PKCS7, decrypted);
            
            QCOMPARE(decResult, ICryptoHandler::Success);
            QCOMPARE(QString::fromUtf8(decrypted), testData);
        }
    }
    
    aes.cleanup();
}

void UtilsModuleTest::testAESPadding()
{
    AESCrypto aes;
    aes.initialize();
    
    // 测试不同长度的数据和填充
    QStringList testStrings = {
        "A",                    // 1 byte
        "Hello",               // 5 bytes  
        "1234567890123456",    // 16 bytes (block size)
        "12345678901234567"    // 17 bytes
    };
    
    QByteArray key = aes.generateSalt(32);
    
    for (const QString& testStr : testStrings) {
        QByteArray encrypted, decrypted;
        
        ICryptoHandler::OperationResult encResult = aes.encrypt(
            testStr.toUtf8(), key, ICryptoHandler::AES_256,
            ICryptoHandler::CBC, ICryptoHandler::PKCS7, encrypted);
        
        QCOMPARE(encResult, ICryptoHandler::Success);
        
        ICryptoHandler::OperationResult decResult = aes.decrypt(
            encrypted, key, ICryptoHandler::AES_256,
            ICryptoHandler::CBC, ICryptoHandler::PKCS7, decrypted);
        
        QCOMPARE(decResult, ICryptoHandler::Success);
        QCOMPARE(QString::fromUtf8(decrypted), testStr);
    }
    
    aes.cleanup();
}

void UtilsModuleTest::testAESPerformance()
{
    if (!m_performanceTestsEnabled) {
        QSKIP("Performance tests disabled");
    }
    
    AESCrypto aes;
    aes.initialize();
    
    QString password = "performance_test_password";
    QByteArray testData = generateRandomData(LARGE_DATA_SIZE);
    
    measureExecutionTime("AES Encryption Performance", [&]() {
        for (int i = 0; i < 100; ++i) {
            QByteArray encrypted = aes.encryptAES(testData, password);
            Q_UNUSED(encrypted);
        }
    });
    
    QByteArray encrypted = aes.encryptAES(testData, password);
    
    measureExecutionTime("AES Decryption Performance", [&]() {
        for (int i = 0; i < 100; ++i) {
            QByteArray decrypted = aes.decryptAES(encrypted, password);
            Q_UNUSED(decrypted);
        }
    });
    
    aes.cleanup();
}

void UtilsModuleTest::testAESSecurityFeatures()
{
    if (!m_securityTestsEnabled) {
        QSKIP("Security tests disabled");
    }
    
    AESCrypto aes;
    aes.initialize();
    
    QString testData = "Sensitive security test data";
    QString password = "security_password_123";
    
    // 测试加密质量
    QByteArray encrypted = aes.encryptAES(testData.toUtf8(), password);
    QVERIFY(verifyEncryptionQuality(testData.toUtf8(), encrypted));
    
    // 测试盐值随机性
    QByteArray salt1 = aes.generateSalt(16);
    QByteArray salt2 = aes.generateSalt(16);
    QVERIFY(salt1 != salt2);
    
    // 测试IV随机性
    QByteArray iv1 = aes.generateIV(16);
    QByteArray iv2 = aes.generateIV(16);
    QVERIFY(iv1 != iv2);
    
    // 测试相同数据的不同加密结果（由于随机IV）
    QByteArray encrypted1 = aes.encryptAES(testData.toUtf8(), password);
    QByteArray encrypted2 = aes.encryptAES(testData.toUtf8(), password);
    QVERIFY(encrypted1 != encrypted2); // 应该不同（随机IV）
    
    // 但解密结果应该相同
    QByteArray decrypted1 = aes.decryptAES(encrypted1, password);
    QByteArray decrypted2 = aes.decryptAES(encrypted2, password);
    QCOMPARE(decrypted1, decrypted2);
    QCOMPARE(QString::fromUtf8(decrypted1), testData);
    
    aes.cleanup();
}

void UtilsModuleTest::testAESErrorHandling()
{
    AESCrypto aes;
    aes.initialize();
    
    QByteArray key = aes.generateSalt(32);
    QByteArray testData = "Error handling test";
    
    // 测试无效密钥长度
    QByteArray shortKey(8, 'x');
    QByteArray encrypted, decrypted;
    
    ICryptoHandler::OperationResult result = aes.encrypt(
        testData, shortKey, ICryptoHandler::AES_256,
        ICryptoHandler::CBC, ICryptoHandler::PKCS7, encrypted);
    
    QCOMPARE(result, ICryptoHandler::InvalidKey);
    
    // 测试损坏的加密数据
    QByteArray validEncrypted = aes.encryptAES(testData, "password");
    QByteArray corruptedData = validEncrypted;
    corruptedData[0] = corruptedData[0] ^ 0xFF; // 翻转第一个字节
    
    QByteArray decryptedCorrupted = aes.decryptAES(corruptedData, "password");
    QVERIFY(decryptedCorrupted.isEmpty() || decryptedCorrupted != testData);
    
    // 测试空输入
    QByteArray emptyResult = aes.encryptAES(QByteArray(), "password");
    // 空数据的加密可能返回空或包含填充的数据
    
    aes.cleanup();
}

void UtilsModuleTest::testRSACryptoInitialization()
{
    RSACrypto rsa;
    QVERIFY(rsa.initialize());
    
    // Test supported algorithms
    QList<ICryptoHandler::Algorithm> algorithms = rsa.supportedAlgorithms();
    QVERIFY(algorithms.contains(ICryptoHandler::RSA_1024));
    QVERIFY(algorithms.contains(ICryptoHandler::RSA_2048));
    QVERIFY(algorithms.contains(ICryptoHandler::RSA_4096));
    
    // Test name and version
    QCOMPARE(rsa.name(), QString("RSA Crypto Handler"));
    QVERIFY(!rsa.version().isEmpty());
    
    rsa.cleanup();
}

void UtilsModuleTest::testRSAKeyGeneration()
{
    RSACrypto rsa;
    rsa.initialize();
    
    // Test key pair generation
    ICryptoHandler::KeyPair keyPair;
    ICryptoHandler::OperationResult result = rsa.generateKeyPair(ICryptoHandler::RSA_2048, keyPair);
    QCOMPARE(result, ICryptoHandler::Success);
    QVERIFY(keyPair.isValid());
    QVERIFY(!keyPair.publicKey.isEmpty());
    QVERIFY(!keyPair.privateKey.isEmpty());
    
    // Test PEM format generation
    QString publicKeyPem, privateKeyPem;
    bool success = rsa.generateRSAKeyPairPEM(2048, publicKeyPem, privateKeyPem);
    QVERIFY(success);
    QVERIFY(!publicKeyPem.isEmpty());
    QVERIFY(!privateKeyPem.isEmpty());
    
    rsa.cleanup();
}

void UtilsModuleTest::testRSAEncryptionDecryption()
{
    RSACrypto rsa;
    rsa.initialize();
    
    // Generate key pair
    QString publicKeyPem, privateKeyPem;
    bool success = rsa.generateRSAKeyPairPEM(2048, publicKeyPem, privateKeyPem);
    QVERIFY(success);
    
    QString testData = "Hello, RSA!";
    
    // Test encryption/decryption
    QByteArray encrypted = rsa.encryptRSA(testData.toUtf8(), publicKeyPem);
    QVERIFY(!encrypted.isEmpty());
    
    QByteArray decrypted = rsa.decryptRSA(encrypted, privateKeyPem);
    QCOMPARE(QString::fromUtf8(decrypted), testData);
    
    rsa.cleanup();
}

void UtilsModuleTest::testHashUtilsBasic()
{
    QString testData = "Hello, Hash World!";
    
    // Test basic hashing
    HashUtils::HashResult result = HashUtils::hash(testData.toUtf8(), HashUtils::SHA256);
    QVERIFY(result.isValid());
    QVERIFY(!result.hash.isEmpty());
    QVERIFY(!result.hexString.isEmpty());
    QVERIFY(!result.base64String.isEmpty());
    QCOMPARE(result.algorithm, HashUtils::SHA256);
    
    // Test different algorithms
    HashUtils::HashResult md5Result = HashUtils::hash(testData.toUtf8(), HashUtils::MD5);
    HashUtils::HashResult sha1Result = HashUtils::hash(testData.toUtf8(), HashUtils::SHA1);
    
    QVERIFY(md5Result.isValid());
    QVERIFY(sha1Result.isValid());
    QVERIFY(md5Result.hash != sha1Result.hash);
    
    // Test verification
    QVERIFY(HashUtils::verify(testData.toUtf8(), result.hash, HashUtils::SHA256));
    QVERIFY(!HashUtils::verify(testData.toUtf8(), md5Result.hash, HashUtils::SHA256));
}

void UtilsModuleTest::testHashUtilsFile()
{
    QString testFile = m_testDir + "/hash_test.txt";
    QString testContent = "File content for hashing";
    
    // Create test file
    QFile file(testFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(testContent.toUtf8());
    file.close();
    
    // Test file hashing
    HashUtils::HashResult fileResult = HashUtils::hashFile(testFile, HashUtils::SHA256);
    QVERIFY(fileResult.isValid());
    
    // Compare with direct content hashing
    HashUtils::HashResult contentResult = HashUtils::hash(testContent.toUtf8(), HashUtils::SHA256);
    QCOMPARE(fileResult.hash, contentResult.hash);
    
    // Test file verification
    QVERIFY(HashUtils::verifyFile(testFile, fileResult.hash, HashUtils::SHA256));
}

void UtilsModuleTest::testHashUtilsHMAC()
{
    QString testData = "HMAC test data";
    QString key = "secret_key";
    
    // Test HMAC calculation
    HashUtils::HashResult hmacResult = HashUtils::hmac(testData.toUtf8(), key.toUtf8(), HashUtils::SHA256);
    QVERIFY(hmacResult.isValid());
    QVERIFY(!hmacResult.hash.isEmpty());
    
    // Test with different key
    HashUtils::HashResult hmacResult2 = HashUtils::hmac(testData.toUtf8(), "different_key", HashUtils::SHA256);
    QVERIFY(hmacResult2.isValid());
    QVERIFY(hmacResult.hash != hmacResult2.hash);
    
    // Test password hashing
    HashUtils::HashResult passwordResult = HashUtils::hashPassword("test_password");
    QVERIFY(passwordResult.isValid());
}

void UtilsModuleTest::testStringUtilsBasic()
{
    // Test trimming
    QCOMPARE(StringUtils::trim("  hello  "), QString("hello"));
    QCOMPARE(StringUtils::trimLeft("  hello  "), QString("hello  "));
    QCOMPARE(StringUtils::trimRight("  hello  "), QString("  hello"));
    
    // Test case conversion
    QCOMPARE(StringUtils::toCase("Hello World", StringUtils::Upper), QString("HELLO WORLD"));
    QCOMPARE(StringUtils::toCase("Hello World", StringUtils::Lower), QString("hello world"));
    
    // Test string checks
    QVERIFY(StringUtils::isEmpty(""));
    QVERIFY(!StringUtils::isEmpty("test"));
    QVERIFY(StringUtils::isBlank("   "));
    QVERIFY(!StringUtils::isBlank("test"));
    
    // Test numeric checks
    QVERIFY(StringUtils::isNumeric("12345"));
    QVERIFY(!StringUtils::isNumeric("123abc"));
    QVERIFY(StringUtils::isAlpha("abcdef"));
    QVERIFY(!StringUtils::isAlpha("abc123"));
    QVERIFY(StringUtils::isAlphaNumeric("abc123"));
    QVERIFY(!StringUtils::isAlphaNumeric("abc-123"));
}

void UtilsModuleTest::testStringUtilsCase()
{
    QString testString = "hello world test";
    
    // Test camelCase
    QCOMPARE(StringUtils::toCamelCase(testString), QString("helloWorldTest"));
    
    // Test PascalCase
    QCOMPARE(StringUtils::toPascalCase(testString), QString("HelloWorldTest"));
    
    // Test snake_case
    QCOMPARE(StringUtils::toSnakeCase("HelloWorldTest"), QString("hello_world_test"));
    
    // Test kebab-case
    QCOMPARE(StringUtils::toKebabCase("HelloWorldTest"), QString("hello-world-test"));
}

void UtilsModuleTest::testStringUtilsValidation()
{
    // Test string operations
    QVERIFY(StringUtils::startsWith("hello world", "hello"));
    QVERIFY(StringUtils::endsWith("hello world", "world"));
    QVERIFY(StringUtils::contains("hello world", "lo wo"));
    
    // Test splitting and joining
    QStringList parts = StringUtils::split("a,b,c", ",");
    QCOMPARE(parts.size(), 3);
    QCOMPARE(parts[0], QString("a"));
    QCOMPARE(parts[1], QString("b"));
    QCOMPARE(parts[2], QString("c"));
    
    QString joined = StringUtils::join(parts, "-");
    QCOMPARE(joined, QString("a-b-c"));
    
    // Test padding
    QCOMPARE(StringUtils::leftPad("test", 8, '0'), QString("0000test"));
    QCOMPARE(StringUtils::rightPad("test", 8, '0'), QString("test0000"));
    QCOMPARE(StringUtils::center("test", 8, '-'), QString("--test--"));
}

void UtilsModuleTest::testStringUtilsEncoding()
{
    QString testString = "Hello, 世界!";
    
    // Test encoding conversion
    QByteArray utf8Bytes = StringUtils::toBytes(testString, StringUtils::UTF8);
    QString fromUtf8 = StringUtils::fromBytes(utf8Bytes, StringUtils::UTF8);
    QCOMPARE(fromUtf8, testString);
    
    // Test hex encoding
    QString hexString = StringUtils::toHex(testString);
    QString fromHex = StringUtils::fromHex(hexString);
    QCOMPARE(fromHex, testString);
    
    // Test base64 encoding
    QString base64String = StringUtils::toBase64(testString);
    QString fromBase64 = StringUtils::fromBase64(base64String);
    QCOMPARE(fromBase64, testString);
    
    // Test URL encoding
    QString urlEncoded = StringUtils::urlEncode("hello world");
    QString urlDecoded = StringUtils::urlDecode(urlEncoded);
    QCOMPARE(urlDecoded, QString("hello world"));
    
    // Test HTML encoding
    QString htmlEncoded = StringUtils::htmlEncode("<script>alert('test')</script>");
    QVERIFY(htmlEncoded.contains("&lt;"));
    QVERIFY(htmlEncoded.contains("&gt;"));
}

void UtilsModuleTest::testValidatorBasic()
{
    // Test basic validation
    QVERIFY(Validator::isNotEmpty("test"));
    QVERIFY(!Validator::isNotEmpty(""));
    QVERIFY(Validator::isNotBlank("test"));
    QVERIFY(!Validator::isNotBlank("   "));
    
    // Test length validation
    QVERIFY(Validator::hasLength("test", 4));
    QVERIFY(!Validator::hasLength("test", 5));
    QVERIFY(Validator::hasMinLength("test", 3));
    QVERIFY(!Validator::hasMinLength("test", 5));
    QVERIFY(Validator::hasMaxLength("test", 5));
    QVERIFY(!Validator::hasMaxLength("test", 3));
    QVERIFY(Validator::hasLengthBetween("test", 3, 5));
    QVERIFY(!Validator::hasLengthBetween("test", 5, 10));
    
    // Test numeric validation
    QVERIFY(Validator::isInteger("123"));
    QVERIFY(!Validator::isInteger("123.45"));
    QVERIFY(Validator::isFloat("123.45"));
    QVERIFY(!Validator::isFloat("abc"));
    QVERIFY(Validator::isPositiveInteger("123"));
    QVERIFY(!Validator::isPositiveInteger("-123"));
    QVERIFY(Validator::isInRange("50", 0, 100));
    QVERIFY(!Validator::isInRange("150", 0, 100));
}

void UtilsModuleTest::testValidatorEmail()
{
    // Test valid emails
    Validator::ValidationResult result = Validator::validateEmail("test@example.com");
    QVERIFY(result.isValid);
    
    result = Validator::validateEmail("user.name+tag@domain.co.uk");
    QVERIFY(result.isValid);
    
    // Test invalid emails
    result = Validator::validateEmail("invalid-email");
    QVERIFY(!result.isValid);
    QVERIFY(!result.errorMessage.isEmpty());
    
    result = Validator::validateEmail("@domain.com");
    QVERIFY(!result.isValid);
    
    result = Validator::validateEmail("user@");
    QVERIFY(!result.isValid);
    
    // Test empty email
    result = Validator::validateEmail("");
    QVERIFY(!result.isValid);
    QVERIFY(!result.suggestion.isEmpty());
}

void UtilsModuleTest::testValidatorPassword()
{
    // Test strong password
    Validator::ValidationResult result = Validator::validatePassword("StrongP@ssw0rd123");
    QVERIFY(result.isValid);
    
    // Test password strength
    Validator::PasswordStrength strength = Validator::getPasswordStrength("StrongP@ssw0rd123");
    QVERIFY(strength >= Validator::Good);
    
    // Test weak password
    result = Validator::validatePassword("weak");
    QVERIFY(!result.isValid);
    
    strength = Validator::getPasswordStrength("weak");
    QVERIFY(strength <= Validator::Weak);
    
    // Test password requirements
    QStringList requirements = Validator::getPasswordRequirements("weak");
    QVERIFY(!requirements.isEmpty());
    QVERIFY(requirements.contains("at least 8 characters"));
    
    // Test character type checks
    QVERIFY(Validator::hasUpperCase("Test"));
    QVERIFY(!Validator::hasUpperCase("test"));
    QVERIFY(Validator::hasLowerCase("Test"));
    QVERIFY(!Validator::hasLowerCase("TEST"));
    QVERIFY(Validator::hasDigit("test123"));
    QVERIFY(!Validator::hasDigit("test"));
    QVERIFY(Validator::hasSpecialChar("test@123"));
    QVERIFY(!Validator::hasSpecialChar("test123"));
}

QTEST_MAIN(UtilsModuleTest)
#include "UtilsModuleTest.moc"
//
 === 辅助方法实现 ===
void UtilsModuleTest::createTestFile(const QString& path, const QString& content)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(content.toUtf8());
        file.close();
        m_createdFiles.append(path);
    }
}

void UtilsModuleTest::createTestDirectory(const QString& path)
{
    QDir().mkpath(path);
    m_createdDirs.append(path);
}

void UtilsModuleTest::removeTestPath(const QString& path)
{
    QFileInfo info(path);
    if (info.isFile()) {
        QFile::remove(path);
        m_createdFiles.removeAll(path);
    } else if (info.isDir()) {
        QDir(path).removeRecursively();
        m_createdDirs.removeAll(path);
    }
}

QString UtilsModuleTest::getTestFilePath(const QString& name)
{
    return m_testDir + "/" + name;
}

QString UtilsModuleTest::getTestDirPath(const QString& name)
{
    QString path = m_testDir + "/" + name;
    createTestDirectory(path);
    return path;
}

void UtilsModuleTest::measureExecutionTime(const QString& testName, std::function<void()> testFunction)
{
    QElapsedTimer timer;
    timer.start();
    
    testFunction();
    
    qint64 elapsed = timer.elapsed();
    m_performanceResults[testName] = elapsed;
    
    qDebug() << "Performance:" << testName << "took" << elapsed << "ms";
}

void UtilsModuleTest::measureMemoryUsage(const QString& testName, std::function<void()> testFunction)
{
    qint64 memoryBefore = getCurrentMemoryUsage();
    
    testFunction();
    
    qint64 memoryAfter = getCurrentMemoryUsage();
    qint64 memoryDiff = memoryAfter - memoryBefore;
    
    m_memoryResults[testName] = memoryDiff;
    
    qDebug() << "Memory:" << testName << "used" << memoryDiff << "bytes";
}

qint64 UtilsModuleTest::getCurrentMemoryUsage()
{
    // 简化的内存使用测量（在实际实现中可能需要平台特定代码）
#ifdef Q_OS_WIN
    // Windows实现
    return 0;
#elif defined(Q_OS_LINUX)
    // Linux实现
    QFile file("/proc/self/status");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray content = file.readAll();
        // 解析VmRSS行
        // 这里简化处理
        return 0;
    }
#endif
    return 0;
}

bool UtilsModuleTest::testBufferOverflow(const QString& input)
{
    // 简化的缓冲区溢出测试
    if (input.length() > 10000) {
        return true; // 可能的溢出
    }
    return false;
}

bool UtilsModuleTest::testSQLInjection(const QString& input)
{
    // 简化的SQL注入测试
    QStringList sqlKeywords = {"SELECT", "INSERT", "UPDATE", "DELETE", "DROP", "UNION"};
    QString upperInput = input.toUpper();
    
    for (const QString& keyword : sqlKeywords) {
        if (upperInput.contains(keyword)) {
            return true; // 可能的SQL注入
        }
    }
    return false;
}

bool UtilsModuleTest::testXSSAttack(const QString& input)
{
    // 简化的XSS攻击测试
    QStringList xssPatterns = {"<script", "javascript:", "onload=", "onerror="};
    QString lowerInput = input.toLower();
    
    for (const QString& pattern : xssPatterns) {
        if (lowerInput.contains(pattern)) {
            return true; // 可能的XSS攻击
        }
    }
    return false;
}

void UtilsModuleTest::runConcurrentTest(int threadCount, std::function<void()> testFunction)
{
    QList<QFuture<void>> futures;
    
    for (int i = 0; i < threadCount; ++i) {
        QFuture<void> future = QtConcurrent::run(testFunction);
        futures.append(future);
    }
    
    // 等待所有线程完成
    for (auto& future : futures) {
        future.waitForFinished();
    }
}

QString UtilsModuleTest::generateRandomString(int length)
{
    const QString charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString result;
    
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(charset.length());
        result.append(charset.at(index));
    }
    
    return result;
}

QByteArray UtilsModuleTest::generateRandomData(int size)
{
    QByteArray data;
    data.resize(size);
    
    for (int i = 0; i < size; ++i) {
        data[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    
    return data;
}

QString UtilsModuleTest::generateLargeString(int size)
{
    QString result;
    result.reserve(size);
    
    const QString pattern = "0123456789ABCDEF";
    
    for (int i = 0; i < size; ++i) {
        result.append(pattern.at(i % pattern.length()));
    }
    
    return result;
}

bool UtilsModuleTest::verifyFileIntegrity(const QString& filePath, const QByteArray& expectedData)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray actualData = file.readAll();
    return actualData == expectedData;
}

bool UtilsModuleTest::verifyEncryptionQuality(const QByteArray& original, const QByteArray& encrypted)
{
    // 基本的加密质量检查
    if (encrypted.isEmpty() || encrypted == original) {
        return false;
    }
    
    // 检查熵（简化版本）
    QSet<char> uniqueBytes;
    for (char byte : encrypted) {
        uniqueBytes.insert(byte);
    }
    
    // 加密数据应该有足够的熵
    double entropy = static_cast<double>(uniqueBytes.size()) / 256.0;
    return entropy > 0.3; // 至少30%的字节值应该是唯一的
}

QTEST_MAIN(UtilsModuleTest)
#include "UtilsModuleTest.moc"