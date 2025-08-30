#include "StateBackup.h"
#include <QDebug>
#include <QMutexLocker>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QCoreApplication>
#include <QSettings>

StateBackup::StateBackup(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
    // 设置默认备份项目
    m_backupItems << "application_state" 
                  << "module_states" 
                  << "user_settings" 
                  << "database_state";
}

StateBackup::~StateBackup()
{
}

bool StateBackup::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing StateBackup...";

    // 这里可以进行初始化工作，比如检查权限、创建临时目录等
    
    m_initialized = true;
    qDebug() << "StateBackup initialized successfully";
    
    return true;
}

bool StateBackup::createBackup(const QString& backupPath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        qWarning() << "StateBackup not initialized";
        return false;
    }

    qDebug() << "Creating state backup at:" << backupPath;
    
    emit progressUpdated(0);

    QDir backupDir(backupPath);
    if (!backupDir.exists()) {
        qWarning() << "Backup directory does not exist:" << backupPath;
        return false;
    }

    bool success = true;
    int totalItems = m_backupItems.size();
    int completedItems = 0;

    for (const QString& item : m_backupItems) {
        bool itemSuccess = false;
        
        if (item == "application_state") {
            itemSuccess = backupApplicationState(backupPath);
        } else if (item == "module_states") {
            itemSuccess = backupModuleStates(backupPath);
        } else if (item == "user_settings") {
            itemSuccess = backupUserSettings(backupPath);
        } else if (item == "database_state") {
            itemSuccess = backupDatabaseState(backupPath);
        } else {
            qWarning() << "Unknown backup item:" << item;
            itemSuccess = false;
        }

        if (!itemSuccess) {
            qWarning() << "Failed to backup item:" << item;
            success = false;
        }

        completedItems++;
        int progress = (completedItems * 100) / totalItems;
        emit progressUpdated(progress);
    }

    emit backupCreated(backupPath, success);
    
    if (success) {
        qDebug() << "State backup created successfully";
    } else {
        qWarning() << "State backup completed with errors";
    }
    
    return success;
}

bool StateBackup::restoreBackup(const QString& backupPath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        qWarning() << "StateBackup not initialized";
        return false;
    }

    qDebug() << "Restoring state backup from:" << backupPath;
    
    if (!validateBackup(backupPath)) {
        qWarning() << "Backup validation failed";
        return false;
    }

    emit progressUpdated(0);

    bool success = true;
    int totalItems = m_backupItems.size();
    int completedItems = 0;

    for (const QString& item : m_backupItems) {
        bool itemSuccess = false;
        
        if (item == "application_state") {
            itemSuccess = restoreApplicationState(backupPath);
        } else if (item == "module_states") {
            itemSuccess = restoreModuleStates(backupPath);
        } else if (item == "user_settings") {
            itemSuccess = restoreUserSettings(backupPath);
        } else if (item == "database_state") {
            itemSuccess = restoreDatabaseState(backupPath);
        } else {
            qWarning() << "Unknown restore item:" << item;
            itemSuccess = false;
        }

        if (!itemSuccess) {
            qWarning() << "Failed to restore item:" << item;
            success = false;
        }

        completedItems++;
        int progress = (completedItems * 100) / totalItems;
        emit progressUpdated(progress);
    }

    emit backupRestored(backupPath, success);
    
    if (success) {
        qDebug() << "State backup restored successfully";
    } else {
        qWarning() << "State backup restoration completed with errors";
    }
    
    return success;
}

void StateBackup::setBackupItems(const QStringList& items)
{
    QMutexLocker locker(&m_mutex);
    m_backupItems = items;
}

QStringList StateBackup::getBackupItems() const
{
    QMutexLocker locker(&m_mutex);
    return m_backupItems;
}

bool StateBackup::validateBackup(const QString& backupPath)
{
    QDir backupDir(backupPath);
    if (!backupDir.exists()) {
        return false;
    }

    // 检查必要的备份文件是否存在
    QStringList requiredFiles;
    for (const QString& item : m_backupItems) {
        QString fileName = QString("%1.json").arg(item);
        if (!backupDir.exists(fileName)) {
            qWarning() << "Missing backup file:" << fileName;
            return false;
        }
        requiredFiles.append(fileName);
    }

    // 验证文件格式
    for (const QString& fileName : requiredFiles) {
        QString filePath = backupDir.absoluteFilePath(fileName);
        QFile file(filePath);
        
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Cannot open backup file:" << fileName;
            return false;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isNull()) {
            qWarning() << "Invalid JSON in backup file:" << fileName;
            return false;
        }
    }

    return true;
}

