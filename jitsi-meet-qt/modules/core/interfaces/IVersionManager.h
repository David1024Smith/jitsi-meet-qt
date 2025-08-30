#ifndef IVERSIONMANAGER_H
#define IVERSIONMANAGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVersionNumber>

/**
 * @brief 版本管理接口
 * 
 * 定义模块版本管理和升级机制的标准接口
 */
class IVersionManager : public QObject
{
    Q_OBJECT

public:
    enum VersionType {
        Major,      // 主版本
        Minor,      // 次版本
        Patch,      // 补丁版本
        Build       // 构建版本
    };
    Q_ENUM(VersionType)

    enum UpgradeStatus {
        NoUpgrade,      // 无需升级
        Available,      // 有可用升级
        InProgress,     // 升级进行中
        Completed,      // 升级完成
        Failed,         // 升级失败
        Rollback        // 回滚中
    };
    Q_ENUM(UpgradeStatus)

    struct VersionInfo {
        QString moduleName;
        QVersionNumber version;
        QString description;
        QDateTime releaseDate;
        QStringList dependencies;
        QStringList changes;
        bool isStable;
        bool isCompatible;
    };

    struct UpgradeInfo {
        QString moduleName;
        QVersionNumber currentVersion;
        QVersionNumber targetVersion;
        UpgradeStatus status;
        QString description;
        QStringList requirements;
        int progress;           // 升级进度 0-100
        QString errorMessage;
    };

    virtual ~IVersionManager() = default;

    // 版本信息管理
    virtual QVersionNumber getModuleVersion(const QString& moduleName) const = 0;
    virtual VersionInfo getVersionInfo(const QString& moduleName) const = 0;
    virtual QList<VersionInfo> getAllVersions(const QString& moduleName) const = 0;
    virtual bool setModuleVersion(const QString& moduleName, const QVersionNumber& version) = 0;

    // 版本兼容性检查
    virtual bool isVersionCompatible(const QString& moduleName, const QVersionNumber& version) const = 0;
    virtual QVersionNumber getMinimumVersion(const QString& moduleName) const = 0;
    virtual QVersionNumber getMaximumVersion(const QString& moduleName) const = 0;
    virtual QStringList getIncompatibleModules(const QString& moduleName) const = 0;

    // 升级管理
    virtual QList<UpgradeInfo> checkForUpdates() = 0;
    virtual UpgradeInfo checkModuleUpdate(const QString& moduleName) = 0;
    virtual bool startUpgrade(const QString& moduleName, const QVersionNumber& targetVersion) = 0;
    virtual bool cancelUpgrade(const QString& moduleName) = 0;
    virtual UpgradeStatus getUpgradeStatus(const QString& moduleName) const = 0;

    // 回滚管理
    virtual bool canRollback(const QString& moduleName) const = 0;
    virtual bool rollbackModule(const QString& moduleName) = 0;
    virtual QVersionNumber getPreviousVersion(const QString& moduleName) const = 0;
    virtual QList<QVersionNumber> getVersionHistory(const QString& moduleName) const = 0;

    // 依赖版本管理
    virtual bool validateDependencyVersions(const QString& moduleName) const = 0;
    virtual QStringList getVersionConflicts() const = 0;
    virtual bool resolveDependencyConflicts() = 0;

    // 版本策略
    virtual void setAutoUpgrade(const QString& moduleName, bool enabled) = 0;
    virtual bool isAutoUpgradeEnabled(const QString& moduleName) const = 0;
    virtual void setUpgradePolicy(const QString& moduleName, VersionType maxAutoUpgrade) = 0;
    virtual VersionType getUpgradePolicy(const QString& moduleName) const = 0;

signals:
    void versionChanged(const QString& moduleName, const QVersionNumber& oldVersion, const QVersionNumber& newVersion);
    void upgradeAvailable(const QString& moduleName, const QVersionNumber& newVersion);
    void upgradeStarted(const QString& moduleName, const QVersionNumber& targetVersion);
    void upgradeProgress(const QString& moduleName, int progress);
    void upgradeCompleted(const QString& moduleName, bool success);
    void upgradeFailed(const QString& moduleName, const QString& error);
    void rollbackStarted(const QString& moduleName);
    void rollbackCompleted(const QString& moduleName, bool success);
    void versionConflictDetected(const QStringList& conflictingModules);
};

Q_DECLARE_METATYPE(IVersionManager::VersionInfo)
Q_DECLARE_METATYPE(IVersionManager::UpgradeInfo)

#endif // IVERSIONMANAGER_H