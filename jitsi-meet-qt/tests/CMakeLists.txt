cmake_minimum_required(VERSION 3.16)

# Basic test configuration
enable_testing()

# Find Qt Test module
find_package(Qt6 REQUIRED COMPONENTS Test)

# Test sources (placeholder)
set(TEST_SOURCES
    # Add test files here when available
)

# Create test executable if sources exist
if(TEST_SOURCES)
    add_executable(JitsiMeetQtTests ${TEST_SOURCES})
    
    target_link_libraries(JitsiMeetQtTests
        Qt6::Test
        Qt6::Core
    )
    
    add_test(NAME JitsiMeetQtTests COMMAND JitsiMeetQtTests)
endif()

message(STATUS "Tests configuration completed")