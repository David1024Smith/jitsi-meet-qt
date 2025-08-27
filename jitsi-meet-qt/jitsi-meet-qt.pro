QT += core widgets webengine webenginewidgets network

CONFIG += c++14

TARGET = JitsiMeetQt
TEMPLATE = app

# 版本信息
VERSION = 1.0.0
QMAKE_TARGET_COMPANY = "Jitsi Meet Qt"
QMAKE_TARGET_PRODUCT = "Jitsi Meet Qt"
QMAKE_TARGET_DESCRIPTION = "Qt version of Jitsi Meet desktop application"
QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2025"

# 包含路径
INCLUDEPATH += $$PWD/include

# 源文件
SOURCES += \
    src/main.cpp \
    src/MainApplication.cpp \
    src/WindowManager.cpp \
    src/ConfigurationManager.cpp \
    src/ProtocolHandler.cpp \
    src/TranslationManager.cpp \
    src/WindowStateManager.cpp \
    src/NavigationBar.cpp \
    src/RecentListWidget.cpp \
    src/WelcomeWindow.cpp \
    src/ConferenceWindow.cpp \
    src/models/ApplicationSettings.cpp \
    src/models/RecentItem.cpp

# 头文件
HEADERS += \
    include/MainApplication.h \
    include/WindowManager.h \
    include/ConfigurationManager.h \
    include/ProtocolHandler.h \
    include/TranslationManager.h \
    include/WindowStateManager.h \
    include/NavigationBar.h \
    include/RecentListWidget.h \
    include/WelcomeWindow.h \
    include/ConferenceWindow.h \
    include/JitsiConstants.h \
    include/models/ApplicationSettings.h \
    include/models/RecentItem.h

# 资源文件
RESOURCES += \
    resources/resources.qrc

# Windows特定配置
win32 {
    RC_FILE = resources/app.rc
    
    # 设置应用程序图标
    RC_ICONS = resources/icons/app.ico
    
    # 启用控制台输出（调试用）
    CONFIG += console
}

# 输出目录
DESTDIR = $$PWD/build
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

# 编译器警告
QMAKE_CXXFLAGS += -Wall -Wextra

# 调试和发布配置
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
    DEFINES += DEBUG_BUILD
}

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}