#include "MediaManagerAdapter.h"

#include <QDebug>

// 这里需要包含实际的管理器类
// #include "MediaManager.h"
// #include "AudioManager.h"
// #include "CameraManager.h"

// 临时的占位符类定义
class MediaManager : public QObject {
    Q_OBJECT
public:
    explicit MediaManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~MediaManager() = default;
    
    virtual bool initialize() { return true; }
    virtual bool startAudio() { return true; }
    virtual bool stopAudio() { return true; }
    virtual bool startVideo() { return true; }
    virtual bool stopVideo() { return true; }
    virtual void setVolume(qreal volume) { Q_UNUSED(volume) }
    virtual qreal getVolume() const { return 1.0; }
};

class AudioManager : public QObject {
    Q_OBJECT
public:
    explicit AudioManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~AudioManager() = default;
    
    virtual bool initialize() { return true; }
    virtual bool isActive() const { return true; }
    
signals:
    void statusChanged();
};

class CameraManager : public QObject {
    Q_OBJECT
public:
    explicit CameraManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~CameraManager() = default;
    
    virtual bool initialize() { return true; }
    virtual bool isActive() const { return true; }
    
signals:
    void statusChanged();
};

MediaManagerAdapter::MediaManagerAdapter(QObject *parent)
    : ICompatibilityAdapter(parent)
    , m_status(NotInitialized)
    , m_legacyManager(nullptr)
    , m_audioManager(nullptr)
    , m_cameraManager(nullptr)
    , m_audioIntegrationValid(false)
    , m_cameraIntegrationValid(false)
{
    // 设置默认配置
    m_config["enable_audio"] = true;
    m_config["enable_video"] = true;
    m_config["compatibility_mode"] = "full";
}

MediaManagerAdapter::~MediaManagerAdapter()
{
    disable();
    
    if (m_legacyManager) {
        m_legacyManager->deleteLater();
    }
}

bool MediaManagerAdapter::initialize()
{
    if (m_status != NotInitialized) {
        return m_status == Ready;
    }

    qDebug() << "Initializing MediaManagerAdapter...";
    
    m_status = Initializing;
    emit statusChanged(m_status);

    // 创建遗留媒体管理器
    createLegacyMediaManager();
    
    if (!m_legacyManager) {
        qWarning() << "Failed to create legacy MediaManager";
        m_status = Error;
        emit statusChanged(m_status);
        return false;
    }

    // 初始化遗留管理器
    if (!m_legacyManager->initialize()) {
        qWarning() << "Failed to initialize legacy MediaManager";
        m_status = Error;
        emit statusChanged(m_status);
        return false;
    }

    m_status = Ready;
    emit statusChanged(m_status);
    
    qDebug() << "MediaManagerAdapter initialized successfully";
    return true;
}

ICompatibilityAdapter::AdapterStatus MediaManagerAdapter::status() const
{
    return m_status;
}

QString MediaManagerAdapter::adapterName() const
{
    return "MediaManagerAdapter";
}

QString MediaManagerAdapter::targetModule() const
{
    return "audio,camera";
}

ICompatibilityAdapter::CompatibilityLevel MediaManagerAdapter::checkCompatibility()
{
    if (m_status != Ready) {
        return NoCompatibility;
    }

    bool audioValid = validateAudioIntegration();
    bool cameraValid = validateCameraIntegration();

    if (audioValid && cameraValid) {
        return FullCompatibility;
    } else if (audioValid || cameraValid) {
        return PartialCompatibility;
    } else {
        return NoCompatibility;
    }
}

bool MediaManagerAdapter::enable()
{
    if (m_status != Ready) {
        return false;
    }

    m_status = Active;
    emit statusChanged(m_status);
    
    qDebug() << "MediaManagerAdapter enabled";
    return true;
}

void MediaManagerAdapter::disable()
{
    if (m_status == Active) {
        m_status = Ready;
        emit statusChanged(m_status);
        qDebug() << "MediaManagerAdapter disabled";
    }
}

QVariantMap MediaManagerAdapter::getConfiguration() const
{
    return m_config;
}

