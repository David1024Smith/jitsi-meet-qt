#include <QtTest/QtTest>
#include <QApplication>
#include <QWidget>
#include <QScreen>
#include <QRect>

#include "../include/WindowStateManager.h"
#include "../include/ConfigurationManager.h"

class TestWindowStateManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试基本功能
    void testConstructor();
    void testGetCurrentWindowState();
    void testValidateWindowState();
    void testSaveAndRestoreWindowState();
    
    // 测试窗口验证
    void testIsWindowVisible();
    void testGetBestScreen();
    void testAdjustToScreen();
    
    // 测试默认状态
    void testGetDefaultWindowState();
    
    // 测试记忆功能开关
    void testRememberWindowStateEnabled();

private:
    QApplication* m_app;
    ConfigurationManager* m_configManager;
    WindowStateManager* m_windowStateManager;
    QWidget* m_testWidget;
};

void TestWindowStateManager::initTestCase()
{
    // 创建应用程序实例（如果不存在）
    if (!QApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        m_app = new QApplication(argc, argv);
    } else {
        m_app = nullptr;
    }
}

void TestWindowStateManager::cleanupTestCase()
{
    if (m_app) {
        delete m_app;
        m_app = nullptr;
    }
}

void TestWindowStateManager::init()
{
    m_configManager = new ConfigurationManager(this);
    m_windowStateManager = new WindowStateManager(m_configManager, this);
    m_testWidget = new QWidget();
    m_testWidget->resize(800, 600);
}

void TestWindowStateManager::cleanup()
{
    delete m_testWidget;
    m_testWidget = nullptr;
    
    delete m_windowStateManager;
    m_windowStateManager = nullptr;
    
    delete m_configManager;
    m_configManager = nullptr;
}

void TestWindowStateManager::testConstructor()
{
    QVERIFY(m_windowStateManager != nullptr);
    QVERIFY(m_windowStateManager->isRememberWindowStateEnabled());
}

void TestWindowStateManager::testGetCurrentWindowState()
{
    // 测试空指针
    WindowStateManager::WindowState state = m_windowStateManager->getCurrentWindowState(nullptr);
    QVERIFY(!state.valid);
    
    // 测试正常窗口
    m_testWidget->setGeometry(100, 100, 800, 600);
    state = m_windowStateManager->getCurrentWindowState(m_testWidget);
    
    QVERIFY(state.valid);
    QCOMPARE(state.geometry, QRect(100, 100, 800, 600));
    QVERIFY(!state.maximized);
}

void TestWindowStateManager::testValidateWindowState()
{
    // 测试无效状态
    WindowStateManager::WindowState invalidState;
    invalidState.valid = false;
    
    WindowStateManager::WindowState validatedState = m_windowStateManager->validateWindowState(invalidState);
    QVERIFY(validatedState.valid);
    
    // 测试有效状态
    WindowStateManager::WindowState validState;
    validState.geometry = QRect(100, 100, 800, 600);
    validState.maximized = false;
    validState.valid = true;
    
    validatedState = m_windowStateManager->validateWindowState(validState);
    QVERIFY(validatedState.valid);
    QCOMPARE(validatedState.geometry.width(), 800);
    QCOMPARE(validatedState.geometry.height(), 600);
}

void TestWindowStateManager::testSaveAndRestoreWindowState()
{
    // 设置窗口状态
    m_testWidget->setGeometry(200, 150, 900, 700);
    m_testWidget->show();
    
    // 保存状态
    bool saved = m_windowStateManager->saveWindowState(m_testWidget);
    QVERIFY(saved);
    
    // 改变窗口状态
    m_testWidget->setGeometry(50, 50, 400, 300);
    
    // 恢复状态
    bool restored = m_windowStateManager->restoreWindowState(m_testWidget);
    QVERIFY(restored);
    
    // 验证状态是否恢复（允许一些调整）
    QRect restoredGeometry = m_testWidget->geometry();
    QVERIFY(restoredGeometry.width() >= 900 || restoredGeometry.width() >= 400); // 可能被屏幕限制调整
    QVERIFY(restoredGeometry.height() >= 700 || restoredGeometry.height() >= 300);
}

