QT += core testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = PerformanceModuleTests

# C++ standard
CONFIG += c++17

# Test configuration
CONFIG += testcase

# Include paths
INCLUDEPATH += \
    $$PWD \
    $$PWD/../include \
    $$PWD/../interfaces \
    $$PWD/../config \
    $$PWD/../monitors \
    $$PWD/../optimizers \
    $$PWD/../utils

# Source files
SOURCES += \
    PerformanceModuleTest.cpp \
    ../src/PerformanceModule.cpp \
    ../src/PerformanceManager.cpp \
    ../src/MetricsCollector.cpp \
    ../config/PerformanceConfig.cpp \
    ../monitors/BaseMonitor.cpp \
    ../monitors/CPUMonitor.cpp \
    ../monitors/MemoryMonitor.cpp \
    ../monitors/NetworkMonitor.cpp \
    ../optimizers/BaseOptimizer.cpp \
    ../optimizers/StartupOptimizer.cpp \
    ../optimizers/MemoryOptimizer.cpp \
    ../optimizers/RenderOptimizer.cpp

# Header files
HEADERS += \
    PerformanceModuleTest.h \
    ../include/PerformanceModule.h \
    ../include/PerformanceManager.h \
    ../include/MetricsCollector.h \
    ../config/PerformanceConfig.h \
    ../monitors/BaseMonitor.h \
    ../monitors/CPUMonitor.h \
    ../monitors/MemoryMonitor.h \
    ../monitors/NetworkMonitor.h \
    ../optimizers/BaseOptimizer.h \
    ../optimizers/StartupOptimizer.h \
    ../optimizers/MemoryOptimizer.h \
    ../optimizers/RenderOptimizer.h \
    ../interfaces/IPerformanceMonitor.h \
    ../interfaces/IResourceTracker.h \
    ../interfaces/IOptimizer.h \
    ../utils/PerformanceUtils.h

# Defines
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_TESTCASE_BUILDDIR=\\\"$$OUT_PWD\\\"

# Debug/Release specific settings
CONFIG(debug, debug|release) {
    DEFINES += DEBUG_BUILD
    TARGET = $$join(TARGET,,,_debug)
}

# Platform specific settings
win32 {
    DEFINES += WIN32_LEAN_AND_MEAN
    LIBS += -lpsapi -lpdh
}

unix:!macx {
    LIBS += -lpthread
}

macx {
    LIBS += -framework CoreFoundation -framework IOKit
}

# Test data directory
test_data.path = $$OUT_PWD/test_data
test_data.files = $$PWD/data/*
INSTALLS += test_data

# Output directory
DESTDIR = $$OUT_PWD/bin

# Intermediate directories
OBJECTS_DIR = $$OUT_PWD/obj
MOC_DIR = $$OUT_PWD/moc
RCC_DIR = $$OUT_PWD/rcc
UI_DIR = $$OUT_PWD/ui

# Enable all warnings
QMAKE_CXXFLAGS += -Wall -Wextra

# Optimization flags for release
CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += -O2
}

# Code coverage (optional)
coverage {
    QMAKE_CXXFLAGS += --coverage
    QMAKE_LFLAGS += --coverage
    LIBS += -lgcov
}

# Memory sanitizer (optional)
sanitizer {
    QMAKE_CXXFLAGS += -fsanitize=address
    QMAKE_LFLAGS += -fsanitize=address
}