#include "CameraModule.h"
#include "../utils/CameraUtils.h"
#include <QDebug>
#include <QApplication>
#include <QPermission>
// #include <QCameraPermission> // Qt 6.5+ only
#include <QVideoFrame>
#include <QRandomGenerator>

CameraModule::CameraModule(QObject *parent)
    : ICameraDevice(parent)
    , m_camera(nullptr)
    , m_captureSession(nullptr)
    , m_videoWidget(nullptr)
    , m_state(Stopped)
    , m_currentDeviceId()
    , m_statusCheckTimer(new QTimer(this))
    , m_deviceScanTimer(new QTimer(this))
    , m_initialized(false)
    , m_hasPermission(false)
    , m_autoRestart(true)
{
    qDebug() << "CameraModule: Initializing...";
    initialize();
}

CameraModule::~CameraModule()
{
    qDebug() << "CameraModule: Destroying...";
    cleanup();
}

// === ICameraDevice接口实现 ===

bool CameraModule::initialize()
{
    qDebug() << "CameraModule::initialize() - Starting initialization, already initialized:" << m_initialized;
    
    if (m_initialized) {
        qDebug() << "CameraModule::initialize() - Already initialized, returning true";
        return true;
    }

    qDebug() << "CameraModule::initialize() - Creating capture session";

    // 创建捕获会话
    m_captureSession = std::make_unique<QMediaCaptureSession>(this);

    qDebug() << "CameraModule::initialize() - Setting up status check timer";
    // 设置状态检查定时器
    m_statusCheckTimer->setInterval(5000); // 5秒检查一次
    connect(m_statusCheckTimer, &QTimer::timeout, this, &CameraModule::onStatusCheckTimer);

    // 设置设备扫描定时器
    m_deviceScanTimer->setInterval(10000); // 10秒扫描一次
    connect(m_deviceScanTimer, &QTimer::timeout, this, &CameraModule::onDeviceListChanged);

    // 初始扫描设备
    updateDeviceList();

    // 检查权限
    m_hasPermission = checkPermission();

    m_initialized = true;
    
    emit statusChanged(ICameraDevice::Loaded);
    qDebug() << "CameraModule: Initialization completed";
    return true;
}

void CameraModule::cleanup()
{
    qDebug() << "CameraModule: Cleaning up resources";

    // 停止定时器
    if (m_statusCheckTimer) {
        m_statusCheckTimer->stop();
    }
    if (m_deviceScanTimer) {
        m_deviceScanTimer->stop();
    }

    // 停止摄像头
    stop();

    // 清理摄像头对象
    destroyCamera();

    // 清理捕获会话
    if (m_captureSession) {
        m_captureSession.reset();
    }

    m_initialized = false;
    emit statusChanged(ICameraDevice::Inactive);
    qDebug() << "CameraModule: Cleanup completed";
}

bool CameraModule::start()
{
    qDebug() << "CameraModule::start() - Starting with current config";
    return start(m_config);
}

bool CameraModule::start(const CameraConfig& config)
{
    qDebug() << "CameraModule::start(config) - Starting with config, device:" << config.deviceId;
    
    if (m_state == Active) {
        qDebug() << "CameraModule::start(config) - Already active, returning true";
        return true;
    }
    
    if (m_state == Starting) {
        qDebug() << "CameraModule::start(config) - Already starting, returning false";
        return false;
    }
    
    qDebug() << "CameraModule::start(config) - Setting state to Starting";
    setState(Starting);
    
    // 保存配置
    m_config = config;
    
    // 创建摄像头
    QString deviceId = config.deviceId.isEmpty() ? QString() : config.deviceId;
    qDebug() << "CameraModule::start(config) - Creating camera with device:" << deviceId;
    
    if (!createCamera(deviceId)) {
        qDebug() << "CameraModule::start(config) - Failed to create camera";
        setState(Error);
        emit errorOccurred("Failed to create camera");
        return false;
    }
    
    // 应用配置
    qDebug() << "CameraModule::start(config) - Applying configuration";
    applyConfig();
    
    // 启动摄像头
    qDebug() << "CameraModule::start(config) - Starting camera";
    if (m_camera) {
        m_camera->start();
        setState(Active);
        emit started();
        qDebug() << "CameraModule::start(config) - Camera started successfully";
        return true;
    }
    
    qDebug() << "CameraModule::start(config) - No camera object available";
    setState(Error);
    emit errorOccurred("No camera object available");
    return false;
}

bool CameraModule::startDefault()
{
    qDebug() << "CameraModule::startDefault() - Starting with default config";
    CameraConfig defaultConfig;
    return start(defaultConfig);
}

void CameraModule::stop()
{
    qDebug() << "CameraModule::stop() - Stopping camera, current state:" << m_state;
    
    if (m_state == Stopped || m_state == Stopping) {
        qDebug() << "CameraModule::stop() - Already stopped or stopping";
        return;
    }
    
    setState(Stopping);
    
    if (m_camera) {
        qDebug() << "CameraModule::stop() - Stopping camera object";
        m_camera->stop();
    }
    
    setState(Stopped);
    emit stopped();
    qDebug() << "CameraModule::stop() - Camera stopped successfully";
}

bool CameraModule::isActive() const
{
    bool active = (m_state == Active);
    qDebug() << "CameraModule::isActive() - State:" << m_state << "Active:" << active;
    return active;
}

ICameraDevice::Status CameraModule::status() const
{
    switch (m_state) {
        case Stopped: return ICameraDevice::Stopped;
        case Starting: return ICameraDevice::Starting;
        case Active: return ICameraDevice::Active;
        case Stopping: return ICameraDevice::Stopping;
        case Error: return ICameraDevice::Error;
        default: return ICameraDevice::Inactive;
    }
}

QString CameraModule::deviceId() const
{
    return m_currentDeviceId;
}

QString CameraModule::deviceName() const
{
    CameraDevice device = currentDevice();
    return device.name;
}

QString CameraModule::description() const
{
    CameraDevice device = currentDevice();
    return device.description;
}

bool CameraModule::isAvailable() const
{
    bool available = !m_devices.isEmpty() && m_initialized;
    qDebug() << "CameraModule::isAvailable() - Available:" << available;
    return available;
}

void CameraModule::setResolution(const QSize& resolution)
{
    qDebug() << "CameraModule::setResolution() - Setting resolution to:" << resolution;
    m_config.resolution = resolution;
}

QSize CameraModule::resolution() const
{
    qDebug() << "CameraModule::resolution() - Current resolution:" << m_config.resolution;
    return m_config.resolution;
}

void CameraModule::setFrameRate(int frameRate)
{
    qDebug() << "CameraModule::setFrameRate() - Setting frame rate to:" << frameRate;
    m_config.frameRate = frameRate;
}

int CameraModule::frameRate() const
{
    qDebug() << "CameraModule::frameRate() - Current frame rate:" << m_config.frameRate;
    return m_config.frameRate;
}

void CameraModule::setQualityPreset(QualityPreset preset)
{
    qDebug() << "CameraModule::setQualityPreset() - Setting quality preset to:" << preset;
    
    switch (preset) {
        case LowQuality:
            setResolution(QSize(320, 240));
            setFrameRate(15);
            break;
        case StandardQuality:
            setResolution(QSize(640, 480));
            setFrameRate(30);
            break;
        case HighQuality:
            setResolution(QSize(1280, 720));
            setFrameRate(30);
            break;
        case UltraQuality:
            setResolution(QSize(1920, 1080));
            setFrameRate(30);
            break;
    }
}

ICameraDevice::QualityPreset CameraModule::qualityPreset() const
{
    // 根据当前分辨率推断质量预设
    QSize res = resolution();
    if (res == QSize(320, 240)) return LowQuality;
    if (res == QSize(640, 480)) return StandardQuality;
    if (res == QSize(1280, 720)) return HighQuality;
    if (res == QSize(1920, 1080)) return UltraQuality;
    return StandardQuality; // 默认
}




QList<QSize> CameraModule::supportedResolutions() const
{
    CameraDevice device = currentDevice();
    if (!device.supportedResolutions.isEmpty()) {
        return device.supportedResolutions;
    }
    
    // 返回推荐的分辨率列表
    return CameraUtils::recommendedResolutions();
}

QList<int> CameraModule::supportedFrameRates() const
{
    // 返回推荐的帧率列表
    return CameraUtils::recommendedFrameRates();
}




QList<CameraModule::CameraDevice> CameraModule::scanDevices()
{
    qDebug() << "CameraModule: Scanning devices...";
    updateDeviceList();
    return m_devices;
}

QList<CameraModule::CameraDevice> CameraModule::availableDevices() const
{
    return m_devices;
}

CameraModule::CameraDevice CameraModule::currentDevice() const
{
    for (const auto& device : m_devices) {
        if (device.id == m_currentDeviceId) {
            return device;
        }
    }
    
    // 如果没找到，返回空设备
    return CameraDevice();
}

bool CameraModule::setDevice(const QString& deviceId)
{
    qDebug() << "CameraModule::setDevice() - Setting device to:" << deviceId;
    
    if (m_currentDeviceId == deviceId) {
        qDebug() << "CameraModule::setDevice() - Device already set";
        return true;
    }
    
    bool wasActive = isActive();
    if (wasActive) {
        qDebug() << "CameraModule::setDevice() - Stopping current camera";
        stop();
    }
    
    m_currentDeviceId = deviceId;
    
    if (wasActive) {
        qDebug() << "CameraModule::setDevice() - Restarting camera with new device";
        return start();
    }
    
    return true;
}

void CameraModule::setConfig(const CameraConfig& config)
{
    qDebug() << "CameraModule::setConfig() - Setting new configuration";
    m_config = config;
}

CameraModule::CameraConfig CameraModule::config() const
{
    return m_config;
}







void CameraModule::restart()
{
    qDebug() << "CameraModule: Restarting camera";
    
    CameraConfig currentConfig = m_config;
    stop();
    
    // 短暂延迟后重启
    QTimer::singleShot(500, [this, currentConfig]() {
        start(currentConfig);
    });
}

bool CameraModule::forceStart()
{
    qDebug() << "CameraModule: Force starting camera (bypass permission check)";

    // 创建强制启动配置
    CameraConfig forceConfig = m_config;
    forceConfig.enablePermissionCheck = false;

    // 确保有设备
    if (m_devices.isEmpty()) {
        updateDeviceList();
    }

    if (m_devices.isEmpty()) {
        qWarning() << "CameraModule: No devices available for force start";
        return false;
    }

    // 强制创建摄像头
    if (!m_camera) {
        QCameraDevice device = QMediaDevices::videoInputs().first();
        m_currentDeviceId = QString::fromUtf8(device.id());
        
        if (!createCamera(m_currentDeviceId)) {
            qWarning() << "CameraModule: Force create camera failed";
            return false;
        }
    }

    // 强制启动
    setState(Starting);
    
    if (m_camera) {
        m_camera->start();
        m_statusCheckTimer->start();
        
        qDebug() << "CameraModule: Force start initiated";
        return true;
    }

    return false;
}

CameraModule::CameraState CameraModule::state() const
{
    return m_state;
}



bool CameraModule::hasDevices() const
{
    return !m_devices.isEmpty();
}

QVideoWidget* CameraModule::videoWidget() const
{
    return m_videoWidget;
}

void CameraModule::setVideoWidget(QVideoWidget* widget)
{
    qDebug() << "CameraModule: Setting video widget";

    m_videoWidget = widget;

    if (m_captureSession && widget) {
        m_captureSession->setVideoOutput(widget);
        qDebug() << "CameraModule: Video widget connected to capture session";

        // 如果摄像头还没启动，自动启动
        if (m_state == Stopped && m_config.autoStart) {
            qDebug() << "CameraModule: Auto-starting camera for video widget";
            forceStart();
        }
    }
}

QVideoWidget* CameraModule::createVideoWidget(QWidget* parent)
{
    qDebug() << "CameraModule: Creating video widget";

    QVideoWidget* widget = new QVideoWidget(parent);
    widget->setMinimumSize(320, 240);
    widget->setStyleSheet(
        "QVideoWidget {"
        "    background-color: #1a1a1a;"
        "    border: 2px solid #4CAF50;"
        "    border-radius: 8px;"
        "}"
    );

    // 自动设置为当前视频组件
    setVideoWidget(widget);

    return widget;
}



bool CameraModule::checkPermission()
{
    QCameraPermission cameraPermission;
    bool hasPermission = (qApp->checkPermission(cameraPermission) == Qt::PermissionStatus::Granted);
    
    qDebug() << "CameraModule: Camera permission check result:" << hasPermission;
    return hasPermission;
}

void CameraModule::requestPermission()
{
    qDebug() << "CameraModule: Requesting camera permission";

    QCameraPermission cameraPermission;
    qApp->requestPermission(cameraPermission, this, [this](const QPermission &permission) {
        bool granted = (permission.status() == Qt::PermissionStatus::Granted);
        m_hasPermission = granted;
        
        qDebug() << "CameraModule: Permission request result:" << granted;
        emit permissionResult(granted);
        
        if (!granted) {
            emit permissionDenied();
        }
    });
}

// === 私有槽函数 ===

void CameraModule::onCameraActiveChanged(bool active)
{
    qDebug() << "CameraModule: Camera active changed:" << active;

    if (active) {
        setState(Active);
        emit started();
    } else {
        if (m_state != Stopping) {
            // 意外停止，可能需要重启
            qWarning() << "CameraModule: Camera unexpectedly stopped";
            if (m_autoRestart) {
                qDebug() << "CameraModule: Auto-restarting camera";
                QTimer::singleShot(1000, this, &CameraModule::restart);
            }
        }
    }
}

void CameraModule::onCameraError(QCamera::Error error)
{
    qWarning() << "CameraModule: Camera error occurred:" << error;

    QString errorMessage;
    switch (error) {
        case QCamera::NoError:
            return;
        case QCamera::CameraError:
            errorMessage = "Camera hardware error";
            break;
        default:
            errorMessage = QString("Camera error: %1").arg(static_cast<int>(error));
            break;
    }

    setState(Error);
    emit errorOccurred(errorMessage);
}

void CameraModule::onDeviceListChanged()
{
    qDebug() << "CameraModule: Device list changed, updating...";
    updateDeviceList();
    emit devicesChanged();
}

void CameraModule::onStatusCheckTimer()
{
    if (!m_camera) {
        return;
    }

    // 检查摄像头状态
    bool cameraActive = m_camera->isActive();
    bool cameraAvailable = m_camera->isAvailable();

    qDebug() << "CameraModule: Status check - Active:" << cameraActive << "Available:" << cameraAvailable;

    // 如果摄像头应该是激活的但实际不是，尝试重启
    if (m_state == Active && !cameraActive && m_autoRestart) {
        qWarning() << "CameraModule: Camera should be active but isn't, restarting";
        restart();
    }
}

// === 私有方法 ===

bool CameraModule::createCamera(const QString& deviceId)
{
    qDebug() << "CameraModule: Creating camera for device:" << deviceId;

    // 清理现有摄像头
    destroyCamera();

    // 获取设备列表
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        qWarning() << "CameraModule: No camera devices available";
        return false;
    }

    // 选择设备
    QCameraDevice selectedDevice;
    if (deviceId.isEmpty()) {
        // 使用默认设备
        selectedDevice = cameras.first();
        m_currentDeviceId = QString::fromUtf8(selectedDevice.id());
    } else {
        // 查找指定设备
        for (const auto& device : cameras) {
            if (QString::fromUtf8(device.id()) == deviceId) {
                selectedDevice = device;
                m_currentDeviceId = deviceId;
                break;
            }
        }
        
        if (selectedDevice.isNull()) {
            qWarning() << "CameraModule: Specified device not found:" << deviceId;
            return false;
        }
    }

    // 创建摄像头对象
    m_camera = std::make_unique<QCamera>(selectedDevice, this);
    
    // 连接信号
    connectCameraSignals();

    // 设置到捕获会话
    if (m_captureSession) {
        m_captureSession->setCamera(m_camera.get());
        
        if (m_videoWidget) {
            m_captureSession->setVideoOutput(m_videoWidget);
        }
    }

    qDebug() << "CameraModule: Camera created successfully for device:" << selectedDevice.description();
    return true;
}

void CameraModule::destroyCamera()
{
    if (m_camera) {
        qDebug() << "CameraModule: Destroying camera";
        
        disconnectCameraSignals();
        
        if (m_camera->isActive()) {
            m_camera->stop();
        }
        
        m_camera.reset();
    }
}

void CameraModule::setState(CameraState state)
{
    if (m_state != state) {
        CameraState oldState = m_state;
        m_state = state;
        
        qDebug() << "CameraModule: State changed from" << oldState << "to" << state;
        emit stateChanged(state);
        
        // 同时发射ICameraDevice接口的状态信号
        emit statusChanged(status());
    }
}

void CameraModule::updateDeviceList()
{
    qDebug() << "CameraModule: Updating device list";

    m_devices.clear();
    
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    QCameraDevice defaultCamera = QMediaDevices::defaultVideoInput();

    for (const auto& camera : cameras) {
        CameraDevice device = createDeviceInfo(camera);
        device.isDefault = (camera.id() == defaultCamera.id());
        device.isActive = (QString::fromUtf8(camera.id()) == m_currentDeviceId);
        
        m_devices.append(device);
    }

    qDebug() << "CameraModule: Found" << m_devices.size() << "camera devices";
}

void CameraModule::applyConfig()
{
    if (!m_camera) {
        return;
    }

    qDebug() << "CameraModule: Applying camera configuration";
    
    // 这里可以应用分辨率、帧率等配置
    // Qt的QCamera API在某些版本中配置选项有限
    // 主要配置通过QCameraFormat来设置，但这需要更复杂的实现
    
    qDebug() << "CameraModule: Configuration applied";
}

void CameraModule::connectCameraSignals()
{
    if (m_camera) {
        connect(m_camera.get(), &QCamera::activeChanged,
                this, &CameraModule::onCameraActiveChanged);
        connect(m_camera.get(), &QCamera::errorOccurred,
                this, &CameraModule::onCameraError);
    }
}

void CameraModule::disconnectCameraSignals()
{
    if (m_camera) {
        disconnect(m_camera.get(), nullptr, this, nullptr);
    }
}

CameraModule::CameraDevice CameraModule::createDeviceInfo(const QCameraDevice& device) const
{
    CameraDevice info;
    info.id = QString::fromUtf8(device.id());
    info.name = device.description();
    info.description = device.description();
    info.isDefault = false;
    info.isActive = false;
    
    // 获取支持的分辨率
    auto formats = device.videoFormats();
    for (const auto& format : formats) {
        QSize resolution = format.resolution();
        if (!info.supportedResolutions.contains(resolution)) {
            info.supportedResolutions.append(resolution);
        }
    }
    
    return info;
}

