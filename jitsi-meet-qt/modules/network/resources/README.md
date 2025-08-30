# 网络模块资源文件

## 概述

本目录包含网络模块使用的各种资源文件，包括图标、样式表、配置模板等。

## 目录结构

```
resources/
├── README.md                   # 本文档
├── icons/                      # 图标资源
│   ├── network_connected.png   # 网络已连接图标
│   ├── network_disconnected.png # 网络断开图标
│   ├── network_connecting.png  # 网络连接中图标
│   ├── network_error.png       # 网络错误图标
│   ├── signal_strength_0.png   # 信号强度0
│   ├── signal_strength_1.png   # 信号强度1
│   ├── signal_strength_2.png   # 信号强度2
│   ├── signal_strength_3.png   # 信号强度3
│   └── signal_strength_4.png   # 信号强度4
├── styles/                     # 样式表
│   ├── network_widget.qss      # 网络组件样式
│   ├── connection_dialog.qss   # 连接对话框样式
│   └── status_indicator.qss    # 状态指示器样式
├── configs/                    # 配置模板
│   ├── default_config.json     # 默认配置模板
│   ├── production_config.json  # 生产环境配置
│   ├── development_config.json # 开发环境配置
│   └── test_config.json        # 测试环境配置
├── translations/               # 翻译文件
│   ├── network_zh_CN.ts        # 中文翻译
│   ├── network_en_US.ts        # 英文翻译
│   └── network_ja_JP.ts        # 日文翻译
└── network_resources.qrc       # Qt资源文件
```

## 图标资源

### 网络状态图标

- **network_connected.png**: 16x16, 24x24, 32x32 像素的网络已连接图标
- **network_disconnected.png**: 网络断开状态图标
- **network_connecting.png**: 网络连接中的动画图标
- **network_error.png**: 网络错误状态图标

### 信号强度图标

- **signal_strength_0.png**: 无信号
- **signal_strength_1.png**: 弱信号
- **signal_strength_2.png**: 一般信号
- **signal_strength_3.png**: 良好信号
- **signal_strength_4.png**: 优秀信号

## 样式表

### network_widget.qss

网络组件的通用样式：

```css
/* 网络状态组件样式 */
NetworkStatusWidget {
    background-color: #f0f0f0;
    border: 1px solid #d0d0d0;
    border-radius: 4px;
    padding: 4px;
}

NetworkStatusWidget[connected="true"] {
    background-color: #e8f5e8;
    border-color: #4caf50;
}

NetworkStatusWidget[connected="false"] {
    background-color: #ffeaea;
    border-color: #f44336;
}

/* 状态标签样式 */
QLabel#statusLabel {
    font-weight: bold;
    color: #333333;
}

QLabel#statusLabel[status="connected"] {
    color: #4caf50;
}

QLabel#statusLabel[status="disconnected"] {
    color: #f44336;
}

QLabel#statusLabel[status="connecting"] {
    color: #ff9800;
}
```

### connection_dialog.qss

连接对话框样式：

```css
/* 连接对话框样式 */
ConnectionWidget {
    background-color: white;
    padding: 16px;
}

QGroupBox {
    font-weight: bold;
    border: 2px solid #cccccc;
    border-radius: 5px;
    margin-top: 1ex;
    padding-top: 10px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 5px 0 5px;
}

/* 输入框样式 */
QLineEdit {
    border: 1px solid #d0d0d0;
    border-radius: 4px;
    padding: 6px;
    font-size: 12px;
}

QLineEdit:focus {
    border-color: #2196f3;
}

/* 按钮样式 */
QPushButton {
    background-color: #2196f3;
    color: white;
    border: none;
    border-radius: 4px;
    padding: 8px 16px;
    font-weight: bold;
}

QPushButton:hover {
    background-color: #1976d2;
}

QPushButton:pressed {
    background-color: #0d47a1;
}

QPushButton:disabled {
    background-color: #cccccc;
    color: #666666;
}
```

## 配置模板

### default_config.json

默认网络配置：

```json
{
  "module": {
    "name": "NetworkModule",
    "version": "1.0.0",
    "enabled": true
  },
  "server": {
    "url": "https://meet.jit.si",
    "port": 443,
    "domain": "meet.jit.si"
  },
  "connection": {
    "timeout": 30000,
    "autoReconnect": true,
    "reconnectInterval": 5000,
    "maxReconnectAttempts": 3
  },
  "protocols": {
    "webrtc": {
      "enabled": true,
      "stunServers": [
        "stun:stun.l.google.com:19302",
        "stun:stun1.l.google.com:19302"
      ],
      "turnServers": []
    },
    "websocket": {
      "enabled": true,
      "heartbeatInterval": 30000
    },
    "http": {
      "enabled": true,
      "httpsOnly": true,
      "requestTimeout": 15000
    }
  },
  "performance": {
    "qualityLevel": "auto",
    "bandwidthLimit": 0,
    "compressionEnabled": true
  },
  "ui": {
    "showNetworkStatus": true,
    "autoUpdate": true,
    "updateInterval": 1000
  }
}
```

