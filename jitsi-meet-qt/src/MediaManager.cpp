#include "MediaManager.h"
#include "WebRTCEngine.h"
#include <QDebug>
#include <QCameraInfo>
#include <QAudioDeviceInfo>
#include <QGuiApplication>
#include <QPixmap>
#include <QBuffer>
#include <QImageWriter>
#include <QVideoFrame>
#include <QCameraViewfinder>

MediaManager::MediaManager(QObject *parent)
    : QObject(parent)
    , m_camera(nullptr)
    , m_localVideoWidget(nullptr)
    , m_cameraViewfinder(nullptr)
    , m_audioInput(nullptr)
    , m_audioOutput(nullptr)
    , m_mediaRecorder(nullptr)
    , m_screenCaptureTimer(nullptr)
    , m_screenShareWidget(nullptr)
    , m_selectedScreen(nullptr)
    , m_videoEnabled(false)
    , m_audioEnabled(false)
    , m_screenShareEnabled(false)
    , m_microphoneMuted(false)
    , m_speakerMuted(false)
    , m_microphoneVolume(80)
    , m_speakerVolume(80)
    , m_webrtcEngine(nullptr)
{
    qDebug() << "MediaManager: Initializing media manager";
    
    // 初始化设备
    initializeDevices();
    
    // 创建屏幕捕获定时器
    m_screenCaptureTimer = new QTimer(this);
    m_screenCaptureTimer->setInterval(SCREEN_CAPTURE_INTERVAL);
    connect(m_screenCaptureTimer, &QTimer::timeout, this, &MediaManager::onScreenCaptureTimer);
    
    qDebug() << "MediaManager: Initialization completed";
}

MediaManager::~MediaManager()
{
    qDebug() << "MediaManager: Cleaning up media manager";
    cleanupDevices();
}

void MediaManager::initializeDevices()
{
    qDebug() << "MediaManager: Initializing devices";
    
    // 枚举所有可用设备
    refreshDeviceList();
    
    // 选择默认设备
    if (!m_cameras.isEmpty()) {
        // 选择第一个可用摄像头或默认摄像头
        for (const auto& camera : m_cameras) {
            if (camera.isDefault) {
                selectCamera(camera.id);
                break;
            }
        }
        if (m_currentCamera.id.isEmpty() && !m_cameras.isEmpty()) {
            selectCamera(m_cameras.first().id);
        }
    }
    
    if (!m_microphones.isEmpty()) {
        // 选择默认麦克风
        for (const auto& mic : m_microphones) {
            if (mic.isDefault) {
                selectMicrophone(mic.id);
                break;
            }
        }
        if (m_currentMicrophone.id.isEmpty() && !m_microphones.isEmpty()) {
            selectMicrophone(m_microphones.first().id);
        }
    }
    
    if (!m_speakers.isEmpty()) {
        // 选择默认扬声器
        for (const auto& speaker : m_speakers) {
            if (speaker.isDefault) {
                selectSpeaker(speaker.id);
                break;
            }
        }
        if (m_currentSpeaker.id.isEmpty() && !m_speakers.isEmpty()) {
            selectSpeaker(m_speakers.first().id);
        }
    }
    
    if (!m_screens.isEmpty()) {
        // 选择主屏幕
        for (const auto& screen : m_screens) {
            if (screen.isPrimary) {
                selectScreen(screen.screenId);
                break;
            }
        }
        if (m_currentScreen.screenId == -1 && !m_screens.isEmpty()) {
            selectScreen(m_screens.first().screenId);
        }
    }
}

void MediaManager::cleanupDevices()
{
    qDebug() << "MediaManager: Cleaning up devices";
    
    // 停止所有媒体流
    stopLocalVideo();
    stopLocalAudio();
    stopScreenShare();
    
    // 清理摄像头
    cleanupCamera();
    
    // 清理音频
    cleanupAudio();
    
    // 清理屏幕捕获
    cleanupScreenCapture();
    
    // 清理远程视频组件
    for (auto it = m_remoteVideoWidgets.begin(); it != m_remoteVideoWidgets.end(); ++it) {
        delete it.value();
    }
    m_remoteVideoWidgets.clear();
}

void MediaManager::refreshDeviceList()
{
    qDebug() << "MediaManager: Refreshing device list";
    
    enumerateCameras();
    enumerateAudioDevices();
    enumerateScreens();
    
    emit deviceListChanged();
}

