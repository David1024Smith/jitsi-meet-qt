# 网络模块示例

## 概述

本目录包含网络模块的使用示例，展示如何在实际应用中使用网络模块的各种功能。

## 示例列表

### 基础示例

1. **basic_connection.cpp** - 基本网络连接示例
2. **network_config.cpp** - 网络配置管理示例
3. **connection_factory.cpp** - 连接工厂使用示例

### 协议示例

4. **webrtc_example.cpp** - WebRTC协议使用示例
5. **http_client.cpp** - HTTP客户端示例
6. **websocket_client.cpp** - WebSocket客户端示例

### 高级示例

7. **network_monitor.cpp** - 网络状态监控示例
8. **multi_protocol.cpp** - 多协议支持示例
9. **performance_test.cpp** - 性能测试示例

### UI示例

10. **network_status_widget.cpp** - 网络状态组件示例
11. **connection_dialog.cpp** - 连接对话框示例

## 编译和运行

### 使用qmake

```bash
# 编译单个示例
qmake basic_connection.pro
make
./basic_connection

# 编译所有示例
qmake examples.pro
make
```

### 使用CMake

```bash
mkdir build && cd build
cmake ..
make
./basic_connection
```

## 示例详情

### 1. 基本网络连接 (basic_connection.cpp)

展示如何创建基本的网络连接：

```cpp
#include "NetworkManager.h"
#include "NetworkConfig.h"

int main() {
    // 创建网络管理器
    NetworkManager* manager = NetworkManager::instance();
    
    // 配置网络参数
    NetworkConfig config;
    config.setServerUrl("https://meet.jit.si");
    config.setConnectionTimeout(30000);
    
    // 初始化并连接
    if (manager->initialize(config.toVariantMap())) {
        manager->connectToServer();
    }
    
    return 0;
}
```

### 2. 网络配置管理 (network_config.cpp)

展示如何管理网络配置：

```cpp
#include "NetworkConfig.h"

int main() {
    NetworkConfig config;
    
    // 设置服务器配置
    config.setServerUrl("https://your-server.com");
    config.setServerPort(443);
    
    // 设置连接选项
    config.setAutoReconnect(true);
    config.setReconnectInterval(5000);
    
    // 设置协议支持
    config.setWebRTCEnabled(true);
    config.setWebSocketEnabled(true);
    
    // 保存配置
    config.saveToFile("network_config.json");
    
    return 0;
}
```

### 3. 连接工厂使用 (connection_factory.cpp)

展示如何使用连接工厂创建不同类型的连接：

```cpp
#include "ConnectionFactory.h"

int main() {
    ConnectionFactory* factory = ConnectionFactory::instance();
    
    // 创建WebRTC连接
    auto webrtcConnection = factory->createConnection(
        ConnectionFactory::WebRTC,
        {{"stunServers", QStringList{"stun:stun.l.google.com:19302"}}}
    );
    
    // 创建WebSocket连接
    auto wsConnection = factory->createConnection(
        ConnectionFactory::WebSocketSecure,
        {{"url", "wss://your-server.com/ws"}}
    );
    
    return 0;
}
```

### 4. WebRTC协议示例 (webrtc_example.cpp)

展示如何使用WebRTC协议进行实时通信：

```cpp
#include "WebRTCProtocol.h"

int main() {
    WebRTCProtocol webrtc;
    
    // 配置STUN/TURN服务器
    webrtc.setStunServers({"stun:stun.l.google.com:19302"});
    
    // 初始化协议
    webrtc.initialize();
    webrtc.start();
    
    // 创建Offer
    QString offer = webrtc.createOffer();
    
    // 处理Answer
    connect(&webrtc, &WebRTCProtocol::remoteDescriptionReceived,
            [&](const QString& sdp) {
        webrtc.setRemoteDescription(sdp);
    });
    
    return 0;
}
```

### 5. HTTP客户端示例 (http_client.cpp)

展示如何使用HTTP协议进行API调用：

```cpp
#include "HTTPProtocol.h"

int main() {
    HTTPProtocol http;
    
    // 设置基础URL和默认头
    http.setBaseUrl("https://api.example.com");
    http.setDefaultHeaders({
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer your-token"}
    });
    
    // 发送GET请求
    QString requestId = http.get("/users");
    
    // 处理响应
    connect(&http, &HTTPProtocol::requestCompleted,
            [](const QString& id, int statusCode, const QByteArray& data) {
        qDebug() << "Response:" << statusCode << data;
    });
    
    return 0;
}
```

### 6. WebSocket客户端示例 (websocket_client.cpp)

展示如何使用WebSocket进行实时通信：

```cpp
#include "WebSocketProtocol.h"

int main() {
    WebSocketProtocol ws;
    
    // 配置WebSocket
    ws.setServerUrl("wss://your-server.com/ws");
    ws.setAutoReconnect(true);
    ws.setHeartbeatInterval(30000);
    
    // 连接到服务器
    ws.connectToServer("wss://your-server.com/ws");
    
    // 处理消息
    connect(&ws, &WebSocketProtocol::textMessageReceived,
            [&](const QString& message) {
        qDebug() << "Received:" << message;
        ws.sendTextMessage("Echo: " + message);
    });
    
    return 0;
}
```

