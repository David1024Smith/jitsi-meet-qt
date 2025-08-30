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
#include <QtConcurrent>

class ConfigValidator::Private
{
public:
    QMap<QString, QList<ValidationRuleInfo>> rules;
    QMap<QString, CustomValidatorFunction> customValidators;
    QMap<QString, QMap<QString, QVariant>> dependencyRules;
    QList<CustomValidatorFunction> globalValidators;
    
    QJsonObject jsonSchema;
    bool strictMode;
    ValidationSeverity defaultSeverity;
    ValidationContext context;
    
    int validationTimeout;
    bool parallelValidationEnabled;
    int maxValidationDepth;
    
    // Statistics
    QVariantMap statistics;
    QMutex statisticsMutex;
    
    Private()
        : strictMode(false)
        , defaultSeverity(Error)
        , validationTimeout(5000) // 5 seconds
        , parallelValidationEnabled(false)
        , maxValidationDepth(10)
    {
        statistics["validations"] = 0;
        statistics["successes"] = 0;
        statistics["failures"] = 0;
        statistics["warnings"] = 0;
        statistics["errors"] = 0;
    }
};

ConfigValidator::ConfigValidator(QObject* parent)
    : IConfigValidator(parent)
    , d(std::make_unique<Private>())
{
    addBuiltinRules();
}

ConfigValidator::~ConfigValidator() = default;

ConfigValidator* ConfigValidator::instance()
{
    static ConfigValidator* instance = nullptr;
    if (!instance) {
        instance = new ConfigValidator();
    }
    return instance;
}

bool ConfigValidator::initialize()
{
    // Load predefined rule sets
    createPredefinedRuleSet("audio");
    createPredefinedRuleSet("video");
    createPredefinedRuleSet("network");
    createPredefinedRuleSet("ui");
    createPredefinedRuleSet("performance");
    createPredefinedRuleSet("security");
    
    return true;
}

void ConfigValidator::addRule(const QString& key, ValidationRule rule, 
                             const QVariantList& parameters, ValidationSeverity severity)
{
    ValidationRuleInfo ruleInfo(rule, parameters, severity);
    addRule(key, ruleInfo);
}

void ConfigValidator::addRule(const QString& key, const ValidationRuleInfo& ruleInfo)
{
    if (!d->rules.contains(key)) {
        d->rules[key] = QList<ValidationRuleInfo>();
    }
    
    // Remove existing rule of the same type
    auto& ruleList = d->rules[key];
    ruleList.removeIf([&](const ValidationRuleInfo& existing) {
        return existing.rule == ruleInfo.rule;
    });
    
    ruleList.append(ruleInfo);
}

void ConfigValidator::addCustomValidator(const QString& key, CustomValidatorFunction validator, 
                                        ValidationSeverity severity)
{
    d->customValidators[key] = validator;
    
    ValidationRuleInfo ruleInfo(Custom, QVariantList(), severity);
    addRule(key, ruleInfo);
}

void ConfigValidator::removeRule(const QString& key, ValidationRule rule)
{
    if (!d->rules.contains(key)) {
        return;
    }
    
    auto& ruleList = d->rules[key];
    if (rule == Custom) {
        // Remove all rules for this key
        ruleList.clear();
        d->customValidators.remove(key);
    } else {
        // Remove specific rule type
        ruleList.removeIf([rule](const ValidationRuleInfo& ruleInfo) {
            return ruleInfo.rule == rule;
        });
    }
    
    if (ruleList.isEmpty()) {
        d->rules.remove(key);
    }
}

