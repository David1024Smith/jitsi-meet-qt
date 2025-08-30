# 音频配置管理系统

## 概述

AudioConfig类是Jitsi Meet Qt音频模块的核心配置管理组件，负责管理所有音频相关的配置参数，包括设备设置、音频质量、音量控制、音频处理功能等。

## 主要功能

### 1. 设备配置管理
- 首选输入设备（麦克风）选择
- 首选输出设备（扬声器）选择
- 设备配置变更通知

### 2. 音频质量管理
- 采样率配置 (8kHz - 192kHz)
- 声道数配置 (1-8声道)
- 缓冲区大小配置 (64-8192样本)
- 比特率配置
- 预设质量模式（低质量/标准质量/高质量）

### 3. 音量控制
- 主音量控制 (0.0-1.0)
- 麦克风增益控制 (0.0-1.0)
- 静音状态管理
- 音量边界值自动限制

### 4. 音频处理功能
- 噪声抑制开关
- 回声消除开关
- 自动增益控制开关

### 5. 配置持久化
- 配置文件自动保存/加载
- INI格式配置存储
- 配置验证和错误处理
- 默认配置重置功能

### 6. 高级功能
- 自定义参数支持
- 配置序列化/反序列化
- 信号通知机制
- 配置变更追踪

## 质量预设

### 低质量模式 (LowQuality)
- 采样率: 16kHz
- 声道数: 1 (单声道)
- 比特率: 64kbps
- 缓冲区: 512样本
- 适用场景: 网络带宽受限环境

### 标准质量模式 (StandardQuality) - 默认
- 采样率: 44.1kHz
- 声道数: 2 (立体声)
- 比特率: 128kbps
- 缓冲区: 1024样本
- 适用场景: 一般会议场景

### 高质量模式 (HighQuality)
- 采样率: 48kHz
- 声道数: 2 (立体声)
- 比特率: 256kbps
- 缓冲区: 2048样本
- 适用场景: 高质量音频需求

## 使用示例

```cpp
#include "AudioConfig.h"

// 创建配置实例
AudioConfig audioConfig;

// 基础设备配置
audioConfig.setPreferredInputDevice("default_microphone");
audioConfig.setPreferredOutputDevice("default_speakers");

// 音质配置
audioConfig.setQualityPreset(AudioConfig::HighQuality);

// 音量配置
audioConfig.setMasterVolume(0.8);
audioConfig.setMicrophoneGain(0.6);

// 音频处理
audioConfig.setNoiseSuppressionEnabled(true);
audioConfig.setEchoCancellationEnabled(true);

// 保存配置
audioConfig.save();
```

## 信号机制

AudioConfig提供了多种信号来通知配置变更：

```cpp
// 通用配置变更信号
connect(&audioConfig, &AudioConfig::configChanged, 
        [](const QString& key, const QVariant& value) {
    qDebug() << "配置变更:" << key << "=" << value;
});

// 特定类型配置变更信号
connect(&audioConfig, &AudioConfig::deviceConfigChanged, 
        []() { qDebug() << "设备配置已变更"; });

connect(&audioConfig, &AudioConfig::qualityConfigChanged, 
        []() { qDebug() << "音质配置已变更"; });

connect(&audioConfig, &AudioConfig::volumeConfigChanged, 
        []() { qDebug() << "音量配置已变更"; });

connect(&audioConfig, &AudioConfig::processingConfigChanged, 
        []() { qDebug() << "处理配置已变更"; });
```

## 配置验证

AudioConfig提供了内置的配置验证功能：

```cpp
bool isValid = audioConfig.validate();
if (!isValid) {
    qWarning() << "音频配置无效";
    audioConfig.resetToDefaults(); // 重置为默认配置
}
```

验证规则：
- 采样率: 8000-192000 Hz
- 声道数: 1-8
- 缓冲区大小: 64-8192样本
- 音量值: 0.0-1.0

## 配置文件格式

配置以INI格式存储，默认位置：
- Windows: `%APPDATA%/JitsiMeetQt/audio_config.ini`
- Linux: `~/.config/JitsiMeetQt/audio_config.ini`
- macOS: `~/Library/Preferences/JitsiMeetQt/audio_config.ini`

示例配置文件：
```ini
[Audio]
PreferredInputDevice=default_microphone
PreferredOutputDevice=default_speakers
SampleRate=48000
Channels=2
BufferSize=1024
Bitrate=128
QualityPreset=1
MasterVolume=0.8
MicrophoneGain=0.6
Muted=false
NoiseSuppressionEnabled=true
EchoCancellationEnabled=true
AutoGainControlEnabled=false

[CustomParameters]
custom_filter=low_pass
custom_threshold=0.5
```

## 自定义参数

支持添加自定义配置参数：

```cpp
// 设置自定义参数
audioConfig.setCustomParameter("custom_filter", "low_pass");
audioConfig.setCustomParameter("custom_threshold", 0.5);

// 获取自定义参数
QString filter = audioConfig.customParameter("custom_filter").toString();
double threshold = audioConfig.customParameter("custom_threshold", 0.0).toDouble();

// 获取所有自定义参数
QVariantMap customParams = audioConfig.customParameters();
```

## 序列化支持

AudioConfig支持完整的序列化和反序列化：

```cpp
// 序列化为QVariantMap
QVariantMap configMap = audioConfig.toVariantMap();

// 从QVariantMap反序列化
AudioConfig newConfig;
newConfig.fromVariantMap(configMap);
```

## 线程安全

AudioConfig不是线程安全的，在多线程环境中使用时需要适当的同步机制。

## 最佳实践

1. **配置初始化**: 应用启动时加载配置
2. **配置验证**: 加载配置后进行验证
3. **信号连接**: 连接配置变更信号以响应变化
4. **定期保存**: 配置变更后及时保存
5. **错误处理**: 处理配置加载/保存失败的情况

## 相关文件

- `AudioConfig.h` - 头文件定义
- `AudioConfig.cpp` - 实现文件
- `AudioConfigTest.cpp` - 单元测试
- `audio_config_example.cpp` - 使用示例