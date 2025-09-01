#include "ConfigValidator.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QRegularExpression>
#include <QMetaEnum>
#include <QDateTime>
#include <QUrl>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

// Private implementation class
class ConfigValidator::Private
{
public:
    QMap<QString, QList<ConfigValidator::ValidationRuleInfo>> rules;
    QMap<QString, CustomValidatorFunction> customValidators;
    QMap<QString, QMap<QString, QVariant>> dependencyRules;
    QList<CustomValidatorFunction> globalValidators;
    QJsonObject jsonSchema;
    ValidationContext context;
    
    bool strictMode = false;
    ValidationSeverity defaultSeverity = ErrorLevel;
    int validationTimeout = 5000;
    bool parallelValidation = false;
    int maxValidationDepth = 10;
    
    mutable QMutex mutex;
    QVariantMap statistics;
    QList<ValidationError> lastErrors;
    QStringList warnings;
    
    static ConfigValidator* s_instance;
};

ConfigValidator* ConfigValidator::Private::s_instance = nullptr;

ConfigValidator::ConfigValidator(QObject* parent)
    : IConfigValidator(parent)
    , d(std::make_unique<Private>())
{
    addBuiltinRules();
}

ConfigValidator::~ConfigValidator() = default;

ConfigValidator* ConfigValidator::instance()
{
    if (!Private::s_instance) {
        Private::s_instance = new ConfigValidator();
    }
    return Private::s_instance;
}

bool ConfigValidator::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    // 清理现有规则
    d->rules.clear();
    d->customValidators.clear();
    d->dependencyRules.clear();
    d->globalValidators.clear();
    
    // 添加内置规则
    addBuiltinRules();
    
    // 重置统计
    resetStatistics();
    
    return true;
}

void ConfigValidator::addBuiltinRules()
{
    // 添加一些内置的验证规则
    // 服务器配置规则
    addRule("server.host", Required, QVariantList(), ErrorLevel);
    addRule("server.port", Range, QVariantList() << 1 << 65535, ErrorLevel);
    
    // 用户配置规则
    addRule("user.name", Required, QVariantList(), ErrorLevel);
    addRule("user.name", MinLength, QVariantList() << 1, ErrorLevel);
    addRule("user.name", MaxLength, QVariantList() << 50, Warning);
    
    // 网络配置规则
    addRule("network.timeout", Range, QVariantList() << 1000 << 30000, Warning);
    addRule("network.retries", Range, QVariantList() << 1 << 10, Warning);
}

