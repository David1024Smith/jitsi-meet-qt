#include "SettingsModuleTest.h"
#include "SettingsModule.h"
#include "SettingsManager.h"
#include "PreferencesHandler.h"
#include "SettingsConfig.h"
#include "LocalStorage.h"
#include "CloudStorage.h"
#include "RegistryStorage.h"
#include "ConfigValidator.h"
#include "SchemaValidator.h"
#include "SettingsWidget.h"
#include "PreferencesDialog.h"
#include "ConfigEditor.h"

#include <QApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QThread>
#include <QStandardPaths>

void SettingsModuleTest::initTestCase()
{
    // Initialize test environment
    setupTestEnvironment();
    
    // Ensure we have a QApplication instance for widget tests
    if (!QApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        m_app = new QApplication(argc, argv);
    }
}

void SettingsModuleTest::cleanupTestCase()
{
    // Cleanup test environment
    cleanupTestEnvironment();
    
    if (m_app) {
        delete m_app;
        m_app = nullptr;
    }
}

void SettingsModuleTest::init()
{
    // Setup for each test
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    // Create test settings manager
    m_settingsManager = new SettingsManager(this);
    m_settingsManager->setConfigPath(m_tempDir->path() + "/test_settings.ini");
    
    // Create test preferences handler
    m_preferencesHandler = new PreferencesHandler(this);
    
    // Initialize test data
    setupTestData();
}

void SettingsModuleTest::cleanup()
{
    // Cleanup after each test
    if (m_settingsManager) {
        delete m_settingsManager;
        m_settingsManager = nullptr;
    }
    
    if (m_preferencesHandler) {
        delete m_preferencesHandler;
        m_preferencesHandler = nullptr;
    }
    
    if (m_tempDir) {
        delete m_tempDir;
        m_tempDir = nullptr;
    }
    
    cleanupTestData();
}

void SettingsModuleTest::testModuleInitialization()
{
    // Test module initialization
    SettingsModule* module = SettingsModule::instance();
    QVERIFY(module != nullptr);
    
    // Test initialization
    bool result = module->initialize();
    QVERIFY(result == true);
    QVERIFY(module->isInitialized() == true);
    
    // Test double initialization
    bool secondInit = module->initialize();
    QVERIFY(secondInit == true); // Should handle gracefully
    
    // Test module status
    QCOMPARE(module->status(), SettingsModule::Ready);
}

void SettingsModuleTest::testSettingsManager()
{
    // Test settings manager initialization
    QVERIFY(m_settingsManager->initialize());
    QCOMPARE(m_settingsManager->status(), ISettingsManager::Ready);
    
    // Test basic value operations
    m_settingsManager->setValue("test/string", "test_value");
    QCOMPARE(m_settingsManager->value("test/string").toString(), QString("test_value"));
    
    m_settingsManager->setValue("test/int", 42);
    QCOMPARE(m_settingsManager->value("test/int").toInt(), 42);
    
    m_settingsManager->setValue("test/bool", true);
    QCOMPARE(m_settingsManager->value("test/bool").toBool(), true);
    
    // Test contains functionality
    QVERIFY(m_settingsManager->contains("test/string"));
    QVERIFY(!m_settingsManager->contains("nonexistent/key"));
    
    // Test default values
    QCOMPARE(m_settingsManager->value("nonexistent/key", "default").toString(), QString("default"));
    
    // Test key enumeration
    QStringList keys = m_settingsManager->allKeys();
    QVERIFY(keys.contains("test/string"));
    QVERIFY(keys.contains("test/int"));
    QVERIFY(keys.contains("test/bool"));
    
    // Test child keys and groups
    QStringList childKeys = m_settingsManager->childKeys("test");
    QVERIFY(childKeys.contains("string"));
    QVERIFY(childKeys.contains("int"));
    QVERIFY(childKeys.contains("bool"));
    
    // Test removal
    m_settingsManager->remove("test/string");
    QVERIFY(!m_settingsManager->contains("test/string"));
    
    // Test sync functionality
    QVERIFY(m_settingsManager->sync());
}

