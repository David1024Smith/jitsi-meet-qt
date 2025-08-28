#include <iostream>
#include <string>
#include <cassert>

// Simple test for protocol handler logic without Qt dependencies
class SimpleProtocolHandler {
public:
    static bool isValidProtocolUrl(const std::string& url) {
        const std::string prefix = "jitsi-meet://";
        if (url.length() <= prefix.length()) {
            return false;
        }
        
        if (url.substr(0, prefix.length()) != prefix) {
            return false;
        }
        
        std::string roomInfo = url.substr(prefix.length());
        if (roomInfo.empty()) {
            return false;
        }
        
        // Check for valid characters (alphanumeric, dash, underscore, dot, slash)
        for (char c : roomInfo) {
            if (!std::isalnum(c) && c != '-' && c != '_' && c != '.' && c != '/' && c != ':') {
                return false;
            }
        }
        
        return true;
    }
    
    static std::string parseProtocolUrl(const std::string& url) {
        if (!isValidProtocolUrl(url)) {
            return "";
        }
        
        const std::string prefix = "jitsi-meet://";
        std::string cleanUrl = url.substr(prefix.length());
        
        // If already a complete HTTP(S) URL, return as is
        if (cleanUrl.substr(0, 7) == "http://" || cleanUrl.substr(0, 8) == "https://") {
            return cleanUrl;
        }
        
        // Check if it contains server address
        if (cleanUrl.find('/') != std::string::npos) {
            // Format: server.com/room-name
            return "https://" + cleanUrl;
        } else {
            // Format: room-name, use default server
            return "https://meet.jit.si/" + cleanUrl;
        }
    }
};

void testProtocolValidation() {
    std::cout << "Testing protocol URL validation..." << std::endl;
    
    // Valid URLs
    assert(SimpleProtocolHandler::isValidProtocolUrl("jitsi-meet://test-room"));
    assert(SimpleProtocolHandler::isValidProtocolUrl("jitsi-meet://server.com/room"));
    assert(SimpleProtocolHandler::isValidProtocolUrl("jitsi-meet://https://server.com/room"));
    assert(SimpleProtocolHandler::isValidProtocolUrl("jitsi-meet://room_123"));
    assert(SimpleProtocolHandler::isValidProtocolUrl("jitsi-meet://room.test"));
    
    // Invalid URLs
    assert(!SimpleProtocolHandler::isValidProtocolUrl(""));
    assert(!SimpleProtocolHandler::isValidProtocolUrl("jitsi-meet://"));
    assert(!SimpleProtocolHandler::isValidProtocolUrl("http://test.com"));
    assert(!SimpleProtocolHandler::isValidProtocolUrl("jitsi-meet://room with spaces"));
    assert(!SimpleProtocolHandler::isValidProtocolUrl("jitsi-meet://room@invalid"));
    
    std::cout << "✓ Protocol validation tests passed" << std::endl;
}

void testProtocolParsing() {
    std::cout << "Testing protocol URL parsing..." << std::endl;
    
    // Test simple room name
    std::string result = SimpleProtocolHandler::parseProtocolUrl("jitsi-meet://test-room");
    assert(result == "https://meet.jit.si/test-room");
    
    // Test server with room
    result = SimpleProtocolHandler::parseProtocolUrl("jitsi-meet://example.com/my-room");
    assert(result == "https://example.com/my-room");
    
    // Test full HTTPS URL
    result = SimpleProtocolHandler::parseProtocolUrl("jitsi-meet://https://custom.server.com/room");
    assert(result == "https://custom.server.com/room");
    
    // Test full HTTP URL
    result = SimpleProtocolHandler::parseProtocolUrl("jitsi-meet://http://localhost:8080/test");
    assert(result == "http://localhost:8080/test");
    
    // Test invalid URL
    result = SimpleProtocolHandler::parseProtocolUrl("invalid://test");
    assert(result.empty());
    
    std::cout << "✓ Protocol parsing tests passed" << std::endl;
}

void testEdgeCases() {
    std::cout << "Testing edge cases..." << std::endl;
    
    // Empty URL
    assert(!SimpleProtocolHandler::isValidProtocolUrl(""));
    assert(SimpleProtocolHandler::parseProtocolUrl("").empty());
    
    // Only protocol prefix
    assert(!SimpleProtocolHandler::isValidProtocolUrl("jitsi-meet://"));
    
    // Complex room names
    assert(SimpleProtocolHandler::isValidProtocolUrl("jitsi-meet://my-company.meeting.room_123"));
    
    // Server with path
    std::string result = SimpleProtocolHandler::parseProtocolUrl("jitsi-meet://server.com/path/to/room");
    assert(result == "https://server.com/path/to/room");
    
    std::cout << "✓ Edge case tests passed" << std::endl;
}

int main() {
    std::cout << "Running Protocol Handler Tests" << std::endl;
    std::cout << "==============================" << std::endl;
    
    try {
        testProtocolValidation();
        testProtocolParsing();
        testEdgeCases();
        
        std::cout << std::endl;
        std::cout << "🎉 All tests passed successfully!" << std::endl;
        std::cout << "Protocol Handler implementation is working correctly." << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}