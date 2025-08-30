QT += core testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = simple_test

SOURCES += test_runner_simple.cpp

DESTDIR = $$PWD/bin

message("Simple test configuration loaded")