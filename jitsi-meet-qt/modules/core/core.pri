# Core Module Configuration Management - Qt项目包含文件
# 模块化架构的核心管理系统

CORE_MODULE_VERSION = 1.0.0
DEFINES += CORE_MODULE_VERSION=\\\"$$CORE_MODULE_VERSION\\\"
DEFINES += CORE_MODULE_AVAILABLE

message("Loading Core Module Management v$$CORE_MODULE_VERSION...")

# 核心模块目录
CORE_MODULE_PATH = $$PWD

# 包含路径
INCLUDEPATH += \
    $$CORE_MODULE_PATH/include \
    $$CORE_MODULE_PATH/interfaces \
    $$CORE_MODULE_PATH/config \
    $$CORE_MODULE_PATH/management \
    $$CORE_MODULE_PATH/monitoring \
    $$CORE_MODULE_PATH/versioning

# 头文件
HEADERS += \
    $$CORE_MODULE_PATH/include/ModuleManager.h \
    $$CORE_MODULE_PATH/include/GlobalModuleConfig.h \
    $$CORE_MODULE_PATH/include/ModuleHealthMonitor.h \
    $$CORE_MODULE_PATH/include/ModuleVersionManager.h \
    $$CORE_MODULE_PATH/interfaces/IModuleManager.h \
    $$CORE_MODULE_PATH/interfaces/IModuleConfig.h \
    $$CORE_MODULE_PATH/interfaces/IHealthMonitor.h \
    $$CORE_MODULE_PATH/interfaces/IVersionManager.h \
    $$CORE_MODULE_PATH/config/ModuleConfig.h \
    $$CORE_MODULE_PATH/management/RuntimeController.h \
    $$CORE_MODULE_PATH/monitoring/HealthChecker.h \
    $$CORE_MODULE_PATH/monitoring/StatusMonitor.h \
    $$CORE_MODULE_PATH/versioning/VersionController.h \
    $$CORE_MODULE_PATH/versioning/UpgradeManager.h

# 源文件
SOURCES += \
    $$CORE_MODULE_PATH/src/ModuleManager.cpp \
    $$CORE_MODULE_PATH/src/GlobalModuleConfig.cpp \
    $$CORE_MODULE_PATH/src/ModuleHealthMonitor.cpp \
    $$CORE_MODULE_PATH/src/ModuleVersionManager.cpp \
    $$CORE_MODULE_PATH/config/ModuleConfig.cpp \
    $$CORE_MODULE_PATH/management/RuntimeController.cpp \
    $$CORE_MODULE_PATH/monitoring/HealthChecker.cpp \
    $$CORE_MODULE_PATH/monitoring/StatusMonitor.cpp \
    $$CORE_MODULE_PATH/versioning/VersionController.cpp \
    $$CORE_MODULE_PATH/versioning/UpgradeManager.cpp

# Qt模块依赖
QT += core widgets network

# 编译器标志
QMAKE_CXXFLAGS += -std=c++17

message("✓ Core Module Management loaded successfully")