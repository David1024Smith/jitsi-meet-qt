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
    src/ScreenCapture.cpp
    src/ScreenSelector.cpp
    
    # Capture engines
    capture/WindowCapture.cpp
    capture/DesktopCapture.cpp
    capture/RegionCapture.cpp
    
    # Processing
    processing/VideoProcessor.cpp
    processing/CompressionManager.cpp
    
    # UI components
    ui/ScreenShareWidget.cpp
    ui/ScreenSelectorDialog.cpp
)

# Header files
set(SCREENSHARE_HEADERS
    # Core headers
    include/ScreenShareManager.h
    include/ScreenCapture.h
    include/ScreenSelector.h
    
    # Capture headers
    capture/WindowCapture.h
    capture/DesktopCapture.h
    capture/RegionCapture.h
    
    # Processing headers
    processing/VideoProcessor.h
    processing/CompressionManager.h
    
    # UI headers
    ui/ScreenShareWidget.h
    ui/ScreenSelectorDialog.h
    
    # Interface headers
    interfaces/IScreenShareManager.h
    interfaces/IScreenCapture.h
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