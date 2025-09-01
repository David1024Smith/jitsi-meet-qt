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
    src/AudioDevice.cpp
    
    # Processing
    processing/AudioProcessor.cpp
    processing/NoiseReduction.cpp
    processing/EchoCancel.cpp
    
    # Devices
    devices/InputDevice.cpp
    devices/OutputDevice.cpp
    devices/DeviceManager.cpp
)

# Header files
set(AUDIO_HEADERS
    # Core headers
    include/AudioModule.h
    include/AudioManager.h
    include/AudioDevice.h
    
    # Processing headers
    processing/AudioProcessor.h
    processing/NoiseReduction.h
    processing/EchoCancel.h
    
    # Device headers
    devices/InputDevice.h
    devices/OutputDevice.h
    devices/DeviceManager.h
    
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