void MediaManager::enumerateCameras()
{
    m_cameras.clear();
    
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QCameraInfo defaultCamera = QCameraInfo::defaultCamera();
    
    for (const QCameraInfo& cameraInfo : cameras) {
        MediaDevice device;
        device.id = cameraInfo.deviceName();
        device.name = cameraInfo.description();
        device.description = QString("Camera: %1").arg(cameraInfo.description());
        device.isDefault = (cameraInfo == defaultCamera);
        
        m_cameras.append(device);
        qDebug() << "MediaManager: Found camera:" << device.name << "ID:" << device.id << "Default:" << device.isDefault;
    }
    
    qDebug() << "MediaManager: Enumerated" << m_cameras.size() << "cameras";
}

void MediaManager::enumerateAudioDevices()
{
    m_microphones.clear();
    m_speakers.clear();
    
    // 枚举音频输入设备（麦克风）
    QList<QAudioDeviceInfo> inputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    QAudioDeviceInfo defaultInputDevice = QAudioDeviceInfo::defaultInputDevice();
    
    for (const QAudioDeviceInfo& deviceInfo : inputDevices) {
        MediaDevice device;
        device.id = deviceInfo.deviceName();
        device.name = deviceInfo.deviceName();
        device.description = QString("Microphone: %1").arg(deviceInfo.deviceName());
        device.isDefault = (deviceInfo == defaultInputDevice);
        
        m_microphones.append(device);
        qDebug() << "MediaManager: Found microphone:" << device.name << "Default:" << device.isDefault;
    }
    
    // 枚举音频输出设备（扬声器）
    QList<QAudioDeviceInfo> outputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    QAudioDeviceInfo defaultOutputDevice = QAudioDeviceInfo::defaultOutputDevice();
    
    for (const QAudioDeviceInfo& deviceInfo : outputDevices) {
        MediaDevice device;
        device.id = deviceInfo.deviceName();
        device.name = deviceInfo.deviceName();
        device.description = QString("Speaker: %1").arg(deviceInfo.deviceName());
        device.isDefault = (deviceInfo == defaultOutputDevice);
        
        m_speakers.append(device);
        qDebug() << "MediaManager: Found speaker:" << device.name << "Default:" << device.isDefault;
    }
    
    qDebug() << "MediaManager: Enumerated" << m_microphones.size() << "microphones and" << m_speakers.size() << "speakers";
}

void MediaManager::enumerateScreens()
{
    m_screens.clear();
    
    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    
    for (int i = 0; i < screens.size(); ++i) {
        QScreen* screen = screens[i];
        ScreenInfo screenInfo;
        screenInfo.screenId = i;
        screenInfo.name = screen->name();
        screenInfo.size = screen->size();
        screenInfo.geometry = screen->geometry();
        screenInfo.isPrimary = (screen == primaryScreen);
        
        m_screens.append(screenInfo);
        qDebug() << "MediaManager: Found screen:" << screenInfo.name 
                 << "Size:" << screenInfo.size 
                 << "Primary:" << screenInfo.isPrimary;
    }
    
    qDebug() << "MediaManager: Enumerated" << m_screens.size() << "screens";
}

QList<MediaManager::MediaDevice> MediaManager::availableCameras() const
{
    return m_cameras;
}

QList<MediaManager::MediaDevice> MediaManager::availableMicrophones() const
{
    return m_microphones;
}

QList<MediaManager::MediaDevice> MediaManager::availableSpeakers() const
{
    return m_speakers;
}

QList<MediaManager::ScreenInfo> MediaManager::availableScreens() const
{
    return m_screens;
}

bool MediaManager::selectCamera(const QString& deviceId)
{
    qDebug() << "MediaManager: Selecting camera:" << deviceId;
    
    // 查找指定的摄像头
    MediaDevice selectedDevice;
    bool found = false;
    for (const auto& camera : m_cameras) {
        if (camera.id == deviceId) {
            selectedDevice = camera;
            found = true;
            break;
        }
    }
    
    if (!found) {
        qWarning() << "MediaManager: Camera not found:" << deviceId;
        return false;
    }
    
    // 如果当前已经选择了这个摄像头，直接返回
    if (m_currentCamera.id == deviceId) {
        qDebug() << "MediaManager: Camera already selected:" << deviceId;
        return true;
    }
    
    // 停止当前摄像头
    bool wasVideoEnabled = m_videoEnabled;
    if (m_videoEnabled) {
        stopLocalVideo();
    }
    
    // 清理当前摄像头
    cleanupCamera();
    
    // 设置新摄像头
    m_currentCamera = selectedDevice;
    
    // 重新启动视频（如果之前是启用的）
    if (wasVideoEnabled) {
        startLocalVideo();
    }
    
    emit cameraChanged(m_currentCamera);
    qDebug() << "MediaManager: Camera selected successfully:" << deviceId;
    return true;
}

