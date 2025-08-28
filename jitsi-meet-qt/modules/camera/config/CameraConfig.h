#ifndef CAMERACONFIG_H
#define CAMERACONFIG_H

#include <QObject>
#include <QSettings>
#include <QSize>
#include <QVariantMap>
#include "../interfaces/ICameraDevice.h"

/**
 * @brief 摄像头配置管理类
 * 
 * 负责摄像头模块的配置管理，包括设备偏好、质量设置、性能参数等
 */
class CameraConfig : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 配置键名常量
     */
    struct Keys {
        static const QString PREFERRED_DEVICE;
        static const QString DEFAULT_RESOLUTION;
        static const QString DEFAULT_FRAME_RATE;
        static const QString DEFAULT_QUALITY_PRESET;
        static const QString AUTO_START_CAMERA;
        static const QString ENABLE_HARDWARE_ACCELERATION;
        static const QString MAX_RETRY_COUNT;
        static const QString RETRY_DELAY_MS;
        static const QString ENABLE_PERFORMANCE_MONITORING;
        static const QString LOG_LEVEL;
    };

    /**
     * @brief 默认配置值
     */
    struct Defaults {
        static const QSize RESOLUTION;
        static const int FRAME_RATE;
        static const ICameraDevice::QualityPreset QUALITY_PRESET;
        static const bool AUTO_START_CAMERA;
        static const bool ENABLE_HARDWARE_ACCELERATION;
        static const int MAX_RETRY_COUNT;
        static const int RETRY_DELAY_MS;
        static const bool ENABLE_PERFORMANCE_MONITORING;
        static const QString LOG_LEVEL;
    };

    explicit CameraConfig(QObject* parent = nullptr);
    ~CameraConfig();

    // 单例访问
    static CameraConfig* instance();

    // 基本配置
    void setPreferredDevice(const QString& deviceId);
    QString preferredDevice() const;

    void setDefaultResolution(const QSize& resolution);
    QSize defaultResolution() const;

    void setDefaultFrameRate(int frameRate);
    int defaultFrameRate() const;

    void setDefaultQualityPreset(ICameraDevice::QualityPreset preset);
    ICameraDevice::QualityPreset defaultQualityPreset() const;

    // 行为配置
    void setAutoStartCamera(bool autoStart);
    bool autoStartCamera() const;

    void setEnableHardwareAcceleration(bool enable);
    bool enableHardwareAcceleration() const;

    // 错误处理配置
    void setMaxRetryCount(int count);
    int maxRetryCount() const;

    void setRetryDelay(int delayMs);
    int retryDelay() const;

    // 性能配置
    void setEnablePerformanceMonitoring(bool enable);
    bool enablePerformanceMonitoring() const;

    // 日志配置
    void setLogLevel(const QString& level);
    QString logLevel() const;

    // 批量操作
    void loadFromSettings();
    void saveToSettings();
    void resetToDefaults();

    // 配置导入导出
    QVariantMap toVariantMap() const;
    void fromVariantMap(const QVariantMap& config);

    // 配置验证
    bool isValid() const;
    QStringList validate() const;

public slots:
    /**
     * @brief 重新加载配置
     */
    void reload();

    /**
     * @brief 保存配置
     */
    void save();

signals:
    /**
     * @brief 配置改变信号
     */
    void configChanged();

    /**
     * @brief 特定配置项改变
     */
    void preferredDeviceChanged(const QString& deviceId);
    void defaultResolutionChanged(const QSize& resolution);
    void defaultFrameRateChanged(int frameRate);
    void qualityPresetChanged(ICameraDevice::QualityPreset preset);

private:
    void initializeDefaults();
    void connectSignals();

    QSettings* m_settings;
    static CameraConfig* s_instance;

    // 缓存的配置值
    QString m_preferredDevice;
    QSize m_defaultResolution;
    int m_defaultFrameRate;
    ICameraDevice::QualityPreset m_defaultQualityPreset;
    bool m_autoStartCamera;
    bool m_enableHardwareAcceleration;
    int m_maxRetryCount;
    int m_retryDelay;
    bool m_enablePerformanceMonitoring;
    QString m_logLevel;
};

#endif // CAMERACONFIG_H