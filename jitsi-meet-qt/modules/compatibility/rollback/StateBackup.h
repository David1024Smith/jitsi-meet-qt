#ifndef STATEBACKUP_H
#define STATEBACKUP_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QMutex>

/**
 * @brief 状态备份管理器
 * 
 * 负责备份和恢复应用程序状态。
 */
class StateBackup : public QObject
{
    Q_OBJECT

public:
    explicit StateBackup(QObject *parent = nullptr);
    ~StateBackup();

    bool initialize();
    
    bool createBackup(const QString& backupPath);
    bool restoreBackup(const QString& backupPath);
    
    void setBackupItems(const QStringList& items);
    QStringList getBackupItems() const;
    
    bool validateBackup(const QString& backupPath);

signals:
    void backupCreated(const QString& backupPath, bool success);
    void backupRestored(const QString& backupPath, bool success);
    void progressUpdated(int percentage);

private:
    bool backupApplicationState(const QString& backupPath);
    bool backupModuleStates(const QString& backupPath);
    bool backupUserSettings(const QString& backupPath);
    bool backupDatabaseState(const QString& backupPath);
    
    bool restoreApplicationState(const QString& backupPath);
    bool restoreModuleStates(const QString& backupPath);
    bool restoreUserSettings(const QString& backupPath);
    bool restoreDatabaseState(const QString& backupPath);
    
    QVariantMap getCurrentApplicationState();
    QVariantMap getCurrentModuleStates();
    QVariantMap getCurrentUserSettings();
    
    bool setApplicationState(const QVariantMap& state);
    bool setModuleStates(const QVariantMap& states);
    bool setUserSettings(const QVariantMap& settings);

    bool m_initialized;
    QStringList m_backupItems;
    mutable QMutex m_mutex;
};

#endif // STATEBACKUP_H