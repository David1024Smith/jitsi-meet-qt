#ifndef IROLLBACKMANAGER_H
#define IROLLBACKMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>

/**
 * @brief 回滚管理器接口
 * 
 * 定义了回滚管理器的标准接口，用于管理系统状态检查点和回滚操作。
 */
class IRollbackManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 回滚状态枚举
     */
    enum RollbackStatus {
        Idle,               ///< 空闲
        CreatingCheckpoint, ///< 创建检查点中
        RollingBack,        ///< 回滚中
        Completed,          ///< 完成
        Failed              ///< 失败
    };
    Q_ENUM(RollbackStatus)

    /**
     * @brief 检查点信息结构
     */
    struct CheckpointInfo {
        QString name;           ///< 检查点名称
        QDateTime timestamp;    ///< 创建时间
        QString description;    ///< 描述
        qint64 size;           ///< 大小（字节）
        QVariantMap metadata;   ///< 元数据
    };

    explicit IRollbackManager(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IRollbackManager() = default;

    /**
     * @brief 初始化回滚管理器
     * @return 是否初始化成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 获取当前状态
     * @return 回滚状态
     */
    virtual RollbackStatus status() const = 0;

    /**
     * @brief 创建检查点
     * @param checkpointName 检查点名称
     * @param description 描述信息
     * @return 是否创建成功
     */
    virtual bool createCheckpoint(const QString& checkpointName, 
                                const QString& description = QString()) = 0;

    /**
     * @brief 回滚到指定检查点
     * @param checkpointName 检查点名称
     * @return 是否回滚成功
     */
    virtual bool rollbackToCheckpoint(const QString& checkpointName) = 0;

    /**
     * @brief 获取可用检查点列表
     * @return 检查点名称列表
     */
    virtual QStringList availableCheckpoints() const = 0;

    /**
     * @brief 获取检查点详细信息
     * @param checkpointName 检查点名称
     * @return 检查点信息
     */
    virtual CheckpointInfo getCheckpointInfo(const QString& checkpointName) const = 0;

    /**
     * @brief 删除检查点
     * @param checkpointName 检查点名称
     * @return 是否删除成功
     */
    virtual bool deleteCheckpoint(const QString& checkpointName) = 0;

    /**
     * @brief 清理过期检查点
     * @param daysToKeep 保留天数
     * @return 清理的检查点数量
     */
    virtual int cleanupExpiredCheckpoints(int daysToKeep = 30) = 0;

    /**
     * @brief 验证检查点完整性
     * @param checkpointName 检查点名称
     * @return 是否完整
     */
    virtual bool validateCheckpoint(const QString& checkpointName) = 0;

    /**
     * @brief 获取回滚历史
     * @return 回滚历史记录
     */
    virtual QStringList getRollbackHistory() const = 0;

    /**
     * @brief 设置自动清理
     * @param enabled 是否启用
     * @param intervalDays 清理间隔（天）
     */
    virtual void setAutoCleanup(bool enabled, int intervalDays = 7) = 0;

signals:
    /**
     * @brief 状态变化信号
     * @param status 新状态
     */
    void statusChanged(RollbackStatus status);

    /**
     * @brief 检查点创建完成信号
     * @param checkpointName 检查点名称
     * @param success 是否成功
     */
    void checkpointCreated(const QString& checkpointName, bool success);

    /**
     * @brief 回滚完成信号
     * @param checkpointName 检查点名称
     * @param success 是否成功
     */
    void rollbackCompleted(const QString& checkpointName, bool success);

    /**
     * @brief 进度更新信号
     * @param operation 操作类型
     * @param progress 进度百分比 (0-100)
     */
    void progressUpdated(const QString& operation, int progress);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);
};

// 注册元类型以支持信号槽
Q_DECLARE_METATYPE(IRollbackManager::CheckpointInfo)

#endif // IROLLBACKMANAGER_H