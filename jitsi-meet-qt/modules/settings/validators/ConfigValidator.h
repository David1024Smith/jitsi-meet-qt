#ifndef CONFIGVALIDATOR_H
#define CONFIGVALIDATOR_H

#include "interfaces/IConfigValidator.h"
#include <QRegularExpression>
#include <QJsonSchema>
#include <QMutex>
#include <memory>

/**
 * @brief 配置验证器实现类
 * 
 * IConfigValidator 接口的具体实现，提供完整的配置验证功能。
 * 支持多种验证规则、自定义验证器和JSON模式验证。
 */
class ConfigValidator : public IConfigValidator
{
    Q_OBJECT

public:
    /**
     * @brief 验证规则结构
     */
    struct ValidationRuleInfo {
        ValidationRule rule;            ///< 规则类型
        QVariantList parameters;        ///< 规则参数
        ValidationSeverity severity;    ///< 严重程度
        QString description;            ///< 规则描述
        bool enabled;                   ///< 是否启用
        
        ValidationRuleInfo() 
            : rule(Required), severity(Error), enabled(true) {}
        ValidationRuleInfo(ValidationRule r, const QVariantList& params, 
                          ValidationSeverity sev = Error, const QString& desc = QString())
            : rule(r), parameters(params), severity(sev), description(desc), enabled(true) {}
    };

    /**
     * @brief 验证上下文结构
     */
    struct ValidationContext {
        QString currentKey;             ///< 当前验证的键
        QVariantMap fullConfig;         ///< 完整配置
        QStringList validationPath;     ///< 验证路径
        int depth;                      ///< 验证深度
        
        ValidationContext() : depth(0) {}
    };

    explicit ConfigValidator(QObject* parent = nullptr);
    ~ConfigValidator();

    /**
     * @brief 获取单例实例
     * @return 验证器实例
     */
    static ConfigValidator* instance();

    // IConfigValidator 接口实现
    bool initialize() override;
    
    void addRule(const QString& key, ValidationRule rule, 
                const QVariantList& parameters = QVariantList(), 
                ValidationSeverity severity = Error) override;
    
    void addCustomValidator(const QString& key, CustomValidatorFunction validator, 
                           ValidationSeverity severity = Error) override;
    
    void removeRule(const QString& key, ValidationRule rule = ValidationRule::Custom) override;
    
    ValidationResult validateValue(const QString& key, const QVariant& value) const override;
    QList<ValidationResult> validateConfig(const QVariantMap& config) const override;
    QList<ValidationResult> validateJson(const QJsonObject& json) const override;
    
    bool setJsonSchema(const QJsonObject& schema) override;
    bool loadJsonSchema(const QString& schemaFilePath) override;
    QList<ValidationResult> validateWithSchema(const QJsonObject& json) const override;
    
    QMap<QString, QList<ValidationRule>> getAllRules() const override;
    bool hasRules(const QString& key) const override;
    void clearRules() override;
    
    void setStrictMode(bool strict) override;
    bool isStrictMode() const override;
    void setDefaultSeverity(ValidationSeverity severity) override;
    ValidationSeverity defaultSeverity() const override;
    
    QJsonObject exportRulesToJson() const override;
    bool importRulesFromJson(const QJsonObject& json) override;
    void createPredefinedRuleSet(const QString& ruleSetName) override;

    // 扩展功能
    /**
     * @brief 添加规则（使用结构体）
     * @param key 配置键
     * @param ruleInfo 规则信息
     */
    void addRule(const QString& key, const ValidationRuleInfo& ruleInfo);

    /**
     * @brief 获取规则信息
     * @param key 配置键
     * @return 规则信息列表
     */
    QList<ValidationRuleInfo> getRuleInfo(const QString& key) const;

    /**
     * @brief 启用/禁用规则
     * @param key 配置键
     * @param rule 规则类型
     * @param enabled 是否启用
     */
    void setRuleEnabled(const QString& key, ValidationRule rule, bool enabled);

    /**
     * @brief 检查规则是否启用
     * @param key 配置键
     * @param rule 规则类型
     * @return 是否启用
     */
    bool isRuleEnabled(const QString& key, ValidationRule rule) const;

    /**
     * @brief 设置规则描述
     * @param key 配置键
     * @param rule 规则类型
     * @param description 描述
     */
    void setRuleDescription(const QString& key, ValidationRule rule, const QString& description);

    /**
     * @brief 获取规则描述
     * @param key 配置键
     * @param rule 规则类型
     * @return 描述
     */
    QString getRuleDescription(const QString& key, ValidationRule rule) const;

    /**
     * @brief 设置验证上下文
     * @param context 验证上下文
     */
    void setValidationContext(const ValidationContext& context);

    /**
     * @brief 获取验证上下文
     * @return 验证上下文
     */
    ValidationContext validationContext() const;

