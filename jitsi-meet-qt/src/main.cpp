#include "MainApplication.h"
#include <QDebug>
#include <QMessageBox>
#include <QStyleFactory>
#include <QDir>

/**
 * @brief Application entry point
 * @param argc Command line argument count
 * @param argv Command line arguments
 * @return Application exit code
 */
int main(int argc, char *argv[])
{
    // Enable high DPI support before creating QApplication
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    
    try {
        // Create main application instance
        MainApplication app(argc, argv);
        
        // Check if this is the primary instance
        if (!app.isPrimaryInstance()) {
            qDebug() << "Secondary instance detected, exiting...";
            return 0;
        }
        
        qDebug() << "Starting" << MainApplication::applicationTitle().data() 
                 << "version" << app.applicationVersion();
        qDebug() << "Minimum window size:" << MainApplication::minimumWindowSize();
        
        // Set application style
        if (QStyleFactory::keys().contains("Fusion")) {
            app.setStyle("Fusion");
        }
        
        // Protocol URL and second instance handling is now integrated in MainApplication
        
        qDebug() << "Application initialized successfully, entering event loop...";
        
        // Start the application event loop
        return app.exec();
        
    } catch (const std::exception& e) {
        qCritical() << "Fatal error:" << e.what();
        
        // Show error dialog if possible
        if (QApplication::instance()) {
            QMessageBox::critical(nullptr, "Fatal Error", 
                                QString("Application failed to start: %1").arg(e.what()));
        }
        
        return 1;
    } catch (...) {
        qCritical() << "Unknown fatal error occurred";
        
        if (QApplication::instance()) {
            QMessageBox::critical(nullptr, "Fatal Error", 
                                "An unknown error occurred during application startup");
        }
        
        return 1;
    }
}