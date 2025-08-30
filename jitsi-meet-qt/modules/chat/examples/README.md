# Chat Module Examples

## 概述

本目录包含聊天模块的使用示例，展示如何集成和使用聊天功能。

## 示例列表

### 基础示例
- **BasicChatExample.cpp**: 基本聊天功能示例
- **ChatWidgetExample.cpp**: 聊天组件使用示例
- **MessageHandlingExample.cpp**: 消息处理示例
- **ConfigurationExample.cpp**: 配置管理示例

### 高级示例
- **CustomThemeExample.cpp**: 自定义主题示例
- **FileTransferExample.cpp**: 文件传输示例
- **ChatBotExample.cpp**: 聊天机器人示例
- **MultiRoomExample.cpp**: 多房间聊天示例

### 集成示例
- **NetworkIntegrationExample.cpp**: 网络集成示例
- **DatabaseIntegrationExample.cpp**: 数据库集成示例
- **UIIntegrationExample.cpp**: UI集成示例

## 快速开始

### 1. 基本聊天应用

```cpp
#include "ChatModule.h"
#include "ChatManager.h"
#include "ChatWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 创建聊天模块
    ChatModule chatModule;
    if (!chatModule.initialize()) {
        qCritical() << "Failed to initialize chat module";
        return -1;
    }
    
    // 创建聊天界面
    ChatWidget chatWidget;
    chatWidget.setChatManager(chatModule.chatManager());
    chatWidget.show();
    
    return app.exec();
}
```

### 2. 消息发送和接收

```cpp
// 连接到聊天服务
ChatManager* manager = chatModule.chatManager();
manager->connectToService("wss://chat.example.com");

// 加入房间
manager->joinRoom("general");

// 发送消息
manager->sendMessage("Hello, World!");

// 接收消息
connect(manager, &ChatManager::messageReceived, [](ChatMessage* message) {
    qDebug() << "Received:" << message->content();
});
```

### 3. 自定义配置

```cpp
ChatConfig config;
config.setServerUrl("wss://my-chat-server.com");
config.setMaxMessageLength(1000);
config.setHistoryEnabled(true);
config.setNotificationsEnabled(true);

chatModule.setConfiguration(config);
```

## 编译和运行

### 使用qmake
```bash
cd examples
qmake examples.pro
make
./BasicChatExample
```

### 使用CMake
```bash
mkdir build && cd build
cmake ..
make
./BasicChatExample
```

## 示例说明

### BasicChatExample
展示最基本的聊天功能：
- 初始化聊天模块
- 创建聊天界面
- 连接到服务器
- 发送和接收消息

### ChatWidgetExample
演示聊天组件的使用：
- 自定义聊天界面
- 设置主题和样式
- 处理用户交互
- 集成到现有应用

### MessageHandlingExample
展示消息处理功能：
- 消息验证和过滤
- 消息格式化
- 消息存储和检索
- 错误处理

### ConfigurationExample
演示配置管理：
- 加载和保存配置
- 运行时配置更改
- 配置验证
- 默认配置设置

### CustomThemeExample
展示主题定制：
- 创建自定义主题
- 动态切换主题
- 样式表定制
- 响应式设计

### FileTransferExample
演示文件传输功能：
- 文件上传和下载
- 进度显示
- 文件类型验证
- 错误处理

### ChatBotExample
展示聊天机器人集成：
- 自动回复
- 命令处理
- AI集成
- 自定义响应

### MultiRoomExample
演示多房间聊天：
- 房间管理
- 房间切换
- 权限控制
- 通知管理

## 依赖项

运行示例需要以下依赖：
- Qt 5.15+ (Core, Widgets, Network)
- Chat Module
- 可选：WebSocket支持

## 配置要求

### 最小配置
- 内存：512MB
- 存储：100MB
- 网络：宽带连接

### 推荐配置
- 内存：2GB+
- 存储：1GB+
- 网络：稳定的宽带连接

## 故障排除

### 常见问题

1. **编译错误**
   - 检查Qt版本兼容性
   - 确保包含路径正确
   - 验证依赖项安装

2. **运行时错误**
   - 检查网络连接
   - 验证服务器地址
   - 查看日志输出

3. **连接问题**
   - 检查防火墙设置
   - 验证SSL证书
   - 测试网络连通性

### 调试技巧

1. **启用详细日志**
```cpp
QLoggingCategory::setFilterRules("chat.*.debug=true");
```

2. **使用调试器**
```bash
gdb ./BasicChatExample
```

3. **网络调试**
```cpp
// 启用网络日志
qputenv("QT_LOGGING_RULES", "qt.network.ssl.debug=true");
```

## 扩展示例

### 添加新示例
1. 创建新的.cpp文件
2. 包含必要的头文件
3. 实现示例功能
4. 更新examples.pro或CMakeLists.txt
5. 添加文档说明

### 示例模板
```cpp
#include <QApplication>
#include "ChatModule.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 你的示例代码
    
    return app.exec();
}
```

## 贡献

欢迎贡献新的示例：
1. Fork项目仓库
2. 创建示例分支
3. 添加示例代码和文档
4. 提交Pull Request

## 许可证

示例代码遵循项目主许可证。

## 支持

如需帮助：
1. 查看文档和FAQ
2. 搜索已知问题
3. 在项目仓库创建Issue
4. 联系开发团队