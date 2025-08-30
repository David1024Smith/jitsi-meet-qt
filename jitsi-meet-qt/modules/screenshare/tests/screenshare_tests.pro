# 屏幕共享模块测试 qmake 配置

QT += core gui widgets testlib multimedia
CONFIG += testcase

TARGET = screenshare_tests
TEMPLATE = app

# C++ 标准
CONFIG += c++17

# 包含路径
INCLUDEPATH += \
    .. \
    ../include \
    ../interfaces \
    ../config \
    ../capture \
    ../encoding \
    ../widgets \
    mocks

# 测试源文件
SOURCES += \
    ScreenShareModuleTest.cpp \
    TestRunner.cpp \
    mocks/MockScreenShareManager.cpp \
    ../src/ScreenShareModule.cpp \
    ../src/ScreenShareManager.cpp \
    ../src/CaptureEngine.cpp \
    ../config/ScreenShareConfig.cpp \
    ../capture/ScreenCapture.cpp \
    ../capture/WindowCapture.cpp \
    ../capture/RegionCapture.cpp \
    ../encoding/VideoEncoder.cpp \
    ../encoding/FrameProcessor.cpp \
    ../widgets/ScreenShareWidget.cpp \
    ../widgets/ScreenSelector.cpp \
    ../widgets/CapturePreview.cpp

# 测试头文件
HEADERS += \
    ScreenShareModuleTest.h \
    mocks/MockScreenShareManager.h \
    ../include/ScreenShareModule.h \
    ../include/ScreenShareManager.h \
    ../include/CaptureEngine.h \
    ../config/ScreenShareConfig.h \
    ../interfaces/IScreenCapture.h \
    ../interfaces/IScreenShareManager.h \
    ../interfaces/IDisplayManager.h \
    ../capture/ScreenCapture.h \
    ../capture/WindowCapture.h \
    ../capture/RegionCapture.h \
    ../encoding/VideoEncoder.h \
    ../encoding/FrameProcessor.h \
    ../widgets/ScreenShareWidget.h \
    ../widgets/ScreenSelector.h \
    ../widgets/CapturePreview.h

# 编译定义
DEFINES += \
    SCREENSHARE_MODULE_VERSION=\\\"1.0.0\\\" \
    SCREENSHARE_MODULE_ENABLED \
    QT_TESTCASE_BUILDDIR=\\\"$$OUT_PWD\\\"

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
    TARGET = screenshare_tests_debug
}

# 输出目录
DESTDIR = $$OUT_PWD/bin