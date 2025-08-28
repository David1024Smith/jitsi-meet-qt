QT += core network testlib
QT -= gui

CONFIG += console
CONFIG += c++17
CONFIG -= app_bundle

TARGET = test_performance_optimization
TEMPLATE = app

# Include directories
INCLUDEPATH += include

# Source files
SOURCES += \
    test_performance_optimization.cpp \
    src/PerformanceManager.cpp \
    src/MemoryLeakDetector.cpp \
    src/NetworkOptimizer.cpp \
    src/MediaPerformanceOptimizer.cpp

# Header files
HEADERS += \
    include/PerformanceManager.h \
    include/MemoryLeakDetector.h \
    include/NetworkOptimizer.h \
    include/MediaPerformanceOptimizer.h

# Windows specific libraries
win32 {
    LIBS += -lpsapi -lkernel32
}

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