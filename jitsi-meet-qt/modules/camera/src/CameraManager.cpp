#include "CameraManager.h"
#include "../include/CameraModule.h"
#include "../utils/CameraUtils.h"
#include <QDebug>
#include <QDateTime>

CameraManager::CameraManager(QObject *parent)
    : ICameraManager(parent)
    , m_cameraModule(nullptr)
    , m_state(Idle)
    , m_currentPreset(StandardQuality)
    , m_statsTimer(new QTimer(this))
    , m_recoveryTimer(new QTimer(this))
    , m_startTime(0)
    , m_monitoringEnabled(false)
    , m_autoRecoveryEnabled(true)
    , m_maxRetries(3)
    , m_currentRetries(0)
{
    qDebug() << "CameraManager: Initializing...";

    // 创建摄像头模块
    m_cameraModule = new CameraModule(this);

    // 连接摄像头模块信号
    QObject::connect(m_cameraModule, &CameraModule::stateChanged,
            this, &CameraManager::onCameraStateChanged);
    QObject::connect(m_cameraModule, &CameraModule::started,
            this, &CameraManager::onCameraStarted);
    QObject::connect(m_cameraModule, &CameraModule::stopped,
            this, &CameraManager::onCameraStopped);
    QObject::connect(m_cameraModule, &CameraModule::errorOccurred,
            this, &CameraManager::onCameraError);
    QObject::connect(m_cameraModule, &CameraModule::devicesChanged,
            this, &CameraManager::onDevicesChanged);

    // 设置统计定时器
    m_statsTimer->setInterval(1000); // 每秒更新统计
    QObject::connect(m_statsTimer, &QTimer::timeout, this, &CameraManager::onStatsTimer);

    // 设置恢复定时器
    m_recoveryTimer->setSingleShot(true);
    m_recoveryTimer->setInterval(5000); // 5秒后尝试恢复
    QObject::connect(m_recoveryTimer, &QTimer::timeout, this, &CameraManager::onRecoveryTimer);

    qDebug() << "CameraManager: Initialization completed";
}

CameraManager::~CameraManager()
{
    qDebug() << "CameraManager: Destroying...";
    cleanup();
}

// === ICameraManager接口实现 ===

bool CameraManager::initialize()
{
    qDebug() << "CameraManager::initialize() - Current state:" << m_state;
    
    if (m_state != Idle) {
        qDebug() << "CameraManager::initialize() - Already initialized, returning:" << (m_state == Ready);
        return m_state == Ready;
    }

    qDebug() << "CameraManager::initialize() - Setting state to Initializing";
    setState(Initializing);
    
    qDebug() << "CameraManager::initialize() - Initializing camera module";
    if (!m_cameraModule->initialize()) {
        qDebug() << "CameraManager::initialize() - Camera module initialization failed";
        setState(Error);
        return false;
    }

    qDebug() << "CameraManager::initialize() - Camera module initialized successfully";
    setState(Ready);
    emit ready();
    qDebug() << "CameraManager::initialize() - Initialization completed successfully";
    return true;
}

void CameraManager::cleanup()
{
    stopCamera();
    if (m_cameraModule) {
        m_cameraModule->cleanup();
    }
    setState(Idle);
}

ICameraManager::ManagerStatus CameraManager::status() const
{
    switch (m_state) {
        case Idle: return ICameraManager::Uninitialized;
        case Initializing: return ICameraManager::Initializing;
        case Ready: return ICameraManager::Ready;
        case Error: return ICameraManager::Error;
        default: return ICameraManager::Error;
    }
}

QStringList CameraManager::availableDevices() const
{
    QStringList devices;
    auto deviceList = m_cameraModule->availableDevices();
    for (const auto& device : deviceList) {
        devices << device.name;
    }
    return devices;
}

ICameraDevice* CameraManager::currentDevice() const
{
    return m_cameraModule;
}

bool CameraManager::selectDevice(const QString& deviceId)
{
    return m_cameraModule->setDevice(deviceId);
}

void CameraManager::refreshDevices()
{
    m_cameraModule->scanDevices();
    emit devicesUpdated();
}

bool CameraManager::startCamera()
{
    qDebug() << "CameraManager::startCamera() - Starting camera";
    bool result = m_cameraModule->start();
    qDebug() << "CameraManager::startCamera() - Result:" << result;
    return result;
}

void CameraManager::stopCamera()
{
    qDebug() << "CameraManager::stopCamera() - Stopping camera";
    m_cameraModule->stop();
    qDebug() << "CameraManager::stopCamera() - Camera stopped";
}

bool CameraManager::isCameraActive() const
{
    return m_cameraModule->isActive();
}

QVideoWidget* CameraManager::createPreviewWidget(QWidget* parent)
{
    return m_cameraModule->createVideoWidget(parent);
}

void CameraManager::setPreviewWidget(QVideoWidget* widget)
{
    m_cameraModule->setVideoWidget(widget);
}

QVideoWidget* CameraManager::previewWidget() const
{
    return m_cameraModule->videoWidget();
}

bool CameraManager::startWithPreset(ICameraDevice::QualityPreset preset)
{
    m_cameraModule->setQualityPreset(preset);
    return startCamera();
}

void CameraManager::applyConfiguration(const QVariantMap& config)
{
    CameraModule::CameraConfig cameraConfig;
    
    if (config.contains("resolution")) {
        cameraConfig.resolution = config["resolution"].toSize();
    }
    if (config.contains("frameRate")) {
        cameraConfig.frameRate = config["frameRate"].toInt();
    }
    if (config.contains("deviceId")) {
        cameraConfig.deviceId = config["deviceId"].toString();
    }
    
    m_cameraModule->setConfig(cameraConfig);
    setCustomConfig(config);
}

QVariantMap CameraManager::currentConfiguration() const
{
    QVariantMap config;
    CameraModule::CameraConfig cameraConfig = m_cameraModule->config();
    
    config["resolution"] = cameraConfig.resolution;
    config["frameRate"] = cameraConfig.frameRate;
    config["deviceId"] = cameraConfig.deviceId;
    
    return config;
}

int CameraManager::frameCount() const
{
    return m_stats.frameCount;
}

double CameraManager::averageFrameRate() const
{
    return m_stats.frameRate;
}

QSize CameraManager::currentResolution() const
{
    return m_cameraModule->resolution();
}

// === 扩展方法实现 ===

CameraManager::ManagerState CameraManager::state() const
{
    return m_state;
}

bool CameraManager::isReady() const
{
    return m_state == Ready;
}

bool CameraManager::startDefault()
{
    return m_cameraModule->startDefault();
}

bool CameraManager::startCamera(const QString& deviceId)
{
    if (!selectDevice(deviceId)) {
        return false;
    }
    return startCamera();
}

bool CameraManager::startWithPreset(CameraPreset preset)
{
    setPreset(preset);
    return startCamera();
}

void CameraManager::restartCamera()
{
    stopCamera();
    QTimer::singleShot(1000, [this]() {
        startCamera();
    });
}

bool CameraManager::switchDevice(const QString& deviceId)
{
    bool wasActive = isCameraActive();
    if (wasActive) {
        stopCamera();
    }
    
    bool success = selectDevice(deviceId);
    
    if (success && wasActive) {
        startCamera();
    }
    
    return success;
}

QVariantList CameraManager::availableDevicesExtended()
{
    QVariantList devices;
    auto deviceList = m_cameraModule->availableDevices();
    for (const auto& device : deviceList) {
        QVariantMap deviceMap;
        deviceMap["id"] = device.id;
        deviceMap["name"] = device.name;
        deviceMap["description"] = device.description;
        deviceMap["isDefault"] = device.isDefault;
        deviceMap["isActive"] = device.isActive;
        devices.append(deviceMap);
    }
    return devices;
}

QVariantMap CameraManager::currentDeviceExtended()
{
    auto device = m_cameraModule->currentDevice();
    QVariantMap deviceMap;
    deviceMap["id"] = device.id;
    deviceMap["name"] = device.name;
    deviceMap["description"] = device.description;
    deviceMap["isDefault"] = device.isDefault;
    deviceMap["isActive"] = device.isActive;
    return deviceMap;
}

QVideoWidget* CameraManager::videoWidget() const
{
    return m_cameraModule->videoWidget();
}

void CameraManager::setVideoWidget(QVideoWidget* widget)
{
    m_cameraModule->setVideoWidget(widget);
}

void CameraManager::setPreset(CameraPreset preset)
{
    m_currentPreset = preset;
    
    if (preset != CustomQuality) {
        QVariantMap config = createPresetConfig(preset);
        applyConfiguration(config);
    }
}

