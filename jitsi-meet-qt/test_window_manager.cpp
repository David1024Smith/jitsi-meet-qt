#include <QApplication>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include <QDebug>

#include "WindowManager.h"
#include "ConfigurationManager.h"
#include "TranslationManager.h"

class TestWindowManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // 基本功能测试
    void testWindowCreation();
    void testWindowSwitching();
    void testWindowDataTransfer();
    void testWindowStateManagement();
    void testWindowLifecycleManagement();
    void testWindowStateRestoration();
    
    // 信号测试
    void testWindowSignals();
    void testDataTransferSignals();
    
    // 清理测试
    void testWindowCleanup();
    void testMemoryManagement();

private:
    QApplication* m_app;
    WindowManager* m_windowManager;
    ConfigurationManager* m_configManager;
    TranslationManager* m_translationManager;
};

void TestWindowManager::initTestCase()
{
    // 创建应用程序实例
    int argc = 1;
    char* argv[] = {"test"};
    m_app = new QApplication(argc, argv);
    
    qDebug() << "TestWindowManager: Test case initialized";
}

void TestWindowManager::cleanupTestCase()
{
    delete m_app;
    qDebug() << "TestWindowManager: Test case cleaned up";
}

void TestWindowManager::init()
{
    // 创建配置管理器
    m_configManager = new ConfigurationManager(this);
    
    // 创建翻译管理器
    m_translationManager = new TranslationManager(this);
    
    // 创建窗口管理器
    m_windowManager = new WindowManager(this);
    m_windowManager->setConfigurationManager(m_configManager);
    m_windowManager->setTranslationManager(m_translationManager);
    
    qDebug() << "TestWindowManager: Test initialized";
}

void TestWindowManager::cleanup()
{
    delete m_windowManager;
    delete m_translationManager;
    delete m_configManager;
    
    m_windowManager = nullptr;
    m_translationManager = nullptr;
    m_configManager = nullptr;
    
    qDebug() << "TestWindowManager: Test cleaned up";
}

