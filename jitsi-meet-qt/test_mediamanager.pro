QT += core widgets multimedia multimediawidgets network websockets

CONFIG += c++17
CONFIG += console

TARGET = test_mediamanager
TEMPLATE = app

INCLUDEPATH += include

SOURCES += \
    test_mediamanager_simple.cpp \
    src/MediaManager.cpp \
    src/WebRTCEngine.cpp

HEADERS += \
    include/MediaManager.h \
    include/WebRTCEngine.h