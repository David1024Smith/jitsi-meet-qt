# 屏幕共享模块测试
# Screen Share Module Tests

## 概述 / Overview

本目录包含屏幕共享模块的完整测试套件，实现了Task 43的所有要求：

This directory contains the complete test suite for the Screen Share module, implementing all requirements of Task 43:

- ✅ 创建ScreenShareModuleTest单元测试类
- ✅ 实现屏幕捕获和编码测试  
- ✅ 添加捕获质量和性能测试
- ✅ 创建屏幕共享UI组件测试
- ✅ 实现与现有ScreenShareManager的兼容性测试

## 文件结构 / File Structure

```
tests/
├── ScreenShareModuleTest.h          # 主测试类头文件
├── ScreenShareModuleTest.cpp        # 主测试类实现
├── TestRunner.cpp                   # 测试运行器
├── CMakeLists.txt                   # CMake构建配置
├── run_tests.sh                     # Linux/macOS测试脚本
├── run_tests.bat                    # Windows测试脚本
├── TEST_DOCUMENTATION.md            # 详细测试文档
├── README.md                        # 本文件
├── mocks/                           # 模拟对象
│   ├── MockScreenShareManager.h     # 模拟屏幕共享管理器头文件
│   └── MockScreenShareManager.cpp   # 模拟屏幕共享管理器实现
└── data/                           # 测试数据
    └── test_configurations.json    # 测试配置数据
```

## 测试类别 / Test Categories

### 1. 模块基础测试 / Module Basic Tests
- 模块初始化和配置
- 管理器访问和状态管理
- 模块信息和自检功能

### 2. 捕获系统测试 / Capture System Tests  
- 屏幕捕获 (ScreenCapture)
- 窗口捕获 (WindowCapture)
- 区域捕获 (RegionCapture)
- 捕获引擎 (CaptureEngine)

### 3. 编码处理测试 / Encoding Processing Tests
- 视频编码器 (VideoEncoder)
- 帧处理器 (FrameProcessor)
- 编码质量和格式

### 4. 质量和性能测试 / Quality and Performance Tests
- 捕获质量指标测试
- 性能基准测试
- 内存使用优化测试
- CPU使用监控测试
- 帧率稳定性测试
- 延迟测量测试

### 5. UI组件测试 / UI Component Tests
- 屏幕共享组件 (ScreenShareWidget)
- 屏幕选择器 (ScreenSelector)
- 捕获预览 (CapturePreview)
- UI交互和响应性测试
- UI状态管理测试

### 6. 兼容性测试 / Compatibility Tests
- 旧版兼容性测试
- API兼容性测试
- 配置迁移测试
- 功能对等测试
- 向后兼容性测试
- 现有代码集成测试

## 快速开始 / Quick Start

### 运行所有测试 / Run All Tests

#### Linux/macOS:
```bash
cd jitsi-meet-qt/modules/screenshare/tests
chmod +x run_tests.sh
./run_tests.sh
```

#### Windows:
```cmd
cd jitsi-meet-qt\modules\screenshare\tests
run_tests.bat
```

### 使用CMake / Using CMake

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
./screenshare_tests
```

### 运行特定测试 / Run Specific Tests

```bash
# 运行质量测试
./screenshare_tests testCaptureQualityMetrics

# 运行性能测试
./screenshare_tests testPerformanceBenchmarks

# 运行UI测试
./screenshare_tests testScreenShareWidget

