#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QVideoWidget>
#include <QWidget>
#include <QMap>
#include <QString>
#include <QTimer>
#include "../interfaces/ICameraManager.h"

// 前向声明
class CameraModule;

/**
 * @brief 摄像头管理器 - 高级摄像头管理接口
 * 
 * 这个管理器提供：
 * - 多摄像头管理
 * - 摄像头预设配置
 * - 自动故障恢复
 * - 性能监控
 * - 统一的事件接口
 * 
 * 实现了ICameraManager接口，提供标准化的摄像头管理操作
 */
class CameraManager : public ICameraManager
{
    Q_OBJECT

public:
    /**
     * @brief 摄像头预设配置
     */
    enum CameraPreset {
        LowQuality,     // 低质量 (640x480, 15fps)
        StandardQuality, // 标准质量 (1280x720, 30fps)
        HighQuality,    // 高质量 (1920x1080, 30fps)
        CustomQuality   // 自定义质量
    };

    /**
     * @brief 管理器状态
     */
    enum ManagerState {
        Idle,           // 空闲
        Initializing,   // 初始化中
        Ready,          // 就绪
        Error           // 错误
    };

    /**
     * @brief 摄像头统计信息
     */
    struct CameraStats {
        int frameCount = 0;         // 帧计数
        double frameRate = 0.0;     // 实际帧率
        QSize resolution;           // 当前分辨率
        QString deviceName;         // 设备名称
        qint64 uptime = 0;         // 运行时间（毫秒）
        int errorCount = 0;         // 错误计数
    };

    explicit CameraManager(QObject *parent = nullptr);
    ~CameraManager();

    // === ICameraManager接口实现 ===
    bool initialize() override;
    void cleanup() override;
    ManagerStatus status() const override;
    
    QStringList availableDevices() const override;
    ICameraDevice* currentDevice() const override;
    bool selectDevice(const QString& deviceId) override;
    void refreshDevices() override;
    
    bool startCamera() override;
    void stopCamera() override;
    bool isCameraActive() const override;
    
    QVideoWidget* createPreviewWidget(QWidget* parent = nullptr) override;
    void setPreviewWidget(QVideoWidget* widget) override;
    QVideoWidget* previewWidget() const override;
    
    bool startWithPreset(ICameraDevice::QualityPreset preset) override;
    void applyConfiguration(const QVariantMap& config) override;
    QVariantMap currentConfiguration() const override;
    
    int frameCount() const override;
    double averageFrameRate() const override;
    QSize currentResolution() const override;

    // === 管理器控制 (扩展方法) ===
    /**
     * @brief 获取管理器状态
     */
    ManagerState state() const;

    /**
     * @brief 是否已就绪
     */
    bool isReady() const;

    // === 摄像头控制 (扩展方法) ===
    /**
     * @brief 启动默认摄像头
     */
    bool startDefault();

    /**
     * @brief 启动指定摄像头
     */
    bool startCamera(const QString& deviceId);

    /**
     * @brief 使用预设启动摄像头
     */
    bool startWithPreset(CameraPreset preset);

    /**
     * @brief 重启摄像头
     */
    void restartCamera();

    /**
     * @brief 切换摄像头设备
     */
    bool switchDevice(const QString& deviceId);

    // === 设备管理 (扩展方法) ===
    /**
     * @brief 获取可用设备列表（扩展方法）
     */
    QVariantList availableDevicesExtended();

    /**
     * @brief 获取当前设备（扩展方法）
     */
    QVariantMap currentDeviceExtended();

    /**
     * @brief 获取当前视频组件
     */
    QVideoWidget* videoWidget() const;

    /**
     * @brief 设置视频组件
     */
    void setVideoWidget(QVideoWidget* widget);

    // === 配置管理 ===
    /**
     * @brief 设置摄像头预设
     */
    void setPreset(CameraPreset preset);

    /**
     * @brief 获取当前预设
     */
    CameraPreset currentPreset() const;

    /**
     * @brief 设置自定义配置
     */
    void setCustomConfig(const QVariantMap& config);

    /**
     * @brief 获取当前配置
     */
    QVariantMap getCurrentConfig() const;

    // === 统计和监控 ===
    /**
     * @brief 获取摄像头统计信息
     */
    CameraStats getStats() const;

    /**
     * @brief 重置统计信息
     */
    void resetStats();

    /**
     * @brief 启用性能监控
     */
    void enableMonitoring(bool enable = true);

    // === 故障恢复 ===
    /**
     * @brief 启用自动恢复
     */
    void enableAutoRecovery(bool enable = true);

    /**
     * @brief 设置最大重试次数
     */
    void setMaxRetries(int maxRetries);

signals:
    // === 管理器信号 ===
    /**
     * @brief 管理器状态变化
     */
    void stateChanged(ManagerState state);

    /**
     * @brief 管理器就绪
     */
    void ready();

    /**
     * @brief 管理器错误
     */
    void error(const QString& message);

    // === 摄像头信号 ===
    /**
     * @brief 摄像头启动成功
     */
    void cameraStarted();

    /**
     * @brief 摄像头停止
     */
    void cameraStopped();

    /**
     * @brief 摄像头错误
     */
    void cameraError(const QString& error);

    /**
     * @brief 设备变化（扩展信号）
     */
    void deviceChangedExtended(const QVariantMap& device);

    /**
     * @brief 设备列表更新
     */
    void devicesUpdated();

    // === 统计信号 ===
    /**
     * @brief 统计信息更新
     */
    void statsUpdated(const CameraStats& stats);

private slots:
    /**
     * @brief 处理摄像头模块状态变化
     */
    void onCameraStateChanged(int state);

    /**
     * @brief 处理摄像头启动
     */
    void onCameraStarted();

    /**
     * @brief 处理摄像头停止
     */
    void onCameraStopped();

    /**
     * @brief 处理摄像头错误
     */
    void onCameraError(const QString& error);

    /**
     * @brief 处理设备变化
     */
    void onDeviceChanged(const QVariantMap& device);

    /**
     * @brief 处理设备列表变化
     */
    void onDevicesChanged();

    /**
     * @brief 统计更新定时器
     */
    void onStatsTimer();

    /**
     * @brief 故障恢复定时器
     */
    void onRecoveryTimer();

private:
    /**
     * @brief 设置管理器状态
     */
    void setState(ManagerState state);

    /**
     * @brief 创建预设配置
     */
    QVariantMap createPresetConfig(CameraPreset preset) const;

    /**
     * @brief 更新统计信息
     */
    void updateStats();

    /**
     * @brief 尝试故障恢复
     */
    void attemptRecovery();

    /**
     * @brief 重置重试计数
     */
    void resetRetryCount();

private:
    // === 核心组件 ===
    CameraModule* m_cameraModule;           // 摄像头模块

    // === 状态管理 ===
    ManagerState m_state;                   // 管理器状态
    CameraPreset m_currentPreset;           // 当前预设
    QVariantMap m_customConfig; // 自定义配置

    // === 统计和监控 ===
    CameraStats m_stats;                    // 统计信息
    QTimer* m_statsTimer;                   // 统计定时器
    QTimer* m_recoveryTimer;                // 恢复定时器
    qint64 m_startTime;                     // 启动时间
    bool m_monitoringEnabled;               // 是否启用监控

    // === 故障恢复 ===
    bool m_autoRecoveryEnabled;             // 是否启用自动恢复
    int m_maxRetries;                       // 最大重试次数
    int m_currentRetries;                   // 当前重试次数
    QString m_lastError;                    // 最后错误信息
};

#endif // CAMERAMANAGER_H