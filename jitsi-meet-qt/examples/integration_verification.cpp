/**
 * @file integration_verification.cpp
 * @brief 集成验证脚本，测试所有组件的集成点
 * 
 * 这个脚本验证所有组件是否正确集成，包括：
 * - 组件初始化
 * - 信号槽连接
 * - 数据流传递
 * - 错误处理
 * - 配置管理
 */

#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QSignalSpy>
#include <QTest>

#include "MainApplication.h"
#include "WindowManager.h"
#include "ConfigurationManager.h"
#include "ProtocolHandler.h"
#include "TranslationManager.h"
#include "JitsiConstants.h"

class IntegrationVerifier : public QObject
{
    Q_OBJECT

public:
    IntegrationVerifier(MainApplication* app, QObject* parent = nullptr)
        : QObject(parent)
        , m_app(app)
        , m_testsPassed(0)
        , m_testsFailed(0)
    {
        startVerification();
    }
    
    int testsPassed() const { return m_testsPassed; }
    int testsFailed() const { return m_testsFailed; }

private slots:
    void runNextTest()
    {
        if (m_currentTest < m_tests.size()) {
            auto test = m_tests[m_currentTest];
            qDebug() << "Running test:" << test.name;
            
            bool result = (this->*test.function)();
            
            if (result) {
                qDebug() << "✓ PASS:" << test.name;
                m_testsPassed++;
            } else {
                qDebug() << "✗ FAIL:" << test.name;
                m_testsFailed++;
            }
            
            m_currentTest++;
            QTimer::singleShot(100, this, &IntegrationVerifier::runNextTest);
        } else {
            finishVerification();
        }
    }

private:
    struct TestCase {
        QString name;
        bool (IntegrationVerifier::*function)();
    };
    
    void startVerification()
    {
        qDebug() << "=== Integration Verification Started ===";
        
        // 定义测试用例
        m_tests = {
            {"Component Initialization", &IntegrationVerifier::testComponentInitialization},
            {"Signal-Slot Connections", &IntegrationVerifier::testSignalSlotConnections},
            {"Window Manager Integration", &IntegrationVerifier::testWindowManagerIntegration},
            {"Configuration Manager Integration", &IntegrationVerifier::testConfigurationManagerIntegration},
            {"Protocol Handler Integration", &IntegrationVerifier::testProtocolHandlerIntegration},
            {"Translation Manager Integration", &IntegrationVerifier::testTranslationManagerIntegration},
            {"Data Flow Verification", &IntegrationVerifier::testDataFlow},
            {"Error Handling Integration", &IntegrationVerifier::testErrorHandling},
            {"Window State Management", &IntegrationVerifier::testWindowStateManagement},
            {"Configuration Persistence", &IntegrationVerifier::testConfigurationPersistence}
        };
        
        m_currentTest = 0;
        QTimer::singleShot(500, this, &IntegrationVerifier::runNextTest);
    }
    
    void finishVerification()
    {
        qDebug() << "";
        qDebug() << "=== Integration Verification Complete ===";
        qDebug() << "Tests Passed:" << m_testsPassed;
        qDebug() << "Tests Failed:" << m_testsFailed;
        qDebug() << "Success Rate:" << QString::number((double)m_testsPassed / (m_testsPassed + m_testsFailed) * 100, 'f', 1) << "%";
        
        if (m_testsFailed == 0) {
            qDebug() << "🎉 All integration tests passed! The application is ready for use.";
        } else {
            qDebug() << "⚠️  Some integration tests failed. Please review the issues above.";
        }
        
        // 退出应用程序
        QTimer::singleShot(1000, m_app, &QApplication::quit);
    }
    
