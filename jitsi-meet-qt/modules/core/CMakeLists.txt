# Core Module Management System - CMake Configuration
cmake_minimum_required(VERSION 3.16)

project(JitsiMeetQt_CoreModule VERSION 1.0.0 LANGUAGES CXX)

# Qt requirements
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the core module library
set(CORE_MODULE_SOURCES
    # Main implementation files
    src/ModuleManager.cpp
    src/GlobalModuleConfig.cpp
    src/ModuleHealthMonitor.cpp
    src/ModuleVersionManager.cpp
    
    # Management components
    management/RuntimeController.cpp
    
    # Configuration files
    config/ModuleConfig.cpp
    
    # Monitoring components
    monitoring/HealthChecker.cpp
    monitoring/StatusMonitor.cpp
    
    # Versioning components
    versioning/VersionController.cpp
    versioning/UpgradeManager.cpp
)

set(CORE_MODULE_HEADERS
    # Main headers
    include/ModuleManager.h
    include/GlobalModuleConfig.h
    include/ModuleHealthMonitor.h
    include/ModuleVersionManager.h
    
    # Interface definitions
    interfaces/IModuleManager.h
    interfaces/IModuleConfig.h
    interfaces/IHealthMonitor.h
    interfaces/IVersionManager.h
    
    # Management headers
    management/RuntimeController.h
    
    # Configuration headers
    config/ModuleConfig.h
    
    # Monitoring headers
    monitoring/HealthChecker.h
    monitoring/StatusMonitor.h
    
    # Versioning headers
    versioning/VersionController.h
    versioning/UpgradeManager.h
)

# Create the core module library
add_library(JitsiMeetQt_Core STATIC
    ${CORE_MODULE_SOURCES}
    ${CORE_MODULE_HEADERS}
)

# Set target properties
set_target_properties(JitsiMeetQt_Core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "jitsimeetqt_core"
)

# Include directories
target_include_directories(JitsiMeetQt_Core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interfaces>
        $<INSTALL_INTERFACE:include/jitsimeetqt/core>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/management
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        ${CMAKE_CURRENT_SOURCE_DIR}/monitoring
        ${CMAKE_CURRENT_SOURCE_DIR}/versioning
)

# Link Qt libraries
target_link_libraries(JitsiMeetQt_Core
    PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
)

# Compiler definitions
target_compile_definitions(JitsiMeetQt_Core
    PUBLIC
        CORE_MODULE_VERSION="${PROJECT_VERSION}"
        CORE_MODULE_AVAILABLE
    PRIVATE
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        QT_NO_URL_CAST_FROM_STRING
        QT_NO_CAST_FROM_BYTEARRAY
)

# Compiler features
target_compile_features(JitsiMeetQt_Core
    PUBLIC
        cxx_std_17
)

# Enable Qt MOC for headers with Q_OBJECT
set_target_properties(JitsiMeetQt_Core PROPERTIES
    AUTOMOC ON
)

# Tests (optional)
option(BUILD_CORE_TESTS "Build core module tests" ON)

if(BUILD_CORE_TESTS)
    enable_testing()
    
    find_package(Qt6 REQUIRED COMPONENTS Test)
    
    add_executable(CoreModuleTest
        tests/CoreModuleTest.cpp
        tests/CoreModuleTest.h
    )
    
    target_link_libraries(CoreModuleTest
        PRIVATE
            JitsiMeetQt_Core
            Qt6::Test
    )
    
    set_target_properties(CoreModuleTest PROPERTIES
        AUTOMOC ON
    )
    
    add_test(NAME CoreModuleTest COMMAND CoreModuleTest)
endif()

# Examples (optional)
option(BUILD_CORE_EXAMPLES "Build core module examples" ON)

if(BUILD_CORE_EXAMPLES)
    add_executable(CoreSystemExample
        examples/CoreSystemExample.cpp
    )
    
    target_link_libraries(CoreSystemExample
        PRIVATE
            JitsiMeetQt_Core
    )
    
    set_target_properties(CoreSystemExample PROPERTIES
        AUTOMOC ON
    )
endif()

# Installation
include(GNUInstallDirs)

# Install library
install(TARGETS JitsiMeetQt_Core
    EXPORT JitsiMeetQt_CoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jitsimeetqt/core
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY interfaces/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jitsimeetqt/core/interfaces
    FILES_MATCHING PATTERN "*.h"
)

# Install CMake config files
install(EXPORT JitsiMeetQt_CoreTargets
    FILE JitsiMeetQt_CoreTargets.cmake
    NAMESPACE JitsiMeetQt::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/JitsiMeetQt_Core
)

# Create config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/JitsiMeetQt_CoreConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/JitsiMeetQt_CoreConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/JitsiMeetQt_Core
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/JitsiMeetQt_CoreConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/JitsiMeetQt_CoreConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/JitsiMeetQt_CoreConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/JitsiMeetQt_Core
)

# Package configuration
set(CPACK_PACKAGE_NAME "JitsiMeetQt_Core")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Jitsi Meet Qt Core Module Management System")
set(CPACK_PACKAGE_VENDOR "Jitsi Meet Qt Project")

include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "JitsiMeetQt Core Module Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build tests: ${BUILD_CORE_TESTS}")
message(STATUS "  Build examples: ${BUILD_CORE_EXAMPLES}")
message(STATUS "")