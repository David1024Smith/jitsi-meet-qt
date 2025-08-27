QT += core testlib

CONFIG += c++14 console
CONFIG -= app_bundle

TARGET = test_configuration
TEMPLATE = app

# 包含路径
INCLUDEPATH += ../include

# 源文件
SOURCES += \
    test_configuration.cpp \
    ../src/ConfigurationManager.cpp \
    ../src/models/ApplicationSettings.cpp

# 头文件
HEADERS += \
    ../include/ConfigurationManager.h \
    ../include/models/ApplicationSettings.h \
    ../include/JitsiConstants.h

# 输出目录
DESTDIR = ../build/tests
OBJECTS_DIR = ../build/tests/obj
MOC_DIR = ../build/tests/moc