    bool testComponentInitialization()
    {
        // 验证所有核心组件都已正确初始化
        bool allInitialized = true;
        
        if (!m_app->windowManager()) {
            qDebug() << "  - WindowManager not initialized";
            allInitialized = false;
        }
        
        if (!m_app->configurationManager()) {
            qDebug() << "  - ConfigurationManager not initialized";
            allInitialized = false;
        }
        
        if (!m_app->protocolHandler()) {
            qDebug() << "  - ProtocolHandler not initialized";
            allInitialized = false;
        }
        
        if (!m_app->translationManager()) {
            qDebug() << "  - TranslationManager not initialized";
            allInitialized = false;
        }
        
        return allInitialized;
    }
    
    bool testSignalSlotConnections()
    {
        // 验证关键信号槽连接是否正确建立
        bool connectionsValid = true;
        
        // 测试WindowManager信号
        QSignalSpy windowChangedSpy(m_app->windowManager(), &WindowManager::windowChanged);
        m_app->windowManager()->showWindow(WindowManager::WelcomeWindow);
        
        if (windowChangedSpy.count() == 0) {
            qDebug() << "  - WindowManager::windowChanged signal not connected";
            connectionsValid = false;
        }
        
        // 测试ConfigurationManager信号
        QSignalSpy configChangedSpy(m_app->configurationManager(), &ConfigurationManager::configurationChanged);
        m_app->configurationManager()->setServerUrl("https://test.example.com");
        
        if (configChangedSpy.count() == 0) {
            qDebug() << "  - ConfigurationManager::configurationChanged signal not connected";
            connectionsValid = false;
        }
        
        return connectionsValid;
    }
    
    bool testWindowManagerIntegration()
    {
        auto windowManager = m_app->windowManager();
        
        // 测试窗口创建和切换
        windowManager->showWindow(WindowManager::WelcomeWindow);
        if (windowManager->currentWindowType() != WindowManager::WelcomeWindow) {
            qDebug() << "  - Failed to show WelcomeWindow";
            return false;
        }
        
        // 测试数据传递
        QVariantMap data;
        data["url"] = "https://meet.jit.si/TestRoom";
        windowManager->showWindow(WindowManager::ConferenceWindow, data);
        
        if (windowManager->currentWindowType() != WindowManager::ConferenceWindow) {
            qDebug() << "  - Failed to show ConferenceWindow";
            return false;
        }
        
        return true;
    }
    
    bool testConfigurationManagerIntegration()
    {
        auto configManager = m_app->configurationManager();
        
        // 测试配置读写
        QString originalUrl = configManager->serverUrl();
        QString testUrl = "https://integration-test.example.com";
        
        configManager->setServerUrl(testUrl);
        if (configManager->serverUrl() != testUrl) {
            qDebug() << "  - Failed to set server URL";
            return false;
        }
        
        // 测试最近URL管理
        QString recentUrl = "https://meet.jit.si/IntegrationTest";
        configManager->addRecentUrl(recentUrl);
        
        if (!configManager->recentUrls().contains(recentUrl)) {
            qDebug() << "  - Failed to add recent URL";
            return false;
        }
        
        // 恢复原始配置
        configManager->setServerUrl(originalUrl);
        
        return true;
    }
    
    bool testProtocolHandlerIntegration()
    {
        auto protocolHandler = m_app->protocolHandler();
        
        // 测试协议URL验证
        QString validUrl = "jitsi-meet://meet.jit.si/TestRoom";
        if (!protocolHandler->isValidProtocolUrl(validUrl)) {
            qDebug() << "  - Valid protocol URL rejected";
            return false;
        }
        
        QString invalidUrl = "http://example.com";
        if (protocolHandler->isValidProtocolUrl(invalidUrl)) {
            qDebug() << "  - Invalid protocol URL accepted";
            return false;
        }
        
        // 测试URL解析
        QString parsedUrl = protocolHandler->parseProtocolUrl(validUrl);
        if (parsedUrl.isEmpty() || !parsedUrl.contains("TestRoom")) {
            qDebug() << "  - Protocol URL parsing failed";
            return false;
        }
        
        return true;
    }
    