    /**
     * @brief 添加依赖规则
     * @param key 配置键
     * @param dependentKey 依赖键
     * @param condition 依赖条件
     */
    void addDependencyRule(const QString& key, const QString& dependentKey, const QVariant& condition);

    /**
     * @brief 移除依赖规则
     * @param key 配置键
     * @param dependentKey 依赖键
     */
    void removeDependencyRule(const QString& key, const QString& dependentKey);

    /**
     * @brief 获取依赖规则
     * @param key 配置键
     * @return 依赖规则映射
     */
    QMap<QString, QVariant> getDependencyRules(const QString& key) const;

    /**
     * @brief 设置验证超时
     * @param timeout 超时时间（毫秒）
     */
    void setValidationTimeout(int timeout);

    /**
     * @brief 获取验证超时
     * @return 超时时间（毫秒）
     */
    int validationTimeout() const;

    /**
     * @brief 启用/禁用并行验证
     * @param enabled 是否启用
     */
    void setParallelValidation(bool enabled);

    /**
     * @brief 检查是否启用并行验证
     * @return 是否启用
     */
    bool isParallelValidationEnabled() const;

    /**
     * @brief 设置最大验证深度
     * @param depth 最大深度
     */
    void setMaxValidationDepth(int depth);

    /**
     * @brief 获取最大验证深度
     * @return 最大深度
     */
    int maxValidationDepth() const;

    /**
     * @brief 添加全局验证器
     * @param validator 全局验证函数
     * @param severity 严重程度
     */
    void addGlobalValidator(CustomValidatorFunction validator, ValidationSeverity severity = Error);

    /**
     * @brief 移除所有全局验证器
     */
    void clearGlobalValidators();

    /**
     * @brief 获取验证统计信息
     * @return 统计信息
     */
    QVariantMap validationStatistics() const;

    /**
     * @brief 重置验证统计
     */
    void resetStatistics();

    // 预定义规则集
    /**
     * @brief 创建音频配置规则集
     */
    void createAudioRuleSet();

    /**
     * @brief 创建视频配置规则集
     */
    void createVideoRuleSet();

    /**
     * @brief 创建网络配置规则集
     */
    void createNetworkRuleSet();

    /**
     * @brief 创建UI配置规则集
     */
    void createUIRuleSet();

    /**
     * @brief 创建性能配置规则集
     */
    void createPerformanceRuleSet();

    /**
     * @brief 创建安全配置规则集
     */
    void createSecurityRuleSet();

public slots:
    /**
     * @brief 异步验证配置
     * @param config 配置映射
     */
    void validateConfigAsync(const QVariantMap& config);

    /**
     * @brief 异步验证JSON
     * @param json JSON对象
     */
    void validateJsonAsync(const QJsonObject& json);

    /**
     * @brief 重新加载规则
     */
    void reloadRules();

    /**
     * @brief 优化规则
     */
    void optimizeRules();

private slots:
    void onAsyncValidationFinished();

private:
    // 验证规则实现
    ValidationResult validateRequired(const QString& key, const QVariant& value) const;
    ValidationResult validateRange(const QString& key, const QVariant& value, const QVariantList& parameters) const;
    ValidationResult validateMinLength(const QString& key, const QVariant& value, const QVariantList& parameters) const;
    ValidationResult validateMaxLength(const QString& key, const QVariant& value, const QVariantList& parameters) const;
    ValidationResult validatePattern(const QString& key, const QVariant& value, const QVariantList& parameters) const;
    ValidationResult validateEnum(const QString& key, const QVariant& value, const QVariantList& parameters) const;
    ValidationResult validateType(const QString& key, const QVariant& value, const QVariantList& parameters) const;

    // 辅助方法
    ValidationResult executeRule(const QString& key, const QVariant& value, const ValidationRuleInfo& ruleInfo) const;
    bool checkDependencies(const QString& key, const QVariantMap& config) const;
    QVariant convertValue(const QVariant& value, QMetaType::Type targetType) const;
    QString formatValidationMessage(const ValidationResult& result) const;
    void updateStatistics(const QString& operation, bool success = true);
    
    // 规则管理
    void addBuiltinRules();
    void loadRulesFromFile(const QString& filePath);
    void saveRulesToFile(const QString& filePath) const;
    
    // JSON Schema 相关
    bool validateJsonSchema(const QJsonObject& schema) const;
    QList<ValidationResult> processSchemaValidation(const QJsonObject& json, const QJsonObject& schema) const;
    
    // 类型转换
    QString ruleToString(ValidationRule rule) const;
    ValidationRule stringToRule(const QString& str) const;
    QString severityToString(ValidationSeverity severity) const;
    ValidationSeverity stringToSeverity(const QString& str) const;

    class Private;
    std::unique_ptr<Private> d;
};

#endif // CONFIGVALIDATOR_H