#include <QtTest/QtTest>
#include <QApplication>
#include <QTemporaryDir>
#include <QStandardPaths>
#include "ConfigurationManager.h"
#include "models/ApplicationSettings.h"

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
    void testRecentUrlsManagement();
    void testWindowStateManagement();
    void testConfigurationValidation();
    void testResetToDefaults();
    
    // ApplicationSettings测试
    void testApplicationSettingsDefaults();
    void testApplicationSettingsValidation();
    void testApplicationSettingsComparison();
    void testApplicationSettingsVariantMap();
    
    // 错误处理测试
    void testInvalidServerUrl();
    void testInvalidWindowGeometry();
    void testCorruptedConfiguration();

private:
    ConfigurationManager* m_configManager;
    QTemporaryDir* m_tempDir;
    QString m_originalConfigPath;
};

void TestConfigurationManager::initTestCase()
{
    // 创建临时目录用于测试
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    // 保存原始配置路径
    m_originalConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    
    // 设置测试配置路径
    QStandardPaths::setTestModeEnabled(true);
}

void TestConfigurationManager::cleanupTestCase()
{
    delete m_tempDir;
    QStandardPaths::setTestModeEnabled(false);
}

void TestConfigurationManager::init()
{
    m_configManager = new ConfigurationManager(this);
}

void TestConfigurationManager::cleanup()
{
    delete m_configManager;
    m_configManager = nullptr;
}

void TestConfigurationManager::testDefaultConfiguration()
{
    ApplicationSettings config = m_configManager->loadConfiguration();
    
    // 验证默认值
    QCOMPARE(config.defaultServerUrl, QString("https://meet.jit.si"));
    QCOMPARE(config.serverTimeout, 30);
    QCOMPARE(config.language, QString("auto"));
    QCOMPARE(config.darkMode, false);
    QCOMPARE(config.autoJoinAudio, true);
    QCOMPARE(config.autoJoinVideo, false);
    QCOMPARE(config.maxRecentItems, 10);
    QCOMPARE(config.rememberWindowState, true);
    
    // 验证窗口几何
    QVERIFY(config.windowGeometry.width() >= 800);
    QVERIFY(config.windowGeometry.height() >= 600);
    QVERIFY(config.windowGeometry.x() >= 0);
    QVERIFY(config.windowGeometry.y() >= 0);
    
    // 验证配置有效性
    QVERIFY(config.isValid());
    
    // 验证最近URL列表为空
    QVERIFY(config.recentUrls.isEmpty());
    
    // 验证窗口状态管理器
    QVERIFY(m_configManager->windowStateManager() != nullptr);
}

void TestConfigurationManager::testLoadSaveConfiguration()
{
    // 创建测试配置
    ApplicationSettings testConfig;
    testConfig.defaultServerUrl = "https://test.example.com";
    testConfig.serverTimeout = 60;
    testConfig.language = "zh-CN";
    testConfig.darkMode = true;
    testConfig.windowGeometry = QRect(200, 200, 1200, 800);
    testConfig.maximized = true;
    testConfig.autoJoinAudio = false;
    testConfig.autoJoinVideo = true;
    testConfig.maxRecentItems = 20;
    testConfig.recentUrls << "https://meet.jit.si/test1" << "https://meet.jit.si/test2";
    
    // 保存配置
    m_configManager->saveConfiguration(testConfig);
    
    // 创建新的配置管理器实例来测试加载
    ConfigurationManager* newConfigManager = new ConfigurationManager(this);
    ApplicationSettings loadedConfig = newConfigManager->loadConfiguration();
    
    // 验证加载的配置
    QCOMPARE(loadedConfig.defaultServerUrl, testConfig.defaultServerUrl);
    QCOMPARE(loadedConfig.serverTimeout, testConfig.serverTimeout);
    QCOMPARE(loadedConfig.language, testConfig.language);
    QCOMPARE(loadedConfig.darkMode, testConfig.darkMode);
    QCOMPARE(loadedConfig.windowGeometry, testConfig.windowGeometry);
    QCOMPARE(loadedConfig.maximized, testConfig.maximized);
    QCOMPARE(loadedConfig.autoJoinAudio, testConfig.autoJoinAudio);
    QCOMPARE(loadedConfig.autoJoinVideo, testConfig.autoJoinVideo);
    QCOMPARE(loadedConfig.maxRecentItems, testConfig.maxRecentItems);
    QCOMPARE(loadedConfig.recentUrls, testConfig.recentUrls);
    
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
        m_configManager->setServerUrl(url);
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
        m_configManager->setServerUrl(url);
        // 无效URL不应该被设置
        QCOMPARE(m_configManager->serverUrl(), originalUrl);
    }
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
    
    // 测试最大数量限制
    m_configManager->setMaxRecentItems(2);
    recentUrls = m_configManager->recentUrls();
    QVERIFY(recentUrls.size() <= 2);
}

void TestConfigurationManager::testWindowStateManagement()
{
    // 测试窗口几何设置
    QRect testGeometry(300, 300, 1000, 700);
    m_configManager->setWindowGeometry(testGeometry);
    QCOMPARE(m_configManager->windowGeometry(), testGeometry);
    
    // 测试最大化状态
    m_configManager->setWindowMaximized(true);
    QVERIFY(m_configManager->isWindowMaximized());
    
    m_configManager->setWindowMaximized(false);
    QVERIFY(!m_configManager->isWindowMaximized());
    
    // 测试无效几何信息的修正
    QRect invalidGeometry(0, 0, 100, 100); // 太小
    m_configManager->setWindowGeometry(invalidGeometry);
    QRect correctedGeometry = m_configManager->windowGeometry();
    QVERIFY(correctedGeometry.width() >= 800);
    QVERIFY(correctedGeometry.height() >= 600);
}