ValidationResult ConfigValidator::validateValue(const QString& key, const QVariant& value) const
{
    updateStatistics("validation");
    
    if (!d->rules.contains(key)) {
        ValidationResult result;
        result.isValid = true;
        result.key = key;
        result.value = value;
        result.severity = Info;
        result.message = "No validation rules defined";
        return result;
    }
    
    const auto& ruleList = d->rules[key];
    
    for (const ValidationRuleInfo& ruleInfo : ruleList) {
        if (!ruleInfo.enabled) {
            continue;
        }
        
        ValidationResult result = executeRule(key, value, ruleInfo);
        
        if (!result.isValid) {
            updateStatistics(result.severity == Warning ? "warnings" : "errors");
            updateStatistics("failures");
            return result;
        }
    }
    
    // Check custom validators
    if (d->customValidators.contains(key)) {
        CustomValidatorFunction validator = d->customValidators[key];
        ValidationResult result = validator(key, value);
        
        if (!result.isValid) {
            updateStatistics(result.severity == Warning ? "warnings" : "errors");
            updateStatistics("failures");
            return result;
        }
    }
    
    // Check global validators
    for (const CustomValidatorFunction& validator : d->globalValidators) {
        ValidationResult result = validator(key, value);
        
        if (!result.isValid) {
            updateStatistics(result.severity == Warning ? "warnings" : "errors");
            updateStatistics("failures");
            return result;
        }
    }
    
    updateStatistics("successes");
    
    ValidationResult result;
    result.isValid = true;
    result.key = key;
    result.value = value;
    result.severity = Info;
    result.message = "Validation passed";
    return result;
}

QList<ValidationResult> ConfigValidator::validateConfig(const QVariantMap& config) const
{
    QList<ValidationResult> results;
    
    // Set validation context
    ValidationContext context;
    context.fullConfig = config;
    const_cast<ConfigValidator*>(this)->setValidationContext(context);
    
    if (d->parallelValidationEnabled && config.size() > 10) {
        // Use parallel validation for large configs
        QList<QString> keys = config.keys();
        
        auto validateKey = [this, &config](const QString& key) -> ValidationResult {
            return validateValue(key, config[key]);
        };
        
        results = QtConcurrent::blockingMapped(keys, validateKey);
    } else {
        // Sequential validation
        for (auto it = config.constBegin(); it != config.constEnd(); ++it) {
            ValidationResult result = validateValue(it.key(), it.value());
            results.append(result);
            
            // Check validation depth
            if (d->context.depth >= d->maxValidationDepth) {
                ValidationResult depthResult;
                depthResult.isValid = false;
                depthResult.key = it.key();
                depthResult.severity = Warning;
                depthResult.message = "Maximum validation depth exceeded";
                results.append(depthResult);
                break;
            }
        }
    }
    
    // Check dependencies
    for (auto it = config.constBegin(); it != config.constEnd(); ++it) {
        if (!checkDependencies(it.key(), config)) {
            ValidationResult result;
            result.isValid = false;
            result.key = it.key();
            result.severity = Error;
            result.message = "Dependency validation failed";
            results.append(result);
        }
    }
    
    return results;
}

QList<ValidationResult> ConfigValidator::validateJson(const QJsonObject& json) const
{
    QVariantMap config = json.toVariantMap();
    return validateConfig(config);
}

bool ConfigValidator::setJsonSchema(const QJsonObject& schema)
{
    if (!validateJsonSchema(schema)) {
        return false;
    }
    
    d->jsonSchema = schema;
    return true;
}

bool ConfigValidator::loadJsonSchema(const QString& schemaFilePath)
{
    QFile file(schemaFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    return setJsonSchema(doc.object());
}

QList<ValidationResult> ConfigValidator::validateWithSchema(const QJsonObject& json) const
{
    QList<ValidationResult> results;
    
    if (d->jsonSchema.isEmpty()) {
        ValidationResult result;
        result.isValid = false;
        result.severity = Error;
        result.message = "No JSON schema loaded";
        results.append(result);
        return results;
    }
    
    // Basic schema validation implementation
    // This is a simplified version - a full implementation would be much more complex
    
    QJsonObject properties = d->jsonSchema["properties"].toObject();
    QJsonArray required = d->jsonSchema["required"].toArray();
    
    // Check required properties
    for (const QJsonValue& reqValue : required) {
        QString reqKey = reqValue.toString();
        if (!json.contains(reqKey)) {
            ValidationResult result;
            result.isValid = false;
            result.key = reqKey;
            result.severity = Error;
            result.message = QString("Required property '%1' is missing").arg(reqKey);
            results.append(result);
        }
    }
    
    // Validate properties
    for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
        const QString& key = it.key();
        const QJsonValue& value = it.value();
        
        if (properties.contains(key)) {
            QJsonObject propSchema = properties[key].toObject();
            ValidationResult result = validateJsonValue(key, value, propSchema);
            if (!result.isValid) {
                results.append(result);
            }
        } else if (d->strictMode) {
            ValidationResult result;
            result.isValid = false;
            result.key = key;
            result.severity = Warning;
            result.message = QString("Unknown property '%1'").arg(key);
            results.append(result);
        }
    }
    
    return results;
}

