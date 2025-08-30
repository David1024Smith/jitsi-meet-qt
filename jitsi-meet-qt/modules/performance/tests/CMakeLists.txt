cmake_minimum_required(VERSION 3.16)

project(PerformanceModuleTests)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Test)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../config
    ${CMAKE_CURRENT_SOURCE_DIR}/../monitors
    ${CMAKE_CURRENT_SOURCE_DIR}/../optimizers
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils
)

# Source files for the performance module
set(PERFORMANCE_SOURCES
    ../src/PerformanceModule.cpp
    ../src/PerformanceManager.cpp
    ../src/MetricsCollector.cpp
    ../config/PerformanceConfig.cpp
    ../monitors/BaseMonitor.cpp
    ../monitors/CPUMonitor.cpp
    ../monitors/MemoryMonitor.cpp
    ../monitors/NetworkMonitor.cpp
    ../optimizers/BaseOptimizer.cpp
    ../optimizers/StartupOptimizer.cpp
    ../optimizers/MemoryOptimizer.cpp
    ../optimizers/RenderOptimizer.cpp
)

# Test source files
set(TEST_SOURCES
    PerformanceModuleTest.cpp
    PerformanceModuleTest.h
)

# Create test executable
add_executable(PerformanceModuleTests
    ${TEST_SOURCES}
    ${PERFORMANCE_SOURCES}
)

# Link Qt libraries
target_link_libraries(PerformanceModuleTests
    Qt6::Core
    Qt6::Test
)

# Add compiler flags for testing
target_compile_definitions(PerformanceModuleTests PRIVATE
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
)

# Enable testing
enable_testing()

# Add test
add_test(NAME PerformanceModuleTests COMMAND PerformanceModuleTests)

# Set test properties
set_tests_properties(PerformanceModuleTests PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Create test data directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_data)

# Copy test resources if any
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_data/)
endif()

# Performance test specific settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(PerformanceModuleTests PRIVATE DEBUG_BUILD)
endif()

# Platform specific settings
if(WIN32)
    target_compile_definitions(PerformanceModuleTests PRIVATE WIN32_LEAN_AND_MEAN)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(PerformanceModuleTests pthread)
endif()

# Code coverage support (optional)
if(ENABLE_COVERAGE)
    target_compile_options(PerformanceModuleTests PRIVATE --coverage)
    target_link_options(PerformanceModuleTests PRIVATE --coverage)
endif()

# Memory sanitizer support (optional)
if(ENABLE_SANITIZER)
    target_compile_options(PerformanceModuleTests PRIVATE -fsanitize=address)
    target_link_options(PerformanceModuleTests PRIVATE -fsanitize=address)
endif()