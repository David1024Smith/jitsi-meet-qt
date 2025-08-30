cmake_minimum_required(VERSION 3.16)

project(CompatibilityModule)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/config
)

# Source files
set(COMPATIBILITY_SOURCES
    # Core compatibility files
    src/CompatibilityModule.cpp
    src/LegacyCompatibilityAdapter.cpp
    src/CompatibilityValidator.cpp
    src/RollbackManager.cpp
    
    # Progressive replacement system
    src/ProgressiveReplacementManager.cpp
    
    # Configuration
    config/CompatibilityConfig.cpp
    config/ReplacementConfig.cpp
    
    # Validators
    validators/FunctionValidator.cpp
    validators/PerformanceValidator.cpp
    
    # Rollback system
    rollback/StateBackup.cpp
    rollback/CheckpointManager.cpp
    
    # Adapters
    adapters/ChatManagerAdapter.cpp
    adapters/MediaManagerAdapter.cpp
    adapters/ConferenceManagerAdapter.cpp
    adapters/ScreenShareManagerAdapter.cpp
)

# Header files
set(COMPATIBILITY_HEADERS
    # Core headers
    include/CompatibilityModule.h
    include/LegacyCompatibilityAdapter.h
    include/CompatibilityValidator.h
    include/RollbackManager.h
    include/ProgressiveReplacementManager.h
    
    # Configuration headers
    config/CompatibilityConfig.h
    config/ReplacementConfig.h
    
    # Validator headers
    validators/FunctionValidator.h
    validators/PerformanceValidator.h
    
    # Rollback headers
    rollback/StateBackup.h
    rollback/CheckpointManager.h
    
    # Adapter headers
    adapters/ChatManagerAdapter.h
    adapters/MediaManagerAdapter.h
    
    # Interface headers
    interfaces/ICompatibilityAdapter.h
    interfaces/ICompatibilityValidator.h
    interfaces/IRollbackManager.h
)

# Create the compatibility library
add_library(CompatibilityModule STATIC
    ${COMPATIBILITY_SOURCES}
    ${COMPATIBILITY_HEADERS}
)

# Link Qt libraries
target_link_libraries(CompatibilityModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# Set target properties
set_target_properties(CompatibilityModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(CompatibilityModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

# Tests
option(BUILD_COMPATIBILITY_TESTS "Build compatibility module tests" ON)

if(BUILD_COMPATIBILITY_TESTS)
    enable_testing()
    
    find_package(Qt6 REQUIRED COMPONENTS Test)
    
    # Test sources
    set(TEST_SOURCES
        tests/CompatibilityModuleTest.cpp
        tests/ProgressiveReplacementTest.cpp
    )
    
    # Create test executable
    add_executable(CompatibilityModuleTests
        ${TEST_SOURCES}
    )
    
    # Link libraries for tests
    target_link_libraries(CompatibilityModuleTests
        CompatibilityModule
        Qt6::Test
        Qt6::Core
        Qt6::Widgets
    )
    
    # Set test properties
    set_target_properties(CompatibilityModuleTests PROPERTIES
        AUTOMOC ON
    )
    
    # Add tests
    add_test(NAME CompatibilityModuleTests COMMAND CompatibilityModuleTests)
endif()

# Examples
option(BUILD_COMPATIBILITY_EXAMPLES "Build compatibility module examples" ON)

if(BUILD_COMPATIBILITY_EXAMPLES)
    # Basic usage example
    add_executable(BasicCompatibilityExample
        examples/basic_usage_example.cpp
    )
    
    target_link_libraries(BasicCompatibilityExample
        CompatibilityModule
        Qt6::Core
        Qt6::Widgets
    )
    
    set_target_properties(BasicCompatibilityExample PROPERTIES
        AUTOMOC ON
    )
    
    # Progressive replacement example
    add_executable(ProgressiveReplacementExample
        examples/progressive_replacement_example.cpp
    )
    
    target_link_libraries(ProgressiveReplacementExample
        CompatibilityModule
        Qt6::Core
        Qt6::Widgets
    )
    
    set_target_properties(ProgressiveReplacementExample PROPERTIES
        AUTOMOC ON
    )
endif()

# Installation
install(TARGETS CompatibilityModule
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${COMPATIBILITY_HEADERS}
    DESTINATION include/compatibility
)

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    if(EXISTS ${DOXYGEN_IN})
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        add_custom_target(compatibility_docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating compatibility module documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()

# Print configuration summary
message(STATUS "Compatibility Module Configuration:")
message(STATUS "  Build tests: ${BUILD_COMPATIBILITY_TESTS}")
message(STATUS "  Build examples: ${BUILD_COMPATIBILITY_EXAMPLES}")
message(STATUS "  Documentation: ${DOXYGEN_FOUND}")