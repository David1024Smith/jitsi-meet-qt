#ifndef PERFORMANCEMODULE_H
#define PERFORMANCEMODULE_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QSharedPointer>
#include "IPerformanceMonitor.h"

class PerformanceManager;
class MetricsCollector;
class PerformanceConfig;

/**
 * @brief 性能模块核心类
 * 
 * PerformanceModule是性能监控系统的核心入口点，负责：
 * - 模块初始化和生命周期管理
 * - 性能管理器和指标收集器的创建
 * - 模块配置和状态管理
 * - 与其他模块的接口协调
 */
class PerformanceModule : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 模块状态枚举
     */
    enum ModuleStatus {
        NotInitialized,     ///< 未初始化
        Initializing,       ///< 初始化中
        Ready,              ///< 就绪状态
        Running,            ///< 运行中
        Paused,             ///< 暂停状态
        Error,              ///< 错误状态
        Shutdown            ///< 关闭状态
    };
    Q_ENUM(ModuleStatus)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PerformanceModule(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~PerformanceModule();

    /**
     * @brief 初始化性能模块
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 启动性能监控
     * @return 启动是否成功
     */
    bool start();

    /**
     * @brief 停止性能监控
     */
    void stop();

    /**
     * @brief 暂停性能监控
     */
    void pause();

    /**
     * @brief 恢复性能监控
     */
    void resume();

    /**
     * @brief 关闭性能模块
     */
    void shutdown();

    /**
     * @brief 获取模块状态
     * @return 当前模块状态
     */
    ModuleStatus status() const;

    /**
     * @brief 获取模块版本
     * @return 模块版本字符串
     */
    QString version() const;

    /**
     * @brief 获取性能管理器
     * @return 性能管理器指针
     */
    PerformanceManager* performanceManager() const;

    /**
     * @brief 获取指标收集器
     * @return 指标收集器指针
     */
    MetricsCollector* metricsCollector() const;

    /**
     * @brief 获取模块配置
     * @return 配置对象指针
     */
    PerformanceConfig* config() const;

    /**
     * @brief 检查模块是否已初始化
     * @return 是否已初始化
     */
    bool isInitialized() const;

    /**
     * @brief 检查模块是否正在运行
     * @return 是否正在运行
     */
    bool isRunning() const;

    /**
     * @brief 获取模块统计信息
     * @return 统计信息映射
     */
    QVariantMap getStatistics() const;

    /**
     * @brief 重置模块状态
     */
    void reset();

    /**
     * @brief 获取单例实例
     * @return 模块单例实例
     */
    static PerformanceModule* instance();

signals:
    /**
     * @brief 模块状态改变信号
     * @param status 新的模块状态
     */
    void statusChanged(ModuleStatus status);

    /**
     * @brief 模块初始化完成信号
     * @param success 初始化是否成功
     */
    void initialized(bool success);

    /**
     * @brief 模块启动信号
     */
    void started();

    /**
     * @brief 模块停止信号
     */
    void stopped();

    /**
     * @brief 模块暂停信号
     */
    void paused();

    /**
     * @brief 模块恢复信号
     */
    void resumed();

    /**
     * @brief 模块错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 模块关闭信号
     */
    void shutdown();

private slots:
    /**
     * @brief 处理内部状态更新
     */
    void handleStatusUpdate();

    /**
     * @brief 处理组件错误
     * @param error 错误信息
     */
    void handleComponentError(const QString& error);

private:
    /**
     * @brief 设置模块状态
     * @param status 新状态
     */
    void setStatus(ModuleStatus status);

    /**
     * @brief 初始化组件
     * @return 初始化是否成功
     */
    bool initializeComponents();

    /**
     * @brief 清理资源
     */
    void cleanup();

    /**
     * @brief 验证依赖关系
     * @return 依赖关系是否满足
     */
    bool validateDependencies();

    ModuleStatus m_status;                          ///< 模块状态
    PerformanceManager* m_performanceManager;       ///< 性能管理器
    MetricsCollector* m_metricsCollector;          ///< 指标收集器
    PerformanceConfig* m_config;                   ///< 模块配置
    QTimer* m_statusTimer;                         ///< 状态更新定时器
    QMutex m_mutex;                                ///< 线程安全互斥锁
    
    static PerformanceModule* s_instance;          ///< 单例实例
    static QMutex s_instanceMutex;                 ///< 单例互斥锁
};

#endif // PERFORMANCEMODULE_H