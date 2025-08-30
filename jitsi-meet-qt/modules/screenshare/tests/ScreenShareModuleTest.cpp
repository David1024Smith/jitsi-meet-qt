#include "ScreenShareModuleTest.h"
#include "../include/ScreenShareModule.h"
#include "../include/ScreenShareManager.h"
#include "../include/CaptureEngine.h"
#include "../interfaces/IScreenCapture.h"
#include "../capture/ScreenCapture.h"
#include "../capture/WindowCapture.h"
#include "../capture/RegionCapture.h"
#include "../encoding/VideoEncoder.h"
#include "../encoding/FrameProcessor.h"
#include "../widgets/ScreenShareWidget.h"
#include "../widgets/ScreenSelector.h"
#include "../widgets/CapturePreview.h"
#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include <QElapsedTimer>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QPoint>
#include <QVariantMap>
#include <QStringList>
#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>
#include <numeric>
#include <algorithm>

ScreenShareModuleTest::ScreenShareModuleTest(QObject *parent)
    : QObject(parent)
    , m_module(nullptr)
    , m_manager(nullptr)
    , m_captureEngine(nullptr)
{
}

ScreenShareModuleTest::~ScreenShareModuleTest()
{
    cleanup();
}

void ScreenShareModuleTest::initTestCase()
{
    // 测试套件初始化
    qDebug() << "Initializing ScreenShare Module Test Suite";
}

void ScreenShareModuleTest::cleanupTestCase()
{
    // 测试套件清理
    qDebug() << "Cleaning up ScreenShare Module Test Suite";
}

void ScreenShareModuleTest::init()
{
    // 每个测试前的初始化
    m_module = new ScreenShareModule(this);
    QVERIFY(m_module != nullptr);
}

void ScreenShareModuleTest::cleanup()
{
    // 每个测试后的清理
    if (m_module) {
        m_module->shutdown();
        m_module->deleteLater();
        m_module = nullptr;
    }
    
    if (m_manager) {
        m_manager->deleteLater();
        m_manager = nullptr;
    }
    
    if (m_captureEngine) {
        m_captureEngine->deleteLater();
        m_captureEngine = nullptr;
    }
}
    
    if (m_manager) {
        m_manager->deleteLater();
        m_manager = nullptr;
    }
    
    if (m_captureEngine) {
        m_captureEngine->deleteLater();
        m_captureEngine = nullptr;
    }
}

void ScreenShareModuleTest::testModuleInitialization()
{
    // 测试模块初始化
    QVERIFY(!m_module->isInitialized());
    QCOMPARE(m_module->status(), ScreenShareModule::NotLoaded);
    
    // 初始化模块
    bool result = m_module->initialize();
    QVERIFY(result);
    QVERIFY(m_module->isInitialized());
    QCOMPARE(m_module->status(), ScreenShareModule::Ready);
    
    // 验证版本信息
    QVERIFY(!m_module->version().isEmpty());
    QCOMPARE(m_module->moduleName(), QString("ScreenShare"));
    
    // 验证依赖项
    QStringList deps = m_module->dependencies();
    QVERIFY(deps.contains("Qt5Core"));
    QVERIFY(deps.contains("Qt5Gui"));
    QVERIFY(deps.contains("Qt5Widgets"));
}

void ScreenShareModuleTest::testModuleConfiguration()
{
    // 初始化模块
    QVERIFY(m_module->initialize());
    
    // 测试配置设置
    QVariantMap config;
    config["quality"] = static_cast<int>(IScreenCapture::HighQuality);
    config["frameRate"] = 60;
    config["bitrate"] = 5000;
    
    m_module->setConfiguration(config);
    QVariantMap retrievedConfig = m_module->configuration();
    
    QCOMPARE(retrievedConfig["quality"].toInt(), static_cast<int>(IScreenCapture::HighQuality));
    QCOMPARE(retrievedConfig["frameRate"].toInt(), 60);
    QCOMPARE(retrievedConfig["bitrate"].toInt(), 5000);
}

void ScreenShareModuleTest::testManagerAccess()
{
    // 初始化模块
    QVERIFY(m_module->initialize());
    
    // 获取管理器
    IScreenShareManager* manager = m_module->screenShareManager();
    QVERIFY(manager != nullptr);
    QVERIFY(manager->isReady());
    QCOMPARE(manager->status(), IScreenShareManager::Ready);
}

void ScreenShareModuleTest::testScreenShareManagerInitialization()
{
    // 创建管理器
    m_manager = new ScreenShareManager(this);
    QVERIFY(m_manager != nullptr);
    
    // 测试初始化
    QCOMPARE(m_manager->status(), IScreenShareManager::Uninitialized);
    QVERIFY(!m_manager->isReady());
    
    bool result = m_manager->initialize();
    QVERIFY(result);
    QVERIFY(m_manager->isReady());
    QCOMPARE(m_manager->status(), IScreenShareManager::Ready);
}

void ScreenShareModuleTest::testScreenShareManagerConfiguration()
{
    // 初始化管理器
    m_manager = new ScreenShareManager(this);
    QVERIFY(m_manager->initialize());
    
    // 测试配置
    m_manager->setShareMode(IScreenShareManager::NetworkShare);
    QCOMPARE(m_manager->shareMode(), IScreenShareManager::NetworkShare);
    
    m_manager->setEncodingFormat(IScreenShareManager::VP8);
    QCOMPARE(m_manager->encodingFormat(), IScreenShareManager::VP8);
    
    m_manager->setQuality(IScreenCapture::HighQuality);
    QCOMPARE(m_manager->quality(), IScreenCapture::HighQuality);
    
    m_manager->setFrameRate(30);
    QCOMPARE(m_manager->frameRate(), 30);
    
    m_manager->setBitrate(2000);
    QCOMPARE(m_manager->bitrate(), 2000);
}

void ScreenShareModuleTest::testAvailableSources()
{
    // 初始化管理器
    m_manager = new ScreenShareManager(this);
    QVERIFY(m_manager->initialize());
    
    // 测试可用屏幕
    QStringList screens = m_manager->availableScreens();
    QVERIFY(!screens.isEmpty()); // 至少应该有一个屏幕
    
    // 测试可用窗口
    QStringList windows = m_manager->availableWindows();
    QVERIFY(!windows.isEmpty()); // 至少应该有桌面窗口
}

void ScreenShareModuleTest::testSourceSelection()
{
    // 初始化管理器
    m_manager = new ScreenShareManager(this);
    QVERIFY(m_manager->initialize());
    
    // 选择屏幕
    QStringList screens = m_manager->availableScreens();
    if (!screens.isEmpty()) {
        bool result = m_manager->selectScreen(screens.first());
        QVERIFY(result);
        QCOMPARE(m_manager->currentSource(), screens.first());
    }
    
    // 选择窗口
    QStringList windows = m_manager->availableWindows();
    if (!windows.isEmpty()) {
        bool result = m_manager->selectWindow(windows.first());
        QVERIFY(result);
        QCOMPARE(m_manager->currentSource(), windows.first());
    }
}

