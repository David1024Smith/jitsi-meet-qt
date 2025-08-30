cmake_minimum_required(VERSION 3.16)
project(UIModuleTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../config
    ${CMAKE_CURRENT_SOURCE_DIR}/../themes
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/../layouts
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# UI Module source files
set(UI_MODULE_SOURCES
    ../src/UIModule.cpp
    ../src/UIManager.cpp
    ../src/ThemeManager.cpp
    ../src/ThemeFactory.cpp
    ../config/UIConfig.cpp
    ../themes/BaseTheme.cpp
    ../themes/DefaultTheme.cpp
    ../themes/DarkTheme.cpp
    ../themes/LightTheme.cpp
    ../widgets/BaseWidget.cpp
    ../widgets/CustomButton.cpp
    ../widgets/StatusBar.cpp
    ../widgets/ToolBar.cpp
    ../layouts/BaseLayout.cpp
    ../layouts/MainLayout.cpp
    ../layouts/ConferenceLayout.cpp
    ../layouts/SettingsLayout.cpp
)

# UI Module header files
set(UI_MODULE_HEADERS
    ../include/UIModule.h
    ../include/UIManager.h
    ../include/ThemeManager.h
    ../include/ThemeFactory.h
    ../config/UIConfig.h
    ../themes/BaseTheme.h
    ../themes/DefaultTheme.h
    ../themes/DarkTheme.h
    ../themes/LightTheme.h
    ../widgets/BaseWidget.h
    ../widgets/CustomButton.h
    ../widgets/StatusBar.h
    ../widgets/ToolBar.h
    ../layouts/BaseLayout.h
    ../layouts/MainLayout.h
    ../layouts/ConferenceLayout.h
    ../layouts/SettingsLayout.h
    ../interfaces/IUIManager.h
    ../interfaces/IThemeManager.h
    ../interfaces/ILayoutManager.h
)

# Test source files
set(TEST_SOURCES
    UIModuleTest.cpp
    widgets/UIComponentsTest.cpp
    mocks/MockTheme.cpp
    mocks/MockWidget.cpp
)

# Test header files
set(TEST_HEADERS
    UIModuleTest.h
    mocks/MockTheme.h
    mocks/MockWidget.h
)

# Create test executable
add_executable(ui_module_tests
    ${TEST_SOURCES}
    ${TEST_HEADERS}
    ${UI_MODULE_SOURCES}
    ${UI_MODULE_HEADERS}
)

# Link Qt libraries
target_link_libraries(ui_module_tests
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
)

# Compiler definitions
target_compile_definitions(ui_module_tests PRIVATE
    UI_MODULE_ENABLED
    UI_MODULE_VERSION="1.0.0"
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
)

# Enable testing
enable_testing()

# Add test cases
add_test(NAME UIModuleTest COMMAND ui_module_tests)
add_test(NAME UIComponentsTest COMMAND ui_module_tests)

# Set test properties
set_tests_properties(UIModuleTest PROPERTIES
    TIMEOUT 300
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

set_tests_properties(UIComponentsTest PROPERTIES
    TIMEOUT 300
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Custom target for running tests
add_custom_target(run_ui_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS ui_module_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Install test executable
install(TARGETS ui_module_tests
    RUNTIME DESTINATION bin/tests
)

# Debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(ui_module_tests PRIVATE UI_MODULE_DEBUG)
    set_target_properties(ui_module_tests PROPERTIES
        COMPILE_FLAGS "-g -O0"
    )
endif()

# Release configuration
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(ui_module_tests PRIVATE UI_MODULE_RELEASE)
    set_target_properties(ui_module_tests PROPERTIES
        COMPILE_FLAGS "-O2 -DNDEBUG"
    )
endif()

# Code coverage (if enabled)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(ui_module_tests PRIVATE --coverage)
    target_link_options(ui_module_tests PRIVATE --coverage)
endif()

# Memory sanitizer (if enabled)
option(ENABLE_SANITIZER "Enable address sanitizer" OFF)
if(ENABLE_SANITIZER)
    target_compile_options(ui_module_tests PRIVATE -fsanitize=address)
    target_link_options(ui_module_tests PRIVATE -fsanitize=address)
endif()

message(STATUS "UI Module Tests configured successfully")
message(STATUS "Test executable: ui_module_tests")
message(STATUS "Run tests with: make run_ui_tests")