### 7. 网络状态监控 (network_monitor.cpp)

展示如何监控网络状态和质量：

```cpp
#include "NetworkManager.h"
#include "NetworkUtils.h"

int main() {
    NetworkManager* manager = NetworkManager::instance();
    
    // 监控连接状态
    connect(manager, &NetworkManager::connectionStateChanged,
            [](NetworkManager::ConnectionState state) {
        qDebug() << "Connection state:" << state;
    });
    
    // 监控网络质量
    connect(manager, &NetworkManager::networkQualityChanged,
            [](NetworkManager::NetworkQuality quality) {
        qDebug() << "Network quality:" << quality;
    });
    
    // 获取网络统计信息
    QVariantMap stats = NetworkUtils::getNetworkStats();
    qDebug() << "Network stats:" << stats;
    
    return 0;
}
```

### 8. 多协议支持 (multi_protocol.cpp)

展示如何同时使用多种网络协议：

```cpp
#include "NetworkManager.h"
#include "WebRTCProtocol.h"
#include "HTTPProtocol.h"
#include "WebSocketProtocol.h"

int main() {
    // 创建协议处理器
    WebRTCProtocol webrtc;
    HTTPProtocol http;
    WebSocketProtocol ws;
    
    // 初始化所有协议
    webrtc.initialize();
    http.initialize();
    ws.initialize();
    
    // 启动协议
    webrtc.start();
    http.start();
    ws.start();
    
    // 使用不同协议进行通信
    webrtc.createOffer();
    http.get("/api/status");
    ws.connectToServer("wss://server.com/ws");
    
    return 0;
}
```

### 9. 性能测试 (performance_test.cpp)

展示如何进行网络性能测试：

```cpp
#include "NetworkManager.h"
#include "NetworkUtils.h"
#include <QElapsedTimer>

int main() {
    NetworkManager* manager = NetworkManager::instance();
    
    // 测试连接延迟
    QElapsedTimer timer;
    timer.start();
    
    if (manager->connectToServer("https://test-server.com")) {
        qint64 latency = timer.elapsed();
        qDebug() << "Connection latency:" << latency << "ms";
    }
    
    // 测试网络质量
    int quality = NetworkUtils::calculateNetworkQuality(
        manager->networkLatency(),
        0.1,  // 丢包率
        manager->bandwidth()
    );
    qDebug() << "Network quality score:" << quality;
    
    return 0;
}
```

### 10. 网络状态组件 (network_status_widget.cpp)

展示如何使用网络状态UI组件：

```cpp
#include "NetworkStatusWidget.h"
#include "NetworkManager.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 创建网络状态组件
    NetworkStatusWidget statusWidget;
    statusWidget.setNetworkManager(NetworkManager::instance());
    statusWidget.setDisplayMode(NetworkStatusWidget::Detailed);
    statusWidget.setAutoUpdate(true);
    
    // 显示组件
    statusWidget.show();
    
    return app.exec();
}
```

### 11. 连接对话框 (connection_dialog.cpp)

展示如何使用连接控制UI组件：

```cpp
#include "ConnectionWidget.h"
#include "NetworkManager.h"
#include "NetworkConfig.h"
#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 创建对话框
    QDialog dialog;
    dialog.setWindowTitle("Network Connection");
    
    // 创建连接组件
    ConnectionWidget connectionWidget;
    connectionWidget.setNetworkManager(NetworkManager::instance());
    
    // 设置布局
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(&connectionWidget);
    
    // 显示对话框
    dialog.exec();
    
    return 0;
}
```

## 构建配置

### examples.pro (qmake)

```qmake
TEMPLATE = subdirs

SUBDIRS += \
    basic_connection \
    network_config \
    connection_factory \
    webrtc_example \
    http_client \
    websocket_client \
    network_monitor \
    multi_protocol \
    performance_test \
    network_status_widget \
    connection_dialog

# 设置依赖关系
basic_connection.depends = ../network.pri
network_config.depends = ../network.pri
# ... 其他依赖
```

### CMakeLists.txt (CMake)

```cmake
cmake_minimum_required(VERSION 3.16)
project(NetworkExamples)

find_package(Qt5 REQUIRED COMPONENTS Core Network WebSockets)

# 包含网络模块
include_directories(../include)
include_directories(../interfaces)

# 添加示例可执行文件
add_executable(basic_connection basic_connection.cpp)
target_link_libraries(basic_connection Qt5::Core Qt5::Network)

add_executable(network_config network_config.cpp)
target_link_libraries(network_config Qt5::Core Qt5::Network)

# ... 其他示例
```

## 运行要求

- Qt 5.15 或更高版本
- C++17 支持
- 网络连接（部分示例需要）
- 测试服务器（可选）

## 注意事项

1. 某些示例需要有效的网络连接
2. WebRTC示例可能需要STUN/TURN服务器
3. 部分示例使用模拟数据，可以离线运行
4. UI示例需要图形界面环境

## 扩展示例

可以基于这些示例创建更复杂的应用：

- 视频会议客户端
- 实时聊天应用
- 网络监控工具
- API测试工具
- 文件传输客户端