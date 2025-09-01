# 摄像头模块 (Camera Module)

## 概述

摄像头模块提供了完整的摄像头设备管理和视频流处理功能，采用现代化的模块化架构设计。本模块完全重构，包含接口定义、配置管理、工具类、UI组件、测试框架等完整的企业级功能。

## 🏗️ 架构设计

### 模块化分层架构

```
┌─────────────────────────────────────────────────────────────┐
│                    摄像头模块 v1.2.0                        │
├─────────────────────────────────────────────────────────────┤
│  📱 UI层        │ CameraPreviewWidget, 样式, 资源          │
├─────────────────────────────────────────────────────────────┤
│  🔧 工具层      │ CameraUtils, 配置管理, 性能监控          │
├─────────────────────────────────────────────────────────────┤
│  🎯 接口层      │ ICameraDevice, ICameraManager            │
├─────────────────────────────────────────────────────────────┤
│  🏭 工厂层      │ CameraFactory - 实例创建和管理           │
├─────────────────────────────────────────────────────────────┤
│  📊 管理层      │ CameraManager - 高级功能和配置           │
├─────────────────────────────────────────────────────────────┤
│  🔌 设备层      │ CameraModule - 底层硬件控制              │
└─────────────────────────────────────────────────────────────┘
```

### 📁 完整目录结构

```
modules/camera/
├── 📄 camera.pri              # 模块项目文件
├── 📄 README.md               # 模块文档
├── 📄 CAMERA_MODULE_USAGE.md  # 使用指南
├── 📂 include/                # 核心头文件
│   ├── CameraModule.h         # 底层设备控制
│   ├── CameraManager.h        # 高级管理
│   └── CameraFactory.h        # 工厂模式
├── 📂 src/                    # 核心实现
│   ├── CameraModule.cpp
│   ├── CameraManager.cpp
│   └── CameraFactory.cpp
├── 📂 interfaces/             # 🆕 接口定义
│   ├── ICameraDevice.h        # 设备接口
│   └── ICameraManager.h       # 管理器接口
├── 📂 config/                 # 🆕 配置管理
│   ├── CameraConfig.h         # 配置类
│   └── CameraConfig.cpp
├── 📂 utils/                  # 🆕 工具类
│   ├── CameraUtils.h          # 实用工具
│   └── CameraUtils.cpp
├── 📂 widgets/                # 🆕 UI组件
│   └── CameraPreviewWidget.h  # 预览组件
├── 📂 tests/                  # 🆕 测试框架
│   ├── test_camera_module.h   # 单元测试
│   └── test_camera_module.cpp
├── 📂 examples/               # 🆕 示例代码
│   └── basic_camera_example.cpp
└── 📂 resources/              # 🆕 资源文件
    ├── camera_icons.qrc       # 图标资源
    └── camera_styles.qss      # 样式表
```

## 🎯 核心接口

### ICameraDevice (设备接口)

定义了摄像头设备的标准接口：

```cpp
class ICameraDevice : public QObject {
    Q_OBJECT
public:
    enum Status { Inactive, Loading, Loaded, Starting, Active, Stopping, Stopped, Error };
    enum QualityPreset { LowQuality, StandardQuality, HighQuality, UltraQuality };
    
    // 基本控制
    virtual bool initialize() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isActive() const = 0;
    
    // 设备信息
    virtual QString deviceId() const = 0;
    virtual QString deviceName() const = 0;
    virtual bool isAvailable() const = 0;
    
    // 配置接口
    virtual void setResolution(const QSize& resolution) = 0;
    virtual void setFrameRate(int frameRate) = 0;
    virtual void setQualityPreset(QualityPreset preset) = 0;
    
signals:
    void statusChanged(Status status);
    void errorOccurred(const QString& error);
    void frameAvailable(const QVideoFrame& frame);
};
```

### ICameraManager (管理器接口)

定义了摄像头管理的高级接口：

```cpp
class ICameraManager : public QObject {
    Q_OBJECT
public:
    enum ManagerStatus { Uninitialized, Initializing, Ready, Busy, Error };
    
    // 管理器控制
    virtual bool initialize() = 0;
    virtual ManagerStatus status() const = 0;
    
    // 设备管理
    virtual QStringList availableDevices() const = 0;
    virtual bool selectDevice(const QString& deviceId) = 0;
    
    // 预览控制
    virtual QVideoWidget* createPreviewWidget(QWidget* parent = nullptr) = 0;
    virtual bool startWithPreset(ICameraDevice::QualityPreset preset) = 0;
    
    // 统计信息
    virtual int frameCount() const = 0;
    virtual double averageFrameRate() const = 0;
    
signals:
    void statusChanged(ManagerStatus status);
    void devicesUpdated(const QStringList& devices);
    void cameraStarted();
    void cameraStopped();
};
```

