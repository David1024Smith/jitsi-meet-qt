#include <QApplication>
#include <QDebug>
#include "SettingsModule.h"
#include "SettingsManager.h"

/**
 * @brief Basic Settings Example
 * 
 * This example demonstrates the basic usage of the Settings Module,
 * including initialization, setting and getting values, and cleanup.
 */

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== Basic Settings Example ===";
    
    // Get the settings module instance
    SettingsModule* module = SettingsModule::instance();
    
    // Initialize the module with default options
    SettingsModule::ModuleOptions options;
    options.configPath = "example_settings.json";
    options.enableValidation = true;
    options.autoSync = true;
    
    if (!module->initialize(options)) {
        qCritical() << "Failed to initialize Settings Module";
        return 1;
    }
    
    qDebug() << "Settings Module initialized successfully";
    qDebug() << "Version:" << module->version();
    
    // Get the settings manager
    SettingsManager* settingsManager = module->settingsManager();
    if (!settingsManager) {
        qCritical() << "Failed to get Settings Manager";
        return 1;
    }
    
    // Set some example values
    qDebug() << "\n--- Setting Values ---";
    settingsManager->setValue("audio/volume", 0.8);
    settingsManager->setValue("audio/muted", false);
    settingsManager->setValue("video/resolution", QSize(1920, 1080));
    settingsManager->setValue("ui/theme", "dark");
    settingsManager->setValue("network/timeout", 5000);
    
    // Get values back
    qDebug() << "\n--- Getting Values ---";
    qDebug() << "Audio volume:" << settingsManager->value("audio/volume").toDouble();
    qDebug() << "Audio muted:" << settingsManager->value("audio/muted").toBool();
    qDebug() << "Video resolution:" << settingsManager->value("video/resolution").toSize();
    qDebug() << "UI theme:" << settingsManager->value("ui/theme").toString();
    qDebug() << "Network timeout:" << settingsManager->value("network/timeout").toInt();
    
    // Check if keys exist
    qDebug() << "\n--- Checking Keys ---";
    qDebug() << "Has audio/volume:" << settingsManager->contains("audio/volume");
    qDebug() << "Has audio/nonexistent:" << settingsManager->contains("audio/nonexistent");
    
    // Get all keys
    qDebug() << "\n--- All Keys ---";
    QStringList allKeys = settingsManager->allKeys();
    for (const QString& key : allKeys) {
        qDebug() << "Key:" << key << "Value:" << settingsManager->value(key);
    }
    
    // Get child keys and groups
    qDebug() << "\n--- Child Keys and Groups ---";
    QStringList audioKeys = settingsManager->childKeys("audio");
    qDebug() << "Audio keys:" << audioKeys;
    
    QStringList rootGroups = settingsManager->childGroups("");
    qDebug() << "Root groups:" << rootGroups;
    
    // Sync settings
    qDebug() << "\n--- Syncing Settings ---";
    if (settingsManager->sync()) {
        qDebug() << "Settings synced successfully";
    } else {
        qWarning() << "Failed to sync settings";
    }
    
    // Validate settings
    qDebug() << "\n--- Validating Settings ---";
    if (settingsManager->validate()) {
        qDebug() << "Settings validation passed";
    } else {
        qWarning() << "Settings validation failed";
    }
    
    // Export settings
    qDebug() << "\n--- Exporting Settings ---";
    if (settingsManager->exportSettings("exported_settings.json")) {
        qDebug() << "Settings exported successfully";
    } else {
        qWarning() << "Failed to export settings";
    }
    
    // Reset a specific value
    qDebug() << "\n--- Resetting Values ---";
    qDebug() << "Before reset - Audio volume:" << settingsManager->value("audio/volume").toDouble();
    settingsManager->remove("audio/volume");
    qDebug() << "After reset - Audio volume:" << settingsManager->value("audio/volume", 0.5).toDouble();
    
    // Cleanup
    qDebug() << "\n--- Cleanup ---";
    module->shutdown();
    qDebug() << "Settings Module shutdown completed";
    
    qDebug() << "\n=== Example Completed ===";
    
    return 0;
}