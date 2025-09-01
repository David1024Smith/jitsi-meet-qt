cmake_minimum_required(VERSION 3.16)

project(SettingsModule)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Xml)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/config
)

# Source files - only existing files
set(SETTINGS_SOURCES
    # Core settings files
    src/SettingsModule.cpp
    src/SettingsManager.cpp
    src/PreferencesHandler.cpp
    
    # Validators
    validators/ConfigValidator.cpp
    validators/SchemaValidator.cpp
    
    # Widgets
    widgets/SettingsWidget.cpp
    widgets/PreferencesDialog.cpp
    widgets/ConfigEditor.cpp
    
    # Storage
    storage/LocalStorage.cpp
    storage/RegistryStorage.cpp
    storage/CloudStorage.cpp
    
    # Config
    config/SettingsConfig.cpp
)

# Header files - only existing files
set(SETTINGS_HEADERS
    # Core headers
    include/SettingsModule.h
    include/SettingsManager.h
    include/PreferencesHandler.h
    
    # Validator headers
    validators/ConfigValidator.h
    validators/SchemaValidator.h
    
    # Widget headers
    widgets/SettingsWidget.h
    widgets/PreferencesDialog.h
    widgets/ConfigEditor.h
    
    # Storage headers
    storage/LocalStorage.h
    storage/RegistryStorage.h
    storage/CloudStorage.h
    
    # Config headers
    config/SettingsConfig.h
    
    # Interface headers
    interfaces/ISettingsManager.h
    interfaces/IConfigValidator.h
    interfaces/IPreferencesHandler.h
)

# Create the settings library
add_library(SettingsModule STATIC
    ${SETTINGS_SOURCES}
    ${SETTINGS_HEADERS}
)

# Link Qt libraries
target_link_libraries(SettingsModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
)

# Set target properties
set_target_properties(SettingsModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(SettingsModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)