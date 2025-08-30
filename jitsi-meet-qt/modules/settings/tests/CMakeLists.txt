# Settings Module Tests CMake Configuration
# Jitsi Meet Qt - Settings Module Tests

cmake_minimum_required(VERSION 3.16)
project(SettingsModuleTests)

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Test)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ../include
    ../interfaces
    ../config
    ../storage
    ../validators
    ../widgets
)

# Test source files
set(TEST_SOURCES
    SettingsModuleTest.cpp
    StorageBackendTest.cpp
    UIComponentTest.cpp
    ValidationTest.cpp
)

# Test header files
set(TEST_HEADERS
    SettingsModuleTest.h
    StorageBackendTest.h
    UIComponentTest.h
    ValidationTest.h
)

# Module source files
set(MODULE_SOURCES
    ../src/SettingsModule.cpp
    ../src/SettingsManager.cpp
    ../src/PreferencesHandler.cpp
    ../config/SettingsConfig.cpp
    ../storage/LocalStorage.cpp
    ../storage/CloudStorage.cpp
    ../storage/RegistryStorage.cpp
    ../validators/ConfigValidator.cpp
    ../validators/SchemaValidator.cpp
    ../widgets/SettingsWidget.cpp
    ../widgets/PreferencesDialog.cpp
    ../widgets/ConfigEditor.cpp
)

# Module header files
set(MODULE_HEADERS
    ../include/SettingsModule.h
    ../include/SettingsManager.h
    ../include/PreferencesHandler.h
    ../interfaces/ISettingsManager.h
    ../interfaces/IPreferencesHandler.h
    ../interfaces/IConfigValidator.h
    ../config/SettingsConfig.h
    ../storage/LocalStorage.h
    ../storage/CloudStorage.h
    ../storage/RegistryStorage.h
    ../validators/ConfigValidator.h
    ../validators/SchemaValidator.h
    ../widgets/SettingsWidget.h
    ../widgets/PreferencesDialog.h
    ../widgets/ConfigEditor.h
)

# Create individual test executables
add_executable(settings_module_tests 
    SettingsModuleTest.cpp 
    SettingsModuleTest.h
    ${MODULE_SOURCES} 
    ${MODULE_HEADERS}
)

add_executable(storage_backend_tests 
    StorageBackendTest.cpp 
    StorageBackendTest.h
    ${MODULE_SOURCES} 
    ${MODULE_HEADERS}
)

add_executable(ui_component_tests 
    UIComponentTest.cpp 
    UIComponentTest.h
    ${MODULE_SOURCES} 
    ${MODULE_HEADERS}
)

add_executable(validation_tests 
    ValidationTest.cpp 
    ValidationTest.h
    ${MODULE_SOURCES} 
    ${MODULE_HEADERS}
)

# Create comprehensive test executable
add_executable(settings_tests 
    ${TEST_SOURCES} 
    ${TEST_HEADERS}
    ${MODULE_SOURCES} 
    ${MODULE_HEADERS}
)

# Link Qt libraries for all test executables
set(TEST_TARGETS 
    settings_module_tests 
    storage_backend_tests 
    ui_component_tests 
    validation_tests 
    settings_tests
)

foreach(TARGET ${TEST_TARGETS})
    target_link_libraries(${TARGET}
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Test
    )
    
    # Compile definitions for each target
    target_compile_definitions(${TARGET} PRIVATE
        SETTINGS_MODULE_VERSION="1.0.0"
        SETTINGS_MODULE_AVAILABLE
        SETTINGS_DEBUG_MODE
    )
    
    # Platform-specific definitions
    if(WIN32)
        target_compile_definitions(${TARGET} PRIVATE SETTINGS_WINDOWS_REGISTRY)
    elseif(UNIX AND NOT APPLE)
        target_compile_definitions(${TARGET} PRIVATE SETTINGS_LINUX_CONFIG)
    elseif(APPLE)
        target_compile_definitions(${TARGET} PRIVATE SETTINGS_MACOS_PLIST)
    endif()
endforeach()

# Copy test data
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Enable testing
enable_testing()

# Add individual tests
add_test(NAME SettingsModuleTests COMMAND settings_module_tests)
add_test(NAME StorageBackendTests COMMAND storage_backend_tests)
add_test(NAME UIComponentTests COMMAND ui_component_tests)
add_test(NAME ValidationTests COMMAND validation_tests)
add_test(NAME ComprehensiveSettingsTests COMMAND settings_tests)