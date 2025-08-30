# UI Module

## 概述

UI模块负责管理Jitsi Meet Qt应用程序的用户界面组件、主题系统和布局管理。该模块提供了统一的UI管理接口，支持多主题切换、响应式布局和可重用的UI组件库。

## 版本信息

- **版本**: 1.0.0
- **兼容性**: Qt 5.15+
- **依赖模块**: Utils Module (可选)

## 功能特性

### 核心功能
- 统一的UI管理系统
- 主题管理和切换
- 布局管理和响应式设计
- 可重用的UI组件库
- 界面配置和偏好设置

### 主题系统
- 默认主题 (Default Theme)
- 暗色主题 (Dark Theme)  
- 亮色主题 (Light Theme)
- 自定义主题支持
- 动态主题切换

### 布局管理
- 主布局 (MainLayout)
- 会议布局 (ConferenceLayout)
- 设置布局 (SettingsLayout)
- 响应式设计支持

### UI组件库
- 基础组件 (BaseWidget)
- 自定义按钮 (CustomButton)
- 状态栏 (StatusBar)
- 工具栏 (ToolBar)

## 目录结构

```
modules/ui/
├── ui.pri                      # 模块配置文件
├── README.md                   # 模块文档
├── include/                    # 核心头文件
│   ├── UIModule.h              # UI模块核心
│   ├── UIManager.h             # UI管理器
│   └── ThemeFactory.h          # 主题工厂
├── src/                        # 核心实现
├── interfaces/                 # 接口定义
│   ├── IUIManager.h            # UI管理器接口
│   ├── IThemeManager.h         # 主题管理器接口
│   └── ILayoutManager.h        # 布局管理器接口
├── config/                     # 配置管理
│   ├── UIConfig.h              # UI配置类
│   └── UIConfig.cpp
├── themes/                     # 主题管理
│   ├── DefaultTheme.h          # 默认主题
│   ├── DarkTheme.h             # 暗色主题
│   └── LightTheme.h            # 亮色主题
├── widgets/                    # UI组件库
│   ├── BaseWidget.h            # 基础组件
│   ├── CustomButton.h          # 自定义按钮
│   ├── StatusBar.h             # 状态栏
│   └── ToolBar.h               # 工具栏
├── layouts/                    # 布局管理
│   ├── MainLayout.h            # 主布局
│   ├── ConferenceLayout.h      # 会议布局
│   └── SettingsLayout.h        # 设置布局
├── tests/                      # 测试框架
├── examples/                   # 示例代码
└── resources/                  # 资源文件
    ├── themes/                 # 主题资源
    ├── icons/                  # 图标资源
    └── styles/                 # 样式表
```

## 快速开始

### 基本使用

```cpp
#include "UIManager.h"
#include "UIConfig.h"

// 创建UI管理器
auto uiManager = UIManager::instance();

// 初始化UI模块
if (!uiManager->initialize()) {
    qWarning() << "Failed to initialize UI module";
    return false;
}

// 设置主题
uiManager->setTheme("dark");

// 应用配置
UIConfig config;
config.setTheme("dark");
config.setLanguage("zh_CN");
uiManager->applyConfiguration(config);
```

### 主题切换

```cpp
#include "ThemeFactory.h"

// 获取主题工厂
auto themeFactory = ThemeFactory::instance();

// 切换到暗色主题
auto darkTheme = themeFactory->createTheme("dark");
uiManager->applyTheme(darkTheme);

// 切换到亮色主题
auto lightTheme = themeFactory->createTheme("light");
uiManager->applyTheme(lightTheme);
```

### 自定义组件

```cpp
#include "BaseWidget.h"
#include "CustomButton.h"

// 创建自定义按钮
auto button = new CustomButton("Click Me", this);
button->setStyle(CustomButton::PrimaryStyle);

// 连接信号
connect(button, &CustomButton::clicked, this, &MyWidget::onButtonClicked);
```

## API 参考

### 核心接口

#### IUIManager
- `initialize()` - 初始化UI管理器
- `setTheme(themeName)` - 设置主题
- `getTheme()` - 获取当前主题
- `applyConfiguration(config)` - 应用配置

#### IThemeManager  
- `loadTheme(themeName)` - 加载主题
- `applyTheme(theme)` - 应用主题
- `getAvailableThemes()` - 获取可用主题列表

#### ILayoutManager
- `setLayout(layoutName)` - 设置布局
- `getLayout()` - 获取当前布局
- `updateLayout()` - 更新布局

### 配置选项

#### UIConfig
- `theme` - 主题名称 (default/dark/light)
- `language` - 界面语言
- `fontSize` - 字体大小
- `windowState` - 窗口状态
- `customStyles` - 自定义样式

## 测试

运行UI模块测试：

```bash
# Linux/macOS
cd tests && ./run_tests.sh

# Windows
cd tests && run_tests.bat
```

## 开发指南

### 添加新主题

1. 继承 `BaseTheme` 类
2. 实现主题样式方法
3. 在 `ThemeFactory` 中注册主题
4. 添加主题资源文件

### 创建自定义组件

1. 继承 `BaseWidget` 类
2. 实现组件功能
3. 添加样式支持
4. 编写单元测试

### 布局管理

1. 继承 `BaseLayout` 类
2. 实现布局逻辑
3. 支持响应式设计
4. 测试不同屏幕尺寸

## 故障排除

### 常见问题

1. **主题加载失败**
   - 检查主题文件是否存在
   - 验证主题配置格式
   - 查看错误日志

2. **组件显示异常**
   - 检查样式表语法
   - 验证组件层次结构
   - 确认Qt版本兼容性

3. **布局问题**
   - 检查布局约束
   - 验证尺寸策略
   - 测试窗口调整

## 许可证

本模块遵循项目主许可证。

## 贡献

欢迎提交问题报告和功能请求。请遵循项目的贡献指南。