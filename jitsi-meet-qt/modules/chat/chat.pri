# Chat Module Configuration
# Jitsi Meet Qt - Chat Module v1.0.0

# 模块信息
CHAT_MODULE_VERSION = 1.0.0
DEFINES += CHAT_MODULE_VERSION=\\\"$$CHAT_MODULE_VERSION\\\"

# 模块启用标志
DEFINES += CHAT_MODULE_ENABLED
message("Loading Chat Module v$$CHAT_MODULE_VERSION")

# 模块根目录
CHAT_MODULE_ROOT = $$PWD

# 包含路径
INCLUDEPATH += \
    $$CHAT_MODULE_ROOT/include \
    $$CHAT_MODULE_ROOT/interfaces \
    $$CHAT_MODULE_ROOT/config \
    $$CHAT_MODULE_ROOT/models \
    $$CHAT_MODULE_ROOT/storage \
    $$CHAT_MODULE_ROOT/widgets

# 头文件
HEADERS += \
    # 核心头文件
    $$CHAT_MODULE_ROOT/include/ChatModule.h \
    $$CHAT_MODULE_ROOT/include/ChatManager.h \
    $$CHAT_MODULE_ROOT/include/MessageHandler.h \
    \
    # 接口定义
    $$CHAT_MODULE_ROOT/interfaces/IChatManager.h \
    $$CHAT_MODULE_ROOT/interfaces/IMessageHandler.h \
    $$CHAT_MODULE_ROOT/interfaces/IMessageStorage.h \
    \
    # 配置管理
    $$CHAT_MODULE_ROOT/config/ChatConfig.h \
    \
    # 数据模型
    $$CHAT_MODULE_ROOT/models/ChatMessage.h \
    $$CHAT_MODULE_ROOT/models/ChatRoom.h \
    $$CHAT_MODULE_ROOT/models/Participant.h \
    \
    # 存储系统
    $$CHAT_MODULE_ROOT/storage/MessageStorage.h \
    $$CHAT_MODULE_ROOT/storage/HistoryManager.h \
    \
    # UI组件
    $$CHAT_MODULE_ROOT/widgets/ChatWidget.h \
    $$CHAT_MODULE_ROOT/widgets/MessageList.h \
    $$CHAT_MODULE_ROOT/widgets/InputWidget.h

# 源文件
SOURCES += \
    # 核心实现
    $$CHAT_MODULE_ROOT/src/ChatModule.cpp \
    $$CHAT_MODULE_ROOT/src/ChatManager.cpp \
    $$CHAT_MODULE_ROOT/src/MessageHandler.cpp \
    \
    # 配置管理
    $$CHAT_MODULE_ROOT/config/ChatConfig.cpp \
    \
    # 存储系统
    $$CHAT_MODULE_ROOT/storage/MessageStorage.cpp \
    $$CHAT_MODULE_ROOT/storage/HistoryManager.cpp \
    \
    # UI组件
    $$CHAT_MODULE_ROOT/widgets/ChatWidget.cpp \
    $$CHAT_MODULE_ROOT/widgets/MessageList.cpp \
    $$CHAT_MODULE_ROOT/widgets/InputWidget.cpp

# 资源文件
RESOURCES += \
    $$CHAT_MODULE_ROOT/resources/chat_resources.qrc

# Qt模块依赖
QT += core widgets network

# 编译器标志
QMAKE_CXXFLAGS += -std=c++17

# 调试配置
CONFIG(debug, debug|release) {
    DEFINES += CHAT_DEBUG_ENABLED
    message("Chat Module: Debug mode enabled")
}

# 发布配置
CONFIG(release, debug|release) {
    DEFINES += CHAT_RELEASE_MODE
    message("Chat Module: Release mode enabled")
}

# 平台特定配置
win32 {
    DEFINES += CHAT_PLATFORM_WINDOWS
    message("Chat Module: Windows platform detected")
}

unix:!macx {
    DEFINES += CHAT_PLATFORM_LINUX
    message("Chat Module: Linux platform detected")
}

macx {
    DEFINES += CHAT_PLATFORM_MACOS
    message("Chat Module: macOS platform detected")
}

# 功能特性配置
contains(CONFIG, chat_encryption_enabled) {
    DEFINES += CHAT_ENCRYPTION_ENABLED
    message("Chat Module: Encryption support enabled")
}

contains(CONFIG, chat_file_sharing_enabled) {
    DEFINES += CHAT_FILE_SHARING_ENABLED
    message("Chat Module: File sharing support enabled")
}

contains(CONFIG, chat_emoji_enabled) {
    DEFINES += CHAT_EMOJI_ENABLED
    message("Chat Module: Emoji support enabled")
}

# 依赖模块检查
!contains(DEFINES, UTILS_MODULE_AVAILABLE) {
    warning("Chat Module: Utils module not found - some features may be limited")
}

!contains(DEFINES, NETWORK_MODULE_AVAILABLE) {
    warning("Chat Module: Network module not found - network features may be limited")
}

!contains(DEFINES, UI_MODULE_AVAILABLE) {
    warning("Chat Module: UI module not found - UI components may be limited")
}

# 测试配置
contains(CONFIG, chat_tests_enabled) {
    DEFINES += CHAT_TESTS_ENABLED
    include($$CHAT_MODULE_ROOT/tests/tests.pri)
    message("Chat Module: Tests enabled")
}

# 示例配置
contains(CONFIG, chat_examples_enabled) {
    include($$CHAT_MODULE_ROOT/examples/examples.pri)
    message("Chat Module: Examples enabled")
}

message("Chat Module configuration completed successfully")