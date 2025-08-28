#include <QtTest/QtTest>
#include <QApplication>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QSignalSpy>
#include "ConfigurationManager.h"
#include "models/ApplicationSettings.h"
#include "models/RecentItem.h"

/**
 * @brief ConfigurationManager单元测试类
 * 
 * 测试ConfigurationManager的配置管理功能：
 * - 配置加载和保存
 * - 服务器URL验证
 * - 最近项目管理
 * - 窗口状态管理
 * - 配置验证
 * - 默认值处理
 */
class TestConfigurationManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基本功能测试
    void testDefaultConfiguration();
    void testLoadSaveConfiguration();
    void testServerUrlValidation();
    void testLanguageSettings();
    
    // 最近项目管理测试
    void testRecentUrlsManagement();
    void testRecentItemsManagement();
    void testMaxRecentItemsLimit();
    
    // 窗口状态测试
    void testWindowGeometry();
    void testWindowMaximizedState();
    void testDarkModeSettings();
    
    // 配置验证测试
    void testConfigurationValidation();
    void testResetToDefaults();
    void testInvalidConfigurations();

private:
    ConfigurationManager* m_configManager;
    QTemporaryDir* m_tempDir;
    QSignalSpy* m_configChangedSpy;
    QSignalSpy* m_serverUrlChangedSpy;
    QSignalSpy* m_languageChangedSpy;
};

void TestConfigurationManager::initTestCase()
{
    qDebug() << "Starting ConfigurationManager unit tests";
    
    // 创建临时目录用于测试
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    // 启用测试模式
    QStandardPaths::setTestModeEnabled(true);
}

void TestConfigurationManager::cleanupTestCase()
{
    delete m_tempDir;
    QStandardPaths::setTestModeEnabled(false);
    qDebug() << "ConfigurationManager unit tests completed";
}

void TestConfigurationManager::init()
{
    m_configManager = new ConfigurationManager(this);
    
    // 创建信号监听器
    m_configChangedSpy = new QSignalSpy(m_configManager, &ConfigurationManager::configurationChanged);
    m_serverUrlChangedSpy = new QSignalSpy(m_configManager, &ConfigurationManager::serverUrlChanged);
    m_languageChangedSpy = new QSignalSpy(m_configManager, &ConfigurationManager::languageChanged);
}

void TestConfigurationManager::cleanup()
{
    delete m_configManager;
    m_configManager = nullptr;
    
    delete m_configChangedSpy;
    delete m_serverUrlChangedSpy;
    delete m_languageChangedSpy;
}

void TestConfigurationManager::testDefaultConfiguration()
{
    // 测试默认配置加载
    auto config = m_configManager->loadConfiguration();
    QVERIFY(config.has_value());
    
    ApplicationSettings settings = config.value();
    
    // 验证默认值
    QCOMPARE(settings.defaultServerUrl, QString("https://meet.jit.si"));
    QCOMPARE(settings.language, QString("auto"));
    QCOMPARE(settings.darkMode, false);
    QCOMPARE(settings.autoJoinAudio, true);
    QCOMPARE(settings.autoJoinVideo, false);
    QCOMPARE(settings.maxRecentItems, 10);
    QCOMPARE(settings.rememberWindowState, true);
    
    // 验证窗口几何
    QVERIFY(settings.windowGeometry.width() >= 800);
    QVERIFY(settings.windowGeometry.height() >= 600);
    
    // 验证配置有效性
    QVERIFY(settings.isValid());
}

void TestConfigurationManager::testLoadSaveConfiguration()
{
    // 创建测试配置
    ApplicationSettings testConfig;
    testConfig.defaultServerUrl = "https://test.example.com";
    testConfig.language = "zh-CN";
    testConfig.darkMode = true;
    testConfig.windowGeometry = QRect(200, 200, 1200, 800);
    testConfig.maximized = true;
    testConfig.maxRecentItems = 20;
    
    // 保存配置
    m_configManager->saveConfiguration(testConfig);
    
    // 验证配置改变信号
    QVERIFY(m_configChangedSpy->count() >= 1);
    
    // 创建新的配置管理器实例来测试加载
    ConfigurationManager* newConfigManager = new ConfigurationManager(this);
    auto loadedConfig = newConfigManager->loadConfiguration();
    
    QVERIFY(loadedConfig.has_value());
    ApplicationSettings loaded = loadedConfig.value();
    
    // 验证加载的配置
    QCOMPARE(loaded.defaultServerUrl, testConfig.defaultServerUrl);
    QCOMPARE(loaded.language, testConfig.language);
    QCOMPARE(loaded.darkMode, testConfig.darkMode);
    QCOMPARE(loaded.windowGeometry, testConfig.windowGeometry);
    QCOMPARE(loaded.maximized, testConfig.maximized);
    QCOMPARE(loaded.maxRecentItems, testConfig.maxRecentItems);
    
    delete newConfigManager;
}

