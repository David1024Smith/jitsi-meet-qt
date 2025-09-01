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
    src/FileManager.cpp
    src/UtilsErrorHandler.cpp
    src/UtilsSingletonManager.cpp
    
    # File operations
    file/ConfigFile.cpp
    file/FileWatcher.cpp
    file/TempFile.cpp
    
    # Config
    config/UtilsConfig.cpp
    
    # Crypto utilities
    crypto/AESCrypto.cpp
    crypto/HashUtils.cpp
    crypto/RSACrypto.cpp
    
    # String utilities
    string/StringUtils.cpp
    string/Validator.cpp
    
    # Logging
    logging/ConsoleLogger.cpp
    logging/FileLogger.cpp
    logging/NetworkLogger.cpp
)

# Header files
set(UTILS_HEADERS
    # Core headers
    include/UtilsModule.h
    include/Logger.h
    include/FileManager.h
    include/UtilsErrorHandler.h
    include/UtilsSingletonManager.h
    
    # File headers
    file/ConfigFile.h
    file/FileWatcher.h
    file/TempFile.h
    
    # Config headers
    config/UtilsConfig.h
    
    # Crypto headers
    crypto/AESCrypto.h
    crypto/HashUtils.h
    crypto/RSACrypto.h
    
    # String headers
    string/StringUtils.h
    string/Validator.h
    
    # Logging headers
    logging/ConsoleLogger.h
    logging/FileLogger.h
    logging/NetworkLogger.h
    
    # Interface headers
    interfaces/ICryptoHandler.h
    interfaces/IFileHandler.h
    interfaces/ILogger.h
)

# Create the utils library
add_library(UtilsModule STATIC
    ${UTILS_SOURCES}
    ${UTILS_HEADERS}
)

# Link Qt libraries
target_link_libraries(UtilsModule PRIVATE
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
    INTERPROCEDURAL_OPTIMIZATION OFF
)

# Link OpenSSL libraries if found
if(OPENSSL_FOUND)
    target_link_libraries(UtilsModule PRIVATE ${OPENSSL_LIBRARIES})
    target_include_directories(UtilsModule PRIVATE ${OPENSSL_INCLUDE_DIR})
endif()

# Export include directories
target_include_directories(UtilsModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)