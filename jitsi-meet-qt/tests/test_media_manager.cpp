#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include "MediaManager.h"

class TestMediaManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testDeviceEnumeration();
    void testCameraSelection();
    void testMicrophoneSelection();
    void testSpeakerSelection();
    void testScreenSelection();
    void testVideoControl();
    void testAudioControl();
    void testScreenShare();
    void testVolumeControl();
    void testMuteControl();
    void testMediaQuality();

private:
    MediaManager* m_mediaManager;
};

void TestMediaManager::initTestCase()
{
    m_mediaManager = new MediaManager(this);
    QVERIFY(m_mediaManager != nullptr);
}

void TestMediaManager::cleanupTestCase()
{
    delete m_mediaManager;
    m_mediaManager = nullptr;
}

void TestMediaManager::testDeviceEnumeration()
{
    // 测试设备枚举功能
    QList<MediaManager::MediaDevice> cameras = m_mediaManager->availableCameras();
    QList<MediaManager::MediaDevice> microphones = m_mediaManager->availableMicrophones();
    QList<MediaManager::MediaDevice> speakers = m_mediaManager->availableSpeakers();
    QList<MediaManager::ScreenInfo> screens = m_mediaManager->availableScreens();
    
    // 至少应该有一个屏幕
    QVERIFY(screens.size() > 0);
    
    // 验证屏幕信息的完整性
    for (const auto& screen : screens) {
        QVERIFY(screen.screenId >= 0);
        QVERIFY(!screen.name.isEmpty());
        QVERIFY(screen.size.isValid());
        QVERIFY(screen.geometry.isValid());
        QVERIFY(screen.geometry.width() > 0);
        QVERIFY(screen.geometry.height() > 0);
    }
    
    // 验证摄像头信息（如果有的话）
    for (const auto& camera : cameras) {
        QVERIFY(!camera.id.isEmpty());
        QVERIFY(!camera.name.isEmpty());
        // description可以为空
    }
    
    // 验证麦克风信息（如果有的话）
    for (const auto& microphone : microphones) {
        QVERIFY(!microphone.id.isEmpty());
        QVERIFY(!microphone.name.isEmpty());
    }
    
    // 验证扬声器信息（如果有的话）
    for (const auto& speaker : speakers) {
        QVERIFY(!speaker.id.isEmpty());
        QVERIFY(!speaker.name.isEmpty());
    }
    
    qDebug() << "Found devices - Cameras:" << cameras.size() 
             << "Microphones:" << microphones.size() 
             << "Speakers:" << speakers.size() 
             << "Screens:" << screens.size();
             
    // 验证至少有一个主屏幕
    bool hasPrimaryScreen = false;
    for (const auto& screen : screens) {
        if (screen.isPrimary) {
            hasPrimaryScreen = true;
            break;
        }
    }
    QVERIFY(hasPrimaryScreen);
}

void TestMediaManager::testCameraSelection()
{
    QList<MediaManager::MediaDevice> cameras = m_mediaManager->availableCameras();
    
    if (cameras.isEmpty()) {
        QSKIP("No cameras available for testing");
        return;
    }
    
    // 测试选择第一个摄像头
    QString cameraId = cameras.first().id;
    QSignalSpy spy(m_mediaManager, &MediaManager::cameraChanged);
    
    bool result = m_mediaManager->selectCamera(cameraId);
    QVERIFY(result);
    
    // 验证信号发射
    QCOMPARE(spy.count(), 1);
    
    // 验证当前摄像头
    MediaManager::MediaDevice currentCamera = m_mediaManager->currentCamera();
    QCOMPARE(currentCamera.id, cameraId);
    
    // 测试选择无效摄像头
    bool invalidResult = m_mediaManager->selectCamera("invalid_camera_id");
    QVERIFY(!invalidResult);
}

void TestMediaManager::testMicrophoneSelection()
{
    QList<MediaManager::MediaDevice> microphones = m_mediaManager->availableMicrophones();
    
    if (microphones.isEmpty()) {
        QSKIP("No microphones available for testing");
        return;
    }
    
    // 测试选择第一个麦克风
    QString micId = microphones.first().id;
    QSignalSpy spy(m_mediaManager, &MediaManager::microphoneChanged);
    
    bool result = m_mediaManager->selectMicrophone(micId);
    QVERIFY(result);
    
    // 验证信号发射
    QCOMPARE(spy.count(), 1);
    
    // 验证当前麦克风
    MediaManager::MediaDevice currentMic = m_mediaManager->currentMicrophone();
    QCOMPARE(currentMic.id, micId);
}