void SettingsModuleTest::testPreferencesHandler()
{
    // Test preferences handler initialization
    QVERIFY(m_preferencesHandler->initialize());
    
    // Test setting preferences by category
    m_preferencesHandler->setPreference(IPreferencesHandler::AudioPreferences, "volume", 75);
    m_preferencesHandler->setPreference(IPreferencesHandler::VideoPreferences, "resolution", "1920x1080");
    m_preferencesHandler->setPreference("UI", "theme", "dark");
    
    // Test getting preferences
    QCOMPARE(m_preferencesHandler->preference(IPreferencesHandler::AudioPreferences, "volume").toInt(), 75);
    QCOMPARE(m_preferencesHandler->preference(IPreferencesHandler::VideoPreferences, "resolution").toString(), QString("1920x1080"));
    QCOMPARE(m_preferencesHandler->preference("UI", "theme").toString(), QString("dark"));
    
    // Test default values
    QCOMPARE(m_preferencesHandler->preference("UI", "nonexistent", "default").toString(), QString("default"));
    
    // Test categories and keys
    QStringList categories = m_preferencesHandler->categories();
    QVERIFY(categories.contains("Audio"));
    QVERIFY(categories.contains("Video"));
    QVERIFY(categories.contains("UI"));
    
    QStringList audioKeys = m_preferencesHandler->keys("Audio");
    QVERIFY(audioKeys.contains("volume"));
    
    // Test preference status
    QCOMPARE(m_preferencesHandler->preferenceStatus("Audio", "volume"), IPreferencesHandler::Modified);
    
    // Test category preferences
    QVariantMap audioPrefs = m_preferencesHandler->categoryPreferences("Audio");
    QVERIFY(audioPrefs.contains("volume"));
    QCOMPARE(audioPrefs["volume"].toInt(), 75);
    
    // Test JSON export/import
    QJsonObject exportedJson = m_preferencesHandler->exportToJson();
    QVERIFY(!exportedJson.isEmpty());
    
    // Reset and import
    m_preferencesHandler->resetAll();
    QVERIFY(m_preferencesHandler->importFromJson(exportedJson));
    QCOMPARE(m_preferencesHandler->preference("Audio", "volume").toInt(), 75);
}

void SettingsModuleTest::testConfigValidator()
{
    ConfigValidator validator;
    
    // Test basic validation rules
    validator.addRule("audio/volume", ConfigValidator::IntegerRange, QVariantList() << 0 << 100);
    validator.addRule("video/resolution", ConfigValidator::StringPattern, "\\d+x\\d+");
    validator.addRule("network/port", ConfigValidator::IntegerRange, QVariantList() << 1024 << 65535);
    
    // Test valid configurations
    QVariantMap validConfig;
    validConfig["audio/volume"] = 50;
    validConfig["video/resolution"] = "1920x1080";
    validConfig["network/port"] = 8080;
    
    QVERIFY(validator.validate(validConfig));
    
    // Test invalid configurations
    QVariantMap invalidConfig;
    invalidConfig["audio/volume"] = 150; // Out of range
    invalidConfig["video/resolution"] = "invalid"; // Wrong pattern
    invalidConfig["network/port"] = 80; // Out of range
    
    QVERIFY(!validator.validate(invalidConfig));
    
    QStringList errors = validator.lastErrors();
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("audio/volume"));
    QVERIFY(errors.join(" ").contains("video/resolution"));
    QVERIFY(errors.join(" ").contains("network/port"));
}

