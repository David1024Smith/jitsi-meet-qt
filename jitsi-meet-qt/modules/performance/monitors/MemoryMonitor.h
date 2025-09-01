#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H

#include "BaseMonitor.h"
#include <QProcess>
#include <QFileSystemWatcher>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

/**
 * @brief 内存监控器类
 * 
 * MemoryMonitor专门负责监控内存相关的性能指标：
 * - 系统内存使用监控
 * - 进程内存使用监控
 * - 虚拟内存监控
 * - 内存泄漏检测
 * - 交换空间监控
 */
class MemoryMonitor : public BaseMonitor
{
    Q_OBJECT

public:
    /**
     * @brief 内存监控模式枚举
     */
    enum MonitoringMode {
        SystemMode,         ///< 系统模式 - 监控系统内存
        ProcessMode,        ///< 进程模式 - 监控进程内存
        DetailedMode,       ///< 详细模式 - 包含虚拟内存
        LeakDetectionMode   ///< 泄漏检测模式 - 内存泄漏检测
    };
    Q_ENUM(MonitoringMode)

    /**
     * @brief 内存类型枚举
     */
    enum MemoryType {
        PhysicalMemory,     ///< 物理内存
        VirtualMemory,      ///< 虚拟内存
        SwapMemory,         ///< 交换内存
        CachedMemory,       ///< 缓存内存
        BufferMemory        ///< 缓冲内存
    };
    Q_ENUM(MemoryType)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MemoryMonitor(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MemoryMonitor();

    /**
     * @brief 获取监控器版本
     * @return 版本字符串
     */
    QString version() const override;

    /**
     * @brief 获取监控器描述
     * @return 描述字符串
     */
    QString description() const override;

    /**
     * @brief 设置监控模式
     * @param mode 监控模式
     */
    void setMonitoringMode(MonitoringMode mode);

    /**
     * @brief 获取监控模式
     * @return 监控模式
     */
    MonitoringMode monitoringMode() const;

    /**
     * @brief 获取总物理内存
     * @return 总物理内存(字节)
     */
    qint64 getTotalPhysicalMemory() const;

    /**
     * @brief 获取可用物理内存
     * @return 可用物理内存(字节)
     */
    qint64 getAvailablePhysicalMemory() const;

    /**
     * @brief 获取已使用物理内存
     * @return 已使用物理内存(字节)
     */
    qint64 getUsedPhysicalMemory() const;

    /**
     * @brief 获取物理内存使用率
     * @return 物理内存使用率(%)
     */
    double getPhysicalMemoryUsage() const;

    /**
     * @brief 获取总虚拟内存
     * @return 总虚拟内存(字节)
     */
    qint64 getTotalVirtualMemory() const;

    /**
     * @brief 获取已使用虚拟内存
     * @return 已使用虚拟内存(字节)
     */
    qint64 getUsedVirtualMemory() const;

    /**
     * @brief 获取虚拟内存使用率
     * @return 虚拟内存使用率(%)
     */
    double getVirtualMemoryUsage() const;

    /**
     * @brief 获取交换空间大小
     * @return 交换空间大小(字节)
     */
    qint64 getSwapSize() const;

    /**
     * @brief 获取已使用交换空间
     * @return 已使用交换空间(字节)
     */
    qint64 getUsedSwap() const;

    /**
     * @brief 获取交换空间使用率
     * @return 交换空间使用率(%)
     */
    double getSwapUsage() const;

    /**
     * @brief 获取缓存内存大小
     * @return 缓存内存大小(字节)
     */
    qint64 getCachedMemory() const;

    /**
     * @brief 获取缓冲内存大小
     * @return 缓冲内存大小(字节)
     */
    qint64 getBufferMemory() const;

    /**
     * @brief 获取进程内存使用
     * @param processId 进程ID
     * @return 进程内存使用(字节)
     */
    qint64 getProcessMemoryUsage(qint64 processId) const;

    /**
     * @brief 获取当前进程内存使用
     * @return 当前进程内存使用(字节)
     */
    qint64 getCurrentProcessMemoryUsage() const;

    /**
     * @brief 获取进程虚拟内存使用
     * @param processId 进程ID
     * @return 进程虚拟内存使用(字节)
     */
    qint64 getProcessVirtualMemoryUsage(qint64 processId) const;

    /**
     * @brief 获取内存使用历史
     * @param type 内存类型
     * @param minutes 历史分钟数
     * @return 内存使用历史数据
     */
    QList<qint64> getMemoryUsageHistory(MemoryType type, int minutes) const;

    /**
     * @brief 检测内存泄漏
     * @param processId 进程ID
     * @return 是否检测到内存泄漏
     */
    bool detectMemoryLeak(qint64 processId) const;

    /**
     * @brief 获取内存泄漏率
     * @param processId 进程ID
     * @return 内存泄漏率(字节/秒)
     */
    double getMemoryLeakRate(qint64 processId) const;

    /**
     * @brief 获取内存碎片化程度
     * @return 内存碎片化程度(%)
     */
    double getMemoryFragmentation() const;

    /**
     * @brief 获取内存压力指数
     * @return 内存压力指数(0-100)
     */
    int getMemoryPressure() const;

    /**
     * @brief 设置内存泄漏检测阈值
     * @param threshold 阈值(字节/秒)
     */
    void setLeakDetectionThreshold(double threshold);

    /**
     * @brief 获取内存泄漏检测阈值
     * @return 阈值(字节/秒)
     */
    double leakDetectionThreshold() const;

    /**
     * @brief 强制垃圾回收
     * @return 回收的内存大小(字节)
     */
    qint64 forceGarbageCollection();

    /**
     * @brief 获取内存统计信息
     * @return 内存统计信息
     */
    QVariantMap getMemoryStatistics() const;

protected:
    /**
     * @brief 初始化内存监控器
     * @return 初始化是否成功
     */
    bool initializeMonitor() override;

    /**
     * @brief 收集内存资源使用数据
     * @return 内存资源使用数据
     */
    ResourceUsage collectResourceUsage() override;

    /**
     * @brief 获取支持的资源类型
     * @return 支持的资源类型列表
     */
    QList<ResourceType> supportedResourceTypes() const override;

private slots:
    /**
     * @brief 处理内存信息更新
     */
    void handleMemoryInfoUpdate();

    /**
     * @brief 执行内存泄漏检测
     */
    void performLeakDetection();

private:
    /**
     * @brief 初始化平台特定功能
     * @return 初始化是否成功
     */
    bool initializePlatformSpecific();

    /**
     * @brief 清理平台特定资源
     */
    void cleanupPlatformSpecific();

    /**
     * @brief 读取系统内存信息
     * @return 系统内存信息
     */
    QVariantMap readSystemMemoryInfo() const;

    /**
     * @brief 读取进程内存信息
     * @param processId 进程ID
     * @return 进程内存信息
     */
    QVariantMap readProcessMemoryInfo(qint64 processId) const;

    /**
     * @brief 计算内存碎片化
     * @return 碎片化程度
     */
    double calculateFragmentation();

    /**
     * @brief 计算内存压力
     * @return 内存压力指数
     */
    int calculateMemoryPressure();

#ifdef Q_OS_WIN
    /**
     * @brief Windows平台内存信息读取
     * @return 内存信息
     */
    QVariantMap readMemoryInfoWindows();

    /**
     * @brief Windows平台进程内存读取
     * @param processId 进程ID
     * @return 进程内存信息
     */
    QVariantMap readProcessMemoryWindows(qint64 processId);
#endif

#ifdef Q_OS_LINUX
    /**
     * @brief Linux平台内存信息读取
     * @return 内存信息
     */
    QVariantMap readMemoryInfoLinux();

    /**
     * @brief Linux平台进程内存读取
     * @param processId 进程ID
     * @return 进程内存信息
     */
    QVariantMap readProcessMemoryLinux(qint64 processId);

    /**
     * @brief 解析/proc/meminfo文件
     * @return 内存信息
     */
    QVariantMap parseProcMeminfo();

    /**
     * @brief 解析/proc/[pid]/status文件
     * @param processId 进程ID
     * @return 进程内存信息
     */
    QVariantMap parseProcStatus(qint64 processId);
#endif

#ifdef Q_OS_MACOS
    /**
     * @brief macOS平台内存信息读取
     * @return 内存信息
     */
    QVariantMap readMemoryInfoMacOS();

    /**
     * @brief macOS平台进程内存读取
     * @param processId 进程ID
     * @return 进程内存信息
     */
    QVariantMap readProcessMemoryMacOS(qint64 processId);
#endif

    MonitoringMode m_monitoringMode;                ///< 监控模式
    double m_leakDetectionThreshold;               ///< 内存泄漏检测阈值
    
    QFileSystemWatcher* m_fileWatcher;             ///< 文件监控器
    QProcess* m_systemProcess;                     ///< 系统进程
    QTimer* m_leakDetectionTimer;                  ///< 泄漏检测定时器
    
    // 历史数据缓存
    QList<qint64> m_physicalMemoryHistory;         ///< 物理内存使用历史
    QList<qint64> m_virtualMemoryHistory;          ///< 虚拟内存使用历史
    QList<qint64> m_swapMemoryHistory;             ///< 交换内存使用历史
    QMap<qint64, QList<qint64>> m_processMemoryHistory; ///< 进程内存使用历史
    
    // 内存泄漏检测数据
    struct LeakDetectionData {
        qint64 lastMemoryUsage = 0;
        QDateTime lastCheckTime;
        QList<qint64> memoryTrend;
        double leakRate = 0.0;
    };
    QMap<qint64, LeakDetectionData> m_leakDetectionData; ///< 泄漏检测数据
    
    // 缓存数据
    mutable QMutex m_dataMutex;                    ///< 数据互斥锁
    QVariantMap m_lastSystemMemoryInfo;            ///< 上次系统内存信息
    QMap<qint64, QVariantMap> m_lastProcessMemoryInfo; ///< 上次进程内存信息
    QDateTime m_lastUpdateTime;                    ///< 上次更新时间
    
    // 系统信息缓存
    qint64 m_totalPhysicalMemory;                  ///< 总物理内存
    qint64 m_totalVirtualMemory;                   ///< 总虚拟内存
    qint64 m_totalSwapMemory;                      ///< 总交换内存
};

#endif // MEMORYMONITOR_H