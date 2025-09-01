######################################################################
# Jitsi Meet Qt - Modular Architecture Project File
# Generated after cleanup of redundant code and duplicate implementations
######################################################################

TEMPLATE = app
TARGET = jitsi-meet-qt
INCLUDEPATH += . include

# Qt Configuration
QT += core widgets multimedia multimediawidgets network concurrent websockets xml sql charts

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
           include/ConfigurationManager.h \
           include/ErrorDialog.h \
           include/ErrorEventBus.h \
           include/ErrorLogger.h \
           include/ErrorRecoveryManager.h \
           include/ErrorUtils.h \
           include/ILogger.h \
           include/JitsiConstants.h \
           include/JitsiError.h \
           include/MainApplication.h \
           include/MediaManager.h \
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
           include/WindowStateManager.h \
           include/XMPPClient.h \
           include/models/ApplicationSettings.h \
           include/models/RecentItem.h

# Core Application Sources (Non-modular)
SOURCES += src/AuthenticationManager.cpp \
           src/ConferenceManager.cpp \
           src/ConferenceWindow.cpp \
           src/ConfigurationManager.cpp \
           src/ErrorDialog.cpp \
           src/ErrorEventBus.cpp \
           src/ErrorLogger.cpp \
           src/ErrorRecoveryManager.cpp \
           src/ErrorUtils.cpp \
           src/JitsiError.cpp \
           src/main.cpp \
           src/MainApplication.cpp \
           src/MediaManager.cpp \
           src/MemoryLeakDetector.cpp \
           src/MemoryProfiler.cpp \
           src/ModuleError.cpp \
           src/NavigationBar.cpp \
           src/OptimizedRecentManager.cpp \
           src/RecentListWidget.cpp \
           src/SettingsDialog.cpp \
           src/StyleHelper.cpp \
           src/StyleUtils.cpp \
           src/TranslationManager.cpp \
           src/WebRTCEngine.cpp \
           src/WelcomeWindow.cpp \
           src/WindowManager.cpp \
           src/WindowStateManager.cpp \
           src/XMPPClient.cpp \
           src/models/ApplicationSettings.cpp \
           src/models/RecentItem.cpp \
           modules/meeting/handlers/ProtocolHandler.cpp

# Updated Examples (Using Modular APIs) - Temporarily disabled
# SOURCES += examples/complete_integration_test.cpp \
#            examples/integration_verification.cpp \
#            examples/webrtc_integration_demo.cpp

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
    # Fix MinGW linking issues
    CONFIG += console
    QMAKE_LFLAGS += -Wl,--allow-multiple-definition
    # Remove problematic Qt6EntryPoint
    QMAKE_LIBS_QT_ENTRY_POINT =
    # Add required Windows libraries
    LIBS += -lkernel32 -luser32 -lgdi32 -ladvapi32 -lshell32
}

unix:!macx {
    DEFINES += LINUX_PLATFORM
}

macx {
    DEFINES += MACOS_PLATFORM
}

# Output directories
CONFIG(debug, debug|release) {
    DESTDIR = build/Desktop_Qt_6_8_3_MinGW_64_bit-Debug
    OBJECTS_DIR = build/Desktop_Qt_6_8_3_MinGW_64_bit-Debug/obj
    MOC_DIR = build/Desktop_Qt_6_8_3_MinGW_64_bit-Debug/moc
    RCC_DIR = build/Desktop_Qt_6_8_3_MinGW_64_bit-Debug/rcc
    UI_DIR = build/Desktop_Qt_6_8_3_MinGW_64_bit-Debug/ui
}

CONFIG(release, debug|release) {
    DESTDIR = release
    OBJECTS_DIR = release/obj
    MOC_DIR = release/moc
    RCC_DIR = release/rcc
    UI_DIR = release/ui
}