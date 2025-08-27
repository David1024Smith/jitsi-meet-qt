#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QSignalSpy>

#include "ConfigurationManager.h"
#include "models/ApplicationSettings.h"
#include "JitsiConstants.h"

class TestConfigurationManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // Basic functionality tests
    void testDefaultConfiguration();
    void testConfigurationPersistence();
    void testServerUrlManagement();
    void testRecentUrlsManagement();
    void testConfigurationValidation();
    void testErrorRecovery();
    void testSignalEmission();
    
    // Edge cases and error handling
    void testInvalidConfigurationHandling();
    void testCorruptedConfigFile();
    void testPermissionErrors();
    void testConcurrentAccess();
    
    // Performance tests
    void testLargeRecentUrlsList();
    void testFrequentConfigurationChanges();

private:
    ConfigurationManager* m_manager;
    QTemporaryDir* m_tempDir;
    QString m_originalConfigPath;

};

void TestConfigurationManager::initTestCase()
{
    // Set up test environment
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    // Store original config path
    m_originalConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

void TestConfigurationManager::cleanupTestCase()
{
    delete m_tempDir;
}

void TestConfigurationManager::init()
{
    // Create fresh configuration manager for each test
    m_manager = new ConfigurationManager();
}

void TestConfigurationManager::cleanup()
{
    delete m_manager;
    m_manager = nullptr;
}

void TestConfigurationManager::testDefaultConfiguration()
{
    ApplicationSettings config = m_manager->currentConfiguration();
    
    QCOMPARE(config.defaultServerUrl, JitsiConstants::DEFAULT_SERVER_URL);
    QCOMPARE(config.serverTimeout, JitsiConstants::DEFAULT_SERVER_TIMEOUT);
    QCOMPARE(config.language, JitsiConstants::DEFAULT_LANGUAGE);
    QCOMPARE(config.darkMode, false);
    QCOMPARE(config.maximized, false);
    QCOMPARE(config.rememberWindowState, true);
    QCOMPARE(config.autoJoinAudio, true);
    QCOMPARE(config.autoJoinVideo, false);
    QCOMPARE(config.maxRecentItems, JitsiConstants::MAX_RECENT_ITEMS);
    QVERIFY(config.recentUrls.isEmpty());
    QVERIFY(config.isValid());
}
void TestConfigurationManager::testConfigurationPersistence()
{
    // Create and modify configuration
    ApplicationSettings config = m_manager->currentConfiguration();
    config.defaultServerUrl = "https://persistence.test.com";
    config.language = "zh-CN";
    config.darkMode = true;
    config.autoJoinAudio = false;
    config.recentUrls << "https://test1.com" << "https://test2.com";
    
    // Save configuration
    m_manager->saveConfiguration(config);
    
    // Create new manager instance to test persistence
    delete m_manager;
    m_manager = new ConfigurationManager();
    
    // Load and verify configuration
    ApplicationSettings loadedConfig = m_manager->loadConfiguration();
    
    QCOMPARE(loadedConfig.defaultServerUrl, config.defaultServerUrl);
    QCOMPARE(loadedConfig.language, config.language);
    QCOMPARE(loadedConfig.darkMode, config.darkMode);
    QCOMPARE(loadedConfig.autoJoinAudio, config.autoJoinAudio);
    QCOMPARE(loadedConfig.recentUrls, config.recentUrls);
}
void TestConfigurationManager::testServerUrlManagement()
{
    QString originalUrl = m_manager->serverUrl();
    
    // Test valid URL setting
    QString testUrl = "https://test.example.com";
    m_manager->setServerUrl(testUrl);
    QCOMPARE(m_manager->serverUrl(), testUrl);
    
    // Test invalid URL rejection
    QStringList invalidUrls = {
        "",
        "invalid-url",
        "ftp://example.com",
        "javascript:alert('xss')"
    };
    
    for (const QString& invalidUrl : invalidUrls) {
        m_manager->setServerUrl(invalidUrl);
        QVERIFY(m_manager->serverUrl() != invalidUrl);
    }
    
    // Test valid URLs acceptance
    QStringList validUrls = {
        "https://meet.jit.si",
        "http://localhost:8080",
        "https://example.com/jitsi"
    };
    
    for (const QString& validUrl : validUrls) {
        m_manager->setServerUrl(validUrl);
        QCOMPARE(m_manager->serverUrl(), validUrl);
    }
}
void TestConfigurationManager::testRecentUrlsManagement()
{
    // Clear recent URLs
    m_manager->clearRecentUrls();
    QVERIFY(m_manager->recentUrls().isEmpty());
    
    // Add URLs
    QStringList testUrls = {
        "https://meet1.example.com/room1",
        "https://meet2.example.com/room2",
        "https://meet3.example.com/room3"
    };
    
    for (const QString& url : testUrls) {
        m_manager->addRecentUrl(url);
    }
    
    QStringList recentUrls = m_manager->recentUrls();
    QCOMPARE(recentUrls.size(), testUrls.size());
    
    // Check order (most recent first)
    for (int i = 0; i < testUrls.size(); ++i) {
        QCOMPARE(recentUrls[i], testUrls[testUrls.size() - 1 - i]);
    }
    
    // Test duplicate handling
    m_manager->addRecentUrl(testUrls.first());
    recentUrls = m_manager->recentUrls();
    QCOMPARE(recentUrls.size(), testUrls.size());
    QCOMPARE(recentUrls.first(), testUrls.first());
    
    // Test maximum limit
    ApplicationSettings config = m_manager->currentConfiguration();
    int maxItems = config.maxRecentItems;
    
    for (int i = 0; i < maxItems + 5; ++i) {
        m_manager->addRecentUrl(QString("https://test%1.com").arg(i));
    }
    
    recentUrls = m_manager->recentUrls();
    QVERIFY(recentUrls.size() <= maxItems);
}
void TestConfigurationManager::testConfigurationValidation()
{
    QVERIFY(m_manager->validateConfiguration());
    
    // Test with invalid configuration
    ApplicationSettings invalidConfig;
    invalidConfig.defaultServerUrl = "invalid-url";
    invalidConfig.serverTimeout = -1;
    
    m_manager->saveConfiguration(invalidConfig);
    
    // Configuration should be automatically corrected
    ApplicationSettings correctedConfig = m_manager->currentConfiguration();
    QVERIFY(correctedConfig.isValid());
    QVERIFY(correctedConfig.defaultServerUrl != "invalid-url");
    QVERIFY(correctedConfig.serverTimeout > 0);
}
void TestConfigurationManager::testErrorRecovery()
{
    // Test reset to defaults
    m_manager->resetToDefaults();
    ApplicationSettings config = m_manager->currentConfiguration();
    
    ApplicationSettings defaultSettings;
    QCOMPARE(config.defaultServerUrl, defaultSettings.defaultServerUrl);
    QCOMPARE(config.language, defaultSettings.language);
    QCOMPARE(config.darkMode, defaultSettings.darkMode);
    QVERIFY(config.isValid());
}
    
    static bool testErrorRecovery()
    {
        qDebug() << "Testing error recovery...";
        
        ConfigurationManager manager;
        
        // Test configuration validation
        if (!manager.validateConfiguration()) {
            qWarning() << "Configuration validation failed";
            return false;
        }
        
        // Test reset to defaults
        manager.resetToDefaults();
        ApplicationSettings config = manager.currentConfiguration();
        
        ApplicationSettings defaultSettings;
        if (config.defaultServerUrl != defaultSettings.defaultServerUrl ||
            config.language != defaultSettings.language) {
            qWarning() << "Reset to defaults failed";
            return false;
        }
        
        qDebug() << "Error recovery test passed";
        return true;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set application properties for QSettings
    QCoreApplication::setApplicationName(JitsiConstants::APP_NAME);
    QCoreApplication::setOrganizationName(JitsiConstants::APP_ORGANIZATION);
    QCoreApplication::setApplicationVersion(JitsiConstants::APP_VERSION);
    
    bool success = ConfigurationTest::runTests();
    
    return success ? 0 : 1;
}