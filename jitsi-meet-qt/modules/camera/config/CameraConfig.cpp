#include "CameraConfig.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

// 静态成员初始化
CameraConfig* CameraConfig::s_instance = nullptr;

// 配置键名定义
const QString CameraConfig::Keys::PREFERRED_DEVICE = "camera/preferredDevice";
const QString CameraConfig::Keys::DEFAULT_RESOLUTION = "camera/defaultResolution";
const QString CameraConfig::Keys::DEFAULT_FRAME_RATE = "camera/defaultFrameRate";
const QString CameraConfig::Keys::DEFAULT_QUALITY_PRESET = "camera/defaultQualityPreset";
const QString CameraConfig::Keys::AUTO_START_CAMERA = "camera/autoStartCamera";
const QString CameraConfig::Keys::ENABLE_HARDWARE_ACCELERATION = "camera/enableHardwareAcceleration";
const QString CameraConfig::Keys::MAX_RETRY_COUNT = "camera/maxRetryCount";
const QString CameraConfig::Keys::RETRY_DELAY_MS = "camera/retryDelayMs";
const QString CameraConfig::Keys::ENABLE_PERFORMANCE_MONITORING = "camera/enablePerformanceMonitoring";
const QString CameraConfig::Keys::LOG_LEVEL = "camera/logLevel";

// 默认值定义
const QSize CameraConfig::Defaults::RESOLUTION = QSize(640, 480);
const int CameraConfig::Defaults::FRAME_RATE = 30;
const ICameraDevice::QualityPreset CameraConfig::Defaults::QUALITY_PRESET = ICameraDevice::StandardQuality;
const bool CameraConfig::Defaults::AUTO_START_CAMERA = false;
const bool CameraConfig::Defaults::ENABLE_HARDWARE_ACCELERATION = true;
const int CameraConfig::Defaults::MAX_RETRY_COUNT = 3;
const int CameraConfig::Defaults::RETRY_DELAY_MS = 1000;
const bool CameraConfig::Defaults::ENABLE_PERFORMANCE_MONITORING = true;
const QString CameraConfig::Defaults::LOG_LEVEL = "Info";

CameraConfig::CameraConfig(QObject* parent)
    : QObject(parent)
    , m_settings(nullptr)
{
    // 创建配置目录
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    
    // 初始化设置
    m_settings = new QSettings(configPath + "/camera.ini", QSettings::IniFormat, this);
    
    // 初始化默认值
    initializeDefaults();
    
    // 加载配置
    loadFromSettings();
    
    // 连接信号
    connectSignals();
}

CameraConfig::~CameraConfig()
{
    if (this == s_instance) {
        s_instance = nullptr;
    }
}

CameraConfig* CameraConfig::instance()
{
    if (!s_instance) {
        s_instance = new CameraConfig();
    }
    return s_instance;
}

void CameraConfig::initializeDefaults()
{
    m_preferredDevice = "";
    m_defaultResolution = Defaults::RESOLUTION;
    m_defaultFrameRate = Defaults::FRAME_RATE;
    m_defaultQualityPreset = Defaults::QUALITY_PRESET;
    m_autoStartCamera = Defaults::AUTO_START_CAMERA;
    m_enableHardwareAcceleration = Defaults::ENABLE_HARDWARE_ACCELERATION;
    m_maxRetryCount = Defaults::MAX_RETRY_COUNT;
    m_retryDelay = Defaults::RETRY_DELAY_MS;
    m_enablePerformanceMonitoring = Defaults::ENABLE_PERFORMANCE_MONITORING;
    m_logLevel = Defaults::LOG_LEVEL;
}

void CameraConfig::connectSignals()
{
    // 这里可以连接内部信号
}

void CameraConfig::setPreferredDevice(const QString& deviceId)
{
    if (m_preferredDevice != deviceId) {
        m_preferredDevice = deviceId;
        emit preferredDeviceChanged(deviceId);
        emit configChanged();
    }
}

QString CameraConfig::preferredDevice() const
{
    return m_preferredDevice;
}

void CameraConfig::setDefaultResolution(const QSize& resolution)
{
    if (m_defaultResolution != resolution) {
        m_defaultResolution = resolution;
        emit defaultResolutionChanged(resolution);
        emit configChanged();
    }
}

QSize CameraConfig::defaultResolution() const
{
    return m_defaultResolution;
}

void CameraConfig::setDefaultFrameRate(int frameRate)
{
    if (m_defaultFrameRate != frameRate) {
        m_defaultFrameRate = frameRate;
        emit defaultFrameRateChanged(frameRate);
        emit configChanged();
    }
}