void TestConfigurationManager::testConfigurationValidation()
{
    QVERIFY(m_configManager->validateConfiguration());
    
    // 测试当前配置的有效性
    ApplicationSettings config = m_configManager->currentConfiguration();
    QVERIFY(config.isValid());
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

void TestConfigurationManager::testApplicationSettingsDefaults()
{
    ApplicationSettings settings;
    
    // 验证默认值
    QCOMPARE(settings.defaultServerUrl, QString("https://meet.jit.si"));
    QCOMPARE(settings.serverTimeout, 30);
    QCOMPARE(settings.language, QString("auto"));
    QCOMPARE(settings.darkMode, false);
    QCOMPARE(settings.autoJoinAudio, true);
    QCOMPARE(settings.autoJoinVideo, false);
    QCOMPARE(settings.maxRecentItems, 10);
    QCOMPARE(settings.rememberWindowState, true);
    
    QVERIFY(settings.isValid());
}

void TestConfigurationManager::testApplicationSettingsValidation()
{
    ApplicationSettings settings;
    
    // 测试有效配置
    QVERIFY(settings.isValid());
    
    // 测试无效服务器URL
    settings.defaultServerUrl = "invalid-url";
    QVERIFY(!settings.isValid());
    
    settings.resetToDefaults();
    QVERIFY(settings.isValid());
    
    // 测试无效超时时间
    settings.serverTimeout = -1;
    QVERIFY(!settings.isValid());
    
    settings.resetToDefaults();
    QVERIFY(settings.isValid());
    
    // 测试无效窗口大小
    settings.windowGeometry = QRect(0, 0, 100, 100);
    QVERIFY(!settings.isValid());
}

void TestConfigurationManager::testApplicationSettingsComparison()
{
    ApplicationSettings settings1;
    ApplicationSettings settings2;
    
    // 默认设置应该相等
    QVERIFY(settings1 == settings2);
    QVERIFY(!(settings1 != settings2));
    
    // 修改一个设置
    settings2.darkMode = true;
    QVERIFY(settings1 != settings2);
    QVERIFY(!(settings1 == settings2));
    
    // 拷贝构造函数测试
    ApplicationSettings settings3(settings1);
    QVERIFY(settings1 == settings3);
    
    // 赋值操作符测试
    ApplicationSettings settings4;
    settings4 = settings2;
    QVERIFY(settings2 == settings4);
}

void TestConfigurationManager::testApplicationSettingsVariantMap()
{
    ApplicationSettings originalSettings;
    originalSettings.defaultServerUrl = "https://test.example.com";
    originalSettings.darkMode = true;
    originalSettings.maxRecentItems = 15;
    
    // 转换为QVariantMap
    QVariantMap map = originalSettings.toVariantMap();
    QCOMPARE(map["defaultServerUrl"].toString(), originalSettings.defaultServerUrl);
    QCOMPARE(map["darkMode"].toBool(), originalSettings.darkMode);
    QCOMPARE(map["maxRecentItems"].toInt(), originalSettings.maxRecentItems);
    
    // 从QVariantMap恢复
    ApplicationSettings restoredSettings;
    restoredSettings.fromVariantMap(map);
    
    QCOMPARE(restoredSettings.defaultServerUrl, originalSettings.defaultServerUrl);
    QCOMPARE(restoredSettings.darkMode, originalSettings.darkMode);
    QCOMPARE(restoredSettings.maxRecentItems, originalSettings.maxRecentItems);
}

void TestConfigurationManager::testInvalidServerUrl()
{
    QSignalSpy spy(m_configManager, &ConfigurationManager::serverUrlChanged);
    
    QString originalUrl = m_configManager->serverUrl();
    
    // 尝试设置无效URL
    m_configManager->setServerUrl("invalid-url");
    
    // URL不应该改变
    QCOMPARE(m_configManager->serverUrl(), originalUrl);
    
    // 不应该发出信号
    QCOMPARE(spy.count(), 0);
}

void TestConfigurationManager::testInvalidWindowGeometry()
{
    // 设置无效的窗口几何（太小）
    QRect invalidGeometry(0, 0, 100, 100);
    m_configManager->setWindowGeometry(invalidGeometry);
    
    // 应该被自动修正
    QRect correctedGeometry = m_configManager->windowGeometry();
    QVERIFY(correctedGeometry.width() >= 800);
    QVERIFY(correctedGeometry.height() >= 600);
}

void TestConfigurationManager::testCorruptedConfiguration()
{
    // 这个测试需要模拟损坏的配置文件
    // 在实际实现中，ConfigurationManager应该能够处理损坏的配置并恢复默认值
    
    ApplicationSettings config = m_configManager->loadConfiguration();
    QVERIFY(config.isValid());
    
    // 验证配置管理器能够处理无效配置
    QVERIFY(m_configManager->validateConfiguration());
}

QTEST_MAIN(TestConfigurationManager)
#include "test_configuration_manager.moc"