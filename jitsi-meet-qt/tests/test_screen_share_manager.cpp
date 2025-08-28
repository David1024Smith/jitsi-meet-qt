#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QScreen>
#include "ScreenShareManager.h"

class TestScreenShareManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // 基本功能测试
    void testInitialization();
    void testScreenEnumeration();
    void testWindowEnumeration();
    
    // 屏幕共享测试
    void testScreenShareControl();
    void testWindowShareControl();
    void testQualitySettings();
    
    // 远程屏幕共享测试
    void testRemoteScreenShare();
    
    // 错误处理测试
    void testInvalidScreenShare();
    void testInvalidWindowShare();
    
    // 性能测试
    void testPerformanceAdaptation();

private:
    ScreenShareManager* m_screenShareManager;
};

void TestScreenShareManager::initTestCase()
{
    // 确保有GUI应用程序上下文
    if (!QApplication::instance()) {
        int argc = 0;
        char* argv[] = {nullptr};
        new QApplication(argc, argv);
    }
}

void TestScreenShareManager::cleanupTestCase()
{
}

void TestScreenShareManager::init()
{
    m_screenShareManager = new ScreenShareManager(this);
}

void TestScreenShareManager::cleanup()
{
    delete m_screenShareManager;
    m_screenShareManager = nullptr;
}

void TestScreenShareManager::testInitialization()
{
    // 测试初始状态
    QVERIFY(!m_screenShareManager->isScreenSharing());
    QVERIFY(!m_screenShareManager->isWindowSharing());
    QVERIFY(m_screenShareManager->localScreenShareWidget() != nullptr);
    QVERIFY(m_screenShareManager->webRTCEngine() == nullptr);
    
    // 测试默认质量设置
    ScreenShareManager::ShareQuality quality = m_screenShareManager->shareQuality();
    QCOMPARE(quality.resolution, QSize(1920, 1080));
    QCOMPARE(quality.frameRate, 15);
    QCOMPARE(quality.bitrate, 2000000);
    QVERIFY(quality.adaptiveQuality);
}

void TestScreenShareManager::testScreenEnumeration()
{
    // 测试屏幕枚举
    QList<ScreenShareManager::ScreenInfo> screens = m_screenShareManager->availableScreens();
    QVERIFY(screens.size() > 0);
    
    // 验证主屏幕存在
    bool hasPrimaryScreen = false;
    for (const auto& screen : screens) {
        QVERIFY(screen.screenId >= 0);
        QVERIFY(!screen.name.isEmpty());
        QVERIFY(screen.size.width() > 0);
        QVERIFY(screen.size.height() > 0);
        QVERIFY(screen.screen != nullptr);
        
        if (screen.isPrimary) {
            hasPrimaryScreen = true;
        }
    }
    QVERIFY(hasPrimaryScreen);
    
    // 测试屏幕列表刷新
    QSignalSpy screenListSpy(m_screenShareManager, &ScreenShareManager::screenListChanged);
    m_screenShareManager->refreshScreenList();
    QVERIFY(screenListSpy.wait(100));
    QCOMPARE(screenListSpy.count(), 1);
}

void TestScreenShareManager::testWindowEnumeration()
{
    // 测试窗口枚举
    QList<ScreenShareManager::WindowInfo> windows = m_screenShareManager->availableWindows();
    // 注意：在测试环境中可能没有可见窗口，所以不强制要求有窗口
    
    for (const auto& window : windows) {
        QVERIFY(window.windowId > 0);
        QVERIFY(!window.title.isEmpty());
        QVERIFY(window.geometry.width() > 0);
        QVERIFY(window.geometry.height() > 0);
    }
    
    // 测试窗口列表刷新
    QSignalSpy windowListSpy(m_screenShareManager, &ScreenShareManager::windowListChanged);
    m_screenShareManager->refreshWindowList();
    QVERIFY(windowListSpy.wait(100));
    QCOMPARE(windowListSpy.count(), 1);
}

void TestScreenShareManager::testScreenShareControl()
{
    QSignalSpy startSpy(m_screenShareManager, &ScreenShareManager::screenShareStarted);
    QSignalSpy stopSpy(m_screenShareManager, &ScreenShareManager::screenShareStopped);
    
    // 获取第一个可用屏幕
    QList<ScreenShareManager::ScreenInfo> screens = m_screenShareManager->availableScreens();
    QVERIFY(screens.size() > 0);
    
    int screenId = screens.first().screenId;
    
    // 测试开始屏幕共享
    QVERIFY(!m_screenShareManager->isScreenSharing());
    bool result = m_screenShareManager->startScreenShare(screenId);
    QVERIFY(result);
    QVERIFY(m_screenShareManager->isScreenSharing());
    
    // 验证信号发出
    QVERIFY(startSpy.wait(100));
    QCOMPARE(startSpy.count(), 1);
    
    // 验证当前屏幕信息
    ScreenShareManager::ScreenInfo currentScreen = m_screenShareManager->currentScreen();
    QCOMPARE(currentScreen.screenId, screenId);
    
    // 测试停止屏幕共享
    m_screenShareManager->stopScreenShare();
    QVERIFY(!m_screenShareManager->isScreenSharing());
    
    // 验证信号发出
    QVERIFY(stopSpy.wait(100));
    QCOMPARE(stopSpy.count(), 1);
}

