# Compatibility Adapter Module Configuration
# 兼容性适配器模块配置文件

COMPATIBILITY_VERSION = 1.0.0
DEFINES += COMPATIBILITY_VERSION=\\\"$COMPATIBILITY_VERSION\\\"
DEFINES += COMPATIBILITY_MODULE_ENABLED

message("Loading Compatibility Adapter Module v$COMPATIBILITY_VERSION...")

# 包含路径
INCLUDEPATH += $$PWD/include \
               $$PWD/interfaces \
               $$PWD/adapters \
               $$PWD/validators \
               $$PWD/rollback

# 头文件
HEADERS += \
    $$PWD/include/CompatibilityModule.h \
    $$PWD/include/LegacyCompatibilityAdapter.h \
    $$PWD/include/RollbackManager.h \
    $$PWD/include/CompatibilityValidator.h \
    $$PWD/interfaces/ICompatibilityAdapter.h \
    $$PWD/interfaces/IRollbackManager.h \
    $$PWD/interfaces/ICompatibilityValidator.h \
    $$PWD/adapters/MediaManagerAdapter.h \
    $$PWD/adapters/ChatManagerAdapter.h \
    $$PWD/adapters/ScreenShareManagerAdapter.h \
    $$PWD/adapters/ConferenceManagerAdapter.h \
    $$PWD/validators/FunctionValidator.h \
    $$PWD/validators/PerformanceValidator.h \
    $$PWD/rollback/CheckpointManager.h \
    $$PWD/rollback/StateBackup.h \
    $$PWD/config/CompatibilityConfig.h

# 源文件
SOURCES += \
    $$PWD/src/CompatibilityModule.cpp \
    $$PWD/src/LegacyCompatibilityAdapter.cpp \
    $$PWD/src/RollbackManager.cpp \
    $$PWD/src/CompatibilityValidator.cpp \
    $$PWD/adapters/MediaManagerAdapter.cpp \
    $$PWD/adapters/ChatManagerAdapter.cpp \
    $$PWD/adapters/ScreenShareManagerAdapter.cpp \
    $$PWD/adapters/ConferenceManagerAdapter.cpp \
    $$PWD/validators/FunctionValidator.cpp \
    $$PWD/validators/PerformanceValidator.cpp \
    $$PWD/rollback/CheckpointManager.cpp \
    $$PWD/rollback/StateBackup.cpp \
    $$PWD/config/CompatibilityConfig.cpp

# 测试文件
COMPATIBILITY_TESTS {
    SOURCES += \
        $$PWD/tests/CompatibilityModuleTest.cpp \
        $$PWD/tests/AdapterTest.cpp \
        $$PWD/tests/RollbackTest.cpp
    
    HEADERS += \
        $$PWD/tests/CompatibilityModuleTest.h \
        $$PWD/tests/mocks/MockLegacyManager.h
}

message("✓ Compatibility Adapter Module configured")