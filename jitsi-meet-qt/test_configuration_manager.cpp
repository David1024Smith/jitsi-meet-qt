#include "include/ConfigurationManager.h"
#include "include/models/ApplicationSettings.h"
#include <QCoreApplication>
#include <QDebug>
#include <iostream>

/**
 * @brief 测试配置管理器的基本功能
 */
void testConfigurationManager() {
    qDebug() << "=== Testing ConfigurationManager ===";
    
    // 创建配置管理器实例
    ConfigurationManager configManager;
    
    // 测试 C++17 std::optional 加载配置
    qDebug() << "\n1. Testing configuration loading with std::optional:";
    if (auto config = configManager.loadConfiguration()) {
        qDebug() << "✓ Configuration loaded successfully";
        qDebug() << "  Server URL:" << config->defaultServerUrl;
        qDebug() << "  Language:" << config->language;
        qDebug() << "  Window size:" << config->windowGeometry.size();
    } else {
        qDebug() << "✗ Failed to load configuration";
    }
    
    // 测试 C++17 std::string_view URL 验证
    qDebug() << "\n2. Testing URL validation with std::string_view:";
    std::string_view validUrl = "https://meet.jit.si";
    std::string_view invalidUrl = "invalid-url";
    
    bool result1 = configManager.setServerUrl(validUrl);
    bool result2 = configManager.setServerUrl(invalidUrl);
    
    qDebug() << "✓ Valid URL test:" << (result1 ? "PASSED" : "FAILED");
    qDebug() << "✓ Invalid URL test:" << (!result2 ? "PASSED" : "FAILED");
    
    // 测试 C++17 结构化绑定验证
    qDebug() << "\n3. Testing comprehensive validation with structured bindings:";
    auto [isValid, errors] = configManager.performComprehensiveValidation();
    
    if (isValid) {
        qDebug() << "✓ Configuration is valid";
    } else {
        qDebug() << "✗ Configuration validation failed:";
        for (const QString& error : errors) {
            qDebug() << "  -" << error;
        }
    }
    
    // 测试配置保存和加载
    qDebug() << "\n4. Testing configuration save/load:";
    ApplicationSettings testConfig;
    testConfig.defaultServerUrl = "https://test.example.com";
    testConfig.language = "zh-CN";
    testConfig.darkMode = true;
    
    configManager.saveConfiguration(testConfig);
    
    if (auto loadedConfig = configManager.loadConfiguration()) {
        bool urlMatch = loadedConfig->defaultServerUrl == testConfig.defaultServerUrl;
        bool langMatch = loadedConfig->language == testConfig.language;
        bool darkModeMatch = loadedConfig->darkMode == testConfig.darkMode;
        
        qDebug() << "✓ Save/Load test:" << 
                    (urlMatch && langMatch && darkModeMatch ? "PASSED" : "FAILED");
        qDebug() << "  URL match:" << urlMatch;
        qDebug() << "  Language match:" << langMatch;
        qDebug() << "  Dark mode match:" << darkModeMatch;
    } else {
        qDebug() << "✗ Failed to reload configuration";
    }
    
    // 测试最近URL管理
    qDebug() << "\n5. Testing recent URL management:";
    configManager.addRecentUrl("https://meet.jit.si/test-room-1");
    configManager.addRecentUrl("https://meet.jit.si/test-room-2");
    configManager.addRecentUrl("invalid-url"); // 这个应该被忽略
    
    QStringList recentUrls = configManager.recentUrls();
    qDebug() << "✓ Recent URLs count:" << recentUrls.size();
    qDebug() << "✓ Recent URLs:" << recentUrls;
    
    // 测试默认配置恢复
    qDebug() << "\n6. Testing default configuration reset:";
    configManager.resetToDefaults();
    
    if (auto defaultConfig = configManager.loadConfiguration()) {
        bool isDefault = (defaultConfig->defaultServerUrl == "https://meet.jit.si" &&
                         defaultConfig->language == "auto" &&
                         !defaultConfig->darkMode);
        qDebug() << "✓ Reset to defaults:" << (isDefault ? "PASSED" : "FAILED");
    }
    
    qDebug() << "\n=== ConfigurationManager tests completed ===";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 设置应用程序信息
    QCoreApplication::setApplicationName("Jitsi Meet Qt");
    QCoreApplication::setOrganizationName("Jitsi Meet Qt");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    std::cout << "Starting ConfigurationManager tests...\n" << std::endl;
    
    try {
        testConfigurationManager();
        std::cout << "\nAll tests completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}