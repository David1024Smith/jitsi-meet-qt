# Performance Module Resources

## 概述

本目录包含性能模块使用的各种资源文件，包括图标、样式表、配置模板、报告模板和帮助文档。

## 资源结构

### 图标资源 (`icons/`)
性能模块界面使用的图标文件：

#### 功能图标
- `performance.png` - 性能模块主图标
- `monitoring.png` - 监控功能图标
- `optimization.png` - 优化功能图标
- `chart.png` - 图表显示图标
- `settings.png` - 设置配置图标

#### 监控图标
- `cpu.png` - CPU监控图标
- `memory.png` - 内存监控图标
- `network.png` - 网络监控图标

#### 控制图标
- `start.png` - 开始监控图标
- `stop.png` - 停止监控图标
- `pause.png` - 暂停监控图标
- `resume.png` - 恢复监控图标

#### 操作图标
- `export.png` - 数据导出图标
- `import.png` - 数据导入图标

#### 状态图标
- `success.png` - 成功状态图标
- `warning.png` - 警告状态图标
- `error.png` - 错误状态图标
- `info.png` - 信息提示图标

### 样式表资源 (`styles/`)
界面组件的样式定义：

#### 组件样式
- `performance_widget.qss` - 性能主界面样式
- `metrics_chart.qss` - 指标图表样式
- `monitor_widget.qss` - 监控组件样式

#### 主题样式
- `dark_theme.qss` - 暗色主题样式
- `light_theme.qss` - 亮色主题样式

### 配置模板 (`templates/`)
预定义的配置文件模板：

#### 配置模板
- `default_config.json` - 默认配置模板
- `monitoring_config.json` - 监控配置模板
- `optimization_config.json` - 优化配置模板

#### 报告模板
- `html_report_template.html` - HTML报告模板
- `csv_export_template.csv` - CSV导出模板

### 帮助文档 (`help/`)
用户帮助和指导文档：

- `performance_help.html` - 性能模块帮助文档
- `monitoring_guide.html` - 监控功能指南
- `optimization_guide.html` - 优化功能指南

### 示例数据 (`examples/`)
示例和测试数据：

- `sample_metrics.json` - 示例性能指标数据
- `sample_config.json` - 示例配置数据

## 资源使用

### 在代码中使用资源

```cpp
#include <QIcon>
#include <QPixmap>
#include <QFile>

// 加载图标
QIcon cpuIcon(":/performance/icons/cpu.png");
QIcon memoryIcon(":/performance/icons/memory.png");

// 加载样式表
QFile styleFile(":/performance/styles/performance_widget.qss");
if (styleFile.open(QFile::ReadOnly)) {
    QString styleSheet = styleFile.readAll();
    widget->setStyleSheet(styleSheet);
}

// 加载配置模板
QFile configFile(":/performance/templates/default_config.json");
if (configFile.open(QFile::ReadOnly)) {
    QByteArray configData = configFile.readAll();
    // 解析配置数据
}
```

### 在QML中使用资源

```qml
import QtQuick 2.15

Rectangle {
    Image {
        source: "qrc:/performance/icons/performance.png"
        width: 32
        height: 32
    }
}
```

## 图标规范

### 尺寸规范
- 小图标: 16x16 像素
- 标准图标: 24x24 像素
- 大图标: 32x32 像素
- 高分辨率: 48x48 像素

### 格式要求
- 格式: PNG (支持透明度)
- 颜色深度: 32位 RGBA
- 背景: 透明
- 压缩: 优化压缩

### 设计原则
- 简洁明了
- 风格统一
- 高对比度
- 可缩放性

## 样式表规范

### 性能组件样式示例

```css
/* performance_widget.qss */
QWidget#PerformanceWidget {
    background-color: #f0f0f0;
    border: 1px solid #d0d0d0;
    border-radius: 4px;
}

QLabel#MetricLabel {
    font-size: 12px;
    font-weight: bold;
    color: #333333;
}

QProgressBar#CPUProgressBar {
    border: 1px solid #cccccc;
    border-radius: 3px;
    text-align: center;
}

QProgressBar#CPUProgressBar::chunk {
    background-color: #4CAF50;
    border-radius: 2px;
}

QPushButton#OptimizeButton {
    background-color: #2196F3;
    color: white;
    border: none;
    border-radius: 4px;
    padding: 8px 16px;
    font-weight: bold;
}

QPushButton#OptimizeButton:hover {
    background-color: #1976D2;
}

QPushButton#OptimizeButton:pressed {
    background-color: #0D47A1;
}
```

### 图表样式示例

```css
/* metrics_chart.qss */
QChartView {
    background-color: white;
    border: 1px solid #e0e0e0;
}

QChart {
    background-color: transparent;
    plot-area-background-color: #fafafa;
}

/* 深色主题 */
QWidget[theme="dark"] {
    background-color: #2b2b2b;
    color: #ffffff;
}

QWidget[theme="dark"] QChartView {
    background-color: #3c3c3c;
    border: 1px solid #555555;
}
```