bool MediaManager::selectMicrophone(const QString& deviceId)
{
    qDebug() << "MediaManager: Selecting microphone:" << deviceId;
    
    // 查找指定的麦克风
    MediaDevice selectedDevice;
    bool found = false;
    for (const auto& mic : m_microphones) {
        if (mic.id == deviceId) {
            selectedDevice = mic;
            found = true;
            break;
        }
    }
    
    if (!found) {
        qWarning() << "MediaManager: Microphone not found:" << deviceId;
        return false;
    }
    
    // 如果当前已经选择了这个麦克风，直接返回
    if (m_currentMicrophone.id == deviceId) {
        qDebug() << "MediaManager: Microphone already selected:" << deviceId;
        return true;
    }
    
    // 停止当前音频
    bool wasAudioEnabled = m_audioEnabled;
    if (m_audioEnabled) {
        stopLocalAudio();
    }
    
    // 设置新麦克风
    m_currentMicrophone = selectedDevice;
    
    // 重新启动音频（如果之前是启用的）
    if (wasAudioEnabled) {
        startLocalAudio();
    }
    
    emit microphoneChanged(m_currentMicrophone);
    qDebug() << "MediaManager: Microphone selected successfully:" << deviceId;
    return true;
}

bool MediaManager::selectSpeaker(const QString& deviceId)
{
    qDebug() << "MediaManager: Selecting speaker:" << deviceId;
    
    // 查找指定的扬声器
    MediaDevice selectedDevice;
    bool found = false;
    for (const auto& speaker : m_speakers) {
        if (speaker.id == deviceId) {
            selectedDevice = speaker;
            found = true;
            break;
        }
    }
    
    if (!found) {
        qWarning() << "MediaManager: Speaker not found:" << deviceId;
        return false;
    }
    
    // 设置新扬声器
    m_currentSpeaker = selectedDevice;
    
    // 重新设置音频输出
    if (m_audioOutput) {
        delete m_audioOutput;
        m_audioOutput = nullptr;
    }
    
    setupAudioOutput();
    
    emit speakerChanged(m_currentSpeaker);
    qDebug() << "MediaManager: Speaker selected successfully:" << deviceId;
    return true;
}

bool MediaManager::selectScreen(int screenId)
{
    qDebug() << "MediaManager: Selecting screen:" << screenId;
    
    // 查找指定的屏幕
    ScreenInfo selectedScreen;
    bool found = false;
    for (const auto& screen : m_screens) {
        if (screen.screenId == screenId) {
            selectedScreen = screen;
            found = true;
            break;
        }
    }
    
    if (!found) {
        qWarning() << "MediaManager: Screen not found:" << screenId;
        return false;
    }
    
    // 如果当前已经选择了这个屏幕，直接返回
    if (m_currentScreen.screenId == screenId) {
        qDebug() << "MediaManager: Screen already selected:" << screenId;
        return true;
    }
    
    // 停止当前屏幕共享
    bool wasScreenShareEnabled = m_screenShareEnabled;
    if (m_screenShareEnabled) {
        stopScreenShare();
    }
    
    // 设置新屏幕
    m_currentScreen = selectedScreen;
    
    // 获取屏幕对象
    QList<QScreen*> screens = QGuiApplication::screens();
    if (screenId >= 0 && screenId < screens.size()) {
        m_selectedScreen = screens[screenId];
    }
    
    // 重新启动屏幕共享（如果之前是启用的）
    if (wasScreenShareEnabled) {
        startScreenShare();
    }
    
    emit screenChanged(m_currentScreen);
    qDebug() << "MediaManager: Screen selected successfully:" << screenId;
    return true;
}

MediaManager::MediaDevice MediaManager::currentCamera() const
{
    return m_currentCamera;
}

