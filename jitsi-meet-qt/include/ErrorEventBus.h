#ifndef ERROREVENTBUS_H
#define ERROREVENTBUS_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <QMutex>
#include <QTimer>
#include <QQueue>
#include "ModuleError.h"

class ErrorRecoveryStrategy;

/**
 * @brief 错误事件总线 - 模块间错误传播和处理中心
 * 
 * 该类实现了模块化系统中的错误事件传播机制，包括：
 * - 错误事件的发布和订阅
 * - 错误过滤和路由
 * - 错误恢复策略管理
 * - 错误统计和监控
 */
class ErrorEventBus : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 错误过滤器接口
     */
    class ErrorFilter {
    public:
        virtual ~ErrorFilter() = default;
        
        /**
         * @brief 过滤错误
         * @param error 模块错误
         * @return 是否通过过滤器
         */
        virtual bool filter(const ModuleError& error) const = 0;
        
        /**
         * @brief 获取过滤器名称
         * @return 过滤器名称
         */
        virtual QString name() const = 0;
    };

    /**
     * @brief 错误统计信息
     */
    struct ErrorStatistics {
        int totalErrors;                                    ///< 总错误数
        QMap<ModuleError::ErrorType, int> errorsByType;    ///< 按类型分组的错误数
        QMap<ModuleError::Severity, int> errorsBySeverity; ///< 按严重程度分组的错误数
        QMap<QString, int> errorsByModule;                 ///< 按模块分组的错误数
        QDateTime lastError;                               ///< 最后错误时间
        double errorRate;                                  ///< 错误率 (错误/分钟)
        
        ErrorStatistics() : totalErrors(0), errorRate(0.0) {}
    };

    explicit ErrorEventBus(QObject *parent = nullptr);
    ~ErrorEventBus();

    /**
     * @brief 获取单例实例
     */
    static ErrorEventBus* instance();

    /**
     * @brief 初始化错误事件总线
     */
    bool initialize();

    /**
     * @brief 关闭错误事件总线
     */
    void shutdown();

    /**
     * @brief 报告错误
     * @param error 模块错误
     */
    void reportError(const ModuleError& error);

    /**
     * @brief 订阅错误事件
     * @param subscriber 订阅者
     * @param moduleName 模块名称 (空表示订阅所有模块)
     */
    void subscribeToErrors(QObject* subscriber, const QString& moduleName = QString());

    /**
     * @brief 取消订阅错误事件
     * @param subscriber 订阅者
     * @param moduleName 模块名称 (空表示取消所有订阅)
     */
    void unsubscribeFromErrors(QObject* subscriber, const QString& moduleName = QString());

    /**
     * @brief 订阅特定类型的错误
     * @param subscriber 订阅者
     * @param errorType 错误类型
     */
    void subscribeToErrorType(QObject* subscriber, ModuleError::ErrorType errorType);

    /**
     * @brief 取消订阅特定类型的错误
     * @param subscriber 订阅者
     * @param errorType 错误类型
     */
    void unsubscribeFromErrorType(QObject* subscriber, ModuleError::ErrorType errorType);

    /**
     * @brief 订阅特定严重程度的错误
     * @param subscriber 订阅者
     * @param severity 严重程度
     */
    void subscribeToSeverity(QObject* subscriber, ModuleError::Severity severity);

    /**
     * @brief 取消订阅特定严重程度的错误
     * @param subscriber 订阅者
     * @param severity 严重程度
     */
    void unsubscribeFromSeverity(QObject* subscriber, ModuleError::Severity severity);

    /**
     * @brief 添加错误过滤器
     * @param filter 错误过滤器
     * @param subscriber 订阅者
     */
    void addErrorFilter(ErrorFilter* filter, QObject* subscriber);

    /**
     * @brief 移除错误过滤器
     * @param filter 错误过滤器
     * @param subscriber 订阅者
     */
    void removeErrorFilter(ErrorFilter* filter, QObject* subscriber);

    /**
     * @brief 设置错误恢复策略
     * @param strategy 恢复策略
     * @param moduleName 模块名称 (空表示全局策略)
     */
    void setRecoveryStrategy(ErrorRecoveryStrategy* strategy, const QString& moduleName = QString());

    /**
     * @brief 获取错误恢复策略
     * @param moduleName 模块名称
     * @return 恢复策略
     */
    ErrorRecoveryStrategy* getRecoveryStrategy(const QString& moduleName) const;

    /**
     * @brief 移除错误恢复策略
     * @param moduleName 模块名称
     */
    void removeRecoveryStrategy(const QString& moduleName);

    /**
     * @brief 启用自动错误恢复
     * @param enabled 是否启用
     */
    void setAutoRecoveryEnabled(bool enabled);

    /**
     * @brief 检查自动错误恢复是否启用
     * @return 是否启用
     */
    bool isAutoRecoveryEnabled() const;

    /**
     * @brief 获取错误统计信息
     * @return 错误统计
     */
    ErrorStatistics getStatistics() const;

    /**
     * @brief 获取模块错误统计信息
     * @param moduleName 模块名称
     * @return 模块错误统计
     */
    ErrorStatistics getModuleStatistics(const QString& moduleName) const;

    /**
     * @brief 重置错误统计
     */
    void resetStatistics();

    /**
     * @brief 获取最近的错误列表
     * @param count 错误数量
     * @return 错误列表
     */
    QList<ModuleError> getRecentErrors(int count = 100) const;

    /**
     * @brief 获取模块的最近错误列表
     * @param moduleName 模块名称
     * @param count 错误数量
     * @return 错误列表
     */
    QList<ModuleError> getModuleRecentErrors(const QString& moduleName, int count = 100) const;

    /**
     * @brief 清除错误历史
     */
    void clearErrorHistory();

    /**
     * @brief 设置错误历史大小限制
     * @param maxSize 最大大小
     */
    void setMaxHistorySize(int maxSize);

    /**
     * @brief 获取错误历史大小限制
     * @return 最大大小
     */
    int maxHistorySize() const;

    /**
     * @brief 启用错误日志记录
     * @param enabled 是否启用
     */
    void setErrorLoggingEnabled(bool enabled);

    /**
     * @brief 检查错误日志记录是否启用
     * @return 是否启用
     */
    bool isErrorLoggingEnabled() const;

