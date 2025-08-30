QT += core widgets gui

CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17
TARGET = UIComponentsExample
TEMPLATE = app

# Include UI module
include(../ui.pri)

# Source files
SOURCES += \
    UIComponentsExample.cpp

# Headers
HEADERS +=

# Resources (if any)
# RESOURCES += example_resources.qrc

# Output directory
DESTDIR = $$PWD/bin

# Temporary directories
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

# Compiler settings
QMAKE_CXXFLAGS += -Wall -Wextra

# Debug/Release configuration
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
    DEFINES += DEBUG_BUILD
}

CONFIG(release, debug|release) {
    DEFINES += RELEASE_BUILD
}