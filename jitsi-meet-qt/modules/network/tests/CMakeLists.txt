cmake_minimum_required(VERSION 3.16)

project(NetworkModuleTests)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt组件
find_package(Qt6 REQUIRED COMPONENTS Core Network Test WebSockets)

# 启用Qt的MOC、UIC和RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../protocols
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils
    ${CMAKE_CURRENT_SOURCE_DIR}/../config
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets
)

# 网络模块源文件
set(NETWORK_MODULE_SOURCES
    ../src/NetworkManager.cpp
    ../src/NetworkManagerImpl.cpp
    ../src/ConnectionFactory.cpp
    ../src/BaseConnectionHandler.cpp
    ../src/NetworkModule.cpp
    ../protocols/WebRTCProtocol.cpp
    ../protocols/HTTPProtocol.cpp
    ../protocols/WebSocketProtocol.cpp
    ../utils/NetworkQualityMonitor.cpp
    ../utils/NetworkDiagnostics.cpp
    ../utils/NetworkUtils.cpp
    ../config/NetworkConfig.cpp
    ../widgets/ConnectionWidget.cpp
    ../widgets/NetworkStatusWidget.cpp
)

# 网络模块头文件
set(NETWORK_MODULE_HEADERS
    ../interfaces/INetworkManager.h
    ../interfaces/IConnectionHandler.h
    ../interfaces/IProtocolHandler.h
    ../include/NetworkManager.h
    ../include/ConnectionFactory.h
    ../protocols/WebRTCProtocol.h
    ../protocols/HTTPProtocol.h
    ../protocols/WebSocketProtocol.h
    ../utils/NetworkQualityMonitor.h
    ../utils/NetworkDiagnostics.h
    ../utils/NetworkUtils.h
    ../config/NetworkConfig.h
    ../widgets/ConnectionWidget.h
    ../widgets/NetworkStatusWidget.h
)

# 测试源文件
set(TEST_SOURCES
    main.cpp
    NetworkModuleTest.cpp
    NetworkTestRunner.cpp
    NetworkTestSuite.cpp
    MockObjects.cpp
)

# 测试头文件
set(TEST_HEADERS
    NetworkModuleTest.h
)

# 创建测试可执行文件
add_executable(NetworkModuleTests
    ${TEST_SOURCES}
    ${TEST_HEADERS}
    ${NETWORK_MODULE_SOURCES}
    ${NETWORK_MODULE_HEADERS}
)

# 链接Qt库
target_link_libraries(NetworkModuleTests
    Qt6::Core
    Qt6::Network
    Qt6::Test
    Qt6::WebSockets
)

# 设置编译器标志
if(MSVC)
    target_compile_options(NetworkModuleTests PRIVATE /W4)
else()
    target_compile_options(NetworkModuleTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 启用测试
enable_testing()

# 添加测试
add_test(NAME NetworkModuleTests COMMAND NetworkModuleTests)

# 设置测试环境变量
set_tests_properties(NetworkModuleTests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# 创建测试数据目录
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)

# 复制测试配置文件
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/test_config.json.in
    ${CMAKE_CURRENT_BINARY_DIR}/data/test_config.json
    @ONLY
)

# 设置输出目录
set_target_properties(NetworkModuleTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# 添加自定义目标用于运行测试
add_custom_target(run_network_tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/NetworkModuleTests
    DEPENDS NetworkModuleTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running network module tests"
)

# 添加代码覆盖率支持（如果可用）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_COMPILER_IS_GNUCXX)
        target_compile_options(NetworkModuleTests PRIVATE --coverage)
        target_link_libraries(NetworkModuleTests --coverage)
    endif()
endif()

# 添加内存检查支持
find_program(VALGRIND_PROGRAM valgrind)
if(VALGRIND_PROGRAM)
    add_custom_target(run_network_tests_valgrind
        COMMAND ${VALGRIND_PROGRAM} --tool=memcheck --leak-check=full --show-leak-kinds=all
                ${CMAKE_CURRENT_BINARY_DIR}/NetworkModuleTests
        DEPENDS NetworkModuleTests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running network module tests with Valgrind"
    )
endif()

# 打印配置信息
message(STATUS "Network Module Tests Configuration:")
message(STATUS "  Qt Version: ${Qt6_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Test Executable: ${CMAKE_CURRENT_BINARY_DIR}/NetworkModuleTests")