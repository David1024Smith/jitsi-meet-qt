# Utils Module

通用工具模块，为Jitsi Meet Qt应用程序提供可重用的工具类和实用函数。

## 版本信息

- **版本**: 1.0.0
- **Qt兼容性**: Qt 5.12+
- **平台支持**: Windows, Linux, macOS

## 模块概述

Utils模块提供以下核心功能：

### 🔧 核心组件
- **UtilsModule**: 工具模块核心管理器
- **UtilsConfig**: 统一配置管理系统
- **UtilsSingletonManager**: 单例生命周期管理器
- **UtilsErrorHandler**: 错误处理和恢复系统
- **Logger**: 统一日志记录系统
- **FileManager**: 文件管理器

### 📝 日志系统
- **FileLogger**: 文件日志记录器
- **ConsoleLogger**: 控制台日志记录器
- **NetworkLogger**: 网络日志记录器

### 🔐 加密工具
- **AESCrypto**: AES加密/解密
- **RSACrypto**: RSA加密/解密
- **HashUtils**: 哈希工具（MD5, SHA256等）

### 📁 文件工具
- **ConfigFile**: 配置文件处理
- **TempFile**: 临时文件管理
- **FileWatcher**: 文件监控

### 🔤 字符串工具
- **StringUtils**: 字符串处理工具
- **Validator**: 数据验证器

## 目录结构

```
modules/utils/
├── utils.pri                   # 模块配置文件
├── README.md                   # 模块文档
├── include/                    # 核心头文件
│   ├── UtilsModule.h          # 工具模块核心
│   ├── UtilsConfig.h          # 统一配置管理
│   ├── UtilsSingletonManager.h # 单例管理器
│   ├── UtilsErrorHandler.h    # 错误处理器
│   ├── Logger.h               # 日志记录器
│   └── FileManager.h          # 文件管理器
├── src/                       # 核心实现
├── config/                    # 配置管理
│   ├── UtilsConfig.h          # 配置管理类
│   └── UtilsConfig.cpp
├── interfaces/                # 接口定义
│   ├── ILogger.h              # 日志接口
│   ├── IFileHandler.h         # 文件处理接口
│   └── ICryptoHandler.h       # 加密处理接口
├── logging/                   # 日志系统
│   ├── FileLogger.h           # 文件日志
│   ├── ConsoleLogger.h        # 控制台日志
│   └── NetworkLogger.h        # 网络日志
├── crypto/                    # 加密工具
│   ├── AESCrypto.h            # AES加密
│   ├── RSACrypto.h            # RSA加密
│   └── HashUtils.h            # 哈希工具
├── file/                      # 文件工具
│   ├── ConfigFile.h           # 配置文件
│   ├── TempFile.h             # 临时文件
│   └── FileWatcher.h          # 文件监控
├── string/                    # 字符串工具
│   ├── StringUtils.h          # 字符串工具
│   └── Validator.h            # 验证器
├── tests/                     # 测试框架
├── examples/                  # 示例代码
└── resources/                 # 资源文件
```

## 快速开始

### 1. 包含模块

在你的.pro文件中包含utils模块：

```qmake
include(modules/utils/utils.pri)
```

### 2. 基本使用

```cpp
#include "UtilsModule.h"
#include "config/UtilsConfig.h"
#include "UtilsErrorHandler.h"
#include "Logger.h"
#include "StringUtils.h"

// 初始化工具模块（自动初始化所有子系统）
UtilsModule* utils = UtilsModule::instance();
if (!utils->initialize()) {
    qCritical() << "Failed to initialize utils module:" << utils->lastError();
    return false;
}

// 使用配置系统
UtilsConfig* config = utils->getConfig();
config->setValue(UtilsConfig::DebugMode, true);
bool debugMode = config->getValue(UtilsConfig::DebugMode).toBool();

// 使用错误处理
UtilsErrorHandler* errorHandler = utils->getErrorHandler();
QString errorId = errorHandler->reportError("Test error", "ExampleApp");

// 使用日志系统
Logger* logger = utils->getSingletonManager()->getLogger();
logger->info("Application started");
logger->warning("This is a warning message");

// 使用字符串工具
QString trimmed = StringUtils::trim("  hello world  ");
bool isEmail = Validator::isValidEmail("user@example.com");
```

### 3. 文件操作