// 添加validateJsonValue函数的实现
IConfigValidator::ValidationResult ConfigValidator::validateJsonValue(const QString& key, const QJsonValue& value, const QJsonObject& schema) const
{
    ValidationResult result;
    result.isValid = true;
    result.key = key;
    result.value = value.toVariant();
    result.severityLevel = 0; // Info
    result.message = "Validation passed";
    
    // 检查类型
    if (schema.contains("type")) {
        QString expectedType = schema["type"].toString();
        bool typeValid = false;
        
        if (expectedType == "string" && value.isString()) {
            typeValid = true;
        } else if (expectedType == "number" && (value.isDouble() || value.isString())) {
            typeValid = true;
        } else if (expectedType == "integer" && (value.isDouble() || value.isString())) {
            // 检查是否为整数
            bool ok;
            value.toVariant().toInt(&ok);
            typeValid = ok;
        } else if (expectedType == "boolean" && value.isBool()) {
            typeValid = true;
        } else if (expectedType == "object" && value.isObject()) {
            typeValid = true;
        } else if (expectedType == "array" && value.isArray()) {
            typeValid = true;
        } else if (expectedType == "null" && value.isNull()) {
            typeValid = true;
        }
        
        if (!typeValid) {
            result.isValid = false;
            result.severityLevel = 2; // Error
            result.message = QString("Type mismatch for '%1': expected %2").arg(key, expectedType);
            return result;
        }
    }
    
    // 检查必需属性
    if (schema.contains("required") && schema["required"].isBool() && schema["required"].toBool()) {
        if (value.isNull() || (value.isString() && value.toString().isEmpty())) {
            result.isValid = false;
            result.severityLevel = 2; // Error
            result.message = QString("Required property '%1' is missing or empty").arg(key);
            return result;
        }
    }
    
    // 检查最小值
    if (schema.contains("minimum") && (value.isDouble() || value.isString())) {
        double minValue = schema["minimum"].toDouble();
        double actualValue = value.toVariant().toDouble();
        
        if (actualValue < minValue) {
            result.isValid = false;
            result.severityLevel = 2; // Error
            result.message = QString("Value for '%1' is less than minimum: %2 < %3").arg(key).arg(actualValue).arg(minValue);
            return result;
        }
    }
    
    // 检查最大值
    if (schema.contains("maximum") && (value.isDouble() || value.isString())) {
        double maxValue = schema["maximum"].toDouble();
        double actualValue = value.toVariant().toDouble();
        
        if (actualValue > maxValue) {
            result.isValid = false;
            result.severityLevel = 2; // Error
            result.message = QString("Value for '%1' is greater than maximum: %2 > %3").arg(key).arg(actualValue).arg(maxValue);
            return result;
        }
    }
    
    // 检查字符串长度
    if (schema.contains("minLength") && value.isString()) {
        int minLength = schema["minLength"].toInt();
        int actualLength = value.toString().length();
        
        if (actualLength < minLength) {
            result.isValid = false;
            result.severityLevel = 2; // Error
            result.message = QString("String length for '%1' is less than minimum: %2 < %3").arg(key).arg(actualLength).arg(minLength);
            return result;
        }
    }
    
    if (schema.contains("maxLength") && value.isString()) {
        int maxLength = schema["maxLength"].toInt();
        int actualLength = value.toString().length();
        
        if (actualLength > maxLength) {
            result.isValid = false;
            result.severityLevel = 2; // Error
            result.message = QString("String length for '%1' is greater than maximum: %2 > %3").arg(key).arg(actualLength).arg(maxLength);
            return result;
        }
    }
    
    // 检查模式
    if (schema.contains("pattern") && value.isString()) {
        QString pattern = schema["pattern"].toString();
        QRegularExpression regex(pattern);
        
        if (!regex.isValid()) {
            result.isValid = false;
            result.severityLevel = 1; // Warning
            result.message = QString("Invalid regex pattern for '%1': %2").arg(key, regex.errorString());
            return result;
        }
        
        QRegularExpressionMatch match = regex.match(value.toString());
        if (!match.hasMatch()) {
            result.isValid = false;
            result.severityLevel = 2; // Error
            result.message = QString("Value for '%1' does not match pattern: %2").arg(key, pattern);
            return result;
        }
    }
    
    // 检查枚举值
    if (schema.contains("enum") && schema["enum"].isArray()) {
        QJsonArray enumValues = schema["enum"].toArray();
        bool found = false;
        
        for (const QJsonValue& enumValue : enumValues) {
            if (value == enumValue) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            result.isValid = false;
            result.severityLevel = 2; // Error
            result.message = QString("Value for '%1' is not one of the allowed values").arg(key);
            return result;
        }
    }
    
    return result;
}void 
ConfigValidator::addRule(const QString& key, ValidationRule rule, 
                             const QVariantList& parameters, 
                             ValidationSeverity severity)
{
    QMutexLocker locker(&d->mutex);
    ValidationRuleInfo ruleInfo(rule, parameters, severity);
    d->rules[key].append(ruleInfo);
}

void ConfigValidator::addCustomValidator(const QString& key, CustomValidatorFunction validator, 
                                       ValidationSeverity severity)
{
    QMutexLocker locker(&d->mutex);
    d->customValidators[key] = validator;
}

void ConfigValidator::removeRule(const QString& key, ValidationRule rule)
{
    QMutexLocker locker(&d->mutex);
    if (d->rules.contains(key)) {
        auto& ruleList = d->rules[key];
        ruleList.erase(std::remove_if(ruleList.begin(), ruleList.end(),
                                     [rule](const ValidationRuleInfo& info) {
                                         return info.rule == rule;
                                     }), ruleList.end());
        if (ruleList.isEmpty()) {
            d->rules.remove(key);
        }
    }
}

IConfigValidator::ValidationResult ConfigValidator::validateValue(const QString& key, const QVariant& value) const
{
    QMutexLocker locker(&d->mutex);
    
    ValidationResult result;
    result.isValid = true;
    result.key = key;
    result.value = value;
    result.severityLevel = 0;
    result.message = "Validation passed";
    
    // 检查是否有针对此键的规则
    if (d->rules.contains(key)) {
        const auto& ruleList = d->rules[key];
        for (const auto& ruleInfo : ruleList) {
            ValidationResult ruleResult = validateWithRule(key, value, ruleInfo);
            if (!ruleResult.isValid) {
                return ruleResult;
            }
        }
    }
    
    // 检查自定义验证器
    if (d->customValidators.contains(key)) {
        return d->customValidators[key](key, value);
    }
    
    return result;
}

QList<IConfigValidator::ValidationResult> ConfigValidator::validateConfig(const QVariantMap& config) const
{
    QList<ValidationResult> results;
    
    for (auto it = config.begin(); it != config.end(); ++it) {
        ValidationResult keyResult = validateValue(it.key(), it.value());
        results.append(keyResult);
    }
    
    return results;
}

