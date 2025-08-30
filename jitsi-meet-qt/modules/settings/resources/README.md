# Settings Module Resources

## 概述

本目录包含 Settings Module 的资源文件，包括图标、样式表、配置模板等。

## 资源结构

```
resources/
├── README.md                   # 资源说明文档
├── settings_resources.qrc      # Qt 资源文件
├── icons/                      # 图标资源
│   ├── settings.png
│   ├── preferences.png
│   ├── config.png
│   ├── validation.png
│   └── storage.png
├── styles/                     # 样式表
│   ├── default.qss
│   ├── dark.qss
│   └── light.qss
├── templates/                  # 配置模板
│   ├── default_settings.json
│   ├── audio_config.json
│   ├── video_config.json
│   └── network_config.json
├── schemas/                    # JSON Schema 文件
│   ├── settings_schema.json
│   ├── preferences_schema.json
│   └── validation_rules.json
└── translations/               # 翻译文件
    ├── settings_en.ts
    ├── settings_zh.ts
    └── settings_ja.ts
```

## 图标资源

- `settings.png` - 设置图标
- `preferences.png` - 偏好图标
- `config.png` - 配置图标
- `validation.png` - 验证图标
- `storage.png` - 存储图标

## 样式表

- `default.qss` - 默认样式
- `dark.qss` - 暗色主题
- `light.qss` - 亮色主题

## 配置模板

提供各种配置的默认模板，可用于初始化和重置功能。

## JSON Schema

定义配置文件的结构和验证规则。

## 国际化

支持多语言翻译文件。

## 使用方法

资源文件通过 Qt 资源系统加载：

```cpp
// 加载图标
QIcon icon(":/icons/settings.png");

// 加载样式表
QFile styleFile(":/styles/dark.qss");
styleFile.open(QFile::ReadOnly);
QString style = styleFile.readAll();
```