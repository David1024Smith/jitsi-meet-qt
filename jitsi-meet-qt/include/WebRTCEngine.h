#ifndef WEBRTCENGINE_H
#define WEBRTCENGINE_H

#include <QObject>
#include <QVideoWidget>
#include <QCamera>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>

class WebRTCEngine : public QObject
{
    Q_OBJECT

public:
    enum ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Failed
    };

    enum IceConnectionState {
        IceNew,
        IceChecking,
        IceConnected,
        IceCompleted,
        IceFailed,
        IceDisconnected,
        IceClosed
    };

    struct IceCandidate {
        QString candidate;
        QString sdpMid;
        int sdpMLineIndex;
    };

    explicit WebRTCEngine(QObject *parent = nullptr);
    ~WebRTCEngine();

    // Peer connection management
    void createPeerConnection();
    void closePeerConnection();
    
    // Media stream management
    void addLocalStream(QMediaRecorder* recorder);
    void removeLocalStream();
    
    // SDP handling
    void createOffer();
    void createAnswer(const QString& offer);
    void setRemoteDescription(const QString& sdp, const QString& type);
    void setLocalDescription(const QString& sdp, const QString& type);
    
    // ICE candidate handling
    void addIceCandidate(const IceCandidate& candidate);
    void gatherIceCandidates();
    
    // State getters
    ConnectionState connectionState() const;
    IceConnectionState iceConnectionState() const;
    bool hasLocalStream() const;
    
signals:
    void localStreamReady(QVideoWidget* videoWidget);
    void remoteStreamReceived(const QString& participantId, QVideoWidget* videoWidget);
    void remoteStreamRemoved(const QString& participantId);
    void iceCandidate(const IceCandidate& candidate);
    void offerCreated(const QString& sdp);
    void answerCreated(const QString& sdp);
    void connectionStateChanged(ConnectionState state);
    void iceConnectionStateChanged(IceConnectionState state);
    void error(const QString& message);

private slots:
    void onIceGatheringTimer();
    void onConnectionCheckTimer();
    void onStunServerResponse();

private:
    void setupPeerConnection();
    void setupIceServers();
    void handleIceConnectionStateChange();
    void processRemoteStream(const QString& participantId);
    void generateLocalSdp(bool isOffer);
    void parseRemoteSdp(const QString& sdp);
    void simulateIceGathering();
    void checkConnectionHealth();
    
    // STUN/TURN server interaction
    void queryStunServers();
    void processStunResponse(const QJsonObject& response);
    
    // Media handling
    void initializeLocalMedia();
    void cleanupLocalMedia();
    
    // State management
    ConnectionState m_connectionState;
    IceConnectionState m_iceConnectionState;
    
    // Media components
    QMap<QString, QVideoWidget*> m_remoteStreams;
    QVideoWidget* m_localVideoWidget;
    QCamera* m_camera;
    QAudioInput* m_audioInput;
    QMediaRecorder* m_mediaRecorder;
    
    // ICE and connection management
    QList<IceCandidate> m_localIceCandidates;
    QList<IceCandidate> m_remoteIceCandidates;
    QTimer* m_iceGatheringTimer;
    QTimer* m_connectionCheckTimer;
    
    // Network components
    QNetworkAccessManager* m_networkManager;
    
    // SDP information
    QString m_localSdp;
    QString m_remoteSdp;
    QString m_localSdpType;
    QString m_remoteSdpType;
    
    // Configuration
    QStringList m_stunServers;
    QStringList m_turnServers;
    
    bool m_hasLocalStream;
    bool m_isOfferer;
    
    static const int ICE_GATHERING_TIMEOUT = 3000;
    static const int CONNECTION_CHECK_INTERVAL = 1000;
};

#endif // WEBRTCENGINE_H