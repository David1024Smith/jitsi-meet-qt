#include "SchemaValidator.h"
#include <QFile>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QUrl>
#include <QDebug>
#include <QThread>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

class SchemaValidator::Private
{
public:
    QJsonObject currentSchema;
    SchemaVersion version;
    bool strictMode;
    int maxErrors;
    QMap<QString, std::function<bool(const QString&)>> formatValidators;
    QMap<QString, QJsonObject> schemaReferences;
    
    Private() 
        : version(Draft7)
        , strictMode(false)
        , maxErrors(100)
    {
        setupBuiltinFormatValidators();
    }
    
    void setupBuiltinFormatValidators();
};

SchemaValidator::SchemaValidator(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
}

SchemaValidator::~SchemaValidator() = default;

SchemaValidator* SchemaValidator::instance()
{
    static SchemaValidator* inst = nullptr;
    if (!inst) {
        inst = new SchemaValidator();
    }
    return inst;
}

QString SchemaValidator::schemaVersion() const
{
    return versionToString(d->version);
}

void SchemaValidator::setSchemaVersion(const QString& version)
{
    SchemaVersion ver = stringToVersion(version);
    if (d->version != ver) {
        d->version = ver;
        emit schemaVersionChanged(version);
    }
}

void SchemaValidator::setSchemaVersion(SchemaVersion version)
{
    if (d->version != version) {
        d->version = version;
        emit schemaVersionChanged(versionToString(version));
    }
}

SchemaValidator::SchemaVersion SchemaValidator::schemaVersionEnum() const
{
    return d->version;
}

bool SchemaValidator::isStrictMode() const
{
    return d->strictMode;
}

void SchemaValidator::setStrictMode(bool strict)
{
    if (d->strictMode != strict) {
        d->strictMode = strict;
        emit strictModeChanged(strict);
    }
}

int SchemaValidator::maxErrors() const
{
    return d->maxErrors;
}

void SchemaValidator::setMaxErrors(int max)
{
    if (d->maxErrors != max) {
        d->maxErrors = max;
        emit maxErrorsChanged(max);
    }
}

bool SchemaValidator::setSchema(const QJsonObject& schema)
{
    if (!validateSchema(schema).isEmpty()) {
        emit errorOccurred("Invalid schema provided");
        return false;
    }
    
    d->currentSchema = schema;
    emit schemaSet(true);
    return true;
}

bool SchemaValidator::loadSchemaFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Cannot open schema file: " + filePath);
        return false;
    }
    
    QByteArray data = file.readAll();
    return loadSchemaFromString(QString::fromUtf8(data));
}

bool SchemaValidator::loadSchemaFromString(const QString& schemaString)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(schemaString.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        emit errorOccurred("JSON parse error: " + error.errorString());
        return false;
    }
    
    return setSchema(doc.object());
}

QJsonObject SchemaValidator::schema() const
{
    return d->currentSchema;
}

bool SchemaValidator::hasSchema() const
{
    return !d->currentSchema.isEmpty();
}

void SchemaValidator::clearSchema()
{
    d->currentSchema = QJsonObject();
}