void SettingsModuleTest::testLocalStorage()
{
    LocalStorage storage;
    storage.setFilePath(m_tempDir->path() + "/local_test.ini");
    
    QVERIFY(storage.initialize());
    
    // Test storage operations
    storage.setValue("test/key1", "value1");
    storage.setValue("test/key2", 42);
    
    QVERIFY(storage.sync());
    
    // Test retrieval
    QCOMPARE(storage.value("test/key1").toString(), QString("value1"));
    QCOMPARE(storage.value("test/key2").toInt(), 42);
    
    // Test persistence by creating new instance
    LocalStorage storage2;
    storage2.setFilePath(m_tempDir->path() + "/local_test.ini");
    QVERIFY(storage2.initialize());
    
    QCOMPARE(storage2.value("test/key1").toString(), QString("value1"));
    QCOMPARE(storage2.value("test/key2").toInt(), 42);
}

void SettingsModuleTest::testCloudStorage()
{
    CloudStorage storage;
    
    // Test initialization (should work in offline mode)
    QVERIFY(storage.initialize());
    
    // Test local caching
    storage.setValue("test/cloud_key", "cloud_value");
    QCOMPARE(storage.value("test/cloud_key").toString(), QString("cloud_value"));
    
    // Test sync status
    QCOMPARE(storage.syncStatus(), CloudStorage::Offline);
    
    // Test offline operations
    storage.setValue("test/offline_key", "offline_value");
    QVERIFY(storage.contains("test/offline_key"));
}

void SettingsModuleTest::testRegistryStorage()
{
#ifdef Q_OS_WIN
    RegistryStorage storage;
    storage.setRegistryPath("HKEY_CURRENT_USER/Software/JitsiMeetQt/Test");
    
    QVERIFY(storage.initialize());
    
    // Test registry operations
    storage.setValue("test_key", "test_value");
    QCOMPARE(storage.value("test_key").toString(), QString("test_value"));
    
    // Test cleanup
    storage.remove("test_key");
    QVERIFY(!storage.contains("test_key"));
#else
    QSKIP("Registry storage is Windows-only");
#endif
}

void SettingsModuleTest::testConfigValidation()
{
    SettingsConfig config;
    
    // Test default configuration
    QVERIFY(config.validate());
    
    // Test configuration loading
    QVariantMap testConfig;
    testConfig["audio/enabled"] = true;
    testConfig["audio/volume"] = 75;
    testConfig["video/enabled"] = true;
    testConfig["video/resolution"] = "1920x1080";
    
    config.fromVariantMap(testConfig);
    QVERIFY(config.validate());
    
    // Test invalid configuration
    testConfig["audio/volume"] = -10; // Invalid value
    config.fromVariantMap(testConfig);
    QVERIFY(!config.validate());
}

void SettingsModuleTest::testSchemaValidation()
{
    SchemaValidator validator;
    
    // Create test schema
    QJsonObject schema;
    schema["type"] = "object";
    
    QJsonObject properties;
    QJsonObject audioProperty;
    audioProperty["type"] = "object";
    QJsonObject audioProps;
    QJsonObject volumeProperty;
    volumeProperty["type"] = "integer";
    volumeProperty["minimum"] = 0;
    volumeProperty["maximum"] = 100;
    audioProps["volume"] = volumeProperty;
    audioProperty["properties"] = audioProps;
    properties["audio"] = audioProperty;
    schema["properties"] = properties;
    
    QVERIFY(validator.setSchema(schema));
    
    // Test valid data
    QJsonObject validData;
    QJsonObject audioData;
    audioData["volume"] = 50;
    validData["audio"] = audioData;
    
    QVERIFY(validator.validate(validData));
    
    // Test invalid data
    QJsonObject invalidData;
    QJsonObject invalidAudioData;
    invalidAudioData["volume"] = 150; // Out of range
    invalidData["audio"] = invalidAudioData;
    
    QVERIFY(!validator.validate(invalidData));
}

