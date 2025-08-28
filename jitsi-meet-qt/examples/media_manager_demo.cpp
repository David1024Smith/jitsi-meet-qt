#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QVideoWidget>
#include <QDebug>
#include <QMessageBox>
#include "MediaManager.h"
#include "WebRTCEngine.h"

/**
 * @brief MediaManager演示应用程序
 * 
 * 这个演示程序展示了如何使用MediaManager类来：
 * - 枚举和选择音视频设备
 * - 控制本地音视频流
 * - 管理屏幕共享
 * - 调节音量和静音设置
 * - 配置媒体质量
 */
class MediaManagerDemo : public QMainWindow
{
    Q_OBJECT

public:
    MediaManagerDemo(QWidget *parent = nullptr);
    ~MediaManagerDemo();

private slots:
    // 设备选择
    void onCameraChanged();
    void onMicrophoneChanged();
    void onSpeakerChanged();
    void onScreenChanged();
    
    // 媒体控制
    void onStartVideo();
    void onStopVideo();
    void onStartAudio();
    void onStopAudio();
    void onStartScreenShare();
    void onStopScreenShare();
    
    // 音量控制
    void onMicVolumeChanged(int volume);
    void onSpeakerVolumeChanged(int volume);
    void onMicMuteToggled(bool muted);
    void onSpeakerMuteToggled(bool muted);
    
    // 质量设置
    void onQualityChanged();
    
    // MediaManager事件
    void onLocalVideoStarted();
    void onLocalVideoStopped();
    void onLocalAudioStarted();
    void onLocalAudioStopped();
    void onScreenShareStarted();
    void onScreenShareStopped();
    void onDeviceListChanged();
    void onCameraError(const QString& error);
    void onMicrophoneError(const QString& error);
    void onSpeakerError(const QString& error);
    void onScreenCaptureError(const QString& error);

private:
    void setupUI();
    void setupDeviceControls();
    void setupMediaControls();
    void setupVolumeControls();
    void setupQualityControls();
    void setupVideoDisplay();
    void connectSignals();
    void refreshDeviceLists();
    void updateStatus();
    
    // 核心组件
    MediaManager* m_mediaManager;
    WebRTCEngine* m_webrtcEngine;
    
    // UI组件
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    
    // 设备选择组件
    QGroupBox* m_deviceGroup;
    QComboBox* m_cameraCombo;
    QComboBox* m_microphoneCombo;
    QComboBox* m_speakerCombo;
    QComboBox* m_screenCombo;
    QPushButton* m_refreshDevicesBtn;
    
    // 媒体控制组件
    QGroupBox* m_mediaGroup;
    QPushButton* m_startVideoBtn;
    QPushButton* m_stopVideoBtn;
    QPushButton* m_startAudioBtn;
    QPushButton* m_stopAudioBtn;
    QPushButton* m_startScreenShareBtn;
    QPushButton* m_stopScreenShareBtn;
    
    // 音量控制组件
    QGroupBox* m_volumeGroup;
    QSlider* m_micVolumeSlider;
    QSlider* m_speakerVolumeSlider;
    QCheckBox* m_micMuteCheckBox;
    QCheckBox* m_speakerMuteCheckBox;
    QLabel* m_micVolumeLabel;
    QLabel* m_speakerVolumeLabel;
    
    // 质量设置组件
    QGroupBox* m_qualityGroup;
    QComboBox* m_resolutionCombo;
    QComboBox* m_fpsCombo;
    QComboBox* m_bitrateCombo;
    
    // 视频显示组件
    QGroupBox* m_videoGroup;
    QVideoWidget* m_localVideoWidget;
    QVideoWidget* m_screenShareWidget;
    
    // 状态显示
    QLabel* m_statusLabel;
};

MediaManagerDemo::MediaManagerDemo(QWidget *parent)
    : QMainWindow(parent)
    , m_mediaManager(nullptr)
    , m_webrtcEngine(nullptr)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
{
    setWindowTitle("MediaManager Demo - Jitsi Meet Qt");
    setMinimumSize(800, 600);
    
    // 创建MediaManager和WebRTCEngine
    m_mediaManager = new MediaManager(this);
    m_webrtcEngine = new WebRTCEngine(this);
    
    // 连接MediaManager和WebRTCEngine
    m_mediaManager->setWebRTCEngine(m_webrtcEngine);
    
    // 设置UI
    setupUI();
    connectSignals();
    refreshDeviceLists();
    updateStatus();
    
    qDebug() << "MediaManagerDemo: Demo application started";
}

