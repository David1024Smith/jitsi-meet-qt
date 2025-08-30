#include "CheckpointManager.h"
#include <QDebug>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QCoreApplication>

CheckpointManager::CheckpointManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
    // 设置默认检查点目录
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_checkpointDirectory = QDir(appDataPath).absoluteFilePath("checkpoints");
}

CheckpointManager::~CheckpointManager()
{
}

bool CheckpointManager::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing CheckpointManager...";

    // 创建检查点目录
    QDir dir;
    if (!dir.exists(m_checkpointDirectory)) {
        if (!dir.mkpath(m_checkpointDirectory)) {
            qWarning() << "Failed to create checkpoint directory:" << m_checkpointDirectory;
            return false;
        }
    }

    m_initialized = true;
    qDebug() << "CheckpointManager initialized successfully";
    qDebug() << "Checkpoint directory:" << m_checkpointDirectory;
    
    return true;
}

void CheckpointManager::setCheckpointDirectory(const QString& directory)
{
    QMutexLocker locker(&m_mutex);
    m_checkpointDirectory = directory;
}

QString CheckpointManager::getCheckpointDirectory() const
{
    QMutexLocker locker(&m_mutex);
    return m_checkpointDirectory;
}

bool CheckpointManager::createCheckpoint(const QString& checkpointName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        qWarning() << "CheckpointManager not initialized";
        return false;
    }

    qDebug() << "Creating checkpoint:" << checkpointName;
    
    emit progressUpdated(0);

    QString checkpointPath = QDir(m_checkpointDirectory).absoluteFilePath(checkpointName);
    
    // 检查检查点是否已存在
    if (QDir(checkpointPath).exists()) {
        qWarning() << "Checkpoint already exists:" << checkpointName;
        return false;
    }

    emit progressUpdated(10);

    // 创建检查点目录结构
    if (!createCheckpointStructure(checkpointPath)) {
        qWarning() << "Failed to create checkpoint structure";
        emit checkpointCreated(checkpointName, false);
        return false;
    }

    emit progressUpdated(30);

    // 复制系统文件
    if (!copySystemFiles(checkpointPath)) {
        qWarning() << "Failed to copy system files";
        emit checkpointCreated(checkpointName, false);
        return false;
    }

    emit progressUpdated(80);

    // 创建清单文件
    if (!createManifest(checkpointPath)) {
        qWarning() << "Failed to create manifest";
        emit checkpointCreated(checkpointName, false);
        return false;
    }

    emit progressUpdated(100);
    emit checkpointCreated(checkpointName, true);
    
    qDebug() << "Checkpoint created successfully:" << checkpointName;
    return true;
}

bool CheckpointManager::deleteCheckpoint(const QString& checkpointName)
{
    QMutexLocker locker(&m_mutex);
    
    QString checkpointPath = QDir(m_checkpointDirectory).absoluteFilePath(checkpointName);
    QDir checkpointDir(checkpointPath);
    
    if (!checkpointDir.exists()) {
        qWarning() << "Checkpoint does not exist:" << checkpointName;
        return false;
    }

    bool success = checkpointDir.removeRecursively();
    emit checkpointDeleted(checkpointName, success);
    
    if (success) {
        qDebug() << "Checkpoint deleted successfully:" << checkpointName;
    } else {
        qWarning() << "Failed to delete checkpoint:" << checkpointName;
    }
    
    return success;
}

bool CheckpointManager::validateCheckpoint(const QString& checkpointName)
{
    QMutexLocker locker(&m_mutex);
    
    QString checkpointPath = QDir(m_checkpointDirectory).absoluteFilePath(checkpointName);
    QDir checkpointDir(checkpointPath);
    
    if (!checkpointDir.exists()) {
        return false;
    }

    // 检查必要的文件
    QStringList requiredFiles = {"manifest.json", "config", "state"};
    for (const QString& file : requiredFiles) {
        QString filePath = checkpointDir.absoluteFilePath(file);
        if (!QFileInfo::exists(filePath)) {
            qWarning() << "Missing required file in checkpoint:" << file;
            return false;
        }
    }

    // 验证清单文件
    QString manifestPath = checkpointDir.absoluteFilePath("manifest.json");
    QFile manifestFile(manifestPath);
    
    if (!manifestFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open manifest file";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(manifestFile.readAll());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid manifest file format";
        return false;
    }

    QJsonObject manifest = doc.object();
    if (!manifest.contains("checkpoint_name") || 
        !manifest.contains("timestamp") ||
        !manifest.contains("version")) {
        qWarning() << "Incomplete manifest file";
        return false;
    }

    return true;
}

