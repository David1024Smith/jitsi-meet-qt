# Utils Module Examples

本目录包含Utils模块的使用示例和演示代码。

## 示例列表

### 基础示例
- `BasicUsageExample.cpp` - 基本使用方法
- `ModuleInitExample.cpp` - 模块初始化示例
- `ConfigurationExample.cpp` - 配置管理示例

### 日志系统示例
- `LoggingExample.cpp` - 日志记录基础用法
- `CustomLoggerExample.cpp` - 自定义日志记录器
- `MultiLoggerExample.cpp` - 多日志输出示例
- `LogRotationExample.cpp` - 日志轮转示例

### 文件操作示例
- `FileOperationsExample.cpp` - 基本文件操作
- `ConfigFileExample.cpp` - 配置文件处理
- `TempFileExample.cpp` - 临时文件管理
- `FileWatcherExample.cpp` - 文件监控示例

### 加密示例
- `AESEncryptionExample.cpp` - AES加密示例
- `RSAEncryptionExample.cpp` - RSA加密示例
- `HashingExample.cpp` - 哈希计算示例
- `DigitalSignatureExample.cpp` - 数字签名示例

### 字符串处理示例
- `StringUtilsExample.cpp` - 字符串工具使用
- `ValidationExample.cpp` - 数据验证示例
- `FormatExample.cpp` - 字符串格式化
- `EncodingExample.cpp` - 编码转换示例

### 高级示例
- `ThreadSafetyExample.cpp` - 线程安全使用
- `PerformanceExample.cpp` - 性能优化示例
- `ErrorHandlingExample.cpp` - 错误处理示例
- `IntegrationExample.cpp` - 模块集成示例

## 运行示例

### 编译单个示例
```bash
# 使用qmake
qmake example.pro
make
./example

# 使用g++直接编译
g++ -I../include -I../interfaces example.cpp -o example -lQt5Core
```

### 编译所有示例
```bash
# Linux/macOS
./build_examples.sh

# Windows
build_examples.bat
```

## 示例说明

### BasicUsageExample.cpp
演示Utils模块的基本使用方法：
- 模块初始化
- 基本配置
- 简单的日志记录
- 文件操作

### LoggingExample.cpp
展示日志系统的各种功能：
- 不同日志级别
- 多种输出方式
- 日志格式化
- 异步日志记录

### FileOperationsExample.cpp
文件操作的完整示例：
- 文件读写
- 目录管理
- 文件监控
- 临时文件处理

### AESEncryptionExample.cpp
AES加密的使用示例：
- 密钥生成
- 数据加密解密
- 不同加密模式
- 错误处理

### StringUtilsExample.cpp
字符串处理工具的使用：
- 字符串格式化
- 大小写转换
- 编码转换
- 正则表达式

### ValidationExample.cpp
数据验证功能演示：
- 邮箱验证
- URL验证
- 密码强度检查
- 自定义验证规则

## 最佳实践

### 1. 模块初始化
```cpp
// 在应用程序启动时初始化
UtilsModule* utils = UtilsModule::instance();
if (!utils->initialize()) {
    qCritical() << "Failed to initialize Utils module";
    return -1;
}
```

### 2. 日志使用
```cpp
// 使用便捷宏
LOG_INFO("Application started");
LOG_ERROR("An error occurred");

// 或直接使用Logger
Logger::info("Application started");
Logger::error("An error occurred");
```

### 3. 文件操作
```cpp
// 使用FileManager进行文件操作
FileManager* fm = FileManager::instance();
QByteArray data;
if (fm->readFile("config.txt", data) == FileManager::Success) {
    // 处理文件数据
}
```

### 4. 加密操作
```cpp
// AES加密
AESCrypto aes;
QByteArray encrypted = aes.encryptAES(data, "password");
QByteArray decrypted = aes.decryptAES(encrypted, "password");
```

### 5. 字符串处理
```cpp
// 字符串工具
QString trimmed = StringUtils::trim("  hello world  ");
bool isEmail = Validator::validateEmail("user@example.com").isValid;
```

## 注意事项

1. **线程安全**: 所有示例都考虑了线程安全性
2. **错误处理**: 展示了正确的错误处理方法
3. **资源管理**: 演示了正确的资源清理
4. **性能考虑**: 包含了性能优化的建议

## 扩展示例

如需添加新的示例：

1. 创建新的.cpp文件
2. 包含必要的头文件
3. 添加详细的注释说明
4. 更新本README文档
5. 添加到构建脚本中