MediaManagerDemo::~MediaManagerDemo()
{
    qDebug() << "MediaManagerDemo: Demo application closing";
}

void MediaManagerDemo::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    
    setupDeviceControls();
    setupMediaControls();
    setupVolumeControls();
    setupQualityControls();
    setupVideoDisplay();
    
    // 状态标签
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; border: 1px solid #ccc; }");
    m_mainLayout->addWidget(m_statusLabel);
}

void MediaManagerDemo::setupDeviceControls()
{
    m_deviceGroup = new QGroupBox("Device Selection", this);
    QGridLayout* layout = new QGridLayout(m_deviceGroup);
    
    // 摄像头选择
    layout->addWidget(new QLabel("Camera:"), 0, 0);
    m_cameraCombo = new QComboBox(this);
    layout->addWidget(m_cameraCombo, 0, 1);
    
    // 麦克风选择
    layout->addWidget(new QLabel("Microphone:"), 1, 0);
    m_microphoneCombo = new QComboBox(this);
    layout->addWidget(m_microphoneCombo, 1, 1);
    
    // 扬声器选择
    layout->addWidget(new QLabel("Speaker:"), 2, 0);
    m_speakerCombo = new QComboBox(this);
    layout->addWidget(m_speakerCombo, 2, 1);
    
    // 屏幕选择
    layout->addWidget(new QLabel("Screen:"), 3, 0);
    m_screenCombo = new QComboBox(this);
    layout->addWidget(m_screenCombo, 3, 1);
    
    // 刷新设备按钮
    m_refreshDevicesBtn = new QPushButton("Refresh Devices", this);
    layout->addWidget(m_refreshDevicesBtn, 4, 0, 1, 2);
    
    m_mainLayout->addWidget(m_deviceGroup);
}

void MediaManagerDemo::setupMediaControls()
{
    m_mediaGroup = new QGroupBox("Media Controls", this);
    QGridLayout* layout = new QGridLayout(m_mediaGroup);
    
    // 视频控制
    m_startVideoBtn = new QPushButton("Start Video", this);
    m_stopVideoBtn = new QPushButton("Stop Video", this);
    m_stopVideoBtn->setEnabled(false);
    layout->addWidget(m_startVideoBtn, 0, 0);
    layout->addWidget(m_stopVideoBtn, 0, 1);
    
    // 音频控制
    m_startAudioBtn = new QPushButton("Start Audio", this);
    m_stopAudioBtn = new QPushButton("Stop Audio", this);
    m_stopAudioBtn->setEnabled(false);
    layout->addWidget(m_startAudioBtn, 1, 0);
    layout->addWidget(m_stopAudioBtn, 1, 1);
    
    // 屏幕共享控制
    m_startScreenShareBtn = new QPushButton("Start Screen Share", this);
    m_stopScreenShareBtn = new QPushButton("Stop Screen Share", this);
    m_stopScreenShareBtn->setEnabled(false);
    layout->addWidget(m_startScreenShareBtn, 2, 0);
    layout->addWidget(m_stopScreenShareBtn, 2, 1);
    
    m_mainLayout->addWidget(m_mediaGroup);
}

void MediaManagerDemo::setupVolumeControls()
{
    m_volumeGroup = new QGroupBox("Volume Controls", this);
    QGridLayout* layout = new QGridLayout(m_volumeGroup);
    
    // 麦克风音量
    layout->addWidget(new QLabel("Microphone Volume:"), 0, 0);
    m_micVolumeSlider = new QSlider(Qt::Horizontal, this);
    m_micVolumeSlider->setRange(0, 100);
    m_micVolumeSlider->setValue(80);
    layout->addWidget(m_micVolumeSlider, 0, 1);
    m_micVolumeLabel = new QLabel("80", this);
    layout->addWidget(m_micVolumeLabel, 0, 2);
    m_micMuteCheckBox = new QCheckBox("Mute", this);
    layout->addWidget(m_micMuteCheckBox, 0, 3);
    
    // 扬声器音量
    layout->addWidget(new QLabel("Speaker Volume:"), 1, 0);
    m_speakerVolumeSlider = new QSlider(Qt::Horizontal, this);
    m_speakerVolumeSlider->setRange(0, 100);
    m_speakerVolumeSlider->setValue(80);
    layout->addWidget(m_speakerVolumeSlider, 1, 1);
    m_speakerVolumeLabel = new QLabel("80", this);
    layout->addWidget(m_speakerVolumeLabel, 1, 2);
    m_speakerMuteCheckBox = new QCheckBox("Mute", this);
    layout->addWidget(m_speakerMuteCheckBox, 1, 3);
    
    m_mainLayout->addWidget(m_volumeGroup);
}

