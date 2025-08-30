#ifndef MODULEHEALTHMONITOR_H
#define MODULEHEALTHMONITOR_H

#include "interfaces/IHealthMonitor.h"
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QThreadPool>

/**
 * @brief 模块健康监控器
 * 
 * 实现模块健康检查和状态监控功能
 */
class ModuleHealthMonitor : public IHealthMonitor
{
    Q_OBJECT

public:
    explicit ModuleHealthMonitor(QObject* parent = nullptr);
    ~ModuleHealthMonitor();

    // IHealthMonitor接口实现
    HealthReport checkModuleHealth(const QString& moduleName) override;
    HealthReport performHealthCheck(const QString& moduleName, CheckType type) override;
    QList<HealthReport> checkAllModules() override;

    void startMonitoring(const QString& moduleName) override;
    void stopMonitoring(const QString& moduleName) override;
    bool isMonitoring(const QString& moduleName) const override;
    void setMonitoringInterval(int intervalMs) override;
    int getMonitoringInterval() const override;

    HealthStatus getModuleHealthStatus(const QString& moduleName) const override;
    double getModuleHealthScore(const QString& moduleName) const override;
    QDateTime getLastCheckTime(const QString& moduleName) const override;
    QList<HealthReport> getHealthHistory(const QString& moduleName) const override;

    void setHealthThreshold(const QString& moduleName, HealthStatus threshold) override;
    HealthStatus getHealthThreshold(const QString& moduleName) const override;
    void setPerformanceThreshold(const QString& moduleName, double threshold) override;
    double getPerformanceThreshold(const QString& moduleName) const override;

    void enableAutoRecovery(const QString& moduleName, bool enabled = true) override;
    bool isAutoRecoveryEnabled(const QString& moduleName) const override;
    bool triggerRecovery(const QString& moduleName) override;

    // 扩展功能
    void setMaxHistorySize(int maxSize);
    int getMaxHistorySize() const;
    void clearHistory(const QString& moduleName);
    void clearAllHistory();
    
    // 统计信息
    int getTotalChecksPerformed() const;
    int getFailedChecksCount() const;
    double getAverageCheckDuration() const;
    QStringList getUnhealthyModules() const;

public slots:
    void performScheduledCheck();
    void onModuleStatusChanged(const QString& moduleName, int status);

private slots:
    void processHealthCheckQueue();

private:
    struct ModuleHealthData {
        QString moduleName;
        HealthStatus currentStatus;
        double currentScore;
        QDateTime lastCheckTime;
        QList<HealthReport> history;
        HealthStatus threshold;
        double performanceThreshold;
        bool autoRecoveryEnabled;
        bool isMonitored;
        int consecutiveFailures;
        QDateTime lastRecoveryTime;
    };

    HealthReport performBasicCheck(const QString& moduleName);
    HealthReport performPerformanceCheck(const QString& moduleName);
    HealthReport performResourceCheck(const QString& moduleName);
    HealthReport performConnectivityCheck(const QString& moduleName);
    HealthReport performFunctionalCheck(const QString& moduleName);

    double calculateHealthScore(const QString& moduleName);
    HealthStatus determineHealthStatus(double score, const QString& moduleName);
    void updateHealthData(const QString& moduleName, const HealthReport& report);
    void addToHistory(const QString& moduleName, const HealthReport& report);
    void checkThresholds(const QString& moduleName, const HealthReport& report);
    bool attemptAutoRecovery(const QString& moduleName);
    
    QString generateHealthMessage(const QString& moduleName, HealthStatus status, double score);
    QVariantMap collectModuleMetrics(const QString& moduleName);
    bool isModuleResponsive(const QString& moduleName);
    double measureModulePerformance(const QString& moduleName);

    QMap<QString, ModuleHealthData> m_healthData;
    QTimer* m_monitoringTimer;
    QThreadPool* m_threadPool;
    
    int m_monitoringInterval;
    int m_maxHistorySize;
    int m_totalChecks;
    int m_failedChecks;
    double m_totalCheckDuration;
    
    mutable QMutex m_mutex;
    QMutex m_historyMutex;
};

/**
 * @brief 健康检查任务
 * 
 * 在独立线程中执行健康检查的任务类
 */
class HealthCheckTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    HealthCheckTask(const QString& moduleName, IHealthMonitor::CheckType type, ModuleHealthMonitor* monitor);
    void run() override;

signals:
    void checkCompleted(const QString& moduleName, const IHealthMonitor::HealthReport& report);

private:
    QString m_moduleName;
    IHealthMonitor::CheckType m_checkType;
    ModuleHealthMonitor* m_monitor;
};

#endif // MODULEHEALTHMONITOR_H