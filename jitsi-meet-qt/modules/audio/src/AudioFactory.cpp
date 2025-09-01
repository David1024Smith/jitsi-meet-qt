#include "AudioFactory.h"
#include "AudioModule.h"
#include "AudioManager.h"
#include "IAudioDevice.h"
#include "platform/QtAudioDevice.h"
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioInput>
#include <QAudioOutput>
#include <functional>

/**
 * @brief AudioFactory私有实现类
 */
class AudioFactory::AudioFactoryPrivate
{
public:
    AudioFactoryPrivate()
        : initialized(false)
    {
    }

    ~AudioFactoryPrivate()
    {
        cleanup();
    }

    void cleanup()
    {
        inputDevices.clear();
        outputDevices.clear();
        deviceInfoMap.clear();
        customCreators.clear();
        
        // 清理创建的对象
        for (auto *obj : createdObjects) {
            if (obj) {
                obj->deleteLater();
            }
        }
        createdObjects.clear();
    }

    bool initialized;
    mutable QMutex mutex;

    // 设备列表
    QStringList inputDevices;
    QStringList outputDevices;
    QString defaultInputDevice;
    QString defaultOutputDevice;

    // 设备信息缓存
    QMap<QString, QVariantMap> deviceInfoMap;

    // 自定义创建器
    QMap<AudioFactory::DeviceType, std::function<IAudioDevice*(const QString&, QObject*)>> customCreators;

    // 创建的对象跟踪
    QList<QObject*> createdObjects;
};

// 静态成员初始化
AudioFactory* AudioFactory::s_instance = nullptr;

AudioFactory* AudioFactory::instance()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    
    if (!s_instance) {
        s_instance = new AudioFactory();
        s_instance->initialize();
    }
    
    return s_instance;
}

AudioFactory::AudioFactory(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<AudioFactoryPrivate>())
{
    qDebug() << "AudioFactory created";
}

AudioFactory::~AudioFactory()
{
    qDebug() << "AudioFactory destroyed";
}

void AudioFactory::initialize()
{
    Q_D(AudioFactory);
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        return;
    }
    
    try {
        // 扫描可用设备
        scanDevices();
        
        d->initialized = true;
        qDebug() << "AudioFactory initialized successfully";
        
    } catch (const std::exception &e) {
        qWarning() << "AudioFactory initialization failed:" << e.what();
    }
}

AudioModule* AudioFactory::createAudioModule(QObject *parent)
{
    Q_D(AudioFactory);
    QMutexLocker locker(&d->mutex);
    
    try {
        AudioModule *module = new AudioModule(parent);
        d->createdObjects.append(module);
        
        qDebug() << "Created AudioModule";
        return module;
        
    } catch (const std::exception &e) {
        qWarning() << "Failed to create AudioModule:" << e.what();
        return nullptr;
    }
}

AudioManager* AudioFactory::createAudioManager(QObject *parent)
{
    Q_D(AudioFactory);
    QMutexLocker locker(&d->mutex);
    
    try {
        AudioManager *manager = new AudioManager(parent);
        d->createdObjects.append(manager);
        
        qDebug() << "Created AudioManager";
        return manager;
        
    } catch (const std::exception &e) {
        qWarning() << "Failed to create AudioManager:" << e.what();
        return nullptr;
    }
}

IAudioDevice* AudioFactory::createAudioDevice(const QString &deviceId, 
                                             DeviceType deviceType, 
                                             QObject *parent)
{
    Q_D(AudioFactory);
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "AudioFactory not initialized";
        return nullptr;
    }
    
    // 检查设备是否可用
    if (!isDeviceAvailable(deviceId, deviceType)) {
        qWarning() << "Device not available:" << deviceId;
        return nullptr;
    }
    
    try {
        IAudioDevice *device = nullptr;
        
        // 检查是否有自定义创建器
        if (d->customCreators.contains(deviceType)) {
            device = d->customCreators[deviceType](deviceId, parent);
        } else {
            // 使用默认的平台特定创建器
            device = createPlatformDevice(deviceId, deviceType, parent);
        }
        
        if (device) {
            d->createdObjects.append(device);
            qDebug() << "Created audio device:" << deviceId << "Type:" << deviceType;
        }
        
        return device;
        
    } catch (const std::exception &e) {
        qWarning() << "Failed to create audio device:" << deviceId << e.what();
        return nullptr;
    }
}

QStringList AudioFactory::availableDevices(DeviceType deviceType) const
{
    Q_D(const AudioFactory);
    QMutexLocker locker(&d->mutex);
    
    switch (deviceType) {
    case InputDevice:
        return d->inputDevices;
    case OutputDevice:
        return d->outputDevices;
    default:
        return QStringList();
    }
}

QString AudioFactory::defaultDevice(DeviceType deviceType) const
{
    Q_D(const AudioFactory);
    QMutexLocker locker(&d->mutex);
    
    switch (deviceType) {
    case InputDevice:
        return d->defaultInputDevice;
    case OutputDevice:
        return d->defaultOutputDevice;
    default:
        return QString();
    }
}

bool AudioFactory::isDeviceAvailable(const QString &deviceId, DeviceType deviceType) const
{
    Q_D(const AudioFactory);
    QMutexLocker locker(&d->mutex);
    
    switch (deviceType) {
    case InputDevice:
        return d->inputDevices.contains(deviceId);
    case OutputDevice:
        return d->outputDevices.contains(deviceId);
    default:
        return false;
    }
}

