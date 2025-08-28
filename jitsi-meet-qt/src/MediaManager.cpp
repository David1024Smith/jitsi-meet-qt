#include "MediaManager.h"
#include "WebRTCEngine.h"
#include <QDebug>
#include <QPermission>
#include <QCoreApplication>
#include <QMessageBox>
#include <QApplication>
#include <QPixmap>
#include <QBuffer>
#include <QImageWriter>
#include <QVideoFrame>
#include <QVideoSink>
#include <QRandomGenerator>
#include <QUuid>
#include <QScreen>
#include <QGuiApplication>
#include <QLabel>
#include <QVBoxLayout>

// Static constants
const qreal MediaManager::DEFAULT_VOLUME = 1.0;
const QString MediaManager::DEFAULT_VIDEO_CODEC = "VP8";
const QString MediaManager::DEFAULT_AUDIO_CODEC = "OPUS";

MediaManager::MediaManager(QObject *parent)
    : QObject(parent)
    , m_camera(nullptr)
    , m_audioInput(nullptr)
    , m_audioOutput(nullptr)
    , m_captureSession(std::make_unique<QMediaCaptureSession>())
    , m_mediaRecorder(nullptr)
    , m_localVideoWidget(nullptr)
    , m_screenShareWidget(nullptr)
    , m_videoActive(false)
    , m_audioActive(false)
    , m_screenSharingActive(false)
    , m_videoMuted(false)
    , m_audioMuted(false)
    , m_masterVolume(DEFAULT_VOLUME)
    , m_microphoneVolume(DEFAULT_VOLUME)
    , m_hasVideoPermission(false)
    , m_hasAudioPermission(false)
    , m_currentScreen(nullptr)
    , m_screenCaptureTimer(new QTimer(this))
    , m_currentVideoCodec(DEFAULT_VIDEO_CODEC)
    , m_currentAudioCodec(DEFAULT_AUDIO_CODEC)
    , m_webRTCEngine(nullptr)
{
    qDebug() << "MediaManager initializing...";
    
    // Initialize media settings with defaults
    m_mediaSettings = MediaSettings();
    
    // Setup screen capture timer
    setupScreenCaptureTimer();
    
    // Initialize codecs
    initializeCodecs();
    
    // Initialize devices
    initializeDevices();
    
    // Check initial permissions
    checkMediaPermissions();
    
    // Request permissions immediately if not granted
    if (!m_hasVideoPermission || !m_hasAudioPermission) {
        QTimer::singleShot(1000, this, &MediaManager::requestMediaPermissions);
    }
    
    qDebug() << "MediaManager initialized successfully";
}

MediaManager::~MediaManager()
{
    qDebug() << "MediaManager destroying...";
    
    stopScreenSharing();
    stopLocalVideo();
    stopLocalAudio();
    cleanupMediaResources();
    
    if (m_webRTCEngine) {
        disconnectWebRTCSignals();
    }
    
    qDebug() << "MediaManager destroyed";
}

QList<MediaManager::MediaDevice> MediaManager::availableVideoDevices() const
{
    return m_videoDevices;
}

QList<MediaManager::MediaDevice> MediaManager::availableAudioInputDevices() const
{
    return m_audioInputDevices;
}

QList<MediaManager::MediaDevice> MediaManager::availableAudioOutputDevices() const
{
    return m_audioOutputDevices;
}

QList<QScreen*> MediaManager::availableScreens() const
{
    return QApplication::screens();
}

bool MediaManager::setVideoDevice(const QString& deviceId)
{
    qDebug() << "Setting video device:" << deviceId;
    
    // Find the device
    QCameraDevice selectedDevice;
    bool deviceFound = false;
    
    for (const QCameraDevice& device : QMediaDevices::videoInputs()) {
        if (device.id() == deviceId.toUtf8()) {
            selectedDevice = device;
            deviceFound = true;
            break;
        }
    }
    
    if (!deviceFound) {
        qWarning() << "Video device not found:" << deviceId;
        return false;
    }
    
    // Stop current camera if active
    bool wasActive = m_videoActive;
    if (wasActive) {
        stopLocalVideo();
    }
    
    // Create new camera
    m_camera = std::make_unique<QCamera>(selectedDevice, this);
    connect(m_camera.get(), &QCamera::activeChanged,
            this, &MediaManager::onCameraActiveChanged);
    connect(m_camera.get(), &QCamera::errorOccurred,
            this, &MediaManager::onCameraErrorOccurred);
    
    // Update capture session
    if (m_captureSession) {
        m_captureSession->setCamera(m_camera.get());
        if (m_localVideoWidget) {
            m_captureSession->setVideoOutput(m_localVideoWidget);
        }
    }
    
    m_currentVideoDeviceId = deviceId;
    
    // Update device in list
    for (auto& device : m_videoDevices) {
        if (device.id == deviceId) {
            device.state = DeviceActive;
            emit videoDeviceChanged(device);
            break;
        }
    }
    
    // Restart video if it was active
    if (wasActive) {
        startLocalVideo();
    }
    
    qDebug() << "Video device set successfully:" << selectedDevice.description();
    return true;
}