void ScreenShareModuleTest::testCaptureEngineInitialization()
{
    // 创建捕获引擎
    m_captureEngine = new CaptureEngine(this);
    QVERIFY(m_captureEngine != nullptr);
    
    // 测试初始化
    QVERIFY(!m_captureEngine->isInitialized());
    QCOMPARE(m_captureEngine->status(), CaptureEngine::Stopped);
    
    bool result = m_captureEngine->initialize();
    QVERIFY(result);
    QVERIFY(m_captureEngine->isInitialized());
}

void ScreenShareModuleTest::testCaptureEngineConfiguration()
{
    // 初始化捕获引擎
    m_captureEngine = new CaptureEngine(this);
    QVERIFY(m_captureEngine->initialize());
    
    // 测试配置
    m_captureEngine->setTargetFrameRate(60);
    QCOMPARE(m_captureEngine->targetFrameRate(), 60);
    
    m_captureEngine->setPerformanceMode(CaptureEngine::Performance);
    QCOMPARE(m_captureEngine->performanceMode(), CaptureEngine::Performance);
    
    m_captureEngine->setQualityAdjustmentEnabled(true);
    QVERIFY(m_captureEngine->isQualityAdjustmentEnabled());
    
    m_captureEngine->setAdaptiveFrameRate(true);
    QVERIFY(m_captureEngine->isAdaptiveFrameRateEnabled());
}

void ScreenShareModuleTest::testScreenCaptureBasic()
{
    // 创建屏幕捕获对象
    ScreenCapture* capture = new ScreenCapture(this);
    QVERIFY(capture != nullptr);
    
    // 测试初始化
    bool result = capture->initialize();
    QVERIFY(result);
    QVERIFY(capture->isInitialized());
    QCOMPARE(capture->status(), IScreenCapture::Inactive);
    
    // 测试配置
    capture->setCaptureMode(IScreenCapture::FullScreen);
    QCOMPARE(capture->captureMode(), IScreenCapture::FullScreen);
    
    capture->setCaptureQuality(IScreenCapture::MediumQuality);
    QCOMPARE(capture->captureQuality(), IScreenCapture::MediumQuality);
    
    capture->setFrameRate(30);
    QCOMPARE(capture->frameRate(), 30);
    
    capture->deleteLater();
}

void ScreenShareModuleTest::testScreenCaptureStart()
{
    // 创建屏幕捕获对象
    ScreenCapture* capture = new ScreenCapture(this);
    QVERIFY(capture->initialize());
    
    // 设置信号监听
    QSignalSpy startedSpy(capture, &IScreenCapture::captureStarted);
    QSignalSpy stoppedSpy(capture, &IScreenCapture::captureStopped);
    
    // 启动捕获
    bool result = capture->startCapture();
    QVERIFY(result);
    QVERIFY(capture->isCapturing());
    QCOMPARE(capture->status(), IScreenCapture::Active);
    QCOMPARE(startedSpy.count(), 1);
    
    // 停止捕获
    capture->stopCapture();
    QVERIFY(!capture->isCapturing());
    QCOMPARE(capture->status(), IScreenCapture::Inactive);
    QCOMPARE(stoppedSpy.count(), 1);
    
    capture->deleteLater();
}

void ScreenShareModuleTest::testStatistics()
{
    // 初始化管理器
    m_manager = new ScreenShareManager(this);
    QVERIFY(m_manager->initialize());
    
    // 获取统计信息
    QVariantMap stats = m_manager->getStatistics();
    QVERIFY(stats.contains("totalFrames"));
    QVERIFY(stats.contains("currentFPS"));
    QVERIFY(stats.contains("currentBitrate"));
    
    // 重置统计信息
    m_manager->resetStatistics();
    QCOMPARE(m_manager->getTotalFrames(), 0);
    QCOMPARE(m_manager->getCurrentFPS(), 0.0);
}

void ScreenShareModuleTest::testErrorHandling()
{
    // 测试未初始化的操作
    m_manager = new ScreenShareManager(this);
    
    // 尝试在未初始化时启动共享
    QSignalSpy errorSpy(m_manager, &IScreenShareManager::shareError);
    bool result = m_manager->startScreenShare();
    QVERIFY(!result);
    
    // 测试无效配置
    QVariantMap invalidConfig;
    invalidConfig["frameRate"] = -1; // 无效帧率
    invalidConfig["bitrate"] = -1;   // 无效比特率
    
    m_manager->setConfiguration(invalidConfig);
    // 配置应该被忽略或修正
}

void ScreenShareModuleTest::testModuleInfo()
{
    // 测试模块信息
    QVariantMap info = m_module->moduleInfo();
    
    QVERIFY(info.contains("name"));
    QVERIFY(info.contains("version"));
    QVERIFY(info.contains("description"));
    QVERIFY(info.contains("dependencies"));
    QVERIFY(info.contains("status"));
    QVERIFY(info.contains("initialized"));
    QVERIFY(info.contains("enabled"));
    
    QCOMPARE(info["name"].toString(), QString("ScreenShare"));
    QVERIFY(!info["version"].toString().isEmpty());
    QVERIFY(!info["description"].toString().isEmpty());
}

void ScreenShareModuleTest::testSelfTest()
{
    // 未初始化时的自检应该失败
    bool result = m_module->selfTest();
    QVERIFY(!result);
    
    QStringList errors = m_module->getLastErrors();
    QVERIFY(!errors.isEmpty());
    
    // 初始化后的自检应该成功
    QVERIFY(m_module->initialize());
    result = m_module->selfTest();
    QVERIFY(result);
    
    // 清除错误
    m_module->clearErrors();
    errors = m_module->getLastErrors();
    QVERIFY(errors.isEmpty());
}

// 测试捕获系统 (Task 41 implementation)
void ScreenShareModuleTest::testCaptureSystem()
{
    qDebug() << "Testing capture system implementation";
    
    // 测试屏幕捕获
    testScreenCapture();
    
    // 测试窗口捕获
    testWindowCapture();
    
    // 测试区域捕获
    testRegionCapture();
}

