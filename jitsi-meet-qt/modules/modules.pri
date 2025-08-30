# Jitsi Meet Qt Modular Architecture - Build System
# Jitsi Meet Qt 模块化架构 - 构建系统

######################################################################
# Module System Configuration
######################################################################

# 模块系统版本
MODULES_VERSION = 2.0.0
DEFINES += MODULES_VERSION=\\\"$$MODULES_VERSION\\\"
DEFINES += MODULES_ENABLED
DEFINES += MODULE_SYSTEM_VERSION=\\\"$$MODULES_VERSION\\\"

# 模块系统配置选项
CONFIG += modules_enabled
CONFIG += module_dependency_check
CONFIG += module_version_check

# 包含版本管理系统
include($$PWD/module_version.pri)

# 模块系统日志
message("=== Jitsi Meet Qt Modular Architecture v$$MODULES_VERSION ===")
message("Loading modules with dependency management and version control...")

######################################################################
# Core Modules (按依赖顺序加载)
######################################################################

# 0. 核心模块管理系统 (Core) - 模块管理基础设施，最优先加载
exists($PWD/core/core.pri) {
    include($PWD/core/core.pri)
    message("✓ Core module management system loaded")
    DEFINES += CORE_MODULE_AVAILABLE
    CONFIG += core_module_loaded
} else {
    message("○ Core module management system not found - will be created")
}

# 1. 工具模块 (Utils) - 基础依赖，最先加载
exists($$PWD/utils/utils.pri) {
    include($$PWD/utils/utils.pri)
    message("✓ Utils module loaded")
    DEFINES += UTILS_MODULE_AVAILABLE
    CONFIG += utils_module_loaded
} else {
    message("○ Utils module not found - will be created")
}

# 2. 设置模块 (Settings) - 配置管理
exists($$PWD/settings/settings.pri) {
    include($$PWD/settings/settings.pri)
    message("✓ Settings module loaded")
    DEFINES += SETTINGS_MODULE_AVAILABLE
    CONFIG += settings_module_loaded
} else {
    message("○ Settings module not found - will be created")
}

# 3. 性能模块 (Performance) - 系统监控
exists($$PWD/performance/performance.pri) {
    include($$PWD/performance/performance.pri)
    message("✓ Performance module loaded")
    DEFINES += PERFORMANCE_MODULE_AVAILABLE
    CONFIG += performance_module_loaded
} else {
    message("○ Performance module not found - will be created")
}

######################################################################
# Media Modules (媒体相关模块)
######################################################################

# 4. 相机模块 (Camera) - 已存在
exists($$PWD/camera/camera.pri) {
    include($$PWD/camera/camera.pri)
    message("✓ Camera module loaded")
    DEFINES += CAMERA_MODULE_AVAILABLE
    CONFIG += camera_module_loaded
} else {
    warning("✗ Camera module not found")
}

# 5. 音频模块 (Audio) - 音频设备管理
exists($$PWD/audio/audio.pri) {
    include($$PWD/audio/audio.pri)
    message("✓ Audio module loaded")
    DEFINES += AUDIO_MODULE_AVAILABLE
    CONFIG += audio_module_loaded
} else {
    message("○ Audio module not found - will be created")
}

# 6. 屏幕共享模块 (ScreenShare) - 屏幕捕获
exists($$PWD/screenshare/screenshare.pri) {
    include($$PWD/screenshare/screenshare.pri)
    message("✓ ScreenShare module loaded")
    DEFINES += SCREENSHARE_MODULE_AVAILABLE
    CONFIG += screenshare_module_loaded
} else {
    message("○ ScreenShare module not found - will be created")
}

######################################################################
# Communication Modules (通信相关模块)
######################################################################

# 7. 网络模块 (Network) - 网络通信
exists($$PWD/network/network.pri) {
    include($$PWD/network/network.pri)
    message("✓ Network module loaded")
    DEFINES += NETWORK_MODULE_AVAILABLE
    CONFIG += network_module_loaded
} else {
    message("○ Network module not found - will be created")
}

# 8. 聊天模块 (Chat) - 消息处理
exists($$PWD/chat/chat.pri) {
    include($$PWD/chat/chat.pri)
    message("✓ Chat module loaded")
    DEFINES += CHAT_MODULE_AVAILABLE
    CONFIG += chat_module_loaded
} else {
    message("○ Chat module not found - will be created")
}

# 9. 会议模块 (Meeting) - 会议管理
exists($$PWD/meeting/meeting.pri) {
    include($$PWD/meeting/meeting.pri)
    message("✓ Meeting module loaded")
    DEFINES += MEETING_MODULE_AVAILABLE
    CONFIG += meeting_module_loaded
} else {
    message("○ Meeting module not found - will be created")
}

######################################################################
# UI Modules (界面相关模块，最后加载)
######################################################################

# 10. UI模块 (UI) - 用户界面
exists($$PWD/ui/ui.pri) {
    include($$PWD/ui/ui.pri)
    message("✓ UI module loaded")
    DEFINES += UI_MODULE_AVAILABLE
    CONFIG += ui_module_loaded
} else {
    message("○ UI module not found - will be created")
}

######################################################################
# Compatibility Module (兼容性模块，用于安全重构)
######################################################################

# 11. 兼容性模块 (Compatibility) - 兼容性适配器系统
exists($PWD/compatibility/compatibility.pri) {
    include($PWD/compatibility/compatibility.pri)
    message("✓ Compatibility module loaded")
    DEFINES += COMPATIBILITY_MODULE_AVAILABLE
    CONFIG += compatibility_module_loaded
} else {
    message("○ Compatibility module not found - will be created")
}

######################################################################
# Module Dependency Validation
######################################################################

# 检查关键依赖关系
module_dependency_check {
    # 音频模块依赖工具模块
    audio_module_loaded:!utils_module_loaded {
        warning("Audio module requires Utils module")
    }
    
    # 网络模块依赖工具和设置模块
    network_module_loaded:!utils_module_loaded {
        warning("Network module requires Utils module")
    }
    network_module_loaded:!settings_module_loaded {
        warning("Network module requires Settings module")
    }
    
    # UI模块依赖设置模块
    ui_module_loaded:!settings_module_loaded {
        warning("UI module requires Settings module")
    }
    
    # 聊天模块依赖网络模块
    chat_module_loaded:!network_module_loaded {
        warning("Chat module requires Network module")
    }
    
    # 会议模块依赖网络模块
    meeting_module_loaded:!network_module_loaded {
        warning("Meeting module requires Network module")
    }
}

######################################################################
# Module System Summary
######################################################################

message("=== Module Loading Summary ===")

# 统计加载的模块数量
loaded_modules = 0
camera_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)
audio_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)
network_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)
ui_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)
performance_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)
utils_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)
settings_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)
chat_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)
screenshare_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)
meeting_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)

compatibility_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)
core_module_loaded: loaded_modules = $$num_add($$loaded_modules, 1)

message("Loaded modules: $$loaded_modules/11")

# 执行版本验证
module_version_check {
    message("=== Module Version Validation ===")
    camera_module_loaded {
        message("✓ Camera module loaded with version validation")
    }
    message("=== Version Validation Complete ===")
}

message("Modular architecture initialized successfully")
message("==========================================")

# 导出模块信息供主项目使用
DEFINES += LOADED_MODULES_COUNT=$$loaded_modules