QMap<QString, QList<ValidationRule>> ConfigValidator::getAllRules() const
{
    QMap<QString, QList<ValidationRule>> allRules;
    
    for (auto it = d->rules.constBegin(); it != d->rules.constEnd(); ++it) {
        const QString& key = it.key();
        const QList<ValidationRuleInfo>& ruleInfoList = it.value();
        
        QList<ValidationRule> rules;
        for (const ValidationRuleInfo& ruleInfo : ruleInfoList) {
            rules.append(ruleInfo.rule);
        }
        
        allRules[key] = rules;
    }
    
    return allRules;
}

bool ConfigValidator::hasRules(const QString& key) const
{
    return d->rules.contains(key) && !d->rules[key].isEmpty();
}

void ConfigValidator::clearRules()
{
    d->rules.clear();
    d->customValidators.clear();
    d->dependencyRules.clear();
    d->globalValidators.clear();
}

void ConfigValidator::setStrictMode(bool strict)
{
    d->strictMode = strict;
}

bool ConfigValidator::isStrictMode() const
{
    return d->strictMode;
}

void ConfigValidator::setDefaultSeverity(ValidationSeverity severity)
{
    d->defaultSeverity = severity;
}

ValidationSeverity ConfigValidator::defaultSeverity() const
{
    return d->defaultSeverity;
}

QJsonObject ConfigValidator::exportRulesToJson() const
{
    QJsonObject json;
    
    for (auto it = d->rules.constBegin(); it != d->rules.constEnd(); ++it) {
        const QString& key = it.key();
        const QList<ValidationRuleInfo>& ruleList = it.value();
        
        QJsonArray rulesArray;
        for (const ValidationRuleInfo& ruleInfo : ruleList) {
            QJsonObject ruleObj;
            ruleObj["rule"] = ruleToString(ruleInfo.rule);
            ruleObj["parameters"] = QJsonArray::fromVariantList(ruleInfo.parameters);
            ruleObj["severity"] = severityToString(ruleInfo.severity);
            ruleObj["description"] = ruleInfo.description;
            ruleObj["enabled"] = ruleInfo.enabled;
            
            rulesArray.append(ruleObj);
        }
        
        json[key] = rulesArray;
    }
    
    return json;
}

bool ConfigValidator::importRulesFromJson(const QJsonObject& json)
{
    clearRules();
    
    for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
        const QString& key = it.key();
        const QJsonArray& rulesArray = it.value().toArray();
        
        for (const QJsonValue& ruleValue : rulesArray) {
            QJsonObject ruleObj = ruleValue.toObject();
            
            ValidationRuleInfo ruleInfo;
            ruleInfo.rule = stringToRule(ruleObj["rule"].toString());
            ruleInfo.parameters = ruleObj["parameters"].toArray().toVariantList();
            ruleInfo.severity = stringToSeverity(ruleObj["severity"].toString());
            ruleInfo.description = ruleObj["description"].toString();
            ruleInfo.enabled = ruleObj["enabled"].toBool(true);
            
            addRule(key, ruleInfo);
        }
    }
    
    return true;
}

void ConfigValidator::createPredefinedRuleSet(const QString& ruleSetName)
{
    if (ruleSetName == "audio") {
        createAudioRuleSet();
    } else if (ruleSetName == "video") {
        createVideoRuleSet();
    } else if (ruleSetName == "network") {
        createNetworkRuleSet();
    } else if (ruleSetName == "ui") {
        createUIRuleSet();
    } else if (ruleSetName == "performance") {
        createPerformanceRuleSet();
    } else if (ruleSetName == "security") {
        createSecurityRuleSet();
    }
}