void ScreenShareModuleTest::testScreenCapture()
{
    qDebug() << "Testing ScreenCapture implementation";
    
    ScreenCapture* screenCapture = new ScreenCapture(this);
    QVERIFY(screenCapture != nullptr);
    
    // 测试初始化
    QVERIFY(screenCapture->initialize());
    QVERIFY(screenCapture->isInitialized());
    
    // 测试捕获配置
    screenCapture->setCaptureMode(IScreenCapture::FullScreen);
    QCOMPARE(screenCapture->captureMode(), IScreenCapture::FullScreen);
    
    screenCapture->setCaptureQuality(IScreenCapture::HighQuality);
    QCOMPARE(screenCapture->captureQuality(), IScreenCapture::HighQuality);
    
    screenCapture->setFrameRate(30);
    QCOMPARE(screenCapture->frameRate(), 30);
    
    // 测试捕获功能
    QVERIFY(screenCapture->startCapture());
    QVERIFY(screenCapture->isCapturing());
    QCOMPARE(screenCapture->status(), IScreenCapture::Active);
    
    // 测试帧捕获
    QPixmap frame = screenCapture->captureFrame();
    QVERIFY(!frame.isNull());
    QVERIFY(frame.size().isValid());
    
    QByteArray frameData = screenCapture->captureFrameData();
    QVERIFY(!frameData.isEmpty());
    
    // 测试暂停和恢复
    screenCapture->pauseCapture();
    QCOMPARE(screenCapture->status(), IScreenCapture::Paused);
    
    screenCapture->resumeCapture();
    QCOMPARE(screenCapture->status(), IScreenCapture::Active);
    
    // 测试停止捕获
    screenCapture->stopCapture();
    QCOMPARE(screenCapture->status(), IScreenCapture::Inactive);
    QVERIFY(!screenCapture->isCapturing());
    
    // 测试性能优化
    screenCapture->enableAdaptiveQuality(true);
    screenCapture->optimizeCaptureQuality();
    
    delete screenCapture;
}

void ScreenShareModuleTest::testWindowCapture()
{
    qDebug() << "Testing WindowCapture implementation";
    
    WindowCapture* windowCapture = new WindowCapture(this);
    QVERIFY(windowCapture != nullptr);
    
    // 测试初始化
    QVERIFY(windowCapture->initialize());
    QVERIFY(windowCapture->isInitialized());
    
    // 测试窗口管理
    QStringList windows = windowCapture->availableWindows();
    QVERIFY(!windows.isEmpty());
    
    // 测试窗口选择
    if (!windows.isEmpty()) {
        QString firstWindow = windows.first();
        // 简化测试 - 实际项目中需要解析窗口ID
        windowCapture->setTargetWindowTitle("Test Window");
    }
    
    // 测试捕获配置
    windowCapture->setCaptureQuality(IScreenCapture::MediumQuality);
    QCOMPARE(windowCapture->captureQuality(), IScreenCapture::MediumQuality);
    
    windowCapture->setFrameRate(25);
    QCOMPARE(windowCapture->frameRate(), 25);
    
    // 测试扩展功能
    windowCapture->setFollowWindow(true);
    QVERIFY(windowCapture->isFollowWindowEnabled());
    
    windowCapture->setCaptureClientArea(true);
    QVERIFY(windowCapture->isCaptureClientAreaEnabled());
    
    delete windowCapture;
}

void ScreenShareModuleTest::testRegionCapture()
{
    qDebug() << "Testing RegionCapture implementation";
    
    RegionCapture* regionCapture = new RegionCapture(this);
    QVERIFY(regionCapture != nullptr);
    
    // 测试初始化
    QVERIFY(regionCapture->initialize());
    QVERIFY(regionCapture->isInitialized());
    
    // 测试区域设置
    QRect testRegion(100, 100, 800, 600);
    regionCapture->setCustomRegion(testRegion);
    QCOMPARE(regionCapture->customRegion(), testRegion);
    
    // 测试选择模式
    regionCapture->setSelectionMode(RegionCapture::ManualSelection);
    QCOMPARE(regionCapture->selectionMode(), RegionCapture::ManualSelection);
    
    regionCapture->setBoundaryMode(RegionCapture::Clip);
    QCOMPARE(regionCapture->boundaryMode(), RegionCapture::Clip);
    
    // 测试区域锁定
    regionCapture->setRegionLocked(true);
    QVERIFY(regionCapture->isRegionLocked());
    
    // 测试预设区域
    QList<QRect> presets;
    presets << QRect(0, 0, 640, 480) << QRect(0, 0, 1280, 720);
    regionCapture->setPresetRegions(presets);
    QCOMPARE(regionCapture->presetRegions().size(), 2);
    
    QVERIFY(regionCapture->selectPresetRegion(0));
    QCOMPARE(regionCapture->currentPresetIndex(), 0);
    
    // 测试鼠标跟随
    regionCapture->setMouseFollowSize(QSize(300, 200));
    QCOMPARE(regionCapture->mouseFollowSize(), QSize(300, 200));
    
    regionCapture->setMouseFollowOffset(QPoint(10, 10));
    QCOMPARE(regionCapture->mouseFollowOffset(), QPoint(10, 10));
    
    delete regionCapture;
}

// 测试编码处理 (Task 41 implementation)
void ScreenShareModuleTest::testEncodingProcessing()
{
    qDebug() << "Testing encoding processing implementation";
    
    // 测试视频编码器
    testVideoEncoder();
    
    // 测试帧处理器
    testFrameProcessor();
}

void ScreenShareModuleTest::testVideoEncoder()
{
    qDebug() << "Testing VideoEncoder implementation";
    
    VideoEncoder* encoder = new VideoEncoder(this);
    QVERIFY(encoder != nullptr);
    
    // 测试初始化
    QVERIFY(encoder->initialize());
    QVERIFY(encoder->isInitialized());
    
    // 测试编码配置
    encoder->setEncodingFormat(VideoEncoder::H264);
    QCOMPARE(encoder->encodingFormat(), VideoEncoder::H264);
    
    encoder->setEncodingQuality(VideoEncoder::High);
    QCOMPARE(encoder->encodingQuality(), VideoEncoder::High);
    
    encoder->setBitrate(5000);
    QCOMPARE(encoder->bitrate(), 5000);
    
    encoder->setFrameRate(30);
    QCOMPARE(encoder->frameRate(), 30);
    
    encoder->setResolution(QSize(1920, 1080));
    QCOMPARE(encoder->resolution(), QSize(1920, 1080));
    
    // 测试编码器控制
    QVERIFY(encoder->start());
    QVERIFY(encoder->isActive());
    
    // 测试帧编码
    QPixmap testFrame(640, 480);
    testFrame.fill(Qt::blue);
    
    QByteArray encodedData = encoder->encodeFrame(testFrame);
    QVERIFY(!encodedData.isEmpty());
    
    // 测试统计信息
    QVariantMap stats = encoder->getEncodingStatistics();
    QVERIFY(!stats.isEmpty());
    QVERIFY(stats.contains("format"));
    QVERIFY(stats.contains("bitrate"));
    
    encoder->stop();
    
    delete encoder;
}

