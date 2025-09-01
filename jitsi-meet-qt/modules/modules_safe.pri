# Jitsi Meet Qt Modular Architecture - Safe Build System
# 安全编译版本，暂时禁用有问题的模块

######################################################################
# Module System Configuration
######################################################################

MODULES_VERSION = 2.0.0
DEFINES += MODULES_VERSION=\\\"$MODULES_VERSION\\\"
DEFINES += MODULES_ENABLED
DEFINES += MODULE_SYSTEM_VERSION=\\\"$MODULES_VERSION\\\"

CONFIG += modules_enabled
CONFIG += module_dependency_check
CONFIG += module_version_check

message("=== Jitsi Meet Qt Modular Architecture v$MODULES_VERSION (Safe Mode) ===")
message("Loading core modules only for stable compilation...")

######################################################################
# Core Modules (仅加载稳定模块)
######################################################################

# 1. 工具模块 (Utils) - 基础依赖
exists($PWD/utils/utils.pri) {
    include($PWD/utils/utils.pri)
    message("✓ Utils module loaded")
    DEFINES += UTILS_MODULE_AVAILABLE
    CONFIG += utils_module_loaded
} else {
    message("○ Utils module not found")
}

# 2. 设置模块 (Settings) - 配置管理
exists($PWD/settings/settings.pri) {
    include($PWD/settings/settings.pri)
    message("✓ Settings module loaded")
    DEFINES += SETTINGS_MODULE_AVAILABLE
    CONFIG += settings_module_loaded
} else {
    message("○ Settings module not found")
}

# 3. 性能模块 (Performance) - 系统监控
exists($PWD/performance/performance.pri) {
    include($PWD/performance/performance.pri)
    message("✓ Performance module loaded")
    DEFINES += PERFORMANCE_MODULE_AVAILABLE
    CONFIG += performance_module_loaded
} else {
    message("○ Performance module not found")
}

######################################################################
# Media Modules (媒体相关模块)
######################################################################

# 4. 相机模块 (Camera)
exists($PWD/camera/camera.pri) {
    include($PWD/camera/camera.pri)
    message("✓ Camera module loaded")
    DEFINES += CAMERA_MODULE_AVAILABLE
    CONFIG += camera_module_loaded
} else {
    message("○ Camera module not found")
}

# 5. 音频模块 (Audio) - 音频设备管理
exists($PWD/audio/audio.pri) {
    include($PWD/audio/audio.pri)
    message("✓ Audio module loaded")
    DEFINES += AUDIO_MODULE_AVAILABLE
    CONFIG += audio_module_loaded
} else {
    message("○ Audio module not found")
}

# 6. 屏幕共享模块 (ScreenShare) - 屏幕捕获
exists($PWD/screenshare/screenshare.pri) {
    include($PWD/screenshare/screenshare.pri)
    message("✓ ScreenShare module loaded")
    DEFINES += SCREENSHARE_MODULE_AVAILABLE
    CONFIG += screenshare_module_loaded
} else {
    message("○ ScreenShare module not found")
}

######################################################################
# Communication Modules (通信相关模块)
######################################################################

# 7. 网络模块 (Network) - 网络通信
exists($PWD/network/network.pri) {
    include($PWD/network/network.pri)
    message("✓ Network module loaded")
    DEFINES += NETWORK_MODULE_AVAILABLE
    CONFIG += network_module_loaded
} else {
    message("○ Network module not found")
}

# 8. 聊天模块 (Chat) - 暂时禁用
message("○ Chat module temporarily disabled for compilation")

# 9. 会议模块 (Meeting) - 暂时禁用
message("○ Meeting module temporarily disabled for compilation")

######################################################################
# UI Modules (界面相关模块，暂时禁用)
######################################################################

# 10. UI模块 (UI) - 暂时禁用
message("○ UI module temporarily disabled for compilation")

######################################################################
# Module System Summary
######################################################################

message("=== Safe Mode Module Loading Summary ===")
message("Core modules loaded for stable compilation")
message("Problem modules temporarily disabled")
message("==========================================")