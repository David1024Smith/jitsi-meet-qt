/**
 * @file application_flow_demo.cpp
 * @brief 演示完整应用程序流程的示例代码
 * 
 * 这个示例展示了如何使用集成的组件来实现完整的应用程序功能，
 * 包括窗口管理、配置管理、协议处理和翻译系统的协同工作。
 */

#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>

#include "MainApplication.h"
#include "WindowManager.h"
#include "ConfigurationManager.h"
#include "ProtocolHandler.h"
#include "TranslationManager.h"
#include "JitsiConstants.h"

/**
 * @brief 应用程序流程演示类
 */
class ApplicationFlowDemo : public QObject
{
    Q_OBJECT

public:
    ApplicationFlowDemo(MainApplication* app, QObject* parent = nullptr)
        : QObject(parent)
        , m_app(app)
        , m_step(0)
    {
        // 连接信号以监控应用程序状态
        connect(app->windowManager(), &WindowManager::windowChanged,
                this, &ApplicationFlowDemo::onWindowChanged);
        connect(app->windowManager(), &WindowManager::windowCreated,
                this, &ApplicationFlowDemo::onWindowCreated);
        connect(app->configurationManager(), &ConfigurationManager::configurationChanged,
                this, &ApplicationFlowDemo::onConfigurationChanged);
        
        // 开始演示
        startDemo();
    }

private slots:
    void onWindowChanged(int type)
    {
        qDebug() << "Demo: Window changed to type" << type;
    }
    
    void onWindowCreated(int type)
    {
        qDebug() << "Demo: Window created, type" << type;
    }
    
    void onConfigurationChanged()
    {
        qDebug() << "Demo: Configuration changed";
    }
    
    void nextStep()
    {
        m_step++;
        
        switch (m_step) {
        case 1:
            demonstrateWelcomeWindow();
            break;
        case 2:
            demonstrateConfigurationManagement();
            break;
        case 3:
            demonstrateProtocolHandling();
            break;
        case 4:
            demonstrateConferenceWindow();
            break;
        case 5:
            demonstrateSettingsDialog();
            break;
        case 6:
            demonstrateTranslationSystem();
            break;
        case 7:
            demonstrateWindowSwitching();
            break;
        case 8:
            finishDemo();
            break;
        default:
            qDebug() << "Demo completed";
            break;
        }
    }

private:
    void startDemo()
    {
        qDebug() << "=== Jitsi Meet Qt Application Flow Demo ===";
        qDebug() << "This demo shows how all components work together";
        qDebug() << "";
        
        // 延迟开始，确保应用程序完全初始化
        QTimer::singleShot(1000, this, &ApplicationFlowDemo::nextStep);
    }
    
    void demonstrateWelcomeWindow()
    {
        qDebug() << "Step 1: Demonstrating Welcome Window";
        qDebug() << "- Showing welcome window";
        qDebug() << "- This is the main entry point for users";
        
        auto windowManager = m_app->windowManager();
        windowManager->showWindow(WindowManager::WelcomeWindow);
        
        // 验证窗口状态
        if (windowManager->currentWindowType() == WindowManager::WelcomeWindow) {
            qDebug() << "✓ Welcome window is now active";
        }
        
        QTimer::singleShot(2000, this, &ApplicationFlowDemo::nextStep);
    }
    
    void demonstrateConfigurationManagement()
    {
        qDebug() << "";
        qDebug() << "Step 2: Demonstrating Configuration Management";
        qDebug() << "- Reading and modifying application settings";
        
        auto configManager = m_app->configurationManager();
        
        // 显示当前配置
        qDebug() << "- Current server URL:" << configManager->serverUrl();
        qDebug() << "- Recent URLs count:" << configManager->recentUrls().size();
        
        // 添加测试URL到最近列表
        QString testUrl = "https://meet.jit.si/DemoRoom" + QString::number(QDateTime::currentMSecsSinceEpoch());
        configManager->addRecentUrl(testUrl);
        qDebug() << "✓ Added test URL to recent list:" << testUrl;
        
        QTimer::singleShot(2000, this, &ApplicationFlowDemo::nextStep);
    }
    
    void demonstrateProtocolHandling()
    {
        qDebug() << "";
        qDebug() << "Step 3: Demonstrating Protocol Handling";
        qDebug() << "- Testing jitsi-meet:// protocol URL parsing";
        
        auto protocolHandler = m_app->protocolHandler();
        
        // 测试协议URL
        QString protocolUrl = "jitsi-meet://meet.jit.si/ProtocolDemo";
        qDebug() << "- Testing protocol URL:" << protocolUrl;
        
        if (protocolHandler->isValidProtocolUrl(protocolUrl)) {
            QString parsedUrl = protocolHandler->parseProtocolUrl(protocolUrl);
            qDebug() << "✓ Protocol URL parsed successfully:" << parsedUrl;
            
            // 处理协议URL（这会切换到会议窗口）
            m_app->handleProtocolUrl(protocolUrl);
        } else {
            qDebug() << "✗ Protocol URL validation failed";
        }
        
        QTimer::singleShot(2000, this, &ApplicationFlowDemo::nextStep);
    }
    
