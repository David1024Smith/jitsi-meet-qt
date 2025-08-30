# Performance Module Examples

## 概述

本目录包含性能模块的使用示例和演示程序，帮助开发者快速了解和使用性能模块的各种功能。

## 示例文件

### 核心示例
- `PerformanceExample.cpp` - 完整的性能模块使用示例
- `BasicMonitoringExample.cpp` - 基础监控功能示例
- `OptimizationExample.cpp` - 性能优化功能示例
- `ConfigurationExample.cpp` - 配置管理示例
- `DataExportExample.cpp` - 数据导出示例

### UI示例
- `PerformanceWidgetExample.cpp` - 性能界面组件示例
- `MetricsChartExample.cpp` - 指标图表示例
- `CustomMonitorExample.cpp` - 自定义监控器示例

### 高级示例
- `MultiThreadExample.cpp` - 多线程性能监控
- `NetworkMonitoringExample.cpp` - 网络监控示例
- `MemoryOptimizationExample.cpp` - 内存优化示例
- `RealTimeAnalysisExample.cpp` - 实时分析示例

## 快速开始

### 基础使用示例

```cpp
#include "PerformanceModule.h"
#include "PerformanceManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 1. 获取性能模块实例
    PerformanceModule* module = PerformanceModule::instance();
    
    // 2. 初始化模块
    if (module->initialize()) {
        qDebug() << "Performance module initialized";
        
        // 3. 启动监控
        if (module->start()) {
            qDebug() << "Performance monitoring started";
            
            // 4. 获取性能管理器
            PerformanceManager* manager = module->performanceManager();
            
            // 5. 获取当前指标
            PerformanceMetrics metrics = manager->getCurrentMetrics();
            qDebug() << "CPU Usage:" << metrics.system.cpuUsage << "%";
            qDebug() << "Memory Usage:" << metrics.system.memoryUsage << "MB";
        }
    }
    
    return app.exec();
}
```

### 配置示例

```cpp
#include "PerformanceConfig.h"

void configurePerformance()
{
    PerformanceConfig config;
    
    // 启用监控
    config.setMonitoringEnabled(true);
    config.setMonitoringInterval(1000); // 1秒间隔
    
    // 设置阈值
    config.setCpuThreshold(80.0);        // CPU 80%
    config.setMemoryThreshold(1024);     // 内存 1GB
    config.setNetworkLatencyThreshold(100.0); // 延迟 100ms
    
    // 启用自动优化
    config.setAutoOptimizationEnabled(true);
    config.setOptimizationInterval(30000); // 30秒间隔
    
    // 保存配置
    config.saveConfig("performance.json");
}
```

### 监控示例

```cpp
#include "PerformanceManager.h"

class MonitoringExample : public QObject
{
    Q_OBJECT
    
public slots:
    void onMetricsUpdated(const PerformanceMetrics& metrics)
    {
        // 处理性能指标更新
        qDebug() << "CPU:" << metrics.system.cpuUsage << "%";
        qDebug() << "Memory:" << metrics.system.memoryUsage << "MB";
        qDebug() << "Network Latency:" << metrics.network.latency << "ms";
        
        // 检查性能等级
        if (metrics.system.cpuUsage > 90.0) {
            qWarning() << "High CPU usage detected!";
        }
    }
    
    void onThresholdExceeded(const QString& metric, double value, double threshold)
    {
        qWarning() << "Threshold exceeded:" << metric 
                   << "Value:" << value << "Threshold:" << threshold;
    }
};
```

### 优化示例

```cpp
#include "PerformanceManager.h"

void performOptimization()
{
    PerformanceManager manager;
    
    // 检查是否需要优化
    if (manager.shouldOptimize()) {
        qDebug() << "Optimization needed";
        
        // 执行优化
        bool result = manager.performOptimization();
        if (result) {
            qDebug() << "Optimization completed successfully";
            
            // 获取优化报告
            QVariantMap report = manager.generatePerformanceReport();
            qDebug() << "Optimization report:" << report;
        }
    }
}
```

## 编译和运行

### qmake方式
```bash
# 编译示例
qmake examples.pro
make

# 运行基础示例
./PerformanceExample

# 运行特定示例
./BasicMonitoringExample
```

### CMake方式
```bash
mkdir build
cd build
cmake ..
make

# 运行示例
./PerformanceExample
```

### 直接编译
```bash
# 编译单个示例
g++ -std=c++11 -I../include -I../interfaces \
    PerformanceExample.cpp -o PerformanceExample \
    -lQt5Core -lQt5Widgets -lperformance_module
```

## 示例说明

### 1. PerformanceExample.cpp
完整的性能模块使用演示，包括：
- 模块初始化和配置
- 性能监控启动
- 实时数据获取
- 优化执行
- 数据导出

**运行效果:**
```
========================================
Performance Module Example
========================================

1. Initializing Performance Module...
Module initialized: Success
Version: 1.0.0

2. Configuring Performance Monitoring...
Performance monitoring configured

3. Starting Performance Monitoring...
Performance monitoring started successfully

--- Demo Step 1 ---
Demonstrating Basic Monitoring...
Current Performance Metrics:
  CPU Usage: 25.3%
  Memory Usage: 512MB
  Network Latency: 15.2ms
```

### 2. BasicMonitoringExample.cpp
基础监控功能演示：
- CPU使用率监控
- 内存使用监控
- 网络状态监控
- 阈值检测

