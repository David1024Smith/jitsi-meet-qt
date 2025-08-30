# Meeting Module Resources

## 概述

本目录包含Meeting模块使用的所有资源文件，包括图标、样式表、配置文件和多语言支持。

## 资源结构

```
resources/
├── README.md                   # 本文档
├── meeting_resources.qrc       # Qt资源文件
├── icons/                      # 图标资源
│   ├── meeting.png            # 会议图标
│   ├── join.png               # 加入图标
│   ├── create.png             # 创建图标
│   ├── leave.png              # 离开图标
│   ├── invite.png             # 邀请图标
│   ├── settings.png           # 设置图标
│   ├── audio-on.png           # 音频开启图标
│   ├── audio-off.png          # 音频关闭图标
│   ├── video-on.png           # 视频开启图标
│   ├── video-off.png          # 视频关闭图标
│   ├── screen-share.png       # 屏幕共享图标
│   ├── chat.png               # 聊天图标
│   ├── participants.png       # 参与者图标
│   ├── lock.png               # 锁定图标
│   ├── unlock.png             # 解锁图标
│   ├── record.png             # 录制图标
│   └── quality/               # 质量指示图标
│       ├── excellent.png
│       ├── good.png
│       ├── fair.png
│       └── poor.png
├── styles/                     # 样式表
│   ├── meeting.qss            # 主样式表
│   ├── dark-theme.qss         # 暗色主题
│   ├── light-theme.qss        # 亮色主题
│   └── components/            # 组件样式
│       ├── meeting-widget.qss
│       ├── join-dialog.qss
│       └── create-dialog.qss
├── translations/               # 多语言支持
│   ├── meeting_en.ts          # 英文翻译
│   ├── meeting_zh_CN.ts       # 简体中文翻译
│   ├── meeting_zh_TW.ts       # 繁体中文翻译
│   ├── meeting_ja.ts          # 日文翻译
│   ├── meeting_ko.ts          # 韩文翻译
│   ├── meeting_fr.ts          # 法文翻译
│   ├── meeting_de.ts          # 德文翻译
│   └── meeting_es.ts          # 西班牙文翻译
├── sounds/                     # 音效文件
│   ├── join.wav               # 加入音效
│   ├── leave.wav              # 离开音效
│   ├── notification.wav       # 通知音效
│   └── error.wav              # 错误音效
├── templates/                  # 模板文件
│   ├── meeting-templates.json # 会议模板
│   ├── invitation-templates/  # 邀请模板
│   │   ├── default.html
│   │   ├── formal.html
│   │   └── casual.html
│   └── config-templates/      # 配置模板
│       ├── basic.json
│       ├── advanced.json
│       └── enterprise.json
└── data/                      # 数据文件
    ├── server-list.json       # 服务器列表
    ├── default-config.json    # 默认配置
    └── help/                  # 帮助文档
        ├── user-guide.html
        ├── troubleshooting.html
        └── faq.html
```

## 图标规范

### 尺寸标准
- 16x16: 小图标（菜单、按钮）
- 24x24: 中等图标（工具栏）
- 32x32: 大图标（对话框）
- 48x48: 超大图标（主界面）

### 格式要求
- PNG格式，支持透明背景
- SVG格式用于矢量图标
- 高DPI支持（@2x, @3x版本）

### 命名规范
- 使用小写字母和连字符
- 包含状态后缀（-on, -off, -disabled）
- 包含尺寸后缀（@2x, @3x）

## 样式表

### 主题系统
Meeting模块支持多主题切换：

```css
/* 默认主题 */
MeetingWidget {
    background-color: #ffffff;
    color: #333333;
    border: 1px solid #cccccc;
}

/* 暗色主题 */
MeetingWidget[theme="dark"] {
    background-color: #2b2b2b;
    color: #ffffff;
    border: 1px solid #555555;
}
```

### 组件样式
每个UI组件都有独立的样式文件：

```css
/* 会议组件样式 */
MeetingWidget QPushButton {
    background-color: #4a90e2;
    color: white;
    border: none;
    border-radius: 4px;
    padding: 8px 16px;
}

MeetingWidget QPushButton:hover {
    background-color: #357abd;
}

MeetingWidget QPushButton:pressed {
    background-color: #2968a3;
}
```

