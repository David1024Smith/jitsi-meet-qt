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
    src/CameraFactory.cpp
    
    # Utilities
    utils/CameraUtils.cpp
)

# Header files
set(CAMERA_HEADERS
    # Core headers
    include/CameraModule.h
    include/CameraManager.h
    
    # Interface headers
    interfaces/ICameraManager.h
    interfaces/ICameraDevice.h
    
    # Utilities
    utils/CameraUtils.h
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