bool MediaManager::setAudioInputDevice(const QString& deviceId)
{
    qDebug() << "Setting audio input device:" << deviceId;
    
    // Find the device
    QAudioDevice selectedDevice;
    bool deviceFound = false;
    
    for (const QAudioDevice& device : QMediaDevices::audioInputs()) {
        if (device.id() == deviceId.toUtf8()) {
            selectedDevice = device;
            deviceFound = true;
            break;
        }
    }
    
    if (!deviceFound) {
        qWarning() << "Audio input device not found:" << deviceId;
        return false;
    }
    
    // Stop current audio if active
    bool wasActive = m_audioActive;
    if (wasActive) {
        stopLocalAudio();
    }
    
    // Create new audio input
    m_audioInput = std::make_unique<QAudioInput>(selectedDevice, this);
    connect(m_audioInput.get(), &QAudioInput::deviceChanged,
            this, &MediaManager::onAudioInputStateChanged);
    
    // Update capture session
    if (m_captureSession) {
        m_captureSession->setAudioInput(m_audioInput.get());
    }
    
    m_currentAudioInputDeviceId = deviceId;
    
    // Update device in list
    for (auto& device : m_audioInputDevices) {
        if (device.id == deviceId) {
            device.state = DeviceActive;
            emit audioInputDeviceChanged(device);
            break;
        }
    }
    
    // Restart audio if it was active
    if (wasActive) {
        startLocalAudio();
    }
    
    qDebug() << "Audio input device set successfully:" << selectedDevice.description();
    return true;
}

bool MediaManager::setAudioOutputDevice(const QString& deviceId)
{
    qDebug() << "Setting audio output device:" << deviceId;
    
    // Find the device
    QAudioDevice selectedDevice;
    bool deviceFound = false;
    
    for (const QAudioDevice& device : QMediaDevices::audioOutputs()) {
        if (device.id() == deviceId.toUtf8()) {
            selectedDevice = device;
            deviceFound = true;
            break;
        }
    }
    
    if (!deviceFound) {
        qWarning() << "Audio output device not found:" << deviceId;
        return false;
    }
    
    // Create new audio output
    m_audioOutput = std::make_unique<QAudioOutput>(selectedDevice, this);
    connect(m_audioOutput.get(), &QAudioOutput::deviceChanged,
            this, &MediaManager::onAudioOutputStateChanged);
    
    // Set volume
    m_audioOutput->setVolume(m_masterVolume);
    
    m_currentAudioOutputDeviceId = deviceId;
    
    // Update device in list
    for (auto& device : m_audioOutputDevices) {
        if (device.id == deviceId) {
            device.state = DeviceActive;
            emit audioOutputDeviceChanged(device);
            break;
        }
    }
    
    qDebug() << "Audio output device set successfully:" << selectedDevice.description();
    return true;
}

std::optional<MediaManager::MediaDevice> MediaManager::currentVideoDevice() const
{
    for (const auto& device : m_videoDevices) {
        if (device.id == m_currentVideoDeviceId) {
            return device;
        }
    }
    return std::nullopt;
}

std::optional<MediaManager::MediaDevice> MediaManager::currentAudioInputDevice() const
{
    for (const auto& device : m_audioInputDevices) {
        if (device.id == m_currentAudioInputDeviceId) {
            return device;
        }
    }
    return std::nullopt;
}

std::optional<MediaManager::MediaDevice> MediaManager::currentAudioOutputDevice() const
{
    for (const auto& device : m_audioOutputDevices) {
        if (device.id == m_currentAudioOutputDeviceId) {
            return device;
        }
    }
    return std::nullopt;
}

void MediaManager::startLocalVideo()
{
    qDebug() << "Starting local video";
    
    if (!m_hasVideoPermission) {
        qWarning() << "Video permission not granted";
        emit mediaError("Video permission not granted");
        return;
    }
    
    if (m_videoActive) {
        qDebug() << "Video already active";
        return;
    }
    
    if (!m_camera) {
        initializeVideoCapture();
    }
    
    if (m_camera && !m_videoMuted) {
        m_camera->start();
        m_videoActive = true;
        emit localVideoStarted();
        
        // Notify WebRTC engine if connected
        if (m_webRTCEngine) {
            m_webRTCEngine->startLocalVideo();
        }
        
        qDebug() << "Local video started successfully";
    } else {
        emit mediaError("Failed to start video camera");
    }
}

