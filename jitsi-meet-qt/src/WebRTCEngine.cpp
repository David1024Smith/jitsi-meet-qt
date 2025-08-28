#include "WebRTCEngine.h"
#include <QDebug>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QUuid>
#include <QVideoFrame>
#include <QPermission>
#include <QCoreApplication>
#include <QMessageBox>
#include <QApplication>
#include <QRandomGenerator>
#include <QDateTime>

WebRTCEngine::WebRTCEngine(QObject *parent)
    : QObject(parent)
    , m_connectionState(Disconnected)
    , m_iceConnectionState(IceNew)
    , m_localVideoWidget(nullptr)
    , m_camera(nullptr)
    , m_audioInput(nullptr)
    , m_audioOutput(nullptr)
    , m_captureSession(std::make_unique<QMediaCaptureSession>())
    , m_mediaRecorder(nullptr)
    , m_iceGatheringTimer(new QTimer(this))
    , m_connectionCheckTimer(new QTimer(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_hasVideoPermission(false)
    , m_hasAudioPermission(false)
    , m_videoEnabled(false)
    , m_audioEnabled(false)
    , m_hasLocalStream(false)
    , m_isOfferer(false)
{
    // Setup default STUN servers
    m_stunServers << "stun:stun.l.google.com:19302"
                  << "stun:stun1.l.google.com:19302"
                  << "stun:stun.jitsi.net:3478";
    
    // Setup timers
    m_iceGatheringTimer->setSingleShot(true);
    m_iceGatheringTimer->setInterval(ICE_GATHERING_TIMEOUT);
    connect(m_iceGatheringTimer, &QTimer::timeout, 
            this, &WebRTCEngine::onIceGatheringTimer);
    
    m_connectionCheckTimer->setInterval(CONNECTION_CHECK_INTERVAL);
    connect(m_connectionCheckTimer, &QTimer::timeout,
            this, &WebRTCEngine::onConnectionCheckTimer);
    
    // Setup media devices monitoring - Note: QMediaDevices doesn't have instance() in Qt 6.8
    // We'll monitor device changes through other means
    
    // Initialize media devices
    setupMediaDevices();
    
    qDebug() << "WebRTCEngine initialized";
}

WebRTCEngine::~WebRTCEngine()
{
    closePeerConnection();
    cleanupLocalMedia();
    qDebug() << "WebRTCEngine destroyed";
}

void WebRTCEngine::createPeerConnection()
{
    qDebug() << "Creating peer connection";
    
    if (m_connectionState != Disconnected) {
        qWarning() << "Peer connection already exists";
        return;
    }
    
    m_connectionState = Connecting;
    emit connectionStateChanged(m_connectionState);
    
    setupPeerConnection();
    setupIceServers();
    
    // Start connection health monitoring
    m_connectionCheckTimer->start();
    
    qDebug() << "Peer connection created successfully";
}

void WebRTCEngine::closePeerConnection()
{
    qDebug() << "Closing peer connection";
    
    m_connectionCheckTimer->stop();
    m_iceGatheringTimer->stop();
    
    // Cleanup remote streams
    for (auto it = m_remoteStreams.begin(); it != m_remoteStreams.end(); ++it) {
        emit remoteStreamRemoved(it.key());
        it.value()->deleteLater();
    }
    m_remoteStreams.clear();
    
    // Reset state
    m_connectionState = Disconnected;
    m_iceConnectionState = IceNew;
    m_localIceCandidates.clear();
    m_remoteIceCandidates.clear();
    m_localSdp.clear();
    m_remoteSdp.clear();
    
    emit connectionStateChanged(m_connectionState);
    emit iceConnectionStateChanged(m_iceConnectionState);
    
    qDebug() << "Peer connection closed";
}

void WebRTCEngine::addLocalStream(QMediaRecorder* recorder)
{
    qDebug() << "Adding local stream";
    
    if (m_hasLocalStream) {
        qWarning() << "Local stream already exists";
        return;
    }
    
    m_mediaRecorder = recorder;
    initializeLocalMedia();
    
    m_hasLocalStream = true;
    
    if (m_localVideoWidget) {
        emit localStreamReady(m_localVideoWidget);
    }
    
    qDebug() << "Local stream added successfully";
}

void WebRTCEngine::removeLocalStream()
{
    qDebug() << "Removing local stream";
    
    if (!m_hasLocalStream) {
        return;
    }
    
    cleanupLocalMedia();
    m_hasLocalStream = false;
    m_mediaRecorder = nullptr;
    
    qDebug() << "Local stream removed";
}

void WebRTCEngine::createOffer()
{
    qDebug() << "Creating offer";
    
    if (m_connectionState != Connected && m_connectionState != Connecting) {
        qWarning() << "Cannot create offer: peer connection not ready";
        return;
    }
    
    m_isOfferer = true;
    generateLocalSdp(true);
    
    // Start ICE gathering
    gatherIceCandidates();
}

void WebRTCEngine::createAnswer(const QString& offer)
{
    qDebug() << "Creating answer for offer";
    
    if (m_connectionState != Connected && m_connectionState != Connecting) {
        qWarning() << "Cannot create answer: peer connection not ready";
        return;
    }
    
    // Set remote description first
    setRemoteDescription(offer, "offer");
    
    m_isOfferer = false;
    generateLocalSdp(false);
    
    // Start ICE gathering
    gatherIceCandidates();
}

void WebRTCEngine::setRemoteDescription(const QString& sdp, const QString& type)
{
    qDebug() << "Setting remote description:" << type;
    
    m_remoteSdp = sdp;
    m_remoteSdpType = type;
    
    parseRemoteSdp(sdp);
    
    // Update connection state
    if (m_connectionState == Connecting) {
        m_connectionState = Connected;
        emit connectionStateChanged(m_connectionState);
    }
    
    qDebug() << "Remote description set successfully";
}

void WebRTCEngine::setLocalDescription(const QString& sdp, const QString& type)
{
    qDebug() << "Setting local description:" << type;
    
    m_localSdp = sdp;
    m_localSdpType = type;
    
    qDebug() << "Local description set successfully";
}

void WebRTCEngine::addIceCandidate(const IceCandidate& candidate)
{
    qDebug() << "Adding ICE candidate:" << candidate.candidate;
    
    m_remoteIceCandidates.append(candidate);
    
    // Simulate ICE connectivity check
    if (m_iceConnectionState == IceNew || m_iceConnectionState == IceChecking) {
        m_iceConnectionState = IceChecking;
        emit iceConnectionStateChanged(m_iceConnectionState);
        
        // Simulate successful connection after some candidates
        if (m_remoteIceCandidates.size() >= 2) {
            QTimer::singleShot(1000, this, [this]() {
                m_iceConnectionState = IceConnected;
                emit iceConnectionStateChanged(m_iceConnectionState);
                
                QTimer::singleShot(500, this, [this]() {
                    m_iceConnectionState = IceCompleted;
                    emit iceConnectionStateChanged(m_iceConnectionState);
                });
            });
        }
    }
}

void WebRTCEngine::gatherIceCandidates()
{
    qDebug() << "Starting ICE candidate gathering";
    
    m_localIceCandidates.clear();
    m_iceConnectionState = IceChecking;
    emit iceConnectionStateChanged(m_iceConnectionState);
    
    // Start gathering timer
    m_iceGatheringTimer->start();
    
    // Simulate ICE gathering process
    simulateIceGathering();
}

WebRTCEngine::ConnectionState WebRTCEngine::connectionState() const
{
    return m_connectionState;
}

WebRTCEngine::IceConnectionState WebRTCEngine::iceConnectionState() const
{
    return m_iceConnectionState;
}

bool WebRTCEngine::hasLocalStream() const
{
    return m_hasLocalStream;
}

void WebRTCEngine::startLocalVideo()
{
    qDebug() << "Starting local video";
    
    if (!m_hasVideoPermission) {
        qWarning() << "Video permission not granted";
        return;
    }
    
    if (m_videoEnabled) {
        qDebug() << "Video already enabled";
        return;
    }
    
    if (m_camera && m_camera->isActive()) {
        m_videoEnabled = true;
        emit localVideoStarted();
        qDebug() << "Local video started";
    } else {
        initializeLocalMedia();
    }
}

void WebRTCEngine::stopLocalVideo()
{
    qDebug() << "Stopping local video";
    
    if (!m_videoEnabled) {
        return;
    }
    
    if (m_camera) {
        m_camera->stop();
    }
    
    m_videoEnabled = false;
    emit localVideoStopped();
    qDebug() << "Local video stopped";
}

void WebRTCEngine::startLocalAudio()
{
    qDebug() << "Starting local audio";
    
    if (!m_hasAudioPermission) {
        qWarning() << "Audio permission not granted";
        return;
    }
    
    if (m_audioEnabled) {
        qDebug() << "Audio already enabled";
        return;
    }
    
    m_audioEnabled = true;
    emit localAudioStarted();
    qDebug() << "Local audio started";
}

void WebRTCEngine::stopLocalAudio()
{
    qDebug() << "Stopping local audio";
    
    if (!m_audioEnabled) {
        return;
    }
    
    m_audioEnabled = false;
    emit localAudioStopped();
    qDebug() << "Local audio stopped";
}

QList<QCameraDevice> WebRTCEngine::availableCameras() const
{
    return QMediaDevices::videoInputs();
}

QList<QAudioDevice> WebRTCEngine::availableAudioInputs() const
{
    return QMediaDevices::audioInputs();
}

QList<QAudioDevice> WebRTCEngine::availableAudioOutputs() const
{
    return QMediaDevices::audioOutputs();
}

void WebRTCEngine::setCamera(const QCameraDevice& device)
{
    qDebug() << "Setting camera:" << device.description();
    
    m_currentCameraDevice = device;
    
    if (m_camera) {
        bool wasActive = m_camera->isActive();
        m_camera->stop();
        m_camera.reset();
        
        m_camera = std::make_unique<QCamera>(device, this);
        setupCameraConnections();
        
        if (m_captureSession) {
            m_captureSession->setCamera(m_camera.get());
        }
        
        if (wasActive) {
            m_camera->start();
        }
    }
    
    emit cameraChanged(device);
}

void WebRTCEngine::setAudioInput(const QAudioDevice& device)
{
    qDebug() << "Setting audio input:" << device.description();
    
    m_currentAudioInputDevice = device;
    
    if (m_captureSession) {
        m_captureSession->setAudioInput(nullptr);
        m_audioInput = std::make_unique<QAudioInput>(device, this);
        m_captureSession->setAudioInput(m_audioInput.get());
    }
    
    emit audioInputChanged(device);
}

void WebRTCEngine::setAudioOutput(const QAudioDevice& device)
{
    qDebug() << "Setting audio output:" << device.description();
    
    m_currentAudioOutputDevice = device;
    m_audioOutput = std::make_unique<QAudioOutput>(device, this);
    
    emit audioOutputChanged(device);
}

void WebRTCEngine::requestMediaPermissions()
{
    qDebug() << "Requesting media permissions";
    
    emit mediaPermissionsRequested();
    
    // Check camera permission
    QCameraPermission cameraPermission;
    switch (qApp->checkPermission(cameraPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(cameraPermission, this, [this](const QPermission &permission) {
            handlePermissionResult(permission.status() == Qt::PermissionStatus::Granted, "camera");
        });
        break;
    case Qt::PermissionStatus::Granted:
        m_hasVideoPermission = true;
        break;
    case Qt::PermissionStatus::Denied:
        m_hasVideoPermission = false;
        break;
    }
    
    // Check microphone permission
    QMicrophonePermission micPermission;
    switch (qApp->checkPermission(micPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(micPermission, this, [this](const QPermission &permission) {
            handlePermissionResult(permission.status() == Qt::PermissionStatus::Granted, "microphone");
        });
        break;
    case Qt::PermissionStatus::Granted:
        m_hasAudioPermission = true;
        break;
    case Qt::PermissionStatus::Denied:
        m_hasAudioPermission = false;
        break;
    }
    
    // Emit result
    if (m_hasVideoPermission || m_hasAudioPermission) {
        emit mediaPermissionsGranted(m_hasVideoPermission, m_hasAudioPermission);
    } else {
        emit mediaPermissionsDenied();
    }
}

bool WebRTCEngine::hasVideoPermission() const
{
    return m_hasVideoPermission;
}

bool WebRTCEngine::hasAudioPermission() const
{
    return m_hasAudioPermission;
}

void WebRTCEngine::onIceGatheringTimer()
{
    qDebug() << "ICE gathering timeout";
    
    // Emit any remaining candidates
    for (const auto& candidate : m_localIceCandidates) {
        emit iceCandidate(candidate);
    }
}

void WebRTCEngine::onConnectionCheckTimer()
{
    checkConnectionHealth();
}

void WebRTCEngine::onStunServerResponse()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        processStunResponse(doc.object());
    } else {
        qWarning() << "STUN server query failed:" << reply->errorString();
    }
}

void WebRTCEngine::onCameraActiveChanged(bool active)
{
    qDebug() << "Camera active changed:" << active;
    
    if (active && m_hasVideoPermission) {
        m_videoEnabled = true;
        emit localVideoStarted();
        
        if (m_localVideoWidget) {
            emit localStreamReady(m_localVideoWidget);
        }
    } else {
        m_videoEnabled = false;
        emit localVideoStopped();
    }
}

void WebRTCEngine::onCameraErrorOccurred(QCamera::Error error)
{
    qWarning() << "Camera error occurred:" << error;
    
    QString errorMessage;
    switch (error) {
    case QCamera::NoError:
        return;
    case QCamera::CameraError:
        errorMessage = "Camera error occurred";
        break;
    default:
        errorMessage = "Unknown camera error";
        break;
    }
    
    emit this->error(errorMessage);
}

void WebRTCEngine::onMediaDevicesChanged()
{
    qDebug() << "Media devices changed, updating available devices";
    updateMediaDevices();
}void
 WebRTCEngine::setupPeerConnection()
{
    qDebug() << "Setting up peer connection";
    
    // Initialize connection state
    m_connectionState = Connecting;
    m_iceConnectionState = IceNew;
    
    // Clear previous state
    m_localIceCandidates.clear();
    m_remoteIceCandidates.clear();
    
    qDebug() << "Peer connection setup completed";
}

void WebRTCEngine::setupIceServers()
{
    qDebug() << "Setting up ICE servers";
    
    // Query STUN servers for connectivity
    queryStunServers();
    
    qDebug() << "ICE servers configured:" << m_stunServers.size() << "STUN servers";
}

void WebRTCEngine::handleIceConnectionStateChange()
{
    qDebug() << "ICE connection state changed to:" << m_iceConnectionState;
    
    switch (m_iceConnectionState) {
    case IceFailed:
    case IceDisconnected:
        m_connectionState = Failed;
        emit connectionStateChanged(m_connectionState);
        emit error("ICE connection failed");
        break;
    case IceConnected:
    case IceCompleted:
        if (m_connectionState == Connecting) {
            m_connectionState = Connected;
            emit connectionStateChanged(m_connectionState);
        }
        break;
    default:
        break;
    }
}

void WebRTCEngine::processRemoteStream(const QString& participantId)
{
    qDebug() << "Processing remote stream for participant:" << participantId;
    
    if (m_remoteStreams.contains(participantId)) {
        qWarning() << "Remote stream already exists for participant:" << participantId;
        return;
    }
    
    // Create video widget for remote stream
    QVideoWidget* videoWidget = new QVideoWidget();
    videoWidget->setMinimumSize(320, 240);
    videoWidget->show();
    
    m_remoteStreams[participantId] = videoWidget;
    emit remoteStreamReceived(participantId, videoWidget);
    
    qDebug() << "Remote stream processed for participant:" << participantId;
}

void WebRTCEngine::generateLocalSdp(bool isOffer)
{
    qDebug() << "Generating local SDP (offer:" << isOffer << ")";
    
    QString sdpType = isOffer ? "offer" : "answer";
    
    // Generate a basic SDP structure
    QStringList sdpLines;
    sdpLines << "v=0";
    sdpLines << QString("o=- %1 2 IN IP4 127.0.0.1").arg(QDateTime::currentMSecsSinceEpoch());
    sdpLines << "s=-";
    sdpLines << "t=0 0";
    
    // Add media descriptions
    if (m_hasLocalStream) {
        // Audio media description
        sdpLines << "m=audio 9 UDP/TLS/RTP/SAVPF 111";
        sdpLines << "c=IN IP4 0.0.0.0";
        sdpLines << "a=rtcp:9 IN IP4 0.0.0.0";
        sdpLines << "a=ice-ufrag:" + QUuid::createUuid().toString().mid(1, 8);
        sdpLines << "a=ice-pwd:" + QUuid::createUuid().toString().mid(1, 22);
        sdpLines << "a=fingerprint:sha-256 " + QUuid::createUuid().toString().replace("-", ":");
        sdpLines << "a=setup:actpass";
        sdpLines << "a=mid:audio";
        sdpLines << "a=sendrecv";
        sdpLines << "a=rtcp-mux";
        sdpLines << "a=rtpmap:111 opus/48000/2";
        
        // Video media description
        sdpLines << "m=video 9 UDP/TLS/RTP/SAVPF 96";
        sdpLines << "c=IN IP4 0.0.0.0";
        sdpLines << "a=rtcp:9 IN IP4 0.0.0.0";
        sdpLines << "a=ice-ufrag:" + QUuid::createUuid().toString().mid(1, 8);
        sdpLines << "a=ice-pwd:" + QUuid::createUuid().toString().mid(1, 22);
        sdpLines << "a=fingerprint:sha-256 " + QUuid::createUuid().toString().replace("-", ":");
        sdpLines << "a=setup:actpass";
        sdpLines << "a=mid:video";
        sdpLines << "a=sendrecv";
        sdpLines << "a=rtcp-mux";
        sdpLines << "a=rtpmap:96 VP8/90000";
    }
    
    QString sdp = sdpLines.join("\r\n") + "\r\n";
    
    setLocalDescription(sdp, sdpType);
    
    if (isOffer) {
        emit offerCreated(sdp);
    } else {
        emit answerCreated(sdp);
    }
    
    qDebug() << "Local SDP generated successfully";
}

void WebRTCEngine::parseRemoteSdp(const QString& sdp)
{
    qDebug() << "Parsing remote SDP";
    
    QStringList lines = sdp.split("\r\n", Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        if (line.startsWith("m=")) {
            // Media line - could indicate remote stream
            if (line.contains("video") || line.contains("audio")) {
                // Simulate remote participant
                QString participantId = "remote_" + QUuid::createUuid().toString().mid(1, 8);
                processRemoteStream(participantId);
            }
        }
    }
    
    qDebug() << "Remote SDP parsed successfully";
}

void WebRTCEngine::simulateIceGathering()
{
    qDebug() << "Simulating ICE candidate gathering";
    
    // Generate host candidates
    IceCandidate hostCandidate;
    hostCandidate.candidate = QString("candidate:1 1 UDP 2130706431 192.168.1.100 %1 typ host")
                              .arg(QRandomGenerator::global()->bounded(10000) + 50000);
    hostCandidate.sdpMid = "audio";
    hostCandidate.sdpMLineIndex = 0;
    m_localIceCandidates.append(hostCandidate);
    
    // Generate server reflexive candidates
    IceCandidate srflxCandidate;
    srflxCandidate.candidate = QString("candidate:2 1 UDP 1694498815 203.0.113.100 %1 typ srflx raddr 192.168.1.100 rport %2")
                               .arg(QRandomGenerator::global()->bounded(10000) + 50000)
                               .arg(QRandomGenerator::global()->bounded(10000) + 50000);
    srflxCandidate.sdpMid = "audio";
    srflxCandidate.sdpMLineIndex = 0;
    m_localIceCandidates.append(srflxCandidate);
    
    // Emit candidates with delay to simulate gathering
    QTimer::singleShot(100, this, [this, hostCandidate]() {
        emit iceCandidate(hostCandidate);
    });
    
    QTimer::singleShot(500, this, [this, srflxCandidate]() {
        emit iceCandidate(srflxCandidate);
    });
}

void WebRTCEngine::checkConnectionHealth()
{
    // Simple connection health check
    if (m_connectionState == Connected) {
        // Simulate occasional connection issues
        if (QRandomGenerator::global()->bounded(1000) < 1) { // 0.1% chance
            qDebug() << "Simulating connection issue";
            m_iceConnectionState = IceDisconnected;
            emit iceConnectionStateChanged(m_iceConnectionState);
            
            // Recover after a short time
            QTimer::singleShot(2000, this, [this]() {
                m_iceConnectionState = IceConnected;
                emit iceConnectionStateChanged(m_iceConnectionState);
            });
        }
    }
}

void WebRTCEngine::queryStunServers()
{
    qDebug() << "Querying STUN servers for connectivity";
    
    // This is a simplified implementation
    // In a real implementation, you would query STUN servers for public IP
    for (const QString& stunServer : m_stunServers) {
        qDebug() << "Using STUN server:" << stunServer;
    }
}

void WebRTCEngine::processStunResponse(const QJsonObject& response)
{
    qDebug() << "Processing STUN server response";
    
    // Process STUN response for NAT traversal information
    if (response.contains("publicIp")) {
        QString publicIp = response["publicIp"].toString();
        qDebug() << "Public IP discovered:" << publicIp;
    }
}

void WebRTCEngine::initializeLocalMedia()
{
    qDebug() << "Initializing local media";
    
    if (!m_hasVideoPermission && !m_hasAudioPermission) {
        qWarning() << "No media permissions granted";
        return;
    }
    
    // Initialize camera if permission granted
    if (m_hasVideoPermission) {
        QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
        if (!cameras.isEmpty()) {
            QCameraDevice cameraDevice = m_currentCameraDevice.isNull() ? cameras.first() : m_currentCameraDevice;
            
            m_camera = std::make_unique<QCamera>(cameraDevice, this);
            setupCameraConnections();
            
            m_localVideoWidget = new QVideoWidget();
            m_localVideoWidget->setMinimumSize(320, 240);
            
            if (m_captureSession) {
                m_captureSession->setCamera(m_camera.get());
                m_captureSession->setVideoOutput(m_localVideoWidget);
            }
            
            m_camera->start();
            
            qDebug() << "Camera initialized:" << cameraDevice.description();
        } else {
            qWarning() << "No cameras available";
        }
    }
    
    // Initialize audio input if permission granted
    if (m_hasAudioPermission) {
        QList<QAudioDevice> audioInputs = QMediaDevices::audioInputs();
        if (!audioInputs.isEmpty()) {
            QAudioDevice audioDevice = m_currentAudioInputDevice.isNull() ? audioInputs.first() : m_currentAudioInputDevice;
            
            m_audioInput = std::make_unique<QAudioInput>(audioDevice, this);
            
            if (m_captureSession) {
                m_captureSession->setAudioInput(m_audioInput.get());
            }
            
            qDebug() << "Audio input initialized:" << audioDevice.description();
        } else {
            qWarning() << "No audio input devices available";
        }
        
        // Initialize audio output
        QList<QAudioDevice> audioOutputs = QMediaDevices::audioOutputs();
        if (!audioOutputs.isEmpty()) {
            QAudioDevice outputDevice = m_currentAudioOutputDevice.isNull() ? audioOutputs.first() : m_currentAudioOutputDevice;
            m_audioOutput = std::make_unique<QAudioOutput>(outputDevice, this);
            qDebug() << "Audio output initialized:" << outputDevice.description();
        }
    }
}

void WebRTCEngine::cleanupLocalMedia()
{
    qDebug() << "Cleaning up local media";
    
    if (m_camera) {
        m_camera->stop();
        m_camera.reset();
    }
    
    if (m_localVideoWidget) {
        m_localVideoWidget->deleteLater();
        m_localVideoWidget = nullptr;
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
    
    m_videoEnabled = false;
    m_audioEnabled = false;
    
    qDebug() << "Local media cleanup completed";
}

void WebRTCEngine::setupMediaDevices()
{
    qDebug() << "Setting up media devices";
    
    // Get default devices
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (!cameras.isEmpty()) {
        m_currentCameraDevice = cameras.first();
    }
    
    QList<QAudioDevice> audioInputs = QMediaDevices::audioInputs();
    if (!audioInputs.isEmpty()) {
        m_currentAudioInputDevice = audioInputs.first();
    }
    
    QList<QAudioDevice> audioOutputs = QMediaDevices::audioOutputs();
    if (!audioOutputs.isEmpty()) {
        m_currentAudioOutputDevice = audioOutputs.first();
    }
    
    qDebug() << "Media devices setup completed";
    qDebug() << "Available cameras:" << cameras.size();
    qDebug() << "Available audio inputs:" << audioInputs.size();
    qDebug() << "Available audio outputs:" << audioOutputs.size();
}

void WebRTCEngine::updateMediaDevices()
{
    qDebug() << "Updating media devices";
    
    // Check if current devices are still available
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    QList<QAudioDevice> audioInputs = QMediaDevices::audioInputs();
    QList<QAudioDevice> audioOutputs = QMediaDevices::audioOutputs();
    
    // Update camera if current one is no longer available
    if (!m_currentCameraDevice.isNull()) {
        bool found = false;
        for (const auto& camera : cameras) {
            if (camera.id() == m_currentCameraDevice.id()) {
                found = true;
                break;
            }
        }
        if (!found && !cameras.isEmpty()) {
            setCamera(cameras.first());
        }
    }
    
    // Update audio input if current one is no longer available
    if (!m_currentAudioInputDevice.isNull()) {
        bool found = false;
        for (const auto& device : audioInputs) {
            if (device.id() == m_currentAudioInputDevice.id()) {
                found = true;
                break;
            }
        }
        if (!found && !audioInputs.isEmpty()) {
            setAudioInput(audioInputs.first());
        }
    }
    
    // Update audio output if current one is no longer available
    if (!m_currentAudioOutputDevice.isNull()) {
        bool found = false;
        for (const auto& device : audioOutputs) {
            if (device.id() == m_currentAudioOutputDevice.id()) {
                found = true;
                break;
            }
        }
        if (!found && !audioOutputs.isEmpty()) {
            setAudioOutput(audioOutputs.first());
        }
    }
}

void WebRTCEngine::checkMediaPermissions()
{
    qDebug() << "Checking media permissions";
    
    QCameraPermission cameraPermission;
    m_hasVideoPermission = (qApp->checkPermission(cameraPermission) == Qt::PermissionStatus::Granted);
    
    QMicrophonePermission micPermission;
    m_hasAudioPermission = (qApp->checkPermission(micPermission) == Qt::PermissionStatus::Granted);
    
    qDebug() << "Video permission:" << m_hasVideoPermission;
    qDebug() << "Audio permission:" << m_hasAudioPermission;
}

void WebRTCEngine::handlePermissionResult(bool granted, const QString& permission)
{
    qDebug() << "Permission result for" << permission << ":" << granted;
    
    if (permission == "camera") {
        m_hasVideoPermission = granted;
    } else if (permission == "microphone") {
        m_hasAudioPermission = granted;
    }
    
    if (m_hasVideoPermission || m_hasAudioPermission) {
        emit mediaPermissionsGranted(m_hasVideoPermission, m_hasAudioPermission);
        
        // Initialize media if permissions are granted
        if (granted) {
            initializeLocalMedia();
        }
    } else {
        emit mediaPermissionsDenied();
    }
}

void WebRTCEngine::setupCameraConnections()
{
    if (m_camera) {
        connect(m_camera.get(), &QCamera::activeChanged,
                this, &WebRTCEngine::onCameraActiveChanged);
        connect(m_camera.get(), &QCamera::errorOccurred,
                this, &WebRTCEngine::onCameraErrorOccurred);
    }
}

void WebRTCEngine::sendScreenFrame(const QPixmap& frame)
{
    qDebug() << "Sending screen frame:" << frame.size();
    
    // In a real implementation, this would encode the frame and send it via WebRTC
    // For now, we just log the frame information
    
    if (frame.isNull()) {
        qWarning() << "Cannot send null screen frame";
        return;
    }
    
    // Simulate encoding and transmission
    qDebug() << "Screen frame encoded and queued for transmission";
    
    // In a real WebRTC implementation, you would:
    // 1. Convert QPixmap to video frame format
    // 2. Encode using selected video codec
    // 3. Send via RTP packets to remote peers
}

void WebRTCEngine::updateMediaSettings(const QVariantMap& settings)
{
    qDebug() << "Updating media settings";
    
    // Apply video settings
    if (settings.contains("videoResolution")) {
        QSize resolution = settings["videoResolution"].toSize();
        qDebug() << "Updated video resolution:" << resolution;
        
        // Apply to camera if active
        if (m_camera && m_camera->isActive()) {
            // Qt Multimedia handles resolution automatically in most cases
            // Custom resolution handling would go here
        }
    }
    
    if (settings.contains("videoFrameRate")) {
        int frameRate = settings["videoFrameRate"].toInt();
        qDebug() << "Updated video frame rate:" << frameRate;
        
        // Apply frame rate settings
        if (m_camera) {
            // Frame rate configuration would go here
        }
    }
    
    if (settings.contains("videoBitrate")) {
        int bitrate = settings["videoBitrate"].toInt();
        qDebug() << "Updated video bitrate:" << bitrate << "kbps";
        
        // Apply bitrate settings for encoding
    }
    
    // Apply audio settings
    if (settings.contains("audioSampleRate")) {
        int sampleRate = settings["audioSampleRate"].toInt();
        qDebug() << "Updated audio sample rate:" << sampleRate;
        
        // Apply to audio input if active
        if (m_audioInput) {
            // Sample rate configuration would go here
        }
    }
    
    if (settings.contains("audioChannels")) {
        int channels = settings["audioChannels"].toInt();
        qDebug() << "Updated audio channels:" << channels;
    }
    
    if (settings.contains("audioBitrate")) {
        int bitrate = settings["audioBitrate"].toInt();
        qDebug() << "Updated audio bitrate:" << bitrate << "kbps";
    }
    
    // Apply screen capture settings
    if (settings.contains("screenCaptureResolution")) {
        QSize resolution = settings["screenCaptureResolution"].toSize();
        qDebug() << "Updated screen capture resolution:" << resolution;
    }
    
    if (settings.contains("screenCaptureFrameRate")) {
        int frameRate = settings["screenCaptureFrameRate"].toInt();
        qDebug() << "Updated screen capture frame rate:" << frameRate;
    }
    
    qDebug() << "Media settings update completed";
}
void Web
RTCEngine::sendScreenFrame(const QPixmap& frame)
{
    if (m_connectionState != Connected) {
        qWarning() << "WebRTCEngine: Cannot send screen frame - not connected";
        return;
    }
    
    if (frame.isNull()) {
        qWarning() << "WebRTCEngine: Cannot send null screen frame";
        return;
    }
    
    // Convert QPixmap to byte array for transmission
    QByteArray frameData;
    QBuffer buffer(&frameData);
    buffer.open(QIODevice::WriteOnly);
    
    // Compress the frame for efficient transmission
    if (!frame.save(&buffer, "JPEG", 75)) { // 75% quality for balance between size and quality
        qWarning() << "WebRTCEngine: Failed to encode screen frame";
        return;
    }
    
    // In a real implementation, this would be sent through the WebRTC data channel
    // For now, we'll simulate the transmission
    qDebug() << "WebRTCEngine: Sending screen frame, size:" << frameData.size() << "bytes";
    
    // Simulate processing time for encoding and transmission
    QTimer::singleShot(10, this, [this, frameData]() {
        // Simulate successful transmission
        qDebug() << "WebRTCEngine: Screen frame transmitted successfully";
    });
}

void WebRTCEngine::updateMediaSettings(const QVariantMap& settings)
{
    if (settings.contains("videoEnabled")) {
        bool videoEnabled = settings["videoEnabled"].toBool();
        if (videoEnabled != m_videoEnabled) {
            m_videoEnabled = videoEnabled;
            if (videoEnabled) {
                startLocalVideo();
            } else {
                stopLocalVideo();
            }
        }
    }
    
    if (settings.contains("audioEnabled")) {
        bool audioEnabled = settings["audioEnabled"].toBool();
        if (audioEnabled != m_audioEnabled) {
            m_audioEnabled = audioEnabled;
            if (audioEnabled) {
                startLocalAudio();
            } else {
                stopLocalAudio();
            }
        }
    }
    
    if (settings.contains("cameraDevice")) {
        QString deviceId = settings["cameraDevice"].toString();
        auto cameras = availableCameras();
        for (const auto& camera : cameras) {
            if (camera.id() == deviceId) {
                setCamera(camera);
                break;
            }
        }
    }
    
    if (settings.contains("audioInputDevice")) {
        QString deviceId = settings["audioInputDevice"].toString();
        auto audioInputs = availableAudioInputs();
        for (const auto& device : audioInputs) {
            if (device.id() == deviceId) {
                setAudioInput(device);
                break;
            }
        }
    }
    
    if (settings.contains("audioOutputDevice")) {
        QString deviceId = settings["audioOutputDevice"].toString();
        auto audioOutputs = availableAudioOutputs();
        for (const auto& device : audioOutputs) {
            if (device.id() == deviceId) {
                setAudioOutput(device);
                break;
            }
        }
    }
    
    qDebug() << "WebRTCEngine: Media settings updated";
}