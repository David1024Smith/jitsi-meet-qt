######################################################################
# Jitsi Meet Qt - Main Application Project File
######################################################################

QT += core widgets network websockets multimedia multimediawidgets xml concurrent

CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17

TARGET = jitsi-meet-qt
TEMPLATE = app

# Include paths
INCLUDEPATH += include
INCLUDEPATH += src

# Define version
VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Headers
HEADERS += \
    include/AuthenticationManager.h \
    include/ChatManager.h \
    include/ConferenceManager.h \
    include/ConferenceWindow.h \
    include/ConfigurationManager.h \
    include/ErrorDialog.h \
    include/ErrorRecoveryManager.h \
    include/ErrorUtils.h \
    include/JitsiConstants.h \
    include/JitsiError.h \
    include/MainApplication.h \
    include/MediaManager.h \
    include/MemoryLeakDetector.h \
    include/MemoryProfiler.h \
    include/NavigationBar.h \
    include/OptimizedRecentManager.h \
    include/PerformanceConfig.h \
    include/PerformanceManager.h \
    include/ProtocolHandler.h \
    include/RecentListWidget.h \
    include/ScreenShareManager.h \
    include/SettingsDialog.h \
    include/StartupOptimizer.h \
    include/StyleHelper.h \
    include/StyleUtils.h \
    include/ThemeManager.h \
    include/TranslationManager.h \
    include/WebRTCEngine.h \
    include/WelcomeWindow.h \
    include/WindowManager.h \
    include/WindowStateManager.h \
    include/XMPPClient.h \
    include/models/ApplicationSettings.h \
    include/models/RecentItem.h

# Sources
SOURCES += \
    src/main.cpp \
    src/MainApplication.cpp \
    src/AuthenticationManager.cpp \
    src/ChatManager.cpp \
    src/ConferenceManager.cpp \
    src/ConferenceWindow.cpp \
    src/ConfigurationManager.cpp \
    src/ErrorDialog.cpp \
    src/ErrorRecoveryManager.cpp \
    src/ErrorUtils.cpp \
    src/JitsiError.cpp \
    src/MediaManager.cpp \
    src/MemoryLeakDetector.cpp \
    src/MemoryProfiler.cpp \
    src/NavigationBar.cpp \
    src/OptimizedRecentManager.cpp \
    src/PerformanceConfig.cpp \
    src/PerformanceManager.cpp \
    src/ProtocolHandler.cpp \
    src/RecentListWidget.cpp \
    src/ScreenShareManager.cpp \
    src/SettingsDialog.cpp \
    src/StartupOptimizer.cpp \
    src/StyleHelper.cpp \
    src/StyleUtils.cpp \
    src/ThemeManager.cpp \
    src/TranslationManager.cpp \
    src/WebRTCEngine.cpp \
    src/WelcomeWindow.cpp \
    src/WindowManager.cpp \
    src/WindowStateManager.cpp \
    src/XMPPClient.cpp \
    src/models/ApplicationSettings.cpp \
    src/models/RecentItem.cpp

# Resources
RESOURCES += resources/resources.qrc

# Translations
TRANSLATIONS += \
    translations/jitsimeet_en.ts \
    translations/jitsimeet_zh_CN.ts

# Windows specific
win32 {
    RC_FILE = resources/windows.rc
    LIBS += -lws2_32 -lwinmm -lole32 -loleaut32 -luuid -ladvapi32 -lshell32 -luser32 -lgdi32
}
