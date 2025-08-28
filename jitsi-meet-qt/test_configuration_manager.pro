QT += core widgets testlib

CONFIG += c++17 testcase
TARGET = test_unit_configuration_manager

# Include directories
INCLUDEPATH += include src

# Source files
SOURCES += src/ConfigurationManager.cpp \
           test_unit_configuration_manager.cpp

# Header files
HEADERS += include/ConfigurationManager.h \
           include/models/ApplicationSettings.h \
           include/models/RecentItem.h

# Compiler flags
QMAKE_CXXFLAGS += -Wall -Wextra

# Debug configuration
CONFIG(debug, debug|release) {
    DESTDIR = debug
    OBJECTS_DIR = debug/obj
    MOC_DIR = debug/moc
}

# Release configuration
CONFIG(release, debug|release) {
    DESTDIR = release
    OBJECTS_DIR = release/obj
    MOC_DIR = release/moc
}

# Platform-specific settings
win32 {
    CONFIG += console
    DEFINES += WIN32_LEAN_AND_MEAN
}