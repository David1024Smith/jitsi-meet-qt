#include <QCoreApplication>
#include <QDebug>
#include "include/JitsiError.h"
#include "include/ErrorRecoveryManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Testing Error Handling System...";
    
    // Test JitsiError creation
    JitsiError networkError = JitsiError::networkError("Connection failed", "Timeout occurred");
    qDebug() << "Network Error:" << networkError.toString();
    
    JitsiError webrtcError = JitsiError::webRTCError("WebRTC connection failed", "ICE gathering failed");
    qDebug() << "WebRTC Error:" << webrtcError.toString();
    
    JitsiError xmppError = JitsiError::xmppConnectionError("XMPP connection lost", "Server unreachable");
    qDebug() << "XMPP Error:" << xmppError.toString();
    
    JitsiError authError = JitsiError::authenticationError("Authentication failed", "Invalid credentials");
    qDebug() << "Auth Error:" << authError.toString();
    
    JitsiError mediaError = JitsiError::mediaDeviceError("Camera not found", "No video input devices");
    qDebug() << "Media Error:" << mediaError.toString();
    
    // Test ErrorRecoveryManager
    ErrorRecoveryManager errorManager;
    
    // Test error handling
    auto result = errorManager.handleError(networkError);
    qDebug() << "Recovery result for network error:" << result.success << result.message;
    
    result = errorManager.handleError(webrtcError);
    qDebug() << "Recovery result for WebRTC error:" << result.success << result.message;
    
    result = errorManager.handleError(xmppError);
    qDebug() << "Recovery result for XMPP error:" << result.success << result.message;
    
    result = errorManager.handleError(authError);
    qDebug() << "Recovery result for auth error:" << result.success << result.message;
    
    result = errorManager.handleError(mediaError);
    qDebug() << "Recovery result for media error:" << result.success << result.message;
    
    qDebug() << "Error handling system test completed successfully!";
    
    return 0;
}