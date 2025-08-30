cmake_minimum_required(VERSION 3.16)

project(MeetingModuleTests)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test Network Concurrent)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../models
    ${CMAKE_CURRENT_SOURCE_DIR}/../handlers
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/../config
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

# Test sources
set(TEST_SOURCES
    MeetingModuleTest.cpp
    MeetingModuleTest.h
    mocks/MockMeetingManager.cpp
    mocks/MockMeetingManager.h
)

# Meeting module sources (to be tested)
set(MODULE_SOURCES
    ../src/MeetingModule.cpp
    ../src/MeetingManager.cpp
    ../src/LinkHandler.cpp
    ../config/MeetingConfig.cpp
    ../models/Meeting.cpp
    ../models/Room.cpp
    ../models/Invitation.cpp
    ../handlers/URLHandler.cpp
    ../handlers/ProtocolHandler.cpp
    ../handlers/AuthHandler.cpp
    ../widgets/MeetingWidget.cpp
    ../widgets/JoinDialog.cpp
    ../widgets/CreateDialog.cpp
)

# Meeting module headers
set(MODULE_HEADERS
    ../include/MeetingModule.h
    ../include/MeetingManager.h
    ../include/LinkHandler.h
    ../config/MeetingConfig.h
    ../models/Meeting.h
    ../models/Room.h
    ../models/Invitation.h
    ../handlers/URLHandler.h
    ../handlers/ProtocolHandler.h
    ../handlers/AuthHandler.h
    ../widgets/MeetingWidget.h
    ../widgets/JoinDialog.h
    ../widgets/CreateDialog.h
    ../interfaces/IMeetingManager.h
    ../interfaces/ILinkHandler.h
    ../interfaces/IRoomManager.h
)

# Create test executable
add_executable(MeetingModuleTests
    ${TEST_SOURCES}
    ${MODULE_SOURCES}
    ${MODULE_HEADERS}
)

# Link Qt libraries
target_link_libraries(MeetingModuleTests
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
    Qt6::Network
    Qt6::Concurrent
)

# Compiler definitions
target_compile_definitions(MeetingModuleTests PRIVATE
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
    MEETING_MODULE_TEST_BUILD
)

# Enable testing
enable_testing()

# Add test cases
add_test(NAME MeetingModuleTests COMMAND MeetingModuleTests)

# Test data files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/test_meetings.json
    ${CMAKE_CURRENT_BINARY_DIR}/data/test_meetings.json
    COPYONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/test_configurations.json
    ${CMAKE_CURRENT_BINARY_DIR}/data/test_configurations.json
    COPYONLY
)

# Set test properties
set_tests_properties(MeetingModuleTests PROPERTIES
    TIMEOUT 300
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Coverage support (optional)
if(ENABLE_COVERAGE)
    target_compile_options(MeetingModuleTests PRIVATE --coverage)
    target_link_options(MeetingModuleTests PRIVATE --coverage)
endif()

# Install test executable (optional)
if(INSTALL_TESTS)
    install(TARGETS MeetingModuleTests
        RUNTIME DESTINATION bin/tests
    )
    
    install(FILES
        data/test_meetings.json
        data/test_configurations.json
        DESTINATION bin/tests/data
    )
endif()

# Custom target for running tests
add_custom_target(run_meeting_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS MeetingModuleTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running Meeting Module Tests"
)

# Custom target for running tests with verbose output
add_custom_target(run_meeting_tests_verbose
    COMMAND MeetingModuleTests -v2
    DEPENDS MeetingModuleTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running Meeting Module Tests (Verbose)"
)

# Memory check target (if valgrind is available)
find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    add_custom_target(run_meeting_tests_memcheck
        COMMAND ${VALGRIND_EXECUTABLE} 
            --tool=memcheck 
            --leak-check=full 
            --show-leak-kinds=all 
            --track-origins=yes 
            --verbose 
            $<TARGET_FILE:MeetingModuleTests>
        DEPENDS MeetingModuleTests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running Meeting Module Tests with Memory Check"
    )
endif()

# Performance profiling target (if perf is available)
find_program(PERF_EXECUTABLE perf)
if(PERF_EXECUTABLE)
    add_custom_target(run_meeting_tests_profile
        COMMAND ${PERF_EXECUTABLE} record -g $<TARGET_FILE:MeetingModuleTests>
        DEPENDS MeetingModuleTests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running Meeting Module Tests with Performance Profiling"
    )
endif()

# Code coverage report target (if gcov/lcov is available)
if(ENABLE_COVERAGE)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)
    
    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        add_custom_target(meeting_tests_coverage
            COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_EXECUTABLE} --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND ${LCOV_EXECUTABLE} --list coverage.info
            COMMAND ${GENHTML_EXECUTABLE} coverage.info --output-directory coverage_report
            DEPENDS run_meeting_tests
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating Meeting Module Test Coverage Report"
        )
    endif()
endif()

# Benchmark target for performance tests
add_custom_target(run_meeting_benchmarks
    COMMAND MeetingModuleTests -benchmark
    DEPENDS MeetingModuleTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running Meeting Module Benchmarks"
)

# Test with different Qt platforms
add_custom_target(run_meeting_tests_all_platforms
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen MeetingModuleTests
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=minimal MeetingModuleTests
    DEPENDS MeetingModuleTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running Meeting Module Tests on All Platforms"
)