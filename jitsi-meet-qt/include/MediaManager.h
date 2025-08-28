#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include <QObject>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QCamera>
#include <QAudioInput>
#include <QAudioOutput>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QAudioDevice>
#include <QtGui/QScreen>
#include <QApplication>
#include <QTimer>
#include <QMap>
#include <QString>
#include <QStringList>
#include <memory>
#include <optional>

class WebRTCEngine;

class MediaManager : public QObject
{
    Q_OBJECT

public:
    enum MediaType {
        Audio,
        Video,
        Screen
    };

    enum DeviceState {
        DeviceUnavailable,
        DeviceAvailable,
        DeviceActive,
        DeviceError
    };

    struct MediaDevice {
        QString id;
        QString name;
        QString description;
        MediaType type;
        DeviceState state;
        bool isDefault;
    };

    struct MediaSettings {
        // Video settings
        QSize videoResolution = QSize(1280, 720);
        int videoFrameRate = 30;
        int videoBitrate = 1000; // kbps
        
        // Audio settings
        int audioSampleRate = 48000;
        int audioChannels = 2;
        int audioBitrate = 128; // kbps
        
        // Screen capture settings
        QSize screenCaptureResolution = QSize(1920, 1080);
        int screenCaptureFrameRate = 15;
        bool captureAudio = true;
    };

    explicit MediaManager(QObject *parent = nullptr);
    ~MediaManager();

    // Device enumeration and management
    QList<MediaDevice> availableVideoDevices() const;
    QList<MediaDevice> availableAudioInputDevices() const;
    QList<MediaDevice> availableAudioOutputDevices() const;
    QList<QScreen*> availableScreens() const;
    
    // Device selection
    bool setVideoDevice(const QString& deviceId);
    bool setAudioInputDevice(const QString& deviceId);
    bool setAudioOutputDevice(const QString& deviceId);
    
    // Current device getters
    std::optional<MediaDevice> currentVideoDevice() const;
    std::optional<MediaDevice> currentAudioInputDevice() const;
    std::optional<MediaDevice> currentAudioOutputDevice() const;
    
    // Local media stream management
    void startLocalVideo();
    void stopLocalVideo();
    void startLocalAudio();
    void stopLocalAudio();
    
    // Local media preview
    QVideoWidget* localVideoWidget() const;
    void setLocalVideoWidget(QVideoWidget* widget);
    
    // Media state
    bool isVideoActive() const;
    bool isAudioActive() const;
    bool isScreenSharingActive() const;
    
    // Screen sharing
    void startScreenSharing(QScreen* screen = nullptr);
    void stopScreenSharing();
    QVideoWidget* screenShareWidget() const;
    
    // Media encoding/decoding
    void setVideoCodec(const QString& codec);
    void setAudioCodec(const QString& codec);
    QString currentVideoCodec() const;
    QString currentAudioCodec() const;
    
    // Media settings
    void setMediaSettings(const MediaSettings& settings);
    MediaSettings mediaSettings() const;
    
    // Volume control
    void setMasterVolume(qreal volume); // 0.0 to 1.0
    qreal masterVolume() const;
    void setMicrophoneVolume(qreal volume);
    qreal microphoneVolume() const;
    
    // Mute control
    void setVideoMuted(bool muted);
    void setAudioMuted(bool muted);
    bool isVideoMuted() const;
    bool isAudioMuted() const;
    
    // Media permissions
    void requestMediaPermissions();
    bool hasVideoPermission() const;
    bool hasAudioPermission() const;
    
    // Integration with WebRTC
    void setWebRTCEngine(WebRTCEngine* engine);
    WebRTCEngine* webRTCEngine() const;

signals:
    // Device signals
    void videoDeviceChanged(const MediaDevice& device);
    void audioInputDeviceChanged(const MediaDevice& device);
    void audioOutputDeviceChanged(const MediaDevice& device);
    void deviceListChanged();
    void deviceError(const QString& deviceId, const QString& error);
    