void SettingsModuleTest::testSettingsWidget()
{
    SettingsWidget widget;
    
    // Test widget initialization
    QVERIFY(widget.initialize());
    
    // Test setting values
    widget.setValue("audio/volume", 75);
    QCOMPARE(widget.value("audio/volume").toInt(), 75);
    
    // Test signal emission
    QSignalSpy spy(&widget, &SettingsWidget::valueChanged);
    widget.setValue("test/key", "test_value");
    QCOMPARE(spy.count(), 1);
    
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QString("test/key"));
    QCOMPARE(arguments.at(1).toString(), QString("test_value"));
    
    // Test widget validation
    QVERIFY(widget.validate());
}

void SettingsModuleTest::testPreferencesDialog()
{
    PreferencesDialog dialog;
    
    // Test dialog initialization
    QVERIFY(dialog.initialize());
    
    // Test category management
    dialog.addCategory("Audio", "Audio Settings");
    dialog.addCategory("Video", "Video Settings");
    
    QStringList categories = dialog.categories();
    QVERIFY(categories.contains("Audio"));
    QVERIFY(categories.contains("Video"));
    
    // Test preference setting
    dialog.setPreference("Audio", "volume", 75);
    QCOMPARE(dialog.preference("Audio", "volume").toInt(), 75);
    
    // Test dialog validation
    QVERIFY(dialog.validate());
}

void SettingsModuleTest::testConfigEditor()
{
    ConfigEditor editor;
    
    // Test editor initialization
    QVERIFY(editor.initialize());
    
    // Test configuration loading
    QVariantMap config;
    config["audio/volume"] = 75;
    config["video/resolution"] = "1920x1080";
    
    editor.loadConfiguration(config);
    
    // Test configuration retrieval
    QVariantMap loadedConfig = editor.configuration();
    QCOMPARE(loadedConfig["audio/volume"].toInt(), 75);
    QCOMPARE(loadedConfig["video/resolution"].toString(), QString("1920x1080"));
    
    // Test modification
    editor.setValue("audio/volume", 80);
    QCOMPARE(editor.value("audio/volume").toInt(), 80);
    
    // Test validation
    QVERIFY(editor.validate());
}

void SettingsModuleTest::testModuleIntegration()
{
    // Test integration between components
    SettingsModule* module = SettingsModule::instance();
    QVERIFY(module->initialize());
    
    SettingsManager* manager = module->settingsManager();
    QVERIFY(manager != nullptr);
    
    PreferencesHandler* handler = module->preferencesHandler();
    QVERIFY(handler != nullptr);
    
    // Test cross-component communication
    QSignalSpy managerSpy(manager, &ISettingsManager::valueChanged);
    QSignalSpy handlerSpy(handler, &IPreferencesHandler::preferenceChanged);
    
    manager->setValue("test/integration", "value");
    handler->setPreference("Test", "integration", "preference_value");
    
    QCOMPARE(managerSpy.count(), 1);
    QCOMPARE(handlerSpy.count(), 1);
    
    // Test data consistency
    QCOMPARE(manager->value("test/integration").toString(), QString("value"));
    QCOMPARE(handler->preference("Test", "integration").toString(), QString("preference_value"));
}

void SettingsModuleTest::testErrorHandling()
{
    // Test invalid file path
    SettingsManager manager;
    manager.setConfigPath("/invalid/path/settings.ini");
    
    QSignalSpy errorSpy(&manager, &ISettingsManager::errorOccurred);
    bool result = manager.initialize();
    
    // Should handle error gracefully
    QVERIFY(!result || errorSpy.count() > 0);
    
    // Test invalid configuration
    ConfigValidator validator;
    validator.addRule("test/key", ConfigValidator::IntegerRange, QVariantList() << 0 << 100);
    
    QVariantMap invalidConfig;
    invalidConfig["test/key"] = "invalid_string"; // Should be integer
    
    QVERIFY(!validator.validate(invalidConfig));
    QVERIFY(!validator.lastErrors().isEmpty());
    
    // Test network error simulation for cloud storage
    CloudStorage storage;
    storage.setServerUrl("http://invalid.server.url");
    
    QSignalSpy cloudErrorSpy(&storage, &CloudStorage::errorOccurred);
    storage.initialize();
    storage.sync(); // Should trigger network error
    
    // Allow some time for network timeout
    QTest::qWait(1000);
}

