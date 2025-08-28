#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "ConfigurationManager.h"
#include "models/ApplicationSettings.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Simple Config Test");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Jitsi Meet Qt");
    
    qDebug() << "=== Configuration Management System Test ===";
    
    try {
        // 创建配置管理器
        ConfigurationManager configManager;
        
        qDebug() << "1. Testing default configuration...";
        ApplicationSettings defaultConfig = configManager.loadConfiguration();
        qDebug() << "   Default server URL:" << defaultConfig.defaultServerUrl;
        qDebug() << "   Default language:" << defaultConfig.language;
        qDebug() << "   Default timeout:" << defaultConfig.serverTimeout;
        qDebug() << "   Configuration valid:" << defaultConfig.isValid();
        
        qDebug() << "\n2. Testing configuration modification...";
        configManager.setServerUrl("https://test.example.com");
        configManager.setLanguage("zh-CN");
        configManager.setDarkMode(true);
        
        qDebug() << "   Modified server URL:" << configManager.serverUrl();
        qDebug() << "   Modified language:" << configManager.language();
        qDebug() << "   Modified dark mode:" << configManager.isDarkMode();
        
        qDebug() << "\n3. Testing recent URLs management...";
        configManager.clearRecentUrls();
        configManager.addRecentUrl("https://meet.jit.si/room1");
        configManager.addRecentUrl("https://meet.jit.si/room2");
        configManager.addRecentUrl("https://meet.jit.si/room3");
        
        QStringList recentUrls = configManager.recentUrls();
        qDebug() << "   Recent URLs count:" << recentUrls.size();
        for (int i = 0; i < recentUrls.size(); ++i) {
            qDebug() << "   Recent URL" << (i+1) << ":" << recentUrls.at(i);
        }
        
        qDebug() << "\n4. Testing URL validation...";
        QStringList testUrls = {
            "https://meet.jit.si",
            "http://localhost:8080",
            "invalid-url",
            "ftp://example.com"
        };
        
        for (const QString& url : testUrls) {
            QString originalUrl = configManager.serverUrl();
            configManager.setServerUrl(url);
            QString actualUrl = configManager.serverUrl();
            bool isValid = (actualUrl == url);
            qDebug() << "   URL:" << url << "-> Valid:" << isValid;
            if (!isValid) {
                configManager.setServerUrl(originalUrl); // 恢复原始URL
            }
        }
        
        qDebug() << "\n5. Testing configuration persistence...";
        ApplicationSettings testConfig;
        testConfig.defaultServerUrl = "https://persistent.example.com";
        testConfig.language = "ja";
        testConfig.darkMode = true;
        testConfig.maxRecentItems = 15;
        testConfig.autoJoinAudio = false;
        testConfig.autoJoinVideo = true;
        
        configManager.saveConfiguration(testConfig);
        qDebug() << "   Configuration saved successfully";
        
        // 创建新的配置管理器实例来测试加载
        ConfigurationManager newConfigManager;
        ApplicationSettings loadedConfig = newConfigManager.loadConfiguration();
        
        qDebug() << "   Loaded server URL:" << loadedConfig.defaultServerUrl;
        qDebug() << "   Loaded language:" << loadedConfig.language;
        qDebug() << "   Loaded dark mode:" << loadedConfig.darkMode;
        qDebug() << "   Loaded max recent items:" << loadedConfig.maxRecentItems;
        qDebug() << "   Loaded auto join audio:" << loadedConfig.autoJoinAudio;
        qDebug() << "   Loaded auto join video:" << loadedConfig.autoJoinVideo;
        
        bool persistenceWorked = (loadedConfig.defaultServerUrl == testConfig.defaultServerUrl &&
                                 loadedConfig.language == testConfig.language &&
                                 loadedConfig.darkMode == testConfig.darkMode &&
                                 loadedConfig.maxRecentItems == testConfig.maxRecentItems &&
                                 loadedConfig.autoJoinAudio == testConfig.autoJoinAudio &&
                                 loadedConfig.autoJoinVideo == testConfig.autoJoinVideo);
        
        qDebug() << "   Persistence test:" << (persistenceWorked ? "PASSED" : "FAILED");
        
        qDebug() << "\n6. Testing configuration validation...";
        bool isValid = configManager.validateConfiguration();
        qDebug() << "   Configuration validation:" << (isValid ? "PASSED" : "FAILED");
        
        qDebug() << "\n7. Testing reset to defaults...";
        configManager.resetToDefaults();
        ApplicationSettings resetConfig = configManager.currentConfiguration();
        qDebug() << "   Reset server URL:" << resetConfig.defaultServerUrl;
        qDebug() << "   Reset language:" << resetConfig.language;
        qDebug() << "   Reset dark mode:" << resetConfig.darkMode;
        
        bool resetWorked = (resetConfig.defaultServerUrl == "https://meet.jit.si" &&
                           resetConfig.language == "auto" &&
                           resetConfig.darkMode == false);
        
        qDebug() << "   Reset test:" << (resetWorked ? "PASSED" : "FAILED");
        
        qDebug() << "\n8. Testing ApplicationSettings class...";
        ApplicationSettings settings1;
        ApplicationSettings settings2;
        
        qDebug() << "   Default settings equality:" << (settings1 == settings2 ? "PASSED" : "FAILED");
        
        settings2.darkMode = true;
        qDebug() << "   Modified settings inequality:" << (settings1 != settings2 ? "PASSED" : "FAILED");
        
        ApplicationSettings settings3(settings1);
        qDebug() << "   Copy constructor:" << (settings1 == settings3 ? "PASSED" : "FAILED");
        
        ApplicationSettings settings4;
        settings4 = settings2;
        qDebug() << "   Assignment operator:" << (settings2 == settings4 ? "PASSED" : "FAILED");
        
        qDebug() << "   Settings validation:" << (settings1.isValid() ? "PASSED" : "FAILED");
        
        QVariantMap variantMap = settings1.toVariantMap();
        ApplicationSettings settings5;
        settings5.fromVariantMap(variantMap);
        qDebug() << "   Variant map conversion:" << (settings1 == settings5 ? "PASSED" : "FAILED");
        
        qDebug() << "\n=== All Tests Completed Successfully! ===";
        
    } catch (const std::exception& e) {
        qCritical() << "Test failed with exception:" << e.what();
        return 1;
    } catch (...) {
        qCritical() << "Test failed with unknown exception";
        return 1;
    }
    
    return 0;
}