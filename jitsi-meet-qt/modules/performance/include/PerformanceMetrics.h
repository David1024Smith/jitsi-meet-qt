#ifndef PERFORMANCEMETRICS_H
#define PERFORMANCEMETRICS_H

#include <QDateTime>
#include <QVariantMap>
#include <QJsonObject>
#include <QSize>

/**
 * @brief 性能指标数据结构
 * 
 * 包含系统性能的各项指标数据
 */
struct PerformanceMetrics
{
    // 时间戳
    QDateTime timestamp;
    
    // 系统指标结构
    struct SystemMetrics {
        double cpuUsage = 0.0;          // CPU使用率 (0-100%)
        double cpuTemperature = 0.0;    // CPU温度 (摄氏度)
        int cpuCores = 0;               // CPU核心数
        quint64 memoryUsed = 0;         // 已使用内存 (字节)
        quint64 memoryTotal = 0;        // 总内存 (字节)
        double memoryUsage = 0.0;       // 内存使用率 (0-100%)
        int threadCount = 0;            // 线程数量
        quint64 handleCount = 0;        // 句柄数量
    } system;
    
    // 网络指标结构
    struct NetworkMetrics {
        quint64 bytesReceived = 0;      // 接收字节数
        quint64 bytesSent = 0;          // 发送字节数
        double latency = 0.0;           // 网络延迟 (毫秒)
        double bandwidth = 0.0;         // 带宽 (Mbps)
        double packetLoss = 0.0;        // 丢包率 (0-100%)
        int connectionQuality = 0;      // 连接质量 (0-100)
    } network;
    
    // 音频指标结构
    struct AudioMetrics {
        double latency = 0.0;           // 音频延迟 (毫秒)
        double jitter = 0.0;            // 抖动 (毫秒)
        double packetLoss = 0.0;        // 丢包率 (0-100%)
        int sampleRate = 0;             // 采样率 (Hz)
        int bitrate = 0;                // 比特率 (kbps)
    } audio;
    
    // 视频指标结构
    struct VideoMetrics {
        double frameRate = 0.0;         // 帧率 (FPS)
        QSize resolution;               // 分辨率
        int bitrate = 0;                // 比特率 (kbps)
        double encodingTime = 0.0;      // 编码时间 (毫秒)
        double decodingTime = 0.0;      // 解码时间 (毫秒)
    } video;
    
    // 兼容性字段 (保持向后兼容)
    double cpuUsage = 0.0;          // CPU使用率 (0-100%)
    double cpuTemperature = 0.0;    // CPU温度 (摄氏度)
    int cpuCores = 0;               // CPU核心数
    
    // 内存相关指标
    quint64 memoryUsed = 0;         // 已使用内存 (字节)
    quint64 memoryTotal = 0;        // 总内存 (字节)
    double memoryUsage = 0.0;       // 内存使用率 (0-100%)
    
    // 网络相关指标
    quint64 networkBytesReceived = 0;   // 接收字节数
    quint64 networkBytesSent = 0;       // 发送字节数
    double networkLatency = 0.0;        // 网络延迟 (毫秒)
    
    // 应用程序相关指标
    int threadCount = 0;            // 线程数量
    quint64 handleCount = 0;        // 句柄数量
    double frameRate = 0.0;         // 帧率 (FPS)
    
    // 磁盘相关指标
    quint64 diskReadBytes = 0;      // 磁盘读取字节数
    quint64 diskWriteBytes = 0;     // 磁盘写入字节数
    double diskUsage = 0.0;         // 磁盘使用率 (0-100%)
    
    // 构造函数
    PerformanceMetrics() = default;
    
    // 拷贝构造函数
    PerformanceMetrics(const PerformanceMetrics& other) = default;
    
    // 赋值操作符
    PerformanceMetrics& operator=(const PerformanceMetrics& other) = default;
    
    // 转换为 QVariantMap
    QVariantMap toVariantMap() const;
    
    // 从 QVariantMap 构造
    static PerformanceMetrics fromVariantMap(const QVariantMap& map);
    
    // 转换为 JSON
    QJsonObject toJson() const;
    
    // 从 JSON 构造
    static PerformanceMetrics fromJson(const QJsonObject& json);
    
    // 重置所有指标
    void reset();
    
    // 检查指标是否有效
    bool isValid() const;
};

#endif // PERFORMANCEMETRICS_H