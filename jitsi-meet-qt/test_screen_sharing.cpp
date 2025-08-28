#include <QApplication>
#include <QTimer>
#include <QDebug>
#include "include/MediaManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Testing screen sharing functionality...";
    
    MediaManager manager;
    
    // Test screen enumeration
    auto screens = manager.availableScreens();
    qDebug() << "Available screens:" << screens.size();
    
    for (int i = 0; i < screens.size(); ++i) {
        QScreen* screen = screens[i];
        qDebug() << "Screen" << i << ":" << screen->name() 
                 << "Size:" << screen->size()
                 << "Geometry:" << screen->geometry();
    }
    
    // Test screen sharing start/stop
    if (!screens.isEmpty()) {
        qDebug() << "Starting screen sharing...";
        manager.startScreenSharing(screens.first());
        
        qDebug() << "Screen sharing active:" << manager.isScreenSharingActive();
        
        // Let it run for a short time
        QTimer::singleShot(2000, [&manager]() {
            qDebug() << "Stopping screen sharing...";
            manager.stopScreenSharing();
            qDebug() << "Screen sharing active:" << manager.isScreenSharingActive();
            QApplication::quit();
        });
        
        return app.exec();
    } else {
        qDebug() << "No screens available for testing";
        return 0;
    }
}