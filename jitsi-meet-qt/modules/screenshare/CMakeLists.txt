cmake_minimum_required(VERSION 3.16)

project(ScreenShareModule)

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
set(SCREENSHARE_SOURCES
    # Core screenshare files
    src/ScreenShareManager.cpp
    src/CaptureEngine.cpp
    src/ScreenShareModule.cpp
    
    # Capture engines
    capture/WindowCapture.cpp
    capture/RegionCapture.cpp
    capture/ScreenCapture.cpp
    
    # Encoding
    encoding/VideoEncoder.cpp
    encoding/FrameProcessor.cpp
    
    # Config
    config/ScreenShareConfig.cpp
    
    # Widgets
    widgets/ScreenShareWidget.cpp
    widgets/ScreenSelector.cpp
    widgets/CapturePreview.cpp
)

# Header files
set(SCREENSHARE_HEADERS
    # Core headers
    include/ScreenShareManager.h
    include/CaptureEngine.h
    include/ScreenShareModule.h
    
    # Capture headers
    capture/WindowCapture.h
    capture/RegionCapture.h
    capture/ScreenCapture.h
    
    # Encoding headers
    encoding/VideoEncoder.h
    encoding/FrameProcessor.h
    
    # Config headers
    config/ScreenShareConfig.h
    
    # Widget headers
    widgets/ScreenShareWidget.h
    widgets/ScreenSelector.h
    widgets/CapturePreview.h
    
    # Interface headers
    interfaces/IScreenShareManager.h
    interfaces/IScreenCapture.h
    interfaces/IDisplayManager.h
)

# Create the screenshare library
add_library(ScreenShareModule STATIC
    ${SCREENSHARE_SOURCES}
    ${SCREENSHARE_HEADERS}
)

# Link Qt libraries
target_link_libraries(ScreenShareModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

# Set target properties
set_target_properties(ScreenShareModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(ScreenShareModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)