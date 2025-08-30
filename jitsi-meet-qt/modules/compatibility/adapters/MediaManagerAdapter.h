#ifndef MEDIAMANAGERADAPTER_H
#define MEDIAMANAGERADAPTER_H

#include "ICompatibilityAdapter.h"
#include <QObject>

// 前向声明
class MediaManager;
class AudioManager;
class CameraManager;

/**
 * @brief 媒体管理器适配器
 * 
 * 提供旧的MediaManager API到新的音频和相机模块的适配。
 */
class MediaManagerAdapter : public ICompatibilityAdapter
{
    Q_OBJECT

public:
    explicit MediaManagerAdapter(QObject *parent = nullptr);
    ~MediaManagerAdapter();

    // ICompatibilityAdapter 接口实现
    bool initialize() override;
    AdapterStatus status() const override;
    QString adapterName() const override;
    QString targetModule() const override;
    CompatibilityLevel checkCompatibility() override;
    bool enable() override;
    void disable() override;
    QVariantMap getConfiguration() const override;
    bool setConfiguration(const QVariantMap& config) override;
    QStringList validateFunctionality() override;

    // 特定功能
    MediaManager* getLegacyManager() const;
    void setAudioManager(AudioManager* audioManager);
    void setCameraManager(CameraManager* cameraManager);

private slots:
    void onAudioStatusChanged();
    void onCameraStatusChanged();

private:
    void createLegacyMediaManager();
    bool validateAudioIntegration();
    bool validateCameraIntegration();

    AdapterStatus m_status;
    QVariantMap m_config;
    
    MediaManager* m_legacyManager;
    AudioManager* m_audioManager;
    CameraManager* m_cameraManager;
    
    bool m_audioIntegrationValid;
    bool m_cameraIntegrationValid;
};

#endif // MEDIAMANAGERADAPTER_H