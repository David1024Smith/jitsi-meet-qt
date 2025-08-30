#include "ModuleVersionManager.h"
#include "GlobalModuleConfig.h"
#include <QDebug>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>

ModuleVersionManager::ModuleVersionManager(QObject* parent)
    : IVersionManager(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_updateCheckTimer(new QTimer(this))
    , m_updateServerUrl("https://updates.jitsi-meet-qt.org/api/versions")
    , m_updateCheckInterval(3600000) // 1小时
    , m_cacheValid(false)
{
    // 初始化版本数据
    initializeVersionData();
    
    // 设置更新检查定时器
    m_updateCheckTimer->setSingleShot(false);
    connect(m_updateCheckTimer, &QTimer::timeout, this, &ModuleVersionManager::performScheduledUpdateCheck);
    
    // 设置网络管理器
    connect(m_networkManager, &QNetworkAccessManager::finished, 
            this, &ModuleVersionManager::onNetworkReplyFinished);
    
    // 加载配置
    loadVersionConfiguration();
    
    // 启动定时检查
    m_updateCheckTimer->start(m_updateCheckInterval);
    
    qDebug() << "ModuleVersionManager initialized";
}

ModuleVersionManager::~ModuleVersionManager()
{
    saveVersionConfiguration();
}

void ModuleVersionManager::initializeVersionData()
{
    // 从GlobalModuleConfig获取已注册的模块
    GlobalModuleConfig* config = GlobalModuleConfig::instance();
    QStringList modules = config->getAvailableModules();
    
    QMutexLocker locker(&m_mutex);
    
    for (const QString& moduleName : modules) {
        if (!m_versionData.contains(moduleName)) {
            ModuleVersionData data;
            data.moduleName = moduleName;
            
            // 从模块配置获取当前版本
            auto moduleInfo = config->getModuleInfo(moduleName);
            data.currentVersion = QVersionNumber::fromString(moduleInfo.version);
            data.previousVersion = data.currentVersion;
            data.versionHistory.append(data.currentVersion);
            
            // 设置默认值
            data.minVersion = QVersionNumber(1, 0, 0);
            data.maxVersion = QVersionNumber(99, 99, 99);
            data.upgradeStatus = NoUpgrade;
            data.autoUpgradeEnabled = false;
            data.upgradePolicy = Minor; // 默认允许自动次版本升级
            data.lastUpdateCheck = QDateTime::currentDateTime();
            
            m_versionData[moduleName] = data;
        }
    }
}

QVersionNumber ModuleVersionManager::getModuleVersion(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_versionData.contains(moduleName)) {
        return m_versionData[moduleName].currentVersion;
    }
    
    return QVersionNumber();
}

IVersionManager::VersionInfo ModuleVersionManager::getVersionInfo(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    
    VersionInfo info;
    
    if (m_versionData.contains(moduleName)) {
        const ModuleVersionData& data = m_versionData[moduleName];
        info.moduleName = moduleName;
        info.version = data.currentVersion;
        info.description = QString("Current version of %1").arg(moduleName);
        info.releaseDate = QDateTime::currentDateTime();
        info.isStable = true;
        info.isCompatible = true;
        
        // 获取依赖信息
        GlobalModuleConfig* config = GlobalModuleConfig::instance();
        auto moduleInfo = config->getModuleInfo(moduleName);
        info.dependencies = moduleInfo.dependencies;
    }
    
    return info;
}

QList<IVersionManager::VersionInfo> ModuleVersionManager::getAllVersions(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    
    QList<VersionInfo> versions;
    
    if (m_versionData.contains(moduleName)) {
        const ModuleVersionData& data = m_versionData[moduleName];
        
        // 返回可用版本列表
        for (const VersionInfo& versionInfo : data.availableVersions) {
            versions.append(versionInfo);
        }
        
        // 如果没有可用版本信息，至少返回当前版本
        if (versions.isEmpty()) {
            VersionInfo currentInfo = getVersionInfo(moduleName);
            versions.append(currentInfo);
        }
    }
    
    return versions;
}

