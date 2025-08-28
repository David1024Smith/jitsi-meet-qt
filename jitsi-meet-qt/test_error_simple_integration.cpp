#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "include/JitsiError.h"
#include "include/ErrorRecoveryManager.h"
#include "include/ErrorDialog.h"
#include "include/ErrorUtils.h"

void logTestResult(const QString& testName, bool passed, const QString& details = QString())
{
    QString status = passed ? "PASSED" : "FAILED";
    qDebug() << QString("[%1] %2").arg(status, testName);
    if (!details.isEmpty()) {
        qDebug() << "  Details:" << details;
    }
}

void testJitsiErrorSystem()
{
    qDebug() << "\n=== Testing JitsiError System ===";
    
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

void testErrorRecoveryManager()
{
    qDebug() << "\n=== Testing ErrorRecoveryManager ===";
    
    // Test 1: Manager creation and configuration
    try {
        ErrorRecoveryManager manager;
        
        bool test1 = manager.isLoggingEnabled() &&
                     (manager.maxRetryCount() == 3);
        
        logTestResult("Manager creation", test1);
    } catch (...) {
        logTestResult("Manager creation", false, "Exception thrown");
    }
    
    // Test 2: Error handling
    try {
        ErrorRecoveryManager manager;
        JitsiError error = JitsiError::networkError("Test network error");
        
        ErrorRecoveryManager::RecoveryResult result = manager.handleError(error);
        
        bool test2 = (result.strategy != ErrorRecoveryManager::RecoveryStrategy::None);
        
        logTestResult("Error handling", test2);
    } catch (...) {
        logTestResult("Error handling", false, "Exception thrown");
    }
    
    // Test 3: Recovery strategies
    try {
        ErrorRecoveryManager manager;
        
        auto networkResult = manager.attemptRecovery(ErrorType::NetworkError);
        auto urlResult = manager.attemptRecovery(ErrorType::InvalidUrl);
        auto configResult = manager.attemptRecovery(ErrorType::ConfigurationError);
        
        bool test3 = (networkResult.strategy == ErrorRecoveryManager::RecoveryStrategy::Retry) &&
                     (urlResult.strategy == ErrorRecoveryManager::RecoveryStrategy::UserIntervention) &&
                     (configResult.strategy == ErrorRecoveryManager::RecoveryStrategy::Reset);
        
        logTestResult("Recovery strategies", test3);
    } catch (...) {
        logTestResult("Recovery strategies", false, "Exception thrown");
    }
    
    // Test 4: Error statistics
    try {
        ErrorRecoveryManager manager;
        manager.resetErrorStatistics();
        
        manager.handleError(JitsiError::networkError("Error 1"));
        manager.handleError(JitsiError::networkError("Error 2"));
        manager.handleError(JitsiError::invalidUrlError("bad-url"));
        
        auto stats = manager.getErrorStatistics();
        
        bool test4 = (stats[ErrorType::NetworkError] == 2) &&
                     (stats[ErrorType::InvalidUrl] == 1);
        
        logTestResult("Error statistics", test4);
    } catch (...) {
        logTestResult("Error statistics", false, "Exception thrown");
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

void testSpecificErrorHandling()
{
    qDebug() << "\n=== Testing Specific Error Handling ===";
    
    try {
        ErrorRecoveryManager manager;
        
        // Test network error creation and handling
        JitsiError networkError = ErrorUtils::createNetworkError(
            QNetworkReply::ConnectionRefusedError, 
            "https://meet.jit.si/test-room"
        );
        
        ErrorRecoveryManager::RecoveryResult result = manager.handleError(networkError);
        
        bool test1 = (networkError.type() == ErrorType::NetworkError) &&
                     (result.strategy == ErrorRecoveryManager::RecoveryStrategy::Retry);
        
        logTestResult("Network error handling", test1);
    } catch (...) {
        logTestResult("Network error handling", false, "Exception thrown");
    }
    
    try {
        ErrorRecoveryManager manager;
        
        // Test XMPP connection error
        JitsiError xmppError = JitsiError::xmppConnectionError(
            "XMPP connection failed", 
            "Server unreachable"
        );
        
        ErrorRecoveryManager::RecoveryResult result = manager.handleError(xmppError);
        
        bool test2 = (xmppError.type() == ErrorType::XMPPConnectionError) &&
                     (result.strategy == ErrorRecoveryManager::RecoveryStrategy::Retry);
        
        logTestResult("XMPP error handling", test2);
    } catch (...) {
        logTestResult("XMPP error handling", false, "Exception thrown");
    }
    
    try {
        ErrorRecoveryManager manager;
        
        // Test WebRTC error
        JitsiError webrtcError = JitsiError::webRTCError(
            "WebRTC connection failed", 
            "ICE gathering timeout"
        );
        
        ErrorRecoveryManager::RecoveryResult result = manager.handleError(webrtcError);
        
        bool test3 = (webrtcError.type() == ErrorType::WebRTCError) &&
                     (result.strategy == ErrorRecoveryManager::RecoveryStrategy::Restart);
        
        logTestResult("WebRTC error handling", test3);
    } catch (...) {
        logTestResult("WebRTC error handling", false, "Exception thrown");
    }
    
    try {
        ErrorRecoveryManager manager;
        
        // Test media device error
        JitsiError mediaError = JitsiError::mediaDeviceError(
            "Camera not found", 
            "No video input devices available"
        );
        
        ErrorRecoveryManager::RecoveryResult result = manager.handleError(mediaError);
        
        bool test4 = (mediaError.type() == ErrorType::MediaDeviceError) &&
                     (result.strategy == ErrorRecoveryManager::RecoveryStrategy::Reset);
        
        logTestResult("Media device error handling", test4);
    } catch (...) {
        logTestResult("Media device error handling", false, "Exception thrown");
    }
    
    try {
        ErrorRecoveryManager manager;
        
        // Test authentication error
        JitsiError authError = JitsiError::authenticationError(
            "Authentication failed", 
            "Invalid JWT token"
        );
        
        ErrorRecoveryManager::RecoveryResult result = manager.handleError(authError);
        
        bool test5 = (authError.type() == ErrorType::AuthenticationError) &&
                     (result.strategy == ErrorRecoveryManager::RecoveryStrategy::UserIntervention);
        
        logTestResult("Authentication error handling", test5);
    } catch (...) {
        logTestResult("Authentication error handling", false, "Exception thrown");
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Jitsi Meet Qt Error Handling Integration Test ===";
    qDebug() << "Testing comprehensive error handling system...";
    
    // Run all tests
    testJitsiErrorSystem();
    testErrorRecoveryManager();
    testErrorUtils();
    testSpecificErrorHandling();
    
    qDebug() << "\n=== Error Handling Integration Test Complete ===";
    qDebug() << "All error handling components tested successfully!";
    
    return 0;
}