# 运行兼容性测试
./screenshare_tests testLegacyCompatibility
```

## 测试配置 / Test Configuration

### 环境变量 / Environment Variables

```bash
export QT_QPA_PLATFORM=offscreen     # 无头模式运行
export QT_LOGGING_RULES="*.debug=false"  # 禁用调试日志
```

### 性能阈值 / Performance Thresholds

测试使用以下性能阈值 (可在代码中调整):

The tests use the following performance thresholds (adjustable in code):

| 指标 / Metric | 阈值 / Threshold | 描述 / Description |
|---------------|------------------|-------------------|
| 捕获时间 / Capture Time | < 50ms | 单帧捕获时间 |
| 编码时间 / Encode Time | < 100ms | 单帧编码时间 |
| 内存变化 / Memory Variation | < 100MB | 内存使用稳定性 |
| CPU使用率 / CPU Usage | < 80% (avg), < 95% (max) | CPU使用率限制 |
| 延迟 / Latency | < 200ms (avg), < 500ms (max) | 端到端延迟 |

## 测试数据 / Test Data

### 配置文件 / Configuration Files

`data/test_configurations.json` 包含:
- 不同质量级别的测试配置
- 性能基准数据
- 模拟屏幕和窗口数据
- 旧版配置格式
- 错误场景定义

### 模拟对象 / Mock Objects

`mocks/MockScreenShareManager` 提供:
- 完整的IScreenShareManager接口实现
- 可配置的模拟行为
- 测试计数器和状态跟踪
- 兼容性测试支持

## 故障排除 / Troubleshooting

### 常见问题 / Common Issues

1. **编译错误 / Compilation Errors**
   ```bash
   # 检查Qt版本
   qmake --version
   
   # 检查依赖项
   pkg-config --modversion Qt5Core Qt5Gui Qt5Widgets Qt5Test
   ```

2. **运行时错误 / Runtime Errors**
   ```bash
   # 检查环境变量
   echo $QT_QPA_PLATFORM
   
   # 检查权限 (Linux)
   xhost +local:
   ```

3. **测试失败 / Test Failures**
   ```bash
   # 运行详细模式
   ./screenshare_tests -v2
   
   # 运行单个测试
   ./screenshare_tests -v2 testModuleInitialization
   ```

### 调试模式 / Debug Mode

```bash
# 启用调试输出
export QT_LOGGING_RULES="*.debug=true"

# 运行调试版本
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
gdb ./screenshare_tests
```

## 性能分析 / Performance Analysis

### 内存分析 / Memory Analysis

```bash
# 使用Valgrind检查内存泄漏
valgrind --leak-check=full ./screenshare_tests

# 使用AddressSanitizer
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address"
```

### CPU分析 / CPU Profiling

```bash
# 使用perf分析性能
perf record ./screenshare_tests
perf report
```

## 持续集成 / Continuous Integration

### GitHub Actions

测试可以集成到CI/CD流水线中:

Tests can be integrated into CI/CD pipelines:

```yaml
- name: Run Screen Share Tests
  run: |
    cd jitsi-meet-qt/modules/screenshare/tests
    chmod +x run_tests.sh
    ./run_tests.sh
```

### 测试报告 / Test Reports

测试生成JUnit格式的XML报告，可用于CI系统:

Tests generate JUnit-format XML reports for CI systems:

```bash
./screenshare_tests -o screenshare_tests.xml,junitxml
```

## 贡献 / Contributing

### 添加新测试 / Adding New Tests

1. 在 `ScreenShareModuleTest.h` 中声明测试方法
2. 在 `ScreenShareModuleTest.cpp` 中实现测试方法
3. 更新 `CMakeLists.txt` 如果需要新依赖
4. 更新文档

### 测试指南 / Testing Guidelines

- 使用描述性的测试方法名
- 每个测试应该独立且可重复
- 使用适当的断言宏 (`QVERIFY`, `QCOMPARE`)
- 测试边界条件和错误情况
- 添加性能测试时设置合理的阈值

## 版本历史 / Version History

- **v1.0.0**: 初始测试框架
- **v1.1.0**: 添加Task 41测试 (捕获和编码)
- **v1.2.0**: 添加Task 43测试 (质量、性能、UI、兼容性) - 当前版本

## 许可证 / License

本测试套件遵循与主项目相同的许可证。

This test suite follows the same license as the main project.