void ScreenShareModuleTest::testFrameProcessor()
{
    qDebug() << "Testing FrameProcessor implementation";
    
    FrameProcessor* processor = new FrameProcessor(this);
    QVERIFY(processor != nullptr);
    
    // 测试初始化
    QVERIFY(processor->initialize());
    QVERIFY(processor->isInitialized());
    
    // 测试尺寸和缩放
    processor->setOutputSize(QSize(1280, 720));
    QCOMPARE(processor->outputSize(), QSize(1280, 720));
    
    processor->setScalingMode(FrameProcessor::KeepAspectRatio);
    QCOMPARE(processor->scalingMode(), FrameProcessor::KeepAspectRatio);
    
    // 测试裁剪
    QRect cropRegion(50, 50, 500, 400);
    processor->setCropRegion(cropRegion);
    QCOMPARE(processor->cropRegion(), cropRegion);
    
    processor->setCropEnabled(true);
    QVERIFY(processor->isCropEnabled());
    
    // 测试旋转
    processor->setRotation(FrameProcessor::Rotate90);
    QCOMPARE(processor->rotation(), FrameProcessor::Rotate90);
    
    // 测试质量设置
    processor->setQuality(85);
    QCOMPARE(processor->quality(), 85);
    
    // 测试滤镜
    processor->addFilter(FrameProcessor::Brightness, {{"value", 20}});
    processor->addFilter(FrameProcessor::Contrast, {{"value", 10}});
    
    QList<FrameProcessor::FilterType> filters = processor->activeFilters();
    QVERIFY(filters.contains(FrameProcessor::Brightness));
    QVERIFY(filters.contains(FrameProcessor::Contrast));
    
    // 测试帧处理
    QPixmap testFrame(800, 600);
    testFrame.fill(Qt::red);
    
    QPixmap processedFrame = processor->processFrame(testFrame);
    QVERIFY(!processedFrame.isNull());
    
    // 测试异步处理
    QVERIFY(processor->processFrameAsync(testFrame));
    
    // 测试统计信息
    QVariantMap stats = processor->getProcessingStatistics();
    QVERIFY(!stats.isEmpty());
    
    // 测试重置
    processor->reset();
    processor->clearFilters();
    
    delete processor;
}

// 测试质量自适应和性能优化 (Task 41 implementation)
void ScreenShareModuleTest::testQualityAdaptiveAndPerformanceOptimization()
{
    qDebug() << "Testing quality adaptive and performance optimization";
    
    ScreenCapture* screenCapture = new ScreenCapture(this);
    QVERIFY(screenCapture->initialize());
    
    // 测试自适应质量
    screenCapture->enableAdaptiveQuality(true);
    
    // 模拟不同的系统负载情况
    for (int i = 0; i < 5; ++i) {
        screenCapture->optimizeCaptureQuality();
        QTest::qWait(100); // 等待100ms
    }
    
    // 测试性能监控
    double cpuUsage = screenCapture->getCurrentCPUUsage();
    QVERIFY(cpuUsage >= 0.0 && cpuUsage <= 100.0);
    
    qint64 memoryUsage = screenCapture->getCurrentMemoryUsage();
    QVERIFY(memoryUsage >= 0 && memoryUsage <= 100);
    
    delete screenCapture;
}

// 测试集成功能 (Task 41 implementation)
void ScreenShareModuleTest::testIntegrationFeatures()
{
    qDebug() << "Testing integration features";
    
    // 创建完整的捕获和编码流水线
    ScreenCapture* capture = new ScreenCapture(this);
    VideoEncoder* encoder = new VideoEncoder(this);
    FrameProcessor* processor = new FrameProcessor(this);
    
    // 初始化所有组件
    QVERIFY(capture->initialize());
    QVERIFY(encoder->initialize());
    QVERIFY(processor->initialize());
    
    // 配置捕获
    capture->setCaptureQuality(IScreenCapture::HighQuality);
    capture->setFrameRate(30);
    
    // 配置处理器
    processor->setOutputSize(QSize(1280, 720));
    processor->setQuality(80);
    
    // 配置编码器
    encoder->setEncodingFormat(VideoEncoder::H264);
    encoder->setBitrate(3000);
    encoder->setResolution(QSize(1280, 720));
    
    // 启动捕获
    QVERIFY(capture->startCapture());
    QVERIFY(encoder->start());
    
    // 模拟完整的处理流程
    for (int i = 0; i < 3; ++i) {
        // 捕获帧
        QPixmap frame = capture->captureFrame();
        QVERIFY(!frame.isNull());
        
        // 处理帧
        QPixmap processedFrame = processor->processFrame(frame);
        QVERIFY(!processedFrame.isNull());
        
        // 编码帧
        QByteArray encodedData = encoder->encodeFrame(processedFrame);
        QVERIFY(!encodedData.isEmpty());
        
        QTest::qWait(50); // 模拟实时处理间隔
    }
    
    // 停止捕获
    capture->stopCapture();
    encoder->stop();
    
    delete capture;
    delete encoder;
    delete processor;
}

// Task 43: 捕获质量和性能测试
void ScreenShareModuleTest::testCaptureQualityMetrics()
{
    qDebug() << "Testing capture quality metrics";
    
    ScreenCapture* capture = new ScreenCapture(this);
    QVERIFY(capture->initialize());
    QVERIFY(capture->startCapture());
    
    // 测试不同质量设置的指标
    QList<IScreenCapture::CaptureQuality> qualities = {
        IScreenCapture::LowQuality,
        IScreenCapture::MediumQuality,
        IScreenCapture::HighQuality
    };
    
    for (auto quality : qualities) {
        capture->setCaptureQuality(quality);
        QCOMPARE(capture->captureQuality(), quality);
        
        // 捕获几帧来测试质量指标
        for (int i = 0; i < 5; ++i) {
            QPixmap frame = capture->captureFrame();
            QVERIFY(!frame.isNull());
            QVERIFY(frame.size().isValid());
            
            // 验证帧质量指标
            QVariantMap metrics = capture->getQualityMetrics();
            QVERIFY(metrics.contains("frameSize"));
            QVERIFY(metrics.contains("compressionRatio"));
            QVERIFY(metrics.contains("colorDepth"));
            
            QTest::qWait(50); // 等待50ms模拟实时捕获
        }
    }
    
    capture->stopCapture();
    delete capture;
}

