# Network Module - Qt Project Include File
# 网络模块 - Qt项目包含文件

######################################################################
# Module Information
######################################################################

# 模块基本信息
NETWORK_MODULE_NAME = NETWORK
NETWORK_MODULE_VERSION = 1.0.0
NETWORK_MODULE_DESCRIPTION = "Network communication and protocol handling module"

# 模块标识符
DEFINES += NETWORK_MODULE_AVAILABLE
DEFINES += NETWORK_MODULE_VERSION=\\\"$NETWORK_MODULE_VERSION\\\"

# 模块配置
CONFIG += network_enabled
CONFIG += network_webrtc_support
CONFIG += network_websocket_support

######################################################################
# Module Dependencies
######################################################################

# 检查依赖模块
!utils_module_loaded {
    warning("Network module recommends Utils module for logging and crypto")
}

!settings_module_loaded {
    warning("Network module recommends Settings module for configuration")
}

######################################################################
# Qt Modules
######################################################################

QT += network websockets

######################################################################
# Include Paths
######################################################################

# 模块包含路径
INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/interfaces

######################################################################
# Headers
######################################################################

# 核心头文件
HEADERS += \
    $$PWD/include/NetworkModule.h \
    $$PWD/include/NetworkManager.h \
    $$PWD/include/ConnectionFactory.h

# 接口头文件
HEADERS += \
    $$PWD/interfaces/INetworkManager.h \
    $$PWD/interfaces/IConnectionHandler.h \
    $$PWD/interfaces/IProtocolHandler.h

# 配置头文件
HEADERS += \
    $$PWD/config/NetworkConfig.h

# 工具头文件
HEADERS += \
    $$PWD/utils/NetworkUtils.h

# 协议处理器头文件
HEADERS += \
    $$PWD/protocols/WebRTCProtocol.h \
    $$PWD/protocols/HTTPProtocol.h \
    $$PWD/protocols/WebSocketProtocol.h

# UI组件头文件
HEADERS += \
    $$PWD/widgets/NetworkStatusWidget.h \
    $$PWD/widgets/ConnectionWidget.h

######################################################################
# Sources
######################################################################

# 核心源文件
SOURCES += \
    $$PWD/src/NetworkModule.cpp \
    $$PWD/src/NetworkManager.cpp \
    $$PWD/src/ConnectionFactory.cpp

# 配置源文件
SOURCES += \
    $$PWD/config/NetworkConfig.cpp

# 工具源文件
SOURCES += \
    $$PWD/utils/NetworkUtils.cpp

# 协议处理器源文件
SOURCES += \
    $$PWD/protocols/WebRTCProtocol.cpp \
    $$PWD/protocols/HTTPProtocol.cpp \
    $$PWD/protocols/WebSocketProtocol.cpp

# UI组件源文件
SOURCES += \
    $$PWD/widgets/NetworkStatusWidget.cpp \
    $$PWD/widgets/ConnectionWidget.cpp

######################################################################
# Resources
######################################################################

RESOURCES += $$PWD/resources/network.qrc

######################################################################
# Platform Specific Configuration
######################################################################

# Windows 特定配置
win32 {
    LIBS += -lws2_32 -lwininet
    DEFINES += NETWORK_MODULE_WINDOWS
}

# Linux 特定配置
unix:!macx {
    DEFINES += NETWORK_MODULE_LINUX
}

# macOS 特定配置
macx {
    DEFINES += NETWORK_MODULE_MACOS
}

######################################################################
# Network Specific Configuration
######################################################################

# 网络配置
DEFINES += NETWORK_DEFAULT_TIMEOUT=30000
DEFINES += NETWORK_MAX_CONNECTIONS=100
DEFINES += NETWORK_BUFFER_SIZE=8192

# 协议支持
DEFINES += NETWORK_WEBRTC_ENABLED
DEFINES += NETWORK_WEBSOCKET_ENABLED
DEFINES += NETWORK_HTTP_ENABLED

######################################################################
# Debug Information
######################################################################

message("Network module v$NETWORK_MODULE_VERSION loaded")
message("  - Description: $NETWORK_MODULE_DESCRIPTION")
message("  - WebRTC Support: Enabled")
message("  - WebSocket Support: Enabled")
message("  - HTTP Support: Enabled")

# 导出模块信息
export(NETWORK_MODULE_NAME)
export(NETWORK_MODULE_VERSION)