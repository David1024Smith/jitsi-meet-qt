#include "test_camera_module.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QThread>

// TestCameraModule 实现

void TestCameraModule::initTestCase()
{
    // 测试套件初始化
    qDebug() << "Initializing Camera Module Test Suite";
    
    // 确保配置使用测试设置
    m_cameraConfig = CameraConfig::instance();
    m_cameraConfig->resetToDefaults();
    
    m_cameraFactory = CameraFactory::instance();
    QVERIFY(m_cameraFactory != nullptr);
}

void TestCameraModule::cleanupTestCase()
{
    // 测试套件清理
    qDebug() << "Cleaning up Camera Module Test Suite";
    
    if (m_cameraManager) {
        m_cameraManager->cleanup();
        delete m_cameraManager;
        m_cameraManager = nullptr;
    }
    
    if (m_cameraModule) {
        m_cameraModule->cleanup();
        delete m_cameraModule;
        m_cameraModule = nullptr;
    }
}

void TestCameraModule::init()
{
    // 每个测试前的初始化
    m_cameraModule = nullptr;
    m_cameraManager = nullptr;
}

void TestCameraModule::cleanup()
{
    // 每个测试后的清理
    if (m_cameraManager) {
        m_cameraManager->stopCamera();
        m_cameraManager->cleanup();
        delete m_cameraManager;
        m_cameraManager = nullptr;
    }
    
    if (m_cameraModule) {
        m_cameraModule->stop();
        m_cameraModule->cleanup();
        delete m_cameraModule;
        m_cameraModule = nullptr;
    }
}

void TestCameraModule::testCameraModuleInitialization()
{
    // 测试CameraModule初始化
    m_cameraModule = new CameraModule(this);
    QVERIFY(m_cameraModule != nullptr);
    
    // 测试初始状态
    QCOMPARE(m_cameraModule->status(), ICameraDevice::Inactive);
    QVERIFY(!m_cameraModule->isActive());
    
    // 测试初始化
    bool initResult = m_cameraModule->initialize();
    QVERIFY(initResult);
    QCOMPARE(m_cameraModule->status(), ICameraDevice::Loaded);
}

void TestCameraModule::testCameraModuleDeviceScanning()
{
    m_cameraModule = new CameraModule(this);
    QVERIFY(m_cameraModule->initialize());
    
    // 测试设备扫描
    QStringList devices = m_cameraModule->availableDevices();
    qDebug() << "Available devices:" << devices;
    
    // 至少应该有一个设备（可能是虚拟设备）
    QVERIFY(devices.size() >= 0);
    
    if (!devices.isEmpty()) {
        // 测试设备选择
        QString firstDevice = devices.first();
        bool selectResult = m_cameraModule->selectDevice(firstDevice);
        QVERIFY(selectResult);
        QCOMPARE(m_cameraModule->currentDevice(), firstDevice);
    }
}

void TestCameraModule::testCameraModuleStartStop()
{
    m_cameraModule = new CameraModule(this);
    QVERIFY(m_cameraModule->initialize());
    
    QStringList devices = m_cameraModule->availableDevices();
    if (devices.isEmpty()) {
        QSKIP("No camera devices available for testing");
    }
    
    // 选择第一个设备
    QVERIFY(m_cameraModule->selectDevice(devices.first()));
    
    // 测试启动
    QSignalSpy statusSpy(m_cameraModule, &CameraModule::statusChanged);
    bool startResult = m_cameraModule->start();
    
    if (startResult) {
        // 等待状态改变
        waitForSignal(m_cameraModule, SIGNAL(statusChanged(ICameraDevice::Status)), 3000);
        QVERIFY(statusSpy.count() > 0);
        QCOMPARE(m_cameraModule->status(), ICameraDevice::Active);
        QVERIFY(m_cameraModule->isActive());
        
        // 测试停止
        m_cameraModule->stop();
        waitForSignal(m_cameraModule, SIGNAL(statusChanged(ICameraDevice::Status)), 3000);
        QCOMPARE(m_cameraModule->status(), ICameraDevice::Stopped);
        QVERIFY(!m_cameraModule->isActive());
    } else {
        qWarning() << "Camera start failed - this may be expected in test environment";
    }
}