QList<IConfigValidator::ValidationResult> ConfigValidator::validateJson(const QJsonObject& json) const
{
    QList<ValidationResult> results;
    
    // 如果有 JSON Schema，使用它进行验证
    if (!d->jsonSchema.isEmpty()) {
        return validateWithSchema(json);
    }
    
    // 否则使用基本验证
    for (auto it = json.begin(); it != json.end(); ++it) {
        QVariant value = it.value().toVariant();
        ValidationResult keyResult = validateValue(it.key(), value);
        results.append(keyResult);
    }
    
    return results;
}

QList<IConfigValidator::ValidationError> ConfigValidator::getLastErrors() const
{
    QMutexLocker locker(&d->mutex);
    return d->lastErrors;
}

QStringList ConfigValidator::getWarnings() const
{
    QMutexLocker locker(&d->mutex);
    return d->warnings;
}

bool ConfigValidator::setJsonSchema(const QJsonObject& schema)
{
    QMutexLocker locker(&d->mutex);
    d->jsonSchema = schema;
    return true;
}

bool ConfigValidator::loadJsonSchema(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    setJsonSchema(doc.object());
    return true;
}

QList<IConfigValidator::ValidationResult> ConfigValidator::validateWithSchema(const QJsonObject& json) const
{
    QList<ValidationResult> results;
    
    // 简化的 JSON Schema 验证实现
    if (d->jsonSchema.contains("properties")) {
        QJsonObject properties = d->jsonSchema["properties"].toObject();
        
        for (auto it = properties.begin(); it != properties.end(); ++it) {
            QString key = it.key();
            QJsonObject propertySchema = it.value().toObject();
            
            if (json.contains(key)) {
                ValidationResult propertyResult = validateJsonValue(key, json[key], propertySchema);
                results.append(propertyResult);
            } else if (propertySchema.contains("required") && propertySchema["required"].toBool()) {
                ValidationResult result;
                result.isValid = false;
                result.severityLevel = 2;
                result.key = key;
                result.message = QString("Required property '%1' is missing").arg(key);
                results.append(result);
            }
        }
    }
    
    return results;
}

QMap<QString, QList<IConfigValidator::ValidationRule>> ConfigValidator::getAllRules() const
{
    QMutexLocker locker(&d->mutex);
    QMap<QString, QList<ValidationRule>> rules;
    
    for (auto it = d->rules.begin(); it != d->rules.end(); ++it) {
        QList<ValidationRule> ruleList;
        for (const auto& ruleInfo : it.value()) {
            ruleList.append(ruleInfo.rule);
        }
        rules[it.key()] = ruleList;
    }
    
    return rules;
}

bool ConfigValidator::hasRules(const QString& key) const
{
    QMutexLocker locker(&d->mutex);
    return d->rules.contains(key) || d->customValidators.contains(key);
}

void ConfigValidator::clearRules()
{
    QMutexLocker locker(&d->mutex);
    d->rules.clear();
    d->customValidators.clear();
}

void ConfigValidator::setStrictMode(bool strict)
{
    QMutexLocker locker(&d->mutex);
    d->strictMode = strict;
}

bool ConfigValidator::isStrictMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->strictMode;
}

void ConfigValidator::setDefaultSeverity(ValidationSeverity severity)
{
    QMutexLocker locker(&d->mutex);
    d->defaultSeverity = severity;
}

IConfigValidator::ValidationSeverity ConfigValidator::defaultSeverity() const
{
    QMutexLocker locker(&d->mutex);
    return d->defaultSeverity;
}

QJsonObject ConfigValidator::exportRulesToJson() const
{
    QMutexLocker locker(&d->mutex);
    QJsonObject rulesJson;
    
    for (auto it = d->rules.begin(); it != d->rules.end(); ++it) {
        QJsonArray ruleArray;
        for (const auto& ruleInfo : it.value()) {
            QJsonObject ruleObj;
            ruleObj["rule"] = static_cast<int>(ruleInfo.rule);
            ruleObj["severity"] = static_cast<int>(ruleInfo.severity);
            // 参数序列化可以根据需要实现
            ruleArray.append(ruleObj);
        }
        rulesJson[it.key()] = ruleArray;
    }
    
    return rulesJson;
}

bool ConfigValidator::importRulesFromJson(const QJsonObject& rulesJson)
{
    QMutexLocker locker(&d->mutex);
    
    try {
        for (auto it = rulesJson.begin(); it != rulesJson.end(); ++it) {
            QString key = it.key();
            QJsonArray ruleArray = it.value().toArray();
            
            QList<ValidationRuleInfo> ruleList;
            for (const QJsonValue& ruleValue : ruleArray) {
                QJsonObject ruleObj = ruleValue.toObject();
                ValidationRule rule = static_cast<ValidationRule>(ruleObj["rule"].toInt());
                ValidationSeverity severity = static_cast<ValidationSeverity>(ruleObj["severity"].toInt());
                
                ValidationRuleInfo ruleInfo(rule, QVariantList(), severity);
                ruleList.append(ruleInfo);
            }
            
            d->rules[key] = ruleList;
        }
        return true;
    } catch (...) {
        return false;
    }
}

