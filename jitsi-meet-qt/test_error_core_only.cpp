#include <QCoreApplication>
#include <QDebug>
#include "include/JitsiError.h"
#include "include/ErrorUtils.h"

void logTestResult(const QString& testName, bool passed, const QString& details = QString())
{
    QString status = passed ? "PASSED" : "FAILED";
    qDebug() << QString("[%1] %2").arg(status, testName);
    if (!details.isEmpty()) {
        qDebug() << "  Details:" << details;
    }
}

void testJitsiErrorBasics()
{
    qDebug() << "\n=== Testing JitsiError Basics ===";
    
    // Test 1: Basic error creation
    try {
        JitsiError error(ErrorType::NetworkError, "Test network error", "Connection timeout");
        
        bool test1 = (error.type() == ErrorType::NetworkError) &&
                     (error.message() == "Test network error") &&
                     (error.details() == "Connection timeout") &&
                     (!error.errorCode().isEmpty()) &&
                     (error.timestamp().isValid());
        
        logTestResult("Basic error creation", test1);
    } catch (...) {
        logTestResult("Basic error creation", false, "Exception thrown");
    }
    
    // Test 2: Static factory methods
    try {
        JitsiError networkError = JitsiError::networkError("Network failed");
        JitsiError urlError = JitsiError::invalidUrlError("bad-url", "Invalid format");
        JitsiError webrtcError = JitsiError::webRTCError("WebRTC failed");
        JitsiError xmppError = JitsiError::xmppConnectionError("XMPP failed");
        JitsiError authError = JitsiError::authenticationError("Auth failed");
        JitsiError mediaError = JitsiError::mediaDeviceError("Media failed");
        
        bool test2 = (networkError.type() == ErrorType::NetworkError) &&
                     (urlError.type() == ErrorType::InvalidUrl) &&
                     (webrtcError.type() == ErrorType::WebRTCError) &&
                     (xmppError.type() == ErrorType::XMPPConnectionError) &&
                     (authError.type() == ErrorType::AuthenticationError) &&
                     (mediaError.type() == ErrorType::MediaDeviceError);
        
        logTestResult("Static factory methods", test2);
    } catch (...) {
        logTestResult("Static factory methods", false, "Exception thrown");
    }
    
    // Test 3: Context management
    try {
        JitsiError error(ErrorType::ConfigurationError, "Config error");
        error.addContext("file", "config.ini");
        error.addContext("line", "42");
        
        bool test3 = (error.getContext("file") == "config.ini") &&
                     (error.getContext("line") == "42") &&
                     (error.getContext("nonexistent").isEmpty()) &&
                     (error.getAllContext().size() >= 2);
        
        logTestResult("Context management", test3);
    } catch (...) {
        logTestResult("Context management", false, "Exception thrown");
    }
    
    // Test 4: Error serialization
    try {
        JitsiError error = JitsiError::webEngineError("WebEngine crashed", "Stack trace");
        error.addContext("component", "webview");
        
        QString logString = error.toLogString();
        QString userMessage = error.toUserMessage();
        QString toString = error.toString();
        
        bool test4 = logString.contains("WebEngineError") &&
                     logString.contains("WebEngine crashed") &&
                     logString.contains("component=webview") &&
                     !userMessage.isEmpty() &&
                     !toString.isEmpty();
        
        logTestResult("Error serialization", test4);
    } catch (...) {
        logTestResult("Error serialization", false, "Exception thrown");
    }
}

void testErrorUtils()
{
    qDebug() << "\n=== Testing ErrorUtils ===";
    
    // Test 1: URL validation
    try {
        auto validRoom = ErrorUtils::validateJitsiUrl("test-room");
        auto validUrl = ErrorUtils::validateJitsiUrl("https://meet.jit.si/test-room");
        auto invalidEmpty = ErrorUtils::validateJitsiUrl("");
        auto invalidFormat = ErrorUtils::validateJitsiUrl("invalid url with spaces");
        
        bool test1 = validRoom.isValid &&
                     validUrl.isValid &&
                     !invalidEmpty.isValid &&
                     !invalidFormat.isValid;
        
        logTestResult("URL validation", test1);
    } catch (...) {
        logTestResult("URL validation", false, "Exception thrown");
    }
    
    // Test 2: Server URL validation
    try {
        auto validServer = ErrorUtils::validateServerUrl("https://meet.jit.si");
        auto invalidHttp = ErrorUtils::validateServerUrl("http://meet.jit.si");
        auto invalidEmpty = ErrorUtils::validateServerUrl("");
        
        bool test2 = validServer.isValid &&
                     !invalidHttp.isValid &&
                     !invalidEmpty.isValid;
        
        logTestResult("Server URL validation", test2);
    } catch (...) {
        logTestResult("Server URL validation", false, "Exception thrown");
    }
    
    // Test 3: URL building and extraction
    try {
        QString builtUrl = ErrorUtils::buildConferenceUrl("test-room", "https://meet.jit.si");
        QString roomName = ErrorUtils::extractRoomName("https://meet.jit.si/test-room-123");
        QString serverUrl = ErrorUtils::extractServerUrl("https://meet.jit.si:8443/test-room");
        
        bool test3 = (builtUrl == "https://meet.jit.si/test-room") &&
                     (roomName == "test-room-123") &&
                     (serverUrl == "https://meet.jit.si:8443");
        
        logTestResult("URL building and extraction", test3);
    } catch (...) {
        logTestResult("URL building and extraction", false, "Exception thrown");
    }
    
    // Test 4: Protocol detection and normalization
    try {
        bool isProtocol1 = ErrorUtils::isJitsiProtocolUrl("jitsi-meet://test-room");
        bool isProtocol2 = ErrorUtils::isJitsiProtocolUrl("https://meet.jit.si/room");
        QString normalized = ErrorUtils::normalizeUrl("  test-room  ");
        
        bool test4 = isProtocol1 &&
                     !isProtocol2 &&
                     (normalized == "test-room");
        
        logTestResult("Protocol detection and normalization", test4);
    } catch (...) {
        logTestResult("Protocol detection and normalization", false, "Exception thrown");
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Jitsi Meet Qt Error Handling Core Test ===";
    qDebug() << "Testing core error handling functionality...";
    
    // Run core tests
    testJitsiErrorBasics();
    testErrorUtils();
    
    qDebug() << "\n=== Error Handling Core Test Complete ===";
    qDebug() << "Core error handling components tested successfully!";
    
    return 0;
}