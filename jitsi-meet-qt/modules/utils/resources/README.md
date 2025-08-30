# Utils Module Resources

本目录包含Utils模块使用的资源文件。

## 资源结构

```
resources/
├── README.md                   # 资源说明文档
├── configs/                    # 配置文件模板
│   ├── default_config.ini      # 默认INI配置
│   ├── logging_config.json     # 日志配置模板
│   └── crypto_config.xml       # 加密配置模板
├── templates/                  # 文件模板
│   ├── log_format.txt          # 日志格式模板
│   ├── config_schema.json      # 配置文件架构
│   └── error_messages.txt      # 错误消息模板
├── certificates/               # 证书文件
│   ├── test_cert.pem           # 测试证书
│   ├── test_key.pem            # 测试私钥
│   └── ca_bundle.pem           # CA证书包
├── data/                       # 数据文件
│   ├── country_codes.json      # 国家代码数据
│   ├── currency_codes.json     # 货币代码数据
│   ├── timezone_data.json      # 时区数据
│   └── validation_patterns.json # 验证模式数据
├── localization/               # 本地化文件
│   ├── en_US.json              # 英文本地化
│   ├── zh_CN.json              # 中文本地化
│   └── messages.properties     # 消息属性文件
└── icons/                      # 图标资源
    ├── utils_module.png        # 模块图标
    ├── log_levels/             # 日志级别图标
    └── file_types/             # 文件类型图标
```

## 配置文件模板

### default_config.ini
默认的INI格式配置文件模板，包含：
- 模块基本设置
- 日志配置
- 文件操作设置
- 加密参数

### logging_config.json
JSON格式的日志配置模板：
- 日志级别设置
- 输出目标配置
- 格式化选项
- 轮转策略

### crypto_config.xml
XML格式的加密配置模板：
- 加密算法设置
- 密钥管理配置
- 安全策略

## 文件模板

### log_format.txt
日志格式化模板，支持：
- 时间戳格式
- 日志级别显示
- 消息格式化
- 自定义字段

### config_schema.json
配置文件的JSON Schema定义：
- 配置项验证规则
- 数据类型定义
- 必需字段标识
- 默认值设置

### error_messages.txt
错误消息模板：
- 标准化错误消息
- 多语言支持
- 错误代码映射
- 用户友好提示

## 证书文件

### test_cert.pem / test_key.pem
用于测试的SSL/TLS证书和私钥：
- 仅用于开发和测试
- 不应在生产环境使用
- 支持基本的加密测试

### ca_bundle.pem
CA证书包：
- 常用的根证书
- 用于SSL验证
- 定期更新维护

## 数据文件

### country_codes.json
ISO国家代码数据：
- 国家名称和代码映射
- 支持多语言显示
- 用于地址验证

### currency_codes.json
ISO货币代码数据：
- 货币符号和代码
- 汇率相关信息
- 格式化规则

### timezone_data.json
时区数据：
- 时区标识符
- UTC偏移量
- 夏令时规则

### validation_patterns.json
验证模式数据：
- 正则表达式模式
- 验证规则定义
- 错误消息映射

## 本地化文件

### en_US.json / zh_CN.json
多语言支持文件：
- 界面文本翻译
- 错误消息本地化
- 格式化模板

### messages.properties
Java风格的消息属性文件：
- 键值对格式
- 参数化消息
- 易于维护

## 图标资源

### utils_module.png
模块主图标：
- 32x32像素PNG格式
- 透明背景
- 高质量矢量转换

### log_levels/
日志级别图标：
- debug.png - 调试级别
- info.png - 信息级别
- warning.png - 警告级别
- error.png - 错误级别
- critical.png - 严重级别

### file_types/
文件类型图标：
- config.png - 配置文件
- log.png - 日志文件
- temp.png - 临时文件
- encrypted.png - 加密文件

## 使用方法

### 在代码中访问资源
```cpp
// 获取资源文件路径
QString configPath = ":/utils/configs/default_config.ini";
QString iconPath = ":/utils/icons/utils_module.png";

// 读取资源文件
QFile file(configPath);
if (file.open(QIODevice::ReadOnly)) {
    QByteArray data = file.readAll();
    // 处理数据
}
```

### 在qmake项目中包含资源
```qmake
RESOURCES += utils_resources.qrc
```

### 资源文件格式
```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource prefix="/utils">
        <file>configs/default_config.ini</file>
        <file>templates/log_format.txt</file>
        <file>icons/utils_module.png</file>
    </qresource>
</RCC>
```

## 维护说明

1. **定期更新**: 证书、时区数据等需要定期更新
2. **版本控制**: 重要资源文件需要版本标识
3. **安全考虑**: 敏感数据不应包含在资源中
4. **大小限制**: 控制资源文件大小，避免影响程序体积
5. **格式标准**: 遵循相应的文件格式标准

## 扩展资源

添加新资源时：

1. 选择合适的子目录
2. 使用标准的文件格式
3. 添加适当的文档说明
4. 更新资源配置文件
5. 测试资源访问功能