void MediaManager::stopLocalVideo()
{
    qDebug() << "Stopping local video";
    
    if (!m_videoActive) {
        return;
    }
    
    if (m_camera) {
        m_camera->stop();
    }
    
    m_videoActive = false;
    emit localVideoStopped();
    
    // Notify WebRTC engine if connected
    if (m_webRTCEngine) {
        m_webRTCEngine->stopLocalVideo();
    }
    
    qDebug() << "Local video stopped";
}

void MediaManager::startLocalAudio()
{
    qDebug() << "Starting local audio";
    
    if (!m_hasAudioPermission) {
        qWarning() << "Audio permission not granted";
        emit mediaError("Audio permission not granted");
        return;
    }
    
    if (m_audioActive) {
        qDebug() << "Audio already active";
        return;
    }
    
    if (!m_audioInput) {
        initializeAudioCapture();
    }
    
    if (m_audioInput && !m_audioMuted) {
        // Set microphone volume
        m_audioInput->setVolume(m_microphoneVolume);
        
        m_audioActive = true;
        emit localAudioStarted();
        
        // Notify WebRTC engine if connected
        if (m_webRTCEngine) {
            m_webRTCEngine->startLocalAudio();
        }
        
        qDebug() << "Local audio started successfully";
    } else {
        emit mediaError("Failed to start audio input");
    }
}

void MediaManager::stopLocalAudio()
{
    qDebug() << "Stopping local audio";
    
    if (!m_audioActive) {
        return;
    }
    
    m_audioActive = false;
    emit localAudioStopped();
    
    // Notify WebRTC engine if connected
    if (m_webRTCEngine) {
        m_webRTCEngine->stopLocalAudio();
    }
    
    qDebug() << "Local audio stopped";
}

QVideoWidget* MediaManager::localVideoWidget() const
{
    return m_localVideoWidget;
}

void MediaManager::setLocalVideoWidget(QVideoWidget* widget)
{
    qDebug() << "Setting local video widget";
    
    m_localVideoWidget = widget;
    
    if (m_captureSession && widget) {
        m_captureSession->setVideoOutput(widget);
    }
}

bool MediaManager::isVideoActive() const
{
    return m_videoActive;
}

bool MediaManager::isAudioActive() const
{
    return m_audioActive;
}

bool MediaManager::isScreenSharingActive() const
{
    return m_screenSharingActive;
}

void MediaManager::startScreenSharing(QScreen* screen)
{
    qDebug() << "Starting screen sharing";
    
    if (m_screenSharingActive) {
        qDebug() << "Screen sharing already active";
        return;
    }
    
    // Use primary screen if none specified
    if (!screen) {
        screen = QApplication::primaryScreen();
    }
    
    if (!screen) {
        emit mediaError("No screen available for sharing");
        return;
    }
    
    m_currentScreen = screen;
    
    // Create screen share widget if not exists
    if (!m_screenShareWidget) {
        m_screenShareWidget = new QVideoWidget(nullptr);
        m_screenShareWidget->setMinimumSize(640, 480);
    }
    
    // Start screen capture timer
    m_screenCaptureTimer->start();
    m_screenSharingActive = true;
    
    emit screenSharingStarted();
    qDebug() << "Screen sharing started for screen:" << screen->name();
}

void MediaManager::stopScreenSharing()
{
    qDebug() << "Stopping screen sharing";
    
    if (!m_screenSharingActive) {
        return;
    }
    
    m_screenCaptureTimer->stop();
    m_screenSharingActive = false;
    m_currentScreen = nullptr;
    
    emit screenSharingStopped();
    qDebug() << "Screen sharing stopped";
}

QVideoWidget* MediaManager::screenShareWidget() const
{
    return m_screenShareWidget;
}

void MediaManager::setVideoCodec(const QString& codec)
{
    qDebug() << "Setting video codec:" << codec;
    
    if (m_supportedVideoCodecs.contains(codec)) {
        m_currentVideoCodec = codec;
        applyMediaSettings();
    } else {
        qWarning() << "Unsupported video codec:" << codec;
        emit encodingError(codec, "Unsupported video codec");
    }
}

void MediaManager::setAudioCodec(const QString& codec)
{
    qDebug() << "Setting audio codec:" << codec;
    
    if (m_supportedAudioCodecs.contains(codec)) {
        m_currentAudioCodec = codec;
        applyMediaSettings();
    } else {
        qWarning() << "Unsupported audio codec:" << codec;
        emit encodingError(codec, "Unsupported audio codec");
    }
}

QString MediaManager::currentVideoCodec() const
{
    return m_currentVideoCodec;
}

QString MediaManager::currentAudioCodec() const
{
    return m_currentAudioCodec;
}

void MediaManager::setMediaSettings(const MediaSettings& settings)
{
    qDebug() << "Setting media settings";
    
    if (validateMediaSettings(settings)) {
        m_mediaSettings = settings;
        applyMediaSettings();
    } else {
        qWarning() << "Invalid media settings provided";
        emit mediaError("Invalid media settings");
    }
}

