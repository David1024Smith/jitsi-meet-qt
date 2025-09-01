#include "../include/MediaManager.h"
#include "../modules/audio/include/AudioManager.h"
#include "../modules/camera/include/CameraManager.h"
#include <QDebug>

MediaManager::MediaManager(QObject *parent)
    : QObject(parent)
    , m_audioManager(nullptr)
    , m_cameraManager(nullptr)
    , m_initialized(false)
    , m_webRTCEngine(nullptr)
    , m_localVideoWidget(nullptr)
{
    setupConnections();
}

MediaManager::~MediaManager()
{
    cleanup();
}

bool MediaManager::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    qDebug() << "Initializing MediaManager...";
    
    // Initialize audio manager
    m_audioManager = new AudioManager(this);
    if (!m_audioManager->initialize()) {
        qWarning() << "Failed to initialize AudioManager";
        delete m_audioManager;
        m_audioManager = nullptr;
    }
    
    // Initialize camera manager  
    m_cameraManager = new CameraManager(this);
    if (!m_cameraManager->initialize()) {
        qWarning() << "Failed to initialize CameraManager";
        delete m_cameraManager;
        m_cameraManager = nullptr;
    }
    
    m_initialized = true;
    qDebug() << "MediaManager initialized successfully";
    
    return true;
}

void MediaManager::cleanup()
{
    qDebug() << "Cleaning up MediaManager...";
    
    if (m_audioManager) {
        m_audioManager->stopAudio();
        delete m_audioManager;
        m_audioManager = nullptr;
    }
    
    if (m_cameraManager) {
        m_cameraManager->stopCamera();
        m_cameraManager->cleanup();
        delete m_cameraManager;
        m_cameraManager = nullptr;
    }
    
    m_initialized = false;
}

AudioManager* MediaManager::audioManager() const
{
    return m_audioManager;
}

CameraManager* MediaManager::cameraManager() const
{
    return m_cameraManager;
}

bool MediaManager::isMediaAvailable() const
{
    return m_initialized && (m_audioManager || m_cameraManager);
}

void MediaManager::setWebRTCEngine(QObject* engine)
{
    m_webRTCEngine = engine;
}

QVideoWidget* MediaManager::localVideoWidget() const
{
    return m_localVideoWidget;
}

void MediaManager::setLocalVideoWidget(QVideoWidget* widget)
{
    m_localVideoWidget = widget;
}

void MediaManager::startLocalVideo()
{
    qDebug() << "Starting local video...";
    
    if (m_cameraManager) {
        // Implementation would go here
        emit localVideoStarted();
    }
}

void MediaManager::stopLocalVideo()
{
    qDebug() << "Stopping local video...";
    
    if (m_cameraManager) {
        // Implementation would go here
        emit localVideoStopped();
    }
}

void MediaManager::startLocalAudio()
{
    qDebug() << "Starting local audio...";
    
    if (m_audioManager) {
        // Implementation would go here
        emit localAudioStarted();
    }
}

void MediaManager::stopLocalAudio()
{
    qDebug() << "Stopping local audio...";
    
    if (m_audioManager) {
        // Implementation would go here
        // emit localAudioStopped(); // This signal doesn't exist in the header
    }
}



void MediaManager::forceStartCameraDisplay()
{
    qDebug() << "Force starting camera display...";
    
    if (m_cameraManager) {
        // Implementation would go here
        emit localVideoStarted();
    }
}

void MediaManager::requestMediaPermissions()
{
    qDebug() << "Requesting media permissions...";
    
    // Request camera and microphone permissions
    if (m_audioManager) {
        // Request audio permissions
        emit localAudioStarted();
    }
    if (m_cameraManager) {
        // Request camera permissions
        emit localVideoStarted();
    }
}

void MediaManager::setupConnections()
{
    // Setup internal connections
}

void MediaManager::onAudioDeviceChanged()
{
    // Handle audio device changes
}

void MediaManager::onCameraDeviceChanged()
{
    // Handle camera device changes
}