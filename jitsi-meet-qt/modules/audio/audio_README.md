# Audio Module

## 概述

音频模块是Jitsi Meet Qt项目的核心组件之一，负责音频设备管理、音频流处理和音频配置。该模块遵循模块化架构设计，提供统一的接口和可扩展的实现。

## 版本信息

- **版本**: 1.0.0
- **状态**: 开发中
- **兼容性**: Qt 5.15+, WebRTC

## 功能特性

### 核心功能
- 音频设备枚举和管理
- 麦克风和扬声器独立控制
- 音频质量预设管理
- 实时音频处理
- 音频配置持久化

### 高级功能
- WebRTC集成
- 跨平台音频设备支持
- 音频延迟优化
- 噪声抑制和回声消除
- 音频质量自适应

## 架构设计

### 分层架构
```
┌─────────────────────────────────────────────────────────────┐
│                    Audio Module v1.0.0                     │
├─────────────────────────────────────────────────────────────┤
│  📱 UI层        │ AudioControlWidget, VolumeSliderWidget   │
├─────────────────────────────────────────────────────────────┤
│  🔧 工具层      │ AudioUtils, AudioConfig                  │
├─────────────────────────────────────────────────────────────┤
│  🎯 接口层      │ IAudioDevice, IAudioManager             │
├─────────────────────────────────────────────────────────────┤
│  🏭 工厂层      │ AudioFactory - 设备创建和管理            │
├─────────────────────────────────────────────────────────────┤
│  📊 管理层      │ AudioManager - 高级功能和配置            │
├─────────────────────────────────────────────────────────────┤
│  🔌 核心层      │ AudioModule - 底层音频控制               │
└─────────────────────────────────────────────────────────────┘
```

### 目录结构
```
audio/
├── audio.pri                    # 模块配置文件
├── README.md                    # 本文档
├── include/                     # 核心头文件
│   ├── AudioModule.h           # 底层音频控制
│   ├── AudioManager.h          # 高级音频管理
│   └── AudioFactory.h          # 音频工厂
├── src/                        # 核心实现
│   ├── AudioModule.cpp
│   ├── AudioManager.cpp
│   ├── AudioFactory.cpp
│   └── platform/               # 平台特定实现
│       ├── AudioDevice_Windows.h/cpp
│       ├── AudioDevice_Linux.h/cpp
│       └── AudioDevice_macOS.h/cpp
├── interfaces/                 # 接口定义
│   ├── IAudioDevice.h          # 音频设备接口
│   ├── IAudioManager.h         # 音频管理器接口
│   └── IAudioProcessor.h       # 音频处理接口
├── config/                     # 配置管理
│   ├── AudioConfig.h           # 音频配置类
│   └── AudioConfig.cpp
├── utils/                      # 工具类
│   ├── AudioUtils.h            # 音频工具
│   └── AudioUtils.cpp
├── widgets/                    # UI组件
│   ├── AudioControlWidget.h    # 音频控制组件
│   ├── AudioControlWidget.cpp
│   ├── VolumeSliderWidget.h    # 音量滑块组件
│   └── VolumeSliderWidget.cpp
├── tests/                      # 测试框架
├── examples/                   # 示例代码
└── resources/                  # 资源文件
    └── audio.qrc               # Qt资源文件
```

## 接口设计

### IAudioDevice 接口
```cpp
class IAudioDevice : public QObject {
    Q_OBJECT
public:
    enum DeviceType { Input, Output };
    enum Status { Inactive, Active, Error };
    enum QualityPreset { LowQuality, StandardQuality, HighQuality };
    
    virtual bool initialize() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isActive() const = 0;
    
    virtual QString deviceId() const = 0;
    virtual QString deviceName() const = 0;
    virtual DeviceType deviceType() const = 0;
    
    virtual void setVolume(qreal volume) = 0;
    virtual qreal volume() const = 0;
    virtual void setMuted(bool muted) = 0;
    virtual bool isMuted() const = 0;
    
signals:
    void statusChanged(Status status);
    void volumeChanged(qreal volume);
    void muteChanged(bool muted);
    void errorOccurred(const QString& error);
};
```