bool StateBackup::backupApplicationState(const QString& backupPath)
{
    QVariantMap appState = getCurrentApplicationState();
    
    QString filePath = QDir(backupPath).absoluteFilePath("application_state.json");
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create application state backup file";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromVariant(appState);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool StateBackup::backupModuleStates(const QString& backupPath)
{
    QVariantMap moduleStates = getCurrentModuleStates();
    
    QString filePath = QDir(backupPath).absoluteFilePath("module_states.json");
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create module states backup file";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromVariant(moduleStates);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool StateBackup::backupUserSettings(const QString& backupPath)
{
    QVariantMap userSettings = getCurrentUserSettings();
    
    QString filePath = QDir(backupPath).absoluteFilePath("user_settings.json");
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create user settings backup file";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromVariant(userSettings);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool StateBackup::backupDatabaseState(const QString& backupPath)
{
    // 这里应该备份数据库状态
    // 为了演示，我们创建一个占位符文件
    
    QVariantMap dbState;
    dbState["backup_timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    dbState["database_version"] = "1.0.0";
    dbState["table_count"] = 0;
    dbState["record_count"] = 0;
    
    QString filePath = QDir(backupPath).absoluteFilePath("database_state.json");
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create database state backup file";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromVariant(dbState);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool StateBackup::restoreApplicationState(const QString& backupPath)
{
    QString filePath = QDir(backupPath).absoluteFilePath("application_state.json");
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open application state backup file";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QVariantMap appState = doc.toVariant().toMap();
    
    return setApplicationState(appState);
}

bool StateBackup::restoreModuleStates(const QString& backupPath)
{
    QString filePath = QDir(backupPath).absoluteFilePath("module_states.json");
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open module states backup file";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QVariantMap moduleStates = doc.toVariant().toMap();
    
    return setModuleStates(moduleStates);
}

bool StateBackup::restoreUserSettings(const QString& backupPath)
{
    QString filePath = QDir(backupPath).absoluteFilePath("user_settings.json");
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open user settings backup file";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QVariantMap userSettings = doc.toVariant().toMap();
    
    return setUserSettings(userSettings);
}

bool StateBackup::restoreDatabaseState(const QString& backupPath)
{
    QString filePath = QDir(backupPath).absoluteFilePath("database_state.json");
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open database state backup file";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QVariantMap dbState = doc.toVariant().toMap();
    
    // 这里应该恢复数据库状态
    // 为了演示，我们只是记录日志
    qDebug() << "Restoring database state:" << dbState;
    
    return true;
}

QVariantMap StateBackup::getCurrentApplicationState()
{
    QVariantMap state;
    state["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    state["application_version"] = QCoreApplication::applicationVersion();
    state["organization_name"] = QCoreApplication::organizationName();
    state["application_name"] = QCoreApplication::applicationName();
    
    // 这里应该收集实际的应用程序状态
    state["window_geometry"] = QVariant();
    state["active_modules"] = QStringList();
    state["current_user"] = QString();
    
    return state;
}

QVariantMap StateBackup::getCurrentModuleStates()
{
    QVariantMap states;
    states["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // 这里应该收集所有模块的状态
    QVariantMap audioState;
    audioState["enabled"] = true;
    audioState["volume"] = 1.0;
    audioState["muted"] = false;
    states["audio"] = audioState;
    
    QVariantMap cameraState;
    cameraState["enabled"] = true;
    cameraState["resolution"] = "1920x1080";
    cameraState["fps"] = 30;
    states["camera"] = cameraState;
    
    return states;
}

QVariantMap StateBackup::getCurrentUserSettings()
{
    QVariantMap settings;
    settings["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // 从QSettings读取用户设置
    QSettings qsettings;
    qsettings.beginGroup("UserPreferences");
    
    QStringList keys = qsettings.allKeys();
    for (const QString& key : keys) {
        settings[key] = qsettings.value(key);
    }
    
    qsettings.endGroup();
    
    return settings;
}

bool StateBackup::setApplicationState(const QVariantMap& state)
{
    // 这里应该恢复应用程序状态
    qDebug() << "Restoring application state:" << state.keys();
    
    // 示例：恢复窗口几何形状等
    if (state.contains("window_geometry")) {
        // 恢复窗口几何形状
    }
    
    return true;
}

bool StateBackup::setModuleStates(const QVariantMap& states)
{
    // 这里应该恢复模块状态
    qDebug() << "Restoring module states:" << states.keys();
    
    // 示例：恢复音频模块状态
    if (states.contains("audio")) {
        QVariantMap audioState = states["audio"].toMap();
        // 恢复音频设置
    }
    
    return true;
}

bool StateBackup::setUserSettings(const QVariantMap& settings)
{
    // 恢复用户设置到QSettings
    QSettings qsettings;
    qsettings.beginGroup("UserPreferences");
    
    for (auto it = settings.begin(); it != settings.end(); ++it) {
        if (it.key() != "timestamp") {
            qsettings.setValue(it.key(), it.value());
        }
    }
    
    qsettings.endGroup();
    qsettings.sync();
    
    qDebug() << "Restored user settings:" << settings.size() << "items";
    return true;
}