bool ModuleVersionManager::setModuleVersion(const QString& moduleName, const QVersionNumber& version)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_versionData.contains(moduleName)) {
        qWarning() << "Module not found:" << moduleName;
        return false;
    }
    
    ModuleVersionData& data = m_versionData[moduleName];
    QVersionNumber oldVersion = data.currentVersion;
    
    // 验证版本兼容性
    if (!checkVersionCompatibility(moduleName, version)) {
        qWarning() << "Version" << version.toString() << "is not compatible with module" << moduleName;
        return false;
    }
    
    // 更新版本信息
    data.previousVersion = data.currentVersion;
    data.currentVersion = version;
    addToVersionHistory(moduleName, version);
    
    // 更新GlobalModuleConfig中的版本信息
    GlobalModuleConfig* config = GlobalModuleConfig::instance();
    auto moduleInfo = config->getModuleInfo(moduleName);
    moduleInfo.version = version.toString();
    config->setModuleInfo(moduleName, moduleInfo);
    
    emit versionChanged(moduleName, oldVersion, version);
    
    qDebug() << "Module" << moduleName << "version updated from" << oldVersion.toString() 
             << "to" << version.toString();
    
    return true;
}

bool ModuleVersionManager::isVersionCompatible(const QString& moduleName, const QVersionNumber& version) const
{
    return checkVersionCompatibility(moduleName, version);
}

QVersionNumber ModuleVersionManager::getMinimumVersion(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_versionData.contains(moduleName)) {
        return m_versionData[moduleName].minVersion;
    }
    
    return QVersionNumber(1, 0, 0);
}

QVersionNumber ModuleVersionManager::getMaximumVersion(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_versionData.contains(moduleName)) {
        return m_versionData[moduleName].maxVersion;
    }
    
    return QVersionNumber(99, 99, 99);
}

QList<IVersionManager::UpgradeInfo> ModuleVersionManager::checkForUpdates()
{
    QList<UpgradeInfo> updates;
    
    QMutexLocker locker(&m_mutex);
    
    for (const QString& moduleName : m_versionData.keys()) {
        UpgradeInfo upgrade = checkModuleUpdate(moduleName);
        if (upgrade.status == Available) {
            updates.append(upgrade);
        }
    }
    
    return updates;
}

IVersionManager::UpgradeInfo ModuleVersionManager::checkModuleUpdate(const QString& moduleName)
{
    UpgradeInfo upgrade;
    upgrade.moduleName = moduleName;
    upgrade.status = NoUpgrade;
    upgrade.progress = 0;
    
    QMutexLocker locker(&m_mutex);
    
    if (!m_versionData.contains(moduleName)) {
        upgrade.errorMessage = "Module not found";
        return upgrade;
    }
    
    const ModuleVersionData& data = m_versionData[moduleName];
    upgrade.currentVersion = data.currentVersion;
    
    // 检查是否有可用的更新版本
    QVersionNumber latestVersion = data.currentVersion;
    
    for (const VersionInfo& versionInfo : data.availableVersions) {
        if (versionInfo.version > latestVersion && versionInfo.isStable) {
            latestVersion = versionInfo.version;
        }
    }
    
    if (latestVersion > data.currentVersion) {
        upgrade.targetVersion = latestVersion;
        upgrade.status = Available;
        upgrade.description = QString("Update available: %1 -> %2")
            .arg(data.currentVersion.toString())
            .arg(latestVersion.toString());
        
        // 检查升级要求
        QStringList conflicts = findDependencyConflicts(moduleName, latestVersion);
        if (!conflicts.isEmpty()) {
            upgrade.requirements = conflicts;
        }
    }
    
    return upgrade;
}