QVariantMap AudioFactory::deviceInfo(const QString &deviceId) const
{
    Q_D(const AudioFactory);
    QMutexLocker locker(&d->mutex);
    
    return d->deviceInfoMap.value(deviceId);
}

void AudioFactory::registerDeviceCreator(DeviceType deviceType, 
                                        std::function<IAudioDevice*(const QString&, QObject*)> creator)
{
    Q_D(AudioFactory);
    QMutexLocker locker(&d->mutex);
    
    d->customCreators[deviceType] = creator;
    qDebug() << "Registered custom device creator for type:" << deviceType;
}

void AudioFactory::cleanup()
{
    Q_D(AudioFactory);
    QMutexLocker locker(&d->mutex);
    
    d->cleanup();
    qDebug() << "AudioFactory cleanup completed";
}

void AudioFactory::scanDevices()
{
    Q_D(AudioFactory);
    
    // 清空现有列表
    d->inputDevices.clear();
    d->outputDevices.clear();
    d->deviceInfoMap.clear();
    
    // 获取媒体设备管理器
    QMediaDevices *mediaDevices = new QMediaDevices(this);
    
    // 扫描输入设备
    QList<QAudioDevice> inputDeviceInfos = mediaDevices->audioInputs();
    for (const QAudioDevice &info : inputDeviceInfos) {
        QString deviceId = info.id();
        d->inputDevices.append(deviceId);
        
        QVariantMap deviceInfo;
        deviceInfo["id"] = deviceId;
        deviceInfo["name"] = info.description();
        deviceInfo["type"] = "input";
        deviceInfo["isDefault"] = info.isDefault();
        
        // 添加支持的格式信息
        QVariantList supportedSampleRates;
        QList<int> rates = {8000, 16000, 22050, 44100, 48000, 96000}; // 常见采样率
        for (int rate : rates) {
            if (info.minimumSampleRate() <= rate && rate <= info.maximumSampleRate()) {
                supportedSampleRates.append(rate);
            }
        }
        deviceInfo["supportedSampleRates"] = supportedSampleRates;
        
        QVariantList supportedChannelCounts;
        for (int channels = info.minimumChannelCount(); channels <= info.maximumChannelCount(); ++channels) {
            supportedChannelCounts.append(channels);
        }
        deviceInfo["supportedChannelCounts"] = supportedChannelCounts;
        
        // 在Qt6中，不再直接提供supportedSampleSizes，我们使用常见的值
        QVariantList supportedSampleSizes;
        QList<int> sizes = {8, 16, 24, 32};
        for (int size : sizes) {
            supportedSampleSizes.append(size);
        }
        deviceInfo["supportedSampleSizes"] = supportedSampleSizes;
        
        d->deviceInfoMap[deviceId] = deviceInfo;
        
        // 设置默认设备
        if (info.isDefault()) {
            d->defaultInputDevice = deviceId;
        }
    }
    
    // 扫描输出设备
    QList<QAudioDevice> outputDeviceInfos = mediaDevices->audioOutputs();
    for (const QAudioDevice &info : outputDeviceInfos) {
        QString deviceId = info.id();
        d->outputDevices.append(deviceId);
        
        QVariantMap deviceInfo;
        deviceInfo["id"] = deviceId;
        deviceInfo["name"] = info.description();
        deviceInfo["type"] = "output";
        deviceInfo["isDefault"] = info.isDefault();
        
        // 添加支持的格式信息
        QVariantList supportedSampleRates;
        QList<int> rates = {8000, 16000, 22050, 44100, 48000, 96000}; // 常见采样率
        for (int rate : rates) {
            if (info.minimumSampleRate() <= rate && rate <= info.maximumSampleRate()) {
                supportedSampleRates.append(rate);
            }
        }
        deviceInfo["supportedSampleRates"] = supportedSampleRates;
        
        QVariantList supportedChannelCounts;
        for (int channels = info.minimumChannelCount(); channels <= info.maximumChannelCount(); ++channels) {
            supportedChannelCounts.append(channels);
        }
        deviceInfo["supportedChannelCounts"] = supportedChannelCounts;
        
        // 在Qt6中，不再直接提供supportedSampleSizes，我们使用常见的值
        QVariantList supportedSampleSizes;
        QList<int> sizes = {8, 16, 24, 32};
        for (int size : sizes) {
            supportedSampleSizes.append(size);
        }
        deviceInfo["supportedSampleSizes"] = supportedSampleSizes;
        
        d->deviceInfoMap[deviceId] = deviceInfo;
        
        // 设置默认设备
        if (info.isDefault()) {
            d->defaultOutputDevice = deviceId;
        }
    }
    
    emit devicesChanged();
    
    qDebug() << "Device scan completed - Input devices:" << d->inputDevices.size() 
             << "Output devices:" << d->outputDevices.size();
}

IAudioDevice* AudioFactory::createPlatformDevice(const QString &deviceId, 
                                                DeviceType deviceType, 
                                                QObject *parent)
{
    // 这里应该创建平台特定的音频设备实现
    // 为了简化，我们创建一个基本的Qt音频设备包装器
    
    try {
        QtAudioDevice *device = new QtAudioDevice(deviceId, deviceType, parent);
        return device;
        
    } catch (const std::exception &e) {
        qWarning() << "Failed to create platform device:" << deviceId << e.what();
        return nullptr;
    }
}

