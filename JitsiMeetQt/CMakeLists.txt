cmake_minimum_required(VERSION 3.16)

project(JitsiMeetQt VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编码为UTF-8
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 查找Qt5组件
find_package(Qt5 REQUIRED COMPONENTS Core Widgets WebEngineWidgets Network WebChannel WebSockets)

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
    Qt5::Core
    Qt5::Widgets
    Qt5::WebEngineWidgets
    Qt5::Network
    Qt5::WebChannel
    Qt5::WebSockets
)

# Windows特定设置
if(WIN32)
    set_target_properties(JitsiMeetQt PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# 调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(JitsiMeetQt PRIVATE DEBUG_MODE)
endif()

# 安装规则
install(TARGETS JitsiMeetQt
    RUNTIME DESTINATION bin
)