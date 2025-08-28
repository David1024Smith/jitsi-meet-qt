QT += core widgets multimedia multimediawidgets network websockets

CONFIG += c++17
CONFIG += console

TARGET = test_screen_sharing
TEMPLATE = app

INCLUDEPATH += include

SOURCES += \
    test_screen_sharing.cpp \
    src/MediaManager.cpp \
    src/WebRTCEngine.cpp

HEADERS += \
    include/MediaManager.h \
    include/WebRTCEngine.h