void ScreenShareModuleTest::testPerformanceBenchmarks()
{
    qDebug() << "Testing performance benchmarks";
    
    ScreenCapture* capture = new ScreenCapture(this);
    VideoEncoder* encoder = new VideoEncoder(this);
    
    QVERIFY(capture->initialize());
    QVERIFY(encoder->initialize());
    
    // 基准测试配置
    capture->setCaptureQuality(IScreenCapture::HighQuality);
    capture->setFrameRate(60);
    encoder->setEncodingFormat(VideoEncoder::H264);
    encoder->setBitrate(5000);
    
    QVERIFY(capture->startCapture());
    QVERIFY(encoder->start());
    
    // 性能基准测试
    QElapsedTimer timer;
    timer.start();
    
    int frameCount = 0;
    qint64 totalCaptureTime = 0;
    qint64 totalEncodeTime = 0;
    
    for (int i = 0; i < 30; ++i) { // 测试30帧
        // 测量捕获时间
        QElapsedTimer captureTimer;
        captureTimer.start();
        QPixmap frame = capture->captureFrame();
        qint64 captureTime = captureTimer.elapsed();
        
        QVERIFY(!frame.isNull());
        totalCaptureTime += captureTime;
        
        // 测量编码时间
        QElapsedTimer encodeTimer;
        encodeTimer.start();
        QByteArray encodedData = encoder->encodeFrame(frame);
        qint64 encodeTime = encodeTimer.elapsed();
        
        QVERIFY(!encodedData.isEmpty());
        totalEncodeTime += encodeTime;
        
        frameCount++;
        QTest::qWait(16); // ~60 FPS
    }
    
    qint64 totalTime = timer.elapsed();
    
    // 验证性能指标
    double avgCaptureTime = static_cast<double>(totalCaptureTime) / frameCount;
    double avgEncodeTime = static_cast<double>(totalEncodeTime) / frameCount;
    double actualFPS = (frameCount * 1000.0) / totalTime;
    
    qDebug() << "Performance Metrics:";
    qDebug() << "  Average capture time:" << avgCaptureTime << "ms";
    qDebug() << "  Average encode time:" << avgEncodeTime << "ms";
    qDebug() << "  Actual FPS:" << actualFPS;
    
    // 性能断言 (根据实际硬件调整阈值)
    QVERIFY(avgCaptureTime < 50.0); // 捕获时间应小于50ms
    QVERIFY(avgEncodeTime < 100.0); // 编码时间应小于100ms
    QVERIFY(actualFPS > 20.0); // 实际帧率应大于20 FPS
    
    capture->stopCapture();
    encoder->stop();
    
    delete capture;
    delete encoder;
}

void ScreenShareModuleTest::testMemoryUsageOptimization()
{
    qDebug() << "Testing memory usage optimization";
    
    ScreenCapture* capture = new ScreenCapture(this);
    QVERIFY(capture->initialize());
    
    // 获取初始内存使用量
    qint64 initialMemory = capture->getCurrentMemoryUsage();
    QVERIFY(initialMemory >= 0);
    
    // 启动捕获并监控内存使用
    QVERIFY(capture->startCapture());
    
    QList<qint64> memoryReadings;
    for (int i = 0; i < 20; ++i) {
        QPixmap frame = capture->captureFrame();
        QVERIFY(!frame.isNull());
        
        qint64 currentMemory = capture->getCurrentMemoryUsage();
        memoryReadings.append(currentMemory);
        
        QTest::qWait(100);
    }
    
    // 验证内存使用稳定性
    qint64 maxMemory = *std::max_element(memoryReadings.begin(), memoryReadings.end());
    qint64 minMemory = *std::min_element(memoryReadings.begin(), memoryReadings.end());
    qint64 memoryVariation = maxMemory - minMemory;
    
    qDebug() << "Memory Usage:";
    qDebug() << "  Initial:" << initialMemory << "MB";
    qDebug() << "  Max:" << maxMemory << "MB";
    qDebug() << "  Min:" << minMemory << "MB";
    qDebug() << "  Variation:" << memoryVariation << "MB";
    
    // 内存使用应该相对稳定
    QVERIFY(memoryVariation < 100); // 变化应小于100MB
    
    // 测试内存优化功能
    capture->optimizeMemoryUsage();
    qint64 optimizedMemory = capture->getCurrentMemoryUsage();
    QVERIFY(optimizedMemory <= maxMemory);
    
    capture->stopCapture();
    delete capture;
}

void ScreenShareModuleTest::testCPUUsageMonitoring()
{
    qDebug() << "Testing CPU usage monitoring";
    
    ScreenCapture* capture = new ScreenCapture(this);
    QVERIFY(capture->initialize());
    QVERIFY(capture->startCapture());
    
    QList<double> cpuReadings;
    for (int i = 0; i < 10; ++i) {
        QPixmap frame = capture->captureFrame();
        QVERIFY(!frame.isNull());
        
        double cpuUsage = capture->getCurrentCPUUsage();
        QVERIFY(cpuUsage >= 0.0 && cpuUsage <= 100.0);
        cpuReadings.append(cpuUsage);
        
        QTest::qWait(200);
    }
    
    // 计算平均CPU使用率
    double avgCPU = std::accumulate(cpuReadings.begin(), cpuReadings.end(), 0.0) / cpuReadings.size();
    double maxCPU = *std::max_element(cpuReadings.begin(), cpuReadings.end());
    
    qDebug() << "CPU Usage:";
    qDebug() << "  Average:" << avgCPU << "%";
    qDebug() << "  Maximum:" << maxCPU << "%";
    
    // CPU使用率应该在合理范围内
    QVERIFY(avgCPU < 80.0); // 平均CPU使用率应小于80%
    QVERIFY(maxCPU < 95.0); // 最大CPU使用率应小于95%
    
    capture->stopCapture();
    delete capture;
}

void ScreenShareModuleTest::testFrameRateStability()
{
    qDebug() << "Testing frame rate stability";
    
    ScreenCapture* capture = new ScreenCapture(this);
    QVERIFY(capture->initialize());
    
    // 测试不同帧率设置
    QList<int> frameRates = {15, 30, 60};
    
    for (int targetFPS : frameRates) {
        capture->setFrameRate(targetFPS);
        QCOMPARE(capture->frameRate(), targetFPS);
        
        QVERIFY(capture->startCapture());
        
        QElapsedTimer timer;
        timer.start();
        
        int frameCount = 0;
        QList<qint64> frameIntervals;
        qint64 lastFrameTime = 0;
        
        // 捕获2秒钟的帧
        while (timer.elapsed() < 2000) {
            QPixmap frame = capture->captureFrame();
            QVERIFY(!frame.isNull());
            
            qint64 currentTime = timer.elapsed();
            if (lastFrameTime > 0) {
                frameIntervals.append(currentTime - lastFrameTime);
            }
            lastFrameTime = currentTime;
            frameCount++;
            
            QTest::qWait(1000 / targetFPS); // 目标帧间隔
        }
        
        qint64 totalTime = timer.elapsed();
        double actualFPS = (frameCount * 1000.0) / totalTime;
        
        // 计算帧间隔稳定性
        if (!frameIntervals.isEmpty()) {
            double avgInterval = std::accumulate(frameIntervals.begin(), frameIntervals.end(), 0.0) / frameIntervals.size();
            double expectedInterval = 1000.0 / targetFPS;
            double intervalVariation = std::abs(avgInterval - expectedInterval);
            
            qDebug() << "Frame Rate Test for" << targetFPS << "FPS:";
            qDebug() << "  Actual FPS:" << actualFPS;
            qDebug() << "  Expected interval:" << expectedInterval << "ms";
            qDebug() << "  Actual interval:" << avgInterval << "ms";
            qDebug() << "  Variation:" << intervalVariation << "ms";
            
            // 帧率应该接近目标值 (允许10%误差)
            QVERIFY(std::abs(actualFPS - targetFPS) < targetFPS * 0.1);
            
            // 帧间隔变化应该较小 (允许20%误差)
            QVERIFY(intervalVariation < expectedInterval * 0.2);
        }
        
        capture->stopCapture();
    }
    
    delete capture;
}

