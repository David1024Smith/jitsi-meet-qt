# Meeting Module

## 概述

Meeting模块负责处理Jitsi Meet Qt应用程序中的会议链接管理、会议创建、加入和认证功能。该模块提供了完整的会议生命周期管理，包括链接解析、验证、会议室管理和用户认证。

## 版本信息

- **版本**: 1.0.0
- **兼容性**: Qt 5.15+
- **依赖**: Core, Network, Utils模块

## 功能特性

### 核心功能
- 会议链接解析和验证
- 会议创建和管理
- 会议加入流程
- 用户认证和权限管理
- 会议室状态管理

### 链接处理
- URL协议解析 (jitsi://, https://)
- 会议参数提取和验证
- 自定义协议支持
- 链接安全性验证

### 会议管理
- 会议创建向导
- 会议设置配置
- 参与者管理
- 会议状态监控

### 用户界面
- 会议加入对话框
- 会议创建对话框
- 会议控制组件
- 状态显示组件

## 架构设计

```
Meeting Module
├── Core Layer (MeetingModule, MeetingManager)
├── Handler Layer (URLHandler, ProtocolHandler, AuthHandler)
├── Model Layer (Meeting, Room, Invitation)
├── Widget Layer (MeetingWidget, JoinDialog, CreateDialog)
├── Config Layer (MeetingConfig)
└── Interface Layer (IMeetingManager, ILinkHandler, IRoomManager)
```

## 目录结构

```
meeting/
├── meeting.pri              # 模块配置文件
├── README.md               # 本文档
├── include/                # 核心头文件
│   ├── MeetingModule.h     # 会议模块核心
│   ├── MeetingManager.h    # 会议管理器
│   └── LinkHandler.h       # 链接处理器
├── src/                    # 核心实现
│   ├── MeetingModule.cpp
│   ├── MeetingManager.cpp
│   └── LinkHandler.cpp
├── interfaces/             # 接口定义
│   ├── IMeetingManager.h   # 会议管理器接口
│   ├── ILinkHandler.h      # 链接处理接口
│   └── IRoomManager.h      # 房间管理器接口
├── config/                 # 配置管理
│   ├── MeetingConfig.h     # 会议配置类
│   └── MeetingConfig.cpp
├── handlers/               # 处理器实现
│   ├── URLHandler.h        # URL处理器
│   ├── URLHandler.cpp
│   ├── ProtocolHandler.h   # 协议处理器
│   ├── ProtocolHandler.cpp
│   ├── AuthHandler.h       # 认证处理器
│   └── AuthHandler.cpp
├── models/                 # 数据模型
│   ├── Meeting.h           # 会议模型
│   ├── Meeting.cpp
│   ├── Room.h              # 房间模型
│   ├── Room.cpp
│   ├── Invitation.h        # 邀请模型
│   └── Invitation.cpp
├── widgets/                # UI组件
│   ├── MeetingWidget.h     # 会议组件
│   ├── MeetingWidget.cpp
│   ├── JoinDialog.h        # 加入对话框
│   ├── JoinDialog.cpp
│   ├── CreateDialog.h      # 创建对话框
│   └── CreateDialog.cpp
├── tests/                  # 测试框架
│   ├── README.md
│   ├── CMakeLists.txt
│   ├── meeting_tests.pro
│   ├── MeetingModuleTest.h
│   ├── MeetingModuleTest.cpp
│   ├── run_tests.sh
│   ├── run_tests.bat
│   └── mocks/
│       ├── MockMeetingManager.h
│       └── MockMeetingManager.cpp
├── examples/               # 示例代码
│   ├── README.md
│   ├── BasicMeetingExample.cpp
│   └── MeetingWidgetExample.cpp
└── resources/              # 资源文件
    ├── README.md
    ├── meeting_resources.qrc
    ├── icons/
    │   ├── meeting.png
    │   ├── join.png
    │   └── create.png
    └── styles/
        └── meeting.qss
```

## 快速开始

### 基本使用

```cpp
#include "MeetingManager.h"
#include "MeetingConfig.h"

// 初始化会议管理器
auto* meetingManager = new MeetingManager(this);
meetingManager->initialize();

// 配置会议设置
MeetingConfig config;
config.setDefaultServer("meet.jit.si");
config.setAutoJoin(false);
meetingManager->setConfiguration(config);

// 处理会议链接
QString meetingUrl = "https://meet.jit.si/MyMeeting";
if (meetingManager->validateMeetingUrl(meetingUrl)) {
    meetingManager->joinMeeting(meetingUrl);
}
```

### 创建会议

```cpp
#include "CreateDialog.h"

// 显示会议创建对话框
auto* createDialog = new CreateDialog(this);
if (createDialog->exec() == QDialog::Accepted) {
    QString meetingName = createDialog->getMeetingName();
    QVariantMap settings = createDialog->getMeetingSettings();
    
    meetingManager->createMeeting(meetingName, settings);
}
```

### 加入会议

```cpp
#include "JoinDialog.h"

// 显示会议加入对话框
auto* joinDialog = new JoinDialog(this);
joinDialog->setMeetingUrl(meetingUrl);
if (joinDialog->exec() == QDialog::Accepted) {
    QString displayName = joinDialog->getDisplayName();
    bool audioEnabled = joinDialog->isAudioEnabled();
    bool videoEnabled = joinDialog->isVideoEnabled();
    
    meetingManager->joinMeeting(meetingUrl, displayName, audioEnabled, videoEnabled);
}
```

## API 参考

### 核心接口

#### IMeetingManager
- `initialize()` - 初始化会议管理器
- `createMeeting()` - 创建新会议
- `joinMeeting()` - 加入会议
- `leaveMeeting()` - 离开会议
- `validateMeetingUrl()` - 验证会议链接

#### ILinkHandler
- `parseUrl()` - 解析会议URL
- `validateUrl()` - 验证URL有效性
- `extractParameters()` - 提取会议参数

#### IRoomManager
- `getRoomInfo()` - 获取房间信息
- `setRoomSettings()` - 设置房间配置
- `manageParticipants()` - 管理参与者

### 配置选项

#### MeetingConfig
- `defaultServer` - 默认服务器地址
- `autoJoin` - 自动加入会议
- `displayName` - 默认显示名称
- `audioEnabled` - 默认音频状态
- `videoEnabled` - 默认视频状态

## 测试

### 运行测试

```bash
# Linux/macOS
cd tests
./run_tests.sh

# Windows
cd tests
run_tests.bat
```

### 测试覆盖

- 链接解析和验证测试
- 会议创建和加入测试
- 认证和权限测试
- UI组件交互测试
- 错误处理和恢复测试

## 依赖关系

### 必需依赖
- Qt Core (5.15+)
- Qt Widgets
- Qt Network
- Utils Module (日志和工具)

### 可选依赖
- Network Module (网络管理)
- Settings Module (配置管理)

## 配置示例

### 基本配置

```json
{
    "meeting": {
        "defaultServer": "meet.jit.si",
        "autoJoin": false,
        "displayName": "",
        "audioEnabled": true,
        "videoEnabled": true,
        "protocols": ["https", "jitsi"],
        "authentication": {
            "required": false,
            "method": "guest"
        }
    }
}
```

## 故障排除

### 常见问题

1. **链接解析失败**
   - 检查URL格式是否正确
   - 验证协议支持
   - 确认网络连接

2. **会议加入失败**
   - 检查服务器可达性
   - 验证认证信息
   - 确认权限设置

3. **UI组件显示异常**
   - 检查主题配置
   - 验证资源文件
   - 确认依赖模块

### 调试选项

```cpp
// 启用调试日志
MeetingConfig config;
config.setDebugEnabled(true);
config.setLogLevel(MeetingConfig::Debug);
```

## 贡献指南

1. 遵循现有代码风格
2. 添加适当的测试覆盖
3. 更新相关文档
4. 确保向后兼容性

## 许可证

本模块遵循项目主许可证。

## 更新日志

### v1.0.0 (2024-01-01)
- 初始版本发布
- 基础会议管理功能
- 链接处理和验证
- UI组件实现