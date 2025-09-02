QT += core widgets network
# webenginewidgets temporarily disabled due to MSVC configuration issues
# QT += webenginewidgets

CONFIG += c++17

TARGET = JitsiMeetQt
TEMPLATE = app

# 定义版本信息
VERSION = 1.0.0
QMAKE_TARGET_PRODUCT = "Jitsi Meet Qt"
QMAKE_TARGET_DESCRIPTION = "Qt版本的Jitsi Meet桌面应用程序"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2025"

# 包含路径
INCLUDEPATH += include \
               include/windows \
               include/dialogs \
               include/widgets \
               include/models \
               include/utils

# 源文件
SOURCES += src/main.cpp \
           src/MainApplication.cpp \
           src/ConferenceWindow.cpp \
           src/WelcomeWindow.cpp \
           src/SettingsDialog.cpp \
           src/ConfigurationManager.cpp \
           src/ProtocolHandler.cpp \
           src/JitsiMeetAPI.cpp

# 头文件
HEADERS += include/MainApplication.h \
           include/ConferenceWindow.h \
           include/WelcomeWindow.h \
           include/SettingsDialog.h \
           include/ConfigurationManager.h \
           include/ProtocolHandler.h \
           include/JitsiMeetAPI.h

# 资源文件
RESOURCES += resources/resources.qrc

# Windows特定配置
win32 {
    CONFIG += console
    # RC_ICONS = resources/icons/app.ico
    
    # 调试配置
    CONFIG(debug, debug|release) {
        DEFINES += DEBUG_MODE
        TARGET = $$TARGET"_debug"
    }
    
    # 发布配置
    CONFIG(release, debug|release) {
        # DEFINES += QT_NO_DEBUG_OUTPUT  # 暂时注释以启用调试输出
    }
}

# 输出目录
CONFIG(debug, debug|release) {
    DESTDIR = build/debug
    OBJECTS_DIR = build/debug/obj
    MOC_DIR = build/debug/moc
    RCC_DIR = build/debug/rcc
    UI_DIR = build/debug/ui
}

CONFIG(release, debug|release) {
    DESTDIR = build/release
    OBJECTS_DIR = build/release/obj
    MOC_DIR = build/release/moc
    RCC_DIR = build/release/rcc
    UI_DIR = build/release/ui
}

# 编译器标志
QMAKE_CXXFLAGS += -Wall -Wextra

# 调试模式下启用更多调试信息
CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -g -O0
}

# 发布模式下优化
CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += -O2
}