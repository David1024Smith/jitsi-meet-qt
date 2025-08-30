# Module Version Management System
# 模块版本管理系统

######################################################################
# Version Management Configuration
######################################################################

# 模块系统版本兼容性矩阵
MODULES_COMPATIBILITY_VERSION = 2.0.0
MODULES_MIN_SUPPORTED_VERSION = 1.0.0
MODULES_MAX_SUPPORTED_VERSION = 2.9.9

# 版本检查配置
CONFIG += module_version_strict_check
CONFIG += module_compatibility_warnings

######################################################################
# Module Version Requirements
######################################################################

# 定义每个模块的版本要求
UTILS_REQUIRED_VERSION = 1.0.0
SETTINGS_REQUIRED_VERSION = 1.0.0
PERFORMANCE_REQUIRED_VERSION = 1.0.0
CAMERA_REQUIRED_VERSION = 1.2.0
AUDIO_REQUIRED_VERSION = 1.0.0
SCREENSHARE_REQUIRED_VERSION = 1.0.0
NETWORK_REQUIRED_VERSION = 1.0.0
CHAT_REQUIRED_VERSION = 1.0.0
MEETING_REQUIRED_VERSION = 1.0.0
UI_REQUIRED_VERSION = 1.0.0

######################################################################
# Module Dependency Matrix
######################################################################

# 定义模块间的依赖关系
# Utils模块 - 无依赖
UTILS_DEPENDENCIES = 

# Settings模块 - 依赖Utils模块
SETTINGS_DEPENDENCIES = utils

# Performance模块 - 依赖Utils模块
PERFORMANCE_DEPENDENCIES = utils

# Camera模块 - 依赖Utils模块
CAMERA_DEPENDENCIES = utils

# Audio模块 - 依赖Utils模块
AUDIO_DEPENDENCIES = utils

# ScreenShare模块 - 依赖Utils模块
SCREENSHARE_DEPENDENCIES = utils

# Network模块 - 依赖Utils和Settings模块
NETWORK_DEPENDENCIES = utils settings

# Chat模块 - 依赖Network和Utils模块
CHAT_DEPENDENCIES = network utils

# Meeting模块 - 依赖Network和Utils模块
MEETING_DEPENDENCIES = network utils

# UI模块 - 依赖Settings模块
UI_DEPENDENCIES = settings

######################################################################
# Version Information Export
######################################################################

# 导出版本信息供其他文件使用
DEFINES += MODULES_COMPATIBILITY_VERSION=\\\"$$MODULES_COMPATIBILITY_VERSION\\\"
DEFINES += MODULES_MIN_SUPPORTED_VERSION=\\\"$$MODULES_MIN_SUPPORTED_VERSION\\\"
DEFINES += MODULES_MAX_SUPPORTED_VERSION=\\\"$$MODULES_MAX_SUPPORTED_VERSION\\\"

# 导出模块版本要求
DEFINES += UTILS_REQUIRED_VERSION=\\\"$$UTILS_REQUIRED_VERSION\\\"
DEFINES += SETTINGS_REQUIRED_VERSION=\\\"$$SETTINGS_REQUIRED_VERSION\\\"
DEFINES += PERFORMANCE_REQUIRED_VERSION=\\\"$$PERFORMANCE_REQUIRED_VERSION\\\"
DEFINES += CAMERA_REQUIRED_VERSION=\\\"$$CAMERA_REQUIRED_VERSION\\\"
DEFINES += AUDIO_REQUIRED_VERSION=\\\"$$AUDIO_REQUIRED_VERSION\\\"
DEFINES += SCREENSHARE_REQUIRED_VERSION=\\\"$$SCREENSHARE_REQUIRED_VERSION\\\"
DEFINES += NETWORK_REQUIRED_VERSION=\\\"$$NETWORK_REQUIRED_VERSION\\\"
DEFINES += CHAT_REQUIRED_VERSION=\\\"$$CHAT_REQUIRED_VERSION\\\"
DEFINES += MEETING_REQUIRED_VERSION=\\\"$$MEETING_REQUIRED_VERSION\\\"
DEFINES += UI_REQUIRED_VERSION=\\\"$$UI_REQUIRED_VERSION\\\"

######################################################################
# Debug Information
######################################################################

message("Module version management system loaded")
message("  - Compatibility version: $$MODULES_COMPATIBILITY_VERSION")
message("  - Supported version range: $$MODULES_MIN_SUPPORTED_VERSION - $$MODULES_MAX_SUPPORTED_VERSION")