QT += core widgets

CONFIG += c++17

TARGET = test_simple
TEMPLATE = app

SOURCES += test_simple.cpp

win32 {
    CONFIG += console
}