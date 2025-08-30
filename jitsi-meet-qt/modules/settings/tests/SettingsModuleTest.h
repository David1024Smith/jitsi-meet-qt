#ifndef SETTINGSMODULETEST_H
#define SETTINGSMODULETEST_H

#include <QObject>
#include <QTest>
#include <QVariantMap>
#include <QElapsedTimer>

// Forward declarations
class QApplication;
class QTemporaryDir;
class SettingsManager;
class PreferencesHandler;

class SettingsModuleTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // Basic functionality tests
    void testModuleInitialization();
    void testSettingsManager();
    void testPreferencesHandler();
    void testConfigValidator();
    
    // Storage backend tests
    void testLocalStorage();
    void testCloudStorage();
    void testRegistryStorage();
    
    // Validation tests
    void testConfigValidation();
    void testSchemaValidation();
    
    // UI component tests
    void testSettingsWidget();
    void testPreferencesDialog();
    void testConfigEditor();
    
    // Integration tests
    void testModuleIntegration();
    void testErrorHandling();
    void testPerformance();
    
    // Additional comprehensive tests
    void testSettingsStorageAndSync();
    void testConfigValidationAndDefaults();
    void testUIComponentInteraction();
    void testCompatibilityWithExistingConfigurationManager();

private:
    // Test helper methods
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    void setupTestData();
    void cleanupTestData();
    
    // Test data and utilities
    QApplication* m_app = nullptr;
    QTemporaryDir* m_tempDir = nullptr;
    SettingsManager* m_settingsManager = nullptr;
    PreferencesHandler* m_preferencesHandler = nullptr;
    QString m_testDataPath;
    QVariantMap m_testSettings;
};

#endif // SETTINGSMODULETEST_H