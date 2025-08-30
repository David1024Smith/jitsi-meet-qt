#include "AudioManager.h"
#include "AudioConfig.h"
#include "IAudioDevice.h"
#include "AudioFactory.h"
#include <QDebug>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>

/**
 * @brief AudioManager私有实现类
 */
class AudioManager::AudioManagerPrivate
{
public:
    AudioManagerPrivate(AudioManager *q)
        : q_ptr(q)
        , status(AudioManager::Uninitialized)
        , masterVolume(1.0)
        , microphoneGain(1.0)
        , muted(false)
        , qualityPreset(AudioManager::StandardQuality)
        , audioActive(false)
        , audioConfig(nullptr)
        , refreshTimer(nullptr)
    {
    }

    ~AudioManagerPrivate()
    {
        cleanup();
    }

    void cleanup()
    {
        if (refreshTimer) {
            refreshTimer->stop();
            refreshTimer->deleteLater();
            refreshTimer = nullptr;
        }

        inputDevices.clear();
        outputDevices.clear();
        
        if (currentInputDevicePtr) {
            currentInputDevicePtr->deleteLater();
            currentInputDevicePtr = nullptr;
        }
        
        if (currentOutputDevicePtr) {
            currentOutputDevicePtr->deleteLater();
            currentOutputDevicePtr = nullptr;
        }

        if (audioConfig) {
            audioConfig->deleteLater();
            audioConfig = nullptr;
        }
    }

    AudioManager *q_ptr;
    Q_DECLARE_PUBLIC(AudioManager)

    // 状态管理
    AudioManager::ManagerStatus status;
    QString lastError;
    QMutex mutex;

    // 音频控制
    qreal masterVolume;
    qreal microphoneGain;
    bool muted;
    AudioManager::QualityPreset qualityPreset;
    bool audioActive;

    // 设备管理
    QStringList inputDevices;
    QStringList outputDevices;
    QString currentInputDeviceId;
    QString currentOutputDeviceId;
    IAudioDevice *currentInputDevicePtr = nullptr;
    IAudioDevice *currentOutputDevicePtr = nullptr;

    // 配置和定时器
    AudioConfig *audioConfig;
    QTimer *refreshTimer;

    // 设备信息缓存
    QMap<QString, QString> deviceNames;
    QMap<QString, QVariantMap> deviceInfoCache;
};

AudioManager::AudioManager(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<AudioManagerPrivate>(this))
{
    Q_D(AudioManager);
    
    // 创建音频配置
    d->audioConfig = new AudioConfig(this);
    
    // 创建设备刷新定时器
    d->refreshTimer = new QTimer(this);
    d->refreshTimer->setInterval(5000); // 每5秒刷新一次设备列表
    connect(d->refreshTimer, &QTimer::timeout, this, &AudioManager::refreshDevices);
    
    qDebug() << "AudioManager created";
}

AudioManager::~AudioManager()
{
    Q_D(AudioManager);
    
    if (d->audioActive) {
        stopAudio();
    }
    
    qDebug() << "AudioManager destroyed";
}

bool AudioManager::initialize()
{
    Q_D(AudioManager);
    QMutexLocker locker(&d->mutex);
    
    if (d->status == Ready) {
        return true;
    }
    
    d->status = Busy;
    emit statusChanged(d->status);
    
    try {
        // 初始化音频配置
        if (!d->audioConfig->load()) {
            qWarning() << "Failed to load audio configuration, using defaults";
        }
        
        // 刷新设备列表
        refreshDevices();
        
        // 设置默认设备
        if (!d->inputDevices.isEmpty() && d->currentInputDeviceId.isEmpty()) {
            selectInputDevice(d->inputDevices.first());
        }
        
        if (!d->outputDevices.isEmpty() && d->currentOutputDeviceId.isEmpty()) {
            selectOutputDevice(d->outputDevices.first());
        }
        
        // 应用当前质量预设
        applyQualityPreset(d->qualityPreset);
        
        // 启动设备监控定时器
        d->refreshTimer->start();
        
        d->status = Ready;
        emit statusChanged(d->status);
        
        qDebug() << "AudioManager initialized successfully";
        return true;
        
    } catch (const std::exception &e) {
        d->lastError = QString("Initialization failed: %1").arg(e.what());
        d->status = Error;
        emit statusChanged(d->status);
        emit errorOccurred(d->lastError);
        return false;
    }
}