bool ModuleVersionManager::startUpgrade(const QString& moduleName, const QVersionNumber& targetVersion)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_versionData.contains(moduleName)) {
        qWarning() << "Module not found:" << moduleName;
        return false;
    }
    
    ModuleVersionData& data = m_versionData[moduleName];
    
    // 检查是否已在升级中
    if (data.upgradeStatus == InProgress) {
        qWarning() << "Module" << moduleName << "is already being upgraded";
        return false;
    }
    
    // 验证升级要求
    if (!validateUpgradeRequirements(moduleName, targetVersion)) {
        qWarning() << "Upgrade requirements not met for module" << moduleName;
        return false;
    }
    
    // 开始升级
    data.upgradeStatus = InProgress;
    data.currentUpgrade.moduleName = moduleName;
    data.currentUpgrade.currentVersion = data.currentVersion;
    data.currentUpgrade.targetVersion = targetVersion;
    data.currentUpgrade.status = InProgress;
    data.currentUpgrade.progress = 0;
    
    emit upgradeStarted(moduleName, targetVersion);
    
    // 在后台执行升级
    QTimer::singleShot(0, this, [this, moduleName, targetVersion]() {
        bool success = performUpgrade(moduleName, targetVersion);
        completeUpgrade(moduleName, success);
    });
    
    return true;
}

bool ModuleVersionManager::performUpgrade(const QString& moduleName, const QVersionNumber& targetVersion)
{
    // 模拟升级过程
    for (int i = 0; i <= 100; i += 10) {
        updateUpgradeProgress(moduleName, i);
        QThread::msleep(100); // 模拟升级时间
        
        // 检查是否被取消
        QMutexLocker locker(&m_mutex);
        if (m_versionData[moduleName].upgradeStatus != InProgress) {
            return false; // 升级被取消
        }
    }
    
    // 实际升级逻辑应该在这里实现
    // 例如：下载新版本、替换文件、更新配置等
    
    return setModuleVersion(moduleName, targetVersion);
}

void ModuleVersionManager::updateUpgradeProgress(const QString& moduleName, int progress)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_versionData.contains(moduleName)) {
        m_versionData[moduleName].currentUpgrade.progress = progress;
        emit upgradeProgress(moduleName, progress);
    }
}

void ModuleVersionManager::completeUpgrade(const QString& moduleName, bool success)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_versionData.contains(moduleName)) {
        ModuleVersionData& data = m_versionData[moduleName];
        
        if (success) {
            data.upgradeStatus = Completed;
            data.currentUpgrade.status = Completed;
            emit upgradeCompleted(moduleName, true);
            qDebug() << "Upgrade completed successfully for module:" << moduleName;
        } else {
            data.upgradeStatus = Failed;
            data.currentUpgrade.status = Failed;
            data.currentUpgrade.errorMessage = "Upgrade failed";
            emit upgradeFailed(moduleName, "Upgrade process failed");
            qWarning() << "Upgrade failed for module:" << moduleName;
        }
    }
}

bool ModuleVersionManager::checkVersionCompatibility(const QString& moduleName, const QVersionNumber& version) const
{
    if (!m_versionData.contains(moduleName)) {
        return false;
    }
    
    const ModuleVersionData& data = m_versionData[moduleName];
    
    // 检查版本范围
    if (version < data.minVersion || version > data.maxVersion) {
        return false;
    }
    
    // 检查依赖兼容性
    QStringList conflicts = findDependencyConflicts(moduleName, version);
    return conflicts.isEmpty();
}

QStringList ModuleVersionManager::findDependencyConflicts(const QString& moduleName, const QVersionNumber& version) const
{
    QStringList conflicts;
    
    // 简化实现，实际应该检查所有依赖模块的版本兼容性
    Q_UNUSED(moduleName)
    Q_UNUSED(version)
    
    return conflicts;
}

