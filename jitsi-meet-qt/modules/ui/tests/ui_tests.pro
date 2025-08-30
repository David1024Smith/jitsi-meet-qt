# UI Module Tests Configuration
# Jitsi Meet Qt - UI Module Tests v1.0.0

QT += core widgets testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = ui_module_tests

# Version information
VERSION = 1.0.0
DEFINES += UI_MODULE_VERSION=\\\"$VERSION\\\"
DEFINES += UI_MODULE_ENABLED
DEFINES += UI_MODULE_TESTS

# Test configuration
CONFIG(debug, debug|release) {
    DEFINES += UI_MODULE_DEBUG
    DESTDIR = debug
    OBJECTS_DIR = debug/obj
    MOC_DIR = debug/moc
    RCC_DIR = debug/rcc
    UI_DIR = debug/ui
} else {
    DEFINES += UI_MODULE_RELEASE
    DESTDIR = release
    OBJECTS_DIR = release/obj
    MOC_DIR = release/moc
    RCC_DIR = release/rcc
    UI_DIR = release/ui
}

# Include paths
INCLUDEPATH += \
    ../include \
    ../interfaces \
    ../config \
    ../themes \
    ../widgets \
    ../layouts \
    ../src

# UI Module sources
SOURCES += \
    # Test sources
    UIModuleTest.cpp \
    widgets/UIComponentsTest.cpp \
    mocks/MockTheme.cpp \
    mocks/MockWidget.cpp \
    \
    # UI Module sources
    ../src/UIModule.cpp \
    ../src/UIManager.cpp \
    ../src/ThemeManager.cpp \
    ../src/ThemeFactory.cpp \
    ../config/UIConfig.cpp \
    \
    # Theme sources
    ../themes/BaseTheme.cpp \
    ../themes/DefaultTheme.cpp \
    ../themes/DarkTheme.cpp \
    ../themes/LightTheme.cpp \
    \
    # Widget sources
    ../widgets/BaseWidget.cpp \
    ../widgets/CustomButton.cpp \
    ../widgets/StatusBar.cpp \
    ../widgets/ToolBar.cpp \
    \
    # Layout sources
    ../layouts/BaseLayout.cpp \
    ../layouts/MainLayout.cpp \
    ../layouts/ConferenceLayout.cpp \
    ../layouts/SettingsLayout.cpp

# UI Module headers
HEADERS += \
    # Test headers
    UIModuleTest.h \
    mocks/MockTheme.h \
    mocks/MockWidget.h \
    \
    # UI Module headers
    ../include/UIModule.h \
    ../include/UIManager.h \
    ../include/ThemeManager.h \
    ../include/ThemeFactory.h \
    ../config/UIConfig.h \
    \
    # Theme headers
    ../themes/BaseTheme.h \
    ../themes/DefaultTheme.h \
    ../themes/DarkTheme.h \
    ../themes/LightTheme.h \
    \
    # Widget headers
    ../widgets/BaseWidget.h \
    ../widgets/CustomButton.h \
    ../widgets/StatusBar.h \
    ../widgets/ToolBar.h \
    \
    # Layout headers
    ../layouts/BaseLayout.h \
    ../layouts/MainLayout.h \
    ../layouts/ConferenceLayout.h \
    ../layouts/SettingsLayout.h \
    \
    # Interface headers
    ../interfaces/IUIManager.h \
    ../interfaces/IThemeManager.h \
    ../interfaces/ILayoutManager.h

# Resources
RESOURCES += \
    ../resources/ui_resources.qrc

# Test data files
OTHER_FILES += \
    data/test_config.json \
    data/test_theme.qss \
    README.md \
    run_tests.sh \
    run_tests.bat

# Compiler flags
QMAKE_CXXFLAGS += -std=c++17

# Platform-specific configurations
win32 {
    CONFIG += console
    DEFINES += WIN32_LEAN_AND_MEAN
    QMAKE_CXXFLAGS += /W3
}

unix:!macx {
    QMAKE_CXXFLAGS += -Wall -Wextra
    CONFIG += link_pkgconfig
}

macx {
    QMAKE_CXXFLAGS += -Wall -Wextra
    CONFIG += app_bundle
}

# Test runner targets
test.target = test
test.commands = ./$$TARGET
test.depends = $$TARGET
QMAKE_EXTRA_TARGETS += test

# Clean target
QMAKE_CLEAN += \
    $$DESTDIR/$$TARGET \
    $$OBJECTS_DIR/*.o \
    $$MOC_DIR/*.cpp \
    $$RCC_DIR/*.cpp \
    $$UI_DIR/*.h

# Install configuration
target.path = /usr/local/bin/tests
INSTALLS += target

message("✓ UI Module Tests configuration loaded")
message("Target: $$TARGET")
message("Version: $$VERSION")
message("Build mode: $$CONFIG")
message("Qt version: $$QT_VERSION")
message("Destination: $$DESTDIR")