# Utils Module Tests

本目录包含Utils模块的测试文件。

## 测试结构

```
tests/
├── README.md                   # 测试说明文档
├── UtilsModuleTest.h          # 主测试类头文件
├── UtilsModuleTest.cpp        # 主测试类实现
├── run_tests.sh               # Linux/macOS测试脚本
├── run_tests.bat              # Windows测试脚本
├── utils_tests.pro            # qmake项目文件
├── CMakeLists.txt             # CMake配置文件
├── mocks/                     # 模拟对象
│   ├── MockLogger.h           # 模拟日志记录器
│   ├── MockFileHandler.h      # 模拟文件处理器
│   └── MockCryptoHandler.h    # 模拟加密处理器
├── data/                      # 测试数据
│   ├── test_config.ini        # 测试配置文件
│   ├── test_data.json         # 测试JSON数据
│   └── test_files/            # 测试文件目录
└── benchmarks/                # 性能测试
    ├── LoggingBenchmark.cpp   # 日志性能测试
    ├── CryptoBenchmark.cpp    # 加密性能测试
    └── FileBenchmark.cpp      # 文件操作性能测试
```

## 运行测试

### Linux/macOS
```bash
chmod +x run_tests.sh
./run_tests.sh
```

### Windows
```cmd
run_tests.bat
```

### 使用qmake
```bash
qmake utils_tests.pro
make
./utils_tests
```

### 使用CMake
```bash
mkdir build
cd build
cmake ..
make
./utils_tests
```

## 测试覆盖

- **UtilsModule**: 模块初始化、配置管理、生命周期
- **Logger**: 日志记录、格式化、多输出
- **FileManager**: 文件操作、监控、缓存
- **Crypto**: 加密解密、哈希、签名
- **StringUtils**: 字符串处理、格式化、验证
- **Validator**: 数据验证、格式检查

## 测试类型

### 单元测试
- 测试各个类的独立功能
- 使用模拟对象隔离依赖
- 验证边界条件和错误处理

### 集成测试
- 测试模块间的交互
- 验证完整的工作流程
- 测试配置和初始化

### 性能测试
- 基准测试关键操作
- 内存使用分析
- 并发性能测试

### 压力测试
- 大量数据处理
- 长时间运行稳定性
- 资源泄漏检测

## 测试数据

测试使用的数据文件位于`data/`目录：

- `test_config.ini`: 配置文件测试数据
- `test_data.json`: JSON格式测试数据
- `test_files/`: 各种格式的测试文件

## 模拟对象

`mocks/`目录包含测试用的模拟对象：

- `MockLogger`: 模拟日志记录器
- `MockFileHandler`: 模拟文件处理器
- `MockCryptoHandler`: 模拟加密处理器

## 性能基准

`benchmarks/`目录包含性能测试：

- 日志记录性能
- 文件操作性能
- 加密解密性能
- 字符串处理性能

## 测试报告

测试运行后会生成以下报告：

- 测试结果摘要
- 代码覆盖率报告
- 性能基准报告
- 内存使用报告

## 持续集成

测试脚本支持CI/CD环境：

- 自动化测试执行
- 结果报告生成
- 失败通知机制