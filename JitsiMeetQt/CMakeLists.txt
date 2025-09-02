cmake_minimum_required(VERSION 3.16)

project(JitsiMeetQt VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6组件
find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets Network)

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
    src/windows/ConferenceWindow.cpp
    src/windows/WelcomeWindow.cpp
    src/dialogs/SettingsDialog.cpp
    src/widgets/ConferenceWidget.cpp
    src/models/ConfigurationManager.cpp
    src/models/RecentMeetingsModel.cpp
    src/utils/ProtocolHandler.cpp
    src/utils/JitsiMeetAPI.cpp
    src/utils/NetworkManager.cpp
)

# 头文件
set(HEADERS
    include/MainApplication.h
    include/windows/ConferenceWindow.h
    include/windows/WelcomeWindow.h
    include/dialogs/SettingsDialog.h
    include/widgets/ConferenceWidget.h
    include/models/ConfigurationManager.h
    include/models/RecentMeetingsModel.h
    include/utils/ProtocolHandler.h
    include/utils/JitsiMeetAPI.h
    include/utils/NetworkManager.h
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