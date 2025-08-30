#ifndef MODULEVERSIONMANAGER_H
#define MODULEVERSIONMANAGER_H

#include "interfaces/IVersionManager.h"
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>

/**
 * @brief 模块版本管理器
 * 
 * 实现模块版本管理和升级机制
 */
class ModuleVersionManager : public IVersionManager
{
    Q_OBJECT

public:
    explicit ModuleVersionManager(QObject* parent = nullptr);
    ~ModuleVersionManager();

    // IVersionManager接口实现
    QVersionNumber getModuleVersion(const QString& moduleName) const override;
    VersionInfo getVersionInfo(const QString& moduleName) const override;
    QList<VersionInfo> getAllVersions(const QString& moduleName) const override;
    bool setModuleVersion(const QString& moduleName, const QVersionNumber& version) override;

    bool isVersionCompatible(const QString& moduleName, const QVersionNumber& version) const override;
    QVersionNumber getMinimumVersion(const QString& moduleName) const override;
    QVersionNumber getMaximumVersion(const QString& moduleName) const override;
    QStringList getIncompatibleModules(const QString& moduleName) const override;

    QList<UpgradeInfo> checkForUpdates() override;
    UpgradeInfo checkModuleUpdate(const QString& moduleName) override;
    bool startUpgrade(const QString& moduleName, const QVersionNumber& targetVersion) override;
    bool cancelUpgrade(const QString& moduleName) override;
    UpgradeStatus getUpgradeStatus(const QString& moduleName) const override;

    bool canRollback(const QString& moduleName) const override;
    bool rollbackModule(const QString& moduleName) override;
    QVersionNumber getPreviousVersion(const QString& moduleName) const override;
    QList<QVersionNumber> getVersionHistory(const QString& moduleName) const override;

    bool validateDependencyVersions(const QString& moduleName) const override;
    QStringList getVersionConflicts() const override;
    bool resolveDependencyConflicts() override;

    void setAutoUpgrade(const QString& moduleName, bool enabled) override;
    bool isAutoUpgradeEnabled(const QString& moduleName) const override;
    void setUpgradePolicy(const QString& moduleName, VersionType maxAutoUpgrade) override;
    VersionType getUpgradePolicy(const QString& moduleName) const override;

    // 扩展功能
    void setUpdateCheckInterval(int intervalMs);
    int getUpdateCheckInterval() const;
    void setUpdateServerUrl(const QString& url);
    QString getUpdateServerUrl() const;
    
    // 版本仓库管理
    bool addVersionRepository(const QString& name, const QString& url);
    bool removeVersionRepository(const QString& name);
    QStringList getVersionRepositories() const;
    
    // 缓存管理
    void clearVersionCache();
    void refreshVersionCache();
    bool isVersionCacheValid() const;

public slots:
    void checkForUpdatesAsync();
    void onNetworkReplyFinished();

private slots:
    void performScheduledUpdateCheck();

private:
    struct ModuleVersionData {
        QString moduleName;
        QVersionNumber currentVersion;
        QVersionNumber previousVersion;
        QList<QVersionNumber> versionHistory;
        QList<VersionInfo> availableVersions;
        QVersionNumber minVersion;
        QVersionNumber maxVersion;
        UpgradeStatus upgradeStatus;
        UpgradeInfo currentUpgrade;
        bool autoUpgradeEnabled;
        VersionType upgradePolicy;
        QDateTime lastUpdateCheck;
        QStringList compatibilityRules;
    };

    struct VersionRepository {
        QString name;
        QString url;
        bool enabled;
        QDateTime lastSync;
    };

    void initializeVersionData();
    void loadVersionConfiguration();
    void saveVersionConfiguration();
    
    bool downloadVersionInfo(const QString& moduleName);
    bool parseVersionResponse(const QString& moduleName, const QByteArray& data);
    bool performUpgrade(const QString& moduleName, const QVersionNumber& targetVersion);
    bool validateUpgradeRequirements(const QString& moduleName, const QVersionNumber& targetVersion);
    
    VersionInfo createVersionInfo(const QString& moduleName, const QVersionNumber& version);
    bool checkVersionCompatibility(const QString& moduleName, const QVersionNumber& version) const;
    QStringList findDependencyConflicts(const QString& moduleName, const QVersionNumber& version) const;
    
    void updateUpgradeProgress(const QString& moduleName, int progress);
    void completeUpgrade(const QString& moduleName, bool success);
    void addToVersionHistory(const QString& moduleName, const QVersionNumber& version);

    QMap<QString, ModuleVersionData> m_versionData;
    QMap<QString, VersionRepository> m_repositories;
    QNetworkAccessManager* m_networkManager;
    QTimer* m_updateCheckTimer;
    
    QString m_updateServerUrl;
    int m_updateCheckInterval;
    bool m_cacheValid;
    QDateTime m_lastCacheUpdate;
    
    mutable QMutex m_mutex;
};

#endif // MODULEVERSIONMANAGER_H