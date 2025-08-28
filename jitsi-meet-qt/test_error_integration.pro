TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle

QT += core widgets network websockets multimedia multimediawidgets testlib

TARGET = test_error_integration
INCLUDEPATH += include

SOURCES += test_error_handling_integration.cpp \
           src/JitsiError.cpp \
           src/ErrorRecoveryManager.cpp \
           src/ErrorDialog.cpp \
           src/ErrorUtils.cpp

HEADERS += include/JitsiError.h \
           include/ErrorRecoveryManager.h \
           include/ErrorDialog.h \
           include/ErrorUtils.h \
           include/JitsiConstants.h

# Enable C++17
QMAKE_CXXFLAGS += -std=c++17

# Windows specific settings
win32 {
    QMAKE_CXXFLAGS += -Wall -Wextra
    QMAKE_LFLAGS += -static-libgcc -static-libstdc++
}