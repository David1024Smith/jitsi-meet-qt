cmake_minimum_required(VERSION 3.20)

project(JitsiMeetQt VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Version information
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION_BUILD 0)

# Configure for MinGW compiler , C:\Qt\6.8.3\mingw_64\bin ,C:\Qt\Tools\mingw1310_64\bin
if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DQT_NO_DEBUG_OUTPUT")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    
    # Disable LTO for release builds to avoid symbol linking issues
    # if(CMAKE_BUILD_TYPE STREQUAL "Release")
    #     set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
    #     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
    # endif()
endif()

# Configure version header
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/version.h"
    @ONLY
)

# Find Qt 6.8.3 components  C:\Qt\6.8.3\mingw_64\bin ,C:\Qt\Tools\mingw1310_64\bin
find_package(Qt6 6.8.3 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    WebSockets
    Multimedia
    MultimediaWidgets
    Sql
    LinguistTools
    Xml
    Charts
)

# Find OpenSSL - make it optional for now
find_package(OpenSSL)

# If OpenSSL is not found, set custom path or disable
if(NOT OPENSSL_FOUND)
    message(WARNING "OpenSSL not found, trying alternative paths...")
    set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
    set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
    set(OPENSSL_LIBRARIES "${OPENSSL_ROOT_DIR}/lib/VC/x64/MD/libssl.lib;${OPENSSL_ROOT_DIR}/lib/VC/x64/MD/libcrypto.lib")
    
    # Check if the alternative path exists
    if(EXISTS "${OPENSSL_INCLUDE_DIR}/openssl/ssl.h")
        set(OPENSSL_FOUND TRUE)
        message(STATUS "Found OpenSSL at: ${OPENSSL_ROOT_DIR}")
    else()
        message(WARNING "OpenSSL not found, building without SSL support")
        set(OPENSSL_FOUND FALSE)
        set(OPENSSL_LIBRARIES "")
    endif()
endif()

# Enable Qt MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
)

# Resource files
set(RESOURCES
    resources/resources.qrc
)

# Add module subdirectories - only working modules for now
add_subdirectory(modules/settings)
# Temporarily disable other modules until we create missing source files
# add_subdirectory(modules/chat)
# add_subdirectory(modules/network)
add_subdirectory(modules/performance)
add_subdirectory(modules/screenshare)
add_subdirectory(modules/utils)
# add_subdirectory(modules/ui)
# add_subdirectory(modules/audio)
# add_subdirectory(modules/camera)

# Source files - only include existing files
set(SOURCES
    src/main.cpp
    src/MainApplication.cpp
    src/WebRTCEngine.cpp
    src/MediaManager.cpp
    src/AuthenticationManager.cpp
    src/XMPPClient.cpp
    src/ConferenceManager.cpp
    src/ConfigurationManager.cpp
    src/WindowManager.cpp
    src/WelcomeWindow.cpp
    src/ConferenceWindow.cpp
    src/SettingsDialog.cpp
    src/ProtocolHandler.cpp
    src/TranslationManager.cpp
    src/NavigationBar.cpp
    src/RecentListWidget.cpp
    src/JitsiError.cpp
    src/ErrorRecoveryManager.cpp
    src/ErrorDialog.cpp
    src/ErrorUtils.cpp
    src/StyleHelper.cpp
    src/StyleUtils.cpp
    src/MemoryProfiler.cpp
    src/MemoryLeakDetector.cpp
    src/OptimizedRecentManager.cpp
    src/ErrorLogger.cpp
    src/ErrorEventBus.cpp
    src/WindowStateManager.cpp
    src/ModuleError.cpp
)

# Header files - only existing files
set(HEADERS
    include/MainApplication.h
    include/WebRTCEngine.h
    include/MediaManager.h
    include/AuthenticationManager.h
    include/XMPPClient.h
    include/ConferenceManager.h
    include/ConfigurationManager.h
    include/WindowManager.h
    include/WelcomeWindow.h
    include/ConferenceWindow.h
    include/SettingsDialog.h
    include/ProtocolHandler.h
    include/TranslationManager.h
    include/NavigationBar.h
    include/RecentListWidget.h
    include/JitsiError.h
    include/ErrorRecoveryManager.h
    include/ErrorDialog.h
    include/ErrorUtils.h
    include/StyleHelper.h
    include/StyleUtils.h
    include/MemoryProfiler.h
    include/MemoryLeakDetector.h
    include/OptimizedRecentManager.h
    include/ModuleError.h
    include/ErrorEventBus.h
    include/WindowStateManager.h
    include/ErrorLogger.h
    include/JitsiConstants.h
    include/version.h.in
)