### IAudioManager 接口
```cpp
class IAudioManager : public QObject {
    Q_OBJECT
public:
    enum ManagerStatus { Uninitialized, Ready, Busy, Error };
    
    virtual bool initialize() = 0;
    virtual ManagerStatus status() const = 0;
    
    virtual QStringList availableInputDevices() const = 0;
    virtual QStringList availableOutputDevices() const = 0;
    virtual bool selectInputDevice(const QString& deviceId) = 0;
    virtual bool selectOutputDevice(const QString& deviceId) = 0;
    
    virtual void setMasterVolume(qreal volume) = 0;
    virtual qreal masterVolume() const = 0;
    virtual void setMicrophoneGain(qreal gain) = 0;
    virtual qreal microphoneGain() const = 0;
    
signals:
    void statusChanged(ManagerStatus status);
    void devicesUpdated();
    void audioStarted();
    void audioStopped();
};
```

## 配置选项

### 编译时配置
- `AUDIO_MODULE_AVAILABLE`: 启用音频模块
- `AUDIO_WEBRTC_ENABLED`: 启用WebRTC集成
- `AUDIO_DEFAULT_SAMPLE_RATE`: 默认采样率 (48000Hz)
- `AUDIO_DEFAULT_CHANNELS`: 默认声道数 (2)
- `AUDIO_DEFAULT_BUFFER_SIZE`: 默认缓冲区大小 (1024)

### 运行时配置
通过AudioConfig类管理运行时配置：
- 音频设备偏好设置
- 音频质量预设
- 音量和增益设置
- 噪声抑制和回声消除参数

## 平台支持

### Windows
- 使用Windows Audio Session API (WASAPI)
- 支持DirectSound和MME
- 依赖库: winmm, ole32, oleaut32

### Linux
- 使用PulseAudio和ALSA
- 支持JACK音频连接套件
- 依赖库: pulse, asound

### macOS
- 使用Core Audio框架
- 支持Audio Unit
- 依赖框架: CoreAudio, AudioUnit

## 使用示例

### 基本音频管理
```cpp
#include "AudioManager.h"
#include "AudioConfig.h"

// 创建音频管理器
AudioManager* audioManager = new AudioManager(this);

// 初始化音频系统
if (audioManager->initialize()) {
    // 获取可用设备
    QStringList inputDevices = audioManager->availableInputDevices();
    QStringList outputDevices = audioManager->availableOutputDevices();
    
    // 选择设备
    audioManager->selectInputDevice(inputDevices.first());
    audioManager->selectOutputDevice(outputDevices.first());
    
    // 设置音量
    audioManager->setMasterVolume(0.8);
    audioManager->setMicrophoneGain(0.6);
}
```

### 音频配置管理
```cpp
#include "AudioConfig.h"

// 创建配置对象
AudioConfig config;

// 设置音频参数
config.setSampleRate(48000);
config.setChannels(2);
config.setBufferSize(1024);
config.setQualityPreset(AudioConfig::HighQuality);

// 保存配置
config.save();
```

## 测试

### 单元测试
```bash
# 运行音频模块测试
qmake audio_tests.pro
make
./audio_tests
```

### 集成测试
```bash
# 运行集成测试
qmake integration_tests.pro
make
./integration_tests
```

## 依赖关系

### 必需依赖
- Qt Core (5.15+)
- Qt Multimedia
- WebRTC (可选)

### 推荐依赖
- Utils模块 (用于日志记录)
- Performance模块 (用于性能监控)

## 开发状态

### 已完成 ✅
- [x] 模块架构设计
- [x] 目录结构创建
- [x] 配置文件设置
- [x] 接口定义
- [x] 核心类实现
- [x] 平台特定代码
- [x] UI组件开发
- [x] 测试框架
- [x] 基础功能实现

### 进行中 🔄
- [x] WebRTC集成
- [x] 性能优化
- [x] 文档完善
- [x] 示例程序

### 已验证 ✅
- [x] 跨平台兼容性
- [x] 模块间通信
- [x] 配置管理
- [x] 错误处理

## 贡献指南

1. 遵循现有的代码风格和架构模式
2. 为新功能添加相应的测试
3. 更新文档和示例
4. 确保跨平台兼容性

## 许可证

本模块遵循项目的整体许可证协议。

## 联系信息

如有问题或建议，请通过项目的Issue系统提交。