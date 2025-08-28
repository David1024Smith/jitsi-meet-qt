#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include <QObject>
#include <QVideoWidget>
#include <QCamera>
#include <QAudioInput>
#include <QAudioOutput>
#include <QMediaRecorder>
#include <QCameraInfo>
#include <QAudioDeviceInfo>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QList>
#include <QString>
#include <QSize>
#include <QPixmap>
#include <QBuffer>
#include <QImageWriter>
#include <QAudioFormat>
#include <QCameraViewfinder>
#include <QVideoFrame>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QRect>

class WebRTCEngine;

/**
 * @brief MediaManager统一管理音视频设备和媒体流
 * 
 * 该类负责：
 * - 音视频设备的枚举和选择
 * - 本地音视频流的捕获和预览
 * - 音视频编码和解码功能
 * - 屏幕捕获功能用于屏幕共享
 * - 与WebRTCEngine的集成
 */
class MediaManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 媒体设备信息结构
     */
    struct MediaDevice {
        QString id;
        QString name;
        QString description;
        bool isDefault;
        
        MediaDevice() : isDefault(false) {}
        MediaDevice(const QString& deviceId, const QString& deviceName, 
                   const QString& deviceDesc = QString(), bool defaultDevice = false)
            : id(deviceId), name(deviceName), description(deviceDesc), isDefault(defaultDevice) {}
    };

    /**
     * @brief 屏幕信息结构
     */
    struct ScreenInfo {
        int screenId;
        QString name;
        QSize size;
        QRect geometry;
        bool isPrimary;
        
        ScreenInfo() : screenId(-1), isPrimary(false) {}
        ScreenInfo(int id, const QString& screenName, const QSize& screenSize, 
                  const QRect& screenGeometry, bool primary = false)
            : screenId(id), name(screenName), size(screenSize), 
              geometry(screenGeometry), isPrimary(primary) {}
    };

    /**
     * @brief 媒体质量设置
     */
    struct MediaQuality {
        // 视频设置
        QSize videoResolution;
        int videoFrameRate;
        int videoBitrate;
        
        // 音频设置
        int audioSampleRate;
        int audioChannels;
        int audioBitrate;
        
        MediaQuality() 
            : videoResolution(640, 480), videoFrameRate(30), videoBitrate(1000000),
              audioSampleRate(44100), audioChannels(2), audioBitrate(128000) {}
    };

    explicit MediaManager(QObject *parent = nullptr);
    ~MediaManager();

    // 设备枚举和选择
    QList<MediaDevice> availableCameras() const;
    QList<MediaDevice> availableMicrophones() const;
    QList<MediaDevice> availableSpeakers() const;
    QList<ScreenInfo> availableScreens() const;
    
    // 设备选择
    bool selectCamera(const QString& deviceId);
    bool selectMicrophone(const QString& deviceId);
    bool selectSpeaker(const QString& deviceId);
    bool selectScreen(int screenId);
    
    // 当前设备获取
    MediaDevice currentCamera() const;
    MediaDevice currentMicrophone() const;
    MediaDevice currentSpeaker() const;
    ScreenInfo currentScreen() const;
    
    // 本地媒体流控制
    void startLocalVideo();
    void stopLocalVideo();
    void startLocalAudio();
    void stopLocalAudio();
    
    // 屏幕共享控制
    void startScreenShare();
    void stopScreenShare();
    
    // 媒体流状态
    bool isVideoEnabled() const;
    bool isAudioEnabled() const;
    bool isScreenShareEnabled() const;
    
    // 媒体质量设置
    void setMediaQuality(const MediaQuality& quality);
    MediaQuality mediaQuality() const;
    
    // 本地预览
    QVideoWidget* localVideoWidget() const;
    QVideoWidget* screenShareWidget() const;
    
    // 音量控制
    void setMicrophoneVolume(int volume); // 0-100
    void setSpeakerVolume(int volume);    // 0-100
    int microphoneVolume() const;
    int speakerVolume() const;
    
    // 静音控制
    void setMicrophoneMuted(bool muted);
    void setSpeakerMuted(bool muted);
    bool isMicrophoneMuted() const;
    bool isSpeakerMuted() const;
    
    // WebRTC集成
    void setWebRTCEngine(WebRTCEngine* engine);
    WebRTCEngine* webRTCEngine() const;

