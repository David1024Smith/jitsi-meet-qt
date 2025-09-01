# Settings Module

## 概述

Settings Module 是 Jitsi Meet Qt 模块化架构中的设置管理模块，负责应用程序的配置管理、用户偏好设置和设置持久化存储。

## 版本信息

- **版本**: 1.0.0
- **状态**: 开发中
- **依赖**: Qt Core, Qt Widgets, Qt Network

## 功能特性

### 核心功能
- 统一的设置管理接口
- 多种存储后端支持（本地文件、云端同步、系统注册表）
- 配置验证和模式验证
- 设置项的类型安全访问
- 默认值管理和回退机制

### 存储后端
- **LocalStorage**: 本地文件存储（JSON/INI格式）
- **CloudStorage**: 云端同步存储
- **RegistryStorage**: 系统注册表存储（Windows）

### 验证系统
- **ConfigValidator**: 配置项验证器
- **SchemaValidator**: JSON模式验证器

### UI组件
- **SettingsWidget**: 设置管理组件
- **PreferencesDialog**: 偏好设置对话框
- **ConfigEditor**: 配置编辑器

## 架构设计

```
Settings Module
├── Core Layer (SettingsModule, SettingsManager, PreferencesHandler)
├── Interface Layer (ISettingsManager, IPreferencesHandler, IConfigValidator)
├── Storage Layer (LocalStorage, CloudStorage, RegistryStorage)
├── Validation Layer (ConfigValidator, SchemaValidator)
├── Configuration Layer (SettingsConfig)
└── UI Layer (SettingsWidget, PreferencesDialog, ConfigEditor)
```

## 目录结构

```
modules/settings/
├── settings.pri                # 模块配置文件
├── README.md                   # 模块文档
├── include/                    # 核心头文件
│   ├── SettingsModule.h        # 设置模块核心
│   ├── SettingsManager.h       # 设置管理器
│   └── PreferencesHandler.h    # 偏好处理器
├── src/                        # 核心实现
├── interfaces/                 # 接口定义
│   ├── ISettingsManager.h      # 设置管理器接口
│   ├── IPreferencesHandler.h   # 偏好处理接口
│   └── IConfigValidator.h      # 配置验证接口
├── config/                     # 配置管理
│   ├── SettingsConfig.h        # 设置配置类
│   └── SettingsConfig.cpp
├── storage/                    # 存储后端
│   ├── LocalStorage.h          # 本地存储
│   ├── CloudStorage.h          # 云端存储
│   └── RegistryStorage.h       # 注册表存储
├── validators/                 # 验证器
│   ├── ConfigValidator.h       # 配置验证器
│   └── SchemaValidator.h       # 模式验证器
├── widgets/                    # UI组件
│   ├── SettingsWidget.h        # 设置组件
│   ├── PreferencesDialog.h     # 偏好对话框
│   └── ConfigEditor.h          # 配置编辑器
├── tests/                      # 测试框架
├── examples/                   # 示例代码
└── resources/                  # 资源文件
```

## 使用示例

### 基本设置管理

```cpp
#include "SettingsManager.h"
#include "SettingsConfig.h"

// 初始化设置管理器
auto* settingsManager = SettingsManager::instance();
settingsManager->initialize();

// 设置配置值
settingsManager->setValue("audio/volume", 0.8);
settingsManager->setValue("video/resolution", QSize(1920, 1080));
settingsManager->setValue("ui/theme", "dark");

// 获取配置值
double volume = settingsManager->value("audio/volume", 0.5).toDouble();
QSize resolution = settingsManager->value("video/resolution", QSize(1280, 720)).toSize();
QString theme = settingsManager->value("ui/theme", "light").toString();

// 保存设置
settingsManager->sync();
```

### 使用不同存储后端

```cpp
#include "LocalStorage.h"
#include "CloudStorage.h"

// 使用本地存储
auto* localStorage = new LocalStorage("config.json");
settingsManager->setStorageBackend(localStorage);

// 使用云端存储
auto* cloudStorage = new CloudStorage("https://api.example.com/settings");
settingsManager->setStorageBackend(cloudStorage);
```

### 配置验证

```cpp
#include "ConfigValidator.h"
#include "SchemaValidator.h"

// 创建验证器
auto* validator = new ConfigValidator();
validator->addRule("audio/volume", ConfigValidator::Range, 0.0, 1.0);
validator->addRule("video/fps", ConfigValidator::Range, 15, 60);

// 设置验证器
settingsManager->setValidator(validator);

// 验证配置
if (!settingsManager->validate()) {
    qWarning() << "Configuration validation failed";
}
```

