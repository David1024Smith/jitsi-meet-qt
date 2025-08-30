#ifndef ROLLBACKMANAGER_H
#define ROLLBACKMANAGER_H

#include "IRollbackManager.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <QHash>
#include <QMutex>
#include <QTimer>

class CheckpointManager;
class StateBackup;

/**
 * @brief 回滚管理器实现
 * 
 * 实现了回滚管理器接口，提供系统状态检查点和回滚功能。
 */
class RollbackManager : public IRollbackManager
{
    Q_OBJECT

public:
    explicit RollbackManager(QObject *parent = nullptr);
    ~RollbackManager();

    // IRollbackManager 接口实现
    bool initialize() override;
    RollbackStatus status() const override;
    
    bool createCheckpoint(const QString& checkpointName, 
                         const QString& description = QString()) override;
    bool rollbackToCheckpoint(const QString& checkpointName) override;
    
    QStringList availableCheckpoints() const override;
    CheckpointInfo getCheckpointInfo(const QString& checkpointName) const override;
    bool deleteCheckpoint(const QString& checkpointName) override;
    
    int cleanupExpiredCheckpoints(int daysToKeep = 30) override;
    bool validateCheckpoint(const QString& checkpointName) override;
    QStringList getRollbackHistory() const override;
    
    void setAutoCleanup(bool enabled, int intervalDays = 7) override;

    // 扩展功能
    QString getCheckpointDirectory() const;
    void setCheckpointDirectory(const QString& directory);
    
    qint64 getTotalCheckpointSize() const;
    int getMaxCheckpoints() const;
    void setMaxCheckpoints(int maxCheckpoints);

private slots:
    void performAutoCleanup();
    void onCheckpointCreated(const QString& checkpointName, bool success);
    void onRollbackProgress(int percentage);

private:
    bool createCheckpointDirectory();
    bool backupCurrentState(const QString& checkpointName);
    bool restoreStateFromCheckpoint(const QString& checkpointName);
    
    QString generateCheckpointPath(const QString& checkpointName) const;
    bool isValidCheckpointName(const QString& checkpointName) const;
    
    void updateRollbackHistory(const QString& operation, const QString& checkpointName, bool success);
    void loadCheckpointMetadata();
    void saveCheckpointMetadata();

    bool m_initialized;
    RollbackStatus m_status;
    QString m_checkpointDirectory;
    int m_maxCheckpoints;
    
    QHash<QString, CheckpointInfo> m_checkpoints;
    QStringList m_rollbackHistory;
    
    CheckpointManager* m_checkpointManager;
    StateBackup* m_stateBackup;
    
    QTimer* m_autoCleanupTimer;
    bool m_autoCleanupEnabled;
    int m_autoCleanupInterval;
    
    mutable QMutex m_mutex;
};

#endif // ROLLBACKMANAGER_H