```cpp
#include "ConfigFile.h"
#include "TempFile.h"

// 配置文件操作
ConfigFile config("settings.ini");
config.setValue("audio/volume", 75);
int volume = config.value("audio/volume", 50).toInt();

// 临时文件管理
TempFile temp("temp_data");
temp.write("temporary data");
QString content = temp.readAll();
```

### 4. 加密功能

```cpp
#include "AESCrypto.h"
#include "HashUtils.h"

// AES加密
AESCrypto aes;
QByteArray encrypted = aes.encrypt("sensitive data", "password");
QByteArray decrypted = aes.decrypt(encrypted, "password");

// 哈希计算
QString hash = HashUtils::sha256("data to hash");
QString md5 = HashUtils::md5("data to hash");
```

## API参考

### 核心接口

#### ILogger
```cpp
class ILogger {
public:
    enum LogLevel { Debug, Info, Warning, Error, Critical };
    
    virtual void log(LogLevel level, const QString& message) = 0;
    virtual void setLogLevel(LogLevel level) = 0;
    virtual LogLevel logLevel() const = 0;
};
```

#### IFileHandler
```cpp
class IFileHandler {
public:
    virtual bool exists(const QString& path) const = 0;
    virtual QByteArray read(const QString& path) = 0;
    virtual bool write(const QString& path, const QByteArray& data) = 0;
    virtual bool remove(const QString& path) = 0;
};
```

#### ICryptoHandler
```cpp
class ICryptoHandler {
public:
    virtual QByteArray encrypt(const QByteArray& data, const QString& key) = 0;
    virtual QByteArray decrypt(const QByteArray& data, const QString& key) = 0;
    virtual QString hash(const QByteArray& data) = 0;
};
```

## 配置选项

### 编译时配置

```qmake
# 启用调试模式
CONFIG += debug
DEFINES += UTILS_DEBUG_MODE

# 平台特定配置
win32: DEFINES += UTILS_PLATFORM_WINDOWS
unix:!mac: DEFINES += UTILS_PLATFORM_LINUX
mac: DEFINES += UTILS_PLATFORM_MAC
```

### 运行时配置

```cpp
// 配置日志级别
Logger::setGlobalLogLevel(Logger::Info);

// 配置文件监控
FileWatcher::setWatchInterval(1000); // 1秒
```

## 依赖项

### Qt模块
- Qt Core (必需)
- Qt Network (网络日志功能)

### 外部库
- OpenSSL (加密功能)

### 系统依赖
- Windows: advapi32.lib (注册表访问)
- Linux: 无额外依赖
- macOS: 无额外依赖

## 测试

运行单元测试：

```bash
# Linux/macOS
cd tests && ./run_tests.sh

# Windows
cd tests && run_tests.bat
```

## 性能考虑

### 日志系统
- 异步日志写入，避免阻塞主线程
- 日志轮转，防止日志文件过大
- 可配置的日志级别过滤

### 文件操作
- 缓存机制，减少磁盘I/O
- 异步文件监控
- 内存映射用于大文件操作

### 加密功能
- 硬件加速支持（如果可用）
- 密钥缓存机制
- 流式加密用于大数据

## 安全考虑

### 加密
- 使用行业标准加密算法
- 安全的密钥管理
- 防止时序攻击

### 文件操作
- 路径验证，防止目录遍历攻击
- 权限检查
- 安全的临时文件创建

### 日志
- 敏感信息过滤
- 日志文件权限控制
- 网络日志加密传输

## 故障排除

### 常见问题

1. **编译错误: 找不到OpenSSL**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libssl-dev
   
   # CentOS/RHEL
   sudo yum install openssl-devel
   
   # macOS
   brew install openssl
   ```

2. **日志文件无法创建**
   - 检查目录权限
   - 确保磁盘空间充足
   - 验证文件路径有效性

3. **文件监控不工作**
   - 检查文件系统支持
   - 验证监控路径存在
   - 确认权限设置正确

## 版本历史

### v1.0.0 (当前版本)
- 初始版本
- 基础日志系统
- 文件操作工具
- 加密功能
- 字符串处理工具

## 许可证

本模块遵循项目主许可证。

## 贡献

欢迎提交问题报告和功能请求到项目仓库。

## 联系方式

如有问题，请通过项目仓库的Issue系统联系开发团队。