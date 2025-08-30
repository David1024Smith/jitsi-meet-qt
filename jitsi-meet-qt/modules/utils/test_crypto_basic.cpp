#include <QCoreApplication>
#include <QDebug>
#include "string/StringUtils.h"
#include "string/Validator.h"
#include "crypto/HashUtils.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Testing Utils Module - Crypto and String Processing";
    
    // Test StringUtils
    qDebug() << "\n=== StringUtils Tests ===";
    QString testStr = "  Hello World  ";
    qDebug() << "Original:" << testStr;
    qDebug() << "Trimmed:" << StringUtils::trim(testStr);
    qDebug() << "CamelCase:" << StringUtils::toCamelCase("hello world test");
    qDebug() << "SnakeCase:" << StringUtils::toSnakeCase("HelloWorldTest");
    
    // Test basic validation
    qDebug() << "\n=== Basic Validation ===";
    qDebug() << "Is numeric '123':" << StringUtils::isNumeric("123");
    qDebug() << "Is alpha 'abc':" << StringUtils::isAlpha("abc");
    qDebug() << "Is alphanumeric 'abc123':" << StringUtils::isAlphaNumeric("abc123");
    
    // Test Validator
    qDebug() << "\n=== Validator Tests ===";
    Validator::ValidationResult emailResult = Validator::validateEmail("test@example.com");
    qDebug() << "Email validation (test@example.com):" << emailResult.isValid;
    
    emailResult = Validator::validateEmail("invalid-email");
    qDebug() << "Email validation (invalid-email):" << emailResult.isValid;
    qDebug() << "Error message:" << emailResult.errorMessage;
    
    // Test password validation
    Validator::ValidationResult passResult = Validator::validatePassword("StrongP@ssw0rd123");
    qDebug() << "Password validation (strong):" << passResult.isValid;
    
    passResult = Validator::validatePassword("weak");
    qDebug() << "Password validation (weak):" << passResult.isValid;
    qDebug() << "Error message:" << passResult.errorMessage;
    
    // Test HashUtils (basic Qt functionality)
    qDebug() << "\n=== HashUtils Tests ===";
    QString testData = "Hello, Hash World!";
    HashUtils::HashResult hashResult = HashUtils::hash(testData.toUtf8(), HashUtils::SHA256);
    qDebug() << "Hash valid:" << hashResult.isValid();
    qDebug() << "Hash (hex):" << hashResult.hexString;
    qDebug() << "Hash algorithm:" << HashUtils::algorithmToString(hashResult.algorithm);
    
    // Test verification
    bool verified = HashUtils::verify(testData.toUtf8(), hashResult.hash, HashUtils::SHA256);
    qDebug() << "Hash verification:" << verified;
    
    qDebug() << "\n=== All basic tests completed ===";
    
    return 0;
}