void MediaManagerDemo::setupQualityControls()
{
    m_qualityGroup = new QGroupBox("Quality Settings", this);
    QGridLayout* layout = new QGridLayout(m_qualityGroup);
    
    // 分辨率设置
    layout->addWidget(new QLabel("Resolution:"), 0, 0);
    m_resolutionCombo = new QComboBox(this);
    m_resolutionCombo->addItem("640x480", QSize(640, 480));
    m_resolutionCombo->addItem("1280x720", QSize(1280, 720));
    m_resolutionCombo->addItem("1920x1080", QSize(1920, 1080));
    layout->addWidget(m_resolutionCombo, 0, 1);
    
    // 帧率设置
    layout->addWidget(new QLabel("Frame Rate:"), 1, 0);
    m_fpsCombo = new QComboBox(this);
    m_fpsCombo->addItem("15 fps", 15);
    m_fpsCombo->addItem("30 fps", 30);
    m_fpsCombo->addItem("60 fps", 60);
    m_fpsCombo->setCurrentIndex(1); // 默认30fps
    layout->addWidget(m_fpsCombo, 1, 1);
    
    // 比特率设置
    layout->addWidget(new QLabel("Bitrate:"), 2, 0);
    m_bitrateCombo = new QComboBox(this);
    m_bitrateCombo->addItem("500 kbps", 500000);
    m_bitrateCombo->addItem("1 Mbps", 1000000);
    m_bitrateCombo->addItem("2 Mbps", 2000000);
    m_bitrateCombo->addItem("5 Mbps", 5000000);
    m_bitrateCombo->setCurrentIndex(1); // 默认1Mbps
    layout->addWidget(m_bitrateCombo, 2, 1);
    
    m_mainLayout->addWidget(m_qualityGroup);
}

void MediaManagerDemo::setupVideoDisplay()
{
    m_videoGroup = new QGroupBox("Video Display", this);
    QHBoxLayout* layout = new QHBoxLayout(m_videoGroup);
    
    // 本地视频显示
    QVBoxLayout* localLayout = new QVBoxLayout();
    localLayout->addWidget(new QLabel("Local Video"));
    m_localVideoWidget = new QVideoWidget(this);
    m_localVideoWidget->setMinimumSize(320, 240);
    m_localVideoWidget->setStyleSheet("QVideoWidget { background-color: black; border: 1px solid #ccc; }");
    localLayout->addWidget(m_localVideoWidget);
    layout->addLayout(localLayout);
    
    // 屏幕共享显示
    QVBoxLayout* screenLayout = new QVBoxLayout();
    screenLayout->addWidget(new QLabel("Screen Share"));
    m_screenShareWidget = new QVideoWidget(this);
    m_screenShareWidget->setMinimumSize(320, 240);
    m_screenShareWidget->setStyleSheet("QVideoWidget { background-color: black; border: 1px solid #ccc; }");
    screenLayout->addWidget(m_screenShareWidget);
    layout->addLayout(screenLayout);
    
    m_mainLayout->addWidget(m_videoGroup);
}

