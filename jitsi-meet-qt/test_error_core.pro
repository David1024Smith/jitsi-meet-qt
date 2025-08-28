TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle

QT += core network

TARGET = test_error_core
INCLUDEPATH += include

SOURCES += test_error_core_only.cpp \
           src/JitsiError.cpp \
           src/ErrorUtils.cpp

HEADERS += include/JitsiError.h \
           include/ErrorUtils.h \
           include/JitsiConstants.h

# Enable C++17
QMAKE_CXXFLAGS += -std=c++17

# Windows specific settings
win32 {
    QMAKE_CXXFLAGS += -Wall -Wextra
    QMAKE_LFLAGS += -static-libgcc -static-libstdc++
}