void ModuleVersionManager::addToVersionHistory(const QString& moduleName, const QVersionNumber& version)
{
    if (m_versionData.contains(moduleName)) {
        ModuleVersionData& data = m_versionData[moduleName];
        
        // 避免重复添加
        if (!data.versionHistory.contains(version)) {
            data.versionHistory.append(version);
            
            // 限制历史记录大小
            if (data.versionHistory.size() > 20) {
                data.versionHistory.removeFirst();
            }
        }
    }
}

bool ModuleVersionManager::validateUpgradeRequirements(const QString& moduleName, const QVersionNumber& targetVersion)
{
    // 检查版本兼容性
    if (!checkVersionCompatibility(moduleName, targetVersion)) {
        return false;
    }
    
    // 检查依赖冲突
    QStringList conflicts = findDependencyConflicts(moduleName, targetVersion);
    if (!conflicts.isEmpty()) {
        qWarning() << "Dependency conflicts found:" << conflicts;
        return false;
    }
    
    return true;
}

void ModuleVersionManager::loadVersionConfiguration()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString configFile = configDir + "/versions.conf";
    
    QSettings settings(configFile, QSettings::IniFormat);
    
    QMutexLocker locker(&m_mutex);
    
    // 加载版本数据
    settings.beginGroup("Versions");
    QStringList modules = settings.childGroups();
    
    for (const QString& moduleName : modules) {
        settings.beginGroup(moduleName);
        
        if (m_versionData.contains(moduleName)) {
            ModuleVersionData& data = m_versionData[moduleName];
            
            data.autoUpgradeEnabled = settings.value("autoUpgrade", false).toBool();
            data.upgradePolicy = static_cast<VersionType>(settings.value("upgradePolicy", Minor).toInt());
            
            // 加载版本历史
            QStringList historyStrings = settings.value("versionHistory").toStringList();
            data.versionHistory.clear();
            for (const QString& versionStr : historyStrings) {
                QVersionNumber version = QVersionNumber::fromString(versionStr);
                if (!version.isNull()) {
                    data.versionHistory.append(version);
                }
            }
        }
        
        settings.endGroup();
    }
    settings.endGroup();
    
    // 加载仓库配置
    settings.beginGroup("Repositories");
    QStringList repoNames = settings.childGroups();
    
    for (const QString& repoName : repoNames) {
        settings.beginGroup(repoName);
        
        VersionRepository repo;
        repo.name = repoName;
        repo.url = settings.value("url").toString();
        repo.enabled = settings.value("enabled", true).toBool();
        repo.lastSync = settings.value("lastSync").toDateTime();
        
        m_repositories[repoName] = repo;
        
        settings.endGroup();
    }
    settings.endGroup();
}

void ModuleVersionManager::saveVersionConfiguration()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    QString configFile = configDir + "/versions.conf";
    
    QSettings settings(configFile, QSettings::IniFormat);
    
    QMutexLocker locker(&m_mutex);
    
    // 保存版本数据
    settings.beginGroup("Versions");
    settings.remove(""); // 清空现有配置
    
    for (auto it = m_versionData.begin(); it != m_versionData.end(); ++it) {
        const QString& moduleName = it.key();
        const ModuleVersionData& data = it.value();
        
        settings.beginGroup(moduleName);
        settings.setValue("autoUpgrade", data.autoUpgradeEnabled);
        settings.setValue("upgradePolicy", static_cast<int>(data.upgradePolicy));
        
        // 保存版本历史
        QStringList historyStrings;
        for (const QVersionNumber& version : data.versionHistory) {
            historyStrings.append(version.toString());
        }
        settings.setValue("versionHistory", historyStrings);
        
        settings.endGroup();
    }
    settings.endGroup();
    
    // 保存仓库配置
    settings.beginGroup("Repositories");
    settings.remove(""); // 清空现有配置
    
    for (auto it = m_repositories.begin(); it != m_repositories.end(); ++it) {
        const QString& repoName = it.key();
        const VersionRepository& repo = it.value();
        
        settings.beginGroup(repoName);
        settings.setValue("url", repo.url);
        settings.setValue("enabled", repo.enabled);
        settings.setValue("lastSync", repo.lastSync);
        settings.endGroup();
    }
    settings.endGroup();
    
    settings.sync();
}

