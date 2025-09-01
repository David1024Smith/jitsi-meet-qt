cmake_minimum_required(VERSION 3.16)

project(UtilsModule)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Xml)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

# Source files
set(UTILS_SOURCES
    # Core utility files
    src/UtilsModule.cpp
    src/Logger.cpp
    src/FileUtils.cpp
    
    # File operations
    file/ConfigFile.cpp
    file/FileManager.cpp
    file/PathUtils.cpp
    
    # System utilities
    system/SystemInfo.cpp
    system/ProcessManager.cpp
    
    # Crypto utilities
    crypto/CryptoUtils.cpp
    crypto/HashGenerator.cpp
    
    # String utilities
    string/StringUtils.cpp
    string/TextProcessor.cpp
)

# Header files
set(UTILS_HEADERS
    # Core headers
    include/UtilsModule.h
    include/Logger.h
    include/FileUtils.h
    
    # File headers
    file/ConfigFile.h
    file/FileManager.h
    file/PathUtils.h
    
    # System headers
    system/SystemInfo.h
    system/ProcessManager.h
    
    # Crypto headers
    crypto/CryptoUtils.h
    crypto/HashGenerator.h
    
    # String headers
    string/StringUtils.h
    string/TextProcessor.h
    
    # Interface headers
    interfaces/ILogger.h
    interfaces/IFileManager.h
)

# Create the utils library
add_library(UtilsModule STATIC
    ${UTILS_SOURCES}
    ${UTILS_HEADERS}
)

# Link Qt libraries
target_link_libraries(UtilsModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
)

# Set target properties
set_target_properties(UtilsModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(UtilsModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)