# 屏幕共享模块配置文件 (ScreenShare Module Configuration)
# 版本: 1.0.0

SCREENSHARE_MODULE_VERSION = 1.0.0
DEFINES += SCREENSHARE_MODULE_VERSION=\\\"$$SCREENSHARE_MODULE_VERSION\\\"

# 模块启用标志
DEFINES += SCREENSHARE_MODULE_ENABLED

# 模块根目录
SCREENSHARE_MODULE_ROOT = $$PWD

# 包含路径
INCLUDEPATH += \
    $$SCREENSHARE_MODULE_ROOT/include \
    $$SCREENSHARE_MODULE_ROOT/interfaces \
    $$SCREENSHARE_MODULE_ROOT/config \
    $$SCREENSHARE_MODULE_ROOT/capture \
    $$SCREENSHARE_MODULE_ROOT/encoding \
    $$SCREENSHARE_MODULE_ROOT/widgets

# 头文件
HEADERS += \
    # 核心头文件
    $$SCREENSHARE_MODULE_ROOT/include/ScreenShareModule.h \
    $$SCREENSHARE_MODULE_ROOT/include/ScreenShareManager.h \
    $$SCREENSHARE_MODULE_ROOT/include/CaptureEngine.h \
    \
    # 接口定义
    $$SCREENSHARE_MODULE_ROOT/interfaces/IScreenCapture.h \
    $$SCREENSHARE_MODULE_ROOT/interfaces/IScreenShareManager.h \
    $$SCREENSHARE_MODULE_ROOT/interfaces/IDisplayManager.h \
    \
    # 配置管理
    $$SCREENSHARE_MODULE_ROOT/config/ScreenShareConfig.h \
    \
    # 捕获系统
    $$SCREENSHARE_MODULE_ROOT/capture/ScreenCapture.h \
    $$SCREENSHARE_MODULE_ROOT/capture/WindowCapture.h \
    $$SCREENSHARE_MODULE_ROOT/capture/RegionCapture.h \
    \
    # 编码系统
    $$SCREENSHARE_MODULE_ROOT/encoding/VideoEncoder.h \
    $$SCREENSHARE_MODULE_ROOT/encoding/FrameProcessor.h \
    \
    # UI组件
    $$SCREENSHARE_MODULE_ROOT/widgets/ScreenShareWidget.h \
    $$SCREENSHARE_MODULE_ROOT/widgets/ScreenSelector.h \
    $$SCREENSHARE_MODULE_ROOT/widgets/CapturePreview.h

# 源文件
SOURCES += \
    # 核心实现
    $$SCREENSHARE_MODULE_ROOT/src/ScreenShareModule.cpp \
    $$SCREENSHARE_MODULE_ROOT/src/ScreenShareManager.cpp \
    $$SCREENSHARE_MODULE_ROOT/src/CaptureEngine.cpp \
    \
    # 配置管理
    $$SCREENSHARE_MODULE_ROOT/config/ScreenShareConfig.cpp \
    \
    # 捕获系统
    $$SCREENSHARE_MODULE_ROOT/capture/ScreenCapture.cpp \
    $$SCREENSHARE_MODULE_ROOT/capture/WindowCapture.cpp \
    $$SCREENSHARE_MODULE_ROOT/capture/RegionCapture.cpp \
    \
    # 编码系统
    $$SCREENSHARE_MODULE_ROOT/encoding/VideoEncoder.cpp \
    $$SCREENSHARE_MODULE_ROOT/encoding/FrameProcessor.cpp \
    \
    # UI组件
    $$SCREENSHARE_MODULE_ROOT/widgets/ScreenShareWidget.cpp \
    $$SCREENSHARE_MODULE_ROOT/widgets/ScreenSelector.cpp \
    $$SCREENSHARE_MODULE_ROOT/widgets/CapturePreview.cpp

# 资源文件
RESOURCES += \
    $$SCREENSHARE_MODULE_ROOT/resources/screenshare_resources.qrc

# Qt模块依赖
QT += core gui widgets multimedia

# 编译器标志
QMAKE_CXXFLAGS += -std=c++17

# 平台特定配置
win32 {
    DEFINES += SCREENSHARE_WINDOWS
    LIBS += -lgdi32 -luser32 -ldwmapi
}

unix:!macx {
    DEFINES += SCREENSHARE_LINUX
    LIBS += -lX11 -lXext -lXfixes
}

macx {
    DEFINES += SCREENSHARE_MACOS
    LIBS += -framework CoreGraphics -framework CoreVideo
}

# 调试配置
CONFIG(debug, debug|release) {
    DEFINES += SCREENSHARE_DEBUG
    TARGET = screenshare_debug
} else {
    TARGET = screenshare
}

# 输出信息
message("ScreenShare Module v$$SCREENSHARE_MODULE_VERSION loaded")
message("Platform: $$QMAKE_PLATFORM")
message("Qt version: $$QT_VERSION")