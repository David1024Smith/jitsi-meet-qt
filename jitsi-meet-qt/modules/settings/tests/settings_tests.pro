# Settings Module Test Project
# Jitsi Meet Qt - Settings Module Tests

QT += core widgets network testlib
CONFIG += testcase

TARGET = settings_tests
TEMPLATE = app

# Test configuration
CONFIG += console
CONFIG -= app_bundle

# Include paths
INCLUDEPATH += ../include
INCLUDEPATH += ../interfaces
INCLUDEPATH += ../config
INCLUDEPATH += ../storage
INCLUDEPATH += ../validators
INCLUDEPATH += ../widgets

# Test source files
SOURCES += \
    SettingsModuleTest.cpp \
    StorageBackendTest.cpp \
    UIComponentTest.cpp \
    ValidationTest.cpp

# Test header files
HEADERS += \
    SettingsModuleTest.h \
    StorageBackendTest.h \
    UIComponentTest.h \
    ValidationTest.h

# Module source files
SOURCES += \
    ../src/SettingsModule.cpp \
    ../src/SettingsManager.cpp \
    ../src/PreferencesHandler.cpp \
    ../config/SettingsConfig.cpp \
    ../storage/LocalStorage.cpp \
    ../storage/CloudStorage.cpp \
    ../storage/RegistryStorage.cpp \
    ../validators/ConfigValidator.cpp \
    ../validators/SchemaValidator.cpp \
    ../widgets/SettingsWidget.cpp \
    ../widgets/PreferencesDialog.cpp \
    ../widgets/ConfigEditor.cpp

# Module header files
HEADERS += \
    ../include/SettingsModule.h \
    ../include/SettingsManager.h \
    ../include/PreferencesHandler.h \
    ../interfaces/ISettingsManager.h \
    ../interfaces/IPreferencesHandler.h \
    ../interfaces/IConfigValidator.h \
    ../config/SettingsConfig.h \
    ../storage/LocalStorage.h \
    ../storage/CloudStorage.h \
    ../storage/RegistryStorage.h \
    ../validators/ConfigValidator.h \
    ../validators/SchemaValidator.h \
    ../widgets/SettingsWidget.h \
    ../widgets/PreferencesDialog.h \
    ../widgets/ConfigEditor.h

# Resources
RESOURCES += ../resources/settings_resources.qrc

# Defines
DEFINES += SETTINGS_MODULE_VERSION=\\\"1.0.0\\\"
DEFINES += SETTINGS_MODULE_AVAILABLE
DEFINES += SETTINGS_DEBUG_MODE

# Platform-specific settings
win32 {
    DEFINES += SETTINGS_WINDOWS_REGISTRY
}

unix:!mac {
    DEFINES += SETTINGS_LINUX_CONFIG
}

macx {
    DEFINES += SETTINGS_MACOS_PLIST
}

# Test data
QMAKE_EXTRA_TARGETS += copydata
copydata.commands = $(COPY_DIR) $$PWD/data $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata