# 屏幕共享模块示例目录

本目录包含屏幕共享模块的使用示例和演示程序。

## 示例列表

- `BasicScreenShareExample.cpp` - 基础屏幕共享示例
- `AdvancedCaptureExample.cpp` - 高级捕获功能示例
- `CustomEncodingExample.cpp` - 自定义编码示例
- `UIIntegrationExample.cpp` - UI集成示例

## 编译示例

每个示例都可以独立编译：

```bash
# 编译基础示例
g++ -std=c++17 BasicScreenShareExample.cpp -lQt5Core -lQt5Gui -lQt5Widgets
```

## 示例说明

### BasicScreenShareExample
演示如何使用屏幕共享管理器进行基本的屏幕捕获和共享。

### AdvancedCaptureExample
展示高级捕获功能，包括区域选择、质量控制和性能优化。

### CustomEncodingExample
演示如何自定义编码参数和处理编码数据。

### UIIntegrationExample
展示如何将屏幕共享功能集成到用户界面中。