AudioManager::ManagerStatus AudioManager::status() const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->status;
}

QStringList AudioManager::availableInputDevices() const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->inputDevices;
}

QStringList AudioManager::availableOutputDevices() const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->outputDevices;
}

QString AudioManager::deviceDisplayName(const QString &deviceId) const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->deviceNames.value(deviceId, deviceId);
}

bool AudioManager::selectInputDevice(const QString &deviceId)
{
    Q_D(AudioManager);
    QMutexLocker locker(&d->mutex);
    
    if (deviceId == d->currentInputDeviceId) {
        return true;
    }
    
    if (!d->inputDevices.contains(deviceId)) {
        d->lastError = QString("Input device not found: %1").arg(deviceId);
        emit errorOccurred(d->lastError);
        return false;
    }
    
    try {
        // 停止当前设备
        if (d->currentInputDevicePtr) {
            d->currentInputDevicePtr->stop();
            d->currentInputDevicePtr->deleteLater();
            d->currentInputDevicePtr = nullptr;
        }
        
        // 创建新设备
        AudioFactory *factory = AudioFactory::instance();
        d->currentInputDevicePtr = factory->createAudioDevice(
            deviceId, AudioFactory::InputDevice, this);
        
        if (!d->currentInputDevicePtr) {
            d->lastError = QString("Failed to create input device: %1").arg(deviceId);
            emit errorOccurred(d->lastError);
            return false;
        }
        
        // 连接设备信号
        connect(d->currentInputDevicePtr, &IAudioDevice::statusChanged,
                this, &AudioManager::onDeviceStatusChanged);
        connect(d->currentInputDevicePtr, &IAudioDevice::errorOccurred,
                this, &AudioManager::onDeviceError);
        
        // 初始化设备
        if (!d->currentInputDevicePtr->initialize()) {
            d->lastError = QString("Failed to initialize input device: %1").arg(deviceId);
            emit errorOccurred(d->lastError);
            return false;
        }
        
        d->currentInputDeviceId = deviceId;
        emit deviceChanged(deviceId, true);
        
        qDebug() << "Selected input device:" << deviceId;
        return true;
        
    } catch (const std::exception &e) {
        d->lastError = QString("Error selecting input device: %1").arg(e.what());
        emit errorOccurred(d->lastError);
        return false;
    }
}

bool AudioManager::selectOutputDevice(const QString &deviceId)
{
    Q_D(AudioManager);
    QMutexLocker locker(&d->mutex);
    
    if (deviceId == d->currentOutputDeviceId) {
        return true;
    }
    
    if (!d->outputDevices.contains(deviceId)) {
        d->lastError = QString("Output device not found: %1").arg(deviceId);
        emit errorOccurred(d->lastError);
        return false;
    }
    
    try {
        // 停止当前设备
        if (d->currentOutputDevicePtr) {
            d->currentOutputDevicePtr->stop();
            d->currentOutputDevicePtr->deleteLater();
            d->currentOutputDevicePtr = nullptr;
        }
        
        // 创建新设备
        AudioFactory *factory = AudioFactory::instance();
        d->currentOutputDevicePtr = factory->createAudioDevice(
            deviceId, AudioFactory::OutputDevice, this);
        
        if (!d->currentOutputDevicePtr) {
            d->lastError = QString("Failed to create output device: %1").arg(deviceId);
            emit errorOccurred(d->lastError);
            return false;
        }
        
        // 连接设备信号
        connect(d->currentOutputDevicePtr, &IAudioDevice::statusChanged,
                this, &AudioManager::onDeviceStatusChanged);
        connect(d->currentOutputDevicePtr, &IAudioDevice::errorOccurred,
                this, &AudioManager::onDeviceError);
        
        // 初始化设备
        if (!d->currentOutputDevicePtr->initialize()) {
            d->lastError = QString("Failed to initialize output device: %1").arg(deviceId);
            emit errorOccurred(d->lastError);
            return false;
        }
        
        d->currentOutputDeviceId = deviceId;
        emit deviceChanged(deviceId, false);
        
        qDebug() << "Selected output device:" << deviceId;
        return true;
        
    } catch (const std::exception &e) {
        d->lastError = QString("Error selecting output device: %1").arg(e.what());
        emit errorOccurred(d->lastError);
        return false;
    }
}

