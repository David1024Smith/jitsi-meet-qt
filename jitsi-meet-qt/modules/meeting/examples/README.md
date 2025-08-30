# Meeting Module Examples

## 概述

本目录包含Meeting模块的使用示例，展示如何在应用程序中集成和使用会议功能。

## 示例列表

### 基础示例

#### BasicMeetingExample.cpp
展示Meeting模块的基本使用方法：
- 模块初始化
- 创建会议
- 加入会议
- 基本配置

#### MeetingWidgetExample.cpp
展示如何使用MeetingWidget组件：
- 组件创建和配置
- 事件处理
- 界面定制

#### LinkHandlerExample.cpp
展示链接处理功能：
- URL解析
- 链接验证
- 协议处理

### 高级示例

#### CustomMeetingApp.cpp
完整的会议应用程序示例：
- 完整的用户界面
- 会议管理
- 用户认证
- 设置管理

#### MeetingBotExample.cpp
会议机器人示例：
- 自动加入会议
- 消息处理
- 录制功能

#### ProtocolHandlerExample.cpp
自定义协议处理示例：
- 协议注册
- 自定义URL处理
- 系统集成

### 集成示例

#### QtWidgetsIntegration.cpp
与Qt Widgets应用程序集成：
- 主窗口集成
- 菜单和工具栏
- 状态栏显示

#### QMLIntegration.cpp
与QML应用程序集成：
- QML组件暴露
- 属性绑定
- 信号处理

## 编译和运行

### 前提条件
- Qt 5.15 或更高版本
- Meeting模块已编译
- 相关依赖库

### 编译单个示例

```bash
# 使用qmake
qmake BasicMeetingExample.pro
make

# 使用CMake
mkdir build
cd build
cmake ..
make
```

### 运行示例

```bash
# 基础示例
./BasicMeetingExample

# 组件示例
./MeetingWidgetExample

# 自定义应用
./CustomMeetingApp
```

## 示例说明

### BasicMeetingExample

```cpp
#include "MeetingModule.h"
#include "MeetingManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 初始化Meeting模块
    auto* meetingModule = MeetingModule::instance();
    meetingModule->initialize();
    
    // 获取会议管理器
    auto* meetingManager = meetingModule->meetingManager();
    
    // 创建会议
    meetingManager->createMeeting("Test Meeting");
    
    return app.exec();
}
```

### MeetingWidgetExample

```cpp
#include "MeetingWidget.h"
#include "MeetingManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 创建会议组件
    MeetingWidget widget;
    widget.setMeetingManager(meetingManager);
    widget.show();
    
    return app.exec();
}
```

## 配置示例

### 基本配置

```json
{
    "meeting": {
        "defaultServer": "meet.jit.si",
        "autoJoin": false,
        "displayName": "User",
        "audioEnabled": true,
        "videoEnabled": true
    }
}
```

### 高级配置

```json
{
    "meeting": {
        "servers": [
            "meet.jit.si",
            "custom.meeting.server"
        ],
        "authentication": {
            "method": "guest",
            "required": false
        },
        "ui": {
            "theme": "default",
            "showParticipants": true,
            "showStatistics": false
        },
        "network": {
            "timeout": 30000,
            "retryAttempts": 3
        }
    }
}
```

## 最佳实践

### 错误处理

```cpp
connect(meetingManager, &MeetingManager::errorOccurred,
        [](const QString& error) {
    qWarning() << "Meeting error:" << error;
    // 显示用户友好的错误信息
});
```

### 状态管理

```cpp
connect(meetingManager, &MeetingManager::stateChanged,
        [](MeetingManager::MeetingState state) {
    switch (state) {
        case MeetingManager::Connected:
            // 更新UI状态
            break;
        case MeetingManager::Error:
            // 处理错误状态
            break;
    }
});
```

### 资源清理

```cpp
// 在应用程序退出时清理资源
QObject::connect(&app, &QApplication::aboutToQuit, []() {
    MeetingModule::instance()->shutdown();
});
```

## 故障排除

### 常见问题

1. **模块初始化失败**
   - 检查依赖库
   - 验证配置文件
   - 查看错误日志

2. **网络连接问题**
   - 检查网络设置
   - 验证服务器地址
   - 测试防火墙设置

3. **UI显示异常**
   - 检查主题配置
   - 验证资源文件
   - 确认Qt版本兼容性

### 调试技巧

```cpp
// 启用调试输出
QLoggingCategory::setFilterRules("meeting.debug=true");

// 设置调试级别
meetingConfig->setDebugEnabled(true);
meetingConfig->setLogLevel("debug");
```

## 扩展示例

### 自定义UI主题

```cpp
// 创建自定义主题
class CustomMeetingTheme : public QObject {
public:
    void applyTheme(MeetingWidget* widget) {
        widget->setStyleSheet(
            "MeetingWidget { background-color: #2b2b2b; }"
            "QPushButton { background-color: #4a90e2; }"
        );
    }
};
```

### 插件集成

```cpp
// 创建会议插件接口
class MeetingPlugin {
public:
    virtual void onMeetingJoined(const QString& meetingId) = 0;
    virtual void onMeetingLeft() = 0;
};
```

## 许可证

所有示例代码遵循项目主许可证。

## 贡献

欢迎提交新的示例和改进建议。请确保：
1. 代码清晰易懂
2. 包含适当的注释
3. 提供使用说明
4. 测试示例功能