## ⚙️ 配置管理

### CameraConfig (配置类)

提供完整的配置管理功能：

```cpp
class CameraConfig : public QObject {
    Q_OBJECT
public:
    // 单例访问
    static CameraConfig* instance();
    
    // 基本配置
    void setPreferredDevice(const QString& deviceId);
    QString preferredDevice() const;
    
    void setDefaultResolution(const QSize& resolution);
    QSize defaultResolution() const;
    
    void setDefaultQualityPreset(ICameraDevice::QualityPreset preset);
    ICameraDevice::QualityPreset defaultQualityPreset() const;
    
    // 行为配置
    void setAutoStartCamera(bool autoStart);
    bool autoStartCamera() const;
    
    void setEnableHardwareAcceleration(bool enable);
    bool enableHardwareAcceleration() const;
    
    // 性能配置
    void setEnablePerformanceMonitoring(bool enable);
    bool enablePerformanceMonitoring() const;
    
    // 配置持久化
    void loadFromSettings();
    void saveToSettings();
    void resetToDefaults();
    
    // 配置验证
    bool isValid() const;
    QStringList validate() const;
    
signals:
    void configChanged();
    void preferredDeviceChanged(const QString& deviceId);
};
```

## 🛠️ 工具类

### CameraUtils (实用工具)

提供丰富的工具函数：

```cpp
class CameraUtils {
public:
    // 预设映射
    static QSize resolutionForPreset(ICameraDevice::QualityPreset preset);
    static int frameRateForPreset(ICameraDevice::QualityPreset preset);
    static QString presetName(ICameraDevice::QualityPreset preset);
    
    // 验证函数
    static bool isValidResolution(const QSize& resolution);
    static bool isValidFrameRate(int frameRate);
    static bool isValidDeviceId(const QString& deviceId);
    
    // 格式化函数
    static QString formatResolution(const QSize& resolution);
    static QSize parseResolution(const QString& resolutionStr);
    static QString statusName(ICameraDevice::Status status);
    
    // 计算函数
    static int calculateBitrate(const QSize& resolution, int frameRate, ICameraDevice::QualityPreset preset);
    static qint64 calculateFrameSize(const QSize& resolution, const QString& format = "RGB32");
    static double aspectRatio(const QSize& resolution);
    
    // 推荐值
    static QList<QSize> recommendedResolutions();
    static QList<int> recommendedFrameRates();
    
    // 查找函数
    static QSize findClosestResolution(const QSize& target, const QList<QSize>& supported);
    static int findClosestFrameRate(int target, const QList<int>& supported);
    
    // 性能评估
    static QString performanceLevel(const QSize& resolution, int frameRate);
    static qint64 estimateMemoryUsage(const QSize& resolution, int frameRate, int bufferCount = 3);
};
```

## 🎨 UI组件

### CameraPreviewWidget (预览组件)

完整的摄像头预览界面：

```cpp
class CameraPreviewWidget : public QWidget {
    Q_OBJECT
public:
    enum DisplayMode { VideoOnly, VideoWithControls, FullInterface };
    
    explicit CameraPreviewWidget(QWidget* parent = nullptr);
    
    // 摄像头管理器设置
    void setCameraManager(ICameraManager* manager);
    ICameraManager* cameraManager() const;
    
    // 显示模式
    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;
    
    // 控制接口
    void setControlsVisible(bool visible);
    void setStatusVisible(bool visible);
    void setPreviewSize(const QSize& size);
    
    // 状态查询
    bool isCameraActive() const;
    QString currentDeviceName() const;
    QSize currentResolution() const;
    
public slots:
    void startPreview();
    void stopPreview();
    void toggleCamera();
    void refreshDevices();
    void applyQualityPreset(ICameraDevice::QualityPreset preset);
    void takeSnapshot();
    
signals:
    void cameraStatusChanged(bool active);
    void deviceChanged(const QString& deviceId);
    void qualityChanged(ICameraDevice::QualityPreset preset);
    void snapshotTaken(const QPixmap& snapshot);
    void errorOccurred(const QString& error);
};
```

## 🧪 测试框架

### 完整的单元测试

