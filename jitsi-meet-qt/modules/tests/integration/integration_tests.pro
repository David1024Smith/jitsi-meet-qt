# Module Integration Test qmake project file

QT += core testlib network
QT -= gui

CONFIG += c++17 console testcase
CONFIG -= app_bundle

TARGET = ModuleIntegrationTest
TEMPLATE = app

# Source files
SOURCES += \
    ModuleIntegrationTest.cpp

HEADERS += \
    ModuleIntegrationTest.h

# Include paths
INCLUDEPATH += \
    $$PWD/../../utils/include \
    $$PWD/../../settings/include \
    $$PWD/../../performance/include \
    $$PWD/../../network/include \
    $$PWD/../../ui/include \
    $$PWD/../../chat/include \
    $$PWD/../../screenshare/include \
    $$PWD/../../meeting/include \
    $$PWD/../../audio/include \
    $$PWD/../../camera/include \
    $$PWD/../../compatibility/include

# Conditionally include module libraries
exists($$PWD/../../utils/utils.pri) {
    include($$PWD/../../utils/utils.pri)
    DEFINES += UTILS_MODULE_AVAILABLE
}

exists($$PWD/../../settings/settings.pri) {
    include($$PWD/../../settings/settings.pri)
    DEFINES += SETTINGS_MODULE_AVAILABLE
}

exists($$PWD/../../performance/performance.pri) {
    include($$PWD/../../performance/performance.pri)
    DEFINES += PERFORMANCE_MODULE_AVAILABLE
}

exists($$PWD/../../network/network.pri) {
    include($$PWD/../../network/network.pri)
    DEFINES += NETWORK_MODULE_AVAILABLE
}

exists($$PWD/../../ui/ui.pri) {
    include($$PWD/../../ui/ui.pri)
    DEFINES += UI_MODULE_AVAILABLE
}

exists($$PWD/../../chat/chat.pri) {
    include($$PWD/../../chat/chat.pri)
    DEFINES += CHAT_MODULE_AVAILABLE
}

exists($$PWD/../../screenshare/screenshare.pri) {
    include($$PWD/../../screenshare/screenshare.pri)
    DEFINES += SCREENSHARE_MODULE_AVAILABLE
}

exists($$PWD/../../meeting/meeting.pri) {
    include($$PWD/../../meeting/meeting.pri)
    DEFINES += MEETING_MODULE_AVAILABLE
}

exists($$PWD/../../audio/audio.pri) {
    include($$PWD/../../audio/audio.pri)
    DEFINES += AUDIO_MODULE_AVAILABLE
}

exists($$PWD/../../camera/camera.pri) {
    include($$PWD/../../camera/camera.pri)
    DEFINES += CAMERA_MODULE_AVAILABLE
}

exists($$PWD/../../compatibility/compatibility.pri) {
    include($$PWD/../../compatibility/compatibility.pri)
    DEFINES += COMPATIBILITY_MODULE_AVAILABLE
}

# Test configuration
DEFINES += QT_TESTCASE_BUILDDIR=\\\"$$OUT_PWD\\\"

# Output directory
DESTDIR = $$PWD/bin

# Temporary directories
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui