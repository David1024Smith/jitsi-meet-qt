#ifndef IPERFORMANCEMONITOR_H
#define IPERFORMANCEMONITOR_H

#include <QObject>
#include <QDateTime>
#include <QVariantMap>

// 前向声明
struct PerformanceMetrics;

/**
 * @brief 性能监控接口
 * 
 * IPerformanceMonitor定义了性能监控的标准接口，所有性能监控组件都应该实现此接口。
 * 该接口提供了统一的性能监控操作方法。
 */
class IPerformanceMonitor
{
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~IPerformanceMonitor() = default;

    /**
     * @brief 初始化监控器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 启动监控
     * @return 启动是否成功
     */
    virtual bool start() = 0;

    /**
     * @brief 停止监控
     */
    virtual void stop() = 0;

    /**
     * @brief 检查监控器是否正在运行
     * @return 是否正在运行
     */
    virtual bool isRunning() const = 0;

    /**
     * @brief 获取当前性能指标
     * @return 当前性能指标
     */
    virtual PerformanceMetrics getCurrentMetrics() const = 0;

    /**
     * @brief 获取历史性能指标
     * @param from 开始时间
     * @param to 结束时间
     * @return 历史性能指标列表
     */
    virtual QList<PerformanceMetrics> getHistoricalMetrics(const QDateTime& from, const QDateTime& to) const = 0;

    /**
     * @brief 获取监控器名称
     * @return 监控器名称
     */
    virtual QString getMonitorName() const = 0;

    /**
     * @brief 获取监控器版本
     * @return 监控器版本
     */
    virtual QString getVersion() const = 0;

    /**
     * @brief 获取监控器状态信息
     * @return 状态信息
     */
    virtual QVariantMap getStatus() const = 0;

    /**
     * @brief 重置监控器
     */
    virtual void reset() = 0;
};

Q_DECLARE_INTERFACE(IPerformanceMonitor, "org.jitsi.performance.IPerformanceMonitor/1.0")

#endif // IPERFORMANCEMONITOR_H