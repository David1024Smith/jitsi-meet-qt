# Chat Module Tests

## 概述

本目录包含聊天模块的所有测试文件，包括单元测试、集成测试和性能测试。

## 测试结构

```
tests/
├── README.md                   # 测试说明文档
├── ChatModuleTest.h           # 主测试类头文件
├── ChatModuleTest.cpp         # 主测试类实现
├── chat_tests.pro             # qmake项目文件
├── CMakeLists.txt             # CMake配置文件
├── run_tests.sh               # Linux/macOS测试脚本
├── run_tests.bat              # Windows测试脚本
├── mocks/                     # 模拟对象
│   ├── MockChatManager.h
│   ├── MockMessageHandler.h
│   └── MockMessageStorage.h
├── data/                      # 测试数据
│   ├── test_messages.json
│   ├── test_config.json
│   └── sample_chat.txt
└── integration/               # 集成测试
    ├── ChatIntegrationTest.h
    └── ChatIntegrationTest.cpp
```

## 测试类型

### 单元测试
- **ChatModuleTest**: 聊天模块核心功能测试
- **ChatManagerTest**: 聊天管理器测试
- **MessageHandlerTest**: 消息处理器测试
- **MessageStorageTest**: 消息存储测试
- **ChatConfigTest**: 配置管理测试
- **UIComponentTest**: UI组件测试

### 集成测试
- **ChatIntegrationTest**: 模块间集成测试
- **NetworkIntegrationTest**: 网络集成测试
- **StorageIntegrationTest**: 存储集成测试

### 性能测试
- **MessagePerformanceTest**: 消息处理性能测试
- **StoragePerformanceTest**: 存储性能测试
- **UIPerformanceTest**: UI性能测试

## 运行测试

### 使用qmake
```bash
cd tests
qmake chat_tests.pro
make
./chat_tests
```

### 使用CMake
```bash
mkdir build && cd build
cmake ..
make
ctest
```

### 使用脚本
```bash
# Linux/macOS
./run_tests.sh

# Windows
run_tests.bat
```

## 测试覆盖率

目标测试覆盖率：
- 代码覆盖率：>= 90%
- 分支覆盖率：>= 85%
- 函数覆盖率：>= 95%

## 测试数据

测试使用的数据文件：
- `test_messages.json`: 测试消息数据
- `test_config.json`: 测试配置数据
- `sample_chat.txt`: 示例聊天记录

## 模拟对象

为了隔离测试，使用以下模拟对象：
- `MockChatManager`: 模拟聊天管理器
- `MockMessageHandler`: 模拟消息处理器
- `MockMessageStorage`: 模拟消息存储

## 测试环境

### 依赖项
- Qt Test Framework
- Google Test (可选)
- Qt Network (用于网络测试)
- SQLite (用于存储测试)

### 环境变量
- `CHAT_TEST_DATA_DIR`: 测试数据目录
- `CHAT_TEST_DB_PATH`: 测试数据库路径
- `CHAT_TEST_LOG_LEVEL`: 测试日志级别

## 持续集成

测试集成到CI/CD流水线中：
- 每次提交都运行单元测试
- 每日运行完整测试套件
- 性能测试在发布前运行

## 故障排除

### 常见问题
1. **数据库锁定**: 确保测试数据库文件可写
2. **网络超时**: 检查网络连接和防火墙设置
3. **权限问题**: 确保测试目录有读写权限

### 调试技巧
- 使用 `QTEST_MAIN_IMPL` 宏启用详细输出
- 设置 `QT_LOGGING_RULES` 环境变量查看日志
- 使用调试器逐步执行测试

## 贡献指南

### 添加新测试
1. 在相应的测试类中添加测试方法
2. 使用描述性的测试方法名
3. 遵循AAA模式（Arrange, Act, Assert）
4. 添加必要的注释说明测试目的

### 测试命名规范
- 测试类名：`<ClassName>Test`
- 测试方法名：`test<FunctionName>_<Scenario>_<ExpectedResult>`
- 示例：`testSendMessage_ValidMessage_ReturnsTrue`

### 最佳实践
- 每个测试应该独立且可重复
- 使用适当的断言方法
- 清理测试产生的副作用
- 保持测试简单和专注

## 报告问题

如果发现测试问题，请：
1. 检查测试环境配置
2. 查看测试日志输出
3. 在项目仓库中创建Issue
4. 提供详细的错误信息和重现步骤