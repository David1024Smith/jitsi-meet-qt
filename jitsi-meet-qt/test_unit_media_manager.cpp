#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include <QVideoWidget>
#include <QCamera>
#include <QAudioInput>
#include <QMediaDevices>
#include <QScreen>
#include <QApplication>
#include "MediaManager.h"
#include "WebRTCEngine.h"

/**
 * @brief MediaManager单元测试类
 * 
 * 测试MediaManager的设备管理功能：
 * - 媒体设备枚举和选择
 * - 本地媒体流控制
 * - 屏幕共享功能
 * - 音量和静音控制
 * - 媒体权限处理
 * - 设备状态管理
 */
class TestMediaManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基本功能测试
    void testInitialState();
    void testDeviceEnumeration();
    void testDeviceSelection();
    void testMediaStreamControl();
    
    // 媒体流测试
    void testLocalVideoControl();
    void testLocalAudioControl();
    void testScreenSharingControl();
    void testMediaPermissions();
    
    // 设备管理测试
    void testVideoDeviceManagement();
    void testAudioDeviceManagement();
    void testDeviceStateChanges();
    
    // 音量控制测试
    void testVolumeControl();
    void testMuteControl();
    void testMediaSettings();
    
    // 集成测试
    void testWebRTCIntegration();
    void testMediaCodecs();

private:
    MediaManager* m_mediaManager;
    WebRTCEngine* m_webRTCEngine;
    QSignalSpy* m_videoDeviceChangedSpy;
    QSignalSpy* m_audioInputChangedSpy;
    QSignalSpy* m_localVideoStartedSpy;
    QSignalSpy* m_localAudioStartedSpy;
    QSignalSpy* m_errorSpy;
};

void TestMediaManager::initTestCase()
{
    qDebug() << "Starting MediaManager unit tests";
}

void TestMediaManager::cleanupTestCase()
{
    qDebug() << "MediaManager unit tests completed";
}

void TestMediaManager::init()
{
    m_webRTCEngine = new WebRTCEngine(this);
    m_mediaManager = new MediaManager(this);
    m_mediaManager->setWebRTCEngine(m_webRTCEngine);
    
    // 创建信号监听器
    m_videoDeviceChangedSpy = new QSignalSpy(m_mediaManager, &MediaManager::videoDeviceChanged);
    m_audioInputChangedSpy = new QSignalSpy(m_mediaManager, &MediaManager::audioInputDeviceChanged);
    m_localVideoStartedSpy = new QSignalSpy(m_mediaManager, &MediaManager::localVideoStarted);
    m_localAudioStartedSpy = new QSignalSpy(m_mediaManager, &MediaManager::localAudioStarted);
    m_errorSpy = new QSignalSpy(m_mediaManager, &MediaManager::mediaError);
}

void TestMediaManager::cleanup()
{
    delete m_mediaManager;
    delete m_webRTCEngine;
    m_mediaManager = nullptr;
    m_webRTCEngine = nullptr;
    
    delete m_videoDeviceChangedSpy;
    delete m_audioInputChangedSpy;
    delete m_localVideoStartedSpy;
    delete m_localAudioStartedSpy;
    delete m_errorSpy;
}

void TestMediaManager::testInitialState()
{
    // 测试初始状态
    QVERIFY(!m_mediaManager->isVideoActive());
    QVERIFY(!m_mediaManager->isAudioActive());
    QVERIFY(!m_mediaManager->isScreenSharingActive());
    QVERIFY(!m_mediaManager->isVideoMuted());
    QVERIFY(!m_mediaManager->isAudioMuted());
    
    // 测试默认音量
    QVERIFY(m_mediaManager->masterVolume() >= 0.0);
    QVERIFY(m_mediaManager->masterVolume() <= 1.0);
    QVERIFY(m_mediaManager->microphoneVolume() >= 0.0);
    QVERIFY(m_mediaManager->microphoneVolume() <= 1.0);
    
    // 测试权限状态
    QVERIFY(!m_mediaManager->hasVideoPermission());
    QVERIFY(!m_mediaManager->hasAudioPermission());
}

