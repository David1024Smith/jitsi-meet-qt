#ifndef MODULECONFIG_H
#define MODULECONFIG_H

#include "interfaces/IModuleConfig.h"
#include <QSettings>

/**
 * @brief 基础模块配置实现
 * 
 * 提供IModuleConfig接口的基础实现
 */
class ModuleConfig : public IModuleConfig
{
    Q_OBJECT

public:
    explicit ModuleConfig(const QString& moduleName, QObject* parent = nullptr);
    ~ModuleConfig();

    // IModuleConfig接口实现
    QString moduleName() const override;
    QString moduleVersion() const override;
    QString moduleDescription() const override;
    bool isEnabled() const override;
    void setEnabled(bool enabled) override;

    QVariantMap toVariantMap() const override;
    void fromVariantMap(const QVariantMap& map) override;
    QJsonObject toJsonObject() const override;
    void fromJsonObject(const QJsonObject& json) override;

    bool validate() const override;
    QStringList getValidationErrors() const override;
    bool hasRequiredFields() const override;

    bool save() override;
    bool load() override;
    bool reset() override;
    bool backup() override;
    bool restore() override;

    ConfigScope getScope() const override;
    void setScope(ConfigScope scope) override;

    void notifyConfigChanged() override;

    // 扩展功能
    void setModuleVersion(const QString& version);
    void setModuleDescription(const QString& description);
    
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& key, const QVariant& value);
    
    QStringList getKeys() const;
    void removeKey(const QString& key);
    void clear();

private:
    QString m_moduleName;
    QString m_moduleVersion;
    QString m_moduleDescription;
    bool m_enabled;
    ConfigScope m_scope;
    
    QVariantMap m_configData;
    QSettings* m_settings;
    QString m_configFilePath;
    
    void initializeSettings();
    QString getConfigFilePath() const;
};

#endif // MODULECONFIG_H