void MediaManagerDemo::connectSignals()
{
    // 设备选择信号
    connect(m_cameraCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MediaManagerDemo::onCameraChanged);
    connect(m_microphoneCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MediaManagerDemo::onMicrophoneChanged);
    connect(m_speakerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MediaManagerDemo::onSpeakerChanged);
    connect(m_screenCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MediaManagerDemo::onScreenChanged);
    connect(m_refreshDevicesBtn, &QPushButton::clicked,
            this, &MediaManagerDemo::refreshDeviceLists);
    
    // 媒体控制信号
    connect(m_startVideoBtn, &QPushButton::clicked, this, &MediaManagerDemo::onStartVideo);
    connect(m_stopVideoBtn, &QPushButton::clicked, this, &MediaManagerDemo::onStopVideo);
    connect(m_startAudioBtn, &QPushButton::clicked, this, &MediaManagerDemo::onStartAudio);
    connect(m_stopAudioBtn, &QPushButton::clicked, this, &MediaManagerDemo::onStopAudio);
    connect(m_startScreenShareBtn, &QPushButton::clicked, this, &MediaManagerDemo::onStartScreenShare);
    connect(m_stopScreenShareBtn, &QPushButton::clicked, this, &MediaManagerDemo::onStopScreenShare);
    
    // 音量控制信号
    connect(m_micVolumeSlider, &QSlider::valueChanged, this, &MediaManagerDemo::onMicVolumeChanged);
    connect(m_speakerVolumeSlider, &QSlider::valueChanged, this, &MediaManagerDemo::onSpeakerVolumeChanged);
    connect(m_micMuteCheckBox, &QCheckBox::toggled, this, &MediaManagerDemo::onMicMuteToggled);
    connect(m_speakerMuteCheckBox, &QCheckBox::toggled, this, &MediaManagerDemo::onSpeakerMuteToggled);
    
    // 质量设置信号
    connect(m_resolutionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MediaManagerDemo::onQualityChanged);
    connect(m_fpsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MediaManagerDemo::onQualityChanged);
    connect(m_bitrateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MediaManagerDemo::onQualityChanged);
    
    // MediaManager信号
    connect(m_mediaManager, &MediaManager::localVideoStarted, this, &MediaManagerDemo::onLocalVideoStarted);
    connect(m_mediaManager, &MediaManager::localVideoStopped, this, &MediaManagerDemo::onLocalVideoStopped);
    connect(m_mediaManager, &MediaManager::localAudioStarted, this, &MediaManagerDemo::onLocalAudioStarted);
    connect(m_mediaManager, &MediaManager::localAudioStopped, this, &MediaManagerDemo::onLocalAudioStopped);
    connect(m_mediaManager, &MediaManager::screenShareStarted, this, &MediaManagerDemo::onScreenShareStarted);
    connect(m_mediaManager, &MediaManager::screenShareStopped, this, &MediaManagerDemo::onScreenShareStopped);
    connect(m_mediaManager, &MediaManager::deviceListChanged, this, &MediaManagerDemo::onDeviceListChanged);
    connect(m_mediaManager, &MediaManager::cameraError, this, &MediaManagerDemo::onCameraError);
    connect(m_mediaManager, &MediaManager::microphoneError, this, &MediaManagerDemo::onMicrophoneError);
    connect(m_mediaManager, &MediaManager::speakerError, this, &MediaManagerDemo::onSpeakerError);
    connect(m_mediaManager, &MediaManager::screenCaptureError, this, &MediaManagerDemo::onScreenCaptureError);
}

void MediaManagerDemo::refreshDeviceLists()
{
    qDebug() << "MediaManagerDemo: Refreshing device lists";
    
    // 清空现有列表
    m_cameraCombo->clear();
    m_microphoneCombo->clear();
    m_speakerCombo->clear();
    m_screenCombo->clear();
    
    // 填充摄像头列表
    QList<MediaManager::MediaDevice> cameras = m_mediaManager->availableCameras();
    for (const auto& camera : cameras) {
        QString displayName = camera.name;
        if (camera.isDefault) {
            displayName += " (Default)";
        }
        m_cameraCombo->addItem(displayName, camera.id);
    }
    
    // 填充麦克风列表
    QList<MediaManager::MediaDevice> microphones = m_mediaManager->availableMicrophones();
    for (const auto& mic : microphones) {
        QString displayName = mic.name;
        if (mic.isDefault) {
            displayName += " (Default)";
        }
        m_microphoneCombo->addItem(displayName, mic.id);
    }
    
    // 填充扬声器列表
    QList<MediaManager::MediaDevice> speakers = m_mediaManager->availableSpeakers();
    for (const auto& speaker : speakers) {
        QString displayName = speaker.name;
        if (speaker.isDefault) {
            displayName += " (Default)";
        }
        m_speakerCombo->addItem(displayName, speaker.id);
    }
    
    // 填充屏幕列表
    QList<MediaManager::ScreenInfo> screens = m_mediaManager->availableScreens();
    for (const auto& screen : screens) {
        QString displayName = QString("%1 (%2x%3)").arg(screen.name)
                                                   .arg(screen.size.width())
                                                   .arg(screen.size.height());
        if (screen.isPrimary) {
            displayName += " (Primary)";
        }
        m_screenCombo->addItem(displayName, screen.screenId);
    }
    
    qDebug() << "MediaManagerDemo: Device lists refreshed - Cameras:" << cameras.size()
             << "Microphones:" << microphones.size() << "Speakers:" << speakers.size()
             << "Screens:" << screens.size();
}

