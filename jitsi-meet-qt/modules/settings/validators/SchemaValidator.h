#ifndef SCHEMAVALIDATOR_H
#define SCHEMAVALIDATOR_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QStringList>
#include <QVariantMap>
#include <QRegularExpression>
#include <memory>

/**
 * @brief JSON模式验证器类
 * 
 * 提供完整的JSON Schema验证功能，支持Draft 4/6/7/2019-09规范。
 * 包含类型验证、格式验证、约束验证和自定义验证器。
 */
class SchemaValidator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString schemaVersion READ schemaVersion WRITE setSchemaVersion NOTIFY schemaVersionChanged)
    Q_PROPERTY(bool strictMode READ isStrictMode WRITE setStrictMode NOTIFY strictModeChanged)
    Q_PROPERTY(int maxErrors READ maxErrors WRITE setMaxErrors NOTIFY maxErrorsChanged)

public:
    /**
     * @brief JSON Schema版本枚举
     */
    enum SchemaVersion {
        Draft4,         ///< Draft 4
        Draft6,         ///< Draft 6
        Draft7,         ///< Draft 7 (默认)
        Draft201909     ///< Draft 2019-09
    };
    Q_ENUM(SchemaVersion)

    /**
     * @brief 验证错误类型
     */
    enum ErrorType {
        TypeMismatch,       ///< 类型不匹配
        FormatError,        ///< 格式错误
        ConstraintViolation,///< 约束违反
        RequiredMissing,    ///< 必需字段缺失
        AdditionalProperty, ///< 额外属性
        PatternMismatch,    ///< 模式不匹配
        EnumViolation,      ///< 枚举违反
        RangeError,         ///< 范围错误
        LengthError,        ///< 长度错误
        UniqueViolation,    ///< 唯一性违反
        DependencyError,    ///< 依赖错误
        ConditionalError,   ///< 条件错误
        CustomError         ///< 自定义错误
    };
    Q_ENUM(ErrorType)

    /**
     * @brief 验证结果结构
     */
    struct ValidationError {
        ErrorType type;             ///< 错误类型
        QString path;               ///< JSON路径
        QString message;            ///< 错误消息
        QJsonValue actualValue;     ///< 实际值
        QJsonValue expectedValue;   ///< 期望值
        QString schemaPath;         ///< Schema路径
        
        ValidationError() : type(CustomError) {}
        ValidationError(ErrorType t, const QString& p, const QString& msg, 
                       const QJsonValue& actual = QJsonValue(), 
                       const QJsonValue& expected = QJsonValue(),
                       const QString& sPath = QString())
            : type(t), path(p), message(msg), actualValue(actual), 
              expectedValue(expected), schemaPath(sPath) {}
    };

    /**
     * @brief 验证选项结构
     */
    struct ValidationOptions {
        bool strictMode;            ///< 严格模式
        bool allowAdditionalProperties; ///< 允许额外属性
        bool validateFormats;       ///< 验证格式
        bool collectAllErrors;      ///< 收集所有错误
        int maxErrors;              ///< 最大错误数
        QStringList ignoredPaths;   ///< 忽略的路径
        
        ValidationOptions() 
            : strictMode(false), allowAdditionalProperties(true)
            , validateFormats(true), collectAllErrors(false), maxErrors(100) {}
    };

    explicit SchemaValidator(QObject* parent = nullptr);
    ~SchemaValidator();

    /**
     * @brief 获取单例实例
     * @return 验证器实例
     */
    static SchemaValidator* instance();

    /**
     * @brief 获取Schema版本
     * @return Schema版本字符串
     */
    QString schemaVersion() const;

    /**
     * @brief 设置Schema版本
     * @param version 版本字符串
     */
    void setSchemaVersion(const QString& version);

    /**
     * @brief 设置Schema版本
     * @param version 版本枚举
     */
    void setSchemaVersion(SchemaVersion version);

    /**
     * @brief 获取Schema版本枚举
     * @return 版本枚举
     */
    SchemaVersion schemaVersionEnum() const;

    /**
     * @brief 检查是否为严格模式
     * @return 是否为严格模式
     */
    bool isStrictMode() const;

    /**
     * @brief 设置严格模式
     * @param strict 是否严格
     */
    void setStrictMode(bool strict);

    /**
     * @brief 获取最大错误数
     * @return 最大错误数
     */
    int maxErrors() const;

    /**
     * @brief 设置最大错误数
     * @param max 最大错误数
     */
    void setMaxErrors(int max);

    // Schema管理
    /**
     * @brief 设置JSON Schema
     * @param schema Schema对象
     * @return 设置是否成功
     */
    bool setSchema(const QJsonObject& schema);

    /**
     * @brief 从文件加载Schema
     * @param filePath Schema文件路径
     * @return 加载是否成功
     */
    bool loadSchemaFromFile(const QString& filePath);

    /**
     * @brief 从字符串加载Schema
     * @param schemaString Schema字符串
     * @return 加载是否成功
     */
    bool loadSchemaFromString(const QString& schemaString);

    /**
     * @brief 获取当前Schema
     * @return Schema对象
     */
    QJsonObject schema() const;

    /**
     * @brief 检查是否有有效Schema
     * @return 是否有Schema
     */
    bool hasSchema() const;

    /**
     * @brief 清除Schema
     */
    void clearSchema();

    // 验证功能
    /**
     * @brief 验证JSON对象
     * @param json 待验证的JSON对象
     * @param options 验证选项
     * @return 验证错误列表（空表示验证通过）
     */
    QList<ValidationError> validate(const QJsonObject& json, const ValidationOptions& options = ValidationOptions()) const;

    /**
     * @brief 验证JSON值
     * @param value 待验证的JSON值
     * @param schema 对应的Schema
     * @param path JSON路径
     * @param options 验证选项
     * @return 验证错误列表
     */
    QList<ValidationError> validateValue(const QJsonValue& value, const QJsonObject& schema, 
                                        const QString& path = QString(), 
                                        const ValidationOptions& options = ValidationOptions()) const;

    /**
     * @brief 快速验证（只返回是否通过）
     * @param json 待验证的JSON对象
     * @return 验证是否通过
     */
    bool isValid(const QJsonObject& json) const;

    /**
     * @brief 验证Schema本身
     * @param schema 待验证的Schema
     * @return 验证错误列表
     */
    QList<ValidationError> validateSchema(const QJsonObject& schema) const;

    // 格式验证器
    /**
     * @brief 添加自定义格式验证器
     * @param format 格式名称
     * @param validator 验证函数
     */
    void addFormatValidator(const QString& format, std::function<bool(const QString&)> validator);

    /**
     * @brief 移除格式验证器
     * @param format 格式名称
     */
    void removeFormatValidator(const QString& format);

    /**
     * @brief 获取支持的格式列表
     * @return 格式列表
     */
    QStringList supportedFormats() const;

    /**
     * @brief 验证格式
     * @param value 值
     * @param format 格式名称
     * @return 验证是否通过
     */
    bool validateFormat(const QString& value, const QString& format) const;

    // 引用解析
    /**
     * @brief 添加Schema引用
     * @param uri 引用URI
     * @param schema 引用的Schema
     */
    void addSchemaReference(const QString& uri, const QJsonObject& schema);

    /**
     * @brief 移除Schema引用
     * @param uri 引用URI
     */
    void removeSchemaReference(const QString& uri);

    /**
     * @brief 解析Schema引用
     * @param ref 引用字符串
     * @return 解析后的Schema
     */
    QJsonObject resolveReference(const QString& ref) const;

    /**
     * @brief 获取所有引用
     * @return 引用映射
     */
    QMap<QString, QJsonObject> schemaReferences() const;

    // 工具方法
    /**
     * @brief 生成Schema模板
     * @param json 示例JSON对象
     * @return 生成的Schema
     */
    QJsonObject generateSchema(const QJsonObject& json) const;

    /**
     * @brief 合并Schema
     * @param baseSchema 基础Schema
     * @param extensionSchema 扩展Schema
     * @return 合并后的Schema
     */
    QJsonObject mergeSchemas(const QJsonObject& baseSchema, const QJsonObject& extensionSchema) const;

    /**
     * @brief 简化Schema
     * @param schema 原始Schema
     * @return 简化后的Schema
     */
    QJsonObject simplifySchema(const QJsonObject& schema) const;

    /**
     * @brief 获取Schema统计信息
     * @param schema Schema对象
     * @return 统计信息
     */
    QVariantMap getSchemaStatistics(const QJsonObject& schema) const;

    /**
     * @brief 比较两个Schema
     * @param schema1 Schema 1
     * @param schema2 Schema 2
     * @return 差异列表
     */
    QStringList compareSchemas(const QJsonObject& schema1, const QJsonObject& schema2) const;

    // 错误处理
    /**
     * @brief 格式化验证错误
     * @param errors 错误列表
     * @return 格式化的错误信息
     */
    QString formatErrors(const QList<ValidationError>& errors) const;

    /**
     * @brief 将错误转换为JSON
     * @param errors 错误列表
     * @return JSON数组
     */
    QJsonArray errorsToJson(const QList<ValidationError>& errors) const;

    /**
     * @brief 从JSON加载错误
     * @param json JSON数组
     * @return 错误列表
     */
    QList<ValidationError> errorsFromJson(const QJsonArray& json) const;

    // 预定义Schema
    /**
     * @brief 获取基本类型Schema
     * @param type 类型名称
     * @return Schema对象
     */
    static QJsonObject getBasicTypeSchema(const QString& type);

    /**
     * @brief 获取常用格式Schema
     * @param format 格式名称
     * @return Schema对象
     */
    static QJsonObject getFormatSchema(const QString& format);

    /**
     * @brief 创建设置配置Schema
     * @return 设置配置Schema
     */
    static QJsonObject createSettingsConfigSchema();

    /**
     * @brief 创建偏好设置Schema
     * @return 偏好设置Schema
     */
    static QJsonObject createPreferencesSchema();