void ConfigValidator::createPredefinedRuleSet(const QString& ruleSetName)
{
    QMutexLocker locker(&d->mutex);
    
    if (ruleSetName == "basic") {
        // 添加基本规则集
        addRule("server.host", Required, QVariantList(), ErrorLevel);
        addRule("server.port", Range, QVariantList() << 1 << 65535, ErrorLevel);
        addRule("user.name", Required, QVariantList(), ErrorLevel);
    } else if (ruleSetName == "network") {
        // 添加网络相关规则集
        addRule("network.timeout", Range, QVariantList() << 1000 << 30000, IConfigValidator::Warning);
        addRule("network.retries", Range, QVariantList() << 1 << 10, IConfigValidator::Warning);
    }
}

void ConfigValidator::validateConfigAsync(const QVariantMap& config)
{
    QtConcurrent::run([this, config]() {
        QList<ValidationResult> results = validateConfig(config);
        // 发出第一个结果或创建汇总结果
        ValidationResult summaryResult;
        summaryResult.isValid = true;
        summaryResult.message = "Async validation completed";
        
        for (const auto& result : results) {
            if (!result.isValid) {
                summaryResult = result;
                break;
            }
        }
        
        emit validationFinished(summaryResult);
    });
}

void ConfigValidator::validateJsonAsync(const QJsonObject& json)
{
    QtConcurrent::run([this, json]() {
        QList<ValidationResult> results = validateJson(json);
        // 发出第一个结果或创建汇总结果
        ValidationResult summaryResult;
        summaryResult.isValid = true;
        summaryResult.message = "Async JSON validation completed";
        
        for (const auto& result : results) {
            if (!result.isValid) {
                summaryResult = result;
                break;
            }
        }
        
        emit validationFinished(summaryResult);
    });
}

void ConfigValidator::reloadRules()
{
    QMutexLocker locker(&d->mutex);
    // 重新加载规则的实现
    clearRules();
    addBuiltinRules();
    emit rulesReloaded();
}

void ConfigValidator::optimizeRules()
{
    QMutexLocker locker(&d->mutex);
    // 优化规则的实现
    // 这里可以添加规则优化逻辑
    emit rulesOptimized();
}

void ConfigValidator::onAsyncValidationFinished()
{
    // 异步验证完成的处理
    emit asyncValidationCompleted();
}

void ConfigValidator::resetStatistics()
{
    QMutexLocker locker(&d->mutex);
    d->statistics.clear();
    d->lastErrors.clear();
    d->warnings.clear();
}

IConfigValidator::ValidationResult ConfigValidator::validateWithRule(const QString& key, const QVariant& value, const ValidationRuleInfo& ruleInfo) const
{
    ValidationResult result;
    result.isValid = true;
    result.key = key;
    result.value = value;
    result.severityLevel = static_cast<int>(ruleInfo.severity);
    result.message = "Validation passed";
    
    switch (ruleInfo.rule) {
        case Required:
            if (value.toString().isEmpty()) {
                result.isValid = false;
                result.message = QString("Value for '%1' cannot be empty").arg(key);
            }
            break;
            
        case Range:
            if (ruleInfo.parameters.size() >= 2) {
                double min = ruleInfo.parameters[0].toDouble();
                double max = ruleInfo.parameters[1].toDouble();
                double val = value.toDouble();
                
                if (val < min || val > max) {
                    result.isValid = false;
                    result.message = QString("Value for '%1' must be between %2 and %3").arg(key).arg(min).arg(max);
                }
            }
            break;
            
        case MinLength:
            if (!ruleInfo.parameters.isEmpty()) {
                int minLen = ruleInfo.parameters[0].toInt();
                if (value.toString().length() < minLen) {
                    result.isValid = false;
                    result.message = QString("Value for '%1' must be at least %2 characters long").arg(key).arg(minLen);
                }
            }
            break;
            
        case MaxLength:
            if (!ruleInfo.parameters.isEmpty()) {
                int maxLen = ruleInfo.parameters[0].toInt();
                if (value.toString().length() > maxLen) {
                    result.isValid = false;
                    result.message = QString("Value for '%1' must be at most %2 characters long").arg(key).arg(maxLen);
                }
            }
            break;
            
        case Pattern:
            if (!ruleInfo.parameters.isEmpty()) {
                QString pattern = ruleInfo.parameters[0].toString();
                QRegularExpression regex(pattern);
                if (!regex.match(value.toString()).hasMatch()) {
                    result.isValid = false;
                    result.message = QString("Value for '%1' does not match required pattern").arg(key);
                }
            }
            break;
            
        case Custom:
            // 自定义规则处理
            break;
    }
    
    return result;
}