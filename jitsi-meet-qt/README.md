# Jitsi Meet Qt

Qt版本的Jitsi Meet桌面应用程序，基于原始的Electron版本重新实现。

## 功能特性

- 🎥 完整的Jitsi Meet视频会议功能
- 🖥️ 原生Qt界面，更好的性能和用户体验
- 🔗 支持深度链接协议 (`jitsi-meet://`)
- 📝 最近会议历史记录
- ⚙️ 可配置的服务器设置
- 🌍 多语言支持
- 🎨 深色/浅色主题支持
- 💾 窗口状态记忆功能

## 系统要求

- Windows 10 或更高版本
- Qt 6.8.3 或更高版本
- MinGW 编译器
- CMake 3.20 或更高版本

## 构建说明

### 使用构建脚本

```cmd
build.bat
```

### 手动构建

```cmd
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

## 使用的现代C++17特性

- `std::string_view` 高效字符串处理
- `std::optional` 可选返回值
- `std::unique_ptr` 自动内存管理
- `constexpr` 编译时常量
- `[[nodiscard]]` 属性标记重要返回值
- `noexcept` 异常安全规范
- 结构化绑定和auto类型推导

## 项目结构

```
jitsi-meet-qt/
├── src/                    # 源代码 (.cpp files)
│   ├── main.cpp           # 程序入口
│   └── MainApplication.cpp # 主应用程序类实现
├── include/               # 头文件 (.h files)
│   └── MainApplication.h  # 主应用程序类声明
├── resources/             # 资源文件 (图标、图片等)
├── tests/                 # 单元测试和集成测试
│   ├── CMakeLists.txt    # 测试构建配置
│   ├── test_main.cpp     # 测试运行器
│   └── test_MainApplication.cpp # MainApplication测试
├── build/                 # 构建输出目录 (构建时创建)
├── CMakeLists.txt        # 主CMake配置
├── build.bat             # Windows构建脚本
└── README.md             # 说明文件
```

## 已实现功能

### 任务1: 项目结构和核心框架 ✅

- ✅ Qt项目目录结构 (src, include, resources, tests)
- ✅ CMakeLists.txt配置Qt 6.8.3、WebSocket、Multimedia、Network依赖
- ✅ C++17标准和MinGW编译器配置
- ✅ MainApplication类使用现代C++17特性
- ✅ 单例模式确保应用程序只有一个实例运行

## 满足的需求

- **需求 1.1**: 应用程序启动时显示欢迎界面
- **需求 1.2**: 使用Qt 6.8.3框架和C++17标准
- **需求 1.3**: 显示应用程序标题"Jitsi Meet"

## 开发说明

本项目使用C++17标准，遵循现代C++编程规范。当前已实现的主要组件：

- **MainApplication**: 应用程序入口点和单例管理，支持单实例运行
  - 使用现代C++17特性如`std::string_view`、`std::optional`
  - 实现单例模式确保只有一个应用程序实例
  - 支持协议URL处理和实例间通信

## 协议支持

应用程序支持 `jitsi-meet://` 协议链接：

- `jitsi-meet://room-name` - 使用默认服务器加入房间
- `jitsi-meet://server.com/room-name` - 使用指定服务器加入房间

