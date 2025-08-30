#ifndef LEGACYCOMPATIBILITYADAPTER_H
#define LEGACYCOMPATIBILITYADAPTER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QStringList>
#include <QHash>
#include <QMutex>

// 前向声明
class MediaManager;
class ChatManager;
class ScreenShareManager;
class ConferenceManager;
class ICompatibilityAdapter;
class RollbackManager;
class CompatibilityValidator;

/**
 * @brief 遗留兼容性适配器
 * 
 * 提供旧API到新模块的映射机制，确保在模块化过程中不会破坏现有功能。
 */
class LegacyCompatibilityAdapter : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 适配器类型枚举
     */
    enum AdapterType {
        MediaAdapter,
        ChatAdapter,
        ScreenShareAdapter,
        ConferenceAdapter,
        AllAdapters
    };
    Q_ENUM(AdapterType)

    /**
     * @brief 迁移状态枚举
     */
    enum MigrationStatus {
        NotStarted,
        InProgress,
        Completed,
        Failed,
        RolledBack
    };
    Q_ENUM(MigrationStatus)

    explicit LegacyCompatibilityAdapter(QObject *parent = nullptr);
    ~LegacyCompatibilityAdapter();

    // 静态工厂方法
    static MediaManager* createLegacyMediaManager();
    static ChatManager* createLegacyChatManager();
    static ScreenShareManager* createLegacyScreenShareManager();
    static ConferenceManager* createLegacyConferenceManager();

    // 功能验证
    static bool validateFunctionality(const QString& moduleName);
    static QStringList runCompatibilityTests();
    static QVariantMap getValidationReport(const QString& moduleName);

    // 实例方法
    bool initialize();
    bool isInitialized() const;

    // 适配器管理
    bool registerAdapter(AdapterType type, ICompatibilityAdapter* adapter);
    ICompatibilityAdapter* getAdapter(AdapterType type) const;
    QList<AdapterType> getRegisteredAdapters() const;

    // 迁移管理
    bool startMigration(AdapterType type);
    bool completeMigration(AdapterType type);
    bool rollbackMigration(AdapterType type);
    MigrationStatus getMigrationStatus(AdapterType type) const;

    // 配置管理
    void setGlobalConfig(const QVariantMap& config);
    QVariantMap getGlobalConfig() const;
    void setAdapterConfig(AdapterType type, const QVariantMap& config);
    QVariantMap getAdapterConfig(AdapterType type) const;

private slots:
    void onAdapterStatusChanged();
    void onValidationCompleted(const QStringList& results);
    void onRollbackCompleted(const QString& checkpointName, bool success);

signals:
    void migrationStarted(AdapterType type);
    void migrationCompleted(AdapterType type, bool success);
    void migrationProgress(AdapterType type, int percentage);
    void validationFailed(const QString& moduleName, const QString& error);
    void errorOccurred(const QString& error);

private:
    void setupDefaultConfig();
    bool validateAdapterType(AdapterType type) const;
    QString adapterTypeToString(AdapterType type) const;

    static LegacyCompatibilityAdapter* s_instance;
    static QMutex s_mutex;

    bool m_initialized;
    QHash<AdapterType, ICompatibilityAdapter*> m_adapters;
    QHash<AdapterType, MigrationStatus> m_migrationStatus;
    QVariantMap m_globalConfig;
    QHash<AdapterType, QVariantMap> m_adapterConfigs;
    
    RollbackManager* m_rollbackManager;
    CompatibilityValidator* m_validator;
    
    mutable QMutex m_mutex;
};

#endif // LEGACYCOMPATIBILITYADAPTER_H