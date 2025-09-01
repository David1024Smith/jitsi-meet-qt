# 网络模块 (Network Module)

## 概述

网络模块负责管理Jitsi Meet Qt应用程序的所有网络通信功能，包括WebRTC连接、HTTP请求、WebSocket通信以及网络状态监控。

## 版本信息

- **模块版本**: 1.0.0
- **兼容性**: Qt 5.15+
- **依赖**: WebRTC, Qt Network

## 功能特性

### 核心功能
- 网络连接管理
- 多协议支持 (WebRTC, HTTP, WebSocket)
- 网络质量监控
- 自动重连机制
- 连接状态跟踪

### 支持的协议
- **WebRTC**: 实时音视频通信
- **HTTP/HTTPS**: RESTful API调用
- **WebSocket**: 实时消息传输
- **XMPP**: 聊天和信令

## 目录结构

```
network/
├── network.pri              # 模块配置文件
├── README.md               # 本文档
├── include/                # 核心头文件
│   ├── NetworkModule.h     # 底层网络控制
│   ├── NetworkManager.h    # 网络管理器
│   └── ConnectionFactory.h # 连接工厂
├── src/                    # 核心实现
├── interfaces/             # 接口定义
│   ├── INetworkManager.h   # 网络管理器接口
│   ├── IConnectionHandler.h # 连接处理接口
│   └── IProtocolHandler.h  # 协议处理接口
├── config/                 # 配置管理
│   ├── NetworkConfig.h     # 网络配置类
│   └── NetworkConfig.cpp
├── utils/                  # 工具类
│   ├── NetworkUtils.h      # 网络工具
│   └── NetworkUtils.cpp
├── widgets/                # UI组件
│   ├── NetworkStatusWidget.h # 网络状态组件
│   └── ConnectionWidget.h  # 连接控制组件
├── protocols/              # 协议实现
│   ├── WebRTCProtocol.h    # WebRTC协议
│   ├── HTTPProtocol.h      # HTTP协议
│   └── WebSocketProtocol.h # WebSocket协议
├── tests/                  # 测试框架
├── examples/               # 示例代码
└── resources/              # 资源文件
```

## 快速开始

### 基本使用

```cpp
#include "NetworkManager.h"
#include "NetworkConfig.h"

// 创建网络管理器
auto networkManager = NetworkManager::instance();

// 配置网络参数
NetworkConfig config;
config.setServerUrl("https://meet.jit.si");
config.setConnectionTimeout(30000);

// 初始化网络模块
if (networkManager->initialize(config)) {
    // 连接到服务器
    networkManager->connectToServer();
}
```

### 监听网络状态

```cpp
// 连接信号
connect(networkManager, &INetworkManager::connectionStateChanged,
        this, [](INetworkManager::ConnectionState state) {
    switch (state) {
        case INetworkManager::Connected:
            qDebug() << "网络已连接";
            break;
        case INetworkManager::Disconnected:
            qDebug() << "网络已断开";
            break;
        case INetworkManager::Error:
            qDebug() << "网络连接错误";
            break;
    }
});
```

## API 参考

### 核心接口

#### INetworkManager
- `bool initialize()` - 初始化网络管理器
- `ConnectionState connectionState() const` - 获取连接状态
- `bool connectToServer(const QString& serverUrl)` - 连接到服务器
- `void disconnect()` - 断开连接

#### IConnectionHandler
- `bool establishConnection()` - 建立连接
- `void closeConnection()` - 关闭连接
- `bool isConnected() const` - 检查连接状态

## 配置选项

### NetworkConfig 配置项

```cpp
// 服务器配置
config.setServerUrl("https://your-server.com");
config.setServerPort(443);

// 连接配置
config.setConnectionTimeout(30000);  // 30秒超时
config.setRetryAttempts(3);          // 重试3次
config.setRetryInterval(5000);       // 重试间隔5秒

// 协议配置
config.setWebRTCEnabled(true);
config.setWebSocketEnabled(true);
config.setHTTPSOnly(true);
```

## 测试

运行网络模块测试：

```bash
# 编译测试
qmake network_tests.pro
make

# 运行测试
./network_tests
```

## 故障排除

### 常见问题

1. **连接超时**
   - 检查网络连接
   - 验证服务器地址和端口
   - 调整超时设置

2. **WebRTC连接失败**
   - 检查防火墙设置
   - 验证STUN/TURN服务器配置
   - 检查网络NAT类型

3. **SSL证书错误**
   - 验证服务器证书
   - 检查系统时间
   - 更新CA证书

## 开发指南

### 添加新协议支持

1. 创建协议处理器类继承 `IProtocolHandler`
2. 实现协议特定的连接逻辑
3. 在 `ConnectionFactory` 中注册新协议
4. 添加相应的配置选项

### 性能优化建议

- 使用连接池管理多个连接
- 实现智能重连策略
- 优化数据传输格式
- 启用网络压缩

## 许可证

本模块遵循项目主许可证。

## 贡献

欢迎提交问题报告和功能请求到项目仓库。