MediaManager::MediaSettings MediaManager::mediaSettings() const
{
    return m_mediaSettings;
}

void MediaManager::setMasterVolume(qreal volume)
{
    qDebug() << "Setting master volume:" << volume;
    
    m_masterVolume = qBound(0.0, volume, 1.0);
    
    if (m_audioOutput) {
        m_audioOutput->setVolume(m_masterVolume);
    }
    
    emit volumeChanged(m_masterVolume);
}

qreal MediaManager::masterVolume() const
{
    return m_masterVolume;
}

void MediaManager::setMicrophoneVolume(qreal volume)
{
    qDebug() << "Setting microphone volume:" << volume;
    
    m_microphoneVolume = qBound(0.0, volume, 1.0);
    
    if (m_audioInput) {
        m_audioInput->setVolume(m_microphoneVolume);
    }
    
    emit microphoneVolumeChanged(m_microphoneVolume);
}

qreal MediaManager::microphoneVolume() const
{
    return m_microphoneVolume;
}

void MediaManager::setVideoMuted(bool muted)
{
    qDebug() << "Setting video muted:" << muted;
    
    if (m_videoMuted == muted) {
        return;
    }
    
    m_videoMuted = muted;
    
    if (muted && m_videoActive) {
        stopLocalVideo();
    } else if (!muted && m_hasVideoPermission) {
        startLocalVideo();
    }
    
    emit videoMuteChanged(muted);
}

void MediaManager::setAudioMuted(bool muted)
{
    qDebug() << "Setting audio muted:" << muted;
    
    if (m_audioMuted == muted) {
        return;
    }
    
    m_audioMuted = muted;
    
    if (muted && m_audioActive) {
        stopLocalAudio();
    } else if (!muted && m_hasAudioPermission) {
        startLocalAudio();
    }
    
    emit audioMuteChanged(muted);
}

bool MediaManager::isVideoMuted() const
{
    return m_videoMuted;
}

bool MediaManager::isAudioMuted() const
{
    return m_audioMuted;
}

void MediaManager::requestMediaPermissions()
{
    qDebug() << "Requesting media permissions";
    
    emit mediaPermissionsRequested();
    
    if (m_webRTCEngine) {
        // Delegate to WebRTC engine which handles permissions
        m_webRTCEngine->requestMediaPermissions();
    } else {
        // Handle permissions directly
        checkMediaPermissions();
        
        if (m_hasVideoPermission || m_hasAudioPermission) {
            emit mediaPermissionsGranted(m_hasVideoPermission, m_hasAudioPermission);
        } else {
            emit mediaPermissionsDenied();
        }
    }
}

bool MediaManager::hasVideoPermission() const
{
    return m_hasVideoPermission;
}

bool MediaManager::hasAudioPermission() const
{
    return m_hasAudioPermission;
}

void MediaManager::setWebRTCEngine(WebRTCEngine* engine)
{
    qDebug() << "Setting WebRTC engine";
    
    if (m_webRTCEngine) {
        disconnectWebRTCSignals();
    }
    
    m_webRTCEngine = engine;
    
    if (m_webRTCEngine) {
        connectWebRTCSignals();
    }
}

WebRTCEngine* MediaManager::webRTCEngine() const
{
    return m_webRTCEngine;
}

// Private slots implementation

void MediaManager::onDeviceListChanged()
{
    qDebug() << "Device list changed, updating...";
    updateDeviceList();
    emit deviceListChanged();
}

void MediaManager::onCameraActiveChanged(bool active)
{
    qDebug() << "Camera active changed:" << active;
    
    if (active && m_hasVideoPermission && !m_videoMuted) {
        m_videoActive = true;
        emit localVideoStarted();
    } else {
        m_videoActive = false;
        emit localVideoStopped();
    }
}

void MediaManager::onCameraErrorOccurred(QCamera::Error error)
{
    qWarning() << "Camera error occurred:" << error;
    
    QString errorMessage;
    switch (error) {
    case QCamera::NoError:
        return;
    case QCamera::CameraError:
        errorMessage = "Camera hardware error";
        break;
    default:
        errorMessage = "Unknown camera error";
        break;
    }
    
    emit mediaError(errorMessage);
    
    // Update device state
    if (!m_currentVideoDeviceId.isEmpty()) {
        emit deviceError(m_currentVideoDeviceId, errorMessage);
    }
}

void MediaManager::onAudioInputStateChanged()
{
    qDebug() << "Audio input state changed";
    // Handle audio input state changes
}

void MediaManager::onAudioOutputStateChanged()
{
    qDebug() << "Audio output state changed";
    // Handle audio output state changes
}

void MediaManager::onScreenCaptureTimer()
{
    if (m_screenSharingActive && m_currentScreen) {
        captureScreen();
    }
}

