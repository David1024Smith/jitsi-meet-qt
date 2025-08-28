#ifndef CAMERAMODULE_H
#define CAMERAMODULE_H

#include <QObject>
#include <QVideoWidget>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QTimer>
#include <QList>
#include <QString>
#include <memory>
#include "../interfaces/ICameraDevice.h"

/**
 * @brief 摄像头模块 - 独立管理摄像头功能
 * 
 * 这个模块负责：
 * - 摄像头设备的枚举和管理
 * - 摄像头的启动、停止和切换
 * - 视频流的捕获和显示
 * - 摄像头权限处理
 * - 摄像头错误处理和恢复
 * 
 * 实现了ICameraDevice接口，提供标准化的摄像头设备操作
 */
class CameraModule : public ICameraDevice
{
    Q_OBJECT

public:
    /**
     * @brief 摄像头设备信息
     */
    struct CameraDevice {
        QString id;              // 设备ID
        QString name;            // 设备名称
        QString description;     // 设备描述
        bool isDefault;          // 是否为默认设备
        bool isActive;           // 是否正在使用
        QList<QSize> supportedResolutions; // 支持的分辨率
    };

    /**
     * @brief 摄像头状态
     */
    enum CameraState {
        Stopped,        // 已停止
        Starting,       // 启动中
        Active,         // 运行中
        Stopping,       // 停止中
        Error           // 错误状态
    };

    /**
     * @brief 摄像头配置
     */
    struct CameraConfig {
        QSize resolution = QSize(1280, 720);    // 分辨率
        int frameRate = 30;                     // 帧率
        QString deviceId;                       // 指定设备ID（空则使用默认）
        bool autoStart = true;                  // 是否自动启动
        bool enablePermissionCheck = false;     // 是否启用权限检查
    };

    explicit CameraModule(QObject *parent = nullptr);
    ~CameraModule();

    // === ICameraDevice接口实现 ===
    bool initialize() override;
    void cleanup() override;
    bool start() override;
    void stop() override;
    bool isActive() const override;
    ICameraDevice::Status status() const override;
    
    QString deviceId() const override;
    QString deviceName() const override;
    QString description() const override;
    bool isAvailable() const override;
    
    void setResolution(const QSize& resolution) override;
    QSize resolution() const override;
    void setFrameRate(int frameRate) override;
    int frameRate() const override;
    void setQualityPreset(QualityPreset preset) override;
    QualityPreset qualityPreset() const override;
    
    QList<QSize> supportedResolutions() const override;
    QList<int> supportedFrameRates() const override;

    // === 设备管理 ===
    /**
     * @brief 扫描可用的摄像头设备
     * @return 设备列表
     */
    QList<CameraDevice> scanDevices();

    /**
     * @brief 获取当前可用设备列表
     */
    QList<CameraDevice> availableDevices() const;

    /**
     * @brief 获取当前使用的设备
     */
    CameraDevice currentDevice() const;

    /**
     * @brief 设置要使用的摄像头设备
     * @param deviceId 设备ID
     * @return 是否设置成功
     */
    bool setDevice(const QString& deviceId);

    // === 摄像头控制 (扩展方法) ===
    /**
     * @brief 启动摄像头
     * @param config 摄像头配置
     * @return 是否启动成功
     */
    bool start(const CameraConfig& config);
    
    /**
     * @brief 启动摄像头（使用默认配置）
     * @return 是否启动成功
     */
    bool startDefault();

    /**
     * @brief 重启摄像头
     */
    void restart();

    /**
     * @brief 强制启动摄像头（绕过权限检查）
     */
    bool forceStart();

    // === 状态查询 (扩展方法) ===
    /**
     * @brief 获取摄像头状态
     */
    CameraState state() const;

    /**
     * @brief 是否有可用设备
     */
    bool hasDevices() const;

    // === 视频显示 ===
    /**
     * @brief 获取视频显示组件
     */
    QVideoWidget* videoWidget() const;

    /**
     * @brief 设置视频显示组件
     * @param widget 视频组件
     */
    void setVideoWidget(QVideoWidget* widget);

    /**
     * @brief 创建新的视频显示组件
     * @param parent 父组件
     * @return 创建的视频组件
     */
    QVideoWidget* createVideoWidget(QWidget* parent = nullptr);

    // === 配置管理 ===
    /**
     * @brief 设置摄像头配置
     */
    void setConfig(const CameraConfig& config);

    /**
     * @brief 获取当前配置
     */
    CameraConfig config() const;

    // === 权限管理 ===
    /**
     * @brief 检查摄像头权限
     */
    bool checkPermission();

    /**
     * @brief 请求摄像头权限
     */
    void requestPermission();

signals:
    // === 状态信号 ===
    /**
     * @brief 摄像头状态变化
     */
    void stateChanged(CameraState state);

    /**
     * @brief 摄像头启动成功
     */
    void started();

    /**
     * @brief 摄像头停止
     */
    void stopped();

    /**
     * @brief 摄像头错误
     */
    void errorOccurred(const QString& error);

    // === 设备信号 ===
    /**
     * @brief 设备列表变化
     */
    void devicesChanged();

    /**
     * @brief 当前设备变化
     */
    void deviceChanged(const CameraDevice& device);

    // === 权限信号 ===
    /**
     * @brief 权限请求结果
     */
    void permissionResult(bool granted);

    /**
     * @brief 权限被拒绝
     */
    void permissionDenied();

private slots:
    /**
     * @brief 处理摄像头激活状态变化
     */
    void onCameraActiveChanged(bool active);

    /**
     * @brief 处理摄像头错误
     */
    void onCameraError(QCamera::Error error);

    /**
     * @brief 处理设备列表变化
     */
    void onDeviceListChanged();

    /**
     * @brief 状态检查定时器
     */
    void onStatusCheckTimer();

private:
    // === 内部方法 ===

    /**
     * @brief 创建摄像头对象
     */
    bool createCamera(const QString& deviceId = QString());

    /**
     * @brief 销毁摄像头对象
     */
    void destroyCamera();

    /**
     * @brief 设置摄像头状态
     */
    void setState(CameraState state);

    /**
     * @brief 更新设备列表
     */
    void updateDeviceList();

    /**
     * @brief 应用摄像头配置
     */
    void applyConfig();

    /**
     * @brief 连接摄像头信号
     */
    void connectCameraSignals();

    /**
     * @brief 断开摄像头信号
     */
    void disconnectCameraSignals();

    /**
     * @brief 创建设备信息
     */
    CameraDevice createDeviceInfo(const QCameraDevice& device) const;

private:
    // === 核心组件 ===
    std::unique_ptr<QCamera> m_camera;                    // 摄像头对象
    std::unique_ptr<QMediaCaptureSession> m_captureSession; // 捕获会话
    QVideoWidget* m_videoWidget;                          // 视频显示组件

    // === 状态管理 ===
    CameraState m_state;                                  // 当前状态
    CameraConfig m_config;                                // 摄像头配置
    QString m_currentDeviceId;                            // 当前设备ID
    QList<CameraDevice> m_devices;                        // 设备列表

    // === 定时器 ===
    QTimer* m_statusCheckTimer;                           // 状态检查定时器
    QTimer* m_deviceScanTimer;                            // 设备扫描定时器

    // === 标志位 ===
    bool m_initialized;                                   // 是否已初始化
    bool m_hasPermission;                                 // 是否有权限
    bool m_autoRestart;                                   // 是否自动重启
};

#endif // CAMERAMODULE_H