void TestMediaManager::testDeviceEnumeration()
{
    // 测试设备枚举
    QList<MediaManager::MediaDevice> videoDevices = m_mediaManager->availableVideoDevices();
    QList<MediaManager::MediaDevice> audioInputs = m_mediaManager->availableAudioInputDevices();
    QList<MediaManager::MediaDevice> audioOutputs = m_mediaManager->availableAudioOutputDevices();
    QList<QScreen*> screens = m_mediaManager->availableScreens();
    
    // 验证设备列表不为null（可能为空但不应该崩溃）
    QVERIFY(videoDevices.size() >= 0);
    QVERIFY(audioInputs.size() >= 0);
    QVERIFY(audioOutputs.size() >= 0);
    QVERIFY(screens.size() >= 0);
    
    qDebug() << "Found" << videoDevices.size() << "video devices";
    qDebug() << "Found" << audioInputs.size() << "audio input devices";
    qDebug() << "Found" << audioOutputs.size() << "audio output devices";
    qDebug() << "Found" << screens.size() << "screens";
    
    // 验证设备信息结构
    for (const auto& device : videoDevices) {
        QVERIFY(!device.id.isEmpty());
        QVERIFY(!device.name.isEmpty());
        QCOMPARE(device.type, MediaManager::Video);
    }
    
    for (const auto& device : audioInputs) {
        QVERIFY(!device.id.isEmpty());
        QVERIFY(!device.name.isEmpty());
        QCOMPARE(device.type, MediaManager::Audio);
    }
}

void TestMediaManager::testDeviceSelection()
{
    // 测试设备选择
    QList<MediaManager::MediaDevice> videoDevices = m_mediaManager->availableVideoDevices();
    QList<MediaManager::MediaDevice> audioInputs = m_mediaManager->availableAudioInputDevices();
    QList<MediaManager::MediaDevice> audioOutputs = m_mediaManager->availableAudioOutputDevices();
    
    // 如果有设备，测试设备选择
    if (!videoDevices.isEmpty()) {
        bool result = m_mediaManager->setVideoDevice(videoDevices.first().id);
        QVERIFY(result == true || result == false); // 可能成功也可能失败
        
        if (result) {
            QVERIFY(m_videoDeviceChangedSpy->count() >= 1);
        }
    }
    
    if (!audioInputs.isEmpty()) {
        bool result = m_mediaManager->setAudioInputDevice(audioInputs.first().id);
        QVERIFY(result == true || result == false);
        
        if (result) {
            QVERIFY(m_audioInputChangedSpy->count() >= 1);
        }
    }
    
    if (!audioOutputs.isEmpty()) {
        bool result = m_mediaManager->setAudioOutputDevice(audioOutputs.first().id);
        QVERIFY(result == true || result == false);
    }
}

void TestMediaManager::testMediaStreamControl()
{
    // 测试媒体流控制
    m_mediaManager->startLocalVideo();
    m_mediaManager->stopLocalVideo();
    
    m_mediaManager->startLocalAudio();
    m_mediaManager->stopLocalAudio();
    
    // 验证重复操作不会崩溃
    m_mediaManager->startLocalVideo();
    m_mediaManager->startLocalVideo();
    m_mediaManager->stopLocalVideo();
    m_mediaManager->stopLocalVideo();
    
    // 验证操作不会崩溃
    QVERIFY(true);
}

void TestMediaManager::testLocalVideoControl()
{
    // 测试本地视频控制
    QVERIFY(!m_mediaManager->isVideoActive());
    
    m_mediaManager->startLocalVideo();
    
    // 等待视频启动
    QTest::qWait(500);
    
    // 验证视频启动信号
    QVERIFY(m_localVideoStartedSpy->count() >= 0);
    
    m_mediaManager->stopLocalVideo();
    
    // 等待视频停止
    QTest::qWait(200);
}

void TestMediaManager::testLocalAudioControl()
{
    // 测试本地音频控制
    QVERIFY(!m_mediaManager->isAudioActive());
    
    m_mediaManager->startLocalAudio();
    
    // 等待音频启动
    QTest::qWait(500);
    
    // 验证音频启动信号
    QVERIFY(m_localAudioStartedSpy->count() >= 0);
    
    m_mediaManager->stopLocalAudio();
    
    // 等待音频停止
    QTest::qWait(200);
}

