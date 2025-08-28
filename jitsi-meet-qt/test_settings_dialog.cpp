#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <memory>

#include "SettingsDialog.h"
#include "ConfigurationManager.h"
#include "TranslationManager.h"
#include "MediaManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Testing SettingsDialog implementation...";
    
    try {
        // Create managers
        auto configManager = std::make_unique<ConfigurationManager>();
        auto translationManager = std::make_unique<TranslationManager>();
        auto mediaManager = std::make_unique<MediaManager>();
        
        qDebug() << "Managers created successfully";
        
        // Initialize translation manager
        translationManager->initialize();
        qDebug() << "Translation manager initialized";
        
        // Create settings dialog
        SettingsDialog dialog(configManager.get(), translationManager.get(), mediaManager.get());
        qDebug() << "SettingsDialog created successfully";
        
        // Test basic functionality
        dialog.showSettings();
        qDebug() << "SettingsDialog shown successfully";
        
        // Connect signals for testing
        QObject::connect(&dialog, &SettingsDialog::settingsSaved, [&]() {
            qDebug() << "Settings saved signal received";
            QMessageBox::information(nullptr, "Test", "Settings saved successfully!");
        });
        
        QObject::connect(&dialog, &SettingsDialog::languageChanged, [&](const QString& language) {
            qDebug() << "Language changed to:" << language;
        });
        
        qDebug() << "Test setup completed. Dialog should be visible.";
        qDebug() << "Available features:";
        qDebug() << "- Server URL configuration with validation";
        qDebug() << "- Language selection and interface settings";
        qDebug() << "- Audio/video device selection and testing";
        qDebug() << "- Conference settings (auto-join options)";
        qDebug() << "- Advanced settings (recent items management)";
        
        return app.exec();
        
    } catch (const std::exception& e) {
        qCritical() << "Exception occurred:" << e.what();
        QMessageBox::critical(nullptr, "Error", QString("Test failed: %1").arg(e.what()));
        return 1;
    } catch (...) {
        qCritical() << "Unknown exception occurred";
        QMessageBox::critical(nullptr, "Error", "Test failed with unknown exception");
        return 1;
    }
}