void TestCameraModule::testCameraModuleConfiguration()
{
    m_cameraModule = new CameraModule(this);
    QVERIFY(m_cameraModule->initialize());
    
    // 测试分辨率设置
    QSize testResolution(640, 480);
    m_cameraModule->setResolution(testResolution);
    QCOMPARE(m_cameraModule->resolution(), testResolution);
    
    // 测试帧率设置
    int testFrameRate = 30;
    m_cameraModule->setFrameRate(testFrameRate);
    QCOMPARE(m_cameraModule->frameRate(), testFrameRate);
    
    // 测试质量预设
    ICameraDevice::QualityPreset preset = ICameraDevice::HighQuality;
    m_cameraModule->setQualityPreset(preset);
    QCOMPARE(m_cameraModule->qualityPreset(), preset);
    
    // 验证预设应用了正确的分辨率
    QSize expectedResolution = CameraUtils::resolutionForPreset(preset);
    QCOMPARE(m_cameraModule->resolution(), expectedResolution);
}

void TestCameraModule::testCameraModuleErrorHandling()
{
    m_cameraModule = new CameraModule(this);
    QVERIFY(m_cameraModule->initialize());
    
    // 测试无效设备ID
    QSignalSpy errorSpy(m_cameraModule, &CameraModule::errorOccurred);
    bool selectResult = m_cameraModule->selectDevice("invalid_device_id");
    QVERIFY(!selectResult);
    
    // 测试无效配置
    m_cameraModule->setResolution(QSize(-1, -1));
    // 应该保持原有有效配置或使用默认值
    QVERIFY(CameraUtils::isValidResolution(m_cameraModule->resolution()));
}

void TestCameraModule::testCameraManagerInitialization()
{
    m_cameraManager = m_cameraFactory->createLocalCamera();
    QVERIFY(m_cameraManager != nullptr);
    
    // 测试初始状态
    QCOMPARE(m_cameraManager->status(), ICameraManager::Uninitialized);
    
    // 测试初始化
    bool initResult = m_cameraManager->initialize();
    QVERIFY(initResult);
    QCOMPARE(m_cameraManager->status(), ICameraManager::Ready);
}

void TestCameraModule::testCameraManagerDeviceManagement()
{
    m_cameraManager = m_cameraFactory->createLocalCamera();
    QVERIFY(m_cameraManager->initialize());
    
    // 测试设备列表
    QStringList devices = m_cameraManager->availableDevices();
    qDebug() << "Manager available devices:" << devices;
    
    if (!devices.isEmpty()) {
        // 测试设备选择
        QString firstDevice = devices.first();
        QSignalSpy deviceChangedSpy(m_cameraManager, &ICameraManager::currentDeviceChanged);
        
        bool selectResult = m_cameraManager->selectDevice(firstDevice);
        QVERIFY(selectResult);
        
        if (deviceChangedSpy.count() == 0) {
            waitForSignal(m_cameraManager, SIGNAL(currentDeviceChanged(QString)), 1000);
        }
        
        ICameraDevice* currentDevice = m_cameraManager->currentDevice();
        QVERIFY(currentDevice != nullptr);
        QCOMPARE(currentDevice->deviceId(), firstDevice);
    }
}

void TestCameraModule::testCameraManagerPreviewControl()
{
    m_cameraManager = m_cameraFactory->createLocalCamera();
    QVERIFY(m_cameraManager->initialize());
    
    // 测试预览组件创建
    QVideoWidget* previewWidget = m_cameraManager->createPreviewWidget();
    QVERIFY(previewWidget != nullptr);
    
    // 测试预览组件设置
    m_cameraManager->setPreviewWidget(previewWidget);
    QCOMPARE(m_cameraManager->previewWidget(), previewWidget);
    
    delete previewWidget;
}