MediaManager::MediaDevice MediaManager::currentMicrophone() const
{
    return m_currentMicrophone;
}

MediaManager::MediaDevice MediaManager::currentSpeaker() const
{
    return m_currentSpeaker;
}

MediaManager::ScreenInfo MediaManager::currentScreen() const
{
    return m_currentScreen;
}

void MediaManager::startLocalVideo()
{
    qDebug() << "MediaManager: Starting local video";
    
    if (m_videoEnabled) {
        qDebug() << "MediaManager: Video already enabled";
        return;
    }
    
    if (m_currentCamera.id.isEmpty()) {
        qWarning() << "MediaManager: No camera selected";
        emit cameraError("No camera selected");
        return;
    }
    
    try {
        setupCamera();
        
        if (m_camera && m_camera->state() == QCamera::ActiveState) {
            m_videoEnabled = true;
            emit localVideoStarted();
            qDebug() << "MediaManager: Local video started successfully";
        } else {
            qWarning() << "MediaManager: Failed to start camera";
            emit cameraError("Failed to start camera");
        }
    } catch (const std::exception& e) {
        qWarning() << "MediaManager: Exception starting video:" << e.what();
        emit cameraError(QString("Exception: %1").arg(e.what()));
    }
}

void MediaManager::stopLocalVideo()
{
    qDebug() << "MediaManager: Stopping local video";
    
    if (!m_videoEnabled) {
        qDebug() << "MediaManager: Video already disabled";
        return;
    }
    
    m_videoEnabled = false;
    
    if (m_camera) {
        m_camera->stop();
    }
    
    emit localVideoStopped();
    qDebug() << "MediaManager: Local video stopped";
}

void MediaManager::startLocalAudio()
{
    qDebug() << "MediaManager: Starting local audio";
    
    if (m_audioEnabled) {
        qDebug() << "MediaManager: Audio already enabled";
        return;
    }
    
    if (m_currentMicrophone.id.isEmpty()) {
        qWarning() << "MediaManager: No microphone selected";
        emit microphoneError("No microphone selected");
        return;
    }
    
    try {
        setupAudioInput();
        
        if (m_audioInput) {
            m_audioEnabled = true;
            emit localAudioStarted();
            qDebug() << "MediaManager: Local audio started successfully";
        } else {
            qWarning() << "MediaManager: Failed to start microphone";
            emit microphoneError("Failed to start microphone");
        }
    } catch (const std::exception& e) {
        qWarning() << "MediaManager: Exception starting audio:" << e.what();
        emit microphoneError(QString("Exception: %1").arg(e.what()));
    }
}

void MediaManager::stopLocalAudio()
{
    qDebug() << "MediaManager: Stopping local audio";
    
    if (!m_audioEnabled) {
        qDebug() << "MediaManager: Audio already disabled";
        return;
    }
    
    m_audioEnabled = false;
    
    if (m_audioInput) {
        // 停止音频输入
        // QAudioInput没有直接的stop方法，通过删除重新创建来停止
    }
    
    emit localAudioStopped();
    qDebug() << "MediaManager: Local audio stopped";
}

void MediaManager::startScreenShare()
{
    qDebug() << "MediaManager: Starting screen share";
    
    if (m_screenShareEnabled) {
        qDebug() << "MediaManager: Screen share already enabled";
        return;
    }
    
    if (m_currentScreen.screenId == -1 || !m_selectedScreen) {
        qWarning() << "MediaManager: No screen selected";
        emit screenCaptureError("No screen selected");
        return;
    }
    
    try {
        setupScreenCapture();
        
        m_screenShareEnabled = true;
        m_screenCaptureTimer->start();
        
        emit screenShareStarted();
        qDebug() << "MediaManager: Screen share started successfully";
    } catch (const std::exception& e) {
        qWarning() << "MediaManager: Exception starting screen share:" << e.what();
        emit screenCaptureError(QString("Exception: %1").arg(e.what()));
    }
}

void MediaManager::stopScreenShare()
{
    qDebug() << "MediaManager: Stopping screen share";
    
    if (!m_screenShareEnabled) {
        qDebug() << "MediaManager: Screen share already disabled";
        return;
    }
    
    m_screenShareEnabled = false;
    
    if (m_screenCaptureTimer) {
        m_screenCaptureTimer->stop();
    }
    
    emit screenShareStopped();
    qDebug() << "MediaManager: Screen share stopped";
}