void MediaManager::onWebRTCPermissionsGranted(bool video, bool audio)
{
    qDebug() << "WebRTC permissions granted - Video:" << video << "Audio:" << audio;
    
    m_hasVideoPermission = video;
    m_hasAudioPermission = audio;
    
    emit mediaPermissionsGranted(video, audio);
    
    // Initialize media if permissions granted
    if (video) {
        initializeVideoCapture();
    }
    if (audio) {
        initializeAudioCapture();
    }
}

void MediaManager::onWebRTCPermissionsDenied()
{
    qDebug() << "WebRTC permissions denied";
    
    m_hasVideoPermission = false;
    m_hasAudioPermission = false;
    
    emit mediaPermissionsDenied();
}

// Private methods implementation

void MediaManager::initializeDevices()
{
    qDebug() << "Initializing media devices";
    
    updateDeviceList();
    
    // Set default devices if available
    if (!m_videoDevices.isEmpty()) {
        for (const auto& device : m_videoDevices) {
            if (device.isDefault) {
                setVideoDevice(device.id);
                break;
            }
        }
        // If no default found, use first available
        if (m_currentVideoDeviceId.isEmpty()) {
            setVideoDevice(m_videoDevices.first().id);
        }
    }
    
    if (!m_audioInputDevices.isEmpty()) {
        for (const auto& device : m_audioInputDevices) {
            if (device.isDefault) {
                setAudioInputDevice(device.id);
                break;
            }
        }
        if (m_currentAudioInputDeviceId.isEmpty()) {
            setAudioInputDevice(m_audioInputDevices.first().id);
        }
    }
    
    if (!m_audioOutputDevices.isEmpty()) {
        for (const auto& device : m_audioOutputDevices) {
            if (device.isDefault) {
                setAudioOutputDevice(device.id);
                break;
            }
        }
        if (m_currentAudioOutputDeviceId.isEmpty()) {
            setAudioOutputDevice(m_audioOutputDevices.first().id);
        }
    }
    
    qDebug() << "Media devices initialized";
}

void MediaManager::updateDeviceList()
{
    qDebug() << "Updating device list";
    
    // Update video devices
    m_videoDevices.clear();
    QCameraDevice defaultCamera = QMediaDevices::defaultVideoInput();
    for (const QCameraDevice& device : QMediaDevices::videoInputs()) {
        MediaDevice mediaDevice = createVideoDevice(device);
        mediaDevice.isDefault = (device.id() == defaultCamera.id());
        m_videoDevices.append(mediaDevice);
    }
    
    // Update audio input devices
    m_audioInputDevices.clear();
    QAudioDevice defaultAudioInput = QMediaDevices::defaultAudioInput();
    for (const QAudioDevice& device : QMediaDevices::audioInputs()) {
        MediaDevice mediaDevice = createAudioInputDevice(device);
        mediaDevice.isDefault = (device.id() == defaultAudioInput.id());
        m_audioInputDevices.append(mediaDevice);
    }
    
    // Update audio output devices
    m_audioOutputDevices.clear();
    QAudioDevice defaultAudioOutput = QMediaDevices::defaultAudioOutput();
    for (const QAudioDevice& device : QMediaDevices::audioOutputs()) {
        MediaDevice mediaDevice = createAudioOutputDevice(device);
        mediaDevice.isDefault = (device.id() == defaultAudioOutput.id());
        m_audioOutputDevices.append(mediaDevice);
    }
    
    refreshDeviceStates();
    
    qDebug() << "Device list updated - Video:" << m_videoDevices.size() 
             << "Audio In:" << m_audioInputDevices.size() 
             << "Audio Out:" << m_audioOutputDevices.size();
}

MediaManager::MediaDevice MediaManager::createVideoDevice(const QCameraDevice& device) const
{
    MediaDevice mediaDevice;
    mediaDevice.id = QString::fromUtf8(device.id());
    mediaDevice.name = device.description();
    mediaDevice.description = device.description();
    mediaDevice.type = Video;
    mediaDevice.state = DeviceAvailable;
    mediaDevice.isDefault = false;
    return mediaDevice;
}

MediaManager::MediaDevice MediaManager::createAudioInputDevice(const QAudioDevice& device) const
{
    MediaDevice mediaDevice;
    mediaDevice.id = QString::fromUtf8(device.id());
    mediaDevice.name = device.description();
    mediaDevice.description = device.description();
    mediaDevice.type = Audio;
    mediaDevice.state = DeviceAvailable;
    mediaDevice.isDefault = false;
    return mediaDevice;
}

MediaManager::MediaDevice MediaManager::createAudioOutputDevice(const QAudioDevice& device) const
{
    MediaDevice mediaDevice;
    mediaDevice.id = QString::fromUtf8(device.id());
    mediaDevice.name = device.description();
    mediaDevice.description = device.description();
    mediaDevice.type = Audio;
    mediaDevice.state = DeviceAvailable;
    mediaDevice.isDefault = false;
    return mediaDevice;
}

