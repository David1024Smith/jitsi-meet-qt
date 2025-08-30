# Chat Module Tests CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

project(ChatModuleTests)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test Network)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../models
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/../storage
    ${CMAKE_CURRENT_SOURCE_DIR}/../config
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../network/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ui/include
)

# Source files for the chat module (to be tested)
set(CHAT_MODULE_SOURCES
    ../src/ChatModule.cpp
    ../src/ChatManager.cpp
    ../src/MessageHandler.cpp
    ../storage/MessageStorage.cpp
    ../storage/HistoryManager.cpp
    ../models/ChatMessage.cpp
    ../models/ChatRoom.cpp
    ../models/Participant.cpp
    ../widgets/ChatWidget.cpp
    ../widgets/MessageList.cpp
    ../widgets/InputWidget.cpp
    ../config/ChatConfig.cpp
)

# Header files for the chat module
set(CHAT_MODULE_HEADERS
    ../include/ChatModule.h
    ../include/ChatManager.h
    ../include/MessageHandler.h
    ../storage/MessageStorage.h
    ../storage/HistoryManager.h
    ../models/ChatMessage.h
    ../models/ChatRoom.h
    ../models/Participant.h
    ../widgets/ChatWidget.h
    ../widgets/MessageList.h
    ../widgets/InputWidget.h
    ../config/ChatConfig.h
    ../interfaces/IChatManager.h
    ../interfaces/IMessageHandler.h
    ../interfaces/IMessageStorage.h
)

# Test source files
set(TEST_SOURCES
    ChatModuleTest.cpp
    ChatModuleTest.h
)

# Mock source files
set(MOCK_SOURCES
    mocks/MockChatManager.cpp
    mocks/MockMessageHandler.cpp
    mocks/MockMessageStorage.cpp
    mocks/MockNetworkManager.cpp
)

# Create mock files if they don't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/mocks)

# Create the test executable
add_executable(ChatModuleTests
    ${TEST_SOURCES}
    ${CHAT_MODULE_SOURCES}
    ${CHAT_MODULE_HEADERS}
    ${MOCK_SOURCES}
)

# Link Qt libraries
target_link_libraries(ChatModuleTests
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
    Qt6::Network
)

# Add compiler definitions
target_compile_definitions(ChatModuleTests PRIVATE
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
    CHAT_MODULE_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

# Enable testing
enable_testing()

# Add the test
add_test(NAME ChatModuleTests COMMAND ChatModuleTests)

# Set test properties
set_tests_properties(ChatModuleTests PROPERTIES
    TIMEOUT 300
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Custom target for running tests with verbose output
add_custom_target(run_chat_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose --output-on-failure
    DEPENDS ChatModuleTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Custom target for running specific test categories
add_custom_target(run_chat_tests_core
    COMMAND ChatModuleTests --skip-ui --skip-performance --skip-stress
    DEPENDS ChatModuleTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run_chat_tests_ui
    COMMAND ChatModuleTests --skip-performance --skip-stress
    DEPENDS ChatModuleTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run_chat_tests_performance
    COMMAND ChatModuleTests --skip-ui --skip-stress
    DEPENDS ChatModuleTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Install test executable (optional)
install(TARGETS ChatModuleTests
    RUNTIME DESTINATION bin/tests
)

# Create test data directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)

# Copy test data files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/test_messages.json
    ${CMAKE_CURRENT_BINARY_DIR}/data/test_messages.json
    COPYONLY
)

# Generate test report target
add_custom_target(chat_test_report
    COMMAND ChatModuleTests --verbose > ${CMAKE_BINARY_DIR}/chat_test_report.txt 2>&1 || true
    DEPENDS ChatModuleTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Code coverage target (if gcov is available)
find_program(GCOV_PATH gcov)
if(GCOV_PATH)
    add_custom_target(chat_test_coverage
        COMMAND gcov ${CHAT_MODULE_SOURCES}
        DEPENDS ChatModuleTests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Memory check target (if valgrind is available)
find_program(VALGRIND_PATH valgrind)
if(VALGRIND_PATH)
    add_custom_target(chat_test_memcheck
        COMMAND ${VALGRIND_PATH} --tool=memcheck --leak-check=full --show-leak-kinds=all 
                --track-origins=yes --verbose --log-file=chat_memcheck.log 
                $<TARGET_FILE:ChatModuleTests>
        DEPENDS ChatModuleTests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Benchmark target
add_custom_target(chat_test_benchmark
    COMMAND ChatModuleTests --benchmark
    DEPENDS ChatModuleTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Print configuration summary
message(STATUS "Chat Module Tests Configuration:")
message(STATUS "  Qt Version: ${Qt6_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Test Data Dir: ${CMAKE_CURRENT_SOURCE_DIR}/data")
message(STATUS "  Mock Sources: ${MOCK_SOURCES}")