bool MediaManager::isVideoEnabled() const
{
    return m_videoEnabled;
}

bool MediaManager::isAudioEnabled() const
{
    return m_audioEnabled;
}

bool MediaManager::isScreenShareEnabled() const
{
    return m_screenShareEnabled;
}

void MediaManager::setMediaQuality(const MediaQuality& quality)
{
    qDebug() << "MediaManager: Setting media quality - Video:" 
             << quality.videoResolution << "@" << quality.videoFrameRate << "fps"
             << "Audio:" << quality.audioSampleRate << "Hz";
    
    m_mediaQuality = quality;
    
    // 如果当前有活动的媒体流，需要重新配置
    if (m_videoEnabled) {
        updateCameraSettings();
    }
    
    if (m_audioEnabled) {
        updateAudioSettings();
    }
}

MediaManager::MediaQuality MediaManager::mediaQuality() const
{
    return m_mediaQuality;
}

QVideoWidget* MediaManager::localVideoWidget() const
{
    return m_localVideoWidget;
}

QVideoWidget* MediaManager::screenShareWidget() const
{
    return m_screenShareWidget;
}

void MediaManager::setMicrophoneVolume(int volume)
{
    m_microphoneVolume = qBound(0, volume, 100);
    
    // 应用音量设置到音频输入
    if (m_audioInput) {
        // QAudioInput的音量控制需要通过QAudioFormat或其他方式实现
        // 这里先记录设置，实际应用需要在音频处理中实现
    }
    
    emit microphoneVolumeChanged(m_microphoneVolume);
    qDebug() << "MediaManager: Microphone volume set to:" << m_microphoneVolume;
}

void MediaManager::setSpeakerVolume(int volume)
{
    m_speakerVolume = qBound(0, volume, 100);
    
    // 应用音量设置到音频输出
    if (m_audioOutput) {
        // QAudioOutput的音量控制
        qreal normalizedVolume = m_speakerVolume / 100.0;
        m_audioOutput->setVolume(normalizedVolume);
    }
    
    emit speakerVolumeChanged(m_speakerVolume);
    qDebug() << "MediaManager: Speaker volume set to:" << m_speakerVolume;
}

int MediaManager::microphoneVolume() const
{
    return m_microphoneVolume;
}

int MediaManager::speakerVolume() const
{
    return m_speakerVolume;
}

void MediaManager::setMicrophoneMuted(bool muted)
{
    m_microphoneMuted = muted;
    
    // 应用静音设置
    if (m_audioInput) {
        // 实现麦克风静音逻辑
        // 可以通过停止音频输入或在音频处理中实现
    }
    
    emit microphoneMutedChanged(m_microphoneMuted);
    qDebug() << "MediaManager: Microphone muted:" << m_microphoneMuted;
}

void MediaManager::setSpeakerMuted(bool muted)
{
    m_speakerMuted = muted;
    
    // 应用静音设置
    if (m_audioOutput) {
        if (muted) {
            m_audioOutput->setVolume(0.0);
        } else {
            m_audioOutput->setVolume(m_speakerVolume / 100.0);
        }
    }
    
    emit speakerMutedChanged(m_speakerMuted);
    qDebug() << "MediaManager: Speaker muted:" << m_speakerMuted;
}

bool MediaManager::isMicrophoneMuted() const
{
    return m_microphoneMuted;
}

bool MediaManager::isSpeakerMuted() const
{
    return m_speakerMuted;
}

void MediaManager::setWebRTCEngine(WebRTCEngine* engine)
{
    if (m_webrtcEngine == engine) {
        return;
    }
    
    // 断开旧连接
    if (m_webrtcEngine) {
        disconnectFromWebRTC();
    }
    
    m_webrtcEngine = engine;
    
    // 建立新连接
    if (m_webrtcEngine) {
        connectToWebRTC();
    }
    
    qDebug() << "MediaManager: WebRTC engine set:" << (engine ? "connected" : "disconnected");
}

WebRTCEngine* MediaManager::webRTCEngine() const
{
    return m_webrtcEngine;
}

