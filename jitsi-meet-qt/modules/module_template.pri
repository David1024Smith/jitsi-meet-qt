# Module Template - Qt Project Include File
# 模块模板 - Qt项目包含文件
# 
# 使用说明:
# 1. 复制此文件到新模块目录，重命名为 <module_name>.pri
# 2. 替换所有 MODULE_NAME 为实际模块名称（大写）
# 3. 替换所有 module_name 为实际模块名称（小写）
# 4. 更新版本号、描述和文件列表
# 5. 根据需要添加依赖项和配置选项

######################################################################
# Module Information
######################################################################

# 模块基本信息
MODULE_NAME = MODULE_NAME
MODULE_VERSION = 1.0.0
MODULE_DESCRIPTION = "Module description here"

# 模块标识符
DEFINES += MODULE_NAME_MODULE_AVAILABLE
DEFINES += MODULE_NAME_VERSION=\\\"$MODULE_VERSION\\\"

# 模块配置
CONFIG += module_name_enabled

######################################################################
# Module Dependencies
######################################################################

# 检查依赖模块 (根据需要取消注释)
# !utils_module_loaded {
#     error("MODULE_NAME module requires Utils module")
# }
# 
# !settings_module_loaded {
#     error("MODULE_NAME module requires Settings module")
# }

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
    $$PWD/include/ModuleNameModule.h \
    $$PWD/include/ModuleNameManager.h \
    $$PWD/include/ModuleNameFactory.h

# 接口头文件
HEADERS += \
    $$PWD/interfaces/IModuleNameDevice.h \
    $$PWD/interfaces/IModuleNameManager.h

# 配置头文件
HEADERS += \
    $$PWD/config/ModuleNameConfig.h

# 工具头文件
HEADERS += \
    $$PWD/utils/ModuleNameUtils.h

# UI组件头文件 (如果有)
HEADERS += \
    $$PWD/widgets/ModuleNameWidget.h

######################################################################
# Sources
######################################################################

# 核心源文件
SOURCES += \
    $$PWD/src/ModuleNameModule.cpp \
    $$PWD/src/ModuleNameManager.cpp \
    $$PWD/src/ModuleNameFactory.cpp

# 配置源文件
SOURCES += \
    $$PWD/config/ModuleNameConfig.cpp

# 工具源文件
SOURCES += \
    $$PWD/utils/ModuleNameUtils.cpp

# UI组件源文件 (如果有)
SOURCES += \
    $$PWD/widgets/ModuleNameWidget.cpp

######################################################################
# Resources (如果有)
######################################################################

# RESOURCES += $$PWD/resources/module_name.qrc

######################################################################
# Platform Specific Configuration
######################################################################

# Windows 特定配置
win32 {
    # Windows 特定的库和定义
    # LIBS += -lwinmm
    # DEFINES += MODULE_NAME_WINDOWS
}

# Linux 特定配置
unix:!macx {
    # Linux 特定的库和定义
    # LIBS += -lpulse
    # DEFINES += MODULE_NAME_LINUX
}

# macOS 特定配置
macx {
    # macOS 特定的库和定义
    # LIBS += -framework CoreAudio
    # DEFINES += MODULE_NAME_MACOS
}

######################################################################
# Module Specific Configuration
######################################################################

# 模块特定的编译选项
# QMAKE_CXXFLAGS += -DMODULE_NAME_CUSTOM_FLAG

# 模块特定的链接库
# LIBS += -lmodule_name_dependency

######################################################################
# Debug Information
######################################################################

# 调试信息
message("MODULE_NAME module v$MODULE_VERSION loaded")
message("  - Description: $MODULE_DESCRIPTION")
message("  - Path: $$PWD")

# 导出模块信息
export(MODULE_NAME)
export(MODULE_VERSION)