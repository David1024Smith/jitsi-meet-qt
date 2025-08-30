#ifndef COMPATIBILITYMODULE_H
#define COMPATIBILITYMODULE_H

#include <QObject>
#include <QString>
#include <QVariantMap>

class LegacyCompatibilityAdapter;
class RollbackManager;
class CompatibilityValidator;
class CompatibilityConfig;

/**
 * @brief 兼容性模块
 * 
 * 兼容性适配器系统的主入口点，提供统一的接口来管理所有兼容性功能。
 */
class CompatibilityModule : public QObject
{
    Q_OBJECT

public:
    explicit CompatibilityModule(QObject *parent = nullptr);
    ~CompatibilityModule();

    bool initialize();
    bool isInitialized() const;
    
    // 获取子组件
    LegacyCompatibilityAdapter* getAdapter() const;
    RollbackManager* getRollbackManager() const;
    CompatibilityValidator* getValidator() const;
    CompatibilityConfig* getConfig() const;
    
    // 模块信息
    QString getModuleName() const;
    QString getModuleVersion() const;
    QVariantMap getModuleInfo() const;

signals:
    void moduleInitialized();
    void moduleError(const QString& error);

private:
    bool m_initialized;
    
    LegacyCompatibilityAdapter* m_adapter;
    RollbackManager* m_rollbackManager;
    CompatibilityValidator* m_validator;
    CompatibilityConfig* m_config;
};

#endif // COMPATIBILITYMODULE_H