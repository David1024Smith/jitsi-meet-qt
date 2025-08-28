#include <QCoreApplication>
#include <QDebug>
#include "XMPPClient.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Testing XMPPClient compilation...";
    
    XMPPClient client;
    qDebug() << "XMPPClient created successfully";
    qDebug() << "Initial state:" << client.connectionState();
    qDebug() << "Is connected:" << client.isConnected();
    qDebug() << "Is in room:" << client.isInRoom();
    
    // Test basic functionality
    client.setAudioMuted(true);
    client.setVideoMuted(false);
    
    qDebug() << "XMPPClient test completed successfully";
    
    return 0;
}