### 3. OptimizationExample.cpp
性能优化功能演示：
- 启动优化
- 内存优化
- 渲染优化
- 自动优化策略

### 4. UI示例
界面组件使用演示：
- 性能仪表板
- 实时图表
- 配置界面
- 报告生成

## 自定义示例

### 创建自定义监控器

```cpp
#include "BaseMonitor.h"

class CustomMonitor : public BaseMonitor
{
    Q_OBJECT
    
public:
    CustomMonitor(QObject* parent = nullptr)
        : BaseMonitor("CustomMonitor", parent)
    {
    }
    
protected:
    bool initializeMonitor() override
    {
        // 初始化自定义监控器
        return true;
    }
    
    ResourceUsage collectResourceUsage() override
    {
        ResourceUsage usage;
        // 收集自定义资源使用数据
        usage.timestamp = QDateTime::currentDateTime();
        return usage;
    }
    
    QList<ResourceType> supportedResourceTypes() const override
    {
        return {ResourceType::CPU, ResourceType::Memory};
    }
};
```

### 创建自定义优化器

```cpp
#include "BaseOptimizer.h"

class CustomOptimizer : public BaseOptimizer
{
    Q_OBJECT
    
public:
    CustomOptimizer(QObject* parent = nullptr)
        : BaseOptimizer("CustomOptimizer", parent)
    {
    }
    
protected:
    bool initializeOptimizer() override
    {
        // 初始化自定义优化器
        return true;
    }
    
    OptimizationResult performOptimization(OptimizationStrategy strategy) override
    {
        OptimizationResult result;
        result.success = true;
        result.optimizerName = "CustomOptimizer";
        result.description = "Custom optimization performed";
        result.timestamp = QDateTime::currentDateTime();
        
        // 执行自定义优化逻辑
        
        return result;
    }
    
    bool analyzeOptimizationNeed() const override
    {
        // 分析是否需要优化
        return true;
    }
};
```

## 性能测试示例

### 压力测试
```cpp
void performStressTest()
{
    PerformanceModule* module = PerformanceModule::instance();
    module->initialize();
    module->start();
    
    // 模拟高负载
    for (int i = 0; i < 1000; ++i) {
        // 创建CPU负载
        QThread::msleep(10);
        
        // 分配内存
        QByteArray data(1024 * 1024, 'X');
        
        // 获取性能指标
        PerformanceMetrics metrics = module->performanceManager()->getCurrentMetrics();
        
        if (i % 100 == 0) {
            qDebug() << "Iteration" << i << "CPU:" << metrics.system.cpuUsage << "%";
        }
    }
}
```

### 内存泄漏测试
```cpp
void memoryLeakTest()
{
    PerformanceModule* module = PerformanceModule::instance();
    
    qint64 initialMemory = 0;
    qint64 finalMemory = 0;
    
    // 记录初始内存
    {
        PerformanceMetrics metrics = module->performanceManager()->getCurrentMetrics();
        initialMemory = metrics.system.memoryUsage;
    }
    
    // 执行操作
    for (int i = 0; i < 1000; ++i) {
        module->performanceManager()->getCurrentMetrics();
    }
    
    // 记录最终内存
    {
        PerformanceMetrics metrics = module->performanceManager()->getCurrentMetrics();
        finalMemory = metrics.system.memoryUsage;
    }
    
    qint64 memoryDiff = finalMemory - initialMemory;
    qDebug() << "Memory difference:" << memoryDiff << "MB";
    
    if (memoryDiff > 10) { // 10MB阈值
        qWarning() << "Potential memory leak detected!";
    }
}
```

## 故障排除

### 常见问题

1. **编译错误**
   - 检查Qt版本 (需要5.15+)
   - 确保包含路径正确
   - 检查链接库

2. **运行时错误**
   - 检查模块初始化
   - 验证配置文件
   - 查看错误日志

3. **性能问题**
   - 调整监控间隔
   - 减少数据收集频率
   - 优化监控器实现

### 调试技巧
```cpp
// 启用调试输出
QLoggingCategory::setFilterRules("performance.*=true");

// 检查模块状态
PerformanceModule* module = PerformanceModule::instance();
qDebug() << "Module status:" << module->status();
qDebug() << "Module statistics:" << module->getStatistics();
```

## 扩展示例

### 集成到现有应用
```cpp
class MyApplication : public QApplication
{
    Q_OBJECT
    
public:
    MyApplication(int argc, char* argv[])
        : QApplication(argc, argv)
    {
        // 初始化性能模块
        initializePerformanceModule();
    }
    
private:
    void initializePerformanceModule()
    {
        m_performanceModule = PerformanceModule::instance();
        m_performanceModule->initialize();
        m_performanceModule->start();
        
        // 连接信号
        connect(m_performanceModule->performanceManager(),
                &PerformanceManager::thresholdExceeded,
                this, &MyApplication::handlePerformanceWarning);
    }
    
private slots:
    void handlePerformanceWarning(const QString& metric, double value, double threshold)
    {
        // 处理性能警告
        showNotification(QString("Performance warning: %1").arg(metric));
    }
    
private:
    PerformanceModule* m_performanceModule;
};
```

## 贡献指南

欢迎贡献新的示例！请遵循以下规范：

1. **代码风格**: 遵循项目代码风格
2. **文档**: 提供详细的注释和说明
3. **测试**: 确保示例可以正常编译和运行
4. **命名**: 使用描述性的文件名

## 许可证

所有示例代码遵循项目主许可证。