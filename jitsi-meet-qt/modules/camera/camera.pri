# Camera Module - Qt Project Include File
# 摄像头模块 - Qt项目包含文件

# 模块版本信息
CAMERA_MODULE_VERSION = 1.2.0
DEFINES += CAMERA_MODULE_VERSION=\\\"$$CAMERA_MODULE_VERSION\\\"

# 包含路径
INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/interfaces
INCLUDEPATH += $$PWD/config
INCLUDEPATH += $$PWD/utils
INCLUDEPATH += $$PWD/widgets

# 核心头文件
CAMERA_HEADERS += \
    $$PWD/include/CameraModule.h \
    $$PWD/include/CameraManager.h \
    $$PWD/include/CameraFactory.h

# 接口头文件
CAMERA_HEADERS += \
    $$PWD/interfaces/ICameraDevice.h \
    $$PWD/interfaces/ICameraManager.h

# 配置头文件
CAMERA_HEADERS += \
    $$PWD/config/CameraConfig.h

# 工具头文件
CAMERA_HEADERS += \
    $$PWD/utils/CameraUtils.h

# 组件头文件
CAMERA_HEADERS += \
    $$PWD/widgets/CameraPreviewWidget.h

# 核心源文件
CAMERA_SOURCES += \
    $$PWD/src/CameraModule.cpp \
    $$PWD/src/CameraManager.cpp \
    $$PWD/src/CameraFactory.cpp

# 配置源文件
CAMERA_SOURCES += \
    $$PWD/config/CameraConfig.cpp

# 工具源文件
CAMERA_SOURCES += \
    $$PWD/utils/CameraUtils.cpp

# 组件源文件
CAMERA_SOURCES += \
    $$PWD/widgets/CameraPreviewWidget.cpp

# 资源文件
CAMERA_RESOURCES += \
    $$PWD/resources/camera_icons.qrc

# 添加到主项目
HEADERS += $$CAMERA_HEADERS
SOURCES += $$CAMERA_SOURCES
RESOURCES += $$CAMERA_RESOURCES

# Qt模块依赖
QT += core widgets multimedia multimediawidgets

# 编译器定义
DEFINES += CAMERA_MODULE_ENABLED
DEFINES += CAMERA_INTERFACES_ENABLED
DEFINES += CAMERA_CONFIG_ENABLED
DEFINES += CAMERA_UTILS_ENABLED
DEFINES += CAMERA_WIDGETS_ENABLED

# 模块特定的编译选项
CONFIG += c++17

# 测试支持 (可选)
contains(CONFIG, tests) {
    QT += testlib
    CAMERA_TEST_HEADERS += \
        $$PWD/tests/test_camera_module.h
    
    CAMERA_TEST_SOURCES += \
        $$PWD/tests/test_camera_module.cpp
    
    HEADERS += $$CAMERA_TEST_HEADERS
    SOURCES += $$CAMERA_TEST_SOURCES
    
    DEFINES += CAMERA_TESTS_ENABLED
}

# 示例支持 (可选)
contains(CONFIG, examples) {
    CAMERA_EXAMPLE_SOURCES += \
        $$PWD/examples/basic_camera_example.cpp
    
    # 示例不添加到主项目，需要单独编译
    DEFINES += CAMERA_EXAMPLES_AVAILABLE
}

# 调试信息
message("Camera Module v$$CAMERA_MODULE_VERSION loaded")
message("  - Core components: CameraModule, CameraManager, CameraFactory")
message("  - Interfaces: ICameraDevice, ICameraManager")
message("  - Configuration: CameraConfig")
message("  - Utilities: CameraUtils")
message("  - Widgets: CameraPreviewWidget")
message("  - Resources: Icons, Styles")
contains(CONFIG, tests) {
    message("  - Tests: Enabled")
}
contains(CONFIG, examples) {
    message("  - Examples: Available")
}