void TestScreenShareManager::testWindowShareControl()
{
    QSignalSpy startSpy(m_screenShareManager, &ScreenShareManager::windowShareStarted);
    QSignalSpy stopSpy(m_screenShareManager, &ScreenShareManager::windowShareStopped);
    
    // 获取第一个可用窗口
    QList<ScreenShareManager::WindowInfo> windows = m_screenShareManager->availableWindows();
    if (windows.isEmpty()) {
        QSKIP("No windows available for testing");
        return;
    }
    
    qint64 windowId = windows.first().windowId;
    
    // 测试开始窗口共享
    QVERIFY(!m_screenShareManager->isWindowSharing());
    bool result = m_screenShareManager->startWindowShare(windowId);
    QVERIFY(result);
    QVERIFY(m_screenShareManager->isWindowSharing());
    
    // 验证信号发出
    QVERIFY(startSpy.wait(100));
    QCOMPARE(startSpy.count(), 1);
    
    // 验证当前窗口信息
    ScreenShareManager::WindowInfo currentWindow = m_screenShareManager->currentWindow();
    QCOMPARE(currentWindow.windowId, windowId);
    
    // 测试停止窗口共享
    m_screenShareManager->stopScreenShare();
    QVERIFY(!m_screenShareManager->isWindowSharing());
    
    // 验证信号发出
    QVERIFY(stopSpy.wait(100));
    QCOMPARE(stopSpy.count(), 1);
}

void TestScreenShareManager::testQualitySettings()
{
    // 测试质量设置
    ScreenShareManager::ShareQuality newQuality;
    newQuality.resolution = QSize(1280, 720);
    newQuality.frameRate = 30;
    newQuality.bitrate = 1500000;
    newQuality.adaptiveQuality = false;
    
    m_screenShareManager->setShareQuality(newQuality);
    
    ScreenShareManager::ShareQuality currentQuality = m_screenShareManager->shareQuality();
    QCOMPARE(currentQuality.resolution, newQuality.resolution);
    QCOMPARE(currentQuality.frameRate, newQuality.frameRate);
    QCOMPARE(currentQuality.bitrate, newQuality.bitrate);
    QCOMPARE(currentQuality.adaptiveQuality, newQuality.adaptiveQuality);
}

void TestScreenShareManager::testRemoteScreenShare()
{
    QSignalSpy receivedSpy(m_screenShareManager, &ScreenShareManager::remoteScreenShareReceived);
    QSignalSpy removedSpy(m_screenShareManager, &ScreenShareManager::remoteScreenShareRemoved);
    
    QString participantId = "test-participant-1";
    QVideoWidget* widget = new QVideoWidget();
    
    // 测试添加远程屏幕共享
    QVERIFY(m_screenShareManager->remoteScreenShareParticipants().isEmpty());
    m_screenShareManager->addRemoteScreenShare(participantId, widget);
    
    // 验证信号和状态
    QVERIFY(receivedSpy.wait(100));
    QCOMPARE(receivedSpy.count(), 1);
    QVERIFY(m_screenShareManager->remoteScreenShareParticipants().contains(participantId));
    QCOMPARE(m_screenShareManager->remoteScreenShareWidget(participantId), widget);
    
    // 测试移除远程屏幕共享
    m_screenShareManager->removeRemoteScreenShare(participantId);
    
    // 验证信号和状态
    QVERIFY(removedSpy.wait(100));
    QCOMPARE(removedSpy.count(), 1);
    QVERIFY(!m_screenShareManager->remoteScreenShareParticipants().contains(participantId));
    QVERIFY(m_screenShareManager->remoteScreenShareWidget(participantId) == nullptr);
}

void TestScreenShareManager::testInvalidScreenShare()
{
    QSignalSpy errorSpy(m_screenShareManager, &ScreenShareManager::screenCaptureError);
    
    // 测试无效屏幕ID
    bool result = m_screenShareManager->startScreenShare(999);
    QVERIFY(!result);
    QVERIFY(!m_screenShareManager->isScreenSharing());
    
    // 验证错误信号
    QVERIFY(errorSpy.wait(100));
    QCOMPARE(errorSpy.count(), 1);
}

void TestScreenShareManager::testInvalidWindowShare()
{
    QSignalSpy errorSpy(m_screenShareManager, &ScreenShareManager::windowCaptureError);
    
    // 测试无效窗口ID
    bool result = m_screenShareManager->startWindowShare(999999);
    QVERIFY(!result);
    QVERIFY(!m_screenShareManager->isWindowSharing());
    
    // 验证错误信号
    QVERIFY(errorSpy.wait(100));
    QCOMPARE(errorSpy.count(), 1);
}

void TestScreenShareManager::testPerformanceAdaptation()
{
    // 测试性能自适应功能
    ScreenShareManager::ShareQuality quality;
    quality.frameRate = 20;
    quality.adaptiveQuality = true;
    m_screenShareManager->setShareQuality(quality);
    
    // 开始屏幕共享以触发性能监控
    QList<ScreenShareManager::ScreenInfo> screens = m_screenShareManager->availableScreens();
    if (!screens.isEmpty()) {
        m_screenShareManager->startScreenShare(screens.first().screenId);
        
        // 等待一段时间让性能监控运行
        QTest::qWait(1000);
        
        // 停止共享
        m_screenShareManager->stopScreenShare();
    }
    
    // 注意：实际的性能自适应测试需要更复杂的模拟环境
}

QTEST_MAIN(TestScreenShareManager)
// MOC will be generated automatically