void SettingsModuleTest::testPerformance()
{
    // Test large configuration performance
    SettingsManager manager;
    manager.setConfigPath(m_tempDir->path() + "/perf_test.ini");
    QVERIFY(manager.initialize());
    
    // Measure bulk operations
    QElapsedTimer timer;
    timer.start();
    
    // Write 1000 settings
    for (int i = 0; i < 1000; ++i) {
        manager.setValue(QString("perf/key_%1").arg(i), QString("value_%1").arg(i));
    }
    
    qint64 writeTime = timer.elapsed();
    QVERIFY(writeTime < 5000); // Should complete within 5 seconds
    
    timer.restart();
    
    // Read 1000 settings
    for (int i = 0; i < 1000; ++i) {
        QString value = manager.value(QString("perf/key_%1").arg(i)).toString();
        QCOMPARE(value, QString("value_%1").arg(i));
    }
    
    qint64 readTime = timer.elapsed();
    QVERIFY(readTime < 2000); // Should complete within 2 seconds
    
    // Test sync performance
    timer.restart();
    QVERIFY(manager.sync());
    qint64 syncTime = timer.elapsed();
    QVERIFY(syncTime < 3000); // Should complete within 3 seconds
    
    // Test memory usage (basic check)
    QStringList allKeys = manager.allKeys();
    QVERIFY(allKeys.size() >= 1000);
}

void SettingsModuleTest::testSettingsStorageAndSync()
{
    // Test storage and synchronization functionality
    SettingsManager manager1;
    manager1.setConfigPath(m_tempDir->path() + "/sync_test.ini");
    QVERIFY(manager1.initialize());
    
    // Set values in first manager
    manager1.setValue("sync/test1", "value1");
    manager1.setValue("sync/test2", 42);
    QVERIFY(manager1.sync());
    
    // Create second manager with same file
    SettingsManager manager2;
    manager2.setConfigPath(m_tempDir->path() + "/sync_test.ini");
    QVERIFY(manager2.initialize());
    
    // Verify values are synchronized
    QCOMPARE(manager2.value("sync/test1").toString(), QString("value1"));
    QCOMPARE(manager2.value("sync/test2").toInt(), 42);
    
    // Test automatic sync
    manager1.setSyncStrategy(SettingsManager::OnChange);
    QSignalSpy syncSpy(&manager1, &ISettingsManager::syncCompleted);
    
    manager1.setValue("sync/auto", "auto_value");
    
    // Wait for automatic sync
    QVERIFY(syncSpy.wait(1000));
    QCOMPARE(syncSpy.count(), 1);
}

void SettingsModuleTest::testConfigValidationAndDefaults()
{
    // Test configuration validation and default values
    SettingsConfig config;
    
    // Test default values
    QVERIFY(config.hasDefaultValue("audio/volume"));
    QCOMPARE(config.defaultValue("audio/volume").toInt(), 50);
    
    QVERIFY(config.hasDefaultValue("video/enabled"));
    QCOMPARE(config.defaultValue("video/enabled").toBool(), true);
    
    // Test validation with defaults
    QVariantMap emptyConfig;
    config.fromVariantMap(emptyConfig);
    config.applyDefaults();
    QVERIFY(config.validate());
    
    // Test custom validation rules
    config.addValidationRule("audio/volume", [](const QVariant& value) {
        int vol = value.toInt();
        return vol >= 0 && vol <= 100;
    });
    
    config.setValue("audio/volume", 75);
    QVERIFY(config.validate());
    
    config.setValue("audio/volume", 150);
    QVERIFY(!config.validate());
}