void MediaManager::setupCamera()
{
    qDebug() << "MediaManager: Setting up camera";
    
    // 清理现有摄像头
    cleanupCamera();
    
    if (m_currentCamera.id.isEmpty()) {
        qWarning() << "MediaManager: No camera selected for setup";
        return;
    }
    
    // 查找摄像头信息
    QCameraInfo selectedCameraInfo;
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    
    for (const QCameraInfo& cameraInfo : cameras) {
        if (cameraInfo.deviceName() == m_currentCamera.id) {
            selectedCameraInfo = cameraInfo;
            break;
        }
    }
    
    if (selectedCameraInfo.isNull()) {
        qWarning() << "MediaManager: Camera info not found for:" << m_currentCamera.id;
        return;
    }
    
    // 创建摄像头
    m_camera = new QCamera(selectedCameraInfo, this);
    
    // 创建视频预览组件
    if (!m_localVideoWidget) {
        m_localVideoWidget = new QVideoWidget();
        m_localVideoWidget->setMinimumSize(DEFAULT_VIDEO_WIDTH, DEFAULT_VIDEO_HEIGHT);
    }
    
    // 创建取景器
    if (!m_cameraViewfinder) {
        m_cameraViewfinder = new QCameraViewfinder();
    }
    
    // 设置取景器到摄像头
    m_camera->setViewfinder(m_cameraViewfinder);
    
    // 连接信号
    connect(m_camera, QOverload<QCamera::State>::of(&QCamera::stateChanged),
            this, &MediaManager::onCameraStateChanged);
    connect(m_camera, QOverload<QCamera::Error>::of(&QCamera::error),
            this, &MediaManager::onCameraError);
    
    // 应用摄像头设置
    updateCameraSettings();
    
    // 启动摄像头
    m_camera->start();
    
    qDebug() << "MediaManager: Camera setup completed";
}

void MediaManager::cleanupCamera()
{
    qDebug() << "MediaManager: Cleaning up camera";
    
    if (m_camera) {
        m_camera->stop();
        m_camera->deleteLater();
        m_camera = nullptr;
    }
    
    if (m_cameraViewfinder) {
        m_cameraViewfinder->deleteLater();
        m_cameraViewfinder = nullptr;
    }
    
    // 注意：不删除m_localVideoWidget，因为它可能被外部使用
}

void MediaManager::updateCameraSettings()
{
    if (!m_camera) {
        return;
    }
    
    qDebug() << "MediaManager: Updating camera settings";
    
    // 设置视频分辨率和帧率
    QCameraViewfinderSettings viewfinderSettings;
    viewfinderSettings.setResolution(m_mediaQuality.videoResolution);
    viewfinderSettings.setMaximumFrameRate(m_mediaQuality.videoFrameRate);
    viewfinderSettings.setMinimumFrameRate(m_mediaQuality.videoFrameRate);
    
    m_camera->setViewfinderSettings(viewfinderSettings);
    
    qDebug() << "MediaManager: Camera settings updated - Resolution:" 
             << m_mediaQuality.videoResolution << "FPS:" << m_mediaQuality.videoFrameRate;
}

void MediaManager::setupAudioInput()
{
    qDebug() << "MediaManager: Setting up audio input";
    
    if (m_currentMicrophone.id.isEmpty()) {
        qWarning() << "MediaManager: No microphone selected for setup";
        return;
    }
    
    // 查找音频输入设备
    QAudioDeviceInfo selectedDeviceInfo;
    QList<QAudioDeviceInfo> inputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    
    for (const QAudioDeviceInfo& deviceInfo : inputDevices) {
        if (deviceInfo.deviceName() == m_currentMicrophone.id) {
            selectedDeviceInfo = deviceInfo;
            break;
        }
    }
    
    if (selectedDeviceInfo.isNull()) {
        qWarning() << "MediaManager: Audio input device not found:" << m_currentMicrophone.id;
        return;
    }
    
    // 设置音频格式
    QAudioFormat audioFormat;
    audioFormat.setSampleRate(m_mediaQuality.audioSampleRate);
    audioFormat.setChannelCount(m_mediaQuality.audioChannels);
    audioFormat.setSampleSize(16);
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::SignedInt);
    
    // 检查格式支持
    if (!selectedDeviceInfo.isFormatSupported(audioFormat)) {
        qWarning() << "MediaManager: Audio format not supported, using nearest";
        audioFormat = selectedDeviceInfo.nearestFormat(audioFormat);
    }
    
    // 创建音频输入
    m_audioInput = new QAudioInput(selectedDeviceInfo, audioFormat, this);
    
    // 应用音频设置
    updateAudioSettings();
    
    qDebug() << "MediaManager: Audio input setup completed";
}

