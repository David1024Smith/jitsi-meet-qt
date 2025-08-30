#ifndef RUNTIMECONTROLLER_H
#define RUNTIMECONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QQueue>

/**
 * @brief 运行时控制器
 * 
 * 提供模块的运行时启用/禁用控制，支持热插拔和动态加载
 */
class RuntimeController : public QObject
{
    Q_OBJECT

public:
    enum ControlAction {
        Enable,         // 启用模块
        Disable,        // 禁用模块
        Reload,         // 重新加载模块
        Restart,        // 重启模块
        Suspend,        // 暂停模块
        Resume          // 恢复模块
    };
    Q_ENUM(ControlAction)

    enum ExecutionMode {
        Synchronous,    // 同步执行
        Asynchronous,   // 异步执行
        Queued,         // 队列执行
        Immediate       // 立即执行
    };
    Q_ENUM(ExecutionMode)

    struct ControlRequest {
        QString moduleName;
        ControlAction action;
        QVariantMap parameters;
        ExecutionMode mode;
        int priority;
        QDateTime timestamp;
        QString requestId;
    };

    explicit RuntimeController(QObject* parent = nullptr);
    ~RuntimeController();

    // 模块控制操作
    bool enableModule(const QString& moduleName, ExecutionMode mode = Asynchronous);
    bool disableModule(const QString& moduleName, ExecutionMode mode = Asynchronous);
    bool reloadModule(const QString& moduleName, ExecutionMode mode = Asynchronous);
    bool restartModule(const QString& moduleName, ExecutionMode mode = Asynchronous);
    bool suspendModule(const QString& moduleName);
    bool resumeModule(const QString& moduleName);

    // 批量操作
    bool enableModules(const QStringList& moduleNames, ExecutionMode mode = Asynchronous);
    bool disableModules(const QStringList& moduleNames, ExecutionMode mode = Asynchronous);
    bool reloadAllModules();
    bool restartAllModules();

    // 请求管理
    QString submitRequest(const ControlRequest& request);
    bool cancelRequest(const QString& requestId);
    ControlRequest getRequest(const QString& requestId) const;
    QList<ControlRequest> getPendingRequests() const;
    void clearPendingRequests();

    // 执行控制
    void setExecutionMode(ExecutionMode mode);
    ExecutionMode getExecutionMode() const;
    void setMaxConcurrentOperations(int maxOperations);
    int getMaxConcurrentOperations() const;
    void setOperationTimeout(int timeoutMs);
    int getOperationTimeout() const;

    // 状态查询
    bool isOperationInProgress(const QString& moduleName) const;
    QStringList getActiveOperations() const;
    int getPendingRequestCount() const;
    bool canExecuteOperation(const QString& moduleName, ControlAction action) const;

    // 安全控制
    void setRequireConfirmation(bool require);
    bool isConfirmationRequired() const;
    void setSafeMode(bool enabled);
    bool isSafeModeEnabled() const;
    void setRollbackEnabled(bool enabled);
    bool isRollbackEnabled() const;

    // 调度控制
    void pauseExecution();
    void resumeExecution();
    bool isExecutionPaused() const;
    void setSchedulingEnabled(bool enabled);
    bool isSchedulingEnabled() const;

public slots:
    void processNextRequest();
    void onOperationCompleted(const QString& moduleName, ControlAction action, bool success);
    void onOperationTimeout(const QString& requestId);
    void onConfirmationReceived(const QString& requestId, bool confirmed);

signals:
    void operationStarted(const QString& moduleName, ControlAction action);
    void operationCompleted(const QString& moduleName, ControlAction action, bool success);
    void operationFailed(const QString& moduleName, ControlAction action, const QString& error);
    void requestQueued(const QString& requestId);
    void requestCancelled(const QString& requestId);
    void confirmationRequired(const QString& requestId, const ControlRequest& request);
    void executionPaused();
    void executionResumed();
    void queueEmpty();

private slots:
    void processRequestQueue();
    void checkOperationTimeouts();

private:
    bool executeRequest(const ControlRequest& request);
    bool performModuleOperation(const QString& moduleName, ControlAction action, const QVariantMap& parameters);
    bool validateRequest(const ControlRequest& request) const;
    QString generateRequestId() const;
    void scheduleNextExecution();
    bool createModuleBackup(const QString& moduleName);
    bool restoreModuleBackup(const QString& moduleName);

    ExecutionMode m_executionMode;
    int m_maxConcurrentOperations;
    int m_operationTimeout;
    bool m_requireConfirmation;
    bool m_safeModeEnabled;
    bool m_rollbackEnabled;
    bool m_executionPaused;
    bool m_schedulingEnabled;

    QQueue<ControlRequest> m_requestQueue;
    QMap<QString, ControlRequest> m_activeRequests;
    QMap<QString, QDateTime> m_operationStartTimes;
    QStringList m_activeOperations;

    QTimer* m_processTimer;
    QTimer* m_timeoutTimer;
    mutable QMutex m_mutex;

    static int s_requestCounter;
};

Q_DECLARE_METATYPE(RuntimeController::ControlRequest)

#endif // RUNTIMECONTROLLER_H