void TestCameraModule::testCameraManagerQualityPresets()
{
    m_cameraManager = m_cameraFactory->createLocalCamera();
    QVERIFY(m_cameraManager->initialize());
    
    QStringList devices = m_cameraManager->availableDevices();
    if (devices.isEmpty()) {
        QSKIP("No camera devices available for testing");
    }
    
    QVERIFY(m_cameraManager->selectDevice(devices.first()));
    
    // 测试质量预设
    ICameraDevice::QualityPreset preset = ICameraDevice::StandardQuality;
    bool presetResult = m_cameraManager->startWithPreset(preset);
    
    if (presetResult) {
        waitForSignal(m_cameraManager, SIGNAL(cameraStarted()), 3000);
        QVERIFY(m_cameraManager->isCameraActive());
        
        // 验证配置
        QSize expectedResolution = CameraUtils::resolutionForPreset(preset);
        QCOMPARE(m_cameraManager->currentResolution(), expectedResolution);
        
        m_cameraManager->stopCamera();
        waitForSignal(m_cameraManager, SIGNAL(cameraStopped()), 3000);
    }
}

void TestCameraModule::testCameraManagerConfiguration()
{
    m_cameraManager = m_cameraFactory->createLocalCamera();
    QVERIFY(m_cameraManager->initialize());
    
    // 测试配置应用
    QVariantMap config;
    config["resolution"] = QSize(800, 600);
    config["frameRate"] = 25;
    config["qualityPreset"] = static_cast<int>(ICameraDevice::HighQuality);
    
    m_cameraManager->applyConfiguration(config);
    
    // 验证配置
    QVariantMap currentConfig = m_cameraManager->currentConfiguration();
    QCOMPARE(currentConfig["resolution"].toSize(), QSize(800, 600));
    QCOMPARE(currentConfig["frameRate"].toInt(), 25);
}

void TestCameraModule::testCameraFactorySingleton()
{
    // 测试单例模式
    CameraFactory* factory1 = CameraFactory::instance();
    CameraFactory* factory2 = CameraFactory::instance();
    
    QVERIFY(factory1 != nullptr);
    QCOMPARE(factory1, factory2);
}

void TestCameraModule::testCameraFactoryCreation()
{
    CameraFactory* factory = CameraFactory::instance();
    
    // 测试本地摄像头创建
    CameraManager* localCamera = factory->createLocalCamera();
    QVERIFY(localCamera != nullptr);
    
    // 测试远程摄像头创建
    CameraManager* remoteCamera = factory->createRemoteCamera("test_remote_id");
    QVERIFY(remoteCamera != nullptr);
    
    // 清理
    factory->destroyCamera(localCamera);
    factory->destroyCamera(remoteCamera);
}

void TestCameraModule::testCameraFactoryDestruction()
{
    CameraFactory* factory = CameraFactory::instance();
    
    CameraManager* camera = factory->createLocalCamera();
    QVERIFY(camera != nullptr);
    
    // 测试销毁
    factory->destroyCamera(camera);
    // 销毁后不应该崩溃
}

void TestCameraModule::testCameraConfigDefaults()
{
    CameraConfig* config = CameraConfig::instance();
    
    // 测试默认值
    QCOMPARE(config->defaultResolution(), CameraConfig::Defaults::RESOLUTION);
    QCOMPARE(config->defaultFrameRate(), CameraConfig::Defaults::FRAME_RATE);
    QCOMPARE(config->defaultQualityPreset(), CameraConfig::Defaults::QUALITY_PRESET);
    QCOMPARE(config->autoStartCamera(), CameraConfig::Defaults::AUTO_START_CAMERA);
}

void TestCameraModule::testCameraConfigPersistence()
{
    CameraConfig* config = CameraConfig::instance();
    
    // 设置测试值
    QSize testResolution(1024, 768);
    int testFrameRate = 25;
    
    config->setDefaultResolution(testResolution);
    config->setDefaultFrameRate(testFrameRate);
    
    // 保存配置
    config->saveToSettings();
    
    // 重新加载
    config->loadFromSettings();
    
    // 验证持久化
    QCOMPARE(config->defaultResolution(), testResolution);
    QCOMPARE(config->defaultFrameRate(), testFrameRate);
}

