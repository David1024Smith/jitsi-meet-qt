# UI Module Configuration
# Jitsi Meet Qt - UI Module v1.0.0

UI_MODULE_VERSION = 1.0.0
DEFINES += UI_MODULE_VERSION=\\\"$$UI_MODULE_VERSION\\\"
DEFINES += UI_MODULE_ENABLED

message("Loading UI Module v$$UI_MODULE_VERSION...")

# Module paths
UI_MODULE_PATH = $$PWD
UI_INCLUDE_PATH = $$UI_MODULE_PATH/include
UI_SRC_PATH = $$UI_MODULE_PATH/src
UI_INTERFACES_PATH = $$UI_MODULE_PATH/interfaces
UI_CONFIG_PATH = $$UI_MODULE_PATH/config
UI_THEMES_PATH = $$UI_MODULE_PATH/themes
UI_WIDGETS_PATH = $$UI_MODULE_PATH/widgets
UI_LAYOUTS_PATH = $$UI_MODULE_PATH/layouts
UI_UTILS_PATH = $$UI_MODULE_PATH/utils
UI_TESTS_PATH = $$UI_MODULE_PATH/tests
UI_EXAMPLES_PATH = $$UI_MODULE_PATH/examples
UI_RESOURCES_PATH = $$UI_MODULE_PATH/resources

# Include paths
INCLUDEPATH += \
    $$UI_INCLUDE_PATH \
    $$UI_INTERFACES_PATH \
    $$UI_CONFIG_PATH \
    $$UI_THEMES_PATH \
    $$UI_WIDGETS_PATH \
    $$UI_LAYOUTS_PATH \
    $$UI_UTILS_PATH

# Headers
HEADERS += \
    # Core headers
    $$UI_INCLUDE_PATH/UIModule.h \
    $$UI_INCLUDE_PATH/UIManager.h \
    $$UI_INCLUDE_PATH/ThemeFactory.h \
    $$UI_INCLUDE_PATH/ThemeManager.h \
    \
    # Interface headers
    $$UI_INTERFACES_PATH/IUIManager.h \
    $$UI_INTERFACES_PATH/IThemeManager.h \
    $$UI_INTERFACES_PATH/ILayoutManager.h \
    \
    # Configuration headers
    $$UI_CONFIG_PATH/UIConfig.h \
    \
    # Theme headers
    $$UI_THEMES_PATH/BaseTheme.h \
    $$UI_THEMES_PATH/DefaultTheme.h \
    $$UI_THEMES_PATH/DarkTheme.h \
    $$UI_THEMES_PATH/LightTheme.h \
    \
    # Widget headers
    $$UI_WIDGETS_PATH/BaseWidget.h \
    $$UI_WIDGETS_PATH/CustomButton.h \
    $$UI_WIDGETS_PATH/StatusBar.h \
    $$UI_WIDGETS_PATH/ToolBar.h \
    \
    # Layout headers
    $$UI_LAYOUTS_PATH/MainLayout.h \
    $$UI_LAYOUTS_PATH/ConferenceLayout.h \
    $$UI_LAYOUTS_PATH/SettingsLayout.h

# Sources
SOURCES += \
    # Core sources
    $$UI_SRC_PATH/UIModule.cpp \
    $$UI_SRC_PATH/UIManager.cpp \
    $$UI_SRC_PATH/ThemeFactory.cpp \
    $$UI_SRC_PATH/ThemeManager.cpp \
    \
    # Configuration sources
    $$UI_CONFIG_PATH/UIConfig.cpp \
    \
    # Theme sources
    $$UI_THEMES_PATH/BaseTheme.cpp \
    $$UI_THEMES_PATH/DefaultTheme.cpp \
    $$UI_THEMES_PATH/DarkTheme.cpp \
    $$UI_THEMES_PATH/LightTheme.cpp \
    \
    # Widget sources
    $$UI_WIDGETS_PATH/BaseWidget.cpp \
    $$UI_WIDGETS_PATH/CustomButton.cpp \
    $$UI_WIDGETS_PATH/StatusBar.cpp \
    $$UI_WIDGETS_PATH/ToolBar.cpp \
    \
    # Layout sources
    $$UI_LAYOUTS_PATH/MainLayout.cpp \
    $$UI_LAYOUTS_PATH/ConferenceLayout.cpp \
    $$UI_LAYOUTS_PATH/SettingsLayout.cpp

# Resources
RESOURCES += \
    $$UI_RESOURCES_PATH/ui_resources.qrc

# Qt modules required for UI
QT += widgets gui core

# Compiler flags for UI module
DEFINES += UI_MODULE_EXPORTS

# Debug configuration
CONFIG(debug, debug|release) {
    DEFINES += UI_MODULE_DEBUG
    message("UI Module: Debug mode enabled")
}

# Release configuration
CONFIG(release, debug|release) {
    DEFINES += UI_MODULE_RELEASE
    message("UI Module: Release mode enabled")
}

message("✓ UI Module configuration loaded successfully")