#include "QtAudioDevice.h"
#include <QDebug>
#include <QAudio>

QtAudioDevice::QtAudioDevice(const QString &deviceId, 
                           AudioFactory::DeviceType deviceType, 
                           QObject *parent)
    : IAudioDevice(parent)
    , m_deviceId(deviceId)
    , m_deviceType(deviceType == AudioFactory::InputDevice ? Input : Output)
    , m_status(Inactive)
    , m_volume(1.0)
    , m_muted(false)
    , m_qualityPreset(StandardQuality)
    , m_sampleRate(44100)
    , m_channels(2)
    , m_bufferSize(1024)
    , m_audioInput(nullptr)
    , m_audioOutput(nullptr)
    , m_inputIODevice(nullptr)
    , m_outputIODevice(nullptr)
{
    qDebug() << "QtAudioDevice created:" << deviceId << "Type:" << m_deviceType;
}

QtAudioDevice::~QtAudioDevice()
{
    stop();
    qDebug() << "QtAudioDevice destroyed:" << m_deviceId;
}

bool QtAudioDevice::initialize()
{
    if (m_status != Inactive) {
        return true;
    }
    
    // 查找设备信息
    if (!findDeviceInfo()) {
        emit errorOccurred("Device not found: " + m_deviceId);
        setStatus(Error);
        return false;
    }
    
    // 更新音频格式
    updateAudioFormat();
    
    // 检查格式是否支持
    if (!m_deviceInfo.isFormatSupported(m_format)) {
        QAudioFormat nearestFormat = m_deviceInfo.nearestFormat(m_format);
        qDebug() << "Original format not supported, using nearest format";
        qDebug() << "Original:" << m_format;
        qDebug() << "Nearest:" << nearestFormat;
        m_format = nearestFormat;
        
        // 更新参数以匹配实际格式
        m_sampleRate = m_format.sampleRate();
        m_channels = m_format.channelCount();
    }
    
    qDebug() << "QtAudioDevice initialized:" << m_deviceId;
    return true;
}

bool QtAudioDevice::start()
{
    if (m_status == Active) {
        return true;
    }
    
    if (m_deviceInfo.isNull()) {
        emit errorOccurred("Device not initialized");
        return false;
    }
    
    try {
        if (m_deviceType == Input) {
            // 创建音频输入
            m_audioInput = new QAudioInput(m_deviceInfo, m_format, this);
            m_audioInput->setVolume(m_muted ? 0.0 : m_volume);
            
            // 连接状态变化信号
            connect(m_audioInput, QOverload<QAudio::State>::of(&QAudioInput::stateChanged),
                    this, &QtAudioDevice::onInputStateChanged);
            
            // 启动音频输入
            m_inputIODevice = m_audioInput->start();
            if (!m_inputIODevice) {
                emit errorOccurred("Failed to start audio input");
                return false;
            }
            
        } else {
            // 创建音频输出
            m_audioOutput = new QAudioOutput(m_deviceInfo, m_format, this);
            m_audioOutput->setVolume(m_muted ? 0.0 : m_volume);
            
            // 连接状态变化信号
            connect(m_audioOutput, QOverload<QAudio::State>::of(&QAudioOutput::stateChanged),
                    this, &QtAudioDevice::onOutputStateChanged);
            
            // 启动音频输出
            m_outputIODevice = m_audioOutput->start();
            if (!m_outputIODevice) {
                emit errorOccurred("Failed to start audio output");
                return false;
            }
        }
        
        setStatus(Active);
        qDebug() << "QtAudioDevice started:" << m_deviceId;
        return true;
        
    } catch (const std::exception &e) {
        emit errorOccurred(QString("Failed to start device: %1").arg(e.what()));
        setStatus(Error);
        return false;
    }
}

void QtAudioDevice::stop()
{
    if (m_status == Inactive) {
        return;
    }
    
    try {
        if (m_audioInput) {
            m_audioInput->stop();
            m_audioInput->deleteLater();
            m_audioInput = nullptr;
            m_inputIODevice = nullptr;
        }
        
        if (m_audioOutput) {
            m_audioOutput->stop();
            m_audioOutput->deleteLater();
            m_audioOutput = nullptr;
            m_outputIODevice = nullptr;
        }
        
        setStatus(Inactive);
        qDebug() << "QtAudioDevice stopped:" << m_deviceId;
        
    } catch (const std::exception &e) {
        qWarning() << "Error stopping device:" << m_deviceId << e.what();
    }
}

bool QtAudioDevice::isActive() const
{
    return m_status == Active;
}

QString QtAudioDevice::deviceId() const
{
    return m_deviceId;
}

QString QtAudioDevice::deviceName() const
{
    return m_deviceInfo.isNull() ? m_deviceId : m_deviceInfo.deviceName();
}

IAudioDevice::DeviceType QtAudioDevice::deviceType() const
{
    return m_deviceType;
}

IAudioDevice::Status QtAudioDevice::status() const
{
    return m_status;
}

void QtAudioDevice::setVolume(qreal volume)
{
    volume = qBound(0.0, volume, 1.0);
    
    if (qAbs(m_volume - volume) < 0.001) {
        return;
    }
    
    m_volume = volume;
    
    // 应用音量到实际设备
    qreal actualVolume = m_muted ? 0.0 : volume;
    
    if (m_audioInput) {
        m_audioInput->setVolume(actualVolume);
    }
    
    if (m_audioOutput) {
        m_audioOutput->setVolume(actualVolume);
    }
    
    emit volumeChanged(volume);
    qDebug() << "Volume set to:" << volume << "for device:" << m_deviceId;
}

qreal QtAudioDevice::volume() const
{
    return m_volume;
}

