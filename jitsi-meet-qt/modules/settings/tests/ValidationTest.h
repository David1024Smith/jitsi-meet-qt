#ifndef VALIDATIONTEST_H
#define VALIDATIONTEST_H

#include <QObject>
#include <QTest>
#include <QElapsedTimer>

/**
 * @brief 验证测试类
 * 
 * 专门测试配置验证功能，包括ConfigValidator和SchemaValidator。
 * 测试各种验证规则、自定义验证、条件验证和性能等方面。
 */
class ValidationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // ConfigValidator tests
    void testConfigValidatorBasicRules();
    void testConfigValidatorCustomRules();
    void testConfigValidatorConditionalRules();
    
    // SchemaValidator tests
    void testSchemaValidatorBasicTypes();
    void testSchemaValidatorNestedObjects();
    void testSchemaValidatorArrayValidation();
    
    // SettingsConfig tests
    void testSettingsConfigValidation();
    void testSettingsConfigDefaults();
    
    // Performance and error reporting tests
    void testValidationPerformance();
    void testValidationErrorReporting();
};

#endif // VALIDATIONTEST_H