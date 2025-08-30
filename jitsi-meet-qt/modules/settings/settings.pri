# Settings Module Configuration
# Jitsi Meet Qt - Settings Module v1.0.0

SETTINGS_MODULE_VERSION = 1.0.0
DEFINES += SETTINGS_MODULE_VERSION=\\\"$SETTINGS_MODULE_VERSION\\\"
DEFINES += SETTINGS_MODULE_AVAILABLE

message("Loading Settings Module v$SETTINGS_MODULE_VERSION...")

# Module paths
SETTINGS_MODULE_PATH = $$PWD
INCLUDEPATH += $$SETTINGS_MODULE_PATH/include
INCLUDEPATH += $$SETTINGS_MODULE_PATH/interfaces
INCLUDEPATH += $$SETTINGS_MODULE_PATH/config
INCLUDEPATH += $$SETTINGS_MODULE_PATH/storage
INCLUDEPATH += $$SETTINGS_MODULE_PATH/validators
INCLUDEPATH += $$SETTINGS_MODULE_PATH/widgets

# Headers
HEADERS += \
    # Core headers
    $$SETTINGS_MODULE_PATH/include/SettingsModule.h \
    $$SETTINGS_MODULE_PATH/include/SettingsManager.h \
    $$SETTINGS_MODULE_PATH/include/PreferencesHandler.h \
    \
    # Interface headers
    $$SETTINGS_MODULE_PATH/interfaces/ISettingsManager.h \
    $$SETTINGS_MODULE_PATH/interfaces/IPreferencesHandler.h \
    $$SETTINGS_MODULE_PATH/interfaces/IConfigValidator.h \
    \
    # Configuration headers
    $$SETTINGS_MODULE_PATH/config/SettingsConfig.h \
    \
    # Storage backend headers
    $$SETTINGS_MODULE_PATH/storage/LocalStorage.h \
    $$SETTINGS_MODULE_PATH/storage/CloudStorage.h \
    $$SETTINGS_MODULE_PATH/storage/RegistryStorage.h \
    \
    # Validator headers
    $$SETTINGS_MODULE_PATH/validators/ConfigValidator.h \
    $$SETTINGS_MODULE_PATH/validators/SchemaValidator.h \
    \
    # Widget headers
    $$SETTINGS_MODULE_PATH/widgets/SettingsWidget.h \
    $$SETTINGS_MODULE_PATH/widgets/PreferencesDialog.h \
    $$SETTINGS_MODULE_PATH/widgets/ConfigEditor.h

# Sources
SOURCES += \
    # Core sources
    $$SETTINGS_MODULE_PATH/src/SettingsModule.cpp \
    $$SETTINGS_MODULE_PATH/src/SettingsManager.cpp \
    $$SETTINGS_MODULE_PATH/src/PreferencesHandler.cpp \
    \
    # Configuration sources
    $$SETTINGS_MODULE_PATH/config/SettingsConfig.cpp \
    \
    # Storage backend sources
    $$SETTINGS_MODULE_PATH/storage/LocalStorage.cpp \
    $$SETTINGS_MODULE_PATH/storage/CloudStorage.cpp \
    $$SETTINGS_MODULE_PATH/storage/RegistryStorage.cpp \
    \
    # Validator sources
    $$SETTINGS_MODULE_PATH/validators/ConfigValidator.cpp \
    $$SETTINGS_MODULE_PATH/validators/SchemaValidator.cpp \
    \
    # Widget sources
    $$SETTINGS_MODULE_PATH/widgets/SettingsWidget.cpp \
    $$SETTINGS_MODULE_PATH/widgets/PreferencesDialog.cpp \
    $$SETTINGS_MODULE_PATH/widgets/ConfigEditor.cpp

# Resources
RESOURCES += $$SETTINGS_MODULE_PATH/resources/settings_resources.qrc

# Dependencies
QT += core widgets network

# Conditional compilation
CONFIG(debug, debug|release) {
    DEFINES += SETTINGS_DEBUG_MODE
    message("Settings Module: Debug mode enabled")
}

# Platform-specific settings
win32 {
    DEFINES += SETTINGS_WINDOWS_REGISTRY
    message("Settings Module: Windows Registry support enabled")
}

unix:!mac {
    DEFINES += SETTINGS_LINUX_CONFIG
    message("Settings Module: Linux config support enabled")
}

macx {
    DEFINES += SETTINGS_MACOS_PLIST
    message("Settings Module: macOS plist support enabled")
}

# Module information
message("✓ Settings Module loaded successfully")
message("  - Version: $SETTINGS_MODULE_VERSION")
message("  - Features: Local/Cloud/Registry storage, Config validation")
message("  - UI Components: Settings widget, Preferences dialog, Config editor")