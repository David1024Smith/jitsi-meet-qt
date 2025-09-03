cmake_minimum_required(VERSION 3.16)

project(JitsiMeetQt VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 修复MSVC编译器选项冲突
if(MSVC)
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

# 设置编码为UTF-8
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 设置Qt6路径
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.3/msvc2022_64")

# 查找Qt6组件
find_package(Qt6 6.8.3 REQUIRED COMPONENTS Core Widgets WebEngineWidgets Network WebChannel WebSockets)

# 启用Qt的MOC、UIC和RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含目录
include_directories(include)
include_directories(include/windows)
include_directories(include/dialogs)
include_directories(include/widgets)
include_directories(include/models)
include_directories(include/utils)

# 源文件
set(SOURCES
    src/main.cpp
    src/MainApplication.cpp
    src/ConferenceWindow.cpp
    src/WelcomeWindow.cpp
    src/SettingsDialog.cpp
    src/ConfigurationManager.cpp
    src/ProtocolHandler.cpp
    src/JitsiMeetAPI.cpp
    src/NetworkDiagnostics.cpp
)

# 头文件
set(HEADERS
    include/MainApplication.h
    include/ConferenceWindow.h
    include/WelcomeWindow.h
    include/SettingsDialog.h
    include/ConfigurationManager.h
    include/ProtocolHandler.h
    include/JitsiMeetAPI.h
    include/NetworkDiagnostics.h
)

# 资源文件
set(RESOURCES
    resources/resources.qrc
)

# 创建可执行文件
add_executable(JitsiMeetQt ${SOURCES} ${HEADERS} ${RESOURCES})

# 链接Qt库
target_link_libraries(JitsiMeetQt
    Qt6::Core
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::Network
    Qt6::WebChannel
    Qt6::WebSockets
)

# Windows特定设置
if(WIN32)
    set_target_properties(JitsiMeetQt PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# 编译优化设置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(JitsiMeetQt PRIVATE DEBUG_MODE)
    # 调试模式下启用详细调试信息
    if(MSVC)
        target_compile_options(JitsiMeetQt PRIVATE /W3 /Zi /Od)
    else()
        target_compile_options(JitsiMeetQt PRIVATE -g -O0 -Wall)
    endif()
else()
    # 发布模式下启用优化
    if(MSVC)
        target_compile_options(JitsiMeetQt PRIVATE /O2 /DNDEBUG)
    else()
        target_compile_options(JitsiMeetQt PRIVATE -O2 -DNDEBUG)
    endif()
endif()

# 启用并行编译（Windows MSVC）
if(MSVC)
    target_compile_options(JitsiMeetQt PRIVATE /MP)
endif()

# Qt依赖库复制配置
if(WIN32)
    # 定义Qt6 bin目录
    set(QT6_BIN_DIR "C:/Qt/6.8.3/msvc2022_64/bin")
    
    # 定义需要复制的Qt6 DLL文件
    set(QT6_DLLS
        Qt6Core.dll
        Qt6Gui.dll
        Qt6Widgets.dll
        Qt6Network.dll
        Qt6WebChannel.dll
        Qt6WebSockets.dll
        Qt6WebEngineCore.dll
        Qt6WebEngineWidgets.dll
        Qt6Quick.dll
        Qt6Qml.dll
        Qt6QmlModels.dll
        Qt6QmlMeta.dll
        Qt6QmlWorkerScript.dll
        Qt6QuickWidgets.dll
        Qt6OpenGL.dll
        Qt6Positioning.dll
        Qt6PrintSupport.dll
    )
    
    # 添加自定义目标来复制Qt DLL文件
    add_custom_target(copy_qt_dlls ALL
        COMMENT "复制Qt6依赖库到构建目录"
    )
    
    # 为每个DLL文件添加复制命令
    foreach(dll ${QT6_DLLS})
        if(EXISTS "${QT6_BIN_DIR}/${dll}")
            add_custom_command(TARGET copy_qt_dlls POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT6_BIN_DIR}/${dll}"
                    "$<TARGET_FILE_DIR:JitsiMeetQt>/${dll}"
                COMMENT "复制 ${dll} 到构建目录"
            )
        endif()
    endforeach()
    
    # 复制Qt平台插件
    if(EXISTS "C:/Qt/6.8.3/msvc2022_64/plugins/platforms/qwindows.dll")
        add_custom_command(TARGET copy_qt_dlls POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:JitsiMeetQt>/platforms"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "C:/Qt/6.8.3/msvc2022_64/plugins/platforms/qwindows.dll"
                "$<TARGET_FILE_DIR:JitsiMeetQt>/platforms/qwindows.dll"
            COMMENT "复制Qt平台插件到构建目录"
        )
    endif()
    
    # 复制Qt图像格式插件
    set(IMAGE_PLUGINS qjpeg.dll qpng.dll qico.dll)
    foreach(plugin ${IMAGE_PLUGINS})
        if(EXISTS "C:/Qt/6.8.3/msvc2022_64/plugins/imageformats/${plugin}")
            add_custom_command(TARGET copy_qt_dlls POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:JitsiMeetQt>/imageformats"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "C:/Qt/6.8.3/msvc2022_64/plugins/imageformats/${plugin}"
                    "$<TARGET_FILE_DIR:JitsiMeetQt>/imageformats/${plugin}"
                COMMENT "复制图像格式插件 ${plugin} 到构建目录"
            )
        endif()
    endforeach()
    
    # 确保copy_qt_dlls在JitsiMeetQt之后执行
    add_dependencies(copy_qt_dlls JitsiMeetQt)
endif()

# 安装规则
install(TARGETS JitsiMeetQt
    RUNTIME DESTINATION bin
)