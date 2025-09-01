# Chat Module

## 概述

聊天模块为Jitsi Meet Qt应用程序提供完整的聊天功能，包括消息发送、接收、存储和聊天界面管理。

## 版本信息

- **模块版本**: 1.0.0
- **Qt版本要求**: 5.15+
- **依赖模块**: Utils, Network, UI

## 功能特性

### 核心功能
- 实时消息发送和接收
- 消息历史存储和管理
- 多种消息类型支持（文本、表情符号、文件分享）
- 聊天室管理
- 参与者管理

### 界面功能
- 聊天窗口组件
- 消息列表显示
- 消息输入组件
- 聊天设置界面

### 配置功能
- 聊天参数配置
- 消息过滤设置
- 通知设置管理

## 架构设计

```
Chat Module Architecture
├── 接口层 (Interfaces)
│   ├── IChatManager - 聊天管理器接口
│   ├── IMessageHandler - 消息处理接口
│   └── IMessageStorage - 消息存储接口
├── 核心层 (Core)
│   ├── ChatModule - 聊天模块核心
│   ├── ChatManager - 聊天管理器
│   └── MessageHandler - 消息处理器
├── 数据层 (Models)
│   ├── ChatMessage - 消息模型
│   ├── ChatRoom - 聊天室模型
│   └── Participant - 参与者模型
├── 存储层 (Storage)
│   ├── MessageStorage - 消息存储
│   └── HistoryManager - 历史管理器
├── 界面层 (Widgets)
│   ├── ChatWidget - 聊天组件
│   ├── MessageList - 消息列表
│   └── InputWidget - 输入组件
└── 配置层 (Config)
    └── ChatConfig - 聊天配置
```

## 目录结构

```
modules/chat/
├── chat.pri                    # 模块配置文件
├── README.md                   # 模块文档
├── include/                    # 核心头文件
│   ├── ChatModule.h            # 聊天模块核心
│   ├── ChatManager.h           # 聊天管理器
│   └── MessageHandler.h        # 消息处理器
├── src/                        # 核心实现
│   ├── ChatModule.cpp
│   ├── ChatManager.cpp
│   └── MessageHandler.cpp
├── interfaces/                 # 接口定义
│   ├── IChatManager.h          # 聊天管理器接口
│   ├── IMessageHandler.h       # 消息处理接口
│   └── IMessageStorage.h       # 消息存储接口
├── config/                     # 配置管理
│   ├── ChatConfig.h            # 聊天配置类
│   └── ChatConfig.cpp
├── models/                     # 数据模型
│   ├── ChatMessage.h           # 聊天消息模型
│   ├── ChatRoom.h              # 聊天房间模型
│   └── Participant.h           # 参与者模型
├── storage/                    # 存储系统
│   ├── MessageStorage.h        # 消息存储
│   ├── MessageStorage.cpp
│   ├── HistoryManager.h        # 历史管理器
│   └── HistoryManager.cpp
├── widgets/                    # UI组件
│   ├── ChatWidget.h            # 聊天组件
│   ├── ChatWidget.cpp
│   ├── MessageList.h           # 消息列表
│   ├── MessageList.cpp
│   ├── InputWidget.h           # 输入组件
│   └── InputWidget.cpp
├── tests/                      # 测试框架
│   ├── ChatModuleTest.h
│   ├── ChatModuleTest.cpp
│   ├── chat_tests.pro
│   ├── CMakeLists.txt
│   └── README.md
├── examples/                   # 示例代码
│   ├── BasicChatExample.cpp
│   ├── ChatWidgetExample.cpp
│   └── README.md
└── resources/                  # 资源文件
    ├── icons/
    ├── styles/
    └── README.md
```

## 使用方法

### 基本使用

```cpp
#include "ChatModule.h"
#include "ChatManager.h"

// 初始化聊天模块
ChatModule* chatModule = new ChatModule();
if (chatModule->initialize()) {
    // 获取聊天管理器
    ChatManager* manager = chatModule->chatManager();
    
    // 连接信号
    connect(manager, &ChatManager::messageReceived, 
            this, &MyClass::onMessageReceived);
    
    // 发送消息
    manager->sendMessage("Hello, World!");
}
```

### 配置聊天

```cpp
#include "ChatConfig.h"

ChatConfig config;
config.setMaxMessageLength(1000);
config.setHistoryEnabled(true);
config.setNotificationsEnabled(true);

chatModule->setConfiguration(config);
```

## 编译说明

### qmake
```bash
# 在项目根目录
qmake
make
```

### CMake
```bash
mkdir build && cd build
cmake ..
make
```

## 测试

```bash
# 运行所有测试
cd tests
qmake chat_tests.pro
make
./chat_tests

# 或使用CMake
cd build
make test
```

## 依赖关系

- **Qt Modules**: Core, Widgets, Network
- **Internal Modules**: Utils (日志、工具), Network (网络通信), UI (界面组件)
- **External Libraries**: 无

## 配置选项

在 `chat.pri` 中可以配置以下选项：

```qmake
# 启用聊天模块
CONFIG += chat_module_enabled

# 启用调试输出
CONFIG += chat_debug_enabled

# 启用消息加密
CONFIG += chat_encryption_enabled
```

## 注意事项

1. 确保在使用前正确初始化模块
2. 消息存储需要足够的磁盘空间
3. 网络连接状态会影响消息发送
4. 建议定期清理历史消息以节省空间

## 更新日志

### v1.0.0 (2024-12-XX)
- 初始版本发布
- 实现基础聊天功能
- 添加消息存储和历史管理
- 提供完整的UI组件

## 许可证

本模块遵循项目主许可证。

## 联系方式

如有问题或建议，请通过项目仓库提交Issue。