## 配置模板

### 默认配置模板

```json
{
    "monitoring": {
        "enabled": true,
        "interval": 1000,
        "enabledMonitors": [
            "CPUMonitor",
            "MemoryMonitor",
            "NetworkMonitor"
        ]
    },
    "optimization": {
        "autoEnabled": false,
        "interval": 30000,
        "strategy": "Balanced",
        "enabledOptimizers": [
            "StartupOptimizer",
            "MemoryOptimizer"
        ]
    },
    "thresholds": {
        "cpu": 80.0,
        "memory": 1024,
        "networkLatency": 100.0,
        "frameRate": 30.0
    },
    "storage": {
        "retentionHours": 24,
        "maxSize": 100,
        "path": "./performance_data"
    },
    "ui": {
        "theme": "light",
        "updateInterval": 1000,
        "displayedMetrics": [
            "cpu",
            "memory",
            "network"
        ]
    }
}
```

### 监控配置模板

```json
{
    "monitors": {
        "CPUMonitor": {
            "enabled": true,
            "mode": "DetailedMode",
            "interval": 1000,
            "overheatThreshold": 85.0
        },
        "MemoryMonitor": {
            "enabled": true,
            "mode": "SystemMode",
            "interval": 1000,
            "leakDetectionThreshold": 10.0
        },
        "NetworkMonitor": {
            "enabled": true,
            "mode": "QualityMode",
            "interval": 5000,
            "testHosts": [
                "8.8.8.8",
                "1.1.1.1"
            ]
        }
    }
}
```

## 报告模板

### HTML报告模板

```html
<!DOCTYPE html>
<html>
<head>
    <title>Performance Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .header { background-color: #f5f5f5; padding: 20px; border-radius: 5px; }
        .metric { margin: 10px 0; padding: 10px; border-left: 4px solid #2196F3; }
        .chart { width: 100%; height: 300px; margin: 20px 0; }
        .summary { background-color: #e8f5e8; padding: 15px; border-radius: 5px; }
    </style>
</head>
<body>
    <div class="header">
        <h1>Performance Report</h1>
        <p>Generated: {{timestamp}}</p>
        <p>Period: {{period}}</p>
    </div>
    
    <div class="summary">
        <h2>Summary</h2>
        <p>Average CPU Usage: {{avgCpu}}%</p>
        <p>Average Memory Usage: {{avgMemory}}MB</p>
        <p>Average Network Latency: {{avgLatency}}ms</p>
    </div>
    
    <div class="metrics">
        <h2>Detailed Metrics</h2>
        {{metricsTable}}
    </div>
    
    <div class="charts">
        <h2>Performance Charts</h2>
        {{performanceCharts}}
    </div>
</body>
</html>
```

## 帮助文档

### 性能模块帮助

帮助文档采用HTML格式，包含：

1. **功能概述** - 模块主要功能介绍
2. **快速开始** - 基本使用步骤
3. **详细功能** - 各功能详细说明
4. **配置指南** - 配置参数说明
5. **故障排除** - 常见问题解决
6. **API参考** - 编程接口文档

### 文档结构

```html
<!DOCTYPE html>
<html>
<head>
    <title>Performance Module Help</title>
    <link rel="stylesheet" href="help_styles.css">
</head>
<body>
    <nav class="sidebar">
        <ul>
            <li><a href="#overview">概述</a></li>
            <li><a href="#quickstart">快速开始</a></li>
            <li><a href="#monitoring">性能监控</a></li>
            <li><a href="#optimization">性能优化</a></li>
            <li><a href="#configuration">配置管理</a></li>
            <li><a href="#troubleshooting">故障排除</a></li>
        </ul>
    </nav>
    
    <main class="content">
        <section id="overview">
            <h1>性能模块概述</h1>
            <!-- 内容 -->
        </section>
        
        <section id="quickstart">
            <h1>快速开始</h1>
            <!-- 内容 -->
        </section>
        
        <!-- 其他章节 -->
    </main>
</body>
</html>
```

## 资源管理

### 资源优化
- 图标压缩优化
- 样式表最小化
- 模板文件压缩
- 按需加载资源

### 版本管理
- 资源版本控制
- 向后兼容性
- 增量更新支持
- 缓存策略

### 本地化支持
- 多语言图标
- 本地化样式
- 区域化配置
- 文化适应性

## 自定义资源

### 添加自定义图标

1. 创建符合规范的PNG图标
2. 添加到`icons/`目录
3. 更新`performance_resources.qrc`
4. 重新编译资源

```xml
<file>icons/custom_icon.png</file>
```

### 创建自定义样式

1. 创建CSS样式文件
2. 添加到`styles/`目录
3. 更新资源文件
4. 在代码中应用样式

### 扩展配置模板

1. 创建JSON配置文件
2. 定义配置结构
3. 添加验证规则
4. 提供使用示例

## 许可证

所有资源文件遵循项目主许可证，部分第三方资源可能有独立许可证声明。