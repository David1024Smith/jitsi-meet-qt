cmake_minimum_required(VERSION 3.16)

project(JitsiMeetQt VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 强制设置C++标准编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# MinGW编译器特定设置
if(MINGW)
    # 设置MinGW特定的编译选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    
    # 启用调试信息（即使在Release模式下）
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
    
    # 优化设置
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
endif()

# 设置Qt版本要求
set(QT_MIN_VERSION "5.15.2")

# 查找Qt组件 - 使用纯C++实现，不依赖WebEngine
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Widgets
    Network
    WebSockets
    Multimedia
    MultimediaWidgets
    Xml
    LinguistTools
)

# 设置Qt自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 添加CMake模块路径
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

# 源文件
set(SOURCES
    src/main.cpp
    src/MainApplication.cpp
    src/WindowManager.cpp
    src/WelcomeWindow.cpp
    src/ConferenceWindow.cpp
    src/SettingsDialog.cpp
    src/ConfigurationManager.cpp
    src/ProtocolHandler.cpp
    src/TranslationManager.cpp
    src/WindowStateManager.cpp
    src/NavigationBar.cpp
    src/RecentListWidget.cpp
    src/models/RecentItem.cpp
    src/XMPPClient.cpp
    src/WebRTCEngine.cpp
    src/MediaManager.cpp
    src/ConferenceManager.cpp
    src/AuthenticationManager.cpp
    src/ChatManager.cpp
    src/ScreenShareManager.cpp
    src/JitsiError.cpp
    src/ErrorRecoveryManager.cpp
    src/ErrorDialog.cpp
    src/ErrorUtils.cpp
    src/ThemeManager.cpp
    src/StyleUtils.cpp
    src/StyleHelper.cpp
    src/PerformanceManager.cpp
    src/MemoryLeakDetector.cpp
    src/StartupOptimizer.cpp
    src/OptimizedRecentManager.cpp
    src/PerformanceConfig.cpp
    src/MemoryProfiler.cpp
)

# 头文件
set(HEADERS
    include/MainApplication.h
    include/WindowManager.h
    include/WelcomeWindow.h
    include/ConferenceWindow.h
    include/SettingsDialog.h
    include/ConfigurationManager.h
    include/ProtocolHandler.h
    include/TranslationManager.h
    include/WindowStateManager.h
    include/NavigationBar.h
    include/RecentListWidget.h
    include/models/RecentItem.h
    include/XMPPClient.h
    include/WebRTCEngine.h
    include/MediaManager.h
    include/ConferenceManager.h
    include/AuthenticationManager.h
    include/ChatManager.h
    include/ScreenShareManager.h
    include/JitsiConstants.h
    include/JitsiError.h
    include/ErrorRecoveryManager.h
    include/ErrorDialog.h
    include/ErrorUtils.h
    include/ThemeManager.h
    include/StyleUtils.h
    include/StyleHelper.h
    include/PerformanceManager.h
    include/MemoryLeakDetector.h
    include/StartupOptimizer.h
    include/OptimizedRecentManager.h
    include/PerformanceConfig.h
    include/MemoryProfiler.h
)

# 翻译文件
set(TS_FILES
    translations/jitsi_en.ts
    translations/jitsi_zh_CN.ts
    translations/jitsi_ja.ts
    translations/jitsi_ko.ts
    translations/jitsi_fr.ts
    translations/jitsi_de.ts
    translations/jitsi_es.ts
    translations/jitsi_ru.ts
)

# 编译翻译文件
qt5_create_translation(QM_FILES ${SOURCES} ${HEADERS} ${TS_FILES})

# 资源文件
set(RESOURCES
    resources/resources.qrc
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${QM_FILES}
)

# 第三方库依赖配置
find_package(PkgConfig QUIET)

# OpenSSL依赖（用于安全连接）
find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(${PROJECT_NAME} PRIVATE OPENSSL_AVAILABLE)
endif()

# 链接Qt库 - 使用纯C++实现所需的库
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::WebSockets
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    Qt5::Xml
)

