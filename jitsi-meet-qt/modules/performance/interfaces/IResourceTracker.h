#ifndef IRESOURCETRACKER_H
#define IRESOURCETRACKER_H

#include <QObject>
#include <QVariantMap>
#include <QDateTime>

/**
 * @brief 资源使用信息结构
 */
struct ResourceUsage {
    struct CPUUsage {
        double totalUsage = 0.0;        ///< 总CPU使用率 (%)
        double userUsage = 0.0;         ///< 用户态CPU使用率 (%)
        double systemUsage = 0.0;       ///< 系统态CPU使用率 (%)
        double idleUsage = 0.0;         ///< 空闲CPU使用率 (%)
        int coreCount = 0;              ///< CPU核心数
        double frequency = 0.0;         ///< CPU频率 (GHz)
        double temperature = 0.0;       ///< CPU温度 (°C)
    };
    
    struct MemoryUsage {
        qint64 totalMemory = 0;         ///< 总内存 (bytes)
        qint64 usedMemory = 0;          ///< 已使用内存 (bytes)
        qint64 freeMemory = 0;          ///< 空闲内存 (bytes)
        qint64 cachedMemory = 0;        ///< 缓存内存 (bytes)
        qint64 bufferMemory = 0;        ///< 缓冲内存 (bytes)
        qint64 swapTotal = 0;           ///< 总交换空间 (bytes)
        qint64 swapUsed = 0;            ///< 已使用交换空间 (bytes)
        double usagePercentage = 0.0;   ///< 内存使用率 (%)
    };
    
    struct DiskUsage {
        qint64 totalSpace = 0;          ///< 总磁盘空间 (bytes)
        qint64 usedSpace = 0;           ///< 已使用空间 (bytes)
        qint64 freeSpace = 0;           ///< 空闲空间 (bytes)
        double usagePercentage = 0.0;   ///< 磁盘使用率 (%)
        double readSpeed = 0.0;         ///< 读取速度 (MB/s)
        double writeSpeed = 0.0;        ///< 写入速度 (MB/s)
        qint64 readBytes = 0;           ///< 累计读取字节数
        qint64 writeBytes = 0;          ///< 累计写入字节数
    };
    
    struct NetworkUsage {
        qint64 bytesReceived = 0;       ///< 接收字节数
        qint64 bytesSent = 0;           ///< 发送字节数
        qint64 packetsReceived = 0;     ///< 接收包数
        qint64 packetsSent = 0;         ///< 发送包数
        double receiveSpeed = 0.0;      ///< 接收速度 (MB/s)
        double sendSpeed = 0.0;         ///< 发送速度 (MB/s)
        int connectionCount = 0;        ///< 连接数
        double latency = 0.0;           ///< 网络延迟 (ms)
    };
    
    struct ProcessUsage {
        qint64 processId = 0;           ///< 进程ID
        QString processName;            ///< 进程名称
        double cpuUsage = 0.0;          ///< 进程CPU使用率 (%)
        qint64 memoryUsage = 0;         ///< 进程内存使用 (bytes)
        int threadCount = 0;            ///< 线程数
        int handleCount = 0;            ///< 句柄数
        QDateTime startTime;            ///< 启动时间
    };
    
    CPUUsage cpu;
    MemoryUsage memory;
    DiskUsage disk;
    NetworkUsage network;
    ProcessUsage process;
    QDateTime timestamp;
};

Q_DECLARE_METATYPE(ResourceUsage)

/**
 * @brief 资源跟踪接口
 * 
 * IResourceTracker定义了系统资源跟踪的标准接口，用于监控和跟踪系统资源使用情况。
 */
class IResourceTracker
{
public:
    /**
     * @brief 资源类型枚举
     */
    enum ResourceType {
        CPU,            ///< CPU资源
        Memory,         ///< 内存资源
        Disk,           ///< 磁盘资源
        Network,        ///< 网络资源
        Process,        ///< 进程资源
        All             ///< 所有资源
    };

    /**
     * @brief 虚析构函数
     */
    virtual ~IResourceTracker() = default;

    /**
     * @brief 初始化资源跟踪器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 启动资源跟踪
     * @return 启动是否成功
     */
    virtual bool startTracking() = 0;

    /**
     * @brief 停止资源跟踪
     */
    virtual void stopTracking() = 0;

    /**
     * @brief 检查是否正在跟踪
     * @return 是否正在跟踪
     */
    virtual bool isTracking() const = 0;

    /**
     * @brief 获取当前资源使用情况
     * @param type 资源类型
     * @return 资源使用信息
     */
    virtual ResourceUsage getCurrentUsage(ResourceType type = All) const = 0;

    /**
     * @brief 获取历史资源使用情况
     * @param type 资源类型
     * @param from 开始时间
     * @param to 结束时间
     * @return 历史资源使用信息列表
     */
    virtual QList<ResourceUsage> getHistoricalUsage(ResourceType type, const QDateTime& from, const QDateTime& to) const = 0;

    /**
     * @brief 获取资源使用峰值
     * @param type 资源类型
     * @param from 开始时间
     * @param to 结束时间
     * @return 峰值资源使用信息
     */
    virtual ResourceUsage getPeakUsage(ResourceType type, const QDateTime& from, const QDateTime& to) const = 0;

    /**
     * @brief 获取资源使用平均值
     * @param type 资源类型
     * @param from 开始时间
     * @param to 结束时间
     * @return 平均资源使用信息
     */
    virtual ResourceUsage getAverageUsage(ResourceType type, const QDateTime& from, const QDateTime& to) const = 0;

    /**
     * @brief 设置跟踪间隔
     * @param interval 间隔时间(毫秒)
     */
    virtual void setTrackingInterval(int interval) = 0;

    /**
     * @brief 获取跟踪间隔
     * @return 间隔时间(毫秒)
     */
    virtual int trackingInterval() const = 0;

    /**
     * @brief 设置资源阈值
     * @param type 资源类型
     * @param threshold 阈值
     */
    virtual void setResourceThreshold(ResourceType type, double threshold) = 0;

    /**
     * @brief 获取资源阈值
     * @param type 资源类型
     * @return 阈值
     */
    virtual double resourceThreshold(ResourceType type) const = 0;

    /**
     * @brief 检查资源是否超出阈值
     * @param type 资源类型
     * @return 是否超出阈值
     */
    virtual bool isThresholdExceeded(ResourceType type) const = 0;

    /**
     * @brief 获取系统信息
     * @return 系统信息
     */
    virtual QVariantMap getSystemInfo() const = 0;

    /**
     * @brief 获取跟踪器状态
     * @return 状态信息
     */
    virtual QVariantMap getTrackerStatus() const = 0;

    /**
     * @brief 重置跟踪器
     */
    virtual void reset() = 0;

    /**
     * @brief 清除历史数据
     * @param olderThan 清除指定时间之前的数据
     */
    virtual void clearHistoricalData(const QDateTime& olderThan = QDateTime()) = 0;
};

Q_DECLARE_INTERFACE(IResourceTracker, "org.jitsi.performance.IResourceTracker/1.0")

#endif // IRESOURCETRACKER_H