QList<SchemaValidator::ValidationError> SchemaValidator::validate(const QJsonObject& json, const ValidationOptions& options) const
{
    if (!hasSchema()) {
        return {ValidationError(CustomError, "", "No schema set for validation")};
    }
    
    return validateValue(json, d->currentSchema, "", options);
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateValue(const QJsonValue& value, const QJsonObject& schema, const QString& path, const ValidationOptions& options) const
{
    QList<ValidationError> errors;
    
    // Type validation
    if (schema.contains("type")) {
        QJsonValue typeValue = schema["type"];
        if (typeValue.isString()) {
            QString expectedType = typeValue.toString();
            if (!isValidType(value, expectedType)) {
                QStringList actualTypes = getValueTypes(value);
                errors.append(ValidationError(TypeMismatch, path, 
                    QString("Expected type %1, got %2").arg(expectedType, actualTypes.join(" or ")),
                    value, QJsonValue(expectedType)));
            }
        }
    }
    
    // Format validation
    if (schema.contains("format") && value.isString() && options.validateFormats) {
        QString format = schema["format"].toString();
        if (!validateFormat(value.toString(), format)) {
            errors.append(ValidationError(FormatError, path,
                QString("Value does not match format: %1").arg(format),
                value, QJsonValue(format)));
        }
    }
    
    // Enum validation
    if (schema.contains("enum")) {
        QJsonArray enumArray = schema["enum"].toArray();
        bool found = false;
        for (const QJsonValue& enumValue : enumArray) {
            if (enumValue == value) {
                found = true;
                break;
            }
        }
        if (!found) {
            errors.append(ValidationError(EnumViolation, path,
                "Value is not in allowed enum values", value));
        }
    }
    
    // Type-specific validation
    if (value.isObject()) {
        QList<ValidationError> objectErrors = validateObject(value.toObject(), schema, path, options);
        errors.append(objectErrors);
    } else if (value.isArray()) {
        QList<ValidationError> arrayErrors = validateArray(value.toArray(), schema, path, options);
        errors.append(arrayErrors);
    } else if (value.isString()) {
        QList<ValidationError> stringErrors = validateString(value.toString(), schema, path, options);
        errors.append(stringErrors);
    } else if (value.isDouble()) {
        QList<ValidationError> numberErrors = validateNumber(value.toDouble(), schema, path, options);
        errors.append(numberErrors);
    }
    
    // Conditional validation
    if (schema.contains("allOf")) {
        QList<ValidationError> allOfErrors = validateAllOf(value, schema["allOf"].toArray(), path, options);
        errors.append(allOfErrors);
    }
    
    if (schema.contains("anyOf")) {
        QList<ValidationError> anyOfErrors = validateAnyOf(value, schema["anyOf"].toArray(), path, options);
        errors.append(anyOfErrors);
    }
    
    if (schema.contains("oneOf")) {
        QList<ValidationError> oneOfErrors = validateOneOf(value, schema["oneOf"].toArray(), path, options);
        errors.append(oneOfErrors);
    }
    
    if (schema.contains("not")) {
        QList<ValidationError> notErrors = validateNot(value, schema["not"].toObject(), path, options);
        errors.append(notErrors);
    }
    
    // Limit errors if needed
    if (errors.size() > d->maxErrors) {
        errors = errors.mid(0, d->maxErrors);
    }
    
    return errors;
}

bool SchemaValidator::isValid(const QJsonObject& json) const
{
    return validate(json).isEmpty();
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateSchema(const QJsonObject& schema) const
{
    QList<ValidationError> errors;
    
    // Basic schema structure validation
    if (schema.contains("$schema")) {
        QString schemaUri = schema["$schema"].toString();
        // Validate schema URI format
        QUrl url(schemaUri);
        if (!url.isValid()) {
            errors.append(ValidationError(FormatError, "$schema", "Invalid schema URI"));
        }
    }
    
    // Validate type definitions
    if (schema.contains("type")) {
        QJsonValue typeValue = schema["type"];
        if (typeValue.isString()) {
            QString type = typeValue.toString();
            QStringList validTypes = {"null", "boolean", "object", "array", "number", "string", "integer"};
            if (!validTypes.contains(type)) {
                errors.append(ValidationError(TypeMismatch, "type", "Invalid type: " + type));
            }
        }
    }
    
    return errors;
}

void SchemaValidator::addFormatValidator(const QString& format, std::function<bool(const QString&)> validator)
{
    d->formatValidators[format] = validator;
}

void SchemaValidator::removeFormatValidator(const QString& format)
{
    d->formatValidators.remove(format);
}

QStringList SchemaValidator::supportedFormats() const
{
    return d->formatValidators.keys();
}

bool SchemaValidator::validateFormat(const QString& value, const QString& format) const
{
    if (d->formatValidators.contains(format)) {
        return d->formatValidators[format](value);
    }
    return true; // Unknown formats pass by default
}

void SchemaValidator::addSchemaReference(const QString& uri, const QJsonObject& schema)
{
    d->schemaReferences[uri] = schema;
}

void SchemaValidator::removeSchemaReference(const QString& uri)
{
    d->schemaReferences.remove(uri);
}

QJsonObject SchemaValidator::resolveReference(const QString& ref) const
{
    if (d->schemaReferences.contains(ref)) {
        return d->schemaReferences[ref];
    }
    return QJsonObject();
}

QMap<QString, QJsonObject> SchemaValidator::schemaReferences() const
{
    return d->schemaReferences;
}

// Private implementation methods
QList<SchemaValidator::ValidationError> SchemaValidator::validateObject(const QJsonObject& object, const QJsonObject& schema, const QString& path, const ValidationOptions& options) const
{
    QList<ValidationError> errors;
    
    // Required properties
    if (schema.contains("required")) {
        QJsonArray required = schema["required"].toArray();
        errors.append(validateRequired(object, required, path));
    }
    
    // Properties validation
    if (schema.contains("properties")) {
        QJsonObject properties = schema["properties"].toObject();
        errors.append(validateProperties(object, properties, path, options));
    }
    
    // Additional properties
    if (schema.contains("additionalProperties")) {
        errors.append(validateAdditionalProperties(object, schema, path, options));
    }
    
    return errors;
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateArray(const QJsonArray& array, const QJsonObject& schema, const QString& path, const ValidationOptions& options) const
{
    QList<ValidationError> errors;
    
    // Items validation
    if (schema.contains("items")) {
        QJsonValue itemsSchema = schema["items"];
        if (itemsSchema.isObject()) {
            for (int i = 0; i < array.size(); ++i) {
                QString itemPath = buildPath(path, i);
                errors.append(validateValue(array[i], itemsSchema.toObject(), itemPath, options));
            }
        }
    }
    
    // Length constraints
    if (schema.contains("minItems")) {
        int minItems = schema["minItems"].toInt();
        if (array.size() < minItems) {
            errors.append(ValidationError(LengthError, path,
                QString("Array has %1 items, minimum is %2").arg(array.size()).arg(minItems)));
        }
    }
    
    if (schema.contains("maxItems")) {
        int maxItems = schema["maxItems"].toInt();
        if (array.size() > maxItems) {
            errors.append(ValidationError(LengthError, path,
                QString("Array has %1 items, maximum is %2").arg(array.size()).arg(maxItems)));
        }
    }
    
    return errors;
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateString(const QString& string, const QJsonObject& schema, const QString& path, const ValidationOptions& options) const
{
    QList<ValidationError> errors;
    
    // Length constraints
    if (schema.contains("minLength")) {
        int minLength = schema["minLength"].toInt();
        if (string.length() < minLength) {
            errors.append(ValidationError(LengthError, path,
                QString("String length %1 is less than minimum %2").arg(string.length()).arg(minLength)));
        }
    }
    
    if (schema.contains("maxLength")) {
        int maxLength = schema["maxLength"].toInt();
        if (string.length() > maxLength) {
            errors.append(ValidationError(LengthError, path,
                QString("String length %1 exceeds maximum %2").arg(string.length()).arg(maxLength)));
        }
    }
    
    // Pattern validation
    if (schema.contains("pattern")) {
        QString pattern = schema["pattern"].toString();
        QRegularExpression regex(pattern);
        if (!regex.match(string).hasMatch()) {
            errors.append(ValidationError(PatternMismatch, path,
                QString("String does not match pattern: %1").arg(pattern)));
        }
    }
    
    return errors;
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateNumber(double number, const QJsonObject& schema, const QString& path, const ValidationOptions& options) const
{
    Q_UNUSED(options)
    QList<ValidationError> errors;
    
    // Range constraints
    if (schema.contains("minimum")) {
        double minimum = schema["minimum"].toDouble();
        if (number < minimum) {
            errors.append(ValidationError(RangeError, path,
                QString("Number %1 is less than minimum %2").arg(number).arg(minimum)));
        }
    }
    
    if (schema.contains("maximum")) {
        double maximum = schema["maximum"].toDouble();
        if (number > maximum) {
            errors.append(ValidationError(RangeError, path,
                QString("Number %1 exceeds maximum %2").arg(number).arg(maximum)));
        }
    }
    
    return errors;
}

// Helper methods
QString SchemaValidator::buildPath(const QString& basePath, const QString& key) const
{
    return basePath.isEmpty() ? key : basePath + "." + key;
}

QString SchemaValidator::buildPath(const QString& basePath, int index) const
{
    return basePath + "[" + QString::number(index) + "]";
}

bool SchemaValidator::isValidType(const QJsonValue& value, const QString& type) const
{
    if (type == "null") return value.isNull();
    if (type == "boolean") return value.isBool();
    if (type == "object") return value.isObject();
    if (type == "array") return value.isArray();
    if (type == "string") return value.isString();
    if (type == "number") return value.isDouble();
    if (type == "integer") return value.isDouble() && (value.toDouble() == qFloor(value.toDouble()));
    return false;
}

QStringList SchemaValidator::getValueTypes(const QJsonValue& value) const
{
    QStringList types;
    if (value.isNull()) types << "null";
    if (value.isBool()) types << "boolean";
    if (value.isObject()) types << "object";
    if (value.isArray()) types << "array";
    if (value.isString()) types << "string";
    if (value.isDouble()) {
        types << "number";
        if (value.toDouble() == qFloor(value.toDouble())) {
            types << "integer";
        }
    }
    return types;
}

QString SchemaValidator::versionToString(SchemaVersion version) const
{
    switch (version) {
    case Draft4: return "http://json-schema.org/draft-04/schema#";
    case Draft6: return "http://json-schema.org/draft-06/schema#";
    case Draft7: return "http://json-schema.org/draft-07/schema#";
    case Draft201909: return "https://json-schema.org/draft/2019-09/schema";
    }
    return "http://json-schema.org/draft-07/schema#";
}

SchemaValidator::SchemaVersion SchemaValidator::stringToVersion(const QString& str) const
{
    if (str.contains("draft-04")) return Draft4;
    if (str.contains("draft-06")) return Draft6;
    if (str.contains("draft-07")) return Draft7;
    if (str.contains("2019-09")) return Draft201909;
    return Draft7;
}

void SchemaValidator::Private::setupBuiltinFormatValidators()
{
    // Email format
    formatValidators["email"] = [](const QString& value) {
        QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        return emailRegex.match(value).hasMatch();
    };
    
    // URI format
    formatValidators["uri"] = [](const QString& value) {
        QUrl url(value);
        return url.isValid() && !url.scheme().isEmpty();
    };
    
    // Date format (ISO 8601)
    formatValidators["date"] = [](const QString& value) {
        QRegularExpression dateRegex(R"(^\d{4}-\d{2}-\d{2}$)");
        return dateRegex.match(value).hasMatch();
    };
    
    // Time format
    formatValidators["time"] = [](const QString& value) {
        QRegularExpression timeRegex(R"(^\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[+-]\d{2}:\d{2})?$)");
        return timeRegex.match(value).hasMatch();
    };
    
    // DateTime format
    formatValidators["date-time"] = [](const QString& value) {
        QRegularExpression dateTimeRegex(R"(^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[+-]\d{2}:\d{2})?$)");
        return dateTimeRegex.match(value).hasMatch();
    };
}

// Stub implementations for remaining methods
QList<SchemaValidator::ValidationError> SchemaValidator::validateRequired(const QJsonObject& object, const QJsonArray& required, const QString& path) const
{
    QList<ValidationError> errors;
    for (const QJsonValue& reqValue : required) {
        QString key = reqValue.toString();
        if (!object.contains(key)) {
            errors.append(ValidationError(RequiredMissing, buildPath(path, key),
                QString("Required property '%1' is missing").arg(key)));
        }
    }
    return errors;
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateProperties(const QJsonObject& object, const QJsonObject& properties, const QString& path, const ValidationOptions& options) const
{
    QList<ValidationError> errors;
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
        QString key = it.key();
        if (object.contains(key)) {
            QString propPath = buildPath(path, key);
            errors.append(validateValue(object[key], it.value().toObject(), propPath, options));
        }
    }
    return errors;
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateAdditionalProperties(const QJsonObject& object, const QJsonObject& schema, const QString& path, const ValidationOptions& options) const
{
    Q_UNUSED(object)
    Q_UNUSED(schema)
    Q_UNUSED(path)
    Q_UNUSED(options)
    
    // Simplified implementation
    return QList<ValidationError>();
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateAllOf(const QJsonValue& value, const QJsonArray& allOf, const QString& path, const ValidationOptions& options) const
{
    QList<ValidationError> errors;
    for (const QJsonValue& schemaValue : allOf) {
        errors.append(validateValue(value, schemaValue.toObject(), path, options));
    }
    return errors;
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateAnyOf(const QJsonValue& value, const QJsonArray& anyOf, const QString& path, const ValidationOptions& options) const
{
    Q_UNUSED(value)
    Q_UNUSED(anyOf)
    Q_UNUSED(path)
    Q_UNUSED(options)
    
    // Simplified implementation - should check if at least one schema validates
    return QList<ValidationError>();
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateOneOf(const QJsonValue& value, const QJsonArray& oneOf, const QString& path, const ValidationOptions& options) const
{
    Q_UNUSED(value)
    Q_UNUSED(oneOf)
    Q_UNUSED(path)
    Q_UNUSED(options)
    
    // Simplified implementation - should check if exactly one schema validates
    return QList<ValidationError>();
}

QList<SchemaValidator::ValidationError> SchemaValidator::validateNot(const QJsonValue& value, const QJsonObject& notSchema, const QString& path, const ValidationOptions& options) const
{
    Q_UNUSED(value)
    Q_UNUSED(notSchema)
    Q_UNUSED(path)
    Q_UNUSED(options)
    
    // Simplified implementation - should check that the schema does NOT validate
    return QList<ValidationError>();
}

// Static utility methods
QJsonObject SchemaValidator::getBasicTypeSchema(const QString& type)
{
    QJsonObject schema;
    schema["type"] = type;
    return schema;
}

QJsonObject SchemaValidator::createSettingsConfigSchema()
{
    QJsonObject schema;
    schema["$schema"] = "http://json-schema.org/draft-07/schema#";
    schema["type"] = "object";
    
    QJsonObject properties;
    properties["audio"] = getBasicTypeSchema("object");
    properties["video"] = getBasicTypeSchema("object");
    properties["network"] = getBasicTypeSchema("object");
    
    schema["properties"] = properties;
    return schema;
}

/**
 * @brief 异步验证JSON对象
 * @param json 待验证的JSON对象
 */
void SchemaValidator::validateAsync(const QJsonObject& json)
{
    // 使用QtConcurrent在后台线程中执行验证
    QFuture<QList<ValidationError>> future = QtConcurrent::run([this, json]() {
        return validate(json);
    });
    
    // 监听验证完成
    QFutureWatcher<QList<ValidationError>>* watcher = new QFutureWatcher<QList<ValidationError>>(this);
    connect(watcher, &QFutureWatcher<QList<ValidationError>>::finished, [this, watcher, json]() {
        QList<ValidationError> errors = watcher->result();
        emit asyncValidationCompleted(json, errors);
        emit validationCompleted(errors);
        watcher->deleteLater();
    });
    
    watcher->setFuture(future);
}

/**
 * @brief 重新加载内置格式验证器
 */
void SchemaValidator::reloadBuiltinValidators()
{
    // 清除现有的格式验证器
    d->formatValidators.clear();
    
    // 重新设置内置格式验证器
    d->setupBuiltinFormatValidators();
    
    qDebug() << "Built-in format validators reloaded";
}