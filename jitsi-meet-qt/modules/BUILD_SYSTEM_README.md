# Jitsi Meet Qt Optimized Build System

## 概述

这是Jitsi Meet Qt项目的优化构建系统，提供了模块化、条件编译、动态加载、打包和分发的完整解决方案。

## 主要特性

### 🚀 构建系统优化
- **条件编译**: 根据需要启用/禁用模块和特性
- **动态加载**: 支持模块的运行时加载和卸载
- **并行编译**: 多核CPU并行编译支持
- **增量构建**: 只重新编译修改的文件
- **预编译头**: 加速编译过程
- **ccache支持**: 编译缓存以提高重复构建速度

### 📦 打包和分发
- **模块打包**: 独立的模块包创建
- **完整分发包**: 包含所有组件的分发包
- **跨平台安装程序**: Windows (NSIS), Linux (DEB/RPM), macOS (DMG)
- **包验证**: 自动验证包的完整性和依赖关系

### 🔧 模块管理
- **模块安装/卸载**: 独立模块的安装和卸载工具
- **依赖管理**: 自动处理模块间的依赖关系
- **版本控制**: 模块版本管理和兼容性检查
- **热重载**: 开发模式下的模块热重载支持

## 文件结构

```
modules/
├── build_system.py              # 主构建系统脚本
├── modules_optimized.pri        # 优化的模块配置文件
├── conditional_compilation.pri  # 条件编译配置
├── dynamic_loading.pri         # 动态加载配置
├── packaging.pri               # 打包配置
├── build_config.json          # 构建配置
├── build_optimization.json    # 构建优化配置
├── tools/                     # 构建工具
│   ├── install_module.py      # 模块安装工具
│   ├── uninstall_module.py    # 模块卸载工具
│   ├── generate_plugin_metadata.py # 插件元数据生成器
│   ├── verify_packages.py     # 包验证工具
│   └── optimize_build.py      # 构建优化工具
└── BUILD_SYSTEM_README.md     # 本文档
```

## 快速开始

### 1. 完整构建

```bash
# 完整构建流程（推荐）
python modules/build_system.py /path/to/jitsi-meet-qt build

# 或者使用优化构建
python modules/tools/optimize_build.py /path/to/jitsi-meet-qt build
```

### 2. 条件编译

```bash
# 禁用特定模块
export JITSI_DISABLE_MODULES="audio,chat"
qmake jitsi-meet-qt.pro

# 启用实验性特性
export JITSI_EXPERIMENTAL_FEATURES="ai_enhancement,virtual_background"
qmake jitsi-meet-qt.pro

# 启用特定特性
export JITSI_FEATURES="hd_video,hardware_acceleration,p2p_mode"
qmake jitsi-meet-qt.pro
```

### 3. 动态加载

```bash
# 启用动态加载支持
qmake CONFIG+=dynamic_loading_support jitsi-meet-qt.pro
make

# 生成插件元数据
python modules/tools/generate_plugin_metadata.py build/plugins build/plugins/metadata/plugins.json
```

### 4. 打包

```bash
# 创建所有模块包
make package_all

# 创建分发包
make create_distribution

# 验证包
python modules/tools/verify_packages.py build/packages
```

## 详细配置

### 构建配置 (build_config.json)

```json
{
  "version": "2.1.0",
  "build_type": "release",
  "enable_optimizations": true,
  "modules": {
    "audio": {"enabled": true, "required": false},
    "network": {"enabled": true, "required": false}
  },
  "features": {
    "hd_video": {"enabled": true},
    "hardware_acceleration": {"enabled": true}
  }
}
```

### 优化配置 (build_optimization.json)

```json
{
  "parallel_jobs": 8,
  "use_ccache": true,
  "use_precompiled_headers": true,
  "optimize_level": "O3",
  "link_time_optimization": true
}
```

## 模块管理

### 安装模块

```bash
# 安装单个模块
python modules/tools/install_module.py install audio-module-1.0.0.tar.gz

# 强制重新安装
python modules/tools/install_module.py install audio-module-1.0.0.tar.gz --force

# 列出已安装模块
python modules/tools/install_module.py list
```