void ModuleVersionManager::performScheduledUpdateCheck()
{
    qDebug() << "Performing scheduled update check";
    checkForUpdatesAsync();
}

void ModuleVersionManager::checkForUpdatesAsync()
{
    // 异步检查更新
    QTimer::singleShot(0, this, [this]() {
        QList<UpgradeInfo> updates = checkForUpdates();
        
        for (const UpgradeInfo& upgrade : updates) {
            emit upgradeAvailable(upgrade.moduleName, upgrade.targetVersion);
            
            // 检查自动升级
            QMutexLocker locker(&m_mutex);
            if (m_versionData.contains(upgrade.moduleName)) {
                const ModuleVersionData& data = m_versionData[upgrade.moduleName];
                
                if (data.autoUpgradeEnabled) {
                    // 检查升级策略
                    bool shouldAutoUpgrade = false;
                    QVersionNumber current = upgrade.currentVersion;
                    QVersionNumber target = upgrade.targetVersion;
                    
                    switch (data.upgradePolicy) {
                    case Build:
                        shouldAutoUpgrade = (current.majorVersion() == target.majorVersion() &&
                                           current.minorVersion() == target.minorVersion() &&
                                           current.microVersion() == target.microVersion());
                        break;
                    case Patch:
                        shouldAutoUpgrade = (current.majorVersion() == target.majorVersion() &&
                                           current.minorVersion() == target.minorVersion());
                        break;
                    case Minor:
                        shouldAutoUpgrade = (current.majorVersion() == target.majorVersion());
                        break;
                    case Major:
                        shouldAutoUpgrade = true;
                        break;
                    }
                    
                    if (shouldAutoUpgrade) {
                        qDebug() << "Auto-upgrading module:" << upgrade.moduleName;
                        startUpgrade(upgrade.moduleName, upgrade.targetVersion);
                    }
                }
            }
        }
    });
}

void ModuleVersionManager::onNetworkReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QString moduleName = reply->property("moduleName").toString();
        
        if (!moduleName.isEmpty()) {
            parseVersionResponse(moduleName, data);
        }
    } else {
        qWarning() << "Network error:" << reply->errorString();
    }
    
    reply->deleteLater();
}

bool ModuleVersionManager::parseVersionResponse(const QString& moduleName, const QByteArray& data)
{
    try {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        
        QMutexLocker locker(&m_mutex);
        
        if (m_versionData.contains(moduleName)) {
            ModuleVersionData& moduleData = m_versionData[moduleName];
            moduleData.availableVersions.clear();
            
            QJsonArray versions = obj["versions"].toArray();
            for (const QJsonValue& versionValue : versions) {
                QJsonObject versionObj = versionValue.toObject();
                
                VersionInfo info;
                info.moduleName = moduleName;
                info.version = QVersionNumber::fromString(versionObj["version"].toString());
                info.description = versionObj["description"].toString();
                info.releaseDate = QDateTime::fromString(versionObj["releaseDate"].toString(), Qt::ISODate);
                info.isStable = versionObj["stable"].toBool();
                info.isCompatible = versionObj["compatible"].toBool();
                
                QJsonArray deps = versionObj["dependencies"].toArray();
                for (const QJsonValue& dep : deps) {
                    info.dependencies.append(dep.toString());
                }
                
                QJsonArray changes = versionObj["changes"].toArray();
                for (const QJsonValue& change : changes) {
                    info.changes.append(change.toString());
                }
                
                moduleData.availableVersions.append(info);
            }
            
            moduleData.lastUpdateCheck = QDateTime::currentDateTime();
        }
        
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to parse version response:" << e.what();
        return false;
    }
}