QString AudioManager::currentInputDevice() const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->currentInputDeviceId;
}

QString AudioManager::currentOutputDevice() const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->currentOutputDeviceId;
}

void AudioManager::setMasterVolume(qreal volume)
{
    Q_D(AudioManager);
    QMutexLocker locker(&d->mutex);
    
    volume = qBound(0.0, volume, 1.0);
    
    if (qAbs(d->masterVolume - volume) < 0.001) {
        return;
    }
    
    d->masterVolume = volume;
    
    // 应用到输出设备
    if (d->currentOutputDevicePtr) {
        d->currentOutputDevicePtr->setVolume(volume);
    }
    
    emit volumeChanged(volume);
    qDebug() << "Master volume set to:" << volume;
}

qreal AudioManager::masterVolume() const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->masterVolume;
}

void AudioManager::setMicrophoneGain(qreal gain)
{
    Q_D(AudioManager);
    QMutexLocker locker(&d->mutex);
    
    gain = qBound(0.0, gain, 1.0);
    
    if (qAbs(d->microphoneGain - gain) < 0.001) {
        return;
    }
    
    d->microphoneGain = gain;
    
    // 应用到输入设备
    if (d->currentInputDevicePtr) {
        d->currentInputDevicePtr->setVolume(gain);
    }
    
    qDebug() << "Microphone gain set to:" << gain;
}

qreal AudioManager::microphoneGain() const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->microphoneGain;
}

void AudioManager::setMuted(bool muted)
{
    Q_D(AudioManager);
    QMutexLocker locker(&d->mutex);
    
    if (d->muted == muted) {
        return;
    }
    
    d->muted = muted;
    
    // 应用到所有设备
    if (d->currentInputDevicePtr) {
        d->currentInputDevicePtr->setMuted(muted);
    }
    
    if (d->currentOutputDevicePtr) {
        d->currentOutputDevicePtr->setMuted(muted);
    }
    
    emit muteChanged(muted);
    qDebug() << "Mute state set to:" << muted;
}

bool AudioManager::isMuted() const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->muted;
}

void AudioManager::setQualityPreset(QualityPreset preset)
{
    Q_D(AudioManager);
    QMutexLocker locker(&d->mutex);
    
    if (d->qualityPreset == preset) {
        return;
    }
    
    d->qualityPreset = preset;
    applyQualityPreset(preset);
    
    qDebug() << "Quality preset set to:" << preset;
}

AudioManager::QualityPreset AudioManager::qualityPreset() const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->qualityPreset;
}

bool AudioManager::startAudio()
{
    Q_D(AudioManager);
    QMutexLocker locker(&d->mutex);
    
    if (d->audioActive) {
        return true;
    }
    
    if (d->status != Ready) {
        d->lastError = "AudioManager not ready";
        emit errorOccurred(d->lastError);
        return false;
    }
    
    try {
        // 启动输入设备
        if (d->currentInputDevicePtr && !d->currentInputDevicePtr->start()) {
            d->lastError = "Failed to start input device";
            emit errorOccurred(d->lastError);
            return false;
        }
        
        // 启动输出设备
        if (d->currentOutputDevicePtr && !d->currentOutputDevicePtr->start()) {
            d->lastError = "Failed to start output device";
            emit errorOccurred(d->lastError);
            return false;
        }
        
        d->audioActive = true;
        emit audioStarted();
        
        qDebug() << "Audio started successfully";
        return true;
        
    } catch (const std::exception &e) {
        d->lastError = QString("Error starting audio: %1").arg(e.what());
        emit errorOccurred(d->lastError);
        return false;
    }
}