```cpp
class TestCameraModule : public QObject {
    Q_OBJECT
private slots:
    // CameraModule 测试
    void testCameraModuleInitialization();
    void testCameraModuleDeviceScanning();
    void testCameraModuleStartStop();
    void testCameraModuleConfiguration();
    void testCameraModuleErrorHandling();
    
    // CameraManager 测试
    void testCameraManagerInitialization();
    void testCameraManagerDeviceManagement();
    void testCameraManagerPreviewControl();
    void testCameraManagerQualityPresets();
    
    // CameraFactory 测试
    void testCameraFactorySingleton();
    void testCameraFactoryCreation();
    
    // CameraConfig 测试
    void testCameraConfigDefaults();
    void testCameraConfigPersistence();
    void testCameraConfigValidation();
    
    // CameraUtils 测试
    void testCameraUtilsResolutionMapping();
    void testCameraUtilsValidation();
    void testCameraUtilsFormatting();
    
    // 集成测试
    void testFullWorkflow();
    void testErrorRecovery();
    void testPerformanceMetrics();
};
```

## 📚 使用示例

### 基本使用

```cpp
#include "modules/camera/include/CameraFactory.h"
#include "modules/camera/widgets/CameraPreviewWidget.h"

// 1. 创建摄像头管理器
CameraManager* camera = CameraFactory::instance()->createLocalCamera();
camera->initialize();

// 2. 创建预览组件
CameraPreviewWidget* preview = new CameraPreviewWidget(this);
preview->setCameraManager(camera);
preview->setDisplayMode(CameraPreviewWidget::FullInterface);

// 3. 启动摄像头
preview->startPreview();
```

### 高级配置示例

```cpp
// 使用配置管理
CameraConfig* config = CameraConfig::instance();
config->setPreferredDevice("camera_device_id");
config->setDefaultResolution(QSize(1280, 720));
config->setDefaultQualityPreset(ICameraDevice::HighQuality);
config->setEnableHardwareAcceleration(true);
config->saveToSettings();

// 应用配置
QVariantMap configMap = config->toVariantMap();
camera->applyConfiguration(configMap);
```

## 🎯 质量预设

| 预设 | 分辨率 | 帧率 | 比特率 | 用途 |
|------|--------|------|--------|------|
| **LowQuality** | 320×240 | 15fps | ~300kbps | 低带宽环境 |
| **StandardQuality** | 640×480 | 30fps | ~1Mbps | 标准视频通话 |
| **HighQuality** | 1280×720 | 30fps | ~2.5Mbps | 高清视频会议 |
| **UltraQuality** | 1920×1080 | 30fps | ~5Mbps | 专业录制 |

## 🔧 编译选项

### 基本编译

```qmake
# 包含摄像头模块
include(modules/modules.pri)
```

### 启用测试

```qmake
CONFIG += tests
include(modules/modules.pri)
```

### 启用示例

```qmake
CONFIG += examples
include(modules/modules.pri)
```

## 🚀 最佳实践

### 1. 错误处理

```cpp
connect(camera, &ICameraManager::errorOccurred, [=](const QString& error) {
    qWarning() << "Camera error:" << error;
    
    // 自动恢复策略
    if (error.contains("Device in use")) {
        // 尝试其他设备
        QStringList devices = camera->availableDevices();
        for (const QString& device : devices) {
            if (camera->selectDevice(device) && camera->startCamera()) {
                break;
            }
        }
    }
});
```

### 2. 资源管理

```cpp
// 正确的生命周期管理
class CameraController {
public:
    CameraController() {
        m_camera = CameraFactory::instance()->createLocalCamera();
        m_camera->initialize();
    }
    
    ~CameraController() {
        if (m_camera) {
            m_camera->stopCamera();
            m_camera->cleanup();
            CameraFactory::instance()->destroyCamera(m_camera);
        }
    }
    
private:
    CameraManager* m_camera;
};
```

## 📈 版本历史

- **v1.0.0** (2025-08-26) - 初始版本，基本功能
- **v1.1.0** (2025-08-27) - 添加质量预设和配置管理  
- **v1.2.0** (2025-08-28) - 🆕 **完整模块化重构**
  - ✅ 标准化接口定义 (ICameraDevice, ICameraManager)
  - ✅ 完整配置管理系统 (CameraConfig)
  - ✅ 丰富的工具类 (CameraUtils)
  - ✅ UI预览组件 (CameraPreviewWidget)
  - ✅ 完整测试框架 (单元测试、性能测试、压力测试)
  - ✅ 示例代码和文档
  - ✅ 资源文件和样式表
  - ✅ 企业级错误处理和恢复机制

## 📄 许可证

本模块遵循项目主许可证。

---

**摄像头模块，提供企业级的功能和可靠性！** 🎉