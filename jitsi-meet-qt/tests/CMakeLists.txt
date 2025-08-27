# 查找Qt Test组件
find_package(Qt5 REQUIRED COMPONENTS Test)

# 包含源代码目录
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

# 创建测试库，包含需要测试的源文件
add_library(TestLib STATIC
    ${CMAKE_SOURCE_DIR}/src/MainApplication.cpp
    ${CMAKE_SOURCE_DIR}/src/WindowManager.cpp
    ${CMAKE_SOURCE_DIR}/src/WelcomeWindow.cpp
    ${CMAKE_SOURCE_DIR}/src/ConferenceWindow.cpp
    ${CMAKE_SOURCE_DIR}/src/SettingsDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/ProtocolHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/ConfigurationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/TranslationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/WindowStateManager.cpp
    ${CMAKE_SOURCE_DIR}/src/NavigationBar.cpp
    ${CMAKE_SOURCE_DIR}/src/RecentListWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/models/ApplicationSettings.cpp
    ${CMAKE_SOURCE_DIR}/src/models/RecentItem.cpp
    ${CMAKE_SOURCE_DIR}/src/JitsiError.cpp
    ${CMAKE_SOURCE_DIR}/src/ErrorRecoveryManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ErrorDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/ErrorUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/PerformanceManager.cpp
    ${CMAKE_SOURCE_DIR}/src/MemoryLeakDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/StartupOptimizer.cpp
    ${CMAKE_SOURCE_DIR}/src/OptimizedRecentManager.cpp
)

target_link_libraries(TestLib
    Qt5::Core
    Qt5::Widgets
    Qt5::WebEngine
    Qt5::WebEngineWidgets
    Qt5::Network
    Qt5::Concurrent
)

# 协议处理器测试
add_executable(test_protocolhandler test_protocolhandler.cpp)
target_link_libraries(test_protocolhandler
    TestLib
    Qt5::Test
    Qt5::Core
)
add_test(NAME ProtocolHandlerTest COMMAND test_protocolhandler)

# 配置管理器测试
add_executable(test_configuration test_configuration.cpp)
target_link_libraries(test_configuration
    TestLib
    Qt5::Test
    Qt5::Core
)
add_test(NAME ConfigurationTest COMMAND test_configuration)

# 窗口状态管理器测试
add_executable(test_windowstatemanager test_windowstatemanager.cpp)
target_link_libraries(test_windowstatemanager
    TestLib
    Qt5::Test
    Qt5::Core
)
add_test(NAME WindowStateManagerTest COMMAND test_windowstatemanager)

# 导航栏测试
add_executable(test_navigationbar test_navigationbar.cpp)
target_link_libraries(test_navigationbar
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
)
add_test(NAME NavigationBarTest COMMAND test_navigationbar)

# 最近列表测试
add_executable(test_recentlist test_recentlist.cpp)
target_link_libraries(test_recentlist
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
)
add_test(NAME RecentListTest COMMAND test_recentlist)

# 错误处理系统测试
add_executable(test_error_handling test_error_handling.cpp)
target_link_libraries(test_error_handling
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
)
add_test(NAME ErrorHandlingTest COMMAND test_error_handling)

# 集成测试
add_executable(test_integration test_integration.cpp)
target_link_libraries(test_integration
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
    Qt5::WebEngine
    Qt5::WebEngineWidgets
    Qt5::Network
)
add_test(NAME IntegrationTest COMMAND test_integration)

# 性能优化测试
add_executable(test_performance test_performance.cpp)
target_link_libraries(test_performance
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
    Qt5::WebEngine
    Qt5::WebEngineWidgets
    Qt5::Concurrent
)
add_test(NAME PerformanceTest COMMAND test_performance)