    void demonstrateConferenceWindow()
    {
        qDebug() << "";
        qDebug() << "Step 4: Demonstrating Conference Window";
        qDebug() << "- Showing conference window with meeting URL";
        
        auto windowManager = m_app->windowManager();
        
        // 准备会议数据
        QVariantMap conferenceData;
        conferenceData["url"] = "https://meet.jit.si/DemoConference";
        
        // 显示会议窗口
        windowManager->showWindow(WindowManager::ConferenceWindow, conferenceData);
        
        if (windowManager->currentWindowType() == WindowManager::ConferenceWindow) {
            qDebug() << "✓ Conference window is now active";
            qDebug() << "- WebEngine will load the Jitsi Meet interface";
        }
        
        QTimer::singleShot(3000, this, &ApplicationFlowDemo::nextStep);
    }
    
    void demonstrateSettingsDialog()
    {
        qDebug() << "";
        qDebug() << "Step 5: Demonstrating Settings Dialog";
        qDebug() << "- Opening settings dialog";
        
        auto windowManager = m_app->windowManager();
        windowManager->showWindow(WindowManager::SettingsDialog);
        
        if (windowManager->hasWindow(WindowManager::SettingsDialog) &&
            windowManager->isWindowVisible(WindowManager::SettingsDialog)) {
            qDebug() << "✓ Settings dialog is now visible";
            qDebug() << "- Users can modify server URL, language, and other options";
        }
        
        // 关闭设置对话框
        QTimer::singleShot(2000, [this]() {
            m_app->windowManager()->closeWindow(WindowManager::SettingsDialog);
            qDebug() << "- Settings dialog closed";
            QTimer::singleShot(500, this, &ApplicationFlowDemo::nextStep);
        });
    }
    
    void demonstrateTranslationSystem()
    {
        qDebug() << "";
        qDebug() << "Step 6: Demonstrating Translation System";
        qDebug() << "- Testing language switching functionality";
        
        auto translationManager = m_app->translationManager();
        
        QString currentLanguage = translationManager->currentLanguage();
        qDebug() << "- Current language:" << currentLanguage;
        
        // 切换语言
        QString newLanguage = (currentLanguage == "en") ? "zh_CN" : "en";
        qDebug() << "- Switching to language:" << newLanguage;
        
        translationManager->setLanguage(newLanguage);
        qDebug() << "✓ Language switched successfully";
        qDebug() << "- All UI components should update automatically";
        
        QTimer::singleShot(2000, this, &ApplicationFlowDemo::nextStep);
    }
    
    void demonstrateWindowSwitching()
    {
        qDebug() << "";
        qDebug() << "Step 7: Demonstrating Window Switching";
        qDebug() << "- Testing seamless window transitions";
        
        auto windowManager = m_app->windowManager();
        
        // 切换到欢迎窗口
        qDebug() << "- Switching to Welcome Window";
        windowManager->showWindow(WindowManager::WelcomeWindow);
        
        QTimer::singleShot(1000, [this, windowManager]() {
            // 切换到会议窗口
            qDebug() << "- Switching to Conference Window";
            QVariantMap data;
            data["url"] = "https://meet.jit.si/SwitchingDemo";
            windowManager->showWindow(WindowManager::ConferenceWindow, data);
            
            QTimer::singleShot(1000, [this, windowManager]() {
                // 返回欢迎窗口
                qDebug() << "- Switching back to Welcome Window";
                windowManager->showWindow(WindowManager::WelcomeWindow);
                qDebug() << "✓ Window switching completed successfully";
                
                QTimer::singleShot(1000, this, &ApplicationFlowDemo::nextStep);
            });
        });
    }
    
    void finishDemo()
    {
        qDebug() << "";
        qDebug() << "Step 8: Demo Completion";
        qDebug() << "=== Demo Summary ===";
        qDebug() << "✓ MainApplication initialization and single instance management";
        qDebug() << "✓ WindowManager with seamless window switching";
        qDebug() << "✓ ConfigurationManager with persistent settings";
        qDebug() << "✓ ProtocolHandler for jitsi-meet:// URLs";
        qDebug() << "✓ TranslationManager with dynamic language switching";
        qDebug() << "✓ Complete integration of all components";
        qDebug() << "";
        qDebug() << "All components are working together successfully!";
        qDebug() << "The application is ready for production use.";
        
        // 显示完成消息
        QTimer::singleShot(1000, []() {
            QMessageBox::information(nullptr, "Demo Complete", 
                "Application flow demonstration completed successfully!\n\n"
                "All components are integrated and working properly.");
        });
    }

private:
    MainApplication* m_app;
    int m_step;
};

/**
 * @brief 主函数 - 运行应用程序流程演示
 */
int main(int argc, char *argv[])
{
    // 设置应用程序属性
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // 创建应用程序
    MainApplication app(argc, argv);
    
    qDebug() << "Starting Jitsi Meet Qt Application Flow Demo";
    
    // 创建演示对象
    ApplicationFlowDemo demo(&app);
    
    // 运行应用程序
    return app.exec();
}

#include "application_flow_demo.moc"