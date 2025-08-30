# 屏幕共享模块测试 CMake 配置

cmake_minimum_required(VERSION 3.16)
project(ScreenShareModuleTest)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt5 组件
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Test Multimedia)

# 启用 Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../config
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

# 测试源文件
set(TEST_SOURCES
    ScreenShareModuleTest.cpp
    ScreenShareModuleTest.h
    mocks/MockScreenShareManager.cpp
    mocks/MockScreenShareManager.h
)

# 模块源文件
set(MODULE_SOURCES
    ../src/ScreenShareModule.cpp
    ../src/ScreenShareManager.cpp
    ../src/CaptureEngine.cpp
    ../config/ScreenShareConfig.cpp
    ../capture/ScreenCapture.cpp
    ../capture/WindowCapture.cpp
    ../capture/RegionCapture.cpp
    ../encoding/VideoEncoder.cpp
    ../encoding/FrameProcessor.cpp
    ../widgets/ScreenShareWidget.cpp
    ../widgets/ScreenSelector.cpp
    ../widgets/CapturePreview.cpp
)

# 模块头文件
set(MODULE_HEADERS
    ../include/ScreenShareModule.h
    ../include/ScreenShareManager.h
    ../include/CaptureEngine.h
    ../config/ScreenShareConfig.h
    ../interfaces/IScreenCapture.h
    ../interfaces/IScreenShareManager.h
    ../interfaces/IDisplayManager.h
)

# 创建测试可执行文件
add_executable(screenshare_tests
    ${TEST_SOURCES}
    ${MODULE_SOURCES}
    ${MODULE_HEADERS}
)

# 链接 Qt 库
target_link_libraries(screenshare_tests
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
    Qt5::Multimedia
)

# 平台特定链接
if(WIN32)
    target_link_libraries(screenshare_tests gdi32 user32 dwmapi)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(screenshare_tests X11 Xext Xfixes)
elseif(APPLE)
    target_link_libraries(screenshare_tests "-framework CoreGraphics" "-framework CoreVideo")
endif()

# 编译定义
target_compile_definitions(screenshare_tests PRIVATE
    SCREENSHARE_MODULE_VERSION="1.0.0"
    SCREENSHARE_MODULE_ENABLED
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
)

# 启用测试
enable_testing()
add_test(NAME ScreenShareModuleTest COMMAND screenshare_tests)

# 设置测试属性
set_tests_properties(ScreenShareModuleTest PROPERTIES
    TIMEOUT 300
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)