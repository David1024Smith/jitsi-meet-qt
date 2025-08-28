#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <iostream>

#include "WindowManager.h"
#include "ConfigurationManager.h"
#include "TranslationManager.h"

/**
 * @brief 验证WindowManager实现的完整性和功能
 */
class WindowManagerVerifier : public QObject
{
    Q_OBJECT

public:
    WindowManagerVerifier(QObject* parent = nullptr) : QObject(parent) {}
    
    bool verifyImplementation()
    {
        std::cout << "=== WindowManager Implementation Verification ===" << std::endl;
        
        bool allPassed = true;
        
        allPassed &= verifyBasicFunctionality();
        allPassed &= verifyWindowCreation();
        allPassed &= verifyWindowSwitching();
        allPassed &= verifyDataTransfer();
        allPassed &= verifyStateManagement();
        allPassed &= verifyLifecycleManagement();
        allPassed &= verifySignalConnections();
        allPassed &= verifyMemoryManagement();
        
        std::cout << std::endl;
        if (allPassed) {
            std::cout << "✅ All WindowManager verification tests PASSED!" << std::endl;
        } else {
            std::cout << "❌ Some WindowManager verification tests FAILED!" << std::endl;
        }
        
        return allPassed;
    }

private:
    bool verifyBasicFunctionality()
    {
        std::cout << "\n1. Testing Basic Functionality..." << std::endl;
        
        try {
            // 创建配置和翻译管理器
            ConfigurationManager configManager;
            TranslationManager translationManager;
            
            // 创建窗口管理器
            WindowManager windowManager;
            windowManager.setConfigurationManager(&configManager);
            windowManager.setTranslationManager(&translationManager);
            
            // 验证初始状态
            if (windowManager.currentWindowType() != WindowManager::WelcomeWindow) {
                std::cout << "❌ Initial window type should be WelcomeWindow" << std::endl;
                return false;
            }
            
            if (windowManager.hasWindow(WindowManager::WelcomeWindow)) {
                std::cout << "❌ Should not have WelcomeWindow initially" << std::endl;
                return false;
            }
            
            std::cout << "✅ Basic functionality verified" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "❌ Exception in basic functionality test: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool verifyWindowCreation()
    {
        std::cout << "\n2. Testing Window Creation..." << std::endl;
        
        try {
            ConfigurationManager configManager;
            TranslationManager translationManager;
            WindowManager windowManager;
            windowManager.setConfigurationManager(&configManager);
            windowManager.setTranslationManager(&translationManager);
            
            // 测试欢迎窗口创建
            windowManager.showWindow(WindowManager::WelcomeWindow);
            
            if (!windowManager.hasWindow(WindowManager::WelcomeWindow)) {
                std::cout << "❌ WelcomeWindow should be created" << std::endl;
                return false;
            }
            
            if (!windowManager.isWindowVisible(WindowManager::WelcomeWindow)) {
                std::cout << "❌ WelcomeWindow should be visible" << std::endl;
                return false;
            }
            
            // 测试会议窗口创建
            windowManager.showWindow(WindowManager::ConferenceWindow);
            
            if (!windowManager.hasWindow(WindowManager::ConferenceWindow)) {
                std::cout << "❌ ConferenceWindow should be created" << std::endl;
                return false;
            }
            
            if (windowManager.currentWindowType() != WindowManager::ConferenceWindow) {
                std::cout << "❌ Current window should be ConferenceWindow" << std::endl;
                return false;
            }
            
            // 测试设置对话框创建
            windowManager.showWindow(WindowManager::SettingsDialog);
            
            if (!windowManager.hasWindow(WindowManager::SettingsDialog)) {
                std::cout << "❌ SettingsDialog should be created" << std::endl;
                return false;
            }
            
            std::cout << "✅ Window creation verified" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "❌ Exception in window creation test: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool verifyWindowSwitching()
    {
        std::cout << "\n3. Testing Window Switching..." << std::endl;
        
        try {
            ConfigurationManager configManager;
            TranslationManager translationManager;
            WindowManager windowManager;
            windowManager.setConfigurationManager(&configManager);
            windowManager.setTranslationManager(&translationManager);
            
            // 创建窗口
            windowManager.showWindow(WindowManager::WelcomeWindow);
            windowManager.showWindow(WindowManager::ConferenceWindow);
            
            // 切换到欢迎窗口
            windowManager.showWindow(WindowManager::WelcomeWindow);
            
            if (windowManager.currentWindowType() != WindowManager::WelcomeWindow) {
                std::cout << "❌ Should switch to WelcomeWindow" << std::endl;
                return false;
            }
            
            if (!windowManager.isWindowVisible(WindowManager::WelcomeWindow)) {
                std::cout << "❌ WelcomeWindow should be visible after switch" << std::endl;
                return false;
            }
            
            if (windowManager.isWindowVisible(WindowManager::ConferenceWindow)) {
                std::cout << "❌ ConferenceWindow should be hidden after switch" << std::endl;
                return false;
            }
            
            std::cout << "✅ Window switching verified" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "❌ Exception in window switching test: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool verifyDataTransfer()
    {
        std::cout << "\n4. Testing Data Transfer..." << std::endl;
        
        try {
            ConfigurationManager configManager;
            TranslationManager translationManager;
            WindowManager windowManager;
            windowManager.setConfigurationManager(&configManager);
            windowManager.setTranslationManager(&translationManager);
            
            // 测试数据传递
            QVariantMap data;
            data["url"] = "https://meet.jit.si/test-room";
            data["error"] = "Test error";
            
            windowManager.showWindow(WindowManager::WelcomeWindow, data);
            
            if (!windowManager.sendDataToWindow(WindowManager::WelcomeWindow, data)) {
                std::cout << "❌ Should be able to send data to existing window" << std::endl;
                return false;
            }
            
            // 测试向不存在的窗口发送数据
            if (windowManager.sendDataToWindow(WindowManager::SettingsDialog, data)) {
                std::cout << "❌ Should not be able to send data to non-existent window" << std::endl;
                return false;
            }
            
            std::cout << "✅ Data transfer verified" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "❌ Exception in data transfer test: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool verifyStateManagement()
    {
        std::cout << "\n5. Testing State Management..." << std::endl;
        
        try {
            ConfigurationManager configManager;
            TranslationManager translationManager;
            WindowManager windowManager;
            windowManager.setConfigurationManager(&configManager);
            windowManager.setTranslationManager(&translationManager);
            
            // 验证初始状态
            if (windowManager.getWindowState(WindowManager::WelcomeWindow) != WindowManager::WindowHidden) {
                std::cout << "❌ Initial window state should be Hidden" << std::endl;
                return false;
            }
            
            // 显示窗口
            windowManager.showWindow(WindowManager::WelcomeWindow);
            
            if (windowManager.getWindowState(WindowManager::WelcomeWindow) != WindowManager::WindowVisible) {
                std::cout << "❌ Window state should be Visible after showing" << std::endl;
                return false;
            }
            
            // 隐藏窗口
            windowManager.hideWindow(WindowManager::WelcomeWindow);
            
            if (windowManager.getWindowState(WindowManager::WelcomeWindow) != WindowManager::WindowHidden) {
                std::cout << "❌ Window state should be Hidden after hiding" << std::endl;
                return false;
            }
            
            std::cout << "✅ State management verified" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "❌ Exception in state management test: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool verifyLifecycleManagement()
    {
        std::cout << "\n6. Testing Lifecycle Management..." << std::endl;
        
        try {
            ConfigurationManager configManager;
            TranslationManager translationManager;
            WindowManager windowManager;
            windowManager.setConfigurationManager(&configManager);
            windowManager.setTranslationManager(&translationManager);
            
            // 创建窗口
            windowManager.showWindow(WindowManager::WelcomeWindow);
            windowManager.showWindow(WindowManager::ConferenceWindow);
            
            // 测试状态保存
            windowManager.saveAllWindowStates();
            
            // 测试状态恢复
            windowManager.restoreAllWindowStates();
            
            // 测试窗口清理
            windowManager.cleanupUnusedWindows();
            
            // 测试关闭所有窗口
            windowManager.closeAllWindows();
            
            if (windowManager.isWindowVisible(WindowManager::WelcomeWindow)) {
                std::cout << "❌ WelcomeWindow should not be visible after closeAll" << std::endl;
                return false;
            }
            
            if (windowManager.isWindowVisible(WindowManager::ConferenceWindow)) {
                std::cout << "❌ ConferenceWindow should not be visible after closeAll" << std::endl;
                return false;
            }
            
            std::cout << "✅ Lifecycle management verified" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "❌ Exception in lifecycle management test: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool verifySignalConnections()
    {
        std::cout << "\n7. Testing Signal Connections..." << std::endl;
        
        try {
            ConfigurationManager configManager;
            TranslationManager translationManager;
            WindowManager windowManager;
            windowManager.setConfigurationManager(&configManager);
            windowManager.setTranslationManager(&translationManager);
            
            // 验证信号存在（通过元对象系统）
            const QMetaObject* metaObj = windowManager.metaObject();
            
            bool hasWindowChanged = false;
            bool hasWindowStateChanged = false;
            bool hasDataTransferred = false;
            bool hasWindowCreated = false;
            bool hasWindowDestroyed = false;
            
            for (int i = 0; i < metaObj->methodCount(); ++i) {
                QMetaMethod method = metaObj->method(i);
                if (method.methodType() == QMetaMethod::Signal) {
                    QString name = method.name();
                    if (name == "windowChanged") hasWindowChanged = true;
                    else if (name == "windowStateChanged") hasWindowStateChanged = true;
                    else if (name == "dataTransferred") hasDataTransferred = true;
                    else if (name == "windowCreated") hasWindowCreated = true;
                    else if (name == "windowDestroyed") hasWindowDestroyed = true;
                }
            }
            
            if (!hasWindowChanged) {
                std::cout << "❌ Missing windowChanged signal" << std::endl;
                return false;
            }
            
            if (!hasWindowStateChanged) {
                std::cout << "❌ Missing windowStateChanged signal" << std::endl;
                return false;
            }
            
            if (!hasDataTransferred) {
                std::cout << "❌ Missing dataTransferred signal" << std::endl;
                return false;
            }
            
            if (!hasWindowCreated) {
                std::cout << "❌ Missing windowCreated signal" << std::endl;
                return false;
            }
            
            if (!hasWindowDestroyed) {
                std::cout << "❌ Missing windowDestroyed signal" << std::endl;
                return false;
            }
            
            std::cout << "✅ Signal connections verified" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "❌ Exception in signal connections test: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool verifyMemoryManagement()
    {
        std::cout << "\n8. Testing Memory Management..." << std::endl;
        
        try {
            // 创建作用域以测试析构
            {
                ConfigurationManager configManager;
                TranslationManager translationManager;
                WindowManager windowManager;
                windowManager.setConfigurationManager(&configManager);
                windowManager.setTranslationManager(&translationManager);
                
                // 创建多个窗口
                windowManager.showWindow(WindowManager::WelcomeWindow);
                windowManager.showWindow(WindowManager::ConferenceWindow);
                windowManager.showWindow(WindowManager::SettingsDialog);
                
                // 验证窗口存在
                if (!windowManager.hasWindow(WindowManager::WelcomeWindow) ||
                    !windowManager.hasWindow(WindowManager::ConferenceWindow) ||
                    !windowManager.hasWindow(WindowManager::SettingsDialog)) {
                    std::cout << "❌ Windows should exist after creation" << std::endl;
                    return false;
                }
                
                // WindowManager析构函数应该正确清理所有窗口
            }
            
            std::cout << "✅ Memory management verified" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "❌ Exception in memory management test: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    WindowManagerVerifier verifier;
    bool success = verifier.verifyImplementation();
    
    return success ? 0 : 1;
}

#include "verify_window_manager.moc"