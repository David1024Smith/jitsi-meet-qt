cmake_minimum_required(VERSION 3.16)

project(CameraModule)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Multimedia MultimediaWidgets)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

# Source files
set(CAMERA_SOURCES
    # Core camera files
    src/CameraModule.cpp
    src/CameraManager.cpp
    src/VideoDevice.cpp
    
    # Capture
    capture/VideoCapture.cpp
    capture/CameraCapture.cpp
    
    # Processing
    processing/VideoProcessor.cpp
    processing/FrameProcessor.cpp
    
    # Effects
    effects/VideoEffects.cpp
    effects/FilterManager.cpp
)

# Header files
set(CAMERA_HEADERS
    # Core headers
    include/CameraModule.h
    include/CameraManager.h
    include/VideoDevice.h
    
    # Capture headers
    capture/VideoCapture.h
    capture/CameraCapture.h
    
    # Processing headers
    processing/VideoProcessor.h
    processing/FrameProcessor.h
    
    # Effects headers
    effects/VideoEffects.h
    effects/FilterManager.h
    
    # Interface headers
    interfaces/ICameraManager.h
    interfaces/IVideoDevice.h
)

# Create the camera library
add_library(CameraModule STATIC
    ${CAMERA_SOURCES}
    ${CAMERA_HEADERS}
)

# Link Qt libraries
target_link_libraries(CameraModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

# Set target properties
set_target_properties(CameraModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(CameraModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)