QStringList CheckpointManager::listCheckpoints() const
{
    QMutexLocker locker(&m_mutex);
    
    QDir dir(m_checkpointDirectory);
    if (!dir.exists()) {
        return QStringList();
    }

    QStringList checkpoints;
    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QString& entry : entries) {
        // 验证是否为有效的检查点
        QString checkpointPath = dir.absoluteFilePath(entry);
        QDir checkpointDir(checkpointPath);
        
        if (checkpointDir.exists("manifest.json")) {
            checkpoints.append(entry);
        }
    }
    
    return checkpoints;
}

qint64 CheckpointManager::getCheckpointSize(const QString& checkpointName) const
{
    QMutexLocker locker(&m_mutex);
    
    QString checkpointPath = QDir(m_checkpointDirectory).absoluteFilePath(checkpointName);
    QDir checkpointDir(checkpointPath);
    
    if (!checkpointDir.exists()) {
        return 0;
    }

    // 简化的大小计算 - 实际实现应该递归计算目录大小
    qint64 totalSize = 0;
    QFileInfoList files = checkpointDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Size);
    
    for (const QFileInfo& fileInfo : files) {
        if (fileInfo.isFile()) {
            totalSize += fileInfo.size();
        }
        // 对于目录，这里应该递归计算，但为了简化暂时跳过
    }
    
    return totalSize;
}

bool CheckpointManager::createCheckpointStructure(const QString& checkpointPath)
{
    QDir dir;
    
    // 创建主检查点目录
    if (!dir.mkpath(checkpointPath)) {
        return false;
    }

    // 创建子目录
    QStringList subdirs = {"config", "state", "logs", "temp"};
    for (const QString& subdir : subdirs) {
        QString subdirPath = QDir(checkpointPath).absoluteFilePath(subdir);
        if (!dir.mkpath(subdirPath)) {
            qWarning() << "Failed to create subdirectory:" << subdir;
            return false;
        }
    }

    return true;
}

bool CheckpointManager::copySystemFiles(const QString& checkpointPath)
{
    // 这里应该复制重要的系统文件和配置
    // 为了演示，我们创建一些占位符文件
    
    QDir checkpointDir(checkpointPath);
    
    // 创建配置文件备份
    QString configPath = checkpointDir.absoluteFilePath("config/app_config.json");
    QFile configFile(configPath);
    if (configFile.open(QIODevice::WriteOnly)) {
        QJsonObject config;
        config["backup_timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        config["application_version"] = QCoreApplication::applicationVersion();
        config["checkpoint_type"] = "system_backup";
        
        QJsonDocument doc(config);
        configFile.write(doc.toJson());
        configFile.close();
    }

    // 创建状态文件备份
    QString statePath = checkpointDir.absoluteFilePath("state/app_state.json");
    QFile stateFile(statePath);
    if (stateFile.open(QIODevice::WriteOnly)) {
        QJsonObject state;
        state["modules_loaded"] = QJsonArray();
        state["active_connections"] = 0;
        state["last_activity"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        QJsonDocument doc(state);
        stateFile.write(doc.toJson());
        stateFile.close();
    }

    return true;
}

bool CheckpointManager::createManifest(const QString& checkpointPath)
{
    QString manifestPath = QDir(checkpointPath).absoluteFilePath("manifest.json");
    QFile manifestFile(manifestPath);
    
    if (!manifestFile.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonObject manifest;
    manifest["checkpoint_name"] = QFileInfo(checkpointPath).baseName();
    manifest["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    manifest["version"] = "1.0.0";
    manifest["created_by"] = "CheckpointManager";
    manifest["application_version"] = QCoreApplication::applicationVersion();
    
    QJsonArray files;
    files.append("config/app_config.json");
    files.append("state/app_state.json");
    manifest["files"] = files;
    
    QJsonDocument doc(manifest);
    manifestFile.write(doc.toJson());
    manifestFile.close();
    
    return true;
}