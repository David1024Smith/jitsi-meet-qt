#include "ValidationTest.h"
#include "ConfigValidator.h"
#include "SchemaValidator.h"
#include "SettingsConfig.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

void ValidationTest::initTestCase()
{
    // Setup test environment
}

void ValidationTest::cleanupTestCase()
{
    // Cleanup test environment
}

void ValidationTest::init()
{
    // Setup for each test
}

void ValidationTest::cleanup()
{
    // Cleanup after each test
}

void ValidationTest::testConfigValidatorBasicRules()
{
    ConfigValidator validator;
    
    // Test integer range validation
    validator.addRule("audio/volume", ConfigValidator::IntegerRange, QVariantList() << 0 << 100);
    validator.addRule("network/port", ConfigValidator::IntegerRange, QVariantList() << 1024 << 65535);
    
    // Test string pattern validation
    validator.addRule("video/resolution", ConfigValidator::StringPattern, "\\d+x\\d+");
    validator.addRule("network/url", ConfigValidator::StringPattern, "https?://.*");
    
    // Test boolean validation
    validator.addRule("audio/enabled", ConfigValidator::Boolean);
    validator.addRule("video/enabled", ConfigValidator::Boolean);
    
    // Test valid configuration
    QVariantMap validConfig;
    validConfig["audio/volume"] = 75;
    validConfig["audio/enabled"] = true;
    validConfig["video/resolution"] = "1920x1080";
    validConfig["video/enabled"] = true;
    validConfig["network/port"] = 8080;
    validConfig["network/url"] = "https://meet.jit.si";
    
    QVERIFY(validator.validate(validConfig));
    QVERIFY(validator.lastErrors().isEmpty());
    
    // Test invalid configuration
    QVariantMap invalidConfig;
    invalidConfig["audio/volume"] = 150; // Out of range
    invalidConfig["audio/enabled"] = "not_boolean"; // Wrong type
    invalidConfig["video/resolution"] = "invalid_format"; // Wrong pattern
    invalidConfig["network/port"] = 80; // Out of range
    invalidConfig["network/url"] = "ftp://invalid.protocol"; // Wrong pattern
    
    QVERIFY(!validator.validate(invalidConfig));
    
    QStringList errors = validator.lastErrors();
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("audio/volume"));
    QVERIFY(errors.join(" ").contains("audio/enabled"));
    QVERIFY(errors.join(" ").contains("video/resolution"));
    QVERIFY(errors.join(" ").contains("network/port"));
    QVERIFY(errors.join(" ").contains("network/url"));
}

void ValidationTest::testConfigValidatorCustomRules()
{
    ConfigValidator validator;
    
    // Add custom validation function
    validator.addCustomRule("custom/email", [](const QVariant& value) -> bool {
        QString email = value.toString();
        QRegularExpression emailRegex("^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$");
        return emailRegex.match(email).hasMatch();
    });
    
    validator.addCustomRule("custom/password", [](const QVariant& value) -> bool {
        QString password = value.toString();
        return password.length() >= 8 && 
               password.contains(QRegularExpression("[A-Z]")) &&
               password.contains(QRegularExpression("[a-z]")) &&
               password.contains(QRegularExpression("[0-9]"));
    });
    
    // Test valid custom values
    QVariantMap validConfig;
    validConfig["custom/email"] = "user@example.com";
    validConfig["custom/password"] = "SecurePass123";
    
    QVERIFY(validator.validate(validConfig));
    
    // Test invalid custom values
    QVariantMap invalidConfig;
    invalidConfig["custom/email"] = "invalid.email";
    invalidConfig["custom/password"] = "weak";
    
    QVERIFY(!validator.validate(invalidConfig));
    
    QStringList errors = validator.lastErrors();
    QVERIFY(errors.join(" ").contains("custom/email"));
    QVERIFY(errors.join(" ").contains("custom/password"));
}