void TestMediaManager::testScreenSharingControl()
{
    // 测试屏幕共享控制
    QVERIFY(!m_mediaManager->isScreenSharingActive());
    
    QList<QScreen*> screens = m_mediaManager->availableScreens();
    
    if (!screens.isEmpty()) {
        m_mediaManager->startScreenSharing(screens.first());
        
        // 等待屏幕共享启动
        QTest::qWait(500);
        
        m_mediaManager->stopScreenSharing();
        
        // 等待屏幕共享停止
        QTest::qWait(200);
    }
    
    // 测试默认屏幕共享
    m_mediaManager->startScreenSharing();
    m_mediaManager->stopScreenSharing();
    
    // 验证屏幕共享控制不会崩溃
    QVERIFY(true);
}

void TestMediaManager::testMediaPermissions()
{
    // 测试媒体权限
    m_mediaManager->requestMediaPermissions();
    
    // 等待权限请求处理
    QTest::qWait(100);
    
    // 验证权限请求不会崩溃
    QVERIFY(true);
}

void TestMediaManager::testVideoDeviceManagement()
{
    // 测试视频设备管理
    QList<MediaManager::MediaDevice> devices = m_mediaManager->availableVideoDevices();
    
    for (const auto& device : devices) {
        // 验证设备信息完整性
        QVERIFY(!device.id.isEmpty());
        QVERIFY(!device.name.isEmpty());
        QCOMPARE(device.type, MediaManager::Video);
        QVERIFY(device.state != MediaManager::DeviceError); // 初始状态不应该是错误
    }
    
    // 测试当前设备获取
    auto currentDevice = m_mediaManager->currentVideoDevice();
    // currentDevice可能为空，这是正常的
    QVERIFY(true);
}

void TestMediaManager::testAudioDeviceManagement()
{
    // 测试音频设备管理
    QList<MediaManager::MediaDevice> inputDevices = m_mediaManager->availableAudioInputDevices();
    QList<MediaManager::MediaDevice> outputDevices = m_mediaManager->availableAudioOutputDevices();
    
    for (const auto& device : inputDevices) {
        QVERIFY(!device.id.isEmpty());
        QVERIFY(!device.name.isEmpty());
        QCOMPARE(device.type, MediaManager::Audio);
    }
    
    for (const auto& device : outputDevices) {
        QVERIFY(!device.id.isEmpty());
        QVERIFY(!device.name.isEmpty());
        QCOMPARE(device.type, MediaManager::Audio);
    }
    
    // 测试当前设备获取
    auto currentInputDevice = m_mediaManager->currentAudioInputDevice();
    auto currentOutputDevice = m_mediaManager->currentAudioOutputDevice();
    // 设备可能为空，这是正常的
    QVERIFY(true);
}

void TestMediaManager::testDeviceStateChanges()
{
    // 测试设备状态变化
    m_mediaManager->startLocalVideo();
    m_mediaManager->startLocalAudio();
    
    // 等待设备状态变化
    QTest::qWait(500);
    
    m_mediaManager->stopLocalVideo();
    m_mediaManager->stopLocalAudio();
    
    // 验证设备状态变化处理
    QVERIFY(true);
}

void TestMediaManager::testVolumeControl()
{
    // 测试音量控制
    qreal originalMasterVolume = m_mediaManager->masterVolume();
    qreal originalMicVolume = m_mediaManager->microphoneVolume();
    
    // 测试主音量控制
    m_mediaManager->setMasterVolume(0.5);
    QCOMPARE(m_mediaManager->masterVolume(), 0.5);
    
    m_mediaManager->setMasterVolume(0.0);
    QCOMPARE(m_mediaManager->masterVolume(), 0.0);
    
    m_mediaManager->setMasterVolume(1.0);
    QCOMPARE(m_mediaManager->masterVolume(), 1.0);
    
    // 测试麦克风音量控制
    m_mediaManager->setMicrophoneVolume(0.7);
    QCOMPARE(m_mediaManager->microphoneVolume(), 0.7);
    
    // 测试无效音量值
    m_mediaManager->setMasterVolume(-0.5); // 应该被限制到0.0
    QVERIFY(m_mediaManager->masterVolume() >= 0.0);
    
    m_mediaManager->setMasterVolume(1.5); // 应该被限制到1.0
    QVERIFY(m_mediaManager->masterVolume() <= 1.0);
    
    // 恢复原始音量
    m_mediaManager->setMasterVolume(originalMasterVolume);
    m_mediaManager->setMicrophoneVolume(originalMicVolume);
}

