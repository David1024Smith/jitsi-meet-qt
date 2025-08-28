#include "WebRTCEngine.h"
#include <QDebug>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QUuid>
#include <QCameraInfo>
#include <QAudioDeviceInfo>
#include <QVideoFrame>
#include <QCameraViewfinder>

WebRTCEngine::WebRTCEngine(QObject *parent)
    : QObject(parent)
    , m_connectionState(Disconnected)
    , m_iceConnectionState(IceNew)
    , m_localVideoWidget(nullptr)
    , m_camera(nullptr)
    , m_audioInput(nullptr)
    , m_mediaRecorder(nullptr)
    , m_iceGatheringTimer(new QTimer(this))
    , m_connectionCheckTimer(new QTimer(this))
    , m_networkManager(new QNetworkAccessManager(this))
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
}voi
d WebRTCEngine::addLocalStream(QMediaRecorder* recorder)
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
                              .arg(qrand() % 10000 + 50000);
    hostCandidate.sdpMid = "audio";
    hostCandidate.sdpMLineIndex = 0;
    m_localIceCandidates.append(hostCandidate);
    
    // Generate server reflexive candidates
    IceCandidate srflxCandidate;
    srflxCandidate.candidate = QString("candidate:2 1 UDP 1694498815 203.0.113.100 %1 typ srflx raddr 192.168.1.100 rport %2")
                               .arg(qrand() % 10000 + 50000)
                               .arg(qrand() % 10000 + 50000);
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
        if (qrand() % 1000 < 1) { // 0.1% chance
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
    
    // Initialize camera
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (!cameras.isEmpty()) {
        m_camera = new QCamera(cameras.first(), this);
        m_localVideoWidget = new QVideoWidget();
        m_localVideoWidget->setMinimumSize(320, 240);
        
        m_camera->setViewfinder(m_localVideoWidget);
        m_camera->start();
        
        qDebug() << "Camera initialized:" << cameras.first().description();
    } else {
        qWarning() << "No cameras available";
    }
    
    // Initialize audio input
    QAudioDeviceInfo audioDevice = QAudioDeviceInfo::defaultInputDevice();
    if (!audioDevice.isNull()) {
        QAudioFormat format;
        format.setSampleRate(48000);
        format.setChannelCount(2);
        format.setSampleSize(16);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::SignedInt);
        
        if (audioDevice.isFormatSupported(format)) {
            m_audioInput = new QAudioInput(audioDevice, format, this);
            qDebug() << "Audio input initialized:" << audioDevice.deviceName();
        } else {
            qWarning() << "Audio format not supported";
        }
    } else {
        qWarning() << "No audio input devices available";
    }
}

void WebRTCEngine::cleanupLocalMedia()
{
    qDebug() << "Cleaning up local media";
    
    if (m_camera) {
        m_camera->stop();
        m_camera->deleteLater();
        m_camera = nullptr;
    }
    
    if (m_localVideoWidget) {
        m_localVideoWidget->deleteLater();
        m_localVideoWidget = nullptr;
    }
    
    if (m_audioInput) {
        m_audioInput->stop();
        m_audioInput->deleteLater();
        m_audioInput = nullptr;
    }
    
    qDebug() << "Local media cleanup completed";
}