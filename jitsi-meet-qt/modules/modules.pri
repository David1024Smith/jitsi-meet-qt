# Modules Manager - Qt Project Include File
# 模块管理器 - Qt项目包含文件

# 模块版本信息
MODULES_VERSION = 1.2.0
DEFINES += MODULES_VERSION=\\\"$$MODULES_VERSION\\\"

# 启用模块系统
DEFINES += MODULES_ENABLED

# 包含所有模块
message("Loading modules...")

# 摄像头模块
exists($$PWD/camera/camera.pri) {
    include($$PWD/camera/camera.pri)
    message("✓ Camera module loaded")
    DEFINES += CAMERA_MODULE_AVAILABLE
} else {
    warning("✗ Camera module not found")
}

# 音频模块 (预留)
# exists($$PWD/audio/audio.pri) {
#     include($$PWD/audio/audio.pri)
#     message("✓ Audio module loaded")
#     DEFINES += AUDIO_MODULE_AVAILABLE
# }

# 网络模块 (预留)
# exists($$PWD/network/network.pri) {
#     include($$PWD/network/network.pri)
#     message("✓ Network module loaded")
#     DEFINES += NETWORK_MODULE_AVAILABLE
# }

# 界面模块 (预留)
# exists($$PWD/ui/ui.pri) {
#     include($$PWD/ui/ui.pri)
#     message("✓ UI module loaded")
#     DEFINES += UI_MODULE_AVAILABLE
# }

# 性能模块 (预留)
# exists($$PWD/performance/performance.pri) {
#     include($$PWD/performance/performance.pri)
#     message("✓ Performance module loaded")
#     DEFINES += PERFORMANCE_MODULE_AVAILABLE
# }

message("All available modules loaded successfully")