void ValidationTest::testConfigValidatorConditionalRules()
{
    ConfigValidator validator;
    
    // Add conditional validation
    validator.addConditionalRule("video/bitrate", "video/enabled", true, 
                                ConfigValidator::IntegerRange, QVariantList() << 100 << 10000);
    
    validator.addConditionalRule("audio/samplerate", "audio/enabled", true,
                                ConfigValidator::IntegerRange, QVariantList() << 8000 << 48000);
    
    // Test with conditions met
    QVariantMap configWithConditions;
    configWithConditions["video/enabled"] = true;
    configWithConditions["video/bitrate"] = 2000;
    configWithConditions["audio/enabled"] = true;
    configWithConditions["audio/samplerate"] = 44100;
    
    QVERIFY(validator.validate(configWithConditions));
    
    // Test with conditions not met (should skip validation)
    QVariantMap configWithoutConditions;
    configWithoutConditions["video/enabled"] = false;
    configWithoutConditions["video/bitrate"] = 50000; // Would be invalid if video enabled
    configWithoutConditions["audio/enabled"] = false;
    configWithoutConditions["audio/samplerate"] = 100000; // Would be invalid if audio enabled
    
    QVERIFY(validator.validate(configWithoutConditions));
    
    // Test with conditions met but invalid values
    QVariantMap invalidConditionalConfig;
    invalidConditionalConfig["video/enabled"] = true;
    invalidConditionalConfig["video/bitrate"] = 50; // Too low
    invalidConditionalConfig["audio/enabled"] = true;
    invalidConditionalConfig["audio/samplerate"] = 100000; // Too high
    
    QVERIFY(!validator.validate(invalidConditionalConfig));
}

void ValidationTest::testSchemaValidatorBasicTypes()
{
    SchemaValidator validator;
    
    // Create basic type schema
    QJsonObject schema;
    schema["type"] = "object";
    
    QJsonObject properties;
    
    // String property
    QJsonObject stringProp;
    stringProp["type"] = "string";
    stringProp["minLength"] = 1;
    stringProp["maxLength"] = 50;
    properties["name"] = stringProp;
    
    // Integer property
    QJsonObject intProp;
    intProp["type"] = "integer";
    intProp["minimum"] = 0;
    intProp["maximum"] = 100;
    properties["volume"] = intProp;
    
    // Boolean property
    QJsonObject boolProp;
    boolProp["type"] = "boolean";
    properties["enabled"] = boolProp;
    
    // Array property
    QJsonObject arrayProp;
    arrayProp["type"] = "array";
    QJsonObject arrayItems;
    arrayItems["type"] = "string";
    arrayProp["items"] = arrayItems;
    arrayProp["minItems"] = 1;
    arrayProp["maxItems"] = 5;
    properties["tags"] = arrayProp;
    
    schema["properties"] = properties;
    schema["required"] = QJsonArray({"name", "enabled"});
    
    QVERIFY(validator.setSchema(schema));
    
    // Test valid data
    QJsonObject validData;
    validData["name"] = "Test Name";
    validData["volume"] = 75;
    validData["enabled"] = true;
    validData["tags"] = QJsonArray({"tag1", "tag2"});
    
    QVERIFY(validator.validate(validData));
    
    // Test invalid data
    QJsonObject invalidData;
    invalidData["name"] = ""; // Too short
    invalidData["volume"] = 150; // Out of range
    invalidData["enabled"] = "not_boolean"; // Wrong type
    invalidData["tags"] = QJsonArray(); // Too few items
    
    QVERIFY(!validator.validate(invalidData));
    
    QStringList errors = validator.lastErrors();
    QVERIFY(!errors.isEmpty());
}