void ScreenShareModuleTest::testLatencyMeasurement()
{
    qDebug() << "Testing latency measurement";
    
    ScreenCapture* capture = new ScreenCapture(this);
    VideoEncoder* encoder = new VideoEncoder(this);
    
    QVERIFY(capture->initialize());
    QVERIFY(encoder->initialize());
    QVERIFY(capture->startCapture());
    QVERIFY(encoder->start());
    
    QList<qint64> latencies;
    
    for (int i = 0; i < 10; ++i) {
        QElapsedTimer latencyTimer;
        latencyTimer.start();
        
        // 完整的捕获-编码流程
        QPixmap frame = capture->captureFrame();
        QVERIFY(!frame.isNull());
        
        QByteArray encodedData = encoder->encodeFrame(frame);
        QVERIFY(!encodedData.isEmpty());
        
        qint64 latency = latencyTimer.elapsed();
        latencies.append(latency);
        
        QTest::qWait(100);
    }
    
    // 计算延迟统计
    double avgLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    qint64 maxLatency = *std::max_element(latencies.begin(), latencies.end());
    qint64 minLatency = *std::min_element(latencies.begin(), latencies.end());
    
    qDebug() << "Latency Measurements:";
    qDebug() << "  Average:" << avgLatency << "ms";
    qDebug() << "  Maximum:" << maxLatency << "ms";
    qDebug() << "  Minimum:" << minLatency << "ms";
    
    // 延迟应该在可接受范围内
    QVERIFY(avgLatency < 200.0); // 平均延迟应小于200ms
    QVERIFY(maxLatency < 500); // 最大延迟应小于500ms
    
    capture->stopCapture();
    encoder->stop();
    
    delete capture;
    delete encoder;
}

// Task 43: 屏幕共享UI组件测试
void ScreenShareModuleTest::testScreenShareWidget()
{
    qDebug() << "Testing ScreenShareWidget";
    
    ScreenShareWidget* widget = new ScreenShareWidget();
    QVERIFY(widget != nullptr);
    
    // 测试初始状态
    QVERIFY(!widget->isSharing());
    QCOMPARE(widget->shareStatus(), ScreenShareWidget::Stopped);
    
    // 测试UI控件存在性
    QVERIFY(widget->findChild<QPushButton*>("startButton") != nullptr);
    QVERIFY(widget->findChild<QPushButton*>("stopButton") != nullptr);
    QVERIFY(widget->findChild<QComboBox*>("sourceCombo") != nullptr);
    QVERIFY(widget->findChild<QSlider*>("qualitySlider") != nullptr);
    
    // 测试配置设置
    widget->setQuality(IScreenCapture::HighQuality);
    QCOMPARE(widget->quality(), IScreenCapture::HighQuality);
    
    widget->setFrameRate(30);
    QCOMPARE(widget->frameRate(), 30);
    
    // 测试信号连接
    QSignalSpy startSpy(widget, &ScreenShareWidget::shareStartRequested);
    QSignalSpy stopSpy(widget, &ScreenShareWidget::shareStopRequested);
    QSignalSpy configSpy(widget, &ScreenShareWidget::configurationChanged);
    
    // 模拟用户交互
    widget->startShare();
    QCOMPARE(startSpy.count(), 1);
    
    widget->stopShare();
    QCOMPARE(stopSpy.count(), 1);
    
    widget->setQuality(IScreenCapture::MediumQuality);
    QCOMPARE(configSpy.count(), 1);
    
    delete widget;
}

void ScreenShareModuleTest::testScreenSelector()
{
    qDebug() << "Testing ScreenSelector";
    
    ScreenSelector* selector = new ScreenSelector();
    QVERIFY(selector != nullptr);
    
    // 测试初始化
    selector->refreshSources();
    
    // 测试屏幕列表
    QStringList screens = selector->availableScreens();
    QVERIFY(!screens.isEmpty()); // 至少应该有一个屏幕
    
    // 测试窗口列表
    QStringList windows = selector->availableWindows();
    QVERIFY(!windows.isEmpty()); // 至少应该有一些窗口
    
    // 测试选择功能
    if (!screens.isEmpty()) {
        selector->selectScreen(0);
        QCOMPARE(selector->selectedSourceType(), ScreenSelector::Screen);
        QCOMPARE(selector->selectedSourceIndex(), 0);
    }
    
    if (!windows.isEmpty()) {
        selector->selectWindow(0);
        QCOMPARE(selector->selectedSourceType(), ScreenSelector::Window);
        QCOMPARE(selector->selectedSourceIndex(), 0);
    }
    
    // 测试区域选择
    QRect customRegion(100, 100, 800, 600);
    selector->selectCustomRegion(customRegion);
    QCOMPARE(selector->selectedSourceType(), ScreenSelector::Region);
    QCOMPARE(selector->customRegion(), customRegion);
    
    // 测试信号
    QSignalSpy selectionSpy(selector, &ScreenSelector::sourceSelected);
    selector->selectScreen(0);
    QCOMPARE(selectionSpy.count(), 1);
    
    delete selector;
}