# Translation files
set(TS_FILES
    translations/jitsimeet_en.ts
    translations/jitsimeet_zh_CN.ts
    translations/jitsimeet_es.ts
    translations/jitsimeet_fr.ts
    translations/jitsimeet_de.ts
    translations/jitsimeet_ja.ts
    translations/jitsimeet_ko.ts
    translations/jitsimeet_ru.ts
    translations/jitsimeet_pt.ts
    translations/jitsimeet_it.ts
)

# Create translation targets
qt6_add_translations(${PROJECT_NAME} TS_FILES ${TS_FILES})

# MOC files (Qt will handle these automatically with CMAKE_AUTOMOC)
qt6_standard_project_setup()

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})

# Link Qt libraries and modules
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Sql
    Qt6::Xml
    Qt6::Charts
    # Module libraries - only enabled modules
    ScreenShareModule
    SettingsModule
    UtilsModule
    # Temporarily disabled modules
    # ChatModule
    # NetworkModule
    PerformanceModule
    # UIModule
    # AudioModule
    # CameraModule
)

# Add OpenSSL if found - temporarily disabled due to library path issues
# if(OPENSSL_FOUND)
#     target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
#     target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_OPENSSL)
# endif()

# Windows specific settings
if(WIN32)
    # Set target properties
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "JitsiMeetQt"
    )
    
    # Add Windows resources
    target_sources(${PROJECT_NAME} PRIVATE resources/windows.rc)
    
    # Link Windows libraries
    target_link_libraries(${PROJECT_NAME}
        ws2_32
        winmm
        ole32
        oleaut32
        uuid
        advapi32
        shell32
        user32
        gdi32
    )
endif()

# Install configuration
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install Qt libraries and dependencies
if(WIN32)
    # Find Qt installation path
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_WINDEPLOYQT_EXECUTABLE ${QT_QMAKE_EXECUTABLE} PATH)
    set(QT_WINDEPLOYQT_EXECUTABLE "${QT_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")
    
    # Configure deployment
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/deploy/deploy.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/deploy.cmake"
        @ONLY
    )
    
    install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/deploy.cmake")
endif()

# CPack configuration for Windows installer
set(CPACK_GENERATOR "NSIS")
set(CPACK_PACKAGE_NAME "Jitsi Meet Qt")
set(CPACK_PACKAGE_VENDOR "Jitsi Meet Qt Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Native Qt implementation of Jitsi Meet")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "JitsiMeetQt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# NSIS specific settings
set(CPACK_NSIS_DISPLAY_NAME "Jitsi Meet Qt")
set(CPACK_NSIS_PACKAGE_NAME "Jitsi Meet Qt")
set(CPACK_NSIS_CONTACT "support@jitsimeet.org")
set(CPACK_NSIS_URL_INFO_ABOUT "https://jitsi.org/")
set(CPACK_NSIS_HELP_LINK "https://jitsi.org/support/")
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

# Create desktop shortcut
set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$DESKTOP\\\\Jitsi Meet Qt.lnk' '$INSTDIR\\\\bin\\\\JitsiMeetQt.exe'"
)
set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$DESKTOP\\\\Jitsi Meet Qt.lnk'"
)

include(CPack)

# Enable testing
enable_testing()

# Settings Dialog Test
if(BUILD_SETTINGS_TEST)
    add_executable(test_settings_dialog
        test_settings_dialog.cpp
        src/SettingsDialog.cpp
        src/ConfigurationManager.cpp
        src/TranslationManager.cpp
        src/MediaManager.cpp
        src/WindowStateManager.cpp
        src/JitsiError.cpp
        src/ErrorUtils.cpp
        src/ThemeManager.cpp
        src/StyleHelper.cpp
        src/StyleUtils.cpp
    )
    
    target_link_libraries(test_settings_dialog
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::MultimediaWidgets
    )
    
    set_target_properties(test_settings_dialog PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Settings Dialog Verification
if(BUILD_SETTINGS_VERIFY)
    add_executable(verify_settings_dialog
        verify_settings_dialog.cpp
        src/SettingsDialog.cpp
        src/ConfigurationManager.cpp
        src/TranslationManager.cpp
        src/MediaManager.cpp
        src/WindowStateManager.cpp
        src/JitsiError.cpp
        src/ErrorUtils.cpp
        src/ThemeManager.cpp
        src/StyleHelper.cpp
        src/StyleUtils.cpp
    )
    
    target_link_libraries(verify_settings_dialog
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::MultimediaWidgets
    )
    
    set_target_properties(verify_settings_dialog PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Theme and Resource Test
if(BUILD_THEME_TEST)
    add_executable(test_theme_resources
        test_theme_resources.cpp
        src/ThemeManager.cpp
        src/StyleHelper.cpp
        src/StyleUtils.cpp
        ${RESOURCES}
    )
    
    target_link_libraries(test_theme_resources
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
    )
    
    set_target_properties(test_theme_resources PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

add_subdirectory(tests)