void MediaManager::refreshDeviceStates()
{
    // Update device states based on current usage
    for (auto& device : m_videoDevices) {
        if (device.id == m_currentVideoDeviceId) {
            device.state = m_videoActive ? DeviceActive : DeviceAvailable;
        }
    }
    
    for (auto& device : m_audioInputDevices) {
        if (device.id == m_currentAudioInputDeviceId) {
            device.state = m_audioActive ? DeviceActive : DeviceAvailable;
        }
    }
    
    for (auto& device : m_audioOutputDevices) {
        if (device.id == m_currentAudioOutputDeviceId) {
            device.state = DeviceActive;
        }
    }
}

void MediaManager::initializeVideoCapture()
{
    qDebug() << "Initializing video capture";
    
    if (!m_hasVideoPermission) {
        qWarning() << "Video permission not granted";
        return;
    }
    
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        qWarning() << "No cameras available";
        return;
    }
    
    // Use current device or default
    QCameraDevice cameraDevice;
    if (!m_currentVideoDeviceId.isEmpty()) {
        for (const auto& device : cameras) {
            if (QString::fromUtf8(device.id()) == m_currentVideoDeviceId) {
                cameraDevice = device;
                break;
            }
        }
    }
    
    if (cameraDevice.isNull()) {
        cameraDevice = cameras.first();
        m_currentVideoDeviceId = QString::fromUtf8(cameraDevice.id());
    }
    
    m_camera = std::make_unique<QCamera>(cameraDevice, this);
    connect(m_camera.get(), &QCamera::activeChanged,
            this, &MediaManager::onCameraActiveChanged);
    connect(m_camera.get(), &QCamera::errorOccurred,
            this, &MediaManager::onCameraErrorOccurred);
    
    if (m_captureSession) {
        m_captureSession->setCamera(m_camera.get());
        if (m_localVideoWidget) {
            m_captureSession->setVideoOutput(m_localVideoWidget);
        }
    }
    
    qDebug() << "Video capture initialized with device:" << cameraDevice.description();
}

void MediaManager::initializeAudioCapture()
{
    qDebug() << "Initializing audio capture";
    
    if (!m_hasAudioPermission) {
        qWarning() << "Audio permission not granted";
        return;
    }
    
    QList<QAudioDevice> audioInputs = QMediaDevices::audioInputs();
    if (audioInputs.isEmpty()) {
        qWarning() << "No audio input devices available";
        return;
    }
    
    // Use current device or default
    QAudioDevice audioDevice;
    if (!m_currentAudioInputDeviceId.isEmpty()) {
        for (const auto& device : audioInputs) {
            if (QString::fromUtf8(device.id()) == m_currentAudioInputDeviceId) {
                audioDevice = device;
                break;
            }
        }
    }
    
    if (audioDevice.isNull()) {
        audioDevice = audioInputs.first();
        m_currentAudioInputDeviceId = QString::fromUtf8(audioDevice.id());
    }
    
    m_audioInput = std::make_unique<QAudioInput>(audioDevice, this);
    connect(m_audioInput.get(), &QAudioInput::deviceChanged,
            this, &MediaManager::onAudioInputStateChanged);
    
    if (m_captureSession) {
        m_captureSession->setAudioInput(m_audioInput.get());
    }
    
    // Set initial volume
    m_audioInput->setVolume(m_microphoneVolume);
    
    qDebug() << "Audio capture initialized with device:" << audioDevice.description();
}

void MediaManager::initializeScreenCapture()
{
    qDebug() << "Initializing screen capture";
    
    if (!m_screenShareWidget) {
        m_screenShareWidget = new QVideoWidget(nullptr);
        m_screenShareWidget->setMinimumSize(640, 480);
    }
    
    qDebug() << "Screen capture initialized";
}

void MediaManager::cleanupMediaResources()
{
    qDebug() << "Cleaning up media resources";
    
    if (m_camera) {
        m_camera->stop();
        m_camera.reset();
    }
    
    if (m_audioInput) {
        m_audioInput.reset();
    }
    
    if (m_audioOutput) {
        m_audioOutput.reset();
    }
    
    if (m_captureSession) {
        m_captureSession->setCamera(nullptr);
        m_captureSession->setAudioInput(nullptr);
        m_captureSession->setVideoOutput(nullptr);
    }
    
    if (m_localVideoWidget) {
        m_localVideoWidget->deleteLater();
        m_localVideoWidget = nullptr;
    }
    
    if (m_screenShareWidget) {
        m_screenShareWidget->deleteLater();
        m_screenShareWidget = nullptr;
    }
    
    qDebug() << "Media resources cleanup completed";
}