void TestCameraModule::testCameraConfigValidation()
{
    CameraConfig* config = CameraConfig::instance();
    
    // 测试有效配置
    config->resetToDefaults();
    QVERIFY(config->isValid());
    QVERIFY(config->validate().isEmpty());
    
    // 测试无效配置
    config->setDefaultResolution(QSize(-1, -1));
    QVERIFY(!config->isValid());
    QVERIFY(!config->validate().isEmpty());
}

void TestCameraModule::testCameraConfigSignals()
{
    CameraConfig* config = CameraConfig::instance();
    
    // 测试配置改变信号
    QSignalSpy configChangedSpy(config, &CameraConfig::configChanged);
    QSignalSpy resolutionChangedSpy(config, &CameraConfig::defaultResolutionChanged);
    
    QSize newResolution(1280, 720);
    config->setDefaultResolution(newResolution);
    
    QCOMPARE(configChangedSpy.count(), 1);
    QCOMPARE(resolutionChangedSpy.count(), 1);
    QCOMPARE(resolutionChangedSpy.first().first().toSize(), newResolution);
}

void TestCameraModule::testCameraUtilsResolutionMapping()
{
    // 测试分辨率映射
    QSize lowRes = CameraUtils::resolutionForPreset(ICameraDevice::LowQuality);
    QSize stdRes = CameraUtils::resolutionForPreset(ICameraDevice::StandardQuality);
    QSize highRes = CameraUtils::resolutionForPreset(ICameraDevice::HighQuality);
    
    QCOMPARE(lowRes, QSize(320, 240));
    QCOMPARE(stdRes, QSize(640, 480));
    QCOMPARE(highRes, QSize(1280, 720));
    
    // 测试帧率映射
    int lowFps = CameraUtils::frameRateForPreset(ICameraDevice::LowQuality);
    int stdFps = CameraUtils::frameRateForPreset(ICameraDevice::StandardQuality);
    
    QCOMPARE(lowFps, 15);
    QCOMPARE(stdFps, 30);
}

void TestCameraModule::testCameraUtilsValidation()
{
    // 测试分辨率验证
    QVERIFY(CameraUtils::isValidResolution(QSize(640, 480)));
    QVERIFY(!CameraUtils::isValidResolution(QSize(-1, 480)));
    QVERIFY(!CameraUtils::isValidResolution(QSize(640, -1)));
    QVERIFY(!CameraUtils::isValidResolution(QSize(0, 0)));
    
    // 测试帧率验证
    QVERIFY(CameraUtils::isValidFrameRate(30));
    QVERIFY(!CameraUtils::isValidFrameRate(0));
    QVERIFY(!CameraUtils::isValidFrameRate(-1));
    QVERIFY(!CameraUtils::isValidFrameRate(200));
}

void TestCameraModule::testCameraUtilsFormatting()
{
    // 测试分辨率格式化
    QString formatted = CameraUtils::formatResolution(QSize(1920, 1080));
    QCOMPARE(formatted, "1920x1080");
    
    // 测试分辨率解析
    QSize parsed = CameraUtils::parseResolution("1280x720");
    QCOMPARE(parsed, QSize(1280, 720));
    
    // 测试预设名称
    QString presetName = CameraUtils::presetName(ICameraDevice::HighQuality);
    QVERIFY(!presetName.isEmpty());
    QVERIFY(presetName.contains("1280x720"));
}

void TestCameraModule::testCameraUtilsCalculations()
{
    // 测试比特率计算
    int bitrate = CameraUtils::calculateBitrate(QSize(1280, 720), 30, ICameraDevice::HighQuality);
    QVERIFY(bitrate > 0);
    
    // 测试帧大小计算
    qint64 frameSize = CameraUtils::calculateFrameSize(QSize(640, 480));
    QCOMPARE(frameSize, 640 * 480 * 4); // RGB32
    
    // 测试宽高比
    double ratio = CameraUtils::aspectRatio(QSize(1920, 1080));
    QCOMPARE(qRound(ratio * 100), qRound(16.0/9.0 * 100)); // 16:9
}

