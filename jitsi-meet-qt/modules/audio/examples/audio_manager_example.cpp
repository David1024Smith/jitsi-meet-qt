#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "AudioManager.h"
#include "AudioFactory.h"

/**
 * @brief 音频管理器示例程序
 * 
 * 演示如何使用AudioManager和AudioFactory创建和管理音频设备
 */
class AudioExample : public QObject
{
    Q_OBJECT

public:
    AudioExample(QObject *parent = nullptr)
        : QObject(parent)
        , m_audioManager(nullptr)
    {
    }

    void run()
    {
        qDebug() << "=== Audio Manager Example ===";
        
        // 创建音频管理器
        AudioFactory *factory = AudioFactory::instance();
        m_audioManager = factory->createAudioManager(this);
        
        if (!m_audioManager) {
            qCritical() << "Failed to create AudioManager";
            QCoreApplication::quit();
            return;
        }
        
        // 连接信号
        connect(m_audioManager, &AudioManager::statusChanged,
                this, &AudioExample::onStatusChanged);
        connect(m_audioManager, &AudioManager::devicesUpdated,
                this, &AudioExample::onDevicesUpdated);
        connect(m_audioManager, &AudioManager::audioStarted,
                this, &AudioExample::onAudioStarted);
        connect(m_audioManager, &AudioManager::audioStopped,
                this, &AudioExample::onAudioStopped);
        connect(m_audioManager, &AudioManager::errorOccurred,
                this, &AudioExample::onError);
        
        // 初始化音频管理器
        qDebug() << "Initializing AudioManager...";
        if (!m_audioManager->initialize()) {
            qCritical() << "Failed to initialize AudioManager";
            QCoreApplication::quit();
            return;
        }
        
        // 设置定时器来演示功能
        QTimer::singleShot(1000, this, &AudioExample::demonstrateFeatures);
    }

private slots:
    void onStatusChanged(AudioManager::ManagerStatus status)
    {
        qDebug() << "AudioManager status changed:" << status;
    }

    void onDevicesUpdated()
    {
        qDebug() << "Audio devices updated";
        listDevices();
    }

    void onAudioStarted()
    {
        qDebug() << "Audio started successfully";
    }

    void onAudioStopped()
    {
        qDebug() << "Audio stopped";
    }

    void onError(const QString &error)
    {
        qWarning() << "AudioManager error:" << error;
    }

    void demonstrateFeatures()
    {
        if (!m_audioManager) {
            return;
        }
        
        qDebug() << "\n=== Demonstrating Audio Features ===";
        
        // 列出可用设备
        listDevices();
        
        // 演示音量控制
        demonstrateVolumeControl();
        
        // 演示质量预设
        demonstrateQualityPresets();
        
        // 尝试启动音频
        demonstrateAudioControl();
        
        // 5秒后退出
        QTimer::singleShot(5000, this, &AudioExample::cleanup);
    }

    void listDevices()
    {
        qDebug() << "\n--- Available Audio Devices ---";
        
        QStringList inputDevices = m_audioManager->availableInputDevices();
        qDebug() << "Input devices:" << inputDevices.size();
        for (const QString &device : inputDevices) {
            qDebug() << "  -" << device << "(" << m_audioManager->deviceDisplayName(device) << ")";
        }
        
        QStringList outputDevices = m_audioManager->availableOutputDevices();
        qDebug() << "Output devices:" << outputDevices.size();
        for (const QString &device : outputDevices) {
            qDebug() << "  -" << device << "(" << m_audioManager->deviceDisplayName(device) << ")";
        }
        
        qDebug() << "Current input device:" << m_audioManager->currentInputDevice();
        qDebug() << "Current output device:" << m_audioManager->currentOutputDevice();
    }

    void demonstrateVolumeControl()
    {
        qDebug() << "\n--- Volume Control Demo ---";
        
        qDebug() << "Current master volume:" << m_audioManager->masterVolume();
        qDebug() << "Current microphone gain:" << m_audioManager->microphoneGain();
        qDebug() << "Is muted:" << m_audioManager->isMuted();
        
        // 设置音量
        m_audioManager->setMasterVolume(0.8);
        qDebug() << "Set master volume to 0.8";
        
        m_audioManager->setMicrophoneGain(0.6);
        qDebug() << "Set microphone gain to 0.6";
        
        // 测试静音
        m_audioManager->setMuted(true);
        qDebug() << "Muted audio";
        
        QTimer::singleShot(1000, [this]() {
            m_audioManager->setMuted(false);
            qDebug() << "Unmuted audio";
        });
    }

    void demonstrateQualityPresets()
    {
        qDebug() << "\n--- Quality Presets Demo ---";
        
        qDebug() << "Current quality preset:" << m_audioManager->qualityPreset();
        
        // 测试不同质量预设
        m_audioManager->setQualityPreset(AudioManager::LowQuality);
        qDebug() << "Set to Low Quality";
        
        QTimer::singleShot(500, [this]() {
            m_audioManager->setQualityPreset(AudioManager::StandardQuality);
            qDebug() << "Set to Standard Quality";
        });
        
        QTimer::singleShot(1000, [this]() {
            m_audioManager->setQualityPreset(AudioManager::HighQuality);
            qDebug() << "Set to High Quality";
        });
    }

    void demonstrateAudioControl()
    {
        qDebug() << "\n--- Audio Control Demo ---";
        
        qDebug() << "Is audio active:" << m_audioManager->isAudioActive();
        
        // 尝试启动音频
        if (m_audioManager->startAudio()) {
            qDebug() << "Audio started successfully";
            
            // 2秒后停止
            QTimer::singleShot(2000, [this]() {
                m_audioManager->stopAudio();
                qDebug() << "Audio stopped";
            });
        } else {
            qDebug() << "Failed to start audio";
        }
    }

    void cleanup()
    {
        qDebug() << "\n=== Cleaning up ===";
        
        if (m_audioManager && m_audioManager->isAudioActive()) {
            m_audioManager->stopAudio();
        }
        
        qDebug() << "Example completed successfully";
        QCoreApplication::quit();
    }

private:
    AudioManager *m_audioManager;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Starting Audio Manager Example";
    
    AudioExample example;
    example.run();
    
    return app.exec();
}

#include "audio_manager_example.moc"