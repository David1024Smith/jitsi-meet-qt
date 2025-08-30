#ifndef ICONFIGVALIDATOR_H
#define ICONFIGVALIDATOR_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>
#include <QJsonSchema>

/**
 * @brief 配置验证器接口
 * 
 * 定义了配置验证的核心接口，支持多种验证规则、JSON模式验证和自定义验证器。
 * 提供了完整的配置验证框架，确保配置数据的有效性和一致性。
 */
class IConfigValidator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 验证规则类型枚举
     */
    enum ValidationRule {
        Required,       ///< 必需字段
        Range,          ///< 数值范围
        MinLength,      ///< 最小长度
        MaxLength,      ///< 最大长度
        Pattern,        ///< 正则表达式模式
        Enum,           ///< 枚举值
        Type,           ///< 数据类型
        Custom          ///< 自定义验证
    };
    Q_ENUM(ValidationRule)

    /**
     * @brief 验证严重程度
     */
    enum ValidationSeverity {
        Info,       ///< 信息
        Warning,    ///< 警告
        Error,      ///< 错误
        Critical    ///< 严重错误
    };
    Q_ENUM(ValidationSeverity)

    /**
     * @brief 验证结果结构
     */
    struct ValidationResult {
        bool isValid;                   ///< 是否有效
        QString key;                    ///< 配置键
        QString message;                ///< 验证消息
        ValidationSeverity severity;    ///< 严重程度
        QVariant actualValue;           ///< 实际值
        QVariant expectedValue;         ///< 期望值

        ValidationResult() : isValid(true), severity(Info) {}
        ValidationResult(bool valid, const QString& k, const QString& msg, 
                        ValidationSeverity sev = Error, const QVariant& actual = QVariant(), 
                        const QVariant& expected = QVariant())
            : isValid(valid), key(k), message(msg), severity(sev), 
              actualValue(actual), expectedValue(expected) {}
    };

    /**
     * @brief 自定义验证器函数类型
     */
    using CustomValidatorFunction = std::function<ValidationResult(const QString&, const QVariant&)>;

    virtual ~IConfigValidator() = default;

    /**
     * @brief 初始化验证器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 添加验证规则
     * @param key 配置键
     * @param rule 验证规则
     * @param parameters 规则参数
     * @param severity 严重程度
     */
    virtual void addRule(const QString& key, ValidationRule rule, 
                        const QVariantList& parameters = QVariantList(), 
                        ValidationSeverity severity = Error) = 0;

    /**
     * @brief 添加自定义验证器
     * @param key 配置键
     * @param validator 自定义验证函数
     * @param severity 严重程度
     */
    virtual void addCustomValidator(const QString& key, CustomValidatorFunction validator, 
                                   ValidationSeverity severity = Error) = 0;

    /**
     * @brief 移除验证规则
     * @param key 配置键
     * @param rule 验证规则（可选，不指定则移除所有规则）
     */
    virtual void removeRule(const QString& key, ValidationRule rule = ValidationRule::Custom) = 0;

    /**
     * @brief 验证单个配置项
     * @param key 配置键
     * @param value 配置值
     * @return 验证结果
     */
    virtual ValidationResult validateValue(const QString& key, const QVariant& value) const = 0;

    /**
     * @brief 验证配置映射
     * @param config 配置映射
     * @return 验证结果列表
     */
    virtual QList<ValidationResult> validateConfig(const QVariantMap& config) const = 0;

    /**
     * @brief 验证JSON对象
     * @param json JSON对象
     * @return 验证结果列表
     */
    virtual QList<ValidationResult> validateJson(const QJsonObject& json) const = 0;

    /**
     * @brief 设置JSON模式
     * @param schema JSON模式对象
     * @return 设置是否成功
     */
    virtual bool setJsonSchema(const QJsonObject& schema) = 0;

    /**
     * @brief 从文件加载JSON模式
     * @param schemaFilePath 模式文件路径
     * @return 加载是否成功
     */
    virtual bool loadJsonSchema(const QString& schemaFilePath) = 0;

    /**
     * @brief 使用JSON模式验证
     * @param json 待验证的JSON对象
     * @return 验证结果列表
     */
    virtual QList<ValidationResult> validateWithSchema(const QJsonObject& json) const = 0;

    /**
     * @brief 获取所有验证规则
     * @return 规则映射（键 -> 规则列表）
     */
    virtual QMap<QString, QList<ValidationRule>> getAllRules() const = 0;

    /**
     * @brief 检查是否有验证规则
     * @param key 配置键
     * @return 是否有规则
     */
    virtual bool hasRules(const QString& key) const = 0;

    /**
     * @brief 清除所有验证规则
     */
    virtual void clearRules() = 0;

    /**
     * @brief 设置验证模式
     * @param strict 是否严格模式（严格模式下警告也会导致验证失败）
     */
    virtual void setStrictMode(bool strict) = 0;

    /**
     * @brief 获取验证模式
     * @return 是否为严格模式
     */
    virtual bool isStrictMode() const = 0;

    /**
     * @brief 设置默认严重程度
     * @param severity 默认严重程度
     */
    virtual void setDefaultSeverity(ValidationSeverity severity) = 0;

    /**
     * @brief 获取默认严重程度
     * @return 默认严重程度
     */
    virtual ValidationSeverity defaultSeverity() const = 0;

    /**
     * @brief 导出验证规则到JSON
     * @return JSON对象
     */
    virtual QJsonObject exportRulesToJson() const = 0;

    /**
     * @brief 从JSON导入验证规则
     * @param json JSON对象
     * @return 导入是否成功
     */
    virtual bool importRulesFromJson(const QJsonObject& json) = 0;

    /**
     * @brief 创建预定义规则集
     * @param ruleSetName 规则集名称（如 "audio", "video", "network"）
     */
    virtual void createPredefinedRuleSet(const QString& ruleSetName) = 0;

signals:
    /**
     * @brief 验证完成信号
     * @param results 验证结果列表
     */
    void validationCompleted(const QList<ValidationResult>& results);

    /**
     * @brief 验证失败信号
     * @param key 配置键
     * @param result 验证结果
     */
    void validationFailed(const QString& key, const ValidationResult& result);

    /**
     * @brief 规则添加信号
     * @param key 配置键
     * @param rule 验证规则
     */
    void ruleAdded(const QString& key, ValidationRule rule);

    /**
     * @brief 规则移除信号
     * @param key 配置键
     * @param rule 验证规则
     */
    void ruleRemoved(const QString& key, ValidationRule rule);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);
};

// 便利宏定义
#define VALIDATION_RESULT_SUCCESS ValidationResult(true, QString(), QString(), IConfigValidator::Info)
#define VALIDATION_RESULT_ERROR(key, msg) ValidationResult(false, key, msg, IConfigValidator::Error)
#define VALIDATION_RESULT_WARNING(key, msg) ValidationResult(true, key, msg, IConfigValidator::Warning)

#endif // ICONFIGVALIDATOR_H