void MediaManagerDemo::updateStatus()
{
    QString status = QString("Video: %1 | Audio: %2 | Screen Share: %3")
                     .arg(m_mediaManager->isVideoEnabled() ? "ON" : "OFF")
                     .arg(m_mediaManager->isAudioEnabled() ? "ON" : "OFF")
                     .arg(m_mediaManager->isScreenShareEnabled() ? "ON" : "OFF");
    m_statusLabel->setText(status);
}

// 设备选择槽函数
void MediaManagerDemo::onCameraChanged()
{
    QString cameraId = m_cameraCombo->currentData().toString();
    if (!cameraId.isEmpty()) {
        m_mediaManager->selectCamera(cameraId);
        qDebug() << "MediaManagerDemo: Camera changed to:" << cameraId;
    }
}

void MediaManagerDemo::onMicrophoneChanged()
{
    QString micId = m_microphoneCombo->currentData().toString();
    if (!micId.isEmpty()) {
        m_mediaManager->selectMicrophone(micId);
        qDebug() << "MediaManagerDemo: Microphone changed to:" << micId;
    }
}

void MediaManagerDemo::onSpeakerChanged()
{
    QString speakerId = m_speakerCombo->currentData().toString();
    if (!speakerId.isEmpty()) {
        m_mediaManager->selectSpeaker(speakerId);
        qDebug() << "MediaManagerDemo: Speaker changed to:" << speakerId;
    }
}

void MediaManagerDemo::onScreenChanged()
{
    int screenId = m_screenCombo->currentData().toInt();
    m_mediaManager->selectScreen(screenId);
    qDebug() << "MediaManagerDemo: Screen changed to:" << screenId;
}

// 媒体控制槽函数
void MediaManagerDemo::onStartVideo()
{
    qDebug() << "MediaManagerDemo: Starting video";
    m_mediaManager->startLocalVideo();
}

void MediaManagerDemo::onStopVideo()
{
    qDebug() << "MediaManagerDemo: Stopping video";
    m_mediaManager->stopLocalVideo();
}

void MediaManagerDemo::onStartAudio()
{
    qDebug() << "MediaManagerDemo: Starting audio";
    m_mediaManager->startLocalAudio();
}

void MediaManagerDemo::onStopAudio()
{
    qDebug() << "MediaManagerDemo: Stopping audio";
    m_mediaManager->stopLocalAudio();
}

void MediaManagerDemo::onStartScreenShare()
{
    qDebug() << "MediaManagerDemo: Starting screen share";
    m_mediaManager->startScreenShare();
}

void MediaManagerDemo::onStopScreenShare()
{
    qDebug() << "MediaManagerDemo: Stopping screen share";
    m_mediaManager->stopScreenShare();
}

// 音量控制槽函数
void MediaManagerDemo::onMicVolumeChanged(int volume)
{
    m_mediaManager->setMicrophoneVolume(volume);
    m_micVolumeLabel->setText(QString::number(volume));
    qDebug() << "MediaManagerDemo: Microphone volume changed to:" << volume;
}

void MediaManagerDemo::onSpeakerVolumeChanged(int volume)
{
    m_mediaManager->setSpeakerVolume(volume);
    m_speakerVolumeLabel->setText(QString::number(volume));
    qDebug() << "MediaManagerDemo: Speaker volume changed to:" << volume;
}

