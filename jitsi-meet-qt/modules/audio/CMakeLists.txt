cmake_minimum_required(VERSION 3.16)

project(AudioModule)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Multimedia)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

# Source files
set(AUDIO_SOURCES
    # Core audio files
    src/AudioModule.cpp
    src/AudioManager.cpp
    src/AudioFactory.cpp
    
    # Platform specific
    src/platform/QtAudioDevice.cpp
    
    # Config files
    config/AudioConfig.cpp
)

# Header files
set(AUDIO_HEADERS
    # Core headers
    include/AudioModule.h
    include/AudioManager.h
    include/AudioFactory.h
    
    # Config headers
    config/AudioConfig.h
    
    # Platform specific
    src/platform/QtAudioDevice.h
    
    # Interface headers
    interfaces/IAudioManager.h
    interfaces/IAudioDevice.h
)

# Create the audio library
add_library(AudioModule STATIC
    ${AUDIO_SOURCES}
    ${AUDIO_HEADERS}
)

# Link Qt libraries
target_link_libraries(AudioModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
)

# Set target properties
set_target_properties(AudioModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(AudioModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)