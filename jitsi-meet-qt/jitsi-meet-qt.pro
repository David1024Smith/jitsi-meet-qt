######################################################################
# Jitsi Meet Qt - Modular Architecture Project File
# Generated after cleanup of redundant code and duplicate implementations
######################################################################

TEMPLATE = app
TARGET = jitsi-meet-qt
INCLUDEPATH += .

# Qt Configuration
QT += core widgets multimedia multimediawidgets network

# Compiler Configuration
CONFIG += c++17

# Disable deprecated APIs
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060000

# Include modular architecture
include(modules/modules.pri)

# Core Application Headers (Non-modular)
HEADERS += include/AuthenticationManager.h \
           include/ConferenceManager.h \
           include/ConferenceWindow.h \
           include/ErrorDialog.h \
           include/ErrorEventBus.h \
           include/ErrorLogger.h \
           include/ErrorRecoveryManager.h \
           include/ErrorUtils.h \
           include/ILogger.h \
           include/JitsiConstants.h \
           include/JitsiError.h \
           include/MainApplication.h \
           include/MemoryLeakDetector.h \
           include/MemoryProfiler.h \
           include/ModuleError.h \
           include/NavigationBar.h \
           include/OptimizedRecentManager.h \
           include/ProtocolHandler.h \
           include/RecentListWidget.h \
           include/SettingsDialog.h \
           include/StyleHelper.h \
           include/StyleUtils.h \
           include/TranslationManager.h \
           include/WebRTCEngine.h \
           include/WelcomeWindow.h \
           include/WindowManager.h \
           include/XMPPClient.h \
           include/models/ApplicationSettings.h \
           include/models/RecentItem.h

# Core Application Sources (Non-modular)
SOURCES += src/AuthenticationManager.cpp \
           src/ConferenceManager.cpp \
           src/ConferenceWindow.cpp \
           src/ErrorDialog.cpp \
           src/ErrorEventBus.cpp \
           src/ErrorLogger.cpp \
           src/ErrorRecoveryManager.cpp \
           src/ErrorUtils.cpp \
           src/JitsiError.cpp \
           src/main.cpp \
           src/MainApplication.cpp \
           src/MemoryLeakDetector.cpp \
           src/MemoryProfiler.cpp \
           src/ModuleError.cpp \
           src/NavigationBar.cpp \
           src/OptimizedRecentManager.cpp \
           src/ProtocolHandler.cpp \
           src/RecentListWidget.cpp \
           src/SettingsDialog.cpp \
           src/StyleHelper.cpp \
           src/StyleUtils.cpp \
           src/TranslationManager.cpp \
           src/WebRTCEngine.cpp \
           src/WelcomeWindow.cpp \
           src/WindowManager.cpp \
           src/XMPPClient.cpp \
           src/models/ApplicationSettings.cpp \
           src/models/RecentItem.cpp

# Updated Examples (Using Modular APIs)
SOURCES += examples/complete_integration_test.cpp \
           examples/integration_verification.cpp \
           examples/webrtc_integration_demo.cpp

# Resources
RESOURCES += resources/resources.qrc

# Translations
TRANSLATIONS += translations/jitsi_de.ts \
                translations/jitsi_en.ts \
                translations/jitsi_es.ts \
                translations/jitsi_fr.ts \
                translations/jitsi_ja.ts \
                translations/jitsi_ko.ts \
                translations/jitsi_ru.ts \
                translations/jitsi_zh_CN.ts \
                translations/jitsimeet_de.ts \
                translations/jitsimeet_en.ts \
                translations/jitsimeet_es.ts \
                translations/jitsimeet_fr.ts \
                translations/jitsimeet_it.ts \
                translations/jitsimeet_ja.ts \
                translations/jitsimeet_ko.ts \
                translations/jitsimeet_pt.ts \
                translations/jitsimeet_ru.ts \
                translations/jitsimeet_zh_CN.ts

# Build Configuration
CONFIG(debug, debug|release) {
    DEFINES += DEBUG_BUILD
    TARGET = $$TARGET-debug
}

CONFIG(release, debug|release) {
    DEFINES += RELEASE_BUILD
    QMAKE_CXXFLAGS_RELEASE += -O2
}

# Platform-specific configurations
win32 {
    DEFINES += WIN32_PLATFORM
    RC_FILE = resources/app.rc
}

unix:!macx {
    DEFINES += LINUX_PLATFORM
}

macx {
    DEFINES += MACOS_PLATFORM
}

# Output directories
CONFIG(debug, debug|release) {
    DESTDIR = debug
    OBJECTS_DIR = debug/obj
    MOC_DIR = debug/moc
    RCC_DIR = debug/rcc
    UI_DIR = debug/ui
}

CONFIG(release, debug|release) {
    DESTDIR = release
    OBJECTS_DIR = release/obj
    MOC_DIR = release/moc
    RCC_DIR = release/rcc
    UI_DIR = release/ui
}