void TestConfigurationManager::testServerUrlValidation()
{
    // 测试有效URL
    QStringList validUrls = {
        "https://meet.jit.si",
        "http://localhost:8080",
        "https://example.com/jitsi",
        "http://192.168.1.100:3000"
    };
    
    for (const QString& url : validUrls) {
        bool result = m_configManager->setServerUrl(url.toStdString());
        QVERIFY(result);
        QCOMPARE(m_configManager->serverUrl(), url);
    }
    
    // 测试无效URL
    QStringList invalidUrls = {
        "",
        "invalid-url",
        "ftp://example.com",
        "meet.jit.si",  // 缺少协议
        "https://"      // 不完整的URL
    };
    
    QString originalUrl = m_configManager->serverUrl();
    for (const QString& url : invalidUrls) {
        bool result = m_configManager->setServerUrl(url.toStdString());
        QVERIFY(!result);
        // 无效URL不应该被设置
        QCOMPARE(m_configManager->serverUrl(), originalUrl);
    }
}

void TestConfigurationManager::testLanguageSettings()
{
    // 测试语言设置
    QString originalLanguage = m_configManager->language();
    
    m_configManager->setLanguage("zh-CN");
    QCOMPARE(m_configManager->language(), QString("zh-CN"));
    QVERIFY(m_languageChangedSpy->count() >= 1);
    
    m_configManager->setLanguage("en-US");
    QCOMPARE(m_configManager->language(), QString("en-US"));
    
    m_configManager->setLanguage("auto");
    QCOMPARE(m_configManager->language(), QString("auto"));
}

void TestConfigurationManager::testRecentUrlsManagement()
{
    // 清空最近URL列表
    m_configManager->clearRecentUrls();
    QVERIFY(m_configManager->recentUrls().isEmpty());
    
    // 添加URL
    QStringList testUrls = {
        "https://meet.jit.si/room1",
        "https://meet.jit.si/room2",
        "https://meet.jit.si/room3"
    };
    
    for (const QString& url : testUrls) {
        m_configManager->addRecentUrl(url);
    }
    
    QStringList recentUrls = m_configManager->recentUrls();
    QCOMPARE(recentUrls.size(), 3);
    
    // 验证顺序（最新的在前）
    QCOMPARE(recentUrls.at(0), testUrls.at(2));
    QCOMPARE(recentUrls.at(1), testUrls.at(1));
    QCOMPARE(recentUrls.at(2), testUrls.at(0));
    
    // 添加重复URL
    m_configManager->addRecentUrl(testUrls.at(1));
    recentUrls = m_configManager->recentUrls();
    QCOMPARE(recentUrls.size(), 3);
    QCOMPARE(recentUrls.at(0), testUrls.at(1)); // 应该移到最前面
}

void TestConfigurationManager::testRecentItemsManagement()
{
    // 清空最近项目列表
    m_configManager->clearRecentItems();
    QVERIFY(m_configManager->recentItems().isEmpty());
    
    // 创建测试项目
    RecentItem item1;
    item1.url = "https://meet.jit.si/room1";
    item1.title = "Test Room 1";
    item1.lastAccessed = QDateTime::currentDateTime();
    
    RecentItem item2;
    item2.url = "https://meet.jit.si/room2";
    item2.title = "Test Room 2";
    item2.lastAccessed = QDateTime::currentDateTime().addSecs(-3600);
    
    // 添加项目
    m_configManager->addRecentItem(item1);
    m_configManager->addRecentItem(item2);
    
    QList<RecentItem> items = m_configManager->recentItems();
    QCOMPARE(items.size(), 2);
    
    // 移除项目
    m_configManager->removeRecentItem(item1.url);
    items = m_configManager->recentItems();
    QCOMPARE(items.size(), 1);
    QCOMPARE(items.at(0).url, item2.url);
}