signals:
    /**
     * @brief 错误报告信号
     * @param error 模块错误
     */
    void errorReported(const ModuleError& error);

    /**
     * @brief 模块错误报告信号
     * @param moduleName 模块名称
     * @param error 模块错误
     */
    void moduleErrorReported(const QString& moduleName, const ModuleError& error);

    /**
     * @brief 错误类型报告信号
     * @param errorType 错误类型
     * @param error 模块错误
     */
    void errorTypeReported(ModuleError::ErrorType errorType, const ModuleError& error);

    /**
     * @brief 严重程度报告信号
     * @param severity 严重程度
     * @param error 模块错误
     */
    void severityReported(ModuleError::Severity severity, const ModuleError& error);

    /**
     * @brief 错误恢复开始信号
     * @param error 模块错误
     * @param strategy 恢复策略名称
     */
    void errorRecoveryStarted(const ModuleError& error, const QString& strategy);

    /**
     * @brief 错误恢复完成信号
     * @param error 模块错误
     * @param strategy 恢复策略名称
     * @param success 是否成功
     */
    void errorRecoveryCompleted(const ModuleError& error, const QString& strategy, bool success);

    /**
     * @brief 错误统计更新信号
     * @param statistics 错误统计
     */
    void statisticsUpdated(const ErrorStatistics& statistics);

private slots:
    /**
     * @brief 处理错误恢复
     * @param error 模块错误
     */
    void handleErrorRecovery(const ModuleError& error);

    /**
     * @brief 处理统计更新定时器
     */
    void updateStatistics();

    /**
     * @brief 处理错误队列
     */
    void processErrorQueue();

    /**
     * @brief 处理订阅者对象销毁
     * @param obj 被销毁的对象
     */
    void onSubscriberDestroyed(QObject* obj);