void ValidationTest::testSchemaValidatorNestedObjects()
{
    SchemaValidator validator;
    
    // Create nested object schema
    QJsonObject schema;
    schema["type"] = "object";
    
    QJsonObject properties;
    
    // Nested audio object
    QJsonObject audioProp;
    audioProp["type"] = "object";
    QJsonObject audioProperties;
    
    QJsonObject volumeProp;
    volumeProp["type"] = "integer";
    volumeProp["minimum"] = 0;
    volumeProp["maximum"] = 100;
    audioProperties["volume"] = volumeProp;
    
    QJsonObject mutedProp;
    mutedProp["type"] = "boolean";
    audioProperties["muted"] = mutedProp;
    
    audioProp["properties"] = audioProperties;
    audioProp["required"] = QJsonArray({"volume"});
    properties["audio"] = audioProp;
    
    // Nested video object
    QJsonObject videoProp;
    videoProp["type"] = "object";
    QJsonObject videoProperties;
    
    QJsonObject resolutionProp;
    resolutionProp["type"] = "string";
    resolutionProp["pattern"] = "\\d+x\\d+";
    videoProperties["resolution"] = resolutionProp;
    
    QJsonObject fpsProp;
    fpsProp["type"] = "integer";
    fpsProp["minimum"] = 1;
    fpsProp["maximum"] = 60;
    videoProperties["fps"] = fpsProp;
    
    videoProp["properties"] = videoProperties;
    properties["video"] = videoProp;
    
    schema["properties"] = properties;
    
    QVERIFY(validator.setSchema(schema));
    
    // Test valid nested data
    QJsonObject validData;
    QJsonObject audioData;
    audioData["volume"] = 75;
    audioData["muted"] = false;
    validData["audio"] = audioData;
    
    QJsonObject videoData;
    videoData["resolution"] = "1920x1080";
    videoData["fps"] = 30;
    validData["video"] = videoData;
    
    QVERIFY(validator.validate(validData));
    
    // Test invalid nested data
    QJsonObject invalidData;
    QJsonObject invalidAudioData;
    invalidAudioData["volume"] = 150; // Out of range
    invalidAudioData["muted"] = "not_boolean"; // Wrong type
    invalidData["audio"] = invalidAudioData;
    
    QJsonObject invalidVideoData;
    invalidVideoData["resolution"] = "invalid_format"; // Wrong pattern
    invalidVideoData["fps"] = 100; // Out of range
    invalidData["video"] = invalidVideoData;
    
    QVERIFY(!validator.validate(invalidData));
}

void ValidationTest::testSchemaValidatorArrayValidation()
{
    SchemaValidator validator;
    
    // Create array schema
    QJsonObject schema;
    schema["type"] = "object";
    
    QJsonObject properties;
    
    // Array of objects
    QJsonObject devicesProp;
    devicesProp["type"] = "array";
    
    QJsonObject deviceSchema;
    deviceSchema["type"] = "object";
    QJsonObject deviceProperties;
    
    QJsonObject idProp;
    idProp["type"] = "string";
    deviceProperties["id"] = idProp;
    
    QJsonObject nameProp;
    nameProp["type"] = "string";
    deviceProperties["name"] = nameProp;
    
    deviceSchema["properties"] = deviceProperties;
    deviceSchema["required"] = QJsonArray({"id", "name"});
    
    devicesProp["items"] = deviceSchema;
    devicesProp["minItems"] = 1;
    properties["devices"] = devicesProp;
    
    schema["properties"] = properties;
    
    QVERIFY(validator.setSchema(schema));
    
    // Test valid array data
    QJsonObject validData;
    QJsonArray devicesArray;
    
    QJsonObject device1;
    device1["id"] = "device1";
    device1["name"] = "Device 1";
    devicesArray.append(device1);
    
    QJsonObject device2;
    device2["id"] = "device2";
    device2["name"] = "Device 2";
    devicesArray.append(device2);
    
    validData["devices"] = devicesArray;
    
    QVERIFY(validator.validate(validData));
    
    // Test invalid array data
    QJsonObject invalidData;
    QJsonArray invalidDevicesArray;
    
    QJsonObject invalidDevice;
    invalidDevice["id"] = "device1";
    // Missing required "name" property
    invalidDevicesArray.append(invalidDevice);
    
    invalidData["devices"] = invalidDevicesArray;
    
    QVERIFY(!validator.validate(invalidData));
}

