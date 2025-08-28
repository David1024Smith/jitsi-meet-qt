#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QVideoWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include "include/WebRTCEngine.h"

class WebRTCTestWindow : public QWidget
{
    Q_OBJECT

public:
    WebRTCTestWindow(QWidget* parent = nullptr) : QWidget(parent)
    {
        setWindowTitle("WebRTC Engine Test");
        setMinimumSize(800, 600);
        
        setupUI();
        setupWebRTC();
    }

private slots:
    void onStartVideo()
    {
        m_statusLabel->setText("Requesting media permissions...");
        m_webrtcEngine->requestMediaPermissions();
    }
    
    void onStopVideo()
    {
        m_webrtcEngine->stopLocalVideo();
        m_statusLabel->setText("Video stopped");
    }
    
    void onCreateOffer()
    {
        m_webrtcEngine->createPeerConnection();
        m_webrtcEngine->createOffer();
        m_statusLabel->setText("Creating offer...");
    }
    
    void onPermissionsGranted(bool video, bool audio)
    {
        QString status = QString("Permissions granted - Video: %1, Audio: %2")
                        .arg(video ? "Yes" : "No")
                        .arg(audio ? "Yes" : "No");
        m_statusLabel->setText(status);
        
        if (video) {
            m_webrtcEngine->startLocalVideo();
        }
        if (audio) {
            m_webrtcEngine->startLocalAudio();
        }
    }
    
    void onPermissionsDenied()
    {
        m_statusLabel->setText("Media permissions denied");
    }
    
    void onLocalStreamReady(QVideoWidget* videoWidget)
    {
        m_statusLabel->setText("Local video stream ready");
        
        // Add video widget to layout
        if (videoWidget && !m_videoLayout->indexOf(videoWidget) != -1) {
            m_videoLayout->addWidget(videoWidget);
            videoWidget->show();
        }
    }
    
    void onOfferCreated(const QString& sdp)
    {
        m_statusLabel->setText("SDP Offer created successfully");
        qDebug() << "Offer SDP:" << sdp.left(100) << "...";
    }
    
    void onConnectionStateChanged(WebRTCEngine::ConnectionState state)
    {
        QString stateStr;
        switch (state) {
        case WebRTCEngine::Disconnected:
            stateStr = "Disconnected";
            break;
        case WebRTCEngine::Connecting:
            stateStr = "Connecting";
            break;
        case WebRTCEngine::Connected:
            stateStr = "Connected";
            break;
        case WebRTCEngine::Failed:
            stateStr = "Failed";
            break;
        }
        
        m_statusLabel->setText("Connection state: " + stateStr);
    }
    
    void onError(const QString& message)
    {
        m_statusLabel->setText("Error: " + message);
        qWarning() << "WebRTC Error:" << message;
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Status label
        m_statusLabel = new QLabel("WebRTC Engine Test - Ready");
        mainLayout->addWidget(m_statusLabel);
        
        // Control buttons
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        
        QPushButton* startVideoBtn = new QPushButton("Start Video");
        QPushButton* stopVideoBtn = new QPushButton("Stop Video");
        QPushButton* createOfferBtn = new QPushButton("Create Offer");
        
        connect(startVideoBtn, &QPushButton::clicked, this, &WebRTCTestWindow::onStartVideo);
        connect(stopVideoBtn, &QPushButton::clicked, this, &WebRTCTestWindow::onStopVideo);
        connect(createOfferBtn, &QPushButton::clicked, this, &WebRTCTestWindow::onCreateOffer);
        
        buttonLayout->addWidget(startVideoBtn);
        buttonLayout->addWidget(stopVideoBtn);
        buttonLayout->addWidget(createOfferBtn);
        
        mainLayout->addLayout(buttonLayout);
        
        // Video container
        QWidget* videoContainer = new QWidget();
        m_videoLayout = new QHBoxLayout(videoContainer);
        mainLayout->addWidget(videoContainer);
        
        setLayout(mainLayout);
    }
    
    void setupWebRTC()
    {
        m_webrtcEngine = new WebRTCEngine(this);
        
        // Connect signals
        connect(m_webrtcEngine, &WebRTCEngine::mediaPermissionsGranted,
                this, &WebRTCTestWindow::onPermissionsGranted);
        connect(m_webrtcEngine, &WebRTCEngine::mediaPermissionsDenied,
                this, &WebRTCTestWindow::onPermissionsDenied);
        connect(m_webrtcEngine, &WebRTCEngine::localStreamReady,
                this, &WebRTCTestWindow::onLocalStreamReady);
        connect(m_webrtcEngine, &WebRTCEngine::offerCreated,
                this, &WebRTCTestWindow::onOfferCreated);
        connect(m_webrtcEngine, &WebRTCEngine::connectionStateChanged,
                this, &WebRTCTestWindow::onConnectionStateChanged);
        connect(m_webrtcEngine, &WebRTCEngine::error,
                this, &WebRTCTestWindow::onError);
        
        qDebug() << "WebRTC Engine initialized";
        qDebug() << "Available cameras:" << m_webrtcEngine->availableCameras().size();
        qDebug() << "Available audio inputs:" << m_webrtcEngine->availableAudioInputs().size();
        qDebug() << "Available audio outputs:" << m_webrtcEngine->availableAudioOutputs().size();
    }
    
    WebRTCEngine* m_webrtcEngine;
    QLabel* m_statusLabel;
    QHBoxLayout* m_videoLayout;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Starting WebRTC Engine Test";
    
    WebRTCTestWindow window;
    window.show();
    
    return app.exec();
}

#include "test_webrtc_simple.moc"