void TestMediaManager::testSpeakerSelection()
{
    QList<MediaManager::MediaDevice> speakers = m_mediaManager->availableSpeakers();
    
    if (speakers.isEmpty()) {
        QSKIP("No speakers available for testing");
        return;
    }
    
    // 测试选择第一个扬声器
    QString speakerId = speakers.first().id;
    QSignalSpy spy(m_mediaManager, &MediaManager::speakerChanged);
    
    bool result = m_mediaManager->selectSpeaker(speakerId);
    QVERIFY(result);
    
    // 验证信号发射
    QCOMPARE(spy.count(), 1);
    
    // 验证当前扬声器
    MediaManager::MediaDevice currentSpeaker = m_mediaManager->currentSpeaker();
    QCOMPARE(currentSpeaker.id, speakerId);
}

void TestMediaManager::testScreenSelection()
{
    QList<MediaManager::ScreenInfo> screens = m_mediaManager->availableScreens();
    QVERIFY(screens.size() > 0);
    
    // 测试选择第一个屏幕
    int screenId = screens.first().screenId;
    QSignalSpy spy(m_mediaManager, &MediaManager::screenChanged);
    
    bool result = m_mediaManager->selectScreen(screenId);
    QVERIFY(result);
    
    // 验证信号发射
    QCOMPARE(spy.count(), 1);
    
    // 验证当前屏幕
    MediaManager::ScreenInfo currentScreen = m_mediaManager->currentScreen();
    QCOMPARE(currentScreen.screenId, screenId);
}

void TestMediaManager::testVideoControl()
{
    // 测试视频控制功能
    QVERIFY(!m_mediaManager->isVideoEnabled());
    
    QSignalSpy startSpy(m_mediaManager, &MediaManager::localVideoStarted);
    QSignalSpy stopSpy(m_mediaManager, &MediaManager::localVideoStopped);
    
    // 启动视频
    m_mediaManager->startLocalVideo();
    
    // 如果有摄像头，应该启动成功
    QList<MediaManager::MediaDevice> cameras = m_mediaManager->availableCameras();
    if (!cameras.isEmpty()) {
        // 等待信号
        QVERIFY(startSpy.wait(5000));
        QVERIFY(m_mediaManager->isVideoEnabled());
        
        // 获取本地视频组件
        QVideoWidget* videoWidget = m_mediaManager->localVideoWidget();
        QVERIFY(videoWidget != nullptr);
        
        // 停止视频
        m_mediaManager->stopLocalVideo();
        QVERIFY(stopSpy.wait(1000));
        QVERIFY(!m_mediaManager->isVideoEnabled());
    }
}

void TestMediaManager::testAudioControl()
{
    // 测试音频控制功能
    QVERIFY(!m_mediaManager->isAudioEnabled());
    
    QSignalSpy startSpy(m_mediaManager, &MediaManager::localAudioStarted);
    QSignalSpy stopSpy(m_mediaManager, &MediaManager::localAudioStopped);
    
    // 启动音频
    m_mediaManager->startLocalAudio();
    
    // 如果有麦克风，应该启动成功
    QList<MediaManager::MediaDevice> microphones = m_mediaManager->availableMicrophones();
    if (!microphones.isEmpty()) {
        // 等待信号
        QVERIFY(startSpy.wait(5000));
        QVERIFY(m_mediaManager->isAudioEnabled());
        
        // 停止音频
        m_mediaManager->stopLocalAudio();
        QVERIFY(stopSpy.wait(1000));
        QVERIFY(!m_mediaManager->isAudioEnabled());
    }
}