void ValidationTest::testSettingsConfigValidation()
{
    SettingsConfig config;
    
    // Test default configuration validation
    QVERIFY(config.validate());
    
    // Test setting valid values
    config.setValue("audio/volume", 75);
    config.setValue("audio/enabled", true);
    config.setValue("video/resolution", "1920x1080");
    config.setValue("network/server", "https://meet.jit.si");
    
    QVERIFY(config.validate());
    
    // Test setting invalid values
    config.setValue("audio/volume", -10); // Invalid range
    QVERIFY(!config.validate());
    
    QStringList errors = config.validationErrors();
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("audio/volume"));
    
    // Fix the error and test again
    config.setValue("audio/volume", 50);
    QVERIFY(config.validate());
}

void ValidationTest::testSettingsConfigDefaults()
{
    SettingsConfig config;
    
    // Test default values
    QVERIFY(config.hasDefaultValue("audio/volume"));
    QCOMPARE(config.defaultValue("audio/volume").toInt(), 50);
    
    QVERIFY(config.hasDefaultValue("video/enabled"));
    QCOMPARE(config.defaultValue("video/enabled").toBool(), true);
    
    QVERIFY(config.hasDefaultValue("network/server"));
    QCOMPARE(config.defaultValue("network/server").toString(), QString("https://meet.jit.si"));
    
    // Test applying defaults
    QVariantMap emptyConfig;
    config.fromVariantMap(emptyConfig);
    config.applyDefaults();
    
    QCOMPARE(config.value("audio/volume").toInt(), 50);
    QCOMPARE(config.value("video/enabled").toBool(), true);
    QCOMPARE(config.value("network/server").toString(), QString("https://meet.jit.si"));
    
    QVERIFY(config.validate());
}

void ValidationTest::testValidationPerformance()
{
    ConfigValidator validator;
    
    // Add many validation rules
    for (int i = 0; i < 100; ++i) {
        validator.addRule(QString("test/key_%1").arg(i), ConfigValidator::IntegerRange, 
                         QVariantList() << 0 << 1000);
    }
    
    // Create large configuration
    QVariantMap largeConfig;
    for (int i = 0; i < 100; ++i) {
        largeConfig[QString("test/key_%1").arg(i)] = i * 10;
    }
    
    // Measure validation performance
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < 100; ++i) {
        QVERIFY(validator.validate(largeConfig));
    }
    
    qint64 validationTime = timer.elapsed();
    QVERIFY(validationTime < 5000); // Should complete within 5 seconds
    
    qDebug() << "Validation performance: 100 validations of 100 rules took" << validationTime << "ms";
}

void ValidationTest::testValidationErrorReporting()
{
    ConfigValidator validator;
    
    validator.addRule("test/range", ConfigValidator::IntegerRange, QVariantList() << 1 << 10);
    validator.addRule("test/pattern", ConfigValidator::StringPattern, "^prefix_.*");
    validator.addRule("test/required", ConfigValidator::Required);
    
    // Create configuration with multiple errors
    QVariantMap invalidConfig;
    invalidConfig["test/range"] = 15; // Out of range
    invalidConfig["test/pattern"] = "invalid_pattern"; // Wrong pattern
    // Missing required field
    
    QVERIFY(!validator.validate(invalidConfig));
    
    QStringList errors = validator.lastErrors();
    QVERIFY(errors.size() >= 3);
    
    // Check error messages contain relevant information
    QString allErrors = errors.join(" ");
    QVERIFY(allErrors.contains("test/range"));
    QVERIFY(allErrors.contains("test/pattern"));
    QVERIFY(allErrors.contains("test/required"));
    
    // Test detailed error information
    QVariantMap errorDetails = validator.lastErrorDetails();
    QVERIFY(errorDetails.contains("test/range"));
    QVERIFY(errorDetails.contains("test/pattern"));
    QVERIFY(errorDetails.contains("test/required"));
}

QTEST_MAIN(ValidationTest)