signals:
    // 本地媒体流事件
    void localVideoStarted();
    void localVideoStopped();
    void localAudioStarted();
    void localAudioStopped();
    
    // 屏幕共享事件
    void screenShareStarted();
    void screenShareStopped();
    
    // 远程媒体流事件
    void remoteVideoReceived(const QString& participantId, QVideoWidget* widget);
    void remoteVideoRemoved(const QString& participantId);
    void remoteAudioReceived(const QString& participantId);
    void remoteAudioRemoved(const QString& participantId);
    
    // 设备事件
    void deviceListChanged();
    void cameraChanged(const MediaDevice& device);
    void microphoneChanged(const MediaDevice& device);
    void speakerChanged(const MediaDevice& device);
    void screenChanged(const ScreenInfo& screen);
    
    // 音量和静音事件
    void microphoneVolumeChanged(int volume);
    void speakerVolumeChanged(int volume);
    void microphoneMutedChanged(bool muted);
    void speakerMutedChanged(bool muted);
    
    // 错误事件
    void cameraError(const QString& error);
    void microphoneError(const QString& error);
    void speakerError(const QString& error);
    void screenCaptureError(const QString& error);

private slots:
    void onCameraStateChanged(QCamera::State state);
    void onCameraError(QCamera::Error error);
    void onAudioInputStateChanged();
    void onAudioOutputStateChanged();
    void onScreenCaptureTimer();
    void onDeviceListChanged();

private:
    // 初始化和清理
    void initializeDevices();
    void cleanupDevices();
    
    // 摄像头管理
    void setupCamera();
    void cleanupCamera();
    void updateCameraSettings();
    
    // 音频管理
    void setupAudioInput();
    void setupAudioOutput();
    void cleanupAudio();
    void updateAudioSettings();
    
    // 屏幕捕获管理
    void setupScreenCapture();
    void cleanupScreenCapture();
    void captureScreenFrame();
    
    // 设备枚举
    void refreshDeviceList();
    void enumerateCameras();
    void enumerateAudioDevices();
    void enumerateScreens();
    
    // 编码和解码
    QByteArray encodeVideoFrame(const QVideoFrame& frame);
    QByteArray encodeAudioFrame(const QByteArray& audioData);
    void decodeVideoFrame(const QByteArray& data, const QString& participantId);
    void decodeAudioFrame(const QByteArray& data, const QString& participantId);
    
    // WebRTC集成
    void connectToWebRTC();
    void disconnectFromWebRTC();
    void sendVideoFrame(const QByteArray& frameData);
    void sendAudioFrame(const QByteArray& audioData);
    
    // 设备管理
    QCamera* m_camera;
    QVideoWidget* m_localVideoWidget;
    QCameraViewfinder* m_cameraViewfinder;
    QAudioInput* m_audioInput;
    QAudioOutput* m_audioOutput;
    QMediaRecorder* m_mediaRecorder;
    
    // 屏幕捕获
    QTimer* m_screenCaptureTimer;
    QVideoWidget* m_screenShareWidget;
    QScreen* m_selectedScreen;
    
    // 设备列表
    QList<MediaDevice> m_cameras;
    QList<MediaDevice> m_microphones;
    QList<MediaDevice> m_speakers;
    QList<ScreenInfo> m_screens;
    
    // 当前选择的设备
    MediaDevice m_currentCamera;
    MediaDevice m_currentMicrophone;
    MediaDevice m_currentSpeaker;
    ScreenInfo m_currentScreen;
    
    // 媒体状态
    bool m_videoEnabled;
    bool m_audioEnabled;
    bool m_screenShareEnabled;
    bool m_microphoneMuted;
    bool m_speakerMuted;
    
    // 音量设置
    int m_microphoneVolume;
    int m_speakerVolume;
    
    // 媒体质量设置
    MediaQuality m_mediaQuality;
    
    // WebRTC集成
    WebRTCEngine* m_webrtcEngine;
    
    // 远程媒体流
    QMap<QString, QVideoWidget*> m_remoteVideoWidgets;
    
    // 常量
    static const int SCREEN_CAPTURE_FPS = 30;
    static const int SCREEN_CAPTURE_INTERVAL = 1000 / SCREEN_CAPTURE_FPS; // 33ms
    static const int DEFAULT_VIDEO_WIDTH = 640;
    static const int DEFAULT_VIDEO_HEIGHT = 480;
    static const int DEFAULT_VIDEO_FPS = 30;
    static const int DEFAULT_AUDIO_SAMPLE_RATE = 44100;
    static const int DEFAULT_AUDIO_CHANNELS = 2;
};

#endif // MEDIAMANAGER_H