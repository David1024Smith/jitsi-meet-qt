#include "AudioConfig.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

AudioConfig::AudioConfig(QObject *parent)
    : QObject(parent)
    , m_sampleRate(48000)
    , m_channels(2)
    , m_bufferSize(1024)
    , m_bitrate(128)
    , m_qualityPreset(StandardQuality)
    , m_masterVolume(0.8)
    , m_microphoneGain(0.6)
    , m_muted(false)
    , m_noiseSuppressionEnabled(true)
    , m_echoCancellationEnabled(true)
    , m_autoGainControlEnabled(false)
    , m_settings(nullptr)
{
    initializeDefaults();
    
    // 设置默认配置文件路径
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    m_configFilePath = configDir + "/audio_config.ini";
    
    m_settings = new QSettings(m_configFilePath, QSettings::IniFormat, this);
}

AudioConfig::~AudioConfig()
{
    if (m_settings) {
        save();
    }
}

bool AudioConfig::load()
{
    if (!m_settings) {
        return false;
    }

    m_settings->beginGroup("Audio");
    
    // 加载设备配置
    m_preferredInputDevice = m_settings->value("PreferredInputDevice", "").toString();
    m_preferredOutputDevice = m_settings->value("PreferredOutputDevice", "").toString();
    
    // 加载音频质量配置
    m_sampleRate = m_settings->value("SampleRate", 48000).toInt();
    m_channels = m_settings->value("Channels", 2).toInt();
    m_bufferSize = m_settings->value("BufferSize", 1024).toInt();
    m_bitrate = m_settings->value("Bitrate", 128).toInt();
    m_qualityPreset = static_cast<QualityPreset>(m_settings->value("QualityPreset", StandardQuality).toInt());
    
    // 加载音量配置
    m_masterVolume = m_settings->value("MasterVolume", 0.8).toReal();
    m_microphoneGain = m_settings->value("MicrophoneGain", 0.6).toReal();
    m_muted = m_settings->value("Muted", false).toBool();
    
    // 加载音频处理配置
    m_noiseSuppressionEnabled = m_settings->value("NoiseSuppressionEnabled", true).toBool();
    m_echoCancellationEnabled = m_settings->value("EchoCancellationEnabled", true).toBool();
    m_autoGainControlEnabled = m_settings->value("AutoGainControlEnabled", false).toBool();
    
    m_settings->endGroup();
    
    // 加载自定义参数
    m_settings->beginGroup("CustomParameters");
    QStringList keys = m_settings->childKeys();
    for (const QString &key : keys) {
        m_customParameters[key] = m_settings->value(key);
    }
    m_settings->endGroup();
    
    return true;
}

bool AudioConfig::save()
{
    if (!m_settings) {
        return false;
    }

    m_settings->beginGroup("Audio");
    
    // 保存设备配置
    m_settings->setValue("PreferredInputDevice", m_preferredInputDevice);
    m_settings->setValue("PreferredOutputDevice", m_preferredOutputDevice);
    
    // 保存音频质量配置
    m_settings->setValue("SampleRate", m_sampleRate);
    m_settings->setValue("Channels", m_channels);
    m_settings->setValue("BufferSize", m_bufferSize);
    m_settings->setValue("Bitrate", m_bitrate);
    m_settings->setValue("QualityPreset", static_cast<int>(m_qualityPreset));
    
    // 保存音量配置
    m_settings->setValue("MasterVolume", m_masterVolume);
    m_settings->setValue("MicrophoneGain", m_microphoneGain);
    m_settings->setValue("Muted", m_muted);
    
    // 保存音频处理配置
    m_settings->setValue("NoiseSuppressionEnabled", m_noiseSuppressionEnabled);
    m_settings->setValue("EchoCancellationEnabled", m_echoCancellationEnabled);
    m_settings->setValue("AutoGainControlEnabled", m_autoGainControlEnabled);
    
    m_settings->endGroup();
    
    // 保存自定义参数
    m_settings->beginGroup("CustomParameters");
    m_settings->remove(""); // 清除所有现有键
    for (auto it = m_customParameters.begin(); it != m_customParameters.end(); ++it) {
        m_settings->setValue(it.key(), it.value());
    }
    m_settings->endGroup();
    
    m_settings->sync();
    return true;
}

void AudioConfig::resetToDefaults()
{
    initializeDefaults();
    emit configChanged("all", QVariant());
}

bool AudioConfig::validate() const
{
    // 验证采样率
    if (m_sampleRate < 8000 || m_sampleRate > 192000) {
        return false;
    }
    
    // 验证声道数
    if (m_channels < 1 || m_channels > 8) {
        return false;
    }
    
    // 验证缓冲区大小
    if (m_bufferSize < 64 || m_bufferSize > 8192) {
        return false;
    }
    
    // 验证音量范围
    if (m_masterVolume < 0.0 || m_masterVolume > 1.0) {
        return false;
    }
    
    if (m_microphoneGain < 0.0 || m_microphoneGain > 1.0) {
        return false;
    }
    
    return true;
}

