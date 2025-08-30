# Module Integration Test CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

project(ModuleIntegrationTest)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Test Network)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../settings/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../performance/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../network/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ui/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../chat/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../screenshare/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../meeting/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../audio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../camera/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../compatibility/include
)

# Source files
set(SOURCES
    ModuleIntegrationTest.cpp
    ModuleIntegrationTest.h
)

# Create test executable
add_executable(ModuleIntegrationTest ${SOURCES})

# Link Qt libraries
target_link_libraries(ModuleIntegrationTest
    Qt6::Core
    Qt6::Test
    Qt6::Network
)

# Conditionally link module libraries if they exist
if(TARGET UtilsModule)
    target_link_libraries(ModuleIntegrationTest UtilsModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE UTILS_MODULE_AVAILABLE)
endif()

if(TARGET SettingsModule)
    target_link_libraries(ModuleIntegrationTest SettingsModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE SETTINGS_MODULE_AVAILABLE)
endif()

if(TARGET PerformanceModule)
    target_link_libraries(ModuleIntegrationTest PerformanceModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE PERFORMANCE_MODULE_AVAILABLE)
endif()

if(TARGET NetworkModule)
    target_link_libraries(ModuleIntegrationTest NetworkModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE NETWORK_MODULE_AVAILABLE)
endif()

if(TARGET UIModule)
    target_link_libraries(ModuleIntegrationTest UIModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE UI_MODULE_AVAILABLE)
endif()

if(TARGET ChatModule)
    target_link_libraries(ModuleIntegrationTest ChatModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE CHAT_MODULE_AVAILABLE)
endif()

if(TARGET ScreenShareModule)
    target_link_libraries(ModuleIntegrationTest ScreenShareModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE SCREENSHARE_MODULE_AVAILABLE)
endif()

if(TARGET MeetingModule)
    target_link_libraries(ModuleIntegrationTest MeetingModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE MEETING_MODULE_AVAILABLE)
endif()

if(TARGET AudioModule)
    target_link_libraries(ModuleIntegrationTest AudioModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE AUDIO_MODULE_AVAILABLE)
endif()

if(TARGET CameraModule)
    target_link_libraries(ModuleIntegrationTest CameraModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE CAMERA_MODULE_AVAILABLE)
endif()

if(TARGET CompatibilityModule)
    target_link_libraries(ModuleIntegrationTest CompatibilityModule)
    target_compile_definitions(ModuleIntegrationTest PRIVATE COMPATIBILITY_MODULE_AVAILABLE)
endif()

# Add test to CTest
add_test(NAME ModuleIntegrationTest COMMAND ModuleIntegrationTest)

# Set test properties
set_tests_properties(ModuleIntegrationTest PROPERTIES
    TIMEOUT 300  # 5 minutes timeout
    LABELS "integration;modules"
)

# Enable testing
enable_testing()

# Custom target for running integration tests
add_custom_target(run_integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L integration
    DEPENDS ModuleIntegrationTest
    COMMENT "Running module integration tests"
)

# Install test executable (optional)
install(TARGETS ModuleIntegrationTest
    RUNTIME DESTINATION bin/tests
)