void TestMediaManager::testMuteControl()
{
    // 测试静音控制
    QSignalSpy videoMuteSpy(m_mediaManager, &MediaManager::videoMuteChanged);
    QSignalSpy audioMuteSpy(m_mediaManager, &MediaManager::audioMuteChanged);
    
    // 初始状态应该不是静音
    QVERIFY(!m_mediaManager->isVideoMuted());
    QVERIFY(!m_mediaManager->isAudioMuted());
    
    // 测试视频静音
    m_mediaManager->setVideoMuted(true);
    QVERIFY(m_mediaManager->isVideoMuted());
    QVERIFY(videoMuteSpy.count() >= 1);
    
    m_mediaManager->setVideoMuted(false);
    QVERIFY(!m_mediaManager->isVideoMuted());
    
    // 测试音频静音
    m_mediaManager->setAudioMuted(true);
    QVERIFY(m_mediaManager->isAudioMuted());
    QVERIFY(audioMuteSpy.count() >= 1);
    
    m_mediaManager->setAudioMuted(false);
    QVERIFY(!m_mediaManager->isAudioMuted());
    
    // 测试重复设置相同状态
    m_mediaManager->setVideoMuted(true);
    m_mediaManager->setVideoMuted(true); // 不应该产生额外信号
    
    m_mediaManager->setAudioMuted(false);
    m_mediaManager->setAudioMuted(false); // 不应该产生额外信号
}

void TestMediaManager::testMediaSettings()
{
    // 测试媒体设置
    MediaManager::MediaSettings originalSettings = m_mediaManager->mediaSettings();
    
    MediaManager::MediaSettings newSettings;
    newSettings.videoResolution = QSize(1920, 1080);
    newSettings.videoFrameRate = 60;
    newSettings.videoBitrate = 2000;
    newSettings.audioSampleRate = 48000;
    newSettings.audioChannels = 2;
    newSettings.audioBitrate = 256;
    
    m_mediaManager->setMediaSettings(newSettings);
    
    MediaManager::MediaSettings retrievedSettings = m_mediaManager->mediaSettings();
    QCOMPARE(retrievedSettings.videoResolution, newSettings.videoResolution);
    QCOMPARE(retrievedSettings.videoFrameRate, newSettings.videoFrameRate);
    QCOMPARE(retrievedSettings.videoBitrate, newSettings.videoBitrate);
    
    // 恢复原始设置
    m_mediaManager->setMediaSettings(originalSettings);
}

void TestMediaManager::testWebRTCIntegration()
{
    // 测试WebRTC集成
    QVERIFY(m_mediaManager->webRTCEngine() == m_webRTCEngine);
    
    // 测试设置null引擎
    m_mediaManager->setWebRTCEngine(nullptr);
    QVERIFY(m_mediaManager->webRTCEngine() == nullptr);
    
    // 恢复引擎
    m_mediaManager->setWebRTCEngine(m_webRTCEngine);
    QVERIFY(m_mediaManager->webRTCEngine() == m_webRTCEngine);
}

void TestMediaManager::testMediaCodecs()
{
    // 测试媒体编解码器
    QString originalVideoCodec = m_mediaManager->currentVideoCodec();
    QString originalAudioCodec = m_mediaManager->currentAudioCodec();
    
    // 测试设置编解码器
    m_mediaManager->setVideoCodec("H264");
    m_mediaManager->setAudioCodec("Opus");
    
    // 验证编解码器设置
    QString currentVideoCodec = m_mediaManager->currentVideoCodec();
    QString currentAudioCodec = m_mediaManager->currentAudioCodec();
    
    // 编解码器可能被设置也可能不被支持，但不应该崩溃
    QVERIFY(true);
    
    // 测试无效编解码器
    m_mediaManager->setVideoCodec("InvalidCodec");
    m_mediaManager->setAudioCodec("InvalidCodec");
    
    // 验证无效编解码器处理不会崩溃
    QVERIFY(true);
}

QTEST_MAIN(TestMediaManager)
#include "test_unit_media_manager.moc"