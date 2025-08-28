#include "include/WelcomeWindow.h"
#include "include/ConfigurationManager.h"
#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Create configuration manager
    ConfigurationManager configManager;
    
    // Create welcome window
    WelcomeWindow window;
    window.setConfigurationManager(&configManager);
    window.show();
    
    return app.exec();
}