# 音频模块测试框架

本目录包含Jitsi Meet Qt音频模块的完整测试框架，提供全面的单元测试、集成测试、性能测试和兼容性测试。

## 测试结构

### 测试文件

- **AudioConfigTest.cpp/h** - 音频配置类的单元测试
- **AudioUtilsTest.cpp/h** - 音频工具类的单元测试  
- **AudioModuleTest.cpp/h** - 音频模块的综合测试
- **AudioTestSuite.cpp/h** - 测试套件管理器
- **AudioTestRunner.cpp** - 命令行测试运行器

### 测试类别

1. **基础测试 (BasicTests)**
   - 模块初始化和关闭
   - 模块状态管理
   - 版本信息验证

2. **设备测试 (DeviceTests)**
   - 设备枚举和发现
   - 设备选择和切换
   - 设备显示名称处理
   - 无效设备处理

3. **质量测试 (QualityTests)**
   - 音频质量预设
   - 采样率配置
   - 声道配置
   - 缓冲区大小配置
   - 比特率配置

4. **延迟测试 (LatencyTests)**
   - 输入延迟测量
   - 输出延迟测量
   - 往返延迟测量
   - 缓冲区大小对延迟的影响

5. **性能测试 (PerformanceTests)**
   - 内存使用测量
   - CPU使用测量
   - 启动性能测试
   - 设备枚举性能

6. **压力测试 (StressTests)**
   - 多次初始化测试
   - 快速设备切换
   - 连续音量变化
   - 长时间运行测试
   - 资源泄漏检测

7. **兼容性测试 (CompatibilityTests)**
   - MediaManager兼容性
   - 旧版API兼容性
   - 配置迁移测试
   - 向后兼容性

8. **集成测试 (IntegrationTests)**
   - AudioManager集成
   - AudioConfig集成
   - AudioUtils集成
   - UI组件集成

## 构建和运行

### 使用QMake构建

```bash
cd jitsi-meet-qt/modules/audio/tests
qmake tests.pro
make
```

### 使用CMake构建

```bash
cd jitsi-meet-qt/modules/audio/tests
mkdir build && cd build
cmake ..
make
```

### 运行测试

#### 运行所有测试
```bash
./AudioTestRunner --all
```

#### 运行特定类别的测试
```bash
./AudioTestRunner --category basic
./AudioTestRunner --category device
./AudioTestRunner --category quality
./AudioTestRunner --category latency
./AudioTestRunner --category performance
```

#### 运行单个测试
```bash
./AudioTestRunner --test testModuleInitialization
```

#### 详细输出模式
```bash
./AudioTestRunner --all --verbose
```

#### 性能基准测试模式
```bash
./AudioTestRunner --all --benchmark
```

#### 生成测试报告
```bash
./AudioTestRunner --all --report test_report.txt
./AudioTestRunner --all --html-report test_report.html
```

#### 设置超时时间
```bash
./AudioTestRunner --all --timeout 60
```

### 直接运行单个测试类

```bash
./AudioConfigTest
./AudioUtilsTest  
./AudioModuleTest
```

## 测试配置

### 环境要求

- Qt 6.0+ (Core, Test模块)
- 音频设备 (用于设备相关测试)
- 足够的系统权限 (访问音频设备)

### 测试数据

测试框架会自动创建临时配置文件和测试数据，测试完成后自动清理。

### 平台特定测试

某些测试可能是平台特定的：
- Windows: DirectSound/WASAPI相关测试
- Linux: ALSA/PulseAudio相关测试  
- macOS: CoreAudio相关测试

## 测试报告

### 文本报告格式

```
音频模块测试报告
================

生成时间: 2024-01-15 10:30:00
Qt版本: 6.5.0
平台: Windows 11

测试统计:
---------
总测试数: 45
通过: 42
失败: 2
跳过: 1
错误: 0
成功率: 93.3%
总时间: 15420ms

详细测试结果:
-------------
[通过] testModuleInitialization (125ms)
[通过] testDeviceEnumeration (340ms)
[失败] testLatencyMeasurement (2100ms) - 延迟超过阈值
...
```

### HTML报告格式

HTML报告提供更丰富的可视化界面，包括：
- 测试统计图表
- 颜色编码的测试结果
- 详细的错误信息
- 性能指标图表

## 性能基准

### 延迟基准

- 输入延迟: < 20ms
- 输出延迟: < 20ms  
- 往返延迟: < 50ms

### 内存使用基准

- 模块初始化: < 5MB
- 设备枚举: < 1MB增量
- 长时间运行: < 10MB泄漏

### CPU使用基准

- 空闲状态: < 1%
- 音频处理: < 10%
- 设备切换: < 5%峰值

## 故障排除

### 常见问题

1. **音频设备不可用**
   - 确保系统有可用的音频设备
   - 检查设备权限设置
   - 某些测试可能会被跳过

2. **测试超时**
   - 增加超时时间 `--timeout 120`
   - 检查系统负载
   - 某些性能测试可能需要更长时间

3. **权限错误**
   - 确保有访问音频设备的权限
   - 在某些系统上可能需要管理员权限

4. **编译错误**
   - 确保Qt Test模块已安装
   - 检查包含路径设置
   - 验证音频模块依赖

### 调试模式

启用详细输出和调试信息：

```bash
./AudioTestRunner --all --verbose
export QT_LOGGING_RULES="*.debug=true"
```

## 扩展测试

### 添加新测试

1. 在AudioModuleTest类中添加新的测试方法
2. 使用`private slots:`声明测试方法
3. 方法名以`test`开头
4. 使用QVERIFY/QCOMPARE等断言宏

```cpp
private slots:
    void testNewFeature() {
        // 测试实现
        QVERIFY(condition);
        QCOMPARE(actual, expected);
    }
```

### 添加新测试类别

1. 在AudioTestSuite中添加新的TestCategory枚举值
2. 在runAllTests()中添加对应的测试列表
3. 在AudioTestRunner中添加命令行支持

### 性能测试

使用measureLatency()和measureMemoryUsage()辅助方法：

```cpp
void testPerformance() {
    measureLatency("测试名称", [this]() {
        // 性能测试代码
    });
}
```

## 持续集成

测试框架支持CI/CD集成：

```bash
# 运行所有测试并生成报告
./AudioTestRunner --all --report ci_report.txt --timeout 300

# 检查退出码
if [ $? -eq 0 ]; then
    echo "所有测试通过"
else
    echo "测试失败"
    exit 1
fi
```

## 许可证

本测试框架遵循与Jitsi Meet Qt项目相同的许可证。