void AudioManager::stopAudio()
{
    Q_D(AudioManager);
    QMutexLocker locker(&d->mutex);
    
    if (!d->audioActive) {
        return;
    }
    
    try {
        // 停止输入设备
        if (d->currentInputDevicePtr) {
            d->currentInputDevicePtr->stop();
        }
        
        // 停止输出设备
        if (d->currentOutputDevicePtr) {
            d->currentOutputDevicePtr->stop();
        }
        
        d->audioActive = false;
        emit audioStopped();
        
        qDebug() << "Audio stopped";
        
    } catch (const std::exception &e) {
        qWarning() << "Error stopping audio:" << e.what();
    }
}

bool AudioManager::isAudioActive() const
{
    Q_D(const AudioManager);
    QMutexLocker locker(&d->mutex);
    return d->audioActive;
}

AudioConfig* AudioManager::audioConfig() const
{
    Q_D(const AudioManager);
    return d->audioConfig;
}

void AudioManager::onDeviceStatusChanged()
{
    // 处理设备状态变化
    IAudioDevice *device = qobject_cast<IAudioDevice*>(sender());
    if (device) {
        qDebug() << "Device status changed:" << device->deviceId() << device->status();
    }
}

void AudioManager::onDeviceError(const QString &error)
{
    IAudioDevice *device = qobject_cast<IAudioDevice*>(sender());
    QString deviceInfo = device ? device->deviceId() : "Unknown device";
    
    QString fullError = QString("Device error [%1]: %2").arg(deviceInfo, error);
    qWarning() << fullError;
    emit errorOccurred(fullError);
}

void AudioManager::refreshDevices()
{
    Q_D(AudioManager);
    
    AudioFactory *factory = AudioFactory::instance();
    
    // 获取新的设备列表
    QStringList newInputDevices = factory->availableDevices(AudioFactory::InputDevice);
    QStringList newOutputDevices = factory->availableDevices(AudioFactory::OutputDevice);
    
    bool devicesChanged = false;
    
    // 检查输入设备变化
    if (newInputDevices != d->inputDevices) {
        d->inputDevices = newInputDevices;
        devicesChanged = true;
        
        // 更新设备名称缓存
        for (const QString &deviceId : newInputDevices) {
            QVariantMap info = factory->deviceInfo(deviceId);
            d->deviceNames[deviceId] = info.value("name", deviceId).toString();
            d->deviceInfoCache[deviceId] = info;
        }
    }
    
    // 检查输出设备变化
    if (newOutputDevices != d->outputDevices) {
        d->outputDevices = newOutputDevices;
        devicesChanged = true;
        
        // 更新设备名称缓存
        for (const QString &deviceId : newOutputDevices) {
            QVariantMap info = factory->deviceInfo(deviceId);
            d->deviceNames[deviceId] = info.value("name", deviceId).toString();
            d->deviceInfoCache[deviceId] = info;
        }
    }
    
    if (devicesChanged) {
        emit devicesUpdated();
        qDebug() << "Devices updated - Input:" << d->inputDevices.size() 
                 << "Output:" << d->outputDevices.size();
    }
}

void AudioManager::applyQualityPreset(QualityPreset preset)
{
    Q_D(AudioManager);
    
    int sampleRate = 44100;
    int channels = 2;
    int bufferSize = 1024;
    
    switch (preset) {
    case LowQuality:
        sampleRate = 16000;
        channels = 1;
        bufferSize = 2048;
        break;
    case StandardQuality:
        sampleRate = 44100;
        channels = 2;
        bufferSize = 1024;
        break;
    case HighQuality:
        sampleRate = 48000;
        channels = 2;
        bufferSize = 512;
        break;
    }
    
    // 应用到当前设备
    if (d->currentInputDevicePtr) {
        d->currentInputDevicePtr->setSampleRate(sampleRate);
        d->currentInputDevicePtr->setChannels(channels);
        d->currentInputDevicePtr->setBufferSize(bufferSize);
    }
    
    if (d->currentOutputDevicePtr) {
        d->currentOutputDevicePtr->setSampleRate(sampleRate);
        d->currentOutputDevicePtr->setChannels(channels);
        d->currentOutputDevicePtr->setBufferSize(bufferSize);
    }
    
    qDebug() << "Applied quality preset:" << preset 
             << "SampleRate:" << sampleRate 
             << "Channels:" << channels 
             << "BufferSize:" << bufferSize;
}