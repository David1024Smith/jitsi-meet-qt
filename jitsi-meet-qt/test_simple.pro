QT += core widgets multimedia multimediawidgets gui network

CONFIG += c++17

TARGET = test_simple_compile
TEMPLATE = app

SOURCES += test_simple_compile.cpp

# Windows specific settings
win32 {
    CONFIG += console
}