void AudioConfig::initializeDefaults()
{
    m_preferredInputDevice = "";
    m_preferredOutputDevice = "";
    m_sampleRate = 48000;
    m_channels = 2;
    m_bufferSize = 1024;
    m_bitrate = 128;
    m_qualityPreset = StandardQuality;
    m_masterVolume = 0.8;
    m_microphoneGain = 0.6;
    m_muted = false;
    m_noiseSuppressionEnabled = true;
    m_echoCancellationEnabled = true;
    m_autoGainControlEnabled = false;
    m_customParameters.clear();
}

// 设备配置方法的实现
void AudioConfig::setPreferredInputDevice(const QString &deviceId)
{
    if (m_preferredInputDevice != deviceId) {
        m_preferredInputDevice = deviceId;
        emitConfigChanged("PreferredInputDevice", deviceId);
        emit deviceConfigChanged();
    }
}

QString AudioConfig::preferredInputDevice() const
{
    return m_preferredInputDevice;
}

QString AudioConfig::preferredOutputDevice() const
{
    return m_preferredOutputDevice;
}

void AudioConfig::setPreferredOutputDevice(const QString &deviceId)
{
    if (m_preferredOutputDevice != deviceId) {
        m_preferredOutputDevice = deviceId;
        emitConfigChanged("PreferredOutputDevice", deviceId);
        emit deviceConfigChanged();
    }
}

// 音频质量配置方法
void AudioConfig::setSampleRate(int sampleRate)
{
    if (m_sampleRate != sampleRate) {
        m_sampleRate = sampleRate;
        emitConfigChanged("SampleRate", sampleRate);
        emit qualityConfigChanged();
    }
}

int AudioConfig::sampleRate() const
{
    return m_sampleRate;
}

void AudioConfig::setChannels(int channels)
{
    if (m_channels != channels) {
        m_channels = channels;
        emitConfigChanged("Channels", channels);
        emit qualityConfigChanged();
    }
}

int AudioConfig::channels() const
{
    return m_channels;
}

void AudioConfig::setBufferSize(int bufferSize)
{
    if (m_bufferSize != bufferSize) {
        m_bufferSize = bufferSize;
        emitConfigChanged("BufferSize", bufferSize);
        emit qualityConfigChanged();
    }
}

int AudioConfig::bufferSize() const
{
    return m_bufferSize;
}

void AudioConfig::setBitrate(int bitrate)
{
    if (m_bitrate != bitrate) {
        m_bitrate = bitrate;
        emitConfigChanged("Bitrate", bitrate);
        emit qualityConfigChanged();
    }
}

int AudioConfig::bitrate() const
{
    return m_bitrate;
}

void AudioConfig::setQualityPreset(QualityPreset preset)
{
    if (m_qualityPreset != preset) {
        m_qualityPreset = preset;
        applyQualityPreset(preset);
        emitConfigChanged("QualityPreset", static_cast<int>(preset));
        emit qualityConfigChanged();
    }
}

AudioConfig::QualityPreset AudioConfig::qualityPreset() const
{
    return m_qualityPreset;
}

// 音量配置方法
void AudioConfig::setMasterVolume(qreal volume)
{
    volume = qBound(0.0, volume, 1.0);
    if (qAbs(m_masterVolume - volume) > 0.001) {
        m_masterVolume = volume;
        emitConfigChanged("MasterVolume", volume);
        emit volumeConfigChanged();
    }
}

qreal AudioConfig::masterVolume() const
{
    return m_masterVolume;
}

void AudioConfig::setMicrophoneGain(qreal gain)
{
    gain = qBound(0.0, gain, 1.0);
    if (qAbs(m_microphoneGain - gain) > 0.001) {
        m_microphoneGain = gain;
        emitConfigChanged("MicrophoneGain", gain);
        emit volumeConfigChanged();
    }
}

qreal AudioConfig::microphoneGain() const
{
    return m_microphoneGain;
}

void AudioConfig::setMuted(bool muted)
{
    if (m_muted != muted) {
        m_muted = muted;
        emitConfigChanged("Muted", muted);
        emit volumeConfigChanged();
    }
}

bool AudioConfig::isMuted() const
{
    return m_muted;
}

// 音频处理配置方法
void AudioConfig::setNoiseSuppressionEnabled(bool enabled)
{
    if (m_noiseSuppressionEnabled != enabled) {
        m_noiseSuppressionEnabled = enabled;
        emitConfigChanged("NoiseSuppressionEnabled", enabled);
        emit processingConfigChanged();
    }
}

bool AudioConfig::isNoiseSuppressionEnabled() const
{
    return m_noiseSuppressionEnabled;
}

