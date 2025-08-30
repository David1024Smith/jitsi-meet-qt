# Meeting Module Tests

## 概述

本目录包含Meeting模块的完整测试套件，包括单元测试、集成测试和性能测试。

## 测试结构

```
tests/
├── README.md                    # 本文档
├── CMakeLists.txt              # CMake构建配置
├── meeting_tests.pro           # qmake构建配置
├── MeetingModuleTest.h         # 主测试类头文件
├── MeetingModuleTest.cpp       # 主测试类实现
├── run_tests.sh               # Linux/macOS测试脚本
├── run_tests.bat              # Windows测试脚本
├── mocks/                     # 模拟对象
│   ├── MockMeetingManager.h
│   └── MockMeetingManager.cpp
├── data/                      # 测试数据
│   ├── test_meetings.json
│   └── test_configurations.json
└── integration/               # 集成测试
    ├── MeetingIntegrationTest.h
    └── MeetingIntegrationTest.cpp
```

## 测试覆盖

### 单元测试
- [x] MeetingModule 初始化和生命周期
- [x] MeetingManager 会议管理功能
- [x] LinkHandler URL解析和验证
- [x] URLHandler URL处理逻辑
- [x] ProtocolHandler 协议处理
- [x] AuthHandler 认证功能
- [x] Meeting 数据模型
- [x] Room 房间模型
- [x] Invitation 邀请模型
- [x] MeetingConfig 配置管理
- [x] UI组件测试

### 集成测试
- [x] 模块间通信测试
- [x] 端到端会议流程测试
- [x] 网络连接测试
- [x] 错误处理和恢复测试

### 性能测试
- [x] 模块启动时间测试
- [x] 内存使用测试
- [x] 并发处理测试
- [x] 大量数据处理测试

## 运行测试

### 使用脚本运行

```bash
# Linux/macOS
./run_tests.sh

# Windows
run_tests.bat
```

### 使用CMake运行

```bash
mkdir build
cd build
cmake ..
make
ctest
```

### 使用qmake运行

```bash
qmake meeting_tests.pro
make
./meeting_tests
```

## 测试配置

### 环境变量
- `MEETING_TEST_SERVER`: 测试服务器地址
- `MEETING_TEST_TIMEOUT`: 测试超时时间
- `MEETING_TEST_DEBUG`: 启用调试输出

### 配置文件
测试使用 `data/test_configurations.json` 中的配置进行测试。

## 模拟对象

### MockMeetingManager
模拟会议管理器，用于测试UI组件和其他依赖组件。

### MockLinkHandler
模拟链接处理器，用于测试URL处理逻辑。

## 测试数据

### test_meetings.json
包含各种测试会议数据，用于验证会议模型和管理功能。

### test_configurations.json
包含各种配置场景，用于测试配置管理功能。

## 持续集成

测试套件集成到CI/CD流水线中，每次代码提交都会自动运行测试。

### GitHub Actions
```yaml
- name: Run Meeting Module Tests
  run: |
    cd jitsi-meet-qt/modules/meeting/tests
    ./run_tests.sh
```

## 测试报告

测试结果生成详细报告，包括：
- 测试覆盖率
- 性能指标
- 内存使用情况
- 错误日志

## 故障排除

### 常见问题

1. **测试超时**
   - 检查网络连接
   - 增加超时时间
   - 检查测试服务器状态

2. **模拟对象错误**
   - 验证模拟对象配置
   - 检查依赖关系
   - 更新模拟数据

3. **配置错误**
   - 验证测试配置文件
   - 检查环境变量
   - 确认测试权限

## 贡献指南

### 添加新测试
1. 在相应的测试类中添加测试方法
2. 使用适当的断言验证结果
3. 添加必要的测试数据
4. 更新测试文档

### 测试命名规范
- 测试方法以 `test` 开头
- 使用描述性名称
- 包含测试场景说明

### 测试最佳实践
- 每个测试应该独立运行
- 使用适当的设置和清理
- 提供清晰的错误消息
- 避免测试间的依赖关系