    // Media stream signals
    void localVideoStarted();
    void localVideoStopped();
    void localAudioStarted();
    void localAudioStopped();
    void screenSharingStarted();
    void screenSharingStopped();
    
    // Media state signals
    void videoMuteChanged(bool muted);
    void audioMuteChanged(bool muted);
    void volumeChanged(qreal volume);
    void microphoneVolumeChanged(qreal volume);
    
    // Permission signals
    void mediaPermissionsRequested();
    void mediaPermissionsGranted(bool video, bool audio);
    void mediaPermissionsDenied();
    
    // Error signals
    void mediaError(const QString& error);
    void encodingError(const QString& codec, const QString& error);

private slots:
    void onDeviceListChanged();
    void onCameraActiveChanged(bool active);
    void onCameraErrorOccurred(QCamera::Error error);
    void onAudioInputStateChanged();
    void onAudioOutputStateChanged();
    void onScreenCaptureTimer();
    void onWebRTCPermissionsGranted(bool video, bool audio);
    void onWebRTCPermissionsDenied();

private:
    // Device management
    void initializeDevices();
    void updateDeviceList();
    MediaDevice createVideoDevice(const QCameraDevice& device) const;
    MediaDevice createAudioInputDevice(const QAudioDevice& device) const;
    MediaDevice createAudioOutputDevice(const QAudioDevice& device) const;
    void refreshDeviceStates();
    
    // Media initialization
    void initializeVideoCapture();
    void initializeAudioCapture();
    void initializeScreenCapture();
    void cleanupMediaResources();
    
    // Permission handling
    void checkMediaPermissions();
    void handlePermissionResult(bool granted, const QString& permission);
    
    // Screen capture implementation
    void captureScreen();
    void setupScreenCaptureTimer();
    
    // Codec management
    void initializeCodecs();
    QStringList supportedVideoCodecs() const;
    QStringList supportedAudioCodecs() const;
    
    // Settings validation
    bool validateMediaSettings(const MediaSettings& settings) const;
    void applyMediaSettings();
    
    // WebRTC integration
    void connectWebRTCSignals();
    void disconnectWebRTCSignals();
    
    // Media capture components
    std::unique_ptr<QCamera> m_camera;
    std::unique_ptr<QAudioInput> m_audioInput;
    std::unique_ptr<QAudioOutput> m_audioOutput;
    std::unique_ptr<QMediaCaptureSession> m_captureSession;
    std::unique_ptr<QMediaRecorder> m_mediaRecorder;
    
    // Video widgets
    QVideoWidget* m_localVideoWidget;
    QVideoWidget* m_screenShareWidget;
    
    // Device lists
    QList<MediaDevice> m_videoDevices;
    QList<MediaDevice> m_audioInputDevices;
    QList<MediaDevice> m_audioOutputDevices;
    
    // Current devices
    QString m_currentVideoDeviceId;
    QString m_currentAudioInputDeviceId;
    QString m_currentAudioOutputDeviceId;
    
    // Media state
    bool m_videoActive;
    bool m_audioActive;
    bool m_screenSharingActive;
    bool m_videoMuted;
    bool m_audioMuted;
    
    // Volume control
    qreal m_masterVolume;
    qreal m_microphoneVolume;
    
    // Permissions
    bool m_hasVideoPermission;
    bool m_hasAudioPermission;
    
    // Screen sharing
    QScreen* m_currentScreen;
    QTimer* m_screenCaptureTimer;
    
    // Media settings
    MediaSettings m_mediaSettings;
    
    // Codec settings
    QString m_currentVideoCodec;
    QString m_currentAudioCodec;
    QStringList m_supportedVideoCodecs;
    QStringList m_supportedAudioCodecs;
    
    // WebRTC integration
    WebRTCEngine* m_webRTCEngine;
    
    // Constants
    static const int SCREEN_CAPTURE_INTERVAL = 66; // ~15 FPS
    static const qreal DEFAULT_VOLUME;
    static const QString DEFAULT_VIDEO_CODEC;
    static const QString DEFAULT_AUDIO_CODEC;
};

#endif // MEDIAMANAGER_H