# Chat Module Tests Qt Project File

QT += core widgets testlib network sql

CONFIG += testcase
CONFIG += c++17

TARGET = ChatModuleTests
TEMPLATE = app

# Test configuration
CONFIG(debug, debug|release) {
    DESTDIR = debug
    OBJECTS_DIR = debug/obj
    MOC_DIR = debug/moc
    RCC_DIR = debug/rcc
    UI_DIR = debug/ui
} else {
    DESTDIR = release
    OBJECTS_DIR = release/obj
    MOC_DIR = release/moc
    RCC_DIR = release/rcc
    UI_DIR = release/ui
}

# Include paths
INCLUDEPATH += \
    . \
    .. \
    ../include \
    ../interfaces \
    ../src \
    ../models \
    ../widgets \
    ../storage \
    ../config \
    ../../utils/include \
    ../../network/include \
    ../../ui/include

# Dependency paths
DEPENDPATH += \
    ../src \
    ../models \
    ../widgets \
    ../storage \
    ../config

# Test source files
SOURCES += \
    ChatModuleTest.cpp

# Test header files
HEADERS += \
    ChatModuleTest.h

# Chat module source files to test
SOURCES += \
    ../src/ChatModule.cpp \
    ../src/ChatManager.cpp \
    ../src/MessageHandler.cpp \
    ../storage/MessageStorage.cpp \
    ../storage/HistoryManager.cpp \
    ../models/ChatMessage.cpp \
    ../models/ChatRoom.cpp \
    ../models/Participant.cpp \
    ../widgets/ChatWidget.cpp \
    ../widgets/MessageList.cpp \
    ../widgets/InputWidget.cpp \
    ../config/ChatConfig.cpp

# Chat module header files
HEADERS += \
    ../include/ChatModule.h \
    ../include/ChatManager.h \
    ../include/MessageHandler.h \
    ../storage/MessageStorage.h \
    ../storage/HistoryManager.h \
    ../models/ChatMessage.h \
    ../models/ChatRoom.h \
    ../models/Participant.h \
    ../widgets/ChatWidget.h \
    ../widgets/MessageList.h \
    ../widgets/InputWidget.h \
    ../config/ChatConfig.h \
    ../interfaces/IChatManager.h \
    ../interfaces/IMessageHandler.h \
    ../interfaces/IMessageStorage.h

# Mock files
SOURCES += \
    mocks/MockChatManager.cpp \
    mocks/MockMessageHandler.cpp \
    mocks/MockMessageStorage.cpp \
    mocks/MockNetworkManager.cpp

HEADERS += \
    mocks/MockChatManager.h \
    mocks/MockMessageHandler.h \
    mocks/MockMessageStorage.h \
    mocks/MockNetworkManager.h

# Resource files
RESOURCES += \
    test_resources.qrc

# Defines
DEFINES += \
    QT_TESTCASE_BUILDDIR=\\\"$$OUT_PWD\\\" \
    CHAT_MODULE_TEST_DATA_DIR=\\\"$$PWD/data\\\"

# Platform-specific configurations
win32 {
    CONFIG += console
    DEFINES += WIN32_LEAN_AND_MEAN
}

unix:!macx {
    CONFIG += link_pkgconfig
}

macx {
    CONFIG += app_bundle
}

# Compiler flags
QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic

# Debug configuration
CONFIG(debug, debug|release) {
    DEFINES += DEBUG
    QMAKE_CXXFLAGS += -g -O0
    
    # Enable additional debug features
    DEFINES += QT_QML_DEBUG
    DEFINES += CHAT_MODULE_DEBUG
}

# Release configuration
CONFIG(release, debug|release) {
    DEFINES += NDEBUG QT_NO_DEBUG_OUTPUT
    QMAKE_CXXFLAGS += -O2
}

# Test-specific defines
DEFINES += \
    CHAT_MODULE_TESTING \
    QT_TESTLIB_LIB

# Custom targets
test.target = test
test.commands = ./$$TARGET
test.depends = $$TARGET
QMAKE_EXTRA_TARGETS += test

# Test with coverage
coverage.target = coverage
coverage.commands = \
    gcov *.gcno && \
    lcov --capture --directory . --output-file coverage.info && \
    genhtml coverage.info --output-directory coverage_html
coverage.depends = $$TARGET
QMAKE_EXTRA_TARGETS += coverage

# Memory check target
memcheck.target = memcheck
memcheck.commands = valgrind --tool=memcheck --leak-check=full ./$$TARGET
memcheck.depends = $$TARGET
QMAKE_EXTRA_TARGETS += memcheck

# Benchmark target
benchmark.target = benchmark
benchmark.commands = ./$$TARGET --benchmark
benchmark.depends = $$TARGET
QMAKE_EXTRA_TARGETS += benchmark

# Clean target
QMAKE_CLEAN += \
    *.gcov \
    *.gcno \
    *.gcda \
    coverage.info \
    chat_test_report.txt \
    chat_memcheck.log

# Installation
target.path = /usr/local/bin/tests
INSTALLS += target

# Test data files
testdata.files = data/*
testdata.path = /usr/local/share/jitsi-meet-qt/tests/chat/data
INSTALLS += testdata

# Documentation
docs.files = README.md TEST_DOCUMENTATION.md
docs.path = /usr/local/share/doc/jitsi-meet-qt/tests/chat
INSTALLS += docs

# Print configuration
message("Chat Module Tests Configuration:")
message("  Qt Version: $$QT_VERSION")
message("  Target: $$TARGET")
message("  Destination: $$DESTDIR")
message("  Build Mode: $$CONFIG")
message("  Include Paths: $$INCLUDEPATH")
message("  Defines: $$DEFINES")