## API 参考

### ISettingsManager 接口

```cpp
class ISettingsManager {
public:
    virtual bool initialize() = 0;
    virtual void setValue(const QString& key, const QVariant& value) = 0;
    virtual QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const = 0;
    virtual bool contains(const QString& key) const = 0;
    virtual void remove(const QString& key) = 0;
    virtual void sync() = 0;
    virtual bool validate() const = 0;
};
```

### IPreferencesHandler 接口

```cpp
class IPreferencesHandler {
public:
    virtual void setPreference(const QString& category, const QString& key, const QVariant& value) = 0;
    virtual QVariant preference(const QString& category, const QString& key, const QVariant& defaultValue = QVariant()) const = 0;
    virtual QStringList categories() const = 0;
    virtual QStringList keys(const QString& category) const = 0;
    virtual void resetCategory(const QString& category) = 0;
    virtual void resetAll() = 0;
};
```

## 配置选项

### 编译选项

- `SETTINGS_DEBUG_MODE`: 启用调试模式
- `SETTINGS_WINDOWS_REGISTRY`: 启用Windows注册表支持
- `SETTINGS_LINUX_CONFIG`: 启用Linux配置支持
- `SETTINGS_MACOS_PLIST`: 启用macOS plist支持

### 运行时配置

```cpp
// 设置存储路径
settingsManager->setStoragePath("/path/to/config");

// 设置自动同步
settingsManager->setAutoSync(true);

// 设置加密
settingsManager->setEncryption(true);
```

## 测试

Settings Module 包含全面的测试框架，覆盖模块的所有方面：

### 测试覆盖范围

1. **核心模块测试** (`SettingsModuleTest`)
   - 模块初始化和生命周期管理
   - 设置管理器操作
   - 偏好处理器功能
   - 跨组件集成
   - 错误处理和恢复
   - 性能基准测试
   - 遗留系统兼容性

2. **存储后端测试** (`StorageBackendTest`)
   - 本地文件存储操作和持久化
   - 云端存储离线/在线模式和冲突解决
   - 注册表存储（Windows）和权限处理
   - 跨后端数据迁移
   - 性能基准测试

3. **UI组件测试** (`UIComponentTest`)
   - 设置组件初始化和值管理
   - 偏好对话框类别和偏好管理
   - 配置编辑器编辑和验证
   - 用户交互处理
   - 跨组件数据一致性

4. **验证测试** (`ValidationTest`)
   - 基础验证规则（范围、模式、类型）
   - 自定义验证函数
   - 条件验证逻辑
   - JSON模式验证
   - 默认值处理
   - 错误报告和恢复

### 运行测试

#### 快速开始
```bash
cd tests
# Windows
run_all_tests.bat

# Linux/macOS
./run_all_tests.sh
```

#### 使用 qmake
```bash
cd tests
qmake settings_tests.pro
make
./settings_tests
```

#### 使用 CMake
```bash
cd tests
mkdir build && cd build
cmake ..
make
ctest
```

#### 单独的测试套件
```bash
# 运行特定测试类别
./settings_module_tests      # 核心功能
./storage_backend_tests      # 存储后端
./ui_component_tests         # UI组件
./validation_tests           # 验证逻辑
```

### 测试需求

测试框架满足需求 7.6，全面覆盖：

- ✅ 设置存储和同步测试
- ✅ 配置验证和默认值测试
- ✅ 设置UI组件交互测试
- ✅ 与现有ConfigurationManager的兼容性测试

### 性能基准

测试包含性能基准，确保：
- 设置操作：1000次写入 < 2秒，1000次读取 < 1秒
- 存储同步：完整同步 < 3秒
- 验证：100个规则的100次验证 < 5秒
- UI操作：1000次组件更新 < 3秒

### 测试文档

详细信息请参见 [TEST_DOCUMENTATION.md](tests/TEST_DOCUMENTATION.md)：
- 测试架构和设计
- 单个测试用例描述
- 性能基准和要求
- 错误处理和边缘情况测试
- 集成测试策略
- 故障排除和调试指南

## 依赖关系

- Qt Core (必需)
- Qt Widgets (UI组件)
- Qt Network (云端同步)
- Utils Module (日志和工具)

## 许可证

本模块遵循 Jitsi Meet Qt 项目的许可证。

## 贡献

请参考项目根目录的 CONTRIBUTING.md 文件。

## 更新日志

### v1.0.0 (开发中)
- 初始版本
- 基础设置管理功能
- 多存储后端支持
- 配置验证系统
- UI组件实现