CameraManager::CameraPreset CameraManager::currentPreset() const
{
    return m_currentPreset;
}

void CameraManager::setCustomConfig(const QVariantMap& config)
{
    m_customConfig = config;
    m_currentPreset = CustomQuality;
}

QVariantMap CameraManager::getCurrentConfig() const
{
    if (m_currentPreset == CustomQuality) {
        return m_customConfig;
    } else {
        return createPresetConfig(m_currentPreset);
    }
}

CameraManager::CameraStats CameraManager::getStats() const
{
    return m_stats;
}

void CameraManager::resetStats()
{
    m_stats = CameraStats();
    QVariantMap device = currentDeviceExtended();
    m_stats.deviceName = device["name"].toString();
    m_stats.resolution = currentResolution();
    m_startTime = QDateTime::currentMSecsSinceEpoch();
}

void CameraManager::enableMonitoring(bool enable)
{
    m_monitoringEnabled = enable;
    if (enable) {
        m_statsTimer->start();
    } else {
        m_statsTimer->stop();
    }
}

void CameraManager::enableAutoRecovery(bool enable)
{
    m_autoRecoveryEnabled = enable;
}

void CameraManager::setMaxRetries(int maxRetries)
{
    m_maxRetries = maxRetries;
}

// === 私有方法实现 ===

void CameraManager::setState(ManagerState state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);
    }
}

QVariantMap CameraManager::createPresetConfig(CameraPreset preset) const
{
    QVariantMap config;
    
    switch (preset) {
        case LowQuality:
            config["resolution"] = QSize(640, 480);
            config["frameRate"] = 15;
            break;
        case StandardQuality:
            config["resolution"] = QSize(1280, 720);
            config["frameRate"] = 30;
            break;
        case HighQuality:
            config["resolution"] = QSize(1920, 1080);
            config["frameRate"] = 30;
            break;
        case CustomQuality:
            config = m_customConfig;
            break;
    }
    
    return config;
}

void CameraManager::updateStats()
{
    if (!m_monitoringEnabled) {
        return;
    }
    
    m_stats.frameCount++;
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (m_startTime > 0) {
        m_stats.uptime = currentTime - m_startTime;
        if (m_stats.uptime > 0) {
            m_stats.frameRate = (m_stats.frameCount * 1000.0) / m_stats.uptime;
        }
    }
    
    emit statsUpdated(m_stats);
}

void CameraManager::attemptRecovery()
{
    if (!m_autoRecoveryEnabled || m_currentRetries >= m_maxRetries) {
        return;
    }
    
    m_currentRetries++;
    qDebug() << "CameraManager: Attempting recovery" << m_currentRetries << "/" << m_maxRetries;
    
    // 尝试重启摄像头
    restartCamera();
}

void CameraManager::resetRetryCount()
{
    m_currentRetries = 0;
}

// === 槽函数实现 ===

void CameraManager::onCameraStateChanged(int state)
{
    CameraModule::CameraState cameraState = static_cast<CameraModule::CameraState>(state);
    
    switch (cameraState) {
        case CameraModule::Active:
            resetRetryCount();
            if (m_monitoringEnabled) {
                resetStats();
            }
            break;
        case CameraModule::Error:
            if (m_autoRecoveryEnabled) {
                m_recoveryTimer->start();
            }
            break;
        default:
            break;
    }
}

void CameraManager::onCameraStarted()
{
    emit cameraStarted();
    if (m_monitoringEnabled) {
        enableMonitoring(true);
    }
}

void CameraManager::onCameraStopped()
{
    emit cameraStopped();
    if (m_monitoringEnabled) {
        m_statsTimer->stop();
    }
}

void CameraManager::onCameraError(const QString& error)
{
    m_lastError = error;
    m_stats.errorCount++;
    emit cameraError(error);
    
    if (m_autoRecoveryEnabled) {
        m_recoveryTimer->start();
    }
}

void CameraManager::onDeviceChanged(const QVariantMap& device)
{
    // 处理设备变化
    refreshDevices();
}

void CameraManager::onDevicesChanged()
{
    emit devicesUpdated();
}

void CameraManager::onStatsTimer()
{
    updateStats();
}

void CameraManager::onRecoveryTimer()
{
    attemptRecovery();
}