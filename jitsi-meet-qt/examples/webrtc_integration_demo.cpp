#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QVideoWidget>
#include <QMediaRecorder>
#include <QCamera>
#include <QDebug>
#include "WebRTCEngine.h"

class WebRTCDemo : public QMainWindow
{
    Q_OBJECT

public:
    WebRTCDemo(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , m_webrtcEngine(new WebRTCEngine(this))
        , m_localVideo(new QVideoWidget(this))
        , m_remoteVideo(new QVideoWidget(this))
        , m_mediaRecorder(nullptr)
    {
        setupUI();
        setupConnections();
    }

private slots:
    void onCreateConnection()
    {
        qDebug() << "Creating peer connection";
        m_webrtcEngine->createPeerConnection();
        m_statusLabel->setText("Status: Creating connection...");
    }

    void onCreateOffer()
    {
        qDebug() << "Creating offer";
        m_webrtcEngine->createOffer();
        m_statusLabel->setText("Status: Creating offer...");
    }

    void onAddLocalStream()
    {
        qDebug() << "Adding local stream";
        
        if (!m_mediaRecorder) {
            m_mediaRecorder = new QMediaRecorder(this);
        }
        
        m_webrtcEngine->addLocalStream(m_mediaRecorder);
        m_statusLabel->setText("Status: Local stream added");
    }

    void onConnectionStateChanged(WebRTCEngine::ConnectionState state)
    {
        QString stateText;
        switch (state) {
        case WebRTCEngine::Disconnected:
            stateText = "Disconnected";
            break;
        case WebRTCEngine::Connecting:
            stateText = "Connecting";
            break;
        case WebRTCEngine::Connected:
            stateText = "Connected";
            break;
        case WebRTCEngine::Failed:
            stateText = "Failed";
            break;
        }
        
        m_statusLabel->setText(QString("Status: %1").arg(stateText));
        qDebug() << "Connection state changed to:" << stateText;
    }

    void onIceConnectionStateChanged(WebRTCEngine::IceConnectionState state)
    {
        QString stateText;
        switch (state) {
        case WebRTCEngine::IceNew:
            stateText = "New";
            break;
        case WebRTCEngine::IceChecking:
            stateText = "Checking";
            break;
        case WebRTCEngine::IceConnected:
            stateText = "Connected";
            break;
        case WebRTCEngine::IceCompleted:
            stateText = "Completed";
            break;
        case WebRTCEngine::IceFailed:
            stateText = "Failed";
            break;
        case WebRTCEngine::IceDisconnected:
            stateText = "Disconnected";
            break;
        case WebRTCEngine::IceClosed:
            stateText = "Closed";
            break;
        }
        
        m_iceStatusLabel->setText(QString("ICE: %1").arg(stateText));
        qDebug() << "ICE state changed to:" << stateText;
    }

    void onLocalStreamReady(QVideoWidget* widget)
    {
        qDebug() << "Local stream ready";
        // Replace our placeholder with the actual widget
        if (widget) {
            widget->setParent(this);
            widget->setMinimumSize(320, 240);
            // Add to layout if needed
        }
    }

    void onRemoteStreamReceived(const QString& participantId, QVideoWidget* widget)
    {
        qDebug() << "Remote stream received from:" << participantId;
        if (widget) {
            widget->setParent(this);
            widget->setMinimumSize(320, 240);
            // Add to layout if needed
        }
    }

    void onOfferCreated(const QString& sdp)
    {
        qDebug() << "Offer created, SDP length:" << sdp.length();
        m_statusLabel->setText("Status: Offer created");
        
        // In a real application, you would send this SDP to the remote peer
        // For demo purposes, we'll simulate creating an answer
        QTimer::singleShot(1000, this, [this, sdp]() {
            m_webrtcEngine->createAnswer(sdp);
        });
    }

    void onAnswerCreated(const QString& sdp)
    {
        qDebug() << "Answer created, SDP length:" << sdp.length();
        m_statusLabel->setText("Status: Answer created");
    }

    void onIceCandidate(const WebRTCEngine::IceCandidate& candidate)
    {
        qDebug() << "ICE candidate:" << candidate.candidate;
        
        // In a real application, you would send this candidate to the remote peer
        // For demo purposes, we'll add it back as a remote candidate
        QTimer::singleShot(500, this, [this, candidate]() {
            m_webrtcEngine->addIceCandidate(candidate);
        });
    }

private:
    void setupUI()
    {
        setWindowTitle("WebRTC Integration Demo");
        setMinimumSize(800, 600);

        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

        // Status labels
        m_statusLabel = new QLabel("Status: Ready", this);
        m_iceStatusLabel = new QLabel("ICE: New", this);
        mainLayout->addWidget(m_statusLabel);
        mainLayout->addWidget(m_iceStatusLabel);

        // Video layout
        QHBoxLayout* videoLayout = new QHBoxLayout();
        
        QVBoxLayout* localLayout = new QVBoxLayout();
        localLayout->addWidget(new QLabel("Local Video"));
        m_localVideo->setMinimumSize(320, 240);
        localLayout->addWidget(m_localVideo);
        
        QVBoxLayout* remoteLayout = new QVBoxLayout();
        remoteLayout->addWidget(new QLabel("Remote Video"));
        m_remoteVideo->setMinimumSize(320, 240);
        remoteLayout->addWidget(m_remoteVideo);
        
        videoLayout->addLayout(localLayout);
        videoLayout->addLayout(remoteLayout);
        mainLayout->addLayout(videoLayout);

        // Control buttons
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        
        QPushButton* createConnBtn = new QPushButton("Create Connection", this);
        QPushButton* addStreamBtn = new QPushButton("Add Local Stream", this);
        QPushButton* createOfferBtn = new QPushButton("Create Offer", this);
        
        buttonLayout->addWidget(createConnBtn);
        buttonLayout->addWidget(addStreamBtn);
        buttonLayout->addWidget(createOfferBtn);
        
        mainLayout->addLayout(buttonLayout);

        // Connect buttons
        connect(createConnBtn, &QPushButton::clicked, this, &WebRTCDemo::onCreateConnection);
        connect(addStreamBtn, &QPushButton::clicked, this, &WebRTCDemo::onAddLocalStream);
        connect(createOfferBtn, &QPushButton::clicked, this, &WebRTCDemo::onCreateOffer);
    }

    void setupConnections()
    {
        connect(m_webrtcEngine, &WebRTCEngine::connectionStateChanged,
                this, &WebRTCDemo::onConnectionStateChanged);
        connect(m_webrtcEngine, &WebRTCEngine::iceConnectionStateChanged,
                this, &WebRTCDemo::onIceConnectionStateChanged);
        connect(m_webrtcEngine, &WebRTCEngine::localStreamReady,
                this, &WebRTCDemo::onLocalStreamReady);
        connect(m_webrtcEngine, &WebRTCEngine::remoteStreamReceived,
                this, &WebRTCDemo::onRemoteStreamReceived);
        connect(m_webrtcEngine, &WebRTCEngine::offerCreated,
                this, &WebRTCDemo::onOfferCreated);
        connect(m_webrtcEngine, &WebRTCEngine::answerCreated,
                this, &WebRTCDemo::onAnswerCreated);
        connect(m_webrtcEngine, &WebRTCEngine::iceCandidate,
                this, &WebRTCDemo::onIceCandidate);
    }

private:
    WebRTCEngine* m_webrtcEngine;
    QVideoWidget* m_localVideo;
    QVideoWidget* m_remoteVideo;
    QLabel* m_statusLabel;
    QLabel* m_iceStatusLabel;
    QMediaRecorder* m_mediaRecorder;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    WebRTCDemo demo;
    demo.show();

    return app.exec();
}

#include "webrtc_integration_demo.moc"