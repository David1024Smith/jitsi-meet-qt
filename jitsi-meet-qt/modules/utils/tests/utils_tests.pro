QT += core testlib network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = utils_tests

# 测试源文件
SOURCES += \
    UtilsModuleTest.cpp

HEADERS += \
    UtilsModuleTest.h

# 包含Utils模块
include(../utils.pri)

# 测试特定配置
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += UTILS_TESTS_ENABLED

# 输出目录
DESTDIR = $$PWD/bin

# 临时文件目录
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

# 平台特定配置
win32 {
    CONFIG += console
    LIBS += -ladvapi32
}

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += openssl
}

# 调试配置
CONFIG(debug, debug|release) {
    DEFINES += UTILS_DEBUG_TESTS
    TARGET = $$join(TARGET,,,_debug)
}

# 测试数据文件
RESOURCES += test_data.qrc

message("Utils Module Tests configuration loaded")