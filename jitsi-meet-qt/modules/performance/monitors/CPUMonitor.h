#ifndef CPUMONITOR_H
#define CPUMONITOR_H

#include "BaseMonitor.h"
#include <QProcess>
#include <QFileSystemWatcher>

#ifdef Q_OS_WIN
#include <windows.h>
#include <pdh.h>
#endif

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

/**
 * @brief CPU监控器类
 * 
 * CPUMonitor专门负责监控CPU相关的性能指标：
 * - CPU使用率监控
 * - CPU频率监控
 * - CPU温度监控
 * - 多核心CPU监控
 * - 进程CPU使用监控
 */
class CPUMonitor : public BaseMonitor
{
    Q_OBJECT

public:
    /**
     * @brief CPU监控模式枚举
     */
    enum MonitoringMode {
        BasicMode,          ///< 基础模式 - 仅总体CPU使用率
        DetailedMode,       ///< 详细模式 - 包含各核心使用率
        ProcessMode,        ///< 进程模式 - 包含进程CPU使用
        AdvancedMode        ///< 高级模式 - 包含温度和频率
    };
    Q_ENUM(MonitoringMode)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit CPUMonitor(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~CPUMonitor();

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
     * @brief 获取CPU核心数
     * @return CPU核心数
     */
    int getCoreCount() const;

    /**
     * @brief 获取CPU架构信息
     * @return CPU架构信息
     */
    QString getCPUArchitecture() const;

    /**
     * @brief 获取CPU型号信息
     * @return CPU型号信息
     */
    QString getCPUModel() const;

    /**
     * @brief 获取CPU基础频率
     * @return CPU基础频率(GHz)
     */
    double getBaseClock() const;

    /**
     * @brief 获取当前CPU频率
     * @return 当前CPU频率(GHz)
     */
    double getCurrentClock() const;

    /**
     * @brief 获取CPU温度
     * @return CPU温度(摄氏度)
     */
    double getCPUTemperature() const;

    /**
     * @brief 获取各核心使用率
     * @return 各核心使用率列表
     */
    QList<double> getCoreUsages() const;

    /**
     * @brief 获取进程CPU使用率
     * @param processId 进程ID
     * @return 进程CPU使用率
     */
    double getProcessCPUUsage(qint64 processId) const;

    /**
     * @brief 获取当前进程CPU使用率
     * @return 当前进程CPU使用率
     */
    double getCurrentProcessCPUUsage() const;

    /**
     * @brief 获取系统负载平均值
     * @return 负载平均值(1分钟, 5分钟, 15分钟)
     */
    QList<double> getLoadAverages() const;

    /**
     * @brief 获取CPU使用率历史
     * @param minutes 历史分钟数
     * @return CPU使用率历史数据
     */
    QList<double> getCPUUsageHistory(int minutes) const;

    /**
     * @brief 检查CPU是否过热
     * @return 是否过热
     */
    bool isCPUOverheating() const;

    /**
     * @brief 设置过热阈值
     * @param threshold 过热阈值(摄氏度)
     */
    void setOverheatThreshold(double threshold);

    /**
     * @brief 获取过热阈值
     * @return 过热阈值(摄氏度)
     */
    double overheatThreshold() const;

protected:
    /**
     * @brief 初始化CPU监控器
     * @return 初始化是否成功
     */
    bool initializeMonitor() override;

    /**
     * @brief 收集CPU资源使用数据
     * @return CPU资源使用数据
     */
    ResourceUsage collectResourceUsage() override;

    /**
     * @brief 获取支持的资源类型
     * @return 支持的资源类型列表
     */
    QList<ResourceType> supportedResourceTypes() const override;

private slots:
    /**
     * @brief 处理系统信息更新
     */
    void handleSystemInfoUpdate();

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
     * @brief 读取CPU使用率
     * @return CPU使用率
     */
    double readCPUUsage();

    /**
     * @brief 读取各核心使用率
     * @return 各核心使用率列表
     */
    QList<double> readCoreUsages();

    /**
     * @brief 读取CPU频率
     * @return CPU频率(GHz)
     */
    double readCPUFrequency();

    /**
     * @brief 读取CPU温度
     * @return CPU温度(摄氏度)
     */
    double readCPUTemperature();

    /**
     * @brief 读取系统负载
     * @return 系统负载平均值
     */
    QList<double> readLoadAverages();

    /**
     * @brief 读取进程CPU使用率
     * @param processId 进程ID
     * @return 进程CPU使用率
     */
    double readProcessCPUUsage(qint64 processId);

#ifdef Q_OS_WIN
    /**
     * @brief Windows平台CPU使用率读取
     * @return CPU使用率
     */
    double readCPUUsageWindows();

    /**
     * @brief Windows平台CPU温度读取
     * @return CPU温度
     */
    double readCPUTemperatureWindows();
#endif

#ifdef Q_OS_LINUX
    /**
     * @brief Linux平台CPU使用率读取
     * @return CPU使用率
     */
    double readCPUUsageLinux();

    /**
     * @brief Linux平台CPU温度读取
     * @return CPU温度
     */
    double readCPUTemperatureLinux();

    /**
     * @brief 解析/proc/stat文件
     * @return CPU统计信息
     */
    QVariantMap parseProcStat();

    /**
     * @brief 解析/proc/loadavg文件
     * @return 负载平均值
     */
    QList<double> parseProcLoadavg();
#endif

#ifdef Q_OS_MACOS
    /**
     * @brief macOS平台CPU使用率读取
     * @return CPU使用率
     */
    double readCPUUsageMacOS();

    /**
     * @brief macOS平台CPU温度读取
     * @return CPU温度
     */
    double readCPUTemperatureMacOS();
#endif

    MonitoringMode m_monitoringMode;                ///< 监控模式
    int m_coreCount;                               ///< CPU核心数
    QString m_cpuModel;                            ///< CPU型号
    QString m_cpuArchitecture;                     ///< CPU架构
    double m_baseClock;                            ///< 基础频率
    double m_overheatThreshold;                    ///< 过热阈值
    
    QFileSystemWatcher* m_fileWatcher;             ///< 文件监控器
    QProcess* m_systemProcess;                     ///< 系统进程
    
    // 历史数据缓存
    QList<double> m_cpuUsageHistory;               ///< CPU使用率历史
    QList<QList<double>> m_coreUsageHistory;       ///< 各核心使用率历史
    QList<double> m_temperatureHistory;            ///< 温度历史
    QList<double> m_frequencyHistory;              ///< 频率历史
    
    // 平台特定数据
#ifdef Q_OS_WIN
    PDH_HQUERY m_pdhQuery;                         ///< Windows性能数据查询句柄
    PDH_HCOUNTER m_cpuCounter;                     ///< CPU计数器
    QList<PDH_HCOUNTER> m_coreCounters;            ///< 各核心计数器
#endif

#ifdef Q_OS_LINUX
    QVariantMap m_lastProcStat;                    ///< 上次/proc/stat数据
    QDateTime m_lastStatTime;                      ///< 上次统计时间
#endif

    // 缓存数据
    mutable QMutex m_dataMutex;                    ///< 数据互斥锁
    double m_lastCPUUsage;                         ///< 上次CPU使用率
    QList<double> m_lastCoreUsages;                ///< 上次各核心使用率
    double m_lastTemperature;                      ///< 上次温度
    double m_lastFrequency;                        ///< 上次频率
    QDateTime m_lastUpdateTime;                    ///< 上次更新时间
};

#endif // CPUMONITOR_H