private:
    /**
     * @brief 分发错误事件
     * @param error 模块错误
     */
    void dispatchError(const ModuleError& error);

    /**
     * @brief 检查订阅者是否匹配
     * @param subscriber 订阅者
     * @param error 模块错误
     * @return 是否匹配
     */
    bool matchesSubscriber(QObject* subscriber, const ModuleError& error) const;

    /**
     * @brief 应用错误过滤器
     * @param subscriber 订阅者
     * @param error 模块错误
     * @return 是否通过过滤器
     */
    bool applyFilters(QObject* subscriber, const ModuleError& error) const;

    /**
     * @brief 更新错误统计
     * @param error 模块错误
     */
    void updateErrorStatistics(const ModuleError& error);

    /**
     * @brief 记录错误日志
     * @param error 模块错误
     */
    void logError(const ModuleError& error);

    /**
     * @brief 清理过期的错误历史
     */
    void cleanupErrorHistory();

    // 单例实例
    static ErrorEventBus* s_instance;

    // 订阅者管理
    QMap<QString, QList<QObject*>> m_moduleSubscribers;     ///< 按模块分组的订阅者
    QMap<ModuleError::ErrorType, QList<QObject*>> m_typeSubscribers;    ///< 按类型分组的订阅者
    QMap<ModuleError::Severity, QList<QObject*>> m_severitySubscribers; ///< 按严重程度分组的订阅者
    QList<QObject*> m_globalSubscribers;                   ///< 全局订阅者

    // 过滤器管理
    QMap<QObject*, QList<ErrorFilter*>> m_errorFilters;    ///< 错误过滤器

    // 恢复策略管理
    QMap<QString, ErrorRecoveryStrategy*> m_recoveryStrategies; ///< 恢复策略
    ErrorRecoveryStrategy* m_globalRecoveryStrategy;       ///< 全局恢复策略
    bool m_autoRecoveryEnabled;                            ///< 自动恢复启用状态

    // 错误统计和历史
    ErrorStatistics m_globalStatistics;                    ///< 全局错误统计
    QMap<QString, ErrorStatistics> m_moduleStatistics;     ///< 模块错误统计
    QList<ModuleError> m_errorHistory;                     ///< 错误历史
    int m_maxHistorySize;                                  ///< 最大历史大小

    // 错误队列和处理
    QQueue<ModuleError> m_errorQueue;                      ///< 错误队列
    QTimer* m_processTimer;                                ///< 处理定时器
    QTimer* m_statisticsTimer;                             ///< 统计更新定时器

    // 配置选项
    bool m_errorLoggingEnabled;                            ///< 错误日志启用状态
    bool m_initialized;                                    ///< 初始化状态

    // 线程安全
    mutable QMutex m_mutex;

    // 常量
    static const int DEFAULT_MAX_HISTORY_SIZE = 1000;      ///< 默认最大历史大小
    static const int PROCESS_INTERVAL = 100;               ///< 处理间隔 (ms)
    static const int STATISTICS_INTERVAL = 60000;          ///< 统计更新间隔 (ms)
};

/**
 * @brief 模块名称错误过滤器
 */
class ModuleNameFilter : public ErrorEventBus::ErrorFilter
{
public:
    explicit ModuleNameFilter(const QString& moduleName);
    bool filter(const ModuleError& error) const override;
    QString name() const override;

private:
    QString m_moduleName;
};

/**
 * @brief 错误类型过滤器
 */
class ErrorTypeFilter : public ErrorEventBus::ErrorFilter
{
public:
    explicit ErrorTypeFilter(ModuleError::ErrorType errorType);
    bool filter(const ModuleError& error) const override;
    QString name() const override;

private:
    ModuleError::ErrorType m_errorType;
};

/**
 * @brief 严重程度过滤器
 */
class SeverityFilter : public ErrorEventBus::ErrorFilter
{
public:
    explicit SeverityFilter(ModuleError::Severity severity);
    bool filter(const ModuleError& error) const override;
    QString name() const override;

private:
    ModuleError::Severity m_severity;
};

Q_DECLARE_METATYPE(ErrorEventBus::ErrorStatistics)

#endif // ERROREVENTBUS_H