# Audio Module Tests CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

# 查找Qt组件
find_package(Qt6 REQUIRED COMPONENTS Core Test)

# 启用Qt自动处理
set(CMAKE_AUTOMOC ON)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../config
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets
)

# 共享源文件
set(AUDIO_MODULE_SOURCES
    ../config/AudioConfig.cpp
    ../utils/AudioUtils.cpp
)

# AudioConfig测试
add_executable(AudioConfigTest
    AudioConfigTest.cpp
    ${AUDIO_MODULE_SOURCES}
)

target_link_libraries(AudioConfigTest
    Qt6::Core
    Qt6::Test
)

# AudioUtils测试
add_executable(AudioUtilsTest
    AudioUtilsTest.cpp
    ${AUDIO_MODULE_SOURCES}
)

target_link_libraries(AudioUtilsTest
    Qt6::Core
    Qt6::Test
)

# AudioModule综合测试
add_executable(AudioModuleTest
    AudioModuleTest.cpp
    AudioTestSuite.cpp
    ${AUDIO_MODULE_SOURCES}
)

target_link_libraries(AudioModuleTest
    Qt6::Core
    Qt6::Test
)

# 测试运行器
add_executable(AudioTestRunner
    AudioTestRunner.cpp
    AudioTestSuite.cpp
    AudioModuleTest.cpp
    ${AUDIO_MODULE_SOURCES}
)

target_link_libraries(AudioTestRunner
    Qt6::Core
    Qt6::Test
)

# 添加测试
add_test(NAME AudioConfigTest COMMAND AudioConfigTest)
add_test(NAME AudioUtilsTest COMMAND AudioUtilsTest)
add_test(NAME AudioModuleTest COMMAND AudioModuleTest)

# 设置测试属性
set_tests_properties(AudioConfigTest AudioUtilsTest AudioModuleTest PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# 编译定义
target_compile_definitions(AudioConfigTest PRIVATE
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
    AUDIO_CONFIG_TEST
)

target_compile_definitions(AudioUtilsTest PRIVATE
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
    AUDIO_UTILS_TEST
)

target_compile_definitions(AudioModuleTest PRIVATE
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
    AUDIO_MODULE_TEST
)

target_compile_definitions(AudioTestRunner PRIVATE
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
    AUDIO_TEST_RUNNER
)

# 如果是Debug模式，启用更多调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(AudioConfigTest AudioUtilsTest AudioModuleTest AudioTestRunner PRIVATE
        QT_QML_DEBUG
        QT_DECLARATIVE_DEBUG
    )
endif()

# 创建测试输出目录
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/reports)

message(STATUS "Audio module comprehensive tests configured")
message(STATUS "Tests: AudioConfig, AudioUtils, AudioModule, TestRunner")
message(STATUS "Output directory: ${CMAKE_CURRENT_BINARY_DIR}")