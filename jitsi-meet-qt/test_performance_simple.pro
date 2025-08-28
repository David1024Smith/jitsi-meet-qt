QT += core testlib
QT -= gui

CONFIG += console
CONFIG -= app_bundle

TARGET = test_performance_simple
TEMPLATE = app

# Source files
SOURCES += test_performance_simple.cpp

# Compiler flags
QMAKE_CXXFLAGS += -Wall -Wextra

# Debug configuration
CONFIG(debug, debug|release) {
    DEFINES += QT_DEBUG
    TARGET = $$TARGET"_debug"
}

# Release configuration
CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}