void SettingsModuleTest::testUIComponentInteraction()
{
    // Test UI component interactions
    SettingsWidget widget;
    PreferencesDialog dialog;
    ConfigEditor editor;
    
    QVERIFY(widget.initialize());
    QVERIFY(dialog.initialize());
    QVERIFY(editor.initialize());
    
    // Test widget-dialog interaction
    QSignalSpy widgetSpy(&widget, &SettingsWidget::valueChanged);
    QSignalSpy dialogSpy(&dialog, &PreferencesDialog::preferenceChanged);
    
    widget.setValue("ui/theme", "dark");
    dialog.setPreference("UI", "theme", "light");
    
    QCOMPARE(widgetSpy.count(), 1);
    QCOMPARE(dialogSpy.count(), 1);
    
    // Test editor configuration sync
    QVariantMap config;
    config["ui/theme"] = "dark";
    config["audio/volume"] = 75;
    
    editor.loadConfiguration(config);
    QVariantMap loadedConfig = editor.configuration();
    
    QCOMPARE(loadedConfig["ui/theme"].toString(), QString("dark"));
    QCOMPARE(loadedConfig["audio/volume"].toInt(), 75);
    
    // Test validation across components
    QVERIFY(widget.validate());
    QVERIFY(dialog.validate());
    QVERIFY(editor.validate());
}

void SettingsModuleTest::testCompatibilityWithExistingConfigurationManager()
{
    // Test compatibility with existing ConfigurationManager
    // This would test integration with legacy code
    
    SettingsModule* module = SettingsModule::instance();
    QVERIFY(module->initialize());
    
    // Test legacy API compatibility
    SettingsManager* manager = module->settingsManager();
    
    // Simulate legacy configuration format
    QVariantMap legacyConfig;
    legacyConfig["JitsiMeet/Audio/InputDevice"] = "default";
    legacyConfig["JitsiMeet/Audio/OutputDevice"] = "default";
    legacyConfig["JitsiMeet/Video/Camera"] = "default";
    legacyConfig["JitsiMeet/Network/ServerUrl"] = "https://meet.jit.si";
    
    // Test importing legacy configuration
    QString tempFile = m_tempDir->path() + "/legacy_config.ini";
    QSettings legacySettings(tempFile, QSettings::IniFormat);
    
    for (auto it = legacyConfig.begin(); it != legacyConfig.end(); ++it) {
        legacySettings.setValue(it.key(), it.value());
    }
    legacySettings.sync();
    
    // Import into new settings manager
    QVERIFY(manager->importSettings(tempFile));
    
    // Verify values are accessible through new API
    QCOMPARE(manager->value("JitsiMeet/Audio/InputDevice").toString(), QString("default"));
    QCOMPARE(manager->value("JitsiMeet/Network/ServerUrl").toString(), QString("https://meet.jit.si"));
    
    // Test that new settings work alongside legacy ones
    manager->setValue("NewModule/TestSetting", "test_value");
    QCOMPARE(manager->value("NewModule/TestSetting").toString(), QString("test_value"));
    
    // Verify legacy settings are still accessible
    QCOMPARE(manager->value("JitsiMeet/Audio/InputDevice").toString(), QString("default"));
}

void SettingsModuleTest::setupTestEnvironment()
{
    // Setup test environment
    m_testDataPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/SettingsModuleTest";
    QDir().mkpath(m_testDataPath);
}

void SettingsModuleTest::cleanupTestEnvironment()
{
    // Cleanup test environment
    if (!m_testDataPath.isEmpty()) {
        QDir(m_testDataPath).removeRecursively();
    }
}

void SettingsModuleTest::setupTestData()
{
    // Setup test data for each test
    m_testSettings.clear();
    m_testSettings["audio/volume"] = 50;
    m_testSettings["audio/enabled"] = true;
    m_testSettings["video/resolution"] = "1920x1080";
    m_testSettings["video/enabled"] = true;
    m_testSettings["network/server"] = "https://meet.jit.si";
    m_testSettings["ui/theme"] = "default";
}

void SettingsModuleTest::cleanupTestData()
{
    // Cleanup test data after each test
    m_testSettings.clear();
}

QTEST_MAIN(SettingsModuleTest)