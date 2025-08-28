#include "include/ScreenShareManager.h"
#include "include/WebRTCEngine.h"
#include <iostream>

int main() {
    std::cout << "Testing ScreenShareManager compilation..." << std::endl;
    
    // Test basic instantiation
    ScreenShareManager* manager = new ScreenShareManager();
    WebRTCEngine* engine = new WebRTCEngine();
    
    // Test basic functionality
    manager->setWebRTCEngine(engine);
    
    auto screens = manager->availableScreens();
    auto windows = manager->availableWindows();
    
    std::cout << "Found " << screens.size() << " screens" << std::endl;
    std::cout << "Found " << windows.size() << " windows" << std::endl;
    
    // Test quality settings
    ScreenShareManager::ShareQuality quality;
    quality.resolution = QSize(1920, 1080);
    quality.frameRate = 15;
    quality.bitrate = 2000000;
    quality.adaptiveQuality = true;
    
    manager->setShareQuality(quality);
    auto retrievedQuality = manager->shareQuality();
    
    std::cout << "Quality settings: " 
              << retrievedQuality.resolution.width() << "x" << retrievedQuality.resolution.height()
              << " @ " << retrievedQuality.frameRate << "fps" << std::endl;
    
    // Test state
    std::cout << "Is screen sharing: " << (manager->isScreenSharing() ? "Yes" : "No") << std::endl;
    std::cout << "Is window sharing: " << (manager->isWindowSharing() ? "Yes" : "No") << std::endl;
    
    delete manager;
    delete engine;
    
    std::cout << "ScreenShareManager verification completed successfully!" << std::endl;
    return 0;
}