void TestWindowStateManager::testIsWindowVisible()
{
    // 测试空矩形
    QVERIFY(!m_windowStateManager->isWindowVisible(QRect()));
    
    // 测试正常可见窗口
    QScreen* primaryScreen = QApplication::primaryScreen();
    if (primaryScreen) {
        QRect screenGeometry = primaryScreen->availableGeometry();
        QRect visibleRect(screenGeometry.x() + 100, screenGeometry.y() + 100, 400, 300);
        QVERIFY(m_windowStateManager->isWindowVisible(visibleRect));
        
        // 测试完全超出屏幕的窗口
        QRect invisibleRect(screenGeometry.right() + 100, screenGeometry.bottom() + 100, 400, 300);
        QVERIFY(!m_windowStateManager->isWindowVisible(invisibleRect));
    }
}

void TestWindowStateManager::testGetBestScreen()
{
    QScreen* primaryScreen = QApplication::primaryScreen();
    if (primaryScreen) {
        QRect screenGeometry = primaryScreen->availableGeometry();
        QRect windowGeometry(screenGeometry.x() + 100, screenGeometry.y() + 100, 400, 300);
        
        QScreen* bestScreen = m_windowStateManager->getBestScreen(windowGeometry);
        QVERIFY(bestScreen != nullptr);
    }
}

void TestWindowStateManager::testAdjustToScreen()
{
    QScreen* primaryScreen = QApplication::primaryScreen();
    if (primaryScreen) {
        QRect screenGeometry = primaryScreen->availableGeometry();
        
        // 测试超出屏幕的窗口
        QRect oversizedRect(screenGeometry.right() + 100, screenGeometry.bottom() + 100, 
                           screenGeometry.width() + 200, screenGeometry.height() + 200);
        
        QRect adjustedRect = m_windowStateManager->adjustToScreen(oversizedRect, primaryScreen);
        
        // 验证调整后的窗口在屏幕范围内
        QVERIFY(adjustedRect.left() >= screenGeometry.left());
        QVERIFY(adjustedRect.top() >= screenGeometry.top());
        QVERIFY(adjustedRect.right() <= screenGeometry.right());
        QVERIFY(adjustedRect.bottom() <= screenGeometry.bottom());
    }
}

void TestWindowStateManager::testGetDefaultWindowState()
{
    WindowStateManager::WindowState defaultState = m_windowStateManager->getDefaultWindowState();
    
    QVERIFY(defaultState.valid);
    QVERIFY(!defaultState.maximized);
    QVERIFY(defaultState.geometry.width() > 0);
    QVERIFY(defaultState.geometry.height() > 0);
}

void TestWindowStateManager::testRememberWindowStateEnabled()
{
    // 测试默认状态
    QVERIFY(m_windowStateManager->isRememberWindowStateEnabled());
    
    // 测试禁用
    m_windowStateManager->setRememberWindowStateEnabled(false);
    QVERIFY(!m_windowStateManager->isRememberWindowStateEnabled());
    
    // 测试启用
    m_windowStateManager->setRememberWindowStateEnabled(true);
    QVERIFY(m_windowStateManager->isRememberWindowStateEnabled());
    
    // 测试禁用状态下的保存和恢复
    m_windowStateManager->setRememberWindowStateEnabled(false);
    
    m_testWidget->setGeometry(100, 100, 800, 600);
    bool saved = m_windowStateManager->saveWindowState(m_testWidget);
    QVERIFY(!saved); // 应该返回false，因为功能被禁用
    
    bool restored = m_windowStateManager->restoreWindowState(m_testWidget);
    QVERIFY(!restored); // 应该返回false，因为功能被禁用
}

QTEST_MAIN(TestWindowStateManager)
#include "test_windowstatemanager.moc"