void MediaManagerDemo::onMicMuteToggled(bool muted)
{
    m_mediaManager->setMicrophoneMuted(muted);
    qDebug() << "MediaManagerDemo: Microphone muted:" << muted;
}

void MediaManagerDemo::onSpeakerMuteToggled(bool muted)
{
    m_mediaManager->setSpeakerMuted(muted);
    qDebug() << "MediaManagerDemo: Speaker muted:" << muted;
}

// 质量设置槽函数
void MediaManagerDemo::onQualityChanged()
{
    MediaManager::MediaQuality quality;
    quality.videoResolution = m_resolutionCombo->currentData().toSize();
    quality.videoFrameRate = m_fpsCombo->currentData().toInt();
    quality.videoBitrate = m_bitrateCombo->currentData().toInt();
    
    m_mediaManager->setMediaQuality(quality);
    qDebug() << "MediaManagerDemo: Quality changed - Resolution:" << quality.videoResolution
             << "FPS:" << quality.videoFrameRate << "Bitrate:" << quality.videoBitrate;
}

// MediaManager事件槽函数
void MediaManagerDemo::onLocalVideoStarted()
{
    qDebug() << "MediaManagerDemo: Local video started";
    m_startVideoBtn->setEnabled(false);
    m_stopVideoBtn->setEnabled(true);
    
    // 获取本地视频组件并显示
    QVideoWidget* videoWidget = m_mediaManager->localVideoWidget();
    if (videoWidget) {
        // 这里可以将videoWidget集成到UI中
        // 由于MediaManager已经创建了QVideoWidget，我们可以直接使用它
    }
    
    updateStatus();
}

void MediaManagerDemo::onLocalVideoStopped()
{
    qDebug() << "MediaManagerDemo: Local video stopped";
    m_startVideoBtn->setEnabled(true);
    m_stopVideoBtn->setEnabled(false);
    updateStatus();
}

void MediaManagerDemo::onLocalAudioStarted()
{
    qDebug() << "MediaManagerDemo: Local audio started";
    m_startAudioBtn->setEnabled(false);
    m_stopAudioBtn->setEnabled(true);
    updateStatus();
}

void MediaManagerDemo::onLocalAudioStopped()
{
    qDebug() << "MediaManagerDemo: Local audio stopped";
    m_startAudioBtn->setEnabled(true);
    m_stopAudioBtn->setEnabled(false);
    updateStatus();
}

void MediaManagerDemo::onScreenShareStarted()
{
    qDebug() << "MediaManagerDemo: Screen share started";
    m_startScreenShareBtn->setEnabled(false);
    m_stopScreenShareBtn->setEnabled(true);
    
    // 获取屏幕共享组件并显示
    QVideoWidget* screenWidget = m_mediaManager->screenShareWidget();
    if (screenWidget) {
        // 这里可以将screenWidget集成到UI中
    }
    
    updateStatus();
}

void MediaManagerDemo::onScreenShareStopped()
{
    qDebug() << "MediaManagerDemo: Screen share stopped";
    m_startScreenShareBtn->setEnabled(true);
    m_stopScreenShareBtn->setEnabled(false);
    updateStatus();
}

void MediaManagerDemo::onDeviceListChanged()
{
    qDebug() << "MediaManagerDemo: Device list changed, refreshing";
    refreshDeviceLists();
}

void MediaManagerDemo::onCameraError(const QString& error)
{
    qWarning() << "MediaManagerDemo: Camera error:" << error;
    QMessageBox::warning(this, "Camera Error", error);
}

void MediaManagerDemo::onMicrophoneError(const QString& error)
{
    qWarning() << "MediaManagerDemo: Microphone error:" << error;
    QMessageBox::warning(this, "Microphone Error", error);
}

void MediaManagerDemo::onSpeakerError(const QString& error)
{
    qWarning() << "MediaManagerDemo: Speaker error:" << error;
    QMessageBox::warning(this, "Speaker Error", error);
}

void MediaManagerDemo::onScreenCaptureError(const QString& error)
{
    qWarning() << "MediaManagerDemo: Screen capture error:" << error;
    QMessageBox::warning(this, "Screen Capture Error", error);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    MediaManagerDemo demo;
    demo.show();
    
    return app.exec();
}

#include "media_manager_demo.moc"