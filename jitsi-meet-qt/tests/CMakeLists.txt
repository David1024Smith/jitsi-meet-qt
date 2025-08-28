# Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# Find Qt Test module
find_package(Qt6 REQUIRED COMPONENTS Test)

# Test sources
set(TEST_SOURCES
    test_main.cpp
    test_MainApplication.cpp
)

# Test headers
set(TEST_HEADERS
    ../include/MainApplication.h
)

# Create test executable
add_executable(JitsiMeetQt_Tests ${TEST_SOURCES} ${TEST_HEADERS})

# Link libraries
target_link_libraries(JitsiMeetQt_Tests
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    Qt6::Test
)

# Include directories
target_include_directories(JitsiMeetQt_Tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Add test
add_test(NAME JitsiMeetQt_Tests COMMAND JitsiMeetQt_Tests)