### 卸载模块

```bash
# 卸载单个模块
python modules/tools/uninstall_module.py uninstall audio

# 强制卸载（忽略依赖）
python modules/tools/uninstall_module.py uninstall audio --force

# 卸载所有模块
python modules/tools/uninstall_module.py uninstall-all
```

## 条件编译特性

### 模块控制

- `JITSI_DISABLE_MODULES`: 禁用指定模块
- `JITSI_EXPERIMENTAL_FEATURES`: 启用实验性特性
- `JITSI_FEATURES`: 启用特定特性

### 平台特定特性

#### Windows
- `windows_integration`: Windows系统集成
- `directshow`: DirectShow支持

#### Linux
- `pulseaudio`: PulseAudio支持
- `x11_integration`: X11集成

#### macOS
- `coreaudio`: Core Audio支持

### 特性标志

- `advanced_audio`: 高级音频处理
- `noise_suppression`: 噪音抑制
- `hd_video`: 高清视频支持
- `hardware_acceleration`: 硬件加速
- `p2p_mode`: P2P模式
- `end_to_end_encryption`: 端到端加密

## 动态加载

### 插件配置

动态加载系统支持运行时加载和卸载模块插件：

```cpp
// 加载插件
PluginLoader loader;
if (loader.loadPlugin("audio_module_plugin")) {
    // 使用插件
}

// 卸载插件
loader.unloadPlugin("audio_module_plugin");
```

### 热重载（开发模式）

```bash
# 启用热重载
qmake CONFIG+=debug CONFIG+=dynamic_loading_support
```

## 打包系统

### 创建模块包

```bash
# 单个模块包
make package_audio

# 所有模块包
make package_all
```

### 创建安装程序

#### Windows
```bash
make create_installer_windows
```

#### Linux
```bash
make create_deb_package
make create_rpm_package
```

#### macOS
```bash
make create_dmg
```

## 性能优化

### 编译优化

1. **并行编译**: 使用多核CPU
2. **ccache**: 编译缓存
3. **预编译头**: 减少编译时间
4. **链接时优化**: LTO支持
5. **增量构建**: 只编译修改的文件

### 运行时优化

1. **动态加载**: 按需加载模块
2. **资源压缩**: 减少内存使用
3. **符号剥离**: 减少二进制大小

## 故障排除

### 常见问题

1. **编译失败**
   ```bash
   # 清理构建缓存
   python modules/tools/optimize_build.py /path/to/project clean
   
   # 重新构建
   python modules/build_system.py /path/to/project build
   ```

2. **模块依赖错误**
   ```bash
   # 检查依赖关系
   python modules/tools/verify_packages.py build/packages
   ```

3. **插件加载失败**
   ```bash
   # 验证插件元数据
   python modules/tools/generate_plugin_metadata.py build/plugins build/plugins/metadata/plugins.json --validate
   ```

### 调试模式

```bash
# 启用调试日志
export JITSI_FEATURES="debug_logging"
qmake CONFIG+=debug

# 启用性能分析
export JITSI_FEATURES="performance_profiling"
```

## 持续集成

### CI构建

```bash
# CI构建目标
make ci_build

# CI测试
make ci_test
```

### 自动化部署

构建系统支持自动化部署到各种平台：

- AppImage (Linux)
- Flatpak (Linux)
- Snap (Linux)
- 软件仓库上传

## 开发指南

### 添加新模块

1. 创建模块目录结构
2. 编写模块.pri文件
3. 更新modules.pri
4. 添加依赖关系
5. 创建测试

### 添加新特性

1. 在conditional_compilation.pri中定义特性标志
2. 在代码中使用条件编译
3. 更新build_config.json
4. 添加文档

## 许可证

本构建系统遵循与Jitsi Meet Qt项目相同的许可证。

## 贡献

欢迎提交问题报告和功能请求。请确保：

1. 遵循代码风格
2. 添加适当的测试
3. 更新文档
4. 验证构建系统正常工作

## 联系方式

如有问题或建议，请通过项目的GitHub仓库联系我们。