void AudioConfig::setEchoCancellationEnabled(bool enabled)
{
    if (m_echoCancellationEnabled != enabled) {
        m_echoCancellationEnabled = enabled;
        emitConfigChanged("EchoCancellationEnabled", enabled);
        emit processingConfigChanged();
    }
}

bool AudioConfig::isEchoCancellationEnabled() const
{
    return m_echoCancellationEnabled;
}

void AudioConfig::setAutoGainControlEnabled(bool enabled)
{
    if (m_autoGainControlEnabled != enabled) {
        m_autoGainControlEnabled = enabled;
        emitConfigChanged("AutoGainControlEnabled", enabled);
        emit processingConfigChanged();
    }
}

bool AudioConfig::isAutoGainControlEnabled() const
{
    return m_autoGainControlEnabled;
}

// 高级配置方法
void AudioConfig::setCustomParameter(const QString &key, const QVariant &value)
{
    if (m_customParameters.value(key) != value) {
        m_customParameters[key] = value;
        emitConfigChanged("CustomParameter_" + key, value);
    }
}

QVariant AudioConfig::customParameter(const QString &key, const QVariant &defaultValue) const
{
    return m_customParameters.value(key, defaultValue);
}

QVariantMap AudioConfig::customParameters() const
{
    return m_customParameters;
}

QVariantMap AudioConfig::toVariantMap() const
{
    QVariantMap map;
    
    // 设备配置
    map["PreferredInputDevice"] = m_preferredInputDevice;
    map["PreferredOutputDevice"] = m_preferredOutputDevice;
    
    // 音频质量配置
    map["SampleRate"] = m_sampleRate;
    map["Channels"] = m_channels;
    map["BufferSize"] = m_bufferSize;
    map["Bitrate"] = m_bitrate;
    map["QualityPreset"] = static_cast<int>(m_qualityPreset);
    
    // 音量配置
    map["MasterVolume"] = m_masterVolume;
    map["MicrophoneGain"] = m_microphoneGain;
    map["Muted"] = m_muted;
    
    // 音频处理配置
    map["NoiseSuppressionEnabled"] = m_noiseSuppressionEnabled;
    map["EchoCancellationEnabled"] = m_echoCancellationEnabled;
    map["AutoGainControlEnabled"] = m_autoGainControlEnabled;
    
    // 自定义参数
    map["CustomParameters"] = m_customParameters;
    
    return map;
}

void AudioConfig::fromVariantMap(const QVariantMap &map)
{
    // 设备配置
    setPreferredInputDevice(map.value("PreferredInputDevice", "").toString());
    setPreferredOutputDevice(map.value("PreferredOutputDevice", "").toString());
    
    // 音频质量配置
    setSampleRate(map.value("SampleRate", 48000).toInt());
    setChannels(map.value("Channels", 2).toInt());
    setBufferSize(map.value("BufferSize", 1024).toInt());
    setBitrate(map.value("Bitrate", 128).toInt());
    setQualityPreset(static_cast<QualityPreset>(map.value("QualityPreset", StandardQuality).toInt()));
    
    // 音量配置
    setMasterVolume(map.value("MasterVolume", 0.8).toReal());
    setMicrophoneGain(map.value("MicrophoneGain", 0.6).toReal());
    setMuted(map.value("Muted", false).toBool());
    
    // 音频处理配置
    setNoiseSuppressionEnabled(map.value("NoiseSuppressionEnabled", true).toBool());
    setEchoCancellationEnabled(map.value("EchoCancellationEnabled", true).toBool());
    setAutoGainControlEnabled(map.value("AutoGainControlEnabled", false).toBool());
    
    // 自定义参数
    QVariantMap customParams = map.value("CustomParameters").toMap();
    m_customParameters = customParams;
}

QString AudioConfig::configFilePath() const
{
    return m_configFilePath;
}

void AudioConfig::setConfigFilePath(const QString &filePath)
{
    if (m_configFilePath != filePath) {
        m_configFilePath = filePath;
        
        // 重新创建QSettings对象
        if (m_settings) {
            delete m_settings;
        }
        m_settings = new QSettings(m_configFilePath, QSettings::IniFormat, this);
    }
}

void AudioConfig::applyQualityPreset(QualityPreset preset)
{
    switch (preset) {
    case LowQuality:
        m_sampleRate = 16000;
        m_channels = 1;
        m_bitrate = 64;
        m_bufferSize = 512;
        break;
    case StandardQuality:
        m_sampleRate = 44100;
        m_channels = 2;
        m_bitrate = 128;
        m_bufferSize = 1024;
        break;
    case HighQuality:
        m_sampleRate = 48000;
        m_channels = 2;
        m_bitrate = 256;
        m_bufferSize = 2048;
        break;
    }
}

void AudioConfig::emitConfigChanged(const QString &key, const QVariant &value)
{
    emit configChanged(key, value);
}