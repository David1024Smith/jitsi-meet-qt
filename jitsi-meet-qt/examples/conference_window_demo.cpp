#include <QApplication>
#include <QMainWindow>
#include "../include/ConferenceWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ConferenceWindow window;
    window.show();
    
    // Test joining a conference
    window.joinConference("https://meet.jit.si/test-room");
    
    return app.exec();
}