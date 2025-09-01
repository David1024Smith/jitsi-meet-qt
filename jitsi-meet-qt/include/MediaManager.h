#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVideoWidget>
#include <memory>

// 前向声明
class AudioManager;
class CameraManager;

/**
 * @brief 媒体管理器 - 统一管理音频和视频设备
 * 
 * MediaManager 提供统一的媒体设备管理接口，整合音频和摄像头管理功能
 */
class MediaManager : public QObject
{
    Q_OBJECT

public:
    explicit MediaManager(QObject *parent = nullptr);
    ~MediaManager();

    /**
     * @brief 初始化媒体管理器
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 清理媒体管理器
     */
    void cleanup();

    /**
     * @brief 获取音频管理器
     * @return 音频管理器实例
     */
    AudioManager* audioManager() const;

    /**
     * @brief 获取摄像头管理器
     * @return 摄像头管理器实例
     */
    CameraManager* cameraManager() const;

    /**
     * @brief 检查媒体设备是否可用
     * @return 设备是否可用
     */
    bool isMediaAvailable() const;

    /**
     * @brief 设置WebRTC引擎
     * @param engine WebRTC引擎实例
     */
    void setWebRTCEngine(QObject* engine);

    /**
     * @brief 开始本地视频
     */
    void startLocalVideo();

    /**
     * @brief 停止本地视频
     */
    void stopLocalVideo();

    /**
     * @brief 开始本地音频
     */
    void startLocalAudio();

    /**
     * @brief 停止本地音频
     */
    void stopLocalAudio();

    /**
     * @brief 获取本地视频控件
     * @return 视频控件指针
     */
    QVideoWidget* localVideoWidget() const;

    /**
     * @brief 设置本地视频控件
     * @param widget 视频控件
     */
    void setLocalVideoWidget(QVideoWidget* widget);

    /**
     * @brief 强制启动摄像头显示
     */
    void forceStartCameraDisplay();

    /**
     * @brief 请求媒体权限
     */
    void requestMediaPermissions();

signals:
    /**
     * @brief 媒体设备状态改变信号
     * @param available 设备是否可用
     */
    void mediaAvailabilityChanged(bool available);

    /**
     * @brief 媒体错误信号
     * @param error 错误信息
     */
    void mediaError(const QString& error);

    /**
     * @brief 本地视频开始信号
     */
    void localVideoStarted();

    /**
     * @brief 本地视频停止信号
     */
    void localVideoStopped();

    /**
     * @brief 本地音频开始信号
     */
    void localAudioStarted();

private slots:
    void onAudioDeviceChanged();
    void onCameraDeviceChanged();

private:
    void setupConnections();

    AudioManager* m_audioManager;
    CameraManager* m_cameraManager;
    bool m_initialized;
    QObject* m_webRTCEngine;
    QVideoWidget* m_localVideoWidget;
};

#endif // MEDIAMANAGER_H