### production_config.json

生产环境配置：

```json
{
  "module": {
    "name": "NetworkModule",
    "version": "1.0.0",
    "enabled": true
  },
  "server": {
    "url": "https://your-production-server.com",
    "port": 443,
    "domain": "your-production-server.com"
  },
  "connection": {
    "timeout": 60000,
    "autoReconnect": true,
    "reconnectInterval": 10000,
    "maxReconnectAttempts": 5
  },
  "protocols": {
    "webrtc": {
      "enabled": true,
      "stunServers": [
        "stun:your-stun-server.com:3478"
      ],
      "turnServers": [
        "turn:your-turn-server.com:3478"
      ]
    },
    "websocket": {
      "enabled": true,
      "heartbeatInterval": 60000
    },
    "http": {
      "enabled": true,
      "httpsOnly": true,
      "requestTimeout": 30000
    }
  },
  "performance": {
    "qualityLevel": "high",
    "bandwidthLimit": 0,
    "compressionEnabled": true
  },
  "logging": {
    "level": "warning",
    "enableFileLogging": true,
    "maxLogFileSize": 10485760
  }
}
```

## Qt资源文件

### network_resources.qrc

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource prefix="/network">
        <!-- 图标资源 -->
        <file>icons/network_connected.png</file>
        <file>icons/network_disconnected.png</file>
        <file>icons/network_connecting.png</file>
        <file>icons/network_error.png</file>
        <file>icons/signal_strength_0.png</file>
        <file>icons/signal_strength_1.png</file>
        <file>icons/signal_strength_2.png</file>
        <file>icons/signal_strength_3.png</file>
        <file>icons/signal_strength_4.png</file>
        
        <!-- 样式表 -->
        <file>styles/network_widget.qss</file>
        <file>styles/connection_dialog.qss</file>
        <file>styles/status_indicator.qss</file>
        
        <!-- 配置模板 -->
        <file>configs/default_config.json</file>
        <file>configs/production_config.json</file>
        <file>configs/development_config.json</file>
        <file>configs/test_config.json</file>
        
        <!-- 翻译文件 -->
        <file>translations/network_zh_CN.qm</file>
        <file>translations/network_en_US.qm</file>
        <file>translations/network_ja_JP.qm</file>
    </qresource>
</RCC>
```

## 使用方法

### 在代码中使用资源

```cpp
// 加载图标
QIcon connectedIcon(":/network/icons/network_connected.png");
QIcon disconnectedIcon(":/network/icons/network_disconnected.png");

// 加载样式表
QFile styleFile(":/network/styles/network_widget.qss");
if (styleFile.open(QIODevice::ReadOnly)) {
    QString style = styleFile.readAll();
    widget->setStyleSheet(style);
}

// 加载配置模板
QFile configFile(":/network/configs/default_config.json");
if (configFile.open(QIODevice::ReadOnly)) {
    QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
    QVariantMap config = doc.object().toVariantMap();
}
```

### 在QML中使用资源

```qml
import QtQuick 2.15

Image {
    source: "qrc:/network/icons/network_connected.png"
    width: 24
    height: 24
}
```

## 翻译支持

### 添加新语言

1. 创建新的.ts文件：`lupdate network.pro`
2. 使用Qt Linguist翻译文本
3. 生成.qm文件：`lrelease network_zh_CN.ts`
4. 更新资源文件

### 在代码中使用翻译

```cpp
// 加载翻译
QTranslator translator;
if (translator.load(":/network/translations/network_zh_CN.qm")) {
    QApplication::installTranslator(&translator);
}

// 使用翻译文本
QString text = tr("Network Connected");
```

## 自定义资源

### 添加新图标

1. 将图标文件放入`icons/`目录
2. 更新`network_resources.qrc`文件
3. 重新编译项目

### 修改样式

1. 编辑相应的.qss文件
2. 在应用程序中重新加载样式表

### 更新配置模板

1. 修改相应的.json配置文件
2. 确保配置格式正确
3. 更新相关文档

## 注意事项

- 图标建议使用PNG格式，支持透明背景
- 样式表使用标准CSS语法
- 配置文件必须是有效的JSON格式
- 翻译文件需要使用Qt Linguist工具处理
- 资源文件会被编译到可执行文件中，注意文件大小