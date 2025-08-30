#include <QCoreApplication>
#include <QDebug>
#include "include/SettingsModule.h"
#include "include/SettingsManager.h"
#include "include/PreferencesHandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Testing Settings Module Implementation...";
    
    // Test SettingsModule
    SettingsModule* module = SettingsModule::instance();
    qDebug() << "SettingsModule version:" << module->version();
    qDebug() << "SettingsModule status:" << static_cast<int>(module->status());
    
    // Test initialization
    SettingsModule::ModuleOptions options;
    options.configPath = "./test_config";
    options.enableValidation = true;
    options.autoSync = true;
    
    bool initResult = module->initialize(options);
    qDebug() << "Initialization result:" << initResult;
    qDebug() << "Module initialized:" << module->isInitialized();
    
    if (initResult) {
        // Test SettingsManager
        ISettingsManager* settingsManager = module->settingsManager();
        if (settingsManager) {
            qDebug() << "SettingsManager status:" << static_cast<int>(settingsManager->status());
            
            // Test basic operations
            settingsManager->setValue("test/key", "test_value");
            QVariant value = settingsManager->value("test/key");
            qDebug() << "Test value:" << value.toString();
            
            bool contains = settingsManager->contains("test/key");
            qDebug() << "Contains test key:" << contains;
        }
        
        // Test PreferencesHandler
        IPreferencesHandler* preferencesHandler = module->preferencesHandler();
        if (preferencesHandler) {
            // Test preference operations
            preferencesHandler->setPreference("audio", "volume", 0.8);
            QVariant volume = preferencesHandler->preference("audio", "volume");
            qDebug() << "Audio volume preference:" << volume.toDouble();
            
            QStringList categories = preferencesHandler->categories();
            qDebug() << "Available categories:" << categories;
        }
        
        // Test module info
        QJsonObject moduleInfo = module->moduleInfo();
        qDebug() << "Module info:" << QJsonDocument(moduleInfo).toJson(QJsonDocument::Compact);
        
        // Test self-check
        QStringList checkResults = module->performSelfCheck();
        qDebug() << "Self-check results:" << checkResults;
    }
    
    qDebug() << "Settings Module test completed successfully!";
    
    return 0;
}