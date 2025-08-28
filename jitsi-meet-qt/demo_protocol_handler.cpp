#include <iostream>
#include <string>
#include <vector>

// Demo class showing ProtocolHandler functionality
class ProtocolHandlerDemo {
public:
    static void demonstrateProtocolHandling() {
        std::cout << "Protocol Handler Demonstration" << std::endl;
        std::cout << "==============================" << std::endl;
        std::cout << std::endl;
        
        // Example protocol URLs that the handler can process
        std::vector<std::string> exampleUrls = {
            "jitsi-meet://my-meeting",
            "jitsi-meet://company-standup",
            "jitsi-meet://meet.example.com/team-meeting",
            "jitsi-meet://https://secure.company.com/board-meeting",
            "jitsi-meet://localhost:8080/dev-meeting"
        };
        
        std::cout << "Example Protocol URLs and their parsed results:" << std::endl;
        std::cout << "-----------------------------------------------" << std::endl;
        
        for (const auto& url : exampleUrls) {
            std::string parsed = parseProtocolUrl(url);
            std::cout << "Input:  " << url << std::endl;
            std::cout << "Output: " << parsed << std::endl;
            std::cout << std::endl;
        }
        
        std::cout << "How it works:" << std::endl;
        std::cout << "1. User clicks a jitsi-meet:// link in browser or email" << std::endl;
        std::cout << "2. Windows launches the Jitsi Meet Qt application" << std::endl;
        std::cout << "3. Application parses the protocol URL" << std::endl;
        std::cout << "4. Application joins the specified meeting room" << std::endl;
        std::cout << std::endl;
        
        std::cout << "Registry Registration (Windows):" << std::endl;
        std::cout << "- Protocol: jitsi-meet://" << std::endl;
        std::cout << "- Handler: JitsiMeetQt.exe" << std::endl;
        std::cout << "- Registry Key: HKEY_CURRENT_USER\\Software\\Classes\\jitsi-meet" << std::endl;
        std::cout << std::endl;
    }
    
private:
    static std::string parseProtocolUrl(const std::string& url) {
        const std::string prefix = "jitsi-meet://";
        if (url.substr(0, prefix.length()) != prefix) {
            return "[Invalid URL]";
        }
        
        std::string cleanUrl = url.substr(prefix.length());
        
        // If already a complete HTTP(S) URL, return as is
        if (cleanUrl.substr(0, 7) == "http://" || cleanUrl.substr(0, 8) == "https://") {
            return cleanUrl;
        }
        
        // Check if it contains server address
        if (cleanUrl.find('/') != std::string::npos) {
            return "https://" + cleanUrl;
        } else {
            return "https://meet.jit.si/" + cleanUrl;
        }
    }
};

int main() {
    ProtocolHandlerDemo::demonstrateProtocolHandling();
    
    std::cout << "Press Enter to continue...";
    std::cin.get();
    
    return 0;
}