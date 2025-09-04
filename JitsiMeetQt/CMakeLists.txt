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
find_package(Qt6 6.8.3 REQUIRED COMPONENTS Core Widgets WebEngineWidgets Network WebChannel WebSockets Svg SvgWidgets)

# 启用Qt的MOC、UIC和RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含目录
include_directories(include)

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
    src/Logger.cpp
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
    include/Logger.h
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
    Qt6::Svg
    Qt6::SvgWidgets
)

# Windows特定设置
if(WIN32)
    set_target_properties(JitsiMeetQt PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# 编译优化设置 - 使用生成器表达式支持多配置生成器
# Debug模式配置
target_compile_definitions(JitsiMeetQt PRIVATE 
    $<$<CONFIG:Debug>:DEBUG_MODE>
)

if(MSVC)
    target_compile_options(JitsiMeetQt PRIVATE 
        $<$<CONFIG:Debug>:/W3 /Zi /Od>
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
        $<$<CONFIG:MinSizeRel>:/O1 /DNDEBUG>
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG>
    )
else()
    target_compile_options(JitsiMeetQt PRIVATE 
        $<$<CONFIG:Debug>:-g -O0 -Wall>
        $<$<CONFIG:Release>:-O2 -DNDEBUG>
        $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
        $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
    )
endif()

# 启用并行编译（Windows MSVC）
if(MSVC)
    target_compile_options(JitsiMeetQt PRIVATE /MP)
endif()

# Qt依赖库复制配置
if(WIN32)
    # 定义Qt6 bin目录
    set(QT6_BIN_DIR "C:/Qt/6.8.3/msvc2022_64/bin")
    
    # 定义需要复制的Qt6 DLL文件（Debug和Release版本）
    set(QT6_DEBUG_DLLS
        Qt6Cored.dll
        Qt6Guid.dll
        Qt6Widgetsd.dll
        Qt6Networkd.dll
        Qt6WebChanneld.dll
        Qt6WebSocketsd.dll
        Qt6WebEngineCored.dll
        Qt6WebEngineWidgetsd.dll
        Qt6Quickd.dll
        Qt6Qmld.dll
        Qt6QmlModelsd.dll
        Qt6QmlMetad.dll
        Qt6QmlWorkerScriptd.dll
        Qt6QuickWidgetsd.dll
        Qt6OpenGLd.dll
        Qt6Positioningd.dll
        Qt6PrintSupportd.dll
        Qt6Svgd.dll
        Qt6SvgWidgetsd.dll
    )
    
    set(QT6_RELEASE_DLLS
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
        Qt6Svg.dll
        Qt6SvgWidgets.dll
    )
    
    # 添加自定义目标来复制Qt DLL文件
    add_custom_target(copy_qt_dlls ALL
        COMMENT "copy Qt6 dll to build"
    )
    
    # 为Debug版本DLL文件添加复制命令
    foreach(dll ${QT6_DEBUG_DLLS})
        if(EXISTS "${QT6_BIN_DIR}/${dll}")
            add_custom_command(TARGET copy_qt_dlls POST_BUILD
                COMMAND $<$<CONFIG:Debug>:${CMAKE_COMMAND}> $<$<CONFIG:Debug>:-E> $<$<CONFIG:Debug>:copy_if_different>
                    $<$<CONFIG:Debug>:"${QT6_BIN_DIR}/${dll}">
                    $<$<CONFIG:Debug>:"$<TARGET_FILE_DIR:JitsiMeetQt>/${dll}">
                COMMENT "copy ${dll} for Debug build"
            )
        endif()
    endforeach()
    
    # 为Release版本DLL文件添加复制命令
    foreach(dll ${QT6_RELEASE_DLLS})
        if(EXISTS "${QT6_BIN_DIR}/${dll}")
            add_custom_command(TARGET copy_qt_dlls POST_BUILD
                COMMAND $<$<CONFIG:Release>:${CMAKE_COMMAND}> $<$<CONFIG:Release>:-E> $<$<CONFIG:Release>:copy_if_different>
                    $<$<CONFIG:Release>:"${QT6_BIN_DIR}/${dll}">
                    $<$<CONFIG:Release>:"$<TARGET_FILE_DIR:JitsiMeetQt>/${dll}">
                COMMENT "copy ${dll} for Release build"
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
            COMMENT "copy Qt platforms to build"
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
                COMMENT "copy ${plugin} imageformats to build"
            )
        endif()
    endforeach()
    
    # 复制Qt WebEngine进程文件（根据构建类型选择Debug或Release版本）
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(EXISTS "${QT6_BIN_DIR}/QtWebEngineProcessd.exe")
            add_custom_command(TARGET copy_qt_dlls POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT6_BIN_DIR}/QtWebEngineProcessd.exe"
                    "$<TARGET_FILE_DIR:JitsiMeetQt>/QtWebEngineProcessd.exe"
                COMMENT "copy Qt WebEngine Process (Debug) to build"
            )
        endif()
    else()
        if(EXISTS "${QT6_BIN_DIR}/QtWebEngineProcess.exe")
            add_custom_command(TARGET copy_qt_dlls POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT6_BIN_DIR}/QtWebEngineProcess.exe"
                    "$<TARGET_FILE_DIR:JitsiMeetQt>/QtWebEngineProcess.exe"
                COMMENT "copy Qt WebEngine Process (Release) to build"
            )
        endif()
    endif()
    
    # 复制Qt WebEngine资源文件
    set(WEBENGINE_RESOURCES
        icudtl.dat
        qtwebengine_devtools_resources.pak
        qtwebengine_resources.pak
        qtwebengine_resources_100p.pak
        qtwebengine_resources_200p.pak
        v8_context_snapshot.bin
    )
    
    foreach(resource ${WEBENGINE_RESOURCES})
        if(EXISTS "C:/Qt/6.8.3/msvc2022_64/bin/${resource}")
            add_custom_command(TARGET copy_qt_dlls POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "C:/Qt/6.8.3/msvc2022_64/bin/${resource}"
                    "$<TARGET_FILE_DIR:JitsiMeetQt>/${resource}"
                COMMENT "copy WebEngine ${resource} to build"
            )
        endif()
    endforeach()
    
    # 复制Qt WebEngine resources目录
    if(EXISTS "C:/Qt/6.8.3/msvc2022_64/resources")
        add_custom_command(TARGET copy_qt_dlls POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "C:/Qt/6.8.3/msvc2022_64/resources"
                "$<TARGET_FILE_DIR:JitsiMeetQt>/resources"
            COMMENT "copy Qt WebEngine resources directory to build"
        )
    endif()
    
    # 复制Qt WebEngine本地化文件
    if(EXISTS "C:/Qt/6.8.3/msvc2022_64/translations")
        add_custom_command(TARGET copy_qt_dlls POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:JitsiMeetQt>/translations"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "C:/Qt/6.8.3/msvc2022_64/translations"
                "$<TARGET_FILE_DIR:JitsiMeetQt>/translations"
        )
    endif()
    
    # 确保copy_qt_dlls在JitsiMeetQt之后执行
    add_dependencies(copy_qt_dlls JitsiMeetQt)
endif()

# 安装规则
install(TARGETS JitsiMeetQt
    RUNTIME DESTINATION bin
)
