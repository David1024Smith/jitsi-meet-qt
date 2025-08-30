QT += core widgets gui
CONFIG += console c++17
CONFIG -= app_bundle

TARGET = test_ui_module
TEMPLATE = app

# Include the UI module
include(../ui.pri)

SOURCES += UIModuleTest.cpp

# Output directory
DESTDIR = ../../../build/tests