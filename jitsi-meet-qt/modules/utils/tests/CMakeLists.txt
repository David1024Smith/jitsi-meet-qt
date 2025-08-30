cmake_minimum_required(VERSION 3.16)

project(UtilsModuleTests)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Test Network)

# 查找OpenSSL
find_package(OpenSSL REQUIRED)

# 启用Qt的自动MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 测试源文件
set(TEST_SOURCES
    UtilsModuleTest.cpp
    UtilsModuleTest.h
)

# Utils模块源文件
set(UTILS_SOURCES
    ../src/UtilsModule.cpp
    ../src/Logger.cpp
    ../src/FileManager.cpp
    ../src/UtilsErrorHandler.cpp
    ../src/UtilsSingletonManager.cpp
    ../logging/FileLogger.cpp
    ../logging/ConsoleLogger.cpp
    ../logging/NetworkLogger.cpp
    ../file/ConfigFile.cpp
    ../file/TempFile.cpp
    ../file/FileWatcher.cpp
    ../crypto/AESCrypto.cpp
    ../crypto/RSACrypto.cpp
    ../crypto/HashUtils.cpp
    ../string/StringUtils.cpp
    ../string/Validator.cpp
    ../config/UtilsConfig.cpp
)

# 创建测试可执行文件
add_executable(utils_tests ${TEST_SOURCES} ${UTILS_SOURCES})

# 链接Qt库
target_link_libraries(utils_tests
    Qt6::Core
    Qt6::Test
    Qt6::Network
    OpenSSL::SSL
    OpenSSL::Crypto
)

# 包含目录
target_include_directories(utils_tests PRIVATE
    ../include
    ../interfaces
    ../config
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 编译定义
target_compile_definitions(utils_tests PRIVATE
    QT_DEPRECATED_WARNINGS
    UTILS_TESTS_ENABLED
    UTILS_MODULE_AVAILABLE
)

# 调试配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(utils_tests PRIVATE UTILS_DEBUG_TESTS)
endif()

# 平台特定配置
if(WIN32)
    target_link_libraries(utils_tests advapi32)
endif()

# 添加测试到CTest
enable_testing()
add_test(NAME UtilsModuleTests COMMAND utils_tests)

# 设置测试属性
set_tests_properties(UtilsModuleTests PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# 复制测试数据文件
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "Utils Module Tests CMake configuration completed")