void QtAudioDevice::setMuted(bool muted)
{
    if (m_muted == muted) {
        return;
    }
    
    m_muted = muted;
    
    // 应用静音状态
    qreal actualVolume = muted ? 0.0 : m_volume;
    
    if (m_audioInput) {
        m_audioInput->setVolume(actualVolume);
    }
    
    if (m_audioOutput) {
        m_audioOutput->setVolume(actualVolume);
    }
    
    emit muteChanged(muted);
    qDebug() << "Mute set to:" << muted << "for device:" << m_deviceId;
}

bool QtAudioDevice::isMuted() const
{
    return m_muted;
}

void QtAudioDevice::setQualityPreset(QualityPreset preset)
{
    if (m_qualityPreset == preset) {
        return;
    }
    
    m_qualityPreset = preset;
    
    // 根据预设调整参数
    switch (preset) {
    case LowQuality:
        setSampleRate(16000);
        setChannels(1);
        setBufferSize(2048);
        break;
    case StandardQuality:
        setSampleRate(44100);
        setChannels(2);
        setBufferSize(1024);
        break;
    case HighQuality:
        setSampleRate(48000);
        setChannels(2);
        setBufferSize(512);
        break;
    }
    
    qDebug() << "Quality preset set to:" << preset << "for device:" << m_deviceId;
}

IAudioDevice::QualityPreset QtAudioDevice::qualityPreset() const
{
    return m_qualityPreset;
}

int QtAudioDevice::sampleRate() const
{
    return m_sampleRate;
}

void QtAudioDevice::setSampleRate(int sampleRate)
{
    if (m_sampleRate != sampleRate) {
        m_sampleRate = sampleRate;
        updateAudioFormat();
        emit formatChanged(sampleRate, m_channels);
        qDebug() << "Sample rate set to:" << sampleRate << "for device:" << m_deviceId;
    }
}

int QtAudioDevice::channels() const
{
    return m_channels;
}

void QtAudioDevice::setChannels(int channels)
{
    if (m_channels != channels) {
        m_channels = channels;
        updateAudioFormat();
        emit formatChanged(m_sampleRate, channels);
        qDebug() << "Channels set to:" << channels << "for device:" << m_deviceId;
    }
}

int QtAudioDevice::bufferSize() const
{
    return m_bufferSize;
}

void QtAudioDevice::setBufferSize(int bufferSize)
{
    if (m_bufferSize != bufferSize) {
        m_bufferSize = bufferSize;
        qDebug() << "Buffer size set to:" << bufferSize << "for device:" << m_deviceId;
    }
}

qreal QtAudioDevice::latency() const
{
    // 估算延迟 = 缓冲区大小 / 采样率 * 1000 (毫秒)
    if (m_sampleRate > 0) {
        return (qreal)m_bufferSize / m_sampleRate * 1000.0;
    }
    return 0.0;
}

bool QtAudioDevice::supportsFormat(int sampleRate, int channels) const
{
    if (m_deviceInfo.isNull()) {
        return false;
    }
    
    QAudioFormat testFormat;
    testFormat.setSampleRate(sampleRate);
    testFormat.setChannelCount(channels);
    testFormat.setSampleSize(16);
    testFormat.setCodec("audio/pcm");
    testFormat.setByteOrder(QAudioFormat::LittleEndian);
    testFormat.setSampleType(QAudioFormat::SignedInt);
    
    return m_deviceInfo.isFormatSupported(testFormat);
}

void QtAudioDevice::onInputStateChanged(QAudio::State state)
{
    qDebug() << "Input state changed:" << state << "for device:" << m_deviceId;
    
    switch (state) {
    case QAudio::ActiveState:
        // 设备正在运行
        break;
    case QAudio::SuspendedState:
        // 设备被暂停
        break;
    case QAudio::StoppedState:
        // 设备已停止
        if (m_audioInput && m_audioInput->error() != QAudio::NoError) {
            emit errorOccurred(QString("Audio input error: %1").arg(m_audioInput->error()));
            setStatus(Error);
        }
        break;
    case QAudio::IdleState:
        // 设备空闲
        break;
    }
}

void QtAudioDevice::onOutputStateChanged(QAudio::State state)
{
    qDebug() << "Output state changed:" << state << "for device:" << m_deviceId;
    
    switch (state) {
    case QAudio::ActiveState:
        // 设备正在运行
        break;
    case QAudio::SuspendedState:
        // 设备被暂停
        break;
    case QAudio::StoppedState:
        // 设备已停止
        if (m_audioOutput && m_audioOutput->error() != QAudio::NoError) {
            emit errorOccurred(QString("Audio output error: %1").arg(m_audioOutput->error()));
            setStatus(Error);
        }
        break;
    case QAudio::IdleState:
        // 设备空闲
        break;
    }
}

void QtAudioDevice::setStatus(Status status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
        qDebug() << "Status changed to:" << status << "for device:" << m_deviceId;
    }
}

void QtAudioDevice::updateAudioFormat()
{
    m_format.setSampleRate(m_sampleRate);
    m_format.setChannelCount(m_channels);
    m_format.setSampleSize(16);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);
}

bool QtAudioDevice::findDeviceInfo()
{
    QList<QAudioDeviceInfo> devices;
    
    if (m_deviceType == Input) {
        devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    } else {
        devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    }
    
    for (const QAudioDeviceInfo &info : devices) {
        if (info.deviceName() == m_deviceId) {
            m_deviceInfo = info;
            qDebug() << "Found device info for:" << m_deviceId;
            return true;
        }
    }
    
    qWarning() << "Device info not found for:" << m_deviceId;
    return false;
}