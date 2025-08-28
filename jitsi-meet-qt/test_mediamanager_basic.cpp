#include <iostream>
#include "include/MediaManager.h"

int main()
{
    std::cout << "Testing MediaManager basic functionality..." << std::endl;
    
    // Test basic instantiation
    try {
        MediaManager manager;
        
        // Test device enumeration
        auto videoDevices = manager.availableVideoDevices();
        auto audioInputDevices = manager.availableAudioInputDevices();
        auto audioOutputDevices = manager.availableAudioOutputDevices();
        
        std::cout << "Video devices found: " << videoDevices.size() << std::endl;
        std::cout << "Audio input devices found: " << audioInputDevices.size() << std::endl;
        std::cout << "Audio output devices found: " << audioOutputDevices.size() << std::endl;
        
        // Test media settings
        MediaManager::MediaSettings settings = manager.mediaSettings();
        std::cout << "Default video resolution: " << settings.videoResolution.width() 
                  << "x" << settings.videoResolution.height() << std::endl;
        std::cout << "Default video frame rate: " << settings.videoFrameRate << std::endl;
        std::cout << "Default audio sample rate: " << settings.audioSampleRate << std::endl;
        
        // Test codec support
        std::cout << "Current video codec: " << manager.currentVideoCodec().toStdString() << std::endl;
        std::cout << "Current audio codec: " << manager.currentAudioCodec().toStdString() << std::endl;
        
        // Test state queries
        std::cout << "Video active: " << (manager.isVideoActive() ? "Yes" : "No") << std::endl;
        std::cout << "Audio active: " << (manager.isAudioActive() ? "Yes" : "No") << std::endl;
        std::cout << "Screen sharing active: " << (manager.isScreenSharingActive() ? "Yes" : "No") << std::endl;
        
        // Test volume controls
        std::cout << "Master volume: " << manager.masterVolume() << std::endl;
        std::cout << "Microphone volume: " << manager.microphoneVolume() << std::endl;
        
        // Test mute states
        std::cout << "Video muted: " << (manager.isVideoMuted() ? "Yes" : "No") << std::endl;
        std::cout << "Audio muted: " << (manager.isAudioMuted() ? "Yes" : "No") << std::endl;
        
        // Test permissions
        std::cout << "Has video permission: " << (manager.hasVideoPermission() ? "Yes" : "No") << std::endl;
        std::cout << "Has audio permission: " << (manager.hasAudioPermission() ? "Yes" : "No") << std::endl;
        
        std::cout << "MediaManager basic functionality test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during MediaManager test: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}