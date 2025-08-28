#include <QCoreApplication>
#include <QTest>
#include <QSignalSpy>
#include <QDebug>

#include "include/ScreenShareManager.h"
#include "include/WebRTCEngine.h"

class ScreenShareManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        m_screenShareManager = new ScreenShareManager(this);
        m_webrtcEngine = new WebRTCEngine(this);
        m_screenShareManager->setWebRTCEngine(m_webrtcEngine);
    }
    
    void cleanupTestCase()
    {
        delete m_screenShareManager;
        delete m_webrtcEngine;
    }
    
    void testInitialization()
    {
        QVERIFY(m_screenShareManager != nullptr);
        QVERIFY(m_webrtcEngine != nullptr);
        QVERIFY(m_screenShareManager->webRTCEngine() == m_webrtcEngine);
        
        // 初始状态应该是未共享
        QVERIFY(!m_screenShareManager->isScreenSharing());
        QVERIFY(!m_screenShareManager->isWindowSharing());
    }
    
    void testScreenEnumeration()
    {
        auto screens = m_screenShareManager->availableScreens();
        QVERIFY(screens.size() > 0); // 至少应该有一个屏幕
        
        // 检查主屏幕
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
        QVERIFY(hasPrimaryScreen); // 应该有一个主屏幕
        
        qDebug() << "Found" << screens.size() << "screens";
    }
    
    void testWindowEnumeration()
    {
        auto windows = m_screenShareManager->availableWindows();
        // 窗口数量可能为0（在测试环境中）
        
        for (const auto& window : windows) {
            QVERIFY(window.windowId > 0);
            QVERIFY(!window.title.isEmpty() || !window.processName.isEmpty());
            QVERIFY(window.geometry.width() >= 0);
            QVERIFY(window.geometry.height() >= 0);
        }
        
        qDebug() << "Found" << windows.size() << "windows";
    }
    
    void testQualitySettings()
    {
        // 测试默认质量设置
        auto defaultQuality = m_screenShareManager->shareQuality();
        QVERIFY(defaultQuality.resolution.width() > 0);
        QVERIFY(defaultQuality.resolution.height() > 0);
        QVERIFY(defaultQuality.frameRate > 0);
        QVERIFY(defaultQuality.bitrate > 0);
        
        // 测试自定义质量设置
        ScreenShareManager::ShareQuality customQuality;
        customQuality.resolution = QSize(1280, 720);
        customQuality.frameRate = 10;
        customQuality.bitrate = 1000000;
        customQuality.adaptiveQuality = false;
        
        m_screenShareManager->setShareQuality(customQuality);
        auto updatedQuality = m_screenShareManager->shareQuality();
        
        QCOMPARE(updatedQuality.resolution, QSize(1280, 720));
        QCOMPARE(updatedQuality.frameRate, 10);
        QCOMPARE(updatedQuality.bitrate, 1000000);
        QCOMPARE(updatedQuality.adaptiveQuality, false);
        
        qDebug() << "Quality settings test passed";
    }
    
    void testScreenShareLifecycle()
    {
        auto screens = m_screenShareManager->availableScreens();
        if (screens.isEmpty()) {
            QSKIP("No screens available for testing");
        }
        
        // 设置信号监听
        QSignalSpy startedSpy(m_screenShareManager, &ScreenShareManager::screenShareStarted);
        QSignalSpy stoppedSpy(m_screenShareManager, &ScreenShareManager::screenShareStopped);
        
        // 开始屏幕共享
        int screenId = screens.first().screenId;
        bool started = m_screenShareManager->startScreenShare(screenId);
        QVERIFY(started);
        
        // 验证状态
        QVERIFY(m_screenShareManager->isScreenSharing());
        QVERIFY(!m_screenShareManager->isWindowSharing());
        
        // 验证信号
        QCOMPARE(startedSpy.count(), 1);
        
        // 验证当前屏幕信息
        auto currentScreen = m_screenShareManager->currentScreen();
        QCOMPARE(currentScreen.screenId, screenId);
        
        // 停止屏幕共享
        m_screenShareManager->stopScreenShare();
        
        // 验证状态
        QVERIFY(!m_screenShareManager->isScreenSharing());
        QVERIFY(!m_screenShareManager->isWindowSharing());
        
        // 验证信号
        QCOMPARE(stoppedSpy.count(), 1);
        
        qDebug() << "Screen share lifecycle test passed";
    }
    
    void testRemoteScreenShare()
    {
        QString participantId = "test-participant-123";
        
        // 设置信号监听
        QSignalSpy receivedSpy(m_screenShareManager, &ScreenShareManager::remoteScreenShareReceived);
        QSignalSpy removedSpy(m_screenShareManager, &ScreenShareManager::remoteScreenShareRemoved);
        
        // 添加远程屏幕共享
        QVideoWidget* widget = new QVideoWidget();
        m_screenShareManager->addRemoteScreenShare(participantId, widget);
        
        // 验证
        QVERIFY(m_screenShareManager->remoteScreenShareWidget(participantId) == widget);
        QVERIFY(m_screenShareManager->remoteScreenShareParticipants().contains(participantId));
        QCOMPARE(receivedSpy.count(), 1);
        
        // 移除远程屏幕共享
        m_screenShareManager->removeRemoteScreenShare(participantId);
        
        // 验证
        QVERIFY(m_screenShareManager->remoteScreenShareWidget(participantId) == nullptr);
        QVERIFY(!m_screenShareManager->remoteScreenShareParticipants().contains(participantId));
        QCOMPARE(removedSpy.count(), 1);
        
        qDebug() << "Remote screen share test passed";
    }
    
    void testErrorHandling()
    {
        // 测试无效屏幕ID
        QSignalSpy errorSpy(m_screenShareManager, &ScreenShareManager::screenCaptureError);
        
        bool result = m_screenShareManager->startScreenShare(999); // 无效ID
        QVERIFY(!result);
        QCOMPARE(errorSpy.count(), 1);
        
        // 测试重复启动
        auto screens = m_screenShareManager->availableScreens();
        if (!screens.isEmpty()) {
            int screenId = screens.first().screenId;
            m_screenShareManager->startScreenShare(screenId);
            
            // 尝试再次启动应该失败
            bool secondStart = m_screenShareManager->startScreenShare(screenId);
            QVERIFY(!secondStart);
            
            m_screenShareManager->stopScreenShare();
        }
        
        qDebug() << "Error handling test passed";
    }

private:
    ScreenShareManager* m_screenShareManager = nullptr;
    WebRTCEngine* m_webrtcEngine = nullptr;
};

QTEST_MAIN(ScreenShareManagerTest)
#include "test_screen_share_unit.moc"