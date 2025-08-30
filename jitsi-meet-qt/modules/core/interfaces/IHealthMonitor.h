#ifndef IHEALTHMONITOR_H
#define IHEALTHMONITOR_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariantMap>

/**
 * @brief 健康监控接口
 * 
 * 定义模块健康检查和状态监控的标准接口
 */
class IHealthMonitor : public QObject
{
    Q_OBJECT

public:
    enum HealthStatus {
        Healthy,        // 健康
        Warning,        // 警告
        Critical,       // 严重
        Failure,        // 失败
        Unknown         // 未知
    };
    Q_ENUM(HealthStatus)

    enum CheckType {
        Basic,          // 基础检查
        Performance,    // 性能检查
        Resource,       // 资源检查
        Connectivity,   // 连接检查
        Functional      // 功能检查
    };
    Q_ENUM(CheckType)

    struct HealthReport {
        QString moduleName;
        HealthStatus status;
        QString message;
        QDateTime timestamp;
        QVariantMap details;
        double score;           // 健康评分 0-100
        int checkDuration;      // 检查耗时(ms)
    };

    virtual ~IHealthMonitor() = default;

    // 健康检查
    virtual HealthReport checkModuleHealth(const QString& moduleName) = 0;
    virtual HealthReport performHealthCheck(const QString& moduleName, CheckType type) = 0;
    virtual QList<HealthReport> checkAllModules() = 0;

    // 状态监控
    virtual void startMonitoring(const QString& moduleName) = 0;
    virtual void stopMonitoring(const QString& moduleName) = 0;
    virtual bool isMonitoring(const QString& moduleName) const = 0;
    virtual void setMonitoringInterval(int intervalMs) = 0;
    virtual int getMonitoringInterval() const = 0;

    // 健康状态查询
    virtual HealthStatus getModuleHealthStatus(const QString& moduleName) const = 0;
    virtual double getModuleHealthScore(const QString& moduleName) const = 0;
    virtual QDateTime getLastCheckTime(const QString& moduleName) const = 0;
    virtual QList<HealthReport> getHealthHistory(const QString& moduleName) const = 0;

    // 阈值管理
    virtual void setHealthThreshold(const QString& moduleName, HealthStatus threshold) = 0;
    virtual HealthStatus getHealthThreshold(const QString& moduleName) const = 0;
    virtual void setPerformanceThreshold(const QString& moduleName, double threshold) = 0;
    virtual double getPerformanceThreshold(const QString& moduleName) const = 0;

    // 自动恢复
    virtual void enableAutoRecovery(const QString& moduleName, bool enabled = true) = 0;
    virtual bool isAutoRecoveryEnabled(const QString& moduleName) const = 0;
    virtual bool triggerRecovery(const QString& moduleName) = 0;

signals:
    void healthStatusChanged(const QString& moduleName, HealthStatus status);
    void healthCheckCompleted(const QString& moduleName, const HealthReport& report);
    void healthThresholdExceeded(const QString& moduleName, HealthStatus status);
    void recoveryTriggered(const QString& moduleName);
    void recoveryCompleted(const QString& moduleName, bool success);
    void monitoringStarted(const QString& moduleName);
    void monitoringStopped(const QString& moduleName);
};

Q_DECLARE_METATYPE(IHealthMonitor::HealthReport)

#endif // IHEALTHMONITOR_H