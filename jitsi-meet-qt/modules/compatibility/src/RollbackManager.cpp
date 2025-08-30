#include "RollbackManager.h"
#include "CheckpointManager.h"
#include "StateBackup.h"

#include <QDebug>
#include <QMutexLocker>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>

RollbackManager::RollbackManager(QObject *parent)
    : IRollbackManager(parent)
    , m_initialized(false)
    , m_status(Idle)
    , m_maxCheckpoints(50)
    , m_checkpointManager(nullptr)
    , m_stateBackup(nullptr)
    , m_autoCleanupTimer(nullptr)
    , m_autoCleanupEnabled(false)
    , m_autoCleanupInterval(7)
{
    // 设置默认检查点目录
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_checkpointDirectory = QDir(appDataPath).absoluteFilePath("checkpoints");
}

RollbackManager::~RollbackManager()
{
    if (m_autoCleanupTimer) {
        m_autoCleanupTimer->stop();
        m_autoCleanupTimer->deleteLater();
    }
    
    if (m_checkpointManager) {
        m_checkpointManager->deleteLater();
    }
    
    if (m_stateBackup) {
        m_stateBackup->deleteLater();
    }
}

bool RollbackManager::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing RollbackManager...";

    // 创建检查点目录
    if (!createCheckpointDirectory()) {
        qWarning() << "Failed to create checkpoint directory:" << m_checkpointDirectory;
        return false;
    }

    // 初始化检查点管理器
    m_checkpointManager = new CheckpointManager(this);
    m_checkpointManager->setCheckpointDirectory(m_checkpointDirectory);
    
    connect(m_checkpointManager, &CheckpointManager::checkpointCreated,
            this, &RollbackManager::onCheckpointCreated);

    // 初始化状态备份
    m_stateBackup = new StateBackup(this);
    
    connect(m_stateBackup, &StateBackup::progressUpdated,
            this, &RollbackManager::onRollbackProgress);

    // 加载现有检查点元数据
    loadCheckpointMetadata();

    // 设置自动清理定时器
    m_autoCleanupTimer = new QTimer(this);
    connect(m_autoCleanupTimer, &QTimer::timeout,
            this, &RollbackManager::performAutoCleanup);

    m_initialized = true;
    m_status = Idle;
    
    qDebug() << "RollbackManager initialized successfully";
    qDebug() << "Checkpoint directory:" << m_checkpointDirectory;
    qDebug() << "Found" << m_checkpoints.size() << "existing checkpoints";
    
    return true;
}

IRollbackManager::RollbackStatus RollbackManager::status() const
{
    QMutexLocker locker(&m_mutex);
    return m_status;
}

bool RollbackManager::createCheckpoint(const QString& checkpointName, const QString& description)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        qWarning() << "RollbackManager not initialized";
        return false;
    }

    if (!isValidCheckpointName(checkpointName)) {
        qWarning() << "Invalid checkpoint name:" << checkpointName;
        return false;
    }

    if (m_checkpoints.contains(checkpointName)) {
        qWarning() << "Checkpoint already exists:" << checkpointName;
        return false;
    }

    m_status = CreatingCheckpoint;
    emit statusChanged(m_status);
    emit progressUpdated("Creating checkpoint", 0);

    qDebug() << "Creating checkpoint:" << checkpointName;

    // 检查检查点数量限制
    if (m_checkpoints.size() >= m_maxCheckpoints) {
        qDebug() << "Maximum checkpoints reached, cleaning up oldest ones";
        cleanupExpiredCheckpoints(m_maxCheckpoints - 10);
    }

    // 创建检查点信息
    CheckpointInfo info;
    info.name = checkpointName;
    info.timestamp = QDateTime::currentDateTime();
    info.description = description.isEmpty() ? QString("Checkpoint created at %1").arg(info.timestamp.toString()) : description;
    info.size = 0;
    info.metadata["version"] = QCoreApplication::applicationVersion();
    info.metadata["created_by"] = "RollbackManager";

    emit progressUpdated("Creating checkpoint", 25);

    // 备份当前状态
    if (!backupCurrentState(checkpointName)) {
        qWarning() << "Failed to backup current state for checkpoint:" << checkpointName;
        m_status = Failed;
        emit statusChanged(m_status);
        return false;
    }

    emit progressUpdated("Creating checkpoint", 75);

    // 计算检查点大小
    QString checkpointPath = generateCheckpointPath(checkpointName);
    QDir checkpointDir(checkpointPath);
    if (checkpointDir.exists()) {
        // 计算目录大小的简化实现
        info.size = 1024 * 1024; // 占位符，实际应该递归计算
    }

    // 保存检查点信息
    m_checkpoints[checkpointName] = info;
    saveCheckpointMetadata();

    emit progressUpdated("Creating checkpoint", 100);
    m_status = Idle;
    emit statusChanged(m_status);
    emit checkpointCreated(checkpointName, true);

    updateRollbackHistory("CREATE", checkpointName, true);
    
    qDebug() << "Checkpoint created successfully:" << checkpointName;
    return true;
}b
ool RollbackManager::rollbackToCheckpoint(const QString& checkpointName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        qWarning() << "RollbackManager not initialized";
        return false;
    }

    if (!m_checkpoints.contains(checkpointName)) {
        qWarning() << "Checkpoint not found:" << checkpointName;
        return false;
    }

    if (!validateCheckpoint(checkpointName)) {
        qWarning() << "Checkpoint validation failed:" << checkpointName;
        return false;
    }

    m_status = RollingBack;
    emit statusChanged(m_status);
    emit progressUpdated("Rolling back", 0);

    qDebug() << "Rolling back to checkpoint:" << checkpointName;

    // 恢复状态
    if (!restoreStateFromCheckpoint(checkpointName)) {
        qWarning() << "Failed to restore state from checkpoint:" << checkpointName;
        m_status = Failed;
        emit statusChanged(m_status);
        emit rollbackCompleted(checkpointName, false);
        updateRollbackHistory("ROLLBACK", checkpointName, false);
        return false;
    }

    emit progressUpdated("Rolling back", 100);
    m_status = Completed;
    emit statusChanged(m_status);
    emit rollbackCompleted(checkpointName, true);

    updateRollbackHistory("ROLLBACK", checkpointName, true);
    
    // 重置状态为空闲
    m_status = Idle;
    emit statusChanged(m_status);
    
    qDebug() << "Rollback completed successfully:" << checkpointName;
    return true;
}

