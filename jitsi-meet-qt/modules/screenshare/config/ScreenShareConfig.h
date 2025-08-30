#ifndef SCREENSHARECONFIG_H
#define SCREENSHARECONFIG_H

#include <QObject>
#include <QVariantMap>
#include <QRect>
#include <QSize>
#include "../interfaces/IScreenCapture.h"
#include "../interfaces/IScreenShareManager.h"

/**
 * @brief 屏幕共享配置类
 * 
 * 管理屏幕共享模块的所有配置参数
 */
class ScreenShareConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int frameRate READ frameRate WRITE setFrameRate NOTIFY frameRateChanged)
    Q_PROPERTY(int bitrate READ bitrate WRITE setBitrate NOTIFY bitrateChanged)
    Q_PROPERTY(CaptureMode captureMode READ captureMode WRITE setCaptureMode NOTIFY captureModeChanged)
    Q_PROPERTY(CaptureQuality quality READ quality WRITE setQuality NOTIFY qualityChanged)

public:
    // 从接口导入枚举类型
    using CaptureMode = IScreenCapture::CaptureMode;
    using CaptureQuality = IScreenCapture::CaptureQuality;
    using ShareMode = IScreenShareManager::ShareMode;
    using EncodingFormat = IScreenShareManager::EncodingFormat;

    /**
     * @brief 质量预设枚举
     */
    enum QualityPreset {
        PowerSaving,    ///< 节能模式 (低质量, 低帧率)
        Balanced,       ///< 平衡模式 (中等质量)
        HighQuality,    ///< 高质量模式
        UltraQuality,   ///< 超高质量模式
        Custom          ///< 自定义模式
    };
    Q_ENUM(QualityPreset)

    /**
     * @brief 网络适应模式枚举
     */
    enum NetworkAdaptation {
        Disabled,       ///< 禁用适应
        Conservative,   ///< 保守适应
        Aggressive,     ///< 激进适应
        Automatic       ///< 自动适应
    };
    Q_ENUM(NetworkAdaptation)

    explicit ScreenShareConfig(QObject *parent = nullptr);
    virtual ~ScreenShareConfig();

    // 基础配置接口
    bool isEnabled() const;
    void setEnabled(bool enabled);
    bool isValid() const;
    void reset();

    // 捕获配置接口
    CaptureMode captureMode() const;
    void setCaptureMode(CaptureMode mode);
    CaptureQuality quality() const;
    void setQuality(CaptureQuality quality);
    QualityPreset qualityPreset() const;
    void setQualityPreset(QualityPreset preset);

    // 帧率和比特率配置
    int frameRate() const;
    void setFrameRate(int fps);
    int minFrameRate() const;
    void setMinFrameRate(int fps);
    int maxFrameRate() const;
    void setMaxFrameRate(int fps);
    int bitrate() const;
    void setBitrate(int kbps);
    int minBitrate() const;
    void setMinBitrate(int kbps);
    int maxBitrate() const;
    void setMaxBitrate(int kbps);

    // 分辨率配置
    QSize resolution() const;
    void setResolution(const QSize& size);
    QSize maxResolution() const;
    void setMaxResolution(const QSize& size);
    bool maintainAspectRatio() const;
    void setMaintainAspectRatio(bool maintain);

    // 捕获区域配置
    QRect captureRegion() const;
    void setCaptureRegion(const QRect& region);
    QString targetScreen() const;
    void setTargetScreen(const QString& screenId);
    QString targetWindow() const;
    void setTargetWindow(const QString& windowId);

    // 编码配置
    EncodingFormat encodingFormat() const;
    void setEncodingFormat(EncodingFormat format);
    ShareMode shareMode() const;
    void setShareMode(ShareMode mode);
    int keyFrameInterval() const;
    void setKeyFrameInterval(int interval);

    // 网络适应配置
    NetworkAdaptation networkAdaptation() const;
    void setNetworkAdaptation(NetworkAdaptation adaptation);
    bool autoQualityAdjustment() const;
    void setAutoQualityAdjustment(bool enabled);
    int adaptationInterval() const;
    void setAdaptationInterval(int seconds);

    // 性能配置
    bool hardwareAcceleration() const;
    void setHardwareAcceleration(bool enabled);
    int bufferSize() const;
    void setBufferSize(int size);
    int threadCount() const;
    void setThreadCount(int count);

    // 高级配置
    bool enableCursor() const;
    void setEnableCursor(bool enabled);
    bool enableAudio() const;
    void setEnableAudio(bool enabled);
    int captureDelay() const;
    void setCaptureDelay(int ms);

    // 配置序列化接口
    QVariantMap toVariantMap() const;
    void fromVariantMap(const QVariantMap& map);
    bool loadFromFile(const QString& filePath);
    bool saveToFile(const QString& filePath) const;

    // 预设配置接口
    void applyPreset(QualityPreset preset);
    static QVariantMap getPresetConfiguration(QualityPreset preset);
    QStringList availablePresets() const;

    // 验证接口
    bool validate(QString* errorMessage = nullptr) const;
    QStringList getValidationErrors() const;

public slots:
    /**
     * @brief 应用配置更改
     */
    void apply();

    /**
     * @brief 恢复默认配置
     */
    void restoreDefaults();

    /**
     * @brief 优化配置以适应当前系统
     */
    void optimizeForSystem();

signals:
    /**
     * @brief 启用状态改变信号
     * @param enabled 是否启用
     */
    void enabledChanged(bool enabled);

    /**
     * @brief 帧率改变信号
     * @param frameRate 新的帧率
     */
    void frameRateChanged(int frameRate);

    /**
     * @brief 比特率改变信号
     * @param bitrate 新的比特率
     */
    void bitrateChanged(int bitrate);

    /**
     * @brief 捕获模式改变信号
     * @param mode 新的捕获模式
     */
    void captureModeChanged(CaptureMode mode);

    /**
     * @brief 质量改变信号
     * @param quality 新的质量设置
     */
    void qualityChanged(CaptureQuality quality);

    /**
     * @brief 配置改变信号
     */
    void configurationChanged();

    /**
     * @brief 配置验证失败信号
     * @param errors 验证错误列表
     */
    void validationFailed(const QStringList& errors);

private:
    void initializeDefaults();
    void validateAndEmitChanges();
    QVariantMap createPresetConfiguration(QualityPreset preset) const;

    class Private;
    Private* d;
};

#endif // SCREENSHARECONFIG_H