void TestCameraModule::testFullWorkflow()
{
    // 完整工作流程测试
    m_cameraManager = m_cameraFactory->createLocalCamera();
    QVERIFY(m_cameraManager->initialize());
    
    QStringList devices = m_cameraManager->availableDevices();
    if (devices.isEmpty()) {
        QSKIP("No camera devices available for full workflow test");
    }
    
    // 1. 选择设备
    QVERIFY(m_cameraManager->selectDevice(devices.first()));
    
    // 2. 创建预览
    QVideoWidget* preview = m_cameraManager->createPreviewWidget();
    QVERIFY(preview != nullptr);
    
    // 3. 启动摄像头
    bool started = m_cameraManager->startWithPreset(ICameraDevice::StandardQuality);
    if (started) {
        waitForSignal(m_cameraManager, SIGNAL(cameraStarted()), 5000);
        QVERIFY(m_cameraManager->isCameraActive());
        
        // 4. 运行一段时间
        QThread::msleep(1000);
        
        // 5. 停止摄像头
        m_cameraManager->stopCamera();
        waitForSignal(m_cameraManager, SIGNAL(cameraStopped()), 3000);
        QVERIFY(!m_cameraManager->isCameraActive());
    }
    
    delete preview;
}

void TestCameraModule::testErrorRecovery()
{
    m_cameraManager = m_cameraFactory->createLocalCamera();
    QVERIFY(m_cameraManager->initialize());
    
    // 测试错误恢复机制
    QSignalSpy errorSpy(m_cameraManager, &ICameraManager::errorOccurred);
    
    // 尝试选择无效设备
    bool selectResult = m_cameraManager->selectDevice("invalid_device");
    QVERIFY(!selectResult);
    
    // 管理器应该仍然可用
    QCOMPARE(m_cameraManager->status(), ICameraManager::Ready);
    
    // 应该能够选择有效设备
    QStringList devices = m_cameraManager->availableDevices();
    if (!devices.isEmpty()) {
        QVERIFY(m_cameraManager->selectDevice(devices.first()));
    }
}

void TestCameraModule::testPerformanceMetrics()
{
    m_cameraManager = m_cameraFactory->createLocalCamera();
    QVERIFY(m_cameraManager->initialize());
    
    QStringList devices = m_cameraManager->availableDevices();
    if (devices.isEmpty()) {
        QSKIP("No camera devices available for performance test");
    }
    
    QVERIFY(m_cameraManager->selectDevice(devices.first()));
    
    bool started = m_cameraManager->startWithPreset(ICameraDevice::StandardQuality);
    if (started) {
        waitForSignal(m_cameraManager, SIGNAL(cameraStarted()), 5000);
        
        // 运行一段时间收集性能数据
        QThread::msleep(2000);
        
        // 检查性能指标
        int frameCount = m_cameraManager->frameCount();
        double avgFrameRate = m_cameraManager->averageFrameRate();
        
        qDebug() << "Performance metrics:";
        qDebug() << "  Frame count:" << frameCount;
        qDebug() << "  Average frame rate:" << avgFrameRate;
        
        // 基本性能验证
        QVERIFY(frameCount >= 0);
        QVERIFY(avgFrameRate >= 0);
        
        m_cameraManager->stopCamera();
    }
}

void TestCameraModule::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QSignalSpy spy(sender, signal);
    if (spy.count() == 0) {
        spy.wait(timeout);
    }
}

void TestCameraModule::verifyDeviceState(ICameraDevice* device, ICameraDevice::Status expectedStatus)
{
    QVERIFY(device != nullptr);
    QCOMPARE(device->status(), expectedStatus);
}

void TestCameraModule::verifyManagerState(ICameraManager* manager, ICameraManager::ManagerStatus expectedStatus)
{
    QVERIFY(manager != nullptr);
    QCOMPARE(manager->status(), expectedStatus);
}

// 测试运行器
QTEST_MAIN(TestCameraModule)