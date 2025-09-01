# 屏幕共享模块 (ScreenShare Module)

## 概述

屏幕共享模块提供完整的屏幕捕获、编码和共享功能，支持全屏捕获、窗口捕获和区域捕获等多种模式。

## 功能特性

- **屏幕捕获**: 支持全屏、窗口和自定义区域捕获
- **视频编码**: 高效的视频编码和帧处理
- **质量控制**: 自适应质量调整和性能优化
- **UI组件**: 完整的屏幕共享控制界面
- **配置管理**: 灵活的捕获参数配置

## 架构设计

```
ScreenShare Module
├── 接口层 (Interfaces)
│   ├── IScreenCapture - 屏幕捕获接口
│   ├── IScreenShareManager - 屏幕共享管理器接口
│   └── IDisplayManager - 显示管理器接口
├── 核心层 (Core)
│   ├── ScreenShareModule - 模块核心
│   ├── ScreenShareManager - 共享管理器
│   └── CaptureEngine - 捕获引擎
├── 捕获系统 (Capture)
│   ├── ScreenCapture - 屏幕捕获
│   ├── WindowCapture - 窗口捕获
│   └── RegionCapture - 区域捕获
├── 编码系统 (Encoding)
│   ├── VideoEncoder - 视频编码器
│   └── FrameProcessor - 帧处理器
├── 配置管理 (Config)
│   └── ScreenShareConfig - 配置类
└── UI组件 (Widgets)
    ├── ScreenShareWidget - 主控制组件
    ├── ScreenSelector - 屏幕选择器
    └── CapturePreview - 捕获预览
```

## 使用示例

```cpp
#include "ScreenShareManager.h"
#include "ScreenShareConfig.h"

// 初始化屏幕共享管理器
auto manager = new ScreenShareManager(this);
manager->initialize();

// 配置捕获参数
ScreenShareConfig config;
config.setCaptureMode(ScreenShareConfig::FullScreen);
config.setFrameRate(30);
config.setQuality(ScreenShareConfig::HighQuality);

// 开始屏幕共享
manager->startScreenShare(config);
```

## 编译配置

在项目的 .pro 文件中包含:

```qmake
include(modules/screenshare/screenshare.pri)
```

## 依赖关系

- Qt Core (>= 5.15)
- Qt GUI (>= 5.15)
- Qt Widgets (>= 5.15)
- Qt Multimedia (>= 5.15)
- WebRTC (用于视频编码)

## 版本信息

- 版本: 1.0.0
- 最后更新: 2024-12-29
- 兼容性: Qt 5.15+

## 许可证

本模块遵循项目主许可证。