bool MediaManagerAdapter::setConfiguration(const QVariantMap& config)
{
    m_config = config;
    
    // 应用配置到遗留管理器
    if (m_legacyManager) {
        // 这里应该根据配置调整遗留管理器的行为
        qDebug() << "Applied configuration to MediaManagerAdapter";
    }
    
    return true;
}

QStringList MediaManagerAdapter::validateFunctionality()
{
    QStringList results;
    
    if (!m_legacyManager) {
        results << "ERROR: Legacy MediaManager not created";
        return results;
    }

    // 验证基本功能
    try {
        if (m_legacyManager->startAudio()) {
            results << "PASS: Audio start functionality";
            m_legacyManager->stopAudio();
        } else {
            results << "FAIL: Audio start functionality";
        }

        if (m_legacyManager->startVideo()) {
            results << "PASS: Video start functionality";
            m_legacyManager->stopVideo();
        } else {
            results << "FAIL: Video start functionality";
        }

        // 验证音量控制
        qreal originalVolume = m_legacyManager->getVolume();
        m_legacyManager->setVolume(0.5);
        if (qAbs(m_legacyManager->getVolume() - 0.5) < 0.1) {
            results << "PASS: Volume control functionality";
        } else {
            results << "FAIL: Volume control functionality";
        }
        m_legacyManager->setVolume(originalVolume);

    } catch (const std::exception& e) {
        results << QString("ERROR: Exception during validation: %1").arg(e.what());
    } catch (...) {
        results << "ERROR: Unknown exception during validation";
    }

    // 验证集成
    if (validateAudioIntegration()) {
        results << "PASS: Audio module integration";
    } else {
        results << "FAIL: Audio module integration";
    }

    if (validateCameraIntegration()) {
        results << "PASS: Camera module integration";
    } else {
        results << "FAIL: Camera module integration";
    }

    return results;
}

MediaManager* MediaManagerAdapter::getLegacyManager() const
{
    return m_legacyManager;
}

void MediaManagerAdapter::setAudioManager(AudioManager* audioManager)
{
    if (m_audioManager) {
        disconnect(m_audioManager, &AudioManager::statusChanged,
                   this, &MediaManagerAdapter::onAudioStatusChanged);
    }

    m_audioManager = audioManager;
    
    if (m_audioManager) {
        connect(m_audioManager, &AudioManager::statusChanged,
                this, &MediaManagerAdapter::onAudioStatusChanged);
        m_audioIntegrationValid = validateAudioIntegration();
    }
}

void MediaManagerAdapter::setCameraManager(CameraManager* cameraManager)
{
    if (m_cameraManager) {
        disconnect(m_cameraManager, &CameraManager::statusChanged,
                   this, &MediaManagerAdapter::onCameraStatusChanged);
    }

    m_cameraManager = cameraManager;
    
    if (m_cameraManager) {
        connect(m_cameraManager, &CameraManager::statusChanged,
                this, &MediaManagerAdapter::onCameraStatusChanged);
        m_cameraIntegrationValid = validateCameraIntegration();
    }
}

void MediaManagerAdapter::onAudioStatusChanged()
{
    m_audioIntegrationValid = validateAudioIntegration();
    qDebug() << "Audio integration status changed:" << m_audioIntegrationValid;
}

void MediaManagerAdapter::onCameraStatusChanged()
{
    m_cameraIntegrationValid = validateCameraIntegration();
    qDebug() << "Camera integration status changed:" << m_cameraIntegrationValid;
}

void MediaManagerAdapter::createLegacyMediaManager()
{
    if (m_legacyManager) {
        m_legacyManager->deleteLater();
    }

    m_legacyManager = new MediaManager(this);
    qDebug() << "Created legacy MediaManager";
}

bool MediaManagerAdapter::validateAudioIntegration()
{
    if (!m_audioManager) {
        return false;
    }

    // 验证音频管理器是否正常工作
    return m_audioManager->initialize() && m_audioManager->isActive();
}

bool MediaManagerAdapter::validateCameraIntegration()
{
    if (!m_cameraManager) {
        return false;
    }

    // 验证相机管理器是否正常工作
    return m_cameraManager->initialize() && m_cameraManager->isActive();
}

#include "MediaManagerAdapter.moc"