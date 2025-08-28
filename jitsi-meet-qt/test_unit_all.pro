QT += core widgets network websockets multimedia multimediawidgets testlib

CONFIG += c++17 testcase
TARGET = test_unit_all

# Include directories
INCLUDEPATH += include \
               src \
               tests

# Source files for the classes being tested
SOURCES += src/XMPPClient.cpp \
           src/WebRTCEngine.cpp \
           src/ConfigurationManager.cpp \
           src/ChatManager.cpp \
           src/MediaManager.cpp

# Header files
HEADERS += include/XMPPClient.h \
           include/WebRTCEngine.h \
           include/ConfigurationManager.h \
           include/ChatManager.h \
           include/MediaManager.h \
           include/models/ApplicationSettings.h \
           include/models/RecentItem.h

# Test files
SOURCES += test_unit_xmpp_client.cpp \
           test_unit_webrtc_engine.cpp \
           test_unit_configuration_manager.cpp \
           test_unit_chat_manager.cpp \
           test_unit_media_manager.cpp

# Test main file
SOURCES += test_unit_main.cpp

# Compiler flags
QMAKE_CXXFLAGS += -Wall -Wextra

# Debug configuration
CONFIG(debug, debug|release) {
    DESTDIR = debug
    OBJECTS_DIR = debug/obj
    MOC_DIR = debug/moc
    RCC_DIR = debug/rcc
    UI_DIR = debug/ui
}

# Release configuration
CONFIG(release, debug|release) {
    DESTDIR = release
    OBJECTS_DIR = release/obj
    MOC_DIR = release/moc
    RCC_DIR = release/rcc
    UI_DIR = release/ui
}

# Platform-specific settings
win32 {
    CONFIG += console
    DEFINES += WIN32_LEAN_AND_MEAN
}

unix {
    CONFIG += link_pkgconfig
}

# Test configuration
DEFINES += QT_TESTCASE_BUILDDIR=\\\"$$OUT_PWD\\\"