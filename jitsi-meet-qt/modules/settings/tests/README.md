# Settings Module Tests

## 概述

本目录包含 Settings Module 的测试文件，包括单元测试、集成测试和性能测试。

## 测试结构

```
tests/
├── README.md                   # 测试说明文档
├── SettingsModuleTest.h        # 主测试类头文件
├── SettingsModuleTest.cpp      # 主测试类实现
├── run_tests.sh               # Linux/macOS 测试运行脚本
├── run_tests.bat              # Windows 测试运行脚本
├── settings_tests.pro         # qmake 项目文件
├── CMakeLists.txt             # CMake 配置文件
├── mocks/                     # 模拟对象
│   ├── MockSettingsManager.h
│   ├── MockPreferencesHandler.h
│   └── MockValidator.h
└── data/                      # 测试数据
    ├── test_config.json
    ├── invalid_config.json
    └── schema_test.json
```

## 运行测试

### 使用 qmake
```bash
cd tests
qmake settings_tests.pro
make
./settings_tests
```

### 使用 CMake
```bash
cd tests
mkdir build && cd build
cmake ..
make
./settings_tests
```

### 使用脚本
```bash
# Linux/macOS
./run_tests.sh

# Windows
run_tests.bat
```

## 测试覆盖

- 设置管理器测试
- 偏好处理器测试
- 配置验证器测试
- 存储后端测试
- UI组件测试
- 集成测试
- 性能测试

## 测试数据

测试使用 `data/` 目录中的示例配置文件和测试数据。

## 持续集成

测试可以集成到 CI/CD 流水线中，支持自动化测试和覆盖率报告。