# Windows特定的系统库
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        ws2_32      # Winsock
        winmm       # Windows Multimedia
        ole32       # OLE
        oleaut32    # OLE Automation
        uuid        # UUID
        advapi32    # Advanced API
        shell32     # Shell API
        user32      # User API
        gdi32       # GDI
    )
endif()

# 版本信息配置
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION_BUILD 0)
set(PROJECT_COMPANY_NAME "Jitsi Meet Qt")
set(PROJECT_COPYRIGHT "Copyright (C) 2024 Jitsi Meet Qt Team")
set(PROJECT_DESCRIPTION "Qt-based Jitsi Meet Desktop Application")

# 配置版本头文件
configure_file(
    "${CMAKE_SOURCE_DIR}/include/version.h.in"
    "${CMAKE_BINARY_DIR}/include/version.h"
    @ONLY
)

# Windows特定设置
if(WIN32)
    # 设置应用程序图标和属性
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "JitsiMeetQt"
        VERSION ${PROJECT_VERSION}
    )
    
    # 生成Windows资源文件
    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/app.rc.in"
        "${CMAKE_BINARY_DIR}/resources/app.rc"
        @ONLY
    )
    
    # 添加Windows资源文件
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/resources/app.rc")
    
    # 添加生成的版本头文件路径
    target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/include")
endif()

# 编译器特定设置
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Qt部署工具配置
if(WIN32)
    # 查找windeployqt工具
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt5_DIR}/../../../bin)
    
    if(WINDEPLOYQT_EXECUTABLE)
        # 添加部署目标
        add_custom_target(deploy
            COMMAND ${CMAKE_COMMAND} -E env PATH="${Qt5_DIR}/../../../bin" 
                    ${WINDEPLOYQT_EXECUTABLE} 
                    --qmldir ${CMAKE_SOURCE_DIR}
                    --verbose 2
                    --release
                    --no-translations
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    $<TARGET_FILE:${PROJECT_NAME}>
            DEPENDS ${PROJECT_NAME}
            COMMENT "Deploying Qt libraries"
        )
    endif()
endif()

# CPack配置用于创建安装包
set(CPACK_PACKAGE_NAME "JitsiMeetQt")
set(CPACK_PACKAGE_VENDOR "${PROJECT_COMPANY_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "JitsiMeetQt")
set(CPACK_PACKAGE_EXECUTABLES "JitsiMeetQt;Jitsi Meet Qt")

# Windows特定的CPack设置
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Jitsi Meet Qt")
    set(CPACK_NSIS_PACKAGE_NAME "Jitsi Meet Qt")
    set(CPACK_NSIS_CONTACT "support@jitsimeetqt.com")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/jitsi/jitsi-meet-qt")
    set(CPACK_NSIS_HELP_LINK "https://github.com/jitsi/jitsi-meet-qt/wiki")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    
    # 设置安装图标
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/icons/app.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/resources/icons/app.ico")
    
    # 创建桌面快捷方式
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\Jitsi Meet Qt.lnk' '$INSTDIR\\\\bin\\\\JitsiMeetQt.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\Jitsi Meet Qt.lnk'"
    )
    
    # 注册协议处理器
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
        "WriteRegStr HKCR 'jitsi-meet' '' 'URL:Jitsi Meet Protocol'
         WriteRegStr HKCR 'jitsi-meet' 'URL Protocol' ''
         WriteRegStr HKCR 'jitsi-meet\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\JitsiMeetQt.exe,0'
         WriteRegStr HKCR 'jitsi-meet\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\JitsiMeetQt.exe \\\"%1\\\"'"
    )
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
        "DeleteRegKey HKCR 'jitsi-meet'"
    )
endif()

# 安装配置
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# 安装资源文件
install(DIRECTORY ${CMAKE_SOURCE_DIR}/translations
    DESTINATION .
    COMPONENT Translations
    FILES_MATCHING PATTERN "*.qm"
)

# 安装许可证和文档
install(FILES 
    ${CMAKE_SOURCE_DIR}/README.md
    DESTINATION .
    COMPONENT Documentation
)

include(CPack)

# 测试配置
enable_testing()
add_subdirectory(tests)