#ifndef CHECKPOINTMANAGER_H
#define CHECKPOINTMANAGER_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QMutex>

/**
 * @brief 检查点管理器
 * 
 * 负责创建、管理和验证系统检查点。
 */
class CheckpointManager : public QObject
{
    Q_OBJECT

public:
    explicit CheckpointManager(QObject *parent = nullptr);
    ~CheckpointManager();

    bool initialize();
    
    void setCheckpointDirectory(const QString& directory);
    QString getCheckpointDirectory() const;
    
    bool createCheckpoint(const QString& checkpointName);
    bool deleteCheckpoint(const QString& checkpointName);
    bool validateCheckpoint(const QString& checkpointName);
    
    QStringList listCheckpoints() const;
    qint64 getCheckpointSize(const QString& checkpointName) const;

signals:
    void checkpointCreated(const QString& checkpointName, bool success);
    void checkpointDeleted(const QString& checkpointName, bool success);
    void progressUpdated(int percentage);

private:
    bool createCheckpointStructure(const QString& checkpointPath);
    bool copySystemFiles(const QString& checkpointPath);
    bool createManifest(const QString& checkpointPath);
    
    QString m_checkpointDirectory;
    bool m_initialized;
    mutable QMutex m_mutex;
};

#endif // CHECKPOINTMANAGER_H