void TestMediaManager::testScreenShare()
{
    // 测试屏幕共享功能
    QVERIFY(!m_mediaManager->isScreenShareEnabled());
    
    QSignalSpy startSpy(m_mediaManager, &MediaManager::screenShareStarted);
    QSignalSpy stopSpy(m_mediaManager, &MediaManager::screenShareStopped);
    
    // 启动屏幕共享
    m_mediaManager->startScreenShare();
    
    // 应该启动成功（至少有一个屏幕）
    QVERIFY(startSpy.wait(1000));
    QVERIFY(m_mediaManager->isScreenShareEnabled());
    
    // 获取屏幕共享组件
    QVideoWidget* screenWidget = m_mediaManager->screenShareWidget();
    QVERIFY(screenWidget != nullptr);
    
    // 停止屏幕共享
    m_mediaManager->stopScreenShare();
    QVERIFY(stopSpy.wait(1000));
    QVERIFY(!m_mediaManager->isScreenShareEnabled());
}

void TestMediaManager::testVolumeControl()
{
    // 测试音量控制
    QSignalSpy micVolumeSpy(m_mediaManager, &MediaManager::microphoneVolumeChanged);
    QSignalSpy speakerVolumeSpy(m_mediaManager, &MediaManager::speakerVolumeChanged);
    
    // 测试麦克风音量
    m_mediaManager->setMicrophoneVolume(50);
    QCOMPARE(m_mediaManager->microphoneVolume(), 50);
    QCOMPARE(micVolumeSpy.count(), 1);
    
    // 测试边界值
    m_mediaManager->setMicrophoneVolume(-10);
    QCOMPARE(m_mediaManager->microphoneVolume(), 0);
    
    m_mediaManager->setMicrophoneVolume(150);
    QCOMPARE(m_mediaManager->microphoneVolume(), 100);
    
    // 测试扬声器音量
    m_mediaManager->setSpeakerVolume(75);
    QCOMPARE(m_mediaManager->speakerVolume(), 75);
    QCOMPARE(speakerVolumeSpy.count(), 1);
}

void TestMediaManager::testMuteControl()
{
    // 测试静音控制
    QSignalSpy micMuteSpy(m_mediaManager, &MediaManager::microphoneMutedChanged);
    QSignalSpy speakerMuteSpy(m_mediaManager, &MediaManager::speakerMutedChanged);
    
    // 测试麦克风静音
    QVERIFY(!m_mediaManager->isMicrophoneMuted());
    m_mediaManager->setMicrophoneMuted(true);
    QVERIFY(m_mediaManager->isMicrophoneMuted());
    QCOMPARE(micMuteSpy.count(), 1);
    
    m_mediaManager->setMicrophoneMuted(false);
    QVERIFY(!m_mediaManager->isMicrophoneMuted());
    QCOMPARE(micMuteSpy.count(), 2);
    
    // 测试扬声器静音
    QVERIFY(!m_mediaManager->isSpeakerMuted());
    m_mediaManager->setSpeakerMuted(true);
    QVERIFY(m_mediaManager->isSpeakerMuted());
    QCOMPARE(speakerMuteSpy.count(), 1);
    
    m_mediaManager->setSpeakerMuted(false);
    QVERIFY(!m_mediaManager->isSpeakerMuted());
    QCOMPARE(speakerMuteSpy.count(), 2);
}

void TestMediaManager::testMediaQuality()
{
    // 测试媒体质量设置
    MediaManager::MediaQuality quality;
    quality.videoResolution = QSize(1280, 720);
    quality.videoFrameRate = 60;
    quality.videoBitrate = 2000000;
    quality.audioSampleRate = 48000;
    quality.audioChannels = 2;
    quality.audioBitrate = 256000;
    
    m_mediaManager->setMediaQuality(quality);
    
    MediaManager::MediaQuality currentQuality = m_mediaManager->mediaQuality();
    QCOMPARE(currentQuality.videoResolution, quality.videoResolution);
    QCOMPARE(currentQuality.videoFrameRate, quality.videoFrameRate);
    QCOMPARE(currentQuality.videoBitrate, quality.videoBitrate);
    QCOMPARE(currentQuality.audioSampleRate, quality.audioSampleRate);
    QCOMPARE(currentQuality.audioChannels, quality.audioChannels);
    QCOMPARE(currentQuality.audioBitrate, quality.audioBitrate);
}

QTEST_MAIN(TestMediaManager)
#include "test_media_manager.moc"