## 多语言支持

### 翻译文件
使用Qt的翻译系统支持多语言：

```xml
<!-- meeting_zh_CN.ts -->
<TS version="2.1" language="zh_CN">
<context>
    <name>MeetingWidget</name>
    <message>
        <source>Join Meeting</source>
        <translation>加入会议</translation>
    </message>
    <message>
        <source>Create Meeting</source>
        <translation>创建会议</translation>
    </message>
</context>
</TS>
```

### 使用方法

```cpp
// 在代码中使用翻译
QString joinText = tr("Join Meeting");
QString createText = tr("Create Meeting");

// 动态切换语言
QTranslator translator;
translator.load("meeting_zh_CN.qm");
QApplication::installTranslator(&translator);
```

## 音效文件

### 音效规范
- WAV格式，16位，44.1kHz
- 时长不超过2秒
- 音量适中，避免突兀

### 使用场景
- join.wav: 用户加入会议时播放
- leave.wav: 用户离开会议时播放
- notification.wav: 收到通知时播放
- error.wav: 发生错误时播放

## 模板文件

### 会议模板
预定义的会议配置模板：

```json
{
    "templates": [
        {
            "name": "Quick Meeting",
            "description": "Fast setup for instant meetings",
            "settings": {
                "type": "instant",
                "audioEnabled": true,
                "videoEnabled": true,
                "allowGuests": true
            }
        },
        {
            "name": "Formal Meeting",
            "description": "Professional meeting setup",
            "settings": {
                "type": "scheduled",
                "passwordRequired": true,
                "waitingRoom": true,
                "recording": true
            }
        }
    ]
}
```

### 邀请模板
HTML格式的邀请邮件模板：

```html
<!-- default.html -->
<!DOCTYPE html>
<html>
<head>
    <title>Meeting Invitation</title>
</head>
<body>
    <h2>You're invited to join a meeting</h2>
    <p>Meeting: {{meetingName}}</p>
    <p>Time: {{startTime}}</p>
    <p>Join URL: <a href="{{meetingUrl}}">{{meetingUrl}}</a></p>
    <p>Organizer: {{organizerName}}</p>
</body>
</html>
```

## 配置文件

### 默认配置
系统默认配置文件：

```json
{
    "meeting": {
        "defaultServer": "meet.jit.si",
        "autoJoin": false,
        "displayName": "",
        "audioEnabled": true,
        "videoEnabled": true,
        "theme": "default",
        "language": "en",
        "notifications": {
            "sound": true,
            "desktop": true
        }
    }
}
```

## 资源管理

### 资源文件 (meeting_resources.qrc)

```xml
<RCC>
    <qresource prefix="/meeting">
        <!-- 图标 -->
        <file>icons/meeting.png</file>
        <file>icons/join.png</file>
        <file>icons/create.png</file>
        
        <!-- 样式 -->
        <file>styles/meeting.qss</file>
        <file>styles/dark-theme.qss</file>
        
        <!-- 音效 -->
        <file>sounds/join.wav</file>
        <file>sounds/leave.wav</file>
        
        <!-- 模板 -->
        <file>templates/meeting-templates.json</file>
        <file>templates/invitation-templates/default.html</file>
        
        <!-- 配置 -->
        <file>data/default-config.json</file>
    </qresource>
</RCC>
```

### 使用资源

```cpp
// 加载图标
QIcon joinIcon(":/meeting/icons/join.png");

// 加载样式表
QFile styleFile(":/meeting/styles/meeting.qss");
styleFile.open(QFile::ReadOnly);
QString style = styleFile.readAll();
widget->setStyleSheet(style);

// 加载配置
QFile configFile(":/meeting/data/default-config.json");
configFile.open(QFile::ReadOnly);
QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
```

## 维护指南

### 添加新资源
1. 将文件放入相应目录
2. 更新 meeting_resources.qrc
3. 重新编译资源文件
4. 更新文档

### 更新翻译
1. 使用 lupdate 提取新字符串
2. 翻译 .ts 文件
3. 使用 lrelease 生成 .qm 文件
4. 测试多语言功能

### 优化资源
1. 压缩图片文件
2. 清理未使用的资源
3. 合并相似的样式
4. 定期审查资源使用情况