void ScreenShareModuleTest::testCapturePreview()
{
    qDebug() << "Testing CapturePreview";
    
    CapturePreview* preview = new CapturePreview();
    QVERIFY(preview != nullptr);
    
    // 测试初始状态
    QVERIFY(!preview->isPreviewActive());
    QVERIFY(preview->currentFrame().isNull());
    
    // 测试预览控制
    preview->startPreview();
    QVERIFY(preview->isPreviewActive());
    
    // 测试帧更新
    QPixmap testFrame(640, 480);
    testFrame.fill(Qt::blue);
    
    preview->updateFrame(testFrame);
    QVERIFY(!preview->currentFrame().isNull());
    QCOMPARE(preview->currentFrame().size(), testFrame.size());
    
    // 测试缩放模式
    preview->setScaleMode(CapturePreview::KeepAspectRatio);
    QCOMPARE(preview->scaleMode(), CapturePreview::KeepAspectRatio);
    
    preview->setScaleMode(CapturePreview::StretchToFit);
    QCOMPARE(preview->scaleMode(), CapturePreview::StretchToFit);
    
    // 测试预览设置
    preview->setShowFPS(true);
    QVERIFY(preview->isShowFPSEnabled());
    
    preview->setShowResolution(true);
    QVERIFY(preview->isShowResolutionEnabled());
    
    // 测试停止预览
    preview->stopPreview();
    QVERIFY(!preview->isPreviewActive());
    
    delete preview;
}

void ScreenShareModuleTest::testUIComponentInteractions()
{
    qDebug() << "Testing UI component interactions";
    
    // 创建UI组件
    ScreenShareWidget* mainWidget = new ScreenShareWidget();
    ScreenSelector* selector = new ScreenSelector();
    CapturePreview* preview = new CapturePreview();
    
    // 测试组件间的交互
    QSignalSpy sourceSpy(selector, &ScreenSelector::sourceSelected);
    QSignalSpy shareSpy(mainWidget, &ScreenShareWidget::shareStartRequested);
    
    // 模拟用户工作流程
    selector->refreshSources();
    QStringList screens = selector->availableScreens();
    if (!screens.isEmpty()) {
        selector->selectScreen(0);
        QCOMPARE(sourceSpy.count(), 1);
        
        // 启动预览
        preview->startPreview();
        QVERIFY(preview->isPreviewActive());
        
        // 启动共享
        mainWidget->startShare();
        QCOMPARE(shareSpy.count(), 1);
    }
    
    delete mainWidget;
    delete selector;
    delete preview;
}

void ScreenShareModuleTest::testUIResponsiveness()
{
    qDebug() << "Testing UI responsiveness";
    
    ScreenShareWidget* widget = new ScreenShareWidget();
    
    // 测试UI响应时间
    QElapsedTimer timer;
    
    // 测试按钮点击响应
    timer.start();
    widget->startShare();
    qint64 buttonResponseTime = timer.elapsed();
    QVERIFY(buttonResponseTime < 100); // 按钮响应应小于100ms
    
    // 测试配置更改响应
    timer.restart();
    widget->setQuality(IScreenCapture::HighQuality);
    qint64 configResponseTime = timer.elapsed();
    QVERIFY(configResponseTime < 50); // 配置更改应小于50ms
    
    // 测试UI更新频率
    QSignalSpy updateSpy(widget, &ScreenShareWidget::statusUpdated);
    
    // 模拟状态更新
    for (int i = 0; i < 10; ++i) {
        widget->updateStatus(QString("Status %1").arg(i));
        QTest::qWait(10);
    }
    
    QCOMPARE(updateSpy.count(), 10);
    
    delete widget;
}

void ScreenShareModuleTest::testUIStateManagement()
{
    qDebug() << "Testing UI state management";
    
    ScreenShareWidget* widget = new ScreenShareWidget();
    
    // 测试初始状态
    QCOMPARE(widget->shareStatus(), ScreenShareWidget::Stopped);
    QVERIFY(!widget->isSharing());
    
    // 测试状态转换
    widget->setShareStatus(ScreenShareWidget::Starting);
    QCOMPARE(widget->shareStatus(), ScreenShareWidget::Starting);
    
    widget->setShareStatus(ScreenShareWidget::Active);
    QCOMPARE(widget->shareStatus(), ScreenShareWidget::Active);
    QVERIFY(widget->isSharing());
    
    widget->setShareStatus(ScreenShareWidget::Paused);
    QCOMPARE(widget->shareStatus(), ScreenShareWidget::Paused);
    
    widget->setShareStatus(ScreenShareWidget::Stopping);
    QCOMPARE(widget->shareStatus(), ScreenShareWidget::Stopping);
    
    widget->setShareStatus(ScreenShareWidget::Stopped);
    QCOMPARE(widget->shareStatus(), ScreenShareWidget::Stopped);
    QVERIFY(!widget->isSharing());
    
    // 测试状态持久化
    QVariantMap state = widget->saveState();
    QVERIFY(!state.isEmpty());
    
    widget->setQuality(IScreenCapture::HighQuality);
    widget->setFrameRate(60);
    
    widget->restoreState(state);
    // 状态应该被恢复到保存时的值
    
    delete widget;
}

// Task 43: 与现有ScreenShareManager的兼容性测试
void ScreenShareModuleTest::testLegacyCompatibility()
{
    qDebug() << "Testing legacy compatibility";
    
    // 测试与旧版本ScreenShareManager的兼容性
    m_manager = new ScreenShareManager(this);
    QVERIFY(m_manager->initialize());
    
    // 测试旧API是否仍然可用
    QVERIFY(m_manager->metaObject()->indexOfMethod("startScreenShare()") >= 0);
    QVERIFY(m_manager->metaObject()->indexOfMethod("stopScreenShare()") >= 0);
    QVERIFY(m_manager->metaObject()->indexOfMethod("setQuality(int)") >= 0);
    
    // 测试旧配置格式
    QVariantMap legacyConfig;
    legacyConfig["screenId"] = 0;
    legacyConfig["quality"] = 2; // 旧的质量枚举值
    legacyConfig["fps"] = 30;
    
    // 应该能够处理旧配置格式
    m_manager->setConfiguration(legacyConfig);
    
    // 验证配置被正确转换
    QVariantMap currentConfig = m_manager->configuration();
    QVERIFY(currentConfig.contains("quality"));
    QVERIFY(currentConfig.contains("frameRate"));
}

