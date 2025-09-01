#ifndef ICONFIGVALIDATOR_H
#define ICONFIGVALIDATOR_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <functional>

/**
 * @brief 配置验证器接口
 * 
 * 提供配置数据验证的统一接口
 */
class IConfigValidator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 验证结果枚举
     */
    enum ValidationResultEnum {
        Valid,          ///< 验证通过
        Invalid,        ///< 验证失败
        ValidWithWarning ///< 验证通过但有警告
    };
    Q_ENUM(ValidationResultEnum)
    
    /**
     * @brief 验证结果结构体
     */
    struct ValidationResult {
        bool isValid;       ///< 是否有效
        QString key;        ///< 验证的键
        QVariant value;     ///< 验证的值
        int severityLevel;  ///< 严重程度级别
        QString message;    ///< 验证消息
        
        ValidationResult() : isValid(true), severityLevel(0) {} // 0 对应 Info
    };

    /**
     * @brief 验证错误类型
     */
    enum ErrorType {
        TypeError,      ///< 类型错误
        RangeError,     ///< 范围错误
        FormatError,    ///< 格式错误
        RequiredError,  ///< 必需字段缺失
        CustomError     ///< 自定义错误
    };
    Q_ENUM(ErrorType)

    /**
     * @brief 验证规则枚举
     */
    enum ValidationRule {
        Required,       ///< 必需字段
        Range,          ///< 范围验证
        MinLength,      ///< 最小长度
        MaxLength,      ///< 最大长度
        Pattern,        ///< 正则表达式模式
        Enum,           ///< 枚举值
        Type,           ///< 类型验证
        Custom          ///< 自定义验证
    };
    Q_ENUM(ValidationRule)

    /**
     * @brief 验证严重程度
     */
    enum ValidationSeverity {
        Info,           ///< 信息
        Warning,        ///< 警告
        Error,          ///< 错误
        Critical        ///< 严重错误
    };
    Q_ENUM(ValidationSeverity)
    
    // 定义默认错误级别常量
    static const ValidationSeverity ErrorLevel = Error;

    /**
     * @brief 自定义验证器函数类型
     */
    using CustomValidatorFunction = std::function<ValidationResult(const QString&, const QVariant&)>;

    /**
     * @brief 验证错误信息结构
     */
    struct ValidationError {
        QString field;      ///< 字段名
        ErrorType type;     ///< 错误类型
        QString message;    ///< 错误消息
        QVariant expected;  ///< 期望值
        QVariant actual;    ///< 实际值
    };

    explicit IConfigValidator(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IConfigValidator() = default;

    /**
     * @brief 验证单个配置值
     * @param key 配置键
     * @param value 配置值
     * @return 验证结果
     */
    virtual ValidationResult validateValue(const QString& key, const QVariant& value) const = 0;

    /**
     * @brief 验证配置对象
     * @param config 配置对象
     * @return 验证结果列表
     */
    virtual QList<ValidationResult> validateConfig(const QVariantMap& config) const = 0;

    /**
     * @brief 验证JSON配置
     * @param json JSON对象
     * @return 验证结果列表
     */
    virtual QList<ValidationResult> validateJson(const QJsonObject& json) const = 0;

    /**
     * @brief 获取最后的验证错误
     * @return 错误列表
     */
    virtual QList<ValidationError> getLastErrors() const = 0;

    /**
     * @brief 获取验证警告
     * @return 警告列表
     */
    virtual QStringList getWarnings() const = 0;

    /**
     * @brief 设置JSON模式
     * @param schema JSON模式
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
     * @brief 使用模式验证JSON
     * @param json JSON对象
     * @return 验证结果列表
     */
    virtual QList<ValidationResult> validateWithSchema(const QJsonObject& json) const = 0;

    /**
     * @brief 获取所有验证规则
     * @return 规则映射
     */
    virtual QMap<QString, QList<ValidationRule>> getAllRules() const = 0;

    /**
     * @brief 检查是否有规则
     * @param key 配置键
     * @return 是否有规则
     */
    virtual bool hasRules(const QString& key) const = 0;

    /**
     * @brief 清除所有规则
     */
    virtual void clearRules() = 0;

    /**
     * @brief 设置严格模式
     * @param strict 是否严格
     */
    virtual void setStrictMode(bool strict) = 0;

    /**
     * @brief 检查是否严格模式
     * @return 是否严格模式
     */
    virtual bool isStrictMode() const = 0;

    /**
     * @brief 设置默认严重程度
     * @param severity 严重程度
     */
    virtual void setDefaultSeverity(ValidationSeverity severity) = 0;

    /**
     * @brief 获取默认严重程度
     * @return 严重程度
     */
    virtual ValidationSeverity defaultSeverity() const = 0;

    /**
     * @brief 导出规则到JSON
     * @return JSON对象
     */
    virtual QJsonObject exportRulesToJson() const = 0;

    /**
     * @brief 从JSON导入规则
     * @param json JSON对象
     * @return 导入是否成功
     */
    virtual bool importRulesFromJson(const QJsonObject& json) = 0;

    /**
     * @brief 创建预定义规则集
     * @param ruleSetName 规则集名称
     */
    virtual void createPredefinedRuleSet(const QString& ruleSetName) = 0;

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
                        ValidationSeverity severity = ErrorLevel) = 0;

    /**
     * @brief 添加自定义验证器
     * @param key 字段键
     * @param validator 验证函数
     * @param severity 严重程度
     */
    virtual void addCustomValidator(const QString& key, CustomValidatorFunction validator,
                                  ValidationSeverity severity = ErrorLevel) = 0;

    /**
     * @brief 移除验证规则
     * @param key 配置键
     * @param rule 验证规则
     */
    virtual void removeRule(const QString& key, ValidationRule rule = Custom) = 0;

signals:
    /**
     * @brief 验证完成信号
     * @param result 验证结果
     * @param errors 错误列表
     */
    void validationCompleted(ValidationResult result, const QList<ValidationError>& errors);

    /**
     * @brief 验证警告信号
     * @param warnings 警告列表
     */
    void validationWarning(const QStringList& warnings);
};

Q_DECLARE_METATYPE(IConfigValidator::ValidationResultEnum)
Q_DECLARE_METATYPE(IConfigValidator::ValidationResult)
Q_DECLARE_METATYPE(IConfigValidator::ErrorType)
Q_DECLARE_METATYPE(IConfigValidator::ValidationError)

#endif // ICONFIGVALIDATOR_H