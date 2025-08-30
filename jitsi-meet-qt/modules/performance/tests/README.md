# Performance Module Tests

## 概述

本目录包含性能模块的完整测试套件，确保模块的功能正确性、性能表现和稳定性。

## 测试结构

### 测试文件
- `PerformanceModuleTest.h/cpp` - 主要测试类
- `run_tests.sh` - Linux/macOS测试运行脚本
- `run_tests.bat` - Windows测试运行脚本
- `performance_tests.pro` - qmake项目文件
- `CMakeLists.txt` - CMake配置文件

### 测试类别

#### 1. 基础功能测试
- 模块初始化和生命周期
- 组件创建和配置
- 状态管理和错误处理

#### 2. 监控功能测试
- CPU监控准确性
- 内存监控功能
- 网络监控能力
- 实时数据收集

#### 3. 优化功能测试
- 启动优化效果
- 内存优化策略
- 渲染优化性能
- 自动优化机制

#### 4. 配置管理测试
- 配置加载和保存
- 参数验证
- 默认值处理
- 配置更新机制

#### 5. 性能测试
- 模块性能开销
- 内存使用效率
- CPU使用率
- 响应时间测试

#### 6. 集成测试
- 模块间交互
- 系统集成
- UI集成测试
- 跨平台兼容性

## 运行测试

### Linux/macOS
```bash
# 构建测试
qmake performance_tests.pro
make

# 运行所有测试
./run_tests.sh

# 运行压力测试
./run_tests.sh --stress

# 运行特定测试
./performance_tests testModuleInitialization
```

### Windows
```cmd
# 构建测试
qmake performance_tests.pro
nmake

# 运行所有测试
run_tests.bat

# 运行特定测试
performance_tests.exe testModuleInitialization
```

### CMake构建
```bash
mkdir build
cd build
cmake ..
make
ctest
```

## 测试配置

### 环境变量
- `QT_QPA_PLATFORM=offscreen` - 无头模式运行
- `QT_LOGGING_RULES="*.debug=false"` - 禁用调试输出
- `PERFORMANCE_TEST_DATA_PATH` - 测试数据路径

### 测试参数
- 测试超时: 300秒
- 内存限制: 1GB
- CPU限制: 80%

## 测试报告

### 输出格式
- XML格式 - 用于CI/CD集成
- 文本格式 - 人类可读
- HTML格式 - 详细报告

### 报告内容
- 测试执行统计
- 性能指标对比
- 错误详情和堆栈
- 覆盖率报告

## 持续集成

### GitHub Actions
```yaml
- name: Run Performance Tests
  run: |
    cd modules/performance/tests
    ./run_tests.sh
```

### Jenkins
```groovy
stage('Performance Tests') {
    steps {
        sh 'cd modules/performance/tests && ./run_tests.sh'
    }
    post {
        always {
            publishTestResults testResultsPattern: 'modules/performance/tests/test_results/*.xml'
        }
    }
}
```

## 测试数据

### 模拟数据
- CPU使用率: 0-100%
- 内存使用: 100MB-8GB
- 网络延迟: 1-1000ms
- 帧率: 1-120fps

### 测试场景
- 正常负载
- 高负载
- 极限负载
- 错误条件

## 性能基准

### 基准指标
- 模块启动时间: < 100ms
- 内存占用: < 50MB
- CPU开销: < 5%
- 响应延迟: < 10ms

### 回归测试
- 性能不能下降超过10%
- 内存使用不能增加超过20%
- 启动时间不能增加超过50ms

## 故障排除

### 常见问题

1. **测试超时**
   - 检查系统负载
   - 增加超时时间
   - 检查死锁情况

2. **内存不足**
   - 减少并发测试
   - 清理测试数据
   - 检查内存泄漏

3. **权限错误**
   - 检查文件权限
   - 运行管理员权限
   - 检查系统限制

### 调试技巧
```bash
# 启用详细日志
export QT_LOGGING_RULES="performance.*=true"

# 内存检查
valgrind --tool=memcheck ./performance_tests

# 性能分析
perf record ./performance_tests
perf report
```

## 贡献指南

### 添加新测试
1. 在`PerformanceModuleTest`类中添加测试方法
2. 使用`QVERIFY`和`QCOMPARE`进行断言
3. 添加适当的测试数据和清理
4. 更新测试文档

### 测试命名规范
- 测试方法: `test[功能名称]`
- 测试数据: `m_test[数据名称]`
- 辅助方法: `[动词][名词]`

### 代码覆盖率
- 目标覆盖率: 90%+
- 使用gcov/lcov生成报告
- 关注关键路径覆盖

## 版本历史

### v1.0.0
- 初始测试套件
- 基础功能测试
- 性能基准测试

## 许可证

遵循项目主许可证。