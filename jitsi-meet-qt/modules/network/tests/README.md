# 网络模块测试

## 概述

本目录包含网络模块的所有测试代码，包括单元测试、集成测试和性能测试。

## 测试结构

```
tests/
├── README.md                   # 本文档
├── CMakeLists.txt             # CMake构建配置
├── NetworkTestRunner.cpp      # 测试运行器
├── NetworkTestSuite.cpp       # 测试套件
├── unit/                      # 单元测试
│   ├── NetworkModuleTest.cpp  # 网络模块测试
│   ├── NetworkManagerTest.cpp # 网络管理器测试
│   ├── ConnectionFactoryTest.cpp # 连接工厂测试
│   └── NetworkConfigTest.cpp  # 网络配置测试
├── integration/               # 集成测试
│   ├── NetworkIntegrationTest.cpp # 网络集成测试
│   └── ProtocolIntegrationTest.cpp # 协议集成测试
├── performance/               # 性能测试
│   ├── NetworkPerformanceTest.cpp # 网络性能测试
│   └── BandwidthTest.cpp      # 带宽测试
├── mock/                      # 模拟对象
│   ├── MockNetworkManager.h   # 模拟网络管理器
│   └── MockConnectionHandler.h # 模拟连接处理器
└── data/                      # 测试数据
    ├── test_config.json       # 测试配置
    └── sample_data.bin        # 示例数据
```

## 运行测试

### 使用CMake构建和运行

```bash
# 进入测试目录
cd tests

# 创建构建目录
mkdir build && cd build

# 配置CMake
cmake ..

# 编译测试
make

# 运行所有测试
./NetworkTestRunner

# 运行特定测试
./NetworkTestRunner --test NetworkModuleTest
```

### 使用qmake构建和运行

```bash
# 生成Makefile
qmake network_tests.pro

# 编译测试
make

# 运行测试
./network_tests
```

## 测试类型

### 单元测试

测试网络模块的各个组件的独立功能：

- **NetworkModuleTest**: 测试网络模块的初始化、配置和生命周期管理
- **NetworkManagerTest**: 测试网络管理器的连接管理和状态监控
- **ConnectionFactoryTest**: 测试连接工厂的连接创建和管理
- **NetworkConfigTest**: 测试网络配置的参数验证和持久化

### 集成测试

测试网络模块与其他组件的集成：

- **NetworkIntegrationTest**: 测试网络模块与应用程序的集成
- **ProtocolIntegrationTest**: 测试不同协议处理器的集成

### 性能测试

测试网络模块的性能指标：

- **NetworkPerformanceTest**: 测试网络连接的延迟和吞吐量
- **BandwidthTest**: 测试带宽利用率和传输效率

## 测试配置

### 测试环境变量

```bash
# 设置测试服务器地址
export TEST_SERVER_URL="https://test.example.com"

# 设置测试端口
export TEST_SERVER_PORT="8443"

# 启用详细日志
export NETWORK_TEST_VERBOSE="1"

# 设置测试超时时间（秒）
export NETWORK_TEST_TIMEOUT="30"
```

### 测试配置文件

`data/test_config.json`:
```json
{
  "server": {
    "url": "https://test.example.com",
    "port": 8443,
    "timeout": 30000
  },
  "protocols": {
    "webrtc": true,
    "websocket": true,
    "http": true
  },
  "performance": {
    "max_latency": 100,
    "min_bandwidth": 1000,
    "test_duration": 60
  }
}
```

## 模拟对象

### MockNetworkManager

模拟网络管理器，用于测试网络相关功能而不需要实际的网络连接。

```cpp
MockNetworkManager mockManager;
mockManager.setConnectionState(INetworkManager::Connected);
mockManager.setNetworkQuality(INetworkManager::Good);
```

### MockConnectionHandler

模拟连接处理器，用于测试连接相关功能。

```cpp
MockConnectionHandler mockHandler;
mockHandler.setConnectionStatus(IConnectionHandler::Connected);
mockHandler.simulateDataReceived(testData);
```

## 测试数据

### 示例配置

测试使用的示例配置数据，包括各种有效和无效的配置组合。

### 网络数据

用于测试网络传输的示例数据，包括不同大小和格式的数据包。

## 持续集成

### GitHub Actions

```yaml
name: Network Module Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - name: Install Qt
      run: sudo apt-get install qt5-default
    - name: Build and Test
      run: |
        cd modules/network/tests
        mkdir build && cd build
        cmake ..
        make
        ./NetworkTestRunner
```

### 测试报告

测试结果会生成详细的报告，包括：

- 测试覆盖率报告
- 性能基准测试结果
- 内存泄漏检测报告
- 静态代码分析报告

## 故障排除

### 常见问题

1. **网络连接失败**
   - 检查测试服务器是否可达
   - 验证防火墙设置
   - 确认网络配置正确

2. **测试超时**
   - 增加测试超时时间
   - 检查系统负载
   - 验证测试数据大小

3. **模拟对象错误**
   - 确认模拟对象配置正确
   - 检查测试数据格式
   - 验证期望行为设置

### 调试技巧

- 使用 `NETWORK_TEST_VERBOSE=1` 启用详细日志
- 运行单个测试用例进行调试
- 使用网络抓包工具分析实际网络流量
- 检查测试配置文件的正确性

## 贡献指南

### 添加新测试

1. 在相应的目录下创建测试文件
2. 继承适当的测试基类
3. 实现测试方法
4. 更新CMakeLists.txt或.pro文件
5. 添加测试文档

### 测试命名规范

- 测试类名：`<ComponentName>Test`
- 测试方法名：`test<FunctionName>`
- 测试文件名：`<ComponentName>Test.cpp`

### 代码覆盖率

目标是达到90%以上的代码覆盖率。使用以下工具生成覆盖率报告：

```bash
# 使用gcov生成覆盖率报告
gcov *.cpp
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```