void ScreenShareModuleTest::testAPICompatibility()
{
    qDebug() << "Testing API compatibility";
    
    m_manager = new ScreenShareManager(this);
    QVERIFY(m_manager->initialize());
    
    // 测试所有公共API方法是否存在
    const QMetaObject* metaObj = m_manager->metaObject();
    
    // 核心API方法
    QVERIFY(metaObj->indexOfMethod("initialize()") >= 0);
    QVERIFY(metaObj->indexOfMethod("startScreenShare()") >= 0);
    QVERIFY(metaObj->indexOfMethod("stopScreenShare()") >= 0);
    QVERIFY(metaObj->indexOfMethod("pauseScreenShare()") >= 0);
    QVERIFY(metaObj->indexOfMethod("resumeScreenShare()") >= 0);
    
    // 配置API方法
    QVERIFY(metaObj->indexOfMethod("setQuality(IScreenCapture::CaptureQuality)") >= 0);
    QVERIFY(metaObj->indexOfMethod("setFrameRate(int)") >= 0);
    QVERIFY(metaObj->indexOfMethod("setBitrate(int)") >= 0);
    
    // 查询API方法
    QVERIFY(metaObj->indexOfMethod("availableScreens()") >= 0);
    QVERIFY(metaObj->indexOfMethod("availableWindows()") >= 0);
    QVERIFY(metaObj->indexOfMethod("isSharing()") >= 0);
    QVERIFY(metaObj->indexOfMethod("status()") >= 0);
    
    // 测试信号是否存在
    QVERIFY(metaObj->indexOfSignal("shareStarted()") >= 0);
    QVERIFY(metaObj->indexOfSignal("shareStopped()") >= 0);
    QVERIFY(metaObj->indexOfSignal("shareError(QString)") >= 0);
    QVERIFY(metaObj->indexOfSignal("statusChanged(IScreenShareManager::Status)") >= 0);
}

void ScreenShareModuleTest::testConfigurationMigration()
{
    qDebug() << "Testing configuration migration";
    
    m_manager = new ScreenShareManager(this);
    QVERIFY(m_manager->initialize());
    
    // 测试从旧配置格式迁移
    QVariantMap oldConfig;
    oldConfig["version"] = "1.0";
    oldConfig["screen_id"] = 0; // 旧的下划线命名
    oldConfig["quality_level"] = 2; // 旧的质量级别
    oldConfig["frame_rate"] = 25; // 旧的帧率设置
    oldConfig["enable_audio"] = true; // 旧的音频设置
    
    // 应该能够迁移旧配置
    bool migrationResult = m_manager->migrateConfiguration(oldConfig);
    QVERIFY(migrationResult);
    
    // 验证迁移后的配置
    QVariantMap newConfig = m_manager->configuration();
    QVERIFY(newConfig.contains("screenId")); // 新的驼峰命名
    QVERIFY(newConfig.contains("quality"));
    QVERIFY(newConfig.contains("frameRate"));
    
    // 验证值被正确转换
    QCOMPARE(newConfig["screenId"].toInt(), 0);
    QCOMPARE(newConfig["frameRate"].toInt(), 25);
}

void ScreenShareModuleTest::testFeatureParity()
{
    qDebug() << "Testing feature parity";
    
    m_manager = new ScreenShareManager(this);
    QVERIFY(m_manager->initialize());
    
    // 验证所有旧功能在新实现中都可用
    
    // 1. 屏幕选择功能
    QStringList screens = m_manager->availableScreens();
    QVERIFY(!screens.isEmpty());
    
    if (!screens.isEmpty()) {
        QVERIFY(m_manager->selectScreen(screens.first()));
    }
    
    // 2. 窗口选择功能
    QStringList windows = m_manager->availableWindows();
    QVERIFY(!windows.isEmpty());
    
    if (!windows.isEmpty()) {
        QVERIFY(m_manager->selectWindow(windows.first()));
    }
    
    // 3. 质量控制功能
    m_manager->setQuality(IScreenCapture::HighQuality);
    QCOMPARE(m_manager->quality(), IScreenCapture::HighQuality);
    
    // 4. 帧率控制功能
    m_manager->setFrameRate(30);
    QCOMPARE(m_manager->frameRate(), 30);
    
    // 5. 比特率控制功能
    m_manager->setBitrate(5000);
    QCOMPARE(m_manager->bitrate(), 5000);
    
    // 6. 统计信息功能
    QVariantMap stats = m_manager->getStatistics();
    QVERIFY(stats.contains("totalFrames"));
    QVERIFY(stats.contains("currentFPS"));
    
    // 7. 错误处理功能
    QStringList errors = m_manager->getLastErrors();
    // 初始状态应该没有错误
}

void ScreenShareModuleTest::testBackwardCompatibility()
{
    qDebug() << "Testing backward compatibility";
    
    // 测试旧代码是否能够无修改地使用新实现
    
    // 模拟旧代码的使用方式
    ScreenShareManager* legacyManager = new ScreenShareManager(this);
    
    // 旧代码通常的初始化方式
    bool initResult = legacyManager->initialize();
    QVERIFY(initResult);
    
    // 旧代码通常的配置方式
    legacyManager->setQuality(IScreenCapture::MediumQuality);
    legacyManager->setFrameRate(25);
    
    // 旧代码通常的启动方式
    bool startResult = legacyManager->startScreenShare();
    QVERIFY(startResult);
    
    // 验证状态
    QVERIFY(legacyManager->isSharing());
    QCOMPARE(legacyManager->status(), IScreenShareManager::Active);
    
    // 旧代码通常的停止方式
    legacyManager->stopScreenShare();
    QVERIFY(!legacyManager->isSharing());
    
    delete legacyManager;
}

void ScreenShareModuleTest::testIntegrationWithExistingCode()
{
    qDebug() << "Testing integration with existing code";
    
    // 测试与现有代码库的集成
    
    // 1. 测试与现有MediaManager的集成
    // (假设存在MediaManager类)
    
    // 2. 测试与现有ConferenceManager的集成
    // (假设存在ConferenceManager类)
    
    // 3. 测试与现有UI框架的集成
    m_manager = new ScreenShareManager(this);
    QVERIFY(m_manager->initialize());
    
    // 测试信号槽连接
    QSignalSpy startedSpy(m_manager, &IScreenShareManager::shareStarted);
    QSignalSpy stoppedSpy(m_manager, &IScreenShareManager::shareStopped);
    QSignalSpy errorSpy(m_manager, &IScreenShareManager::shareError);
    
    // 模拟正常工作流程
    QVERIFY(m_manager->startScreenShare());
    QCOMPARE(startedSpy.count(), 1);
    
    m_manager->stopScreenShare();
    QCOMPARE(stoppedSpy.count(), 1);
    
    // 4. 测试配置系统集成
    QVariantMap globalConfig;
    globalConfig["screenshare.enabled"] = true;
    globalConfig["screenshare.defaultQuality"] = static_cast<int>(IScreenCapture::HighQuality);
    globalConfig["screenshare.defaultFrameRate"] = 30;
    
    // 应该能够从全局配置加载设置
    m_manager->loadFromGlobalConfig(globalConfig);
    QCOMPARE(m_manager->quality(), IScreenCapture::HighQuality);
    QCOMPARE(m_manager->frameRate(), 30);
    
    // 5. 测试日志系统集成
    // 验证日志消息被正确记录
    QStringList logMessages = m_manager->getLogMessages();
    QVERIFY(!logMessages.isEmpty());
}

QTEST_MAIN(ScreenShareModuleTest)