    bool testTranslationManagerIntegration()
    {
        auto translationManager = m_app->translationManager();
        
        // 测试语言切换
        QString originalLanguage = translationManager->currentLanguage();
        QString testLanguage = (originalLanguage == "en") ? "zh_CN" : "en";
        
        QSignalSpy languageChangedSpy(translationManager, &TranslationManager::languageChanged);
        translationManager->setLanguage(testLanguage);
        
        if (languageChangedSpy.count() == 0) {
            qDebug() << "  - Language change signal not emitted";
            return false;
        }
        
        if (translationManager->currentLanguage() != testLanguage) {
            qDebug() << "  - Language not changed correctly";
            return false;
        }
        
        // 恢复原始语言
        translationManager->setLanguage(originalLanguage);
        
        return true;
    }
    
    bool testDataFlow()
    {
        // 测试组件间数据流
        auto windowManager = m_app->windowManager();
        auto configManager = m_app->configurationManager();
        
        QSignalSpy dataTransferSpy(windowManager, &WindowManager::dataTransferred);
        
        // 模拟用户加入会议的完整流程
        windowManager->showWindow(WindowManager::WelcomeWindow);
        
        QVariantMap conferenceData;
        conferenceData["url"] = "https://meet.jit.si/DataFlowTest";
        windowManager->showWindow(WindowManager::ConferenceWindow, conferenceData);
        
        // 验证数据传递信号
        if (dataTransferSpy.count() == 0) {
            qDebug() << "  - Data transfer signal not emitted";
            return false;
        }
        
        return true;
    }
    
    bool testErrorHandling()
    {
        // 测试错误处理集成
        // 这里可以模拟各种错误情况并验证处理是否正确
        
        // 测试无效URL处理
        QString invalidUrl = "invalid-url-format";
        m_app->handleProtocolUrl(invalidUrl);
        
        // 应用程序应该优雅地处理无效URL，不崩溃
        // 验证当前窗口是欢迎窗口（fallback行为）
        if (m_app->windowManager()->currentWindowType() != WindowManager::WelcomeWindow) {
            qDebug() << "  - Invalid URL not handled gracefully";
            return false;
        }
        
        return true;
    }
    
    bool testWindowStateManagement()
    {
        auto windowManager = m_app->windowManager();
        
        // 测试窗口状态跟踪
        windowManager->showWindow(WindowManager::WelcomeWindow);
        
        if (!windowManager->isWindowVisible(WindowManager::WelcomeWindow)) {
            qDebug() << "  - Window visibility state not tracked correctly";
            return false;
        }
        
        if (!windowManager->hasWindow(WindowManager::WelcomeWindow)) {
            qDebug() << "  - Window existence not tracked correctly";
            return false;
        }
        
        return true;
    }
    
    bool testConfigurationPersistence()
    {
        auto configManager = m_app->configurationManager();
        
        // 测试配置持久化
        // 注意：这个测试不会实际重启应用程序，只验证配置保存机制
        
        QString testUrl = "https://persistence-test.example.com";
        configManager->addRecentUrl(testUrl);
        
        // 验证URL被添加到列表中
        if (!configManager->recentUrls().contains(testUrl)) {
            qDebug() << "  - Configuration not persisted correctly";
            return false;
        }
        
        return true;
    }

private:
    MainApplication* m_app;
    QList<TestCase> m_tests;
    int m_currentTest;
    int m_testsPassed;
    int m_testsFailed;
};

int main(int argc, char *argv[])
{
    // 设置应用程序属性
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // 创建应用程序
    MainApplication app(argc, argv);
    
    qDebug() << "Jitsi Meet Qt - Integration Verification";
    qDebug() << "This tool verifies that all components are properly integrated.";
    qDebug() << "";
    
    // 创建验证器
    IntegrationVerifier verifier(&app);
    
    // 运行应用程序
    int result = app.exec();
    
    // 返回结果（0表示成功，非0表示有失败的测试）
    return (verifier.testsFailed() == 0) ? 0 : 1;
}

#include "integration_verification.moc"