void MediaManager::setupAudioOutput()
{
    qDebug() << "MediaManager: Setting up audio output";
    
    if (m_currentSpeaker.id.isEmpty()) {
        qWarning() << "MediaManager: No speaker selected for setup";
        return;
    }
    
    // 查找音频输出设备
    QAudioDeviceInfo selectedDeviceInfo;
    QList<QAudioDeviceInfo> outputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    
    for (const QAudioDeviceInfo& deviceInfo : outputDevices) {
        if (deviceInfo.deviceName() == m_currentSpeaker.id) {
            selectedDeviceInfo = deviceInfo;
            break;
        }
    }
    
    if (selectedDeviceInfo.isNull()) {
        qWarning() << "MediaManager: Audio output device not found:" << m_currentSpeaker.id;
        return;
    }
    
    // 设置音频格式
    QAudioFormat audioFormat;
    audioFormat.setSampleRate(m_mediaQuality.audioSampleRate);
    audioFormat.setChannelCount(m_mediaQuality.audioChannels);
    audioFormat.setSampleSize(16);
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::SignedInt);
    
    // 检查格式支持
    if (!selectedDeviceInfo.isFormatSupported(audioFormat)) {
        qWarning() << "MediaManager: Audio format not supported, using nearest";
        audioFormat = selectedDeviceInfo.nearestFormat(audioFormat);
    }
    
    // 创建音频输出
    m_audioOutput = new QAudioOutput(selectedDeviceInfo, audioFormat, this);
    
    // 设置音量
    m_audioOutput->setVolume(m_speakerMuted ? 0.0 : (m_speakerVolume / 100.0));
    
    qDebug() << "MediaManager: Audio output setup completed";
}

void MediaManager::cleanupAudio()
{
    qDebug() << "MediaManager: Cleaning up audio";
    
    if (m_audioInput) {
        m_audioInput->deleteLater();
        m_audioInput = nullptr;
    }
    
    if (m_audioOutput) {
        m_audioOutput->deleteLater();
        m_audioOutput = nullptr;
    }
}

void MediaManager::updateAudioSettings()
{
    // 音频设置更新逻辑
    // 由于QAudioInput/QAudioOutput的限制，通常需要重新创建来应用新设置
    qDebug() << "MediaManager: Audio settings updated";
}

void MediaManager::setupScreenCapture()
{
    qDebug() << "MediaManager: Setting up screen capture";
    
    if (!m_selectedScreen) {
        qWarning() << "MediaManager: No screen selected for capture";
        return;
    }
    
    // 创建屏幕共享视频组件
    if (!m_screenShareWidget) {
        m_screenShareWidget = new QVideoWidget();
        m_screenShareWidget->setMinimumSize(m_currentScreen.size / 4); // 1/4 size for preview
    }
    
    qDebug() << "MediaManager: Screen capture setup completed";
}

void MediaManager::cleanupScreenCapture()
{
    qDebug() << "MediaManager: Cleaning up screen capture";
    
    if (m_screenCaptureTimer) {
        m_screenCaptureTimer->stop();
    }
    
    // 注意：不删除m_screenShareWidget，因为它可能被外部使用
}

void MediaManager::captureScreenFrame()
{
    if (!m_selectedScreen || !m_screenShareEnabled) {
        return;
    }
    
    // 捕获屏幕内容
    QPixmap screenshot = m_selectedScreen->grabWindow(0);
    
    if (screenshot.isNull()) {
        qWarning() << "MediaManager: Failed to capture screen";
        return;
    }
    
    // 编码屏幕帧
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    screenshot.save(&buffer, "PNG");
    QByteArray frameData = buffer.data();
    
    // 发送到WebRTC
    if (m_webrtcEngine) {
        sendVideoFrame(frameData);
    }
    
    // 更新预览（可选）
    if (m_screenShareWidget) {
        // 这里可以更新屏幕共享预览
        // 由于QVideoWidget主要用于视频流，屏幕截图预览可能需要其他方式实现
    }
}