void MediaManager::checkMediaPermissions()
{
    qDebug() << "Checking media permissions";
    
    // Check camera permission
    QCameraPermission cameraPermission;
    m_hasVideoPermission = (qApp->checkPermission(cameraPermission) == Qt::PermissionStatus::Granted);
    
    // Check microphone permission
    QMicrophonePermission micPermission;
    m_hasAudioPermission = (qApp->checkPermission(micPermission) == Qt::PermissionStatus::Granted);
    
    qDebug() << "Video permission:" << m_hasVideoPermission;
    qDebug() << "Audio permission:" << m_hasAudioPermission;
}

void MediaManager::handlePermissionResult(bool granted, const QString& permission)
{
    qDebug() << "Permission result for" << permission << ":" << granted;
    
    if (permission == "camera") {
        m_hasVideoPermission = granted;
        if (granted) {
            initializeVideoCapture();
        }
    } else if (permission == "microphone") {
        m_hasAudioPermission = granted;
        if (granted) {
            initializeAudioCapture();
        }
    }
}

void MediaManager::captureScreen()
{
    if (!m_currentScreen || !m_screenShareWidget) {
        return;
    }
    
    // Capture screen content
    QPixmap screenshot = m_currentScreen->grabWindow(0);
    
    // Scale to appropriate size for sharing
    QSize targetSize = m_mediaSettings.screenCaptureResolution;
    if (screenshot.size() != targetSize) {
        screenshot = screenshot.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    // Convert to video frame and display in widget
    // This is a simplified implementation - in a real scenario,
    // you would encode this as a video stream and send via WebRTC
    
    // For now, just update the widget with the screenshot
    if (m_screenShareWidget) {
        // Create a label to display the screenshot if the widget doesn't have one
        QLabel* label = m_screenShareWidget->findChild<QLabel*>();
        if (!label) {
            label = new QLabel(m_screenShareWidget);
            label->setAlignment(Qt::AlignCenter);
            label->setScaledContents(true);
            
            // Set up layout for the video widget
            QVBoxLayout* layout = new QVBoxLayout(m_screenShareWidget);
            layout->addWidget(label);
            m_screenShareWidget->setLayout(layout);
        }
        label->setPixmap(screenshot);
    }
    
    // Notify WebRTC engine if connected
    if (m_webRTCEngine) {
        m_webRTCEngine->sendScreenFrame(screenshot);
    }
    
    qDebug() << "Screen captured and processed:" << screenshot.size();
}

void MediaManager::setupScreenCaptureTimer()
{
    qDebug() << "Setting up screen capture timer";
    
    m_screenCaptureTimer->setSingleShot(false);
    m_screenCaptureTimer->setInterval(SCREEN_CAPTURE_INTERVAL);
    
    connect(m_screenCaptureTimer, &QTimer::timeout,
            this, &MediaManager::onScreenCaptureTimer);
    
    qDebug() << "Screen capture timer configured with interval:" << SCREEN_CAPTURE_INTERVAL << "ms";
}

void MediaManager::initializeCodecs()
{
    qDebug() << "Initializing supported codecs";
    
    // Initialize supported video codecs
    m_supportedVideoCodecs = supportedVideoCodecs();
    
    // Initialize supported audio codecs  
    m_supportedAudioCodecs = supportedAudioCodecs();
    
    // Set defaults if not already set
    if (!m_supportedVideoCodecs.contains(m_currentVideoCodec)) {
        if (!m_supportedVideoCodecs.isEmpty()) {
            m_currentVideoCodec = m_supportedVideoCodecs.first();
        }
    }
    
    if (!m_supportedAudioCodecs.contains(m_currentAudioCodec)) {
        if (!m_supportedAudioCodecs.isEmpty()) {
            m_currentAudioCodec = m_supportedAudioCodecs.first();
        }
    }
    
    qDebug() << "Codecs initialized - Video:" << m_supportedVideoCodecs.size() 
             << "Audio:" << m_supportedAudioCodecs.size();
}

QStringList MediaManager::supportedVideoCodecs() const
{
    QStringList codecs;
    
    // Common video codecs supported by Qt Multimedia and WebRTC
    codecs << "VP8" << "VP9" << "H264" << "AV1";
    
    // Filter based on actual Qt Multimedia support
    // This is a simplified implementation - in reality you'd query
    // the actual codec support from Qt Multimedia
    
    return codecs;
}

QStringList MediaManager::supportedAudioCodecs() const
{
    QStringList codecs;
    
    // Common audio codecs supported by Qt Multimedia and WebRTC
    codecs << "OPUS" << "G722" << "PCMU" << "PCMA" << "AAC";
    
    // Filter based on actual Qt Multimedia support
    // This is a simplified implementation - in reality you'd query
    // the actual codec support from Qt Multimedia
    
    return codecs;
}

bool MediaManager::validateMediaSettings(const MediaSettings& settings) const
{
    // Validate video settings
    if (settings.videoResolution.width() <= 0 || settings.videoResolution.height() <= 0) {
        qWarning() << "Invalid video resolution:" << settings.videoResolution;
        return false;
    }
    
    if (settings.videoFrameRate <= 0 || settings.videoFrameRate > 60) {
        qWarning() << "Invalid video frame rate:" << settings.videoFrameRate;
        return false;
    }
    
    if (settings.videoBitrate <= 0 || settings.videoBitrate > 10000) {
        qWarning() << "Invalid video bitrate:" << settings.videoBitrate;
        return false;
    }
    
    // Validate audio settings
    if (settings.audioSampleRate <= 0) {
        qWarning() << "Invalid audio sample rate:" << settings.audioSampleRate;
        return false;
    }
    
    if (settings.audioChannels <= 0 || settings.audioChannels > 8) {
        qWarning() << "Invalid audio channels:" << settings.audioChannels;
        return false;
    }
    
    if (settings.audioBitrate <= 0 || settings.audioBitrate > 512) {
        qWarning() << "Invalid audio bitrate:" << settings.audioBitrate;
        return false;
    }
    
    // Validate screen capture settings
    if (settings.screenCaptureResolution.width() <= 0 || settings.screenCaptureResolution.height() <= 0) {
        qWarning() << "Invalid screen capture resolution:" << settings.screenCaptureResolution;
        return false;
    }
    
    if (settings.screenCaptureFrameRate <= 0 || settings.screenCaptureFrameRate > 30) {
        qWarning() << "Invalid screen capture frame rate:" << settings.screenCaptureFrameRate;
        return false;
    }
    
    return true;
}

void MediaManager::applyMediaSettings()
{
    qDebug() << "Applying media settings";
    
    // Apply video settings
    if (m_camera) {
        // Set camera resolution and frame rate
        // This is a simplified implementation - Qt Multimedia
        // handles most codec settings automatically
        qDebug() << "Applied video settings - Resolution:" << m_mediaSettings.videoResolution
                 << "FPS:" << m_mediaSettings.videoFrameRate
                 << "Bitrate:" << m_mediaSettings.videoBitrate << "kbps";
    }
    
    // Apply audio settings
    if (m_audioInput) {
        // Set audio input settings
        qDebug() << "Applied audio settings - Sample rate:" << m_mediaSettings.audioSampleRate
                 << "Channels:" << m_mediaSettings.audioChannels
                 << "Bitrate:" << m_mediaSettings.audioBitrate << "kbps";
    }
    
    // Apply screen capture settings
    if (m_screenCaptureTimer) {
        int interval = 1000 / m_mediaSettings.screenCaptureFrameRate;
        m_screenCaptureTimer->setInterval(interval);
        qDebug() << "Applied screen capture settings - Resolution:" << m_mediaSettings.screenCaptureResolution
                 << "FPS:" << m_mediaSettings.screenCaptureFrameRate
                 << "Interval:" << interval << "ms";
    }
    
    // Notify WebRTC engine of settings changes
    if (m_webRTCEngine) {
        QVariantMap settingsMap;
        settingsMap["videoResolution"] = m_mediaSettings.videoResolution;
        settingsMap["videoFrameRate"] = m_mediaSettings.videoFrameRate;
        settingsMap["videoBitrate"] = m_mediaSettings.videoBitrate;
        settingsMap["audioSampleRate"] = m_mediaSettings.audioSampleRate;
        settingsMap["audioChannels"] = m_mediaSettings.audioChannels;
        settingsMap["audioBitrate"] = m_mediaSettings.audioBitrate;
        settingsMap["screenCaptureResolution"] = m_mediaSettings.screenCaptureResolution;
        settingsMap["screenCaptureFrameRate"] = m_mediaSettings.screenCaptureFrameRate;
        settingsMap["captureAudio"] = m_mediaSettings.captureAudio;
        
        m_webRTCEngine->updateMediaSettings(settingsMap);
    }
    
    qDebug() << "Media settings applied successfully";
}



void MediaManager::connectWebRTCSignals()
{
    if (!m_webRTCEngine) {
        return;
    }
    
    qDebug() << "Connecting WebRTC signals";
    
    connect(m_webRTCEngine, &WebRTCEngine::mediaPermissionsGranted,
            this, &MediaManager::onWebRTCPermissionsGranted);
    connect(m_webRTCEngine, &WebRTCEngine::mediaPermissionsDenied,
            this, &MediaManager::onWebRTCPermissionsDenied);
}

void MediaManager::disconnectWebRTCSignals()
{
    if (!m_webRTCEngine) {
        return;
    }
    
    qDebug() << "Disconnecting WebRTC signals";
    
    disconnect(m_webRTCEngine, &WebRTCEngine::mediaPermissionsGranted,
               this, &MediaManager::onWebRTCPermissionsGranted);
    disconnect(m_webRTCEngine, &WebRTCEngine::mediaPermissionsDenied,
               this, &MediaManager::onWebRTCPermissionsDenied);
}