QStringList RollbackManager::availableCheckpoints() const
{
    QMutexLocker locker(&m_mutex);
    return m_checkpoints.keys();
}

IRollbackManager::CheckpointInfo RollbackManager::getCheckpointInfo(const QString& checkpointName) const
{
    QMutexLocker locker(&m_mutex);
    return m_checkpoints.value(checkpointName);
}

bool RollbackManager::deleteCheckpoint(const QString& checkpointName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_checkpoints.contains(checkpointName)) {
        return false;
    }

    QString checkpointPath = generateCheckpointPath(checkpointName);
    QDir checkpointDir(checkpointPath);
    
    if (checkpointDir.exists()) {
        if (!checkpointDir.removeRecursively()) {
            qWarning() << "Failed to remove checkpoint directory:" << checkpointPath;
            return false;
        }
    }

    m_checkpoints.remove(checkpointName);
    saveCheckpointMetadata();
    
    updateRollbackHistory("DELETE", checkpointName, true);
    
    qDebug() << "Checkpoint deleted:" << checkpointName;
    return true;
}

int RollbackManager::cleanupExpiredCheckpoints(int daysToKeep)
{
    QMutexLocker locker(&m_mutex);
    
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-daysToKeep);
    QStringList toDelete;
    
    for (auto it = m_checkpoints.begin(); it != m_checkpoints.end(); ++it) {
        if (it.value().timestamp < cutoffDate) {
            toDelete.append(it.key());
        }
    }

    int deletedCount = 0;
    for (const QString& checkpointName : toDelete) {
        if (deleteCheckpoint(checkpointName)) {
            deletedCount++;
        }
    }

    if (deletedCount > 0) {
        qDebug() << "Cleaned up" << deletedCount << "expired checkpoints";
    }

    return deletedCount;
}

bool RollbackManager::validateCheckpoint(const QString& checkpointName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_checkpoints.contains(checkpointName)) {
        return false;
    }

    QString checkpointPath = generateCheckpointPath(checkpointName);
    QDir checkpointDir(checkpointPath);
    
    if (!checkpointDir.exists()) {
        qWarning() << "Checkpoint directory does not exist:" << checkpointPath;
        return false;
    }

    // 检查必要的文件是否存在
    QStringList requiredFiles = {"state.json", "config.json"};
    for (const QString& file : requiredFiles) {
        if (!QFile::exists(checkpointDir.absoluteFilePath(file))) {
            qWarning() << "Required checkpoint file missing:" << file;
            return false;
        }
    }

    return true;
}

QStringList RollbackManager::getRollbackHistory() const
{
    QMutexLocker locker(&m_mutex);
    return m_rollbackHistory;
}

void RollbackManager::setAutoCleanup(bool enabled, int intervalDays)
{
    QMutexLocker locker(&m_mutex);
    
    m_autoCleanupEnabled = enabled;
    m_autoCleanupInterval = intervalDays;
    
    if (m_autoCleanupTimer) {
        if (enabled) {
            m_autoCleanupTimer->start(intervalDays * 24 * 60 * 60 * 1000); // 转换为毫秒
        } else {
            m_autoCleanupTimer->stop();
        }
    }
}

QString RollbackManager::getCheckpointDirectory() const
{
    QMutexLocker locker(&m_mutex);
    return m_checkpointDirectory;
}

void RollbackManager::setCheckpointDirectory(const QString& directory)
{
    QMutexLocker locker(&m_mutex);
    m_checkpointDirectory = directory;
}

