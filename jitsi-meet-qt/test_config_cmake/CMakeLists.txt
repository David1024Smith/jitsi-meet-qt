cmake_minimum_required(VERSION 3.16)

project(ConfigTest VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找Qt组件
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Widgets
)

# 设置Qt自动处理
set(CMAKE_AUTOMOC ON)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/../include/models
)

# 源文件
set(SOURCES
    ../examples/simple_config_test.cpp
    ../src/ConfigurationManager.cpp
    ../src/WindowStateManager.cpp
    ../src/models/ApplicationSettings.cpp
)

# 头文件
set(HEADERS
    ../include/ConfigurationManager.h
    ../include/WindowStateManager.h
    ../include/models/ApplicationSettings.h
    ../include/JitsiConstants.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
)

# 编译器特定设置
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()