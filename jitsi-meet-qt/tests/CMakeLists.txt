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
    ${CMAKE_SOURCE_DIR}/src/XMPPClient.cpp
    ${CMAKE_SOURCE_DIR}/src/WebRTCEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/MediaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ConferenceManager.cpp
    ${CMAKE_SOURCE_DIR}/src/AuthenticationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ChatManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ScreenShareManager.cpp
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
    Qt5::Network
    Qt5::WebSockets
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    Qt5::Xml
)

# 协议处理器测试
add_executable(test_protocol_handler test_protocol_handler.cpp)
target_link_libraries(test_protocol_handler
    TestLib
    Qt5::Test
    Qt5::Core
)
add_test(NAME ProtocolHandlerTest COMMAND test_protocol_handler)

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
    Qt5::Network
    Qt5::WebSockets
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    Qt5::Xml
)

# 设置测试属性
set_target_properties(test_integration PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# 添加测试到CTest
add_test(NAME IntegrationTest COMMAND test_integration)
set_tests_properties(IntegrationTest PROPERTIES
    TIMEOUT 300  # 5分钟超时
    LABELS "integration"
)

# 性能优化测试
add_executable(test_performance test_performance.cpp)
target_link_libraries(test_performance
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::WebSockets
    Qt5::Multimedia
    Qt5::MultimediaWidgets
)
add_test(NAME PerformanceTest COMMAND test_performance)

# XMPP客户端测试
add_executable(test_xmpp_client test_xmpp_client.cpp)
target_link_libraries(test_xmpp_client
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Network
    Qt5::WebSockets
    Qt5::Xml
)
add_test(NAME XMPPClientTest COMMAND test_xmpp_client)

# WebRTC Engine Test
add_executable(test_webrtc_engine test_webrtc_engine.cpp)
target_link_libraries(test_webrtc_engine
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Network
    Qt5::Multimedia
    Qt5::MultimediaWidgets
)
add_test(NAME WebRTCEngineTest COMMAND test_webrtc_engine)

# MediaManager Test
add_executable(test_media_manager test_media_manager.cpp)
target_link_libraries(test_media_manager
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
    Qt5::Multimedia
    Qt5::MultimediaWidgets
)
add_test(NAME MediaManagerTest COMMAND test_media_manager)

# ConferenceManager Test
add_executable(test_conference_manager test_conference_manager.cpp)
target_link_libraries(test_conference_manager
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Network
    Qt5::WebSockets
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    Qt5::Xml
)
add_test(NAME ConferenceManagerTest COMMAND test_conference_manager)

# ChatManager Test
add_executable(test_chat_manager test_chat_manager.cpp)
target_link_libraries(test_chat_manager
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Network
    Qt5::WebSockets
    Qt5::Xml
)
add_test(NAME ChatManagerTest COMMAND test_chat_manager)

# ScreenShareManager Test
add_executable(test_screen_share_manager test_screen_share_manager.cpp)
target_link_libraries(test_screen_share_manager
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
    Qt5::Multimedia
    Qt5::MultimediaWidgets
)
add_test(NAME ScreenShareManagerTest COMMAND test_screen_share_manager)

# AuthenticationManager Test
add_executable(test_authentication_manager test_authentication_manager.cpp)
target_link_libraries(test_authentication_manager
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Network
)
add_test(NAME AuthenticationManagerTest COMMAND test_authentication_manager)

# WindowManager Test
add_executable(test_windowmanager test_windowmanager.cpp)
target_link_libraries(test_windowmanager
    TestLib
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
)
add_test(NAME WindowManagerTest COMMAND test_windowmanager)