void TestConfigurationManager::testMaxRecentItemsLimit()
{
    // 设置最大项目数量
    m_configManager->setMaxRecentItems(3);
    QCOMPARE(m_configManager->maxRecentItems(), 3);
    
    // 清空并添加超过限制的项目
    m_configManager->clearRecentUrls();
    
    for (int i = 1; i <= 5; ++i) {
        m_configManager->addRecentUrl(QString("https://meet.jit.si/room%1").arg(i));
    }
    
    QStringList recentUrls = m_configManager->recentUrls();
    QVERIFY(recentUrls.size() <= 3);
}

void TestConfigurationManager::testWindowGeometry()
{
    // 测试窗口几何设置
    QRect testGeometry(300, 300, 1000, 700);
    m_configManager->setWindowGeometry(testGeometry);
    QCOMPARE(m_configManager->windowGeometry(), testGeometry);
    
    // 测试无效几何信息的修正
    QRect invalidGeometry(0, 0, 100, 100); // 太小
    m_configManager->setWindowGeometry(invalidGeometry);
    QRect correctedGeometry = m_configManager->windowGeometry();
    QVERIFY(correctedGeometry.width() >= 800);
    QVERIFY(correctedGeometry.height() >= 600);
}

void TestConfigurationManager::testWindowMaximizedState()
{
    // 测试最大化状态
    m_configManager->setWindowMaximized(true);
    QVERIFY(m_configManager->isWindowMaximized());
    
    m_configManager->setWindowMaximized(false);
    QVERIFY(!m_configManager->isWindowMaximized());
}

void TestConfigurationManager::testDarkModeSettings()
{
    // 测试深色模式设置
    QSignalSpy darkModeSpy(m_configManager, &ConfigurationManager::darkModeChanged);
    
    m_configManager->setDarkMode(true);
    QVERIFY(m_configManager->isDarkMode());
    QVERIFY(darkModeSpy.count() >= 1);
    
    m_configManager->setDarkMode(false);
    QVERIFY(!m_configManager->isDarkMode());
}

void TestConfigurationManager::testConfigurationValidation()
{
    // 测试配置验证
    QVERIFY(m_configManager->validateConfiguration());
    
    // 测试当前配置的有效性
    ApplicationSettings config = m_configManager->currentConfiguration();
    QVERIFY(config.isValid());
    
    // 测试全面验证
    auto [isValid, errors] = m_configManager->performComprehensiveValidation();
    QVERIFY(isValid);
    QVERIFY(errors.isEmpty());
}

void TestConfigurationManager::testResetToDefaults()
{
    // 修改一些设置
    m_configManager->setServerUrl("https://custom.example.com");
    m_configManager->setLanguage("zh-CN");
    m_configManager->setDarkMode(true);
    m_configManager->addRecentUrl("https://meet.jit.si/test");
    
    // 重置为默认值
    m_configManager->resetToDefaults();
    
    // 验证已重置
    ApplicationSettings config = m_configManager->currentConfiguration();
    QCOMPARE(config.defaultServerUrl, QString("https://meet.jit.si"));
    QCOMPARE(config.language, QString("auto"));
    QCOMPARE(config.darkMode, false);
    QVERIFY(config.recentUrls.isEmpty());
}

void TestConfigurationManager::testInvalidConfigurations()
{
    // 测试处理无效配置
    ApplicationSettings invalidConfig;
    invalidConfig.defaultServerUrl = "invalid-url";
    invalidConfig.serverTimeout = -1;
    invalidConfig.windowGeometry = QRect(0, 0, 50, 50);
    
    // 保存无效配置
    m_configManager->saveConfiguration(invalidConfig);
    
    // 加载配置应该返回修正后的有效配置
    auto loadedConfig = m_configManager->loadConfiguration();
    QVERIFY(loadedConfig.has_value());
    
    ApplicationSettings corrected = loadedConfig.value();
    QVERIFY(corrected.isValid());
    
    // 验证无效值被修正
    QVERIFY(corrected.defaultServerUrl != "invalid-url");
    QVERIFY(corrected.serverTimeout > 0);
    QVERIFY(corrected.windowGeometry.width() >= 800);
    QVERIFY(corrected.windowGeometry.height() >= 600);
}

QTEST_MAIN(TestConfigurationManager)
#include "test_unit_configuration_manager.moc"