public slots:
    /**
     * @brief 异步验证
     * @param json 待验证的JSON对象
     */
    void validateAsync(const QJsonObject& json);

    /**
     * @brief 重新加载内置格式验证器
     */
    void reloadBuiltinValidators();

signals:
    /**
     * @brief Schema版本变化信号
     * @param version 新版本
     */
    void schemaVersionChanged(const QString& version);

    /**
     * @brief 严格模式变化信号
     * @param strict 是否严格
     */
    void strictModeChanged(bool strict);

    /**
     * @brief 最大错误数变化信号
     * @param max 最大错误数
     */
    void maxErrorsChanged(int max);

    /**
     * @brief Schema设置完成信号
     * @param success 是否成功
     */
    void schemaSet(bool success);

    /**
     * @brief 验证完成信号
     * @param errors 验证错误列表
     */
    void validationCompleted(const QList<ValidationError>& errors);

    /**
     * @brief 异步验证完成信号
     * @param json 验证的JSON对象
     * @param errors 验证错误列表
     */
    void asyncValidationCompleted(const QJsonObject& json, const QList<ValidationError>& errors);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private:
    // 核心验证方法
    QList<ValidationError> validateObject(const QJsonObject& object, const QJsonObject& schema, 
                                         const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validateArray(const QJsonArray& array, const QJsonObject& schema, 
                                        const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validateString(const QString& string, const QJsonObject& schema, 
                                         const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validateNumber(double number, const QJsonObject& schema, 
                                         const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validateBoolean(bool boolean, const QJsonObject& schema, 
                                          const QString& path, const ValidationOptions& options) const;

    // 约束验证
    QList<ValidationError> validateRequired(const QJsonObject& object, const QJsonArray& required, 
                                           const QString& path) const;
    QList<ValidationError> validateProperties(const QJsonObject& object, const QJsonObject& properties, 
                                             const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validateAdditionalProperties(const QJsonObject& object, const QJsonObject& schema, 
                                                       const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validatePatternProperties(const QJsonObject& object, const QJsonObject& patternProps, 
                                                    const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validateDependencies(const QJsonObject& object, const QJsonObject& dependencies, 
                                               const QString& path, const ValidationOptions& options) const;

    // 条件验证
    QList<ValidationError> validateIf(const QJsonValue& value, const QJsonObject& schema, 
                                     const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validateAllOf(const QJsonValue& value, const QJsonArray& allOf, 
                                        const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validateAnyOf(const QJsonValue& value, const QJsonArray& anyOf, 
                                        const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validateOneOf(const QJsonValue& value, const QJsonArray& oneOf, 
                                        const QString& path, const ValidationOptions& options) const;
    QList<ValidationError> validateNot(const QJsonValue& value, const QJsonObject& notSchema, 
                                      const QString& path, const ValidationOptions& options) const;

    // 辅助方法
    QString buildPath(const QString& basePath, const QString& key) const;
    QString buildPath(const QString& basePath, int index) const;
    bool isValidType(const QJsonValue& value, const QString& type) const;
    QStringList getValueTypes(const QJsonValue& value) const;
    void setupBuiltinFormatValidators();
    QString versionToString(SchemaVersion version) const;
    SchemaVersion stringToVersion(const QString& str) const;
    QString errorTypeToString(ErrorType type) const;
    ErrorType stringToErrorType(const QString& str) const;

    class Private;
    std::unique_ptr<Private> d;
};

// 便利宏定义
#define SCHEMA_ERROR(type, path, msg) SchemaValidator::ValidationError(type, path, msg)
#define SCHEMA_TYPE_ERROR(path, actual, expected) \
    SchemaValidator::ValidationError(SchemaValidator::TypeMismatch, path, \
    QString("Type mismatch: expected %1, got %2").arg(expected, actual))

#endif // SCHEMAVALIDATOR_H