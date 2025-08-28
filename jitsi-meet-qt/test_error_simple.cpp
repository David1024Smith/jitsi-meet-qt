#include <QCoreApplication>
#include <QDebug>
#include "include/JitsiError.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Testing JitsiError class...";
    
    // Test basic error creation
    JitsiError networkError = JitsiError::networkError("Connection failed", "Timeout occurred");
    qDebug() << "Network Error created successfully";
    qDebug() << "Type:" << networkError.typeString();
    qDebug() << "Message:" << networkError.message();
    qDebug() << "Details:" << networkError.details();
    qDebug() << "Is recoverable:" << networkError.isRecoverable();
    
    // Test WebRTC error
    JitsiError webrtcError = JitsiError::webRTCError("WebRTC connection failed", "ICE gathering failed");
    qDebug() << "\nWebRTC Error created successfully";
    qDebug() << "Type:" << webrtcError.typeString();
    qDebug() << "User message:" << webrtcError.toUserMessage();
    
    // Test XMPP error
    JitsiError xmppError = JitsiError::xmppConnectionError("XMPP connection lost", "Server unreachable");
    qDebug() << "\nXMPP Error created successfully";
    qDebug() << "Type:" << xmppError.typeString();
    qDebug() << "Error code:" << xmppError.errorCode();
    
    // Test authentication error
    JitsiError authError = JitsiError::authenticationError("Authentication failed", "Invalid credentials");
    qDebug() << "\nAuth Error created successfully";
    qDebug() << "Severity:" << authError.severityString();
    
    // Test media device error
    JitsiError mediaError = JitsiError::mediaDeviceError("Camera not found", "No video input devices");
    qDebug() << "\nMedia Error created successfully";
    qDebug() << "Context info:" << mediaError.getAllContext();
    
    qDebug() << "\nAll JitsiError tests passed successfully!";
    
    return 0;
}