qint64 RollbackManager::getTotalCheckpointSize() const
{
    QMutexLocker locker(&m_mutex);
    
    qint64 totalSize = 0;
    for (const auto& info : m_checkpoints) {
        totalSize += info.size;
    }
    return totalSize;
}

int RollbackManager::getMaxCheckpoints() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxCheckpoints;
}

void RollbackManager::setMaxCheckpoints(int maxCheckpoints)
{
    QMutexLocker locker(&m_mutex);
    m_maxCheckpoints = maxCheckpoints;
}

void RollbackManager::performAutoCleanup()
{
    qDebug() << "Performing automatic checkpoint cleanup";
    cleanupExpiredCheckpoints(30); // 保留30天的检查点
}

void RollbackManager::onCheckpointCreated(const QString& checkpointName, bool success)
{
    qDebug() << "Checkpoint creation completed:" << checkpointName << "Success:" << success;
}

void RollbackManager::onRollbackProgress(int percentage)
{
    emit progressUpdated("Rolling back", percentage);
}

bool RollbackManager::createCheckpointDirectory()
{
    QDir dir;
    if (!dir.exists(m_checkpointDirectory)) {
        return dir.mkpath(m_checkpointDirectory);
    }
    return true;
}

bool RollbackManager::backupCurrentState(const QString& checkpointName)
{
    if (!m_stateBackup) {
        return false;
    }

    QString checkpointPath = generateCheckpointPath(checkpointName);
    return m_stateBackup->createBackup(checkpointPath);
}

bool RollbackManager::restoreStateFromCheckpoint(const QString& checkpointName)
{
    if (!m_stateBackup) {
        return false;
    }

    QString checkpointPath = generateCheckpointPath(checkpointName);
    return m_stateBackup->restoreBackup(checkpointPath);
}

QString RollbackManager::generateCheckpointPath(const QString& checkpointName) const
{
    return QDir(m_checkpointDirectory).absoluteFilePath(checkpointName);
}

bool RollbackManager::isValidCheckpointName(const QString& checkpointName) const
{
    if (checkpointName.isEmpty() || checkpointName.length() > 255) {
        return false;
    }

    // 检查是否包含非法字符
    QRegExp validName("^[a-zA-Z0-9_\\-\\.]+$");
    return validName.exactMatch(checkpointName);
}

void RollbackManager::updateRollbackHistory(const QString& operation, const QString& checkpointName, bool success)
{
    QString historyEntry = QString("[%1] %2 %3 - %4")
                          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                          .arg(operation)
                          .arg(checkpointName)
                          .arg(success ? "SUCCESS" : "FAILED");
    
    m_rollbackHistory.prepend(historyEntry);
    
    // 限制历史记录数量
    if (m_rollbackHistory.size() > 1000) {
        m_rollbackHistory = m_rollbackHistory.mid(0, 1000);
    }
}

void RollbackManager::loadCheckpointMetadata()
{
    QString metadataFile = QDir(m_checkpointDirectory).absoluteFilePath("metadata.json");
    QFile file(metadataFile);
    
    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open metadata file:" << metadataFile;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    
    QJsonArray checkpoints = root["checkpoints"].toArray();
    for (const QJsonValue& value : checkpoints) {
        QJsonObject obj = value.toObject();
        
        CheckpointInfo info;
        info.name = obj["name"].toString();
        info.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        info.description = obj["description"].toString();
        info.size = obj["size"].toInt();
        
        QJsonObject metadata = obj["metadata"].toObject();
        for (auto it = metadata.begin(); it != metadata.end(); ++it) {
            info.metadata[it.key()] = it.value().toVariant();
        }
        
        m_checkpoints[info.name] = info;
    }
    
    QJsonArray history = root["history"].toArray();
    for (const QJsonValue& value : history) {
        m_rollbackHistory.append(value.toString());
    }
}

void RollbackManager::saveCheckpointMetadata()
{
    QString metadataFile = QDir(m_checkpointDirectory).absoluteFilePath("metadata.json");
    QFile file(metadataFile);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open metadata file for writing:" << metadataFile;
        return;
    }

    QJsonObject root;
    QJsonArray checkpoints;
    
    for (const auto& info : m_checkpoints) {
        QJsonObject obj;
        obj["name"] = info.name;
        obj["timestamp"] = info.timestamp.toString(Qt::ISODate);
        obj["description"] = info.description;
        obj["size"] = static_cast<qint64>(info.size);
        
        QJsonObject metadata;
        for (auto it = info.metadata.begin(); it != info.metadata.end(); ++it) {
            metadata[it.key()] = QJsonValue::fromVariant(it.value());
        }
        obj["metadata"] = metadata;
        
        checkpoints.append(obj);
    }
    
    root["checkpoints"] = checkpoints;
    
    QJsonArray history;
    for (const QString& entry : m_rollbackHistory) {
        history.append(entry);
    }
    root["history"] = history;
    
    QJsonDocument doc(root);
    file.write(doc.toJson());
}