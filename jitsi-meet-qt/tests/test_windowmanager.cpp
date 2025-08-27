#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include "WindowManager.h"
#include "ConfigurationManager.h"

class TestWindowManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testWindowCreation();
    void testWindowSwitching();
    void testDataTransfer();
    void testWindowStateManagement();
    void testWindowCleanup();

private:
    WindowManager* m_windowManager;
    ConfigurationManager* m_configManager;
};

void TestWindowManager::initTestCase()
{
    // 创建配置管理器
    m_configManager = new ConfigurationManager(this);
    
    // 创建窗口管理器
    m_windowManager = new WindowManager(this);
    m_windowManager->setConfigurationManager(m_configManager);
}

void TestWindowManager::cleanupTestCase()
{
    delete m_windowManager;
    delete m_configManager;
}

void TestWindowManager::init()
{
    // 每个测试前的初始化
}

void TestWindowManager::cleanup()
{
    // 每个测试后的清理
    m_windowManager->closeAllWindows();
}

void TestWindowManager::testWindowCreation()
{
    // 测试窗口创建
    QSignalSpy createdSpy(m_windowManager, &WindowManager::windowCreated);
    
    // 显示欢迎窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 验证窗口创建信号
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.at(0).at(0).toInt(), static_cast<int>(WindowManager::WelcomeWindow));
    
    // 验证窗口存在
    QVERIFY(m_windowManager->hasWindow(WindowManager::WelcomeWindow));
    QVERIFY(m_windowManager->isWindowVisible(WindowManager::WelcomeWindow));
    QCOMPARE(m_windowManager->currentWindowType(), WindowManager::WelcomeWindow);
}

void TestWindowManager::testWindowSwitching()
{
    // 测试窗口切换
    QSignalSpy changedSpy(m_windowManager, &WindowManager::windowChanged);
    
    // 显示欢迎窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(changedSpy.count(), 1);
    
    // 切换到会议窗口
    QVariantMap data;
    data["url"] = "https://meet.jit.si/test-room";
    m_windowManager->showWindow(WindowManager::ConferenceWindow, data);
    
    // 验证窗口切换
    QCOMPARE(changedSpy.count(), 2);
    QCOMPARE(m_windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    
    // 验证欢迎窗口被隐藏
    QVERIFY(!m_windowManager->isWindowVisible(WindowManager::WelcomeWindow));
    QVERIFY(m_windowManager->isWindowVisible(WindowManager::ConferenceWindow));
}

void TestWindowManager::testDataTransfer()
{
    // 测试数据传递
    QSignalSpy transferSpy(m_windowManager, &WindowManager::dataTransferred);
    
    // 显示欢迎窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 传递数据到会议窗口
    QVariantMap data;
    data["url"] = "https://meet.jit.si/test-room";
    data["serverUrl"] = "https://custom.jitsi.server";
    
    m_windowManager->showWindow(WindowManager::ConferenceWindow, data);
    
    // 验证数据传递信号
    QCOMPARE(transferSpy.count(), 1);
    
    // 测试直接数据发送
    QVariantMap newData;
    newData["error"] = "Test error message";
    
    bool result = m_windowManager->sendDataToWindow(WindowManager::WelcomeWindow, newData);
    QVERIFY(result);
}

void TestWindowManager::testWindowStateManagement()
{
    // 测试窗口状态管理
    QSignalSpy stateSpy(m_windowManager, &WindowManager::windowStateChanged);
    
    // 显示窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 验证状态改变
    QVERIFY(stateSpy.count() > 0);
    QCOMPARE(m_windowManager->getWindowState(WindowManager::WelcomeWindow), 
             WindowManager::WindowVisible);
    
    // 隐藏窗口
    m_windowManager->hideWindow(WindowManager::WelcomeWindow);
    
    // 验证状态改变
    QCOMPARE(m_windowManager->getWindowState(WindowManager::WelcomeWindow), 
             WindowManager::WindowHidden);
}

void TestWindowManager::testWindowCleanup()
{
    // 测试窗口清理
    QSignalSpy destroyedSpy(m_windowManager, &WindowManager::windowDestroyed);
    
    // 创建窗口
    m_windowManager->showWindow(WindowManager::WelcomeWindow);
    m_windowManager->showWindow(WindowManager::ConferenceWindow);
    
    // 验证窗口存在
    QVERIFY(m_windowManager->hasWindow(WindowManager::WelcomeWindow));
    QVERIFY(m_windowManager->hasWindow(WindowManager::ConferenceWindow));
    
    // 执行清理
    m_windowManager->cleanupUnusedWindows();
    
    // 当前窗口不应该被清理
    QVERIFY(m_windowManager->hasWindow(WindowManager::ConferenceWindow));
}

QTEST_MAIN(TestWindowManager)
#include "test_windowmanager.moc"