int CameraConfig::defaultFrameRate() const
{
    return m_defaultFrameRate;
}

void CameraConfig::setDefaultQualityPreset(ICameraDevice::QualityPreset preset)
{
    if (m_defaultQualityPreset != preset) {
        m_defaultQualityPreset = preset;
        emit qualityPresetChanged(preset);
        emit configChanged();
    }
}

ICameraDevice::QualityPreset CameraConfig::defaultQualityPreset() const
{
    return m_defaultQualityPreset;
}

void CameraConfig::setAutoStartCamera(bool autoStart)
{
    if (m_autoStartCamera != autoStart) {
        m_autoStartCamera = autoStart;
        emit configChanged();
    }
}

bool CameraConfig::autoStartCamera() const
{
    return m_autoStartCamera;
}

void CameraConfig::setEnableHardwareAcceleration(bool enable)
{
    if (m_enableHardwareAcceleration != enable) {
        m_enableHardwareAcceleration = enable;
        emit configChanged();
    }
}

bool CameraConfig::enableHardwareAcceleration() const
{
    return m_enableHardwareAcceleration;
}

void CameraConfig::setMaxRetryCount(int count)
{
    if (m_maxRetryCount != count) {
        m_maxRetryCount = count;
        emit configChanged();
    }
}

int CameraConfig::maxRetryCount() const
{
    return m_maxRetryCount;
}

void CameraConfig::setRetryDelay(int delayMs)
{
    if (m_retryDelay != delayMs) {
        m_retryDelay = delayMs;
        emit configChanged();
    }
}

int CameraConfig::retryDelay() const
{
    return m_retryDelay;
}

void CameraConfig::setEnablePerformanceMonitoring(bool enable)
{
    if (m_enablePerformanceMonitoring != enable) {
        m_enablePerformanceMonitoring = enable;
        emit configChanged();
    }
}

bool CameraConfig::enablePerformanceMonitoring() const
{
    return m_enablePerformanceMonitoring;
}

void CameraConfig::setLogLevel(const QString& level)
{
    if (m_logLevel != level) {
        m_logLevel = level;
        emit configChanged();
    }
}

QString CameraConfig::logLevel() const
{
    return m_logLevel;
}

void CameraConfig::loadFromSettings()
{
    m_preferredDevice = m_settings->value(Keys::PREFERRED_DEVICE, "").toString();
    
    QSize resolution = m_settings->value(Keys::DEFAULT_RESOLUTION, Defaults::RESOLUTION).toSize();
    m_defaultResolution = resolution.isValid() ? resolution : Defaults::RESOLUTION;
    
    m_defaultFrameRate = m_settings->value(Keys::DEFAULT_FRAME_RATE, Defaults::FRAME_RATE).toInt();
    m_defaultQualityPreset = static_cast<ICameraDevice::QualityPreset>(
        m_settings->value(Keys::DEFAULT_QUALITY_PRESET, static_cast<int>(Defaults::QUALITY_PRESET)).toInt());
    
    m_autoStartCamera = m_settings->value(Keys::AUTO_START_CAMERA, Defaults::AUTO_START_CAMERA).toBool();
    m_enableHardwareAcceleration = m_settings->value(Keys::ENABLE_HARDWARE_ACCELERATION, Defaults::ENABLE_HARDWARE_ACCELERATION).toBool();
    m_maxRetryCount = m_settings->value(Keys::MAX_RETRY_COUNT, Defaults::MAX_RETRY_COUNT).toInt();
    m_retryDelay = m_settings->value(Keys::RETRY_DELAY_MS, Defaults::RETRY_DELAY_MS).toInt();
    m_enablePerformanceMonitoring = m_settings->value(Keys::ENABLE_PERFORMANCE_MONITORING, Defaults::ENABLE_PERFORMANCE_MONITORING).toBool();
    m_logLevel = m_settings->value(Keys::LOG_LEVEL, Defaults::LOG_LEVEL).toString();
}

void CameraConfig::saveToSettings()
{
    m_settings->setValue(Keys::PREFERRED_DEVICE, m_preferredDevice);
    m_settings->setValue(Keys::DEFAULT_RESOLUTION, m_defaultResolution);
    m_settings->setValue(Keys::DEFAULT_FRAME_RATE, m_defaultFrameRate);
    m_settings->setValue(Keys::DEFAULT_QUALITY_PRESET, static_cast<int>(m_defaultQualityPreset));
    m_settings->setValue(Keys::AUTO_START_CAMERA, m_autoStartCamera);
    m_settings->setValue(Keys::ENABLE_HARDWARE_ACCELERATION, m_enableHardwareAcceleration);
    m_settings->setValue(Keys::MAX_RETRY_COUNT, m_maxRetryCount);
    m_settings->setValue(Keys::RETRY_DELAY_MS, m_retryDelay);
    m_settings->setValue(Keys::ENABLE_PERFORMANCE_MONITORING, m_enablePerformanceMonitoring);
    m_settings->setValue(Keys::LOG_LEVEL, m_logLevel);
    
    m_settings->sync();
}