void MediaManager::connectToWebRTC()
{
    if (!m_webrtcEngine) {
        return;
    }
    
    qDebug() << "MediaManager: Connecting to WebRTC engine";
    
    // 连接WebRTC信号
    connect(m_webrtcEngine, &WebRTCEngine::remoteStreamReceived,
            this, &MediaManager::remoteVideoReceived);
    connect(m_webrtcEngine, &WebRTCEngine::remoteStreamRemoved,
            this, &MediaManager::remoteVideoRemoved);
    
    qDebug() << "MediaManager: Connected to WebRTC engine";
}

void MediaManager::disconnectFromWebRTC()
{
    if (!m_webrtcEngine) {
        return;
    }
    
    qDebug() << "MediaManager: Disconnecting from WebRTC engine";
    
    // 断开WebRTC信号
    disconnect(m_webrtcEngine, nullptr, this, nullptr);
    
    qDebug() << "MediaManager: Disconnected from WebRTC engine";
}

void MediaManager::sendVideoFrame(const QByteArray& frameData)
{
    // 发送视频帧到WebRTC引擎
    // 这里需要根据WebRTC引擎的具体接口实现
    Q_UNUSED(frameData)
    
    // 示例实现：
    // if (m_webrtcEngine) {
    //     m_webrtcEngine->sendVideoData(frameData);
    // }
}

void MediaManager::sendAudioFrame(const QByteArray& audioData)
{
    // 发送音频帧到WebRTC引擎
    // 这里需要根据WebRTC引擎的具体接口实现
    Q_UNUSED(audioData)
    
    // 示例实现：
    // if (m_webrtcEngine) {
    //     m_webrtcEngine->sendAudioData(audioData);
    // }
}

// 槽函数实现
void MediaManager::onCameraStateChanged(QCamera::State state)
{
    qDebug() << "MediaManager: Camera state changed:" << state;
    
    switch (state) {
    case QCamera::ActiveState:
        qDebug() << "MediaManager: Camera is active";
        break;
    case QCamera::LoadedState:
        qDebug() << "MediaManager: Camera is loaded";
        break;
    case QCamera::UnloadedState:
        qDebug() << "MediaManager: Camera is unloaded";
        break;
    }
}

void MediaManager::onCameraError(QCamera::Error error)
{
    QString errorString;
    switch (error) {
    case QCamera::NoError:
        return; // 没有错误
    case QCamera::CameraError:
        errorString = "Camera error";
        break;
    case QCamera::InvalidRequestError:
        errorString = "Invalid request error";
        break;
    case QCamera::ServiceMissingError:
        errorString = "Service missing error";
        break;
    case QCamera::NotSupportedFeatureError:
        errorString = "Not supported feature error";
        break;
    }
    
    qWarning() << "MediaManager: Camera error:" << errorString;
    emit cameraError(errorString);
}

void MediaManager::onAudioInputStateChanged()
{
    qDebug() << "MediaManager: Audio input state changed";
}

void MediaManager::onAudioOutputStateChanged()
{
    qDebug() << "MediaManager: Audio output state changed";
}

void MediaManager::onScreenCaptureTimer()
{
    captureScreenFrame();
}

void MediaManager::onDeviceListChanged()
{
    qDebug() << "MediaManager: Device list changed, refreshing";
    refreshDeviceList();
}

// 编码和解码函数（占位符实现）
QByteArray MediaManager::encodeVideoFrame(const QVideoFrame& frame)
{
    Q_UNUSED(frame)
    // 这里应该实现视频帧编码逻辑
    // 可以使用Qt的图像处理功能或第三方编码库
    return QByteArray();
}

QByteArray MediaManager::encodeAudioFrame(const QByteArray& audioData)
{
    Q_UNUSED(audioData)
    // 这里应该实现音频帧编码逻辑
    // 可以使用Qt的音频处理功能或第三方编码库
    return QByteArray();
}

void MediaManager::decodeVideoFrame(const QByteArray& data, const QString& participantId)
{
    Q_UNUSED(data)
    Q_UNUSED(participantId)
    // 这里应该实现视频帧解码逻辑
    // 解码后的视频帧应该显示在对应参与者的视频组件中
}

void MediaManager::decodeAudioFrame(const QByteArray& data, const QString& participantId)
{
    Q_UNUSED(data)
    Q_UNUSED(participantId)
    // 这里应该实现音频帧解码逻辑
    // 解码后的音频数据应该通过音频输出播放
}