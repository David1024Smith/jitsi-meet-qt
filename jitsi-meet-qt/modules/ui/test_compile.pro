QT += core widgets gui

CONFIG += c++17
TARGET = test_compile
TEMPLATE = app

# Include UI module
include(ui.pri)

# Source files
SOURCES += test_compile.cpp

# Compiler settings
QMAKE_CXXFLAGS += -std=c++17