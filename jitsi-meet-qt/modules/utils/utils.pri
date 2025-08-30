# Utils Module Configuration
# Jitsi Meet Qt - Utils Module v1.0.0

UTILS_MODULE_VERSION = 1.0.0
DEFINES += UTILS_MODULE_VERSION=\\\"$UTILS_MODULE_VERSION\\\"
DEFINES += UTILS_MODULE_AVAILABLE

message("Loading Utils Module v$UTILS_MODULE_VERSION...")

# Module paths
UTILS_MODULE_PATH = $$PWD
INCLUDEPATH += $$UTILS_MODULE_PATH/include
INCLUDEPATH += $$UTILS_MODULE_PATH/interfaces

# Headers
HEADERS += \
    # Core headers
    $$UTILS_MODULE_PATH/include/UtilsModule.h \
    $$UTILS_MODULE_PATH/include/Logger.h \
    $$UTILS_MODULE_PATH/include/FileManager.h \
    \
    # Interface headers
    $$UTILS_MODULE_PATH/interfaces/ILogger.h \
    $$UTILS_MODULE_PATH/interfaces/IFileHandler.h \
    $$UTILS_MODULE_PATH/interfaces/ICryptoHandler.h \
    \
    # Logging system
    $$UTILS_MODULE_PATH/logging/FileLogger.h \
    $$UTILS_MODULE_PATH/logging/ConsoleLogger.h \
    $$UTILS_MODULE_PATH/logging/NetworkLogger.h \
    \
    # Crypto tools
    $$UTILS_MODULE_PATH/crypto/AESCrypto.h \
    $$UTILS_MODULE_PATH/crypto/RSACrypto.h \
    $$UTILS_MODULE_PATH/crypto/HashUtils.h \
    \
    # File tools
    $$UTILS_MODULE_PATH/file/ConfigFile.h \
    $$UTILS_MODULE_PATH/file/TempFile.h \
    $$UTILS_MODULE_PATH/file/FileWatcher.h \
    \
    # String tools
    $$UTILS_MODULE_PATH/string/StringUtils.h \
    $$UTILS_MODULE_PATH/string/Validator.h

# Sources
SOURCES += \
    # Core sources
    $$UTILS_MODULE_PATH/src/UtilsModule.cpp \
    $$UTILS_MODULE_PATH/src/Logger.cpp \
    $$UTILS_MODULE_PATH/src/FileManager.cpp \
    \
    # Logging system
    $$UTILS_MODULE_PATH/logging/FileLogger.cpp \
    $$UTILS_MODULE_PATH/logging/ConsoleLogger.cpp \
    $$UTILS_MODULE_PATH/logging/NetworkLogger.cpp \
    \
    # Crypto tools
    $$UTILS_MODULE_PATH/crypto/AESCrypto.cpp \
    $$UTILS_MODULE_PATH/crypto/RSACrypto.cpp \
    $$UTILS_MODULE_PATH/crypto/HashUtils.cpp \
    \
    # File tools
    $$UTILS_MODULE_PATH/file/ConfigFile.cpp \
    $$UTILS_MODULE_PATH/file/TempFile.cpp \
    $$UTILS_MODULE_PATH/file/FileWatcher.cpp \
    \
    # String tools
    $$UTILS_MODULE_PATH/string/StringUtils.cpp \
    $$UTILS_MODULE_PATH/string/Validator.cpp

# Qt modules required
QT += core network

# Conditional compilation
CONFIG(debug, debug|release) {
    DEFINES += UTILS_DEBUG_MODE
    message("Utils Module: Debug mode enabled")
}

# Platform-specific configurations
win32 {
    DEFINES += UTILS_PLATFORM_WINDOWS
    LIBS += -ladvapi32  # For Windows registry access
}

unix:!mac {
    DEFINES += UTILS_PLATFORM_LINUX
}

mac {
    DEFINES += UTILS_PLATFORM_MAC
}

# Crypto dependencies (OpenSSL)
LIBS += -lssl -lcrypto

message("✓ Utils Module configuration loaded")
# 
Additional headers for integrated configuration and error handling
HEADERS += \
    $UTILS_MODULE_PATH/include/UtilsSingletonManager.h \
    $UTILS_MODULE_PATH/include/UtilsErrorHandler.h \
    $UTILS_MODULE_PATH/config/UtilsConfig.h

# Additional sources for integrated configuration and error handling
SOURCES += \
    $UTILS_MODULE_PATH/src/UtilsSingletonManager.cpp \
    $UTILS_MODULE_PATH/src/UtilsErrorHandler.cpp \
    $UTILS_MODULE_PATH/config/UtilsConfig.cpp

# Include configuration directory
INCLUDEPATH += $UTILS_MODULE_PATH/config

message("✓ Utils Module integrated configuration and error handling loaded")