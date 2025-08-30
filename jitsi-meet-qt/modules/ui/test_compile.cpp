#include <QApplication>
#include "widgets/BaseWidget.h"
#include "widgets/CustomButton.h"
#include "widgets/StatusBar.h"
#include "widgets/ToolBar.h"
#include "config/UIConfig.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Test basic widget creation
    BaseWidget baseWidget;
    CustomButton button("Test");
    StatusBar statusBar;
    ToolBar toolBar;
    UIConfig config;
    
    // Test basic functionality
    button.setButtonStyle(CustomButton::PrimaryStyle);
    statusBar.setStatusText("Test message");
    toolBar.addAction("Test Action");
    config.setTheme("dark");
    
    return 0; // Don't actually run the app, just test compilation
}