void TestWindowManager::testWindowCreation()
{
    qDebug() << "Testing window creation...";
    
    // 测试初始状态
    QCOMPARE(m_windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    QVERIFY(!m_windowManager->hasWindow(WindowManager::WelcomeWindow));
    QVERIFY(!m_windowManager->hasWindow(WindowManager::ConferenceWindow));
    QVERIFY(!m_windowManager->hasWindow(WindowManager::SettingsDialog));
    
    // 测试欢迎窗口创建
    QSignalSpy createdSpy(m_windowManager, &WindowManager::windowCreated);
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    
    QVERIFY(m_windowManager->hasWindow(WindowManager::WelcomeWindow));
    QVERIFY(m_windowManager->isWindowVisible(WindowManager::WelcomeWindow));
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.at(0).at(0).value<WindowManager::WindowType>(), 
             WindowManager::WelcomeWindow);
    
    // 测试会议窗口创建
    m_windowManager->showWindow(WindowManager::ConferenceWindow);
    
    QVERIFY(m_windowManager->hasWindow(WindowManager::ConferenceWindow));
    QVERIFY(m_windowManager->isWindowVisible(WindowManager::ConferenceWindow));
    QCOMPARE(m_windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    QCOMPARE(createdSpy.count(), 2);
    
    // 测试设置对话框创建
    m_windowManager->showWindow(WindowManager::SettingsDialog);
    
    QVERIFY(m_windowManager->hasWindow(WindowManager::SettingsDialog));
    QVERIFY(m_windowManager->isWindowVisible(WindowManager::SettingsDialog));
    QCOMPARE(createdSpy.count(), 3);
    
    qDebug() << "Window creation test passed";
}

void TestWindowManager::testWindowSwitching()
{
    qDebug() << "Testing window switching...";
    
    // 创建窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    m_windowManager->showWindow(WindowManager::ConferenceWindow);
    
    QSignalSpy changedSpy(m_windowManager, &WindowManager::windowChanged);
    
    // 切换到欢迎窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    
    QCOMPARE(m_windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    QVERIFY(m_windowManager->isWindowVisible(WindowManager::WelcomeWindow));
    QVERIFY(!m_windowManager->isWindowVisible(WindowManager::ConferenceWindow));
    QCOMPARE(changedSpy.count(), 1);
    
    // 切换到会议窗口
    m_windowManager->showWindow(WindowManager::ConferenceWindow);
    
    QCOMPARE(m_windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    QVERIFY(!m_windowManager->isWindowVisible(WindowManager::WelcomeWindow));
    QVERIFY(m_windowManager->isWindowVisible(WindowManager::ConferenceWindow));
    QCOMPARE(changedSpy.count(), 2);
    
    qDebug() << "Window switching test passed";
}

void TestWindowManager::testWindowDataTransfer()
{
    qDebug() << "Testing window data transfer...";
    
    QSignalSpy transferSpy(m_windowManager, &WindowManager::dataTransferred);
    
    // 测试数据传递到欢迎窗口
    QVariantMap welcomeData;
    welcomeData["url"] = "https://meet.jit.si/test-room";
    welcomeData["error"] = "Test error message";
    
    m_windowManager->showWindow(WindowManager::WelcomeWindow, welcomeData);
    QVERIFY(m_windowManager->sendDataToWindow(WindowManager::WelcomeWindow, welcomeData));
    
    // 测试数据传递到会议窗口
    QVariantMap conferenceData;
    conferenceData["url"] = "https://meet.jit.si/conference-room";
    
    m_windowManager->showWindow(WindowManager::ConferenceWindow, conferenceData);
    
    // 验证数据传递信号
    QVERIFY(transferSpy.count() > 0);
    
    // 测试向不存在的窗口发送数据
    QVERIFY(!m_windowManager->sendDataToWindow(WindowManager::SettingsDialog, QVariantMap()));
    
    qDebug() << "Window data transfer test passed";
}

void TestWindowManager::testWindowStateManagement()
{
    qDebug() << "Testing window state management...";
    
    QSignalSpy stateSpy(m_windowManager, &WindowManager::windowStateChanged);
    
    // 测试初始状态
    QCOMPARE(m_windowManager->getWindowState(WindowManager::WelcomeWindow), 
             WindowManager::WindowHidden);
    
    // 显示窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(m_windowManager->getWindowState(WindowManager::WelcomeWindow), 
             WindowManager::WindowVisible);
    QVERIFY(stateSpy.count() > 0);
    
    // 隐藏窗口
    m_windowManager->hideWindow(WindowManager::WelcomeWindow);
    QCOMPARE(m_windowManager->getWindowState(WindowManager::WelcomeWindow), 
             WindowManager::WindowHidden);
    
    // 关闭窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    m_windowManager->closeWindow(WindowManager::WelcomeWindow);
    QCOMPARE(m_windowManager->getWindowState(WindowManager::WelcomeWindow), 
             WindowManager::WindowHidden);
    
    qDebug() << "Window state management test passed";
}

void TestWindowManager::testWindowLifecycleManagement()
{
    qDebug() << "Testing window lifecycle management...";
    
    QSignalSpy createdSpy(m_windowManager, &WindowManager::windowCreated);
    QSignalSpy destroyedSpy(m_windowManager, &WindowManager::windowDestroyed);
    
    // 创建窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    m_windowManager->showWindow(WindowManager::ConferenceWindow);
    
    QCOMPARE(createdSpy.count(), 2);
    
    // 测试窗口清理
    m_windowManager->cleanupUnusedWindows();
    
    // 关闭所有窗口
    m_windowManager->closeAllWindows();
    
    QVERIFY(!m_windowManager->isWindowVisible(WindowManager::WelcomeWindow));
    QVERIFY(!m_windowManager->isWindowVisible(WindowManager::ConferenceWindow));
    
    qDebug() << "Window lifecycle management test passed";
}

void TestWindowManager::testWindowStateRestoration()
{
    qDebug() << "Testing window state restoration...";
    
    // 创建并显示窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    m_windowManager->showWindow(WindowManager::ConferenceWindow);
    
    // 保存窗口状态
    m_windowManager->saveAllWindowStates();
    
    // 隐藏窗口
    m_windowManager->hideWindow(WindowManager::WelcomeWindow);
    m_windowManager->hideWindow(WindowManager::ConferenceWindow);
    
    // 恢复窗口状态
    m_windowManager->restoreAllWindowStates();
    
    // 验证状态恢复（具体验证取决于WindowStateManager的实现）
    QVERIFY(m_windowManager->hasWindow(WindowManager::WelcomeWindow));
    QVERIFY(m_windowManager->hasWindow(WindowManager::ConferenceWindow));
    
    qDebug() << "Window state restoration test passed";
}

void TestWindowManager::testWindowSignals()
{
    qDebug() << "Testing window signals...";
    
    QSignalSpy changedSpy(m_windowManager, &WindowManager::windowChanged);
    QSignalSpy stateSpy(m_windowManager, &WindowManager::windowStateChanged);
    QSignalSpy createdSpy(m_windowManager, &WindowManager::windowCreated);
    
    // 显示窗口应该触发信号
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    
    QVERIFY(changedSpy.count() > 0);
    QVERIFY(stateSpy.count() > 0);
    QVERIFY(createdSpy.count() > 0);
    
    // 切换窗口应该触发信号
    changedSpy.clear();
    m_windowManager->showWindow(WindowManager::ConferenceWindow);
    
    QVERIFY(changedSpy.count() > 0);
    
    qDebug() << "Window signals test passed";
}

void TestWindowManager::testDataTransferSignals()
{
    qDebug() << "Testing data transfer signals...";
    
    QSignalSpy transferSpy(m_windowManager, &WindowManager::dataTransferred);
    
    // 创建欢迎窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 切换到会议窗口并传递数据
    QVariantMap data;
    data["url"] = "https://meet.jit.si/test";
    
    m_windowManager->showWindow(WindowManager::ConferenceWindow, data);
    
    // 验证数据传递信号
    QVERIFY(transferSpy.count() > 0);
    
    auto signalArgs = transferSpy.at(0);
    QCOMPARE(signalArgs.at(0).value<WindowManager::WindowType>(), 
             WindowManager::WelcomeWindow);
    QCOMPARE(signalArgs.at(1).value<WindowManager::WindowType>(), 
             WindowManager::ConferenceWindow);
    
    qDebug() << "Data transfer signals test passed";
}

void TestWindowManager::testWindowCleanup()
{
    qDebug() << "Testing window cleanup...";
    
    QSignalSpy destroyedSpy(m_windowManager, &WindowManager::windowDestroyed);
    
    // 创建窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    m_windowManager->showWindow(WindowManager::ConferenceWindow);
    
    // 切换到欢迎窗口，使会议窗口变为未使用状态
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 手动触发清理
    m_windowManager->cleanupUnusedWindows();
    
    // 验证清理结果（具体行为取决于清理策略）
    QVERIFY(m_windowManager->hasWindow(WindowManager::WelcomeWindow));
    
    qDebug() << "Window cleanup test passed";
}

void TestWindowManager::testMemoryManagement()
{
    qDebug() << "Testing memory management...";
    
    // 创建多个窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    m_windowManager->showWindow(WindowManager::ConferenceWindow);
    m_windowManager->showWindow(WindowManager::SettingsDialog);
    
    // 验证窗口存在
    QVERIFY(m_windowManager->hasWindow(WindowManager::WelcomeWindow));
    QVERIFY(m_windowManager->hasWindow(WindowManager::ConferenceWindow));
    QVERIFY(m_windowManager->hasWindow(WindowManager::SettingsDialog));
    
    // 关闭所有窗口
    m_windowManager->closeAllWindows();
    
    // 验证窗口状态
    QVERIFY(!m_windowManager->isWindowVisible(WindowManager::WelcomeWindow));
    QVERIFY(!m_windowManager->isWindowVisible(WindowManager::ConferenceWindow));
    QVERIFY(!m_windowManager->isWindowVisible(WindowManager::SettingsDialog));
    
    qDebug() << "Memory management test passed";
}

QTEST_MAIN(TestWindowManager)
#include "test_window_manager.moc"