void CameraConfig::resetToDefaults()
{
    initializeDefaults();
    emit configChanged();
}

QVariantMap CameraConfig::toVariantMap() const
{
    QVariantMap config;
    config[Keys::PREFERRED_DEVICE] = m_preferredDevice;
    config[Keys::DEFAULT_RESOLUTION] = m_defaultResolution;
    config[Keys::DEFAULT_FRAME_RATE] = m_defaultFrameRate;
    config[Keys::DEFAULT_QUALITY_PRESET] = static_cast<int>(m_defaultQualityPreset);
    config[Keys::AUTO_START_CAMERA] = m_autoStartCamera;
    config[Keys::ENABLE_HARDWARE_ACCELERATION] = m_enableHardwareAcceleration;
    config[Keys::MAX_RETRY_COUNT] = m_maxRetryCount;
    config[Keys::RETRY_DELAY_MS] = m_retryDelay;
    config[Keys::ENABLE_PERFORMANCE_MONITORING] = m_enablePerformanceMonitoring;
    config[Keys::LOG_LEVEL] = m_logLevel;
    return config;
}

void CameraConfig::fromVariantMap(const QVariantMap& config)
{
    if (config.contains(Keys::PREFERRED_DEVICE)) {
        setPreferredDevice(config[Keys::PREFERRED_DEVICE].toString());
    }
    if (config.contains(Keys::DEFAULT_RESOLUTION)) {
        setDefaultResolution(config[Keys::DEFAULT_RESOLUTION].toSize());
    }
    if (config.contains(Keys::DEFAULT_FRAME_RATE)) {
        setDefaultFrameRate(config[Keys::DEFAULT_FRAME_RATE].toInt());
    }
    if (config.contains(Keys::DEFAULT_QUALITY_PRESET)) {
        setDefaultQualityPreset(static_cast<ICameraDevice::QualityPreset>(
            config[Keys::DEFAULT_QUALITY_PRESET].toInt()));
    }
    if (config.contains(Keys::AUTO_START_CAMERA)) {
        setAutoStartCamera(config[Keys::AUTO_START_CAMERA].toBool());
    }
    if (config.contains(Keys::ENABLE_HARDWARE_ACCELERATION)) {
        setEnableHardwareAcceleration(config[Keys::ENABLE_HARDWARE_ACCELERATION].toBool());
    }
    if (config.contains(Keys::MAX_RETRY_COUNT)) {
        setMaxRetryCount(config[Keys::MAX_RETRY_COUNT].toInt());
    }
    if (config.contains(Keys::RETRY_DELAY_MS)) {
        setRetryDelay(config[Keys::RETRY_DELAY_MS].toInt());
    }
    if (config.contains(Keys::ENABLE_PERFORMANCE_MONITORING)) {
        setEnablePerformanceMonitoring(config[Keys::ENABLE_PERFORMANCE_MONITORING].toBool());
    }
    if (config.contains(Keys::LOG_LEVEL)) {
        setLogLevel(config[Keys::LOG_LEVEL].toString());
    }
}

bool CameraConfig::isValid() const
{
    return validate().isEmpty();
}

QStringList CameraConfig::validate() const
{
    QStringList errors;
    
    if (m_defaultResolution.width() <= 0 || m_defaultResolution.height() <= 0) {
        errors << "Invalid default resolution";
    }
    
    if (m_defaultFrameRate <= 0 || m_defaultFrameRate > 120) {
        errors << "Invalid default frame rate";
    }
    
    if (m_maxRetryCount < 0 || m_maxRetryCount > 10) {
        errors << "Invalid max retry count";
    }
    
    if (m_retryDelay < 0 || m_retryDelay > 10000) {
        errors << "Invalid retry delay";
    }
    
    QStringList validLogLevels = {"Debug", "Info", "Warning", "Error"};
    if (!validLogLevels.contains(m_logLevel)) {
        errors << "Invalid log level";
    }
    
    return errors;
}

void CameraConfig::reload()
{
    loadFromSettings();
    emit configChanged();
}

void CameraConfig::save()
{
    saveToSettings();
}