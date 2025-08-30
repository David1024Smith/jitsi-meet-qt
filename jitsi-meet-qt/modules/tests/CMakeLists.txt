# CMakeLists.txt for Comprehensive Test Suite
# Requirements: 11.5, 11.6, 12.6

cmake_minimum_required(VERSION 3.16)
project(JitsiMeetQtTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test Network)

# Enable testing
enable_testing()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

# Compiler flags for coverage
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Test Coverage Framework
set(TEST_COVERAGE_SOURCES
    TestCoverageFramework.cpp
    TestCoverageFramework.h
)

add_library(TestCoverageFramework STATIC ${TEST_COVERAGE_SOURCES})
target_link_libraries(TestCoverageFramework 
    Qt6::Core 
    Qt6::Test 
    Qt6::Network
)

# Automated Test Runner
set(AUTOMATED_RUNNER_SOURCES
    AutomatedTestRunner.cpp
    AutomatedTestRunner.h
)

add_library(AutomatedTestRunner STATIC ${AUTOMATED_RUNNER_SOURCES})
target_link_libraries(AutomatedTestRunner 
    Qt6::Core 
    Qt6::Test 
    Qt6::Network
    TestCoverageFramework
)

# Performance Benchmark Suite
set(PERFORMANCE_BENCHMARK_SOURCES
    PerformanceBenchmarkSuite.h
    # PerformanceBenchmarkSuite.cpp would be added here
)

add_library(PerformanceBenchmarkSuite STATIC ${PERFORMANCE_BENCHMARK_SOURCES})
target_link_libraries(PerformanceBenchmarkSuite 
    Qt6::Core 
    Qt6::Test
)

# Main Test Runner Executable
add_executable(comprehensive_test_runner
    main_test_runner.cpp
)

target_link_libraries(comprehensive_test_runner
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
    TestCoverageFramework
    AutomatedTestRunner
    PerformanceBenchmarkSuite
)

# Integration Test Executable
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration/ModuleIntegrationTest.cpp)
    add_executable(integration_test
        integration/ModuleIntegrationTest.cpp
        integration/ModuleIntegrationTest.h
    )
    
    target_link_libraries(integration_test
        Qt6::Core
        Qt6::Test
        Qt6::Network
    )
    
    # Add module libraries if they exist
    foreach(MODULE core audio video network ui chat screenshare meeting settings utils performance compatibility)
        if(TARGET ${MODULE}Module)
            target_link_libraries(integration_test ${MODULE}Module)
        endif()
    endforeach()
    
    add_test(NAME IntegrationTests COMMAND integration_test)
endif()

# Performance Benchmark Executable
add_executable(performance_benchmark
    performance_benchmark_main.cpp
)

target_link_libraries(performance_benchmark
    Qt6::Core
    Qt6::Test
    PerformanceBenchmarkSuite
)

add_test(NAME PerformanceBenchmarks COMMAND performance_benchmark)

# Unit Tests for each module
set(MODULES core audio video network ui chat screenshare meeting settings utils performance compatibility)

foreach(MODULE ${MODULES})
    set(MODULE_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../${MODULE}/tests)
    
    if(EXISTS ${MODULE_TEST_DIR})
        # Find all test source files for this module
        file(GLOB MODULE_TEST_SOURCES 
            ${MODULE_TEST_DIR}/*Test.cpp
            ${MODULE_TEST_DIR}/*test.cpp
            ${MODULE_TEST_DIR}/Test*.cpp
        )
        
        file(GLOB MODULE_TEST_HEADERS
            ${MODULE_TEST_DIR}/*Test.h
            ${MODULE_TEST_DIR}/*test.h
            ${MODULE_TEST_DIR}/Test*.h
        )
        
        if(MODULE_TEST_SOURCES)
            # Create test executable for this module
            add_executable(${MODULE}_tests ${MODULE_TEST_SOURCES} ${MODULE_TEST_HEADERS})
            
            target_link_libraries(${MODULE}_tests
                Qt6::Core
                Qt6::Test
                Qt6::Network
            )
            
            # Link module-specific libraries
            if(TARGET ${MODULE}Module)
                target_link_libraries(${MODULE}_tests ${MODULE}Module)
            endif()
            
            # Include module directories
            target_include_directories(${MODULE}_tests PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/../${MODULE}/include
                ${CMAKE_CURRENT_SOURCE_DIR}/../${MODULE}/src
                ${MODULE_TEST_DIR}
            )
            
            # Add test to CTest
            add_test(NAME ${MODULE}_UnitTests COMMAND ${MODULE}_tests)
            
            # Set test properties
            set_tests_properties(${MODULE}_UnitTests PROPERTIES
                TIMEOUT 300
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            )
        endif()
    endif()
endforeach()

# Custom targets for different test types
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "UnitTests" --output-on-failure
    DEPENDS ${MODULES}
    COMMENT "Running all unit tests"
)

add_custom_target(run_integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "IntegrationTests" --output-on-failure
    DEPENDS integration_test
    COMMENT "Running integration tests"
)

add_custom_target(run_performance_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "PerformanceBenchmarks" --output-on-failure
    DEPENDS performance_benchmark
    COMMENT "Running performance benchmarks"
)

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS run_unit_tests run_integration_tests run_performance_tests
    COMMENT "Running all tests"
)

# Coverage target
if(ENABLE_COVERAGE)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            # Cleanup lcov
            COMMAND ${LCOV_PATH} --directory . --zerocounters
            
            # Run tests
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            
            # Capture coverage data
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            
            # Remove external libraries and test files
            COMMAND ${LCOV_PATH} --remove coverage.info 
                '*/tests/*' '*/examples/*' '*/mocks/*' '/usr/*' '*/Qt*/*'
                --output-file coverage_filtered.info
            
            # Generate HTML report
            COMMAND ${GENHTML_PATH} coverage_filtered.info --output-directory coverage_html
            
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report"
        )
        
        add_dependencies(coverage run_all_tests)
    endif()
endif()

# Install test executables
install(TARGETS comprehensive_test_runner
    RUNTIME DESTINATION bin
)

if(TARGET integration_test)
    install(TARGETS integration_test
        RUNTIME DESTINATION bin
    )
endif()

install(TARGETS performance_benchmark
    RUNTIME DESTINATION bin
)

# Install test configuration
install(FILES test_config.json
    DESTINATION share/jitsi-meet-qt/tests
)

# Install test scripts
install(PROGRAMS run_comprehensive_tests.py
    DESTINATION bin
)

# Print configuration summary
message(STATUS "Test Configuration Summary:")
message(STATUS "  Coverage enabled: ${ENABLE_COVERAGE}")
message(STATUS "  Qt version: ${Qt6_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")

# Add custom command to run Python test runner
add_custom_target(run_comprehensive_tests
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/run_comprehensive_tests.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running comprehensive test suite with Python runner"
)