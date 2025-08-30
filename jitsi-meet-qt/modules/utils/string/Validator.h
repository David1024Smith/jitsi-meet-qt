#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QObject>
#include <QString>
#include <QRegularExpression>
#include <QUrl>
#include <QHostAddress>

/**
 * @brief 数据验证器类
 * 
 * Validator提供各种数据验证功能，包括邮箱、URL、IP地址、
 * 电话号码、信用卡号等常见数据格式的验证。
 */
class Validator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 验证结果结构
     */
    struct ValidationResult {
        bool isValid;           ///< 是否有效
        QString errorMessage;   ///< 错误信息
        QString suggestion;     ///< 建议修正
        
        ValidationResult(bool valid = false, const QString& error = QString(), const QString& suggest = QString())
            : isValid(valid), errorMessage(error), suggestion(suggest) {}
    };

    /**
     * @brief 密码强度枚举
     */
    enum PasswordStrength {
        VeryWeak,       ///< 非常弱
        Weak,           ///< 弱
        Fair,           ///< 一般
        Good,           ///< 好
        Strong,         ///< 强
        VeryStrong      ///< 非常强
    };
    Q_ENUM(PasswordStrength)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit Validator(QObject* parent = nullptr);

    // 基础验证
    static bool isNull(const QString& str);
    static bool isNotNull(const QString& str);
    static bool isEmpty(const QString& str);
    static bool isNotEmpty(const QString& str);
    static bool isBlank(const QString& str);
    static bool isNotBlank(const QString& str);
    
    // 长度验证
    static bool hasLength(const QString& str, int exactLength);
    static bool hasMinLength(const QString& str, int minLength);
    static bool hasMaxLength(const QString& str, int maxLength);
    static bool hasLengthBetween(const QString& str, int minLength, int maxLength);
    
    // 数字验证
    static bool isInteger(const QString& str);
    static bool isPositiveInteger(const QString& str);
    static bool isNegativeInteger(const QString& str);
    static bool isFloat(const QString& str);
    static bool isPositiveFloat(const QString& str);
    static bool isNegativeFloat(const QString& str);
    static bool isInRange(const QString& str, double min, double max);
    
    // 字符类型验证
    static bool isAlpha(const QString& str);
    static bool isAlphaNumeric(const QString& str);
    static bool isNumeric(const QString& str);
    static bool isHexadecimal(const QString& str);
    static bool isBase64(const QString& str);
    
    // 格式验证
    static ValidationResult validateEmail(const QString& email);
    static ValidationResult validateUrl(const QString& url);
    static ValidationResult validateIpAddress(const QString& ip);
    static ValidationResult validateMacAddress(const QString& mac);
    static ValidationResult validatePhoneNumber(const QString& phone, const QString& country = QString());
    
    // 身份验证
    static ValidationResult validateCreditCard(const QString& cardNumber);
    static ValidationResult validateSSN(const QString& ssn);
    static ValidationResult validatePassport(const QString& passport, const QString& country = QString());
    static ValidationResult validateDriverLicense(const QString& license, const QString& state = QString());
    
    // 密码验证
    static ValidationResult validatePassword(const QString& password, int minLength = 8);
    static PasswordStrength getPasswordStrength(const QString& password);
    static QStringList getPasswordRequirements(const QString& password);
    static bool hasUpperCase(const QString& str);
    static bool hasLowerCase(const QString& str);
    static bool hasDigit(const QString& str);
    static bool hasSpecialChar(const QString& str);
    
    // 日期时间验证
    static ValidationResult validateDate(const QString& date, const QString& format = "yyyy-MM-dd");
    static ValidationResult validateTime(const QString& time, const QString& format = "hh:mm:ss");
    static ValidationResult validateDateTime(const QString& dateTime, const QString& format = "yyyy-MM-dd hh:mm:ss");
    static bool isValidYear(int year);
    static bool isValidMonth(int month);
    static bool isValidDay(int day, int month, int year);
    
    // 文件验证
    static ValidationResult validateFileName(const QString& fileName);
    static ValidationResult validateFilePath(const QString& filePath);
    static ValidationResult validateFileExtension(const QString& fileName, const QStringList& allowedExtensions);
    static bool isValidFileSize(qint64 fileSize, qint64 maxSize);
    
    // 网络验证
    static ValidationResult validateDomainName(const QString& domain);
    static ValidationResult validateHostname(const QString& hostname);
    static ValidationResult validatePort(const QString& port);
    static ValidationResult validateNetworkAddress(const QString& address);
    
    // 自定义验证
    static ValidationResult validateRegex(const QString& str, const QString& pattern, const QString& errorMessage = QString());
    static ValidationResult validateCustom(const QString& str, std::function<bool(const QString&)> validator, const QString& errorMessage = QString());
    
    // 批量验证
    static QList<ValidationResult> validateBatch(const QStringList& values, std::function<ValidationResult(const QString&)> validator);
    static bool validateAll(const QStringList& values, std::function<ValidationResult(const QString&)> validator);
    static bool validateAny(const QStringList& values, std::function<ValidationResult(const QString&)> validator);
    
    // 验证规则组合
    static ValidationResult validateMultiple(const QString& str, const QList<std::function<ValidationResult(const QString&)>>& validators);
    static ValidationResult validateEither(const QString& str, const QList<std::function<ValidationResult(const QString&)>>& validators);
    
    // 国际化验证
    static ValidationResult validatePostalCode(const QString& postalCode, const QString& country);
    static ValidationResult validateBankAccount(const QString& account, const QString& country);
    static ValidationResult validateTaxId(const QString& taxId, const QString& country);
    
    // 业务逻辑验证
    static ValidationResult validateUsername(const QString& username);
    static ValidationResult validateDisplayName(const QString& displayName);
    static ValidationResult validateCompanyName(const QString& companyName);
    static ValidationResult validateProductCode(const QString& productCode);
    
    // 安全验证
    static bool containsSqlInjection(const QString& str);
    static bool containsXss(const QString& str);
    static bool containsMaliciousCode(const QString& str);
    static ValidationResult validateSecureInput(const QString& str);
    
    // 工具方法
    static QString sanitize(const QString& str);
    static QString escape(const QString& str);
    static QString normalize(const QString& str);
    static QString generateSuggestion(const QString& str, const QString& pattern);

private:
    // 内部验证方法
    static bool isValidEmailFormat(const QString& email);
    static bool isValidUrlFormat(const QString& url);
    static bool isValidIpv4(const QString& ip);
    static bool isValidIpv6(const QString& ip);
    static bool isValidMacFormat(const QString& mac);
    
    // 信用卡验证算法
    static bool luhnCheck(const QString& cardNumber);
    static QString getCardType(const QString& cardNumber);
    
    // 正则表达式缓存
    static QHash<QString, QRegularExpression> s_regexCache;
    static QRegularExpression getRegex(const QString& pattern);
    
    // 常用正则表达式
    static const QString EMAIL_PATTERN;
    static const QString URL_PATTERN;
    static const QString IPV4_PATTERN;
    static const QString IPV6_PATTERN;
    static const QString MAC_PATTERN;
    static const QString PHONE_PATTERN;
    static const QString CREDIT_CARD_PATTERN;
    static const QString SSN_PATTERN;
    static const QString USERNAME_PATTERN;
    static const QString FILENAME_PATTERN;
    static const QString DOMAIN_PATTERN;
};

// 声明元类型以支持信号槽
Q_DECLARE_METATYPE(Validator::ValidationResult)
Q_DECLARE_METATYPE(Validator::PasswordStrength)

#endif // VALIDATOR_H