#include "Validator.h"
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

// Static regex patterns
const QString Validator::EMAIL_PATTERN = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";
const QString Validator::URL_PATTERN = "^https?://[^\\s/$.?#].[^\\s]*$";
const QString Validator::IPV4_PATTERN = "^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
const QString Validator::IPV6_PATTERN = "^(?:[0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$";
const QString Validator::MAC_PATTERN = "^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$";
const QString Validator::PHONE_PATTERN = "^[+]?[1-9]\\d{1,14}$";
const QString Validator::CREDIT_CARD_PATTERN = "^[0-9]{13,19}$";
const QString Validator::SSN_PATTERN = "^\\d{3}-\\d{2}-\\d{4}$";
const QString Validator::USERNAME_PATTERN = "^[a-zA-Z0-9_]{3,20}$";
const QString Validator::FILENAME_PATTERN = "^[^<>:\"/\\\\|?*]+$";
const QString Validator::DOMAIN_PATTERN = "^[a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?\\.[a-zA-Z]{2,}$";

QHash<QString, QRegularExpression> Validator::s_regexCache;

Validator::Validator(QObject* parent)
    : QObject(parent)
{
}

bool Validator::isNull(const QString& str)
{
    return str.isNull();
}

bool Validator::isNotNull(const QString& str)
{
    return !str.isNull();
}

bool Validator::isEmpty(const QString& str)
{
    return str.isEmpty();
}

bool Validator::isNotEmpty(const QString& str)
{
    return !str.isEmpty();
}

bool Validator::isBlank(const QString& str)
{
    return str.trimmed().isEmpty();
}

bool Validator::isNotBlank(const QString& str)
{
    return !str.trimmed().isEmpty();
}

bool Validator::hasLength(const QString& str, int exactLength)
{
    return str.length() == exactLength;
}

bool Validator::hasMinLength(const QString& str, int minLength)
{
    return str.length() >= minLength;
}

bool Validator::hasMaxLength(const QString& str, int maxLength)
{
    return str.length() <= maxLength;
}

bool Validator::hasLengthBetween(const QString& str, int minLength, int maxLength)
{
    int len = str.length();
    return len >= minLength && len <= maxLength;
}

bool Validator::isInteger(const QString& str)
{
    bool ok;
    str.toInt(&ok);
    return ok;
}

bool Validator::isPositiveInteger(const QString& str)
{
    bool ok;
    int value = str.toInt(&ok);
    return ok && value > 0;
}

bool Validator::isNegativeInteger(const QString& str)
{
    bool ok;
    int value = str.toInt(&ok);
    return ok && value < 0;
}

bool Validator::isFloat(const QString& str)
{
    bool ok;
    str.toDouble(&ok);
    return ok;
}

bool Validator::isPositiveFloat(const QString& str)
{
    bool ok;
    double value = str.toDouble(&ok);
    return ok && value > 0.0;
}

bool Validator::isNegativeFloat(const QString& str)
{
    bool ok;
    double value = str.toDouble(&ok);
    return ok && value < 0.0;
}

bool Validator::isInRange(const QString& str, double min, double max)
{
    bool ok;
    double value = str.toDouble(&ok);
    return ok && value >= min && value <= max;
}

bool Validator::isAlpha(const QString& str)
{
    if (str.isEmpty()) {
        return false;
    }
    
    for (const QChar& ch : str) {
        if (!ch.isLetter()) {
            return false;
        }
    }
    
    return true;
}

bool Validator::isAlphaNumeric(const QString& str)
{
    if (str.isEmpty()) {
        return false;
    }
    
    for (const QChar& ch : str) {
        if (!ch.isLetterOrNumber()) {
            return false;
        }
    }
    
    return true;
}

bool Validator::isNumeric(const QString& str)
{
    if (str.isEmpty()) {
        return false;
    }
    
    for (const QChar& ch : str) {
        if (!ch.isDigit()) {
            return false;
        }
    }
    
    return true;
}

bool Validator::isHexadecimal(const QString& str)
{
    QRegularExpression hexRegex("^[0-9A-Fa-f]+$");
    return hexRegex.match(str).hasMatch();
}

bool Validator::isBase64(const QString& str)
{
    QRegularExpression base64Regex("^[A-Za-z0-9+/]*={0,2}$");
    return base64Regex.match(str).hasMatch() && (str.length() % 4 == 0);
}

Validator::ValidationResult Validator::validateEmail(const QString& email)
{
    if (email.isEmpty()) {
        return ValidationResult(false, "Email cannot be empty", "Please enter an email address");
    }
    
    if (!isValidEmailFormat(email)) {
        return ValidationResult(false, "Invalid email format", "Please enter a valid email address (e.g., user@example.com)");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateUrl(const QString& url)
{
    if (url.isEmpty()) {
        return ValidationResult(false, "URL cannot be empty", "Please enter a URL");
    }
    
    QUrl qurl(url);
    if (!qurl.isValid() || qurl.scheme().isEmpty()) {
        return ValidationResult(false, "Invalid URL format", "Please enter a valid URL (e.g., https://example.com)");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateIpAddress(const QString& ip)
{
    if (ip.isEmpty()) {
        return ValidationResult(false, "IP address cannot be empty", "Please enter an IP address");
    }
    
    QHostAddress address(ip);
    if (address.isNull()) {
        return ValidationResult(false, "Invalid IP address format", "Please enter a valid IPv4 or IPv6 address");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateMacAddress(const QString& mac)
{
    if (mac.isEmpty()) {
        return ValidationResult(false, "MAC address cannot be empty", "Please enter a MAC address");
    }
    
    if (!isValidMacFormat(mac)) {
        return ValidationResult(false, "Invalid MAC address format", "Please enter a valid MAC address (e.g., 00:11:22:33:44:55)");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validatePhoneNumber(const QString& phone, const QString& country)
{
    Q_UNUSED(country)
    
    if (phone.isEmpty()) {
        return ValidationResult(false, "Phone number cannot be empty", "Please enter a phone number");
    }
    
    QRegularExpression phoneRegex(PHONE_PATTERN);
    if (!phoneRegex.match(phone).hasMatch()) {
        return ValidationResult(false, "Invalid phone number format", "Please enter a valid phone number");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateCreditCard(const QString& cardNumber)
{
    if (cardNumber.isEmpty()) {
        return ValidationResult(false, "Credit card number cannot be empty", "Please enter a credit card number");
    }
    
    QString cleanNumber = cardNumber;
    cleanNumber.remove(QRegularExpression("[\\s-]"));
    
    if (!luhnCheck(cleanNumber)) {
        return ValidationResult(false, "Invalid credit card number", "Please enter a valid credit card number");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateSSN(const QString& ssn)
{
    if (ssn.isEmpty()) {
        return ValidationResult(false, "SSN cannot be empty", "Please enter a Social Security Number");
    }
    
    QRegularExpression ssnRegex(SSN_PATTERN);
    if (!ssnRegex.match(ssn).hasMatch()) {
        return ValidationResult(false, "Invalid SSN format", "Please enter SSN in format XXX-XX-XXXX");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validatePassport(const QString& passport, const QString& country)
{
    Q_UNUSED(country)
    
    if (passport.isEmpty()) {
        return ValidationResult(false, "Passport number cannot be empty", "Please enter a passport number");
    }
    
    if (passport.length() < 6 || passport.length() > 12) {
        return ValidationResult(false, "Invalid passport length", "Passport number should be 6-12 characters");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateDriverLicense(const QString& license, const QString& state)
{
    Q_UNUSED(state)
    
    if (license.isEmpty()) {
        return ValidationResult(false, "Driver license cannot be empty", "Please enter a driver license number");
    }
    
    if (license.length() < 5 || license.length() > 20) {
        return ValidationResult(false, "Invalid license length", "Driver license should be 5-20 characters");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validatePassword(const QString& password, int minLength)
{
    if (password.isEmpty()) {
        return ValidationResult(false, "Password cannot be empty", "Please enter a password");
    }
    
    if (password.length() < minLength) {
        return ValidationResult(false, QString("Password too short"), 
                              QString("Password must be at least %1 characters long").arg(minLength));
    }
    
    PasswordStrength strength = getPasswordStrength(password);
    if (strength == VeryWeak || strength == Weak) {
        QStringList requirements = getPasswordRequirements(password);
        return ValidationResult(false, "Password too weak", 
                              QString("Password should include: %1").arg(requirements.join(", ")));
    }
    
    return ValidationResult(true);
}

Validator::PasswordStrength Validator::getPasswordStrength(const QString& password)
{
    int score = 0;
    
    if (password.length() >= 8) score++;
    if (password.length() >= 12) score++;
    if (hasUpperCase(password)) score++;
    if (hasLowerCase(password)) score++;
    if (hasDigit(password)) score++;
    if (hasSpecialChar(password)) score++;
    
    switch (score) {
        case 0:
        case 1: return VeryWeak;
        case 2: return Weak;
        case 3: return Fair;
        case 4: return Good;
        case 5: return Strong;
        case 6: return VeryStrong;
        default: return VeryWeak;
    }
}

QStringList Validator::getPasswordRequirements(const QString& password)
{
    QStringList requirements;
    
    if (password.length() < 8) {
        requirements.append("at least 8 characters");
    }
    if (!hasUpperCase(password)) {
        requirements.append("uppercase letters");
    }
    if (!hasLowerCase(password)) {
        requirements.append("lowercase letters");
    }
    if (!hasDigit(password)) {
        requirements.append("numbers");
    }
    if (!hasSpecialChar(password)) {
        requirements.append("special characters");
    }
    
    return requirements;
}

bool Validator::hasUpperCase(const QString& str)
{
    for (const QChar& ch : str) {
        if (ch.isUpper()) {
            return true;
        }
    }
    return false;
}

bool Validator::hasLowerCase(const QString& str)
{
    for (const QChar& ch : str) {
        if (ch.isLower()) {
            return true;
        }
    }
    return false;
}

bool Validator::hasDigit(const QString& str)
{
    for (const QChar& ch : str) {
        if (ch.isDigit()) {
            return true;
        }
    }
    return false;
}

bool Validator::hasSpecialChar(const QString& str)
{
    QString specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    for (const QChar& ch : str) {
        if (specialChars.contains(ch)) {
            return true;
        }
    }
    return false;
}

Validator::ValidationResult Validator::validateDate(const QString& date, const QString& format)
{
    if (date.isEmpty()) {
        return ValidationResult(false, "Date cannot be empty", "Please enter a date");
    }
    
    QDateTime dateTime = QDateTime::fromString(date, format);
    if (!dateTime.isValid()) {
        return ValidationResult(false, "Invalid date format", 
                              QString("Please enter date in format: %1").arg(format));
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateTime(const QString& time, const QString& format)
{
    if (time.isEmpty()) {
        return ValidationResult(false, "Time cannot be empty", "Please enter a time");
    }
    
    QTime qtime = QTime::fromString(time, format);
    if (!qtime.isValid()) {
        return ValidationResult(false, "Invalid time format", 
                              QString("Please enter time in format: %1").arg(format));
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateDateTime(const QString& dateTime, const QString& format)
{
    if (dateTime.isEmpty()) {
        return ValidationResult(false, "DateTime cannot be empty", "Please enter a date and time");
    }
    
    QDateTime qdateTime = QDateTime::fromString(dateTime, format);
    if (!qdateTime.isValid()) {
        return ValidationResult(false, "Invalid datetime format", 
                              QString("Please enter datetime in format: %1").arg(format));
    }
    
    return ValidationResult(true);
}

bool Validator::isValidYear(int year)
{
    return year >= 1900 && year <= 2100;
}

bool Validator::isValidMonth(int month)
{
    return month >= 1 && month <= 12;
}

bool Validator::isValidDay(int day, int month, int year)
{
    if (day < 1 || day > 31) {
        return false;
    }
    
    QDate date(year, month, day);
    return date.isValid();
}

Validator::ValidationResult Validator::validateFileName(const QString& fileName)
{
    if (fileName.isEmpty()) {
        return ValidationResult(false, "Filename cannot be empty", "Please enter a filename");
    }
    
    QRegularExpression filenameRegex(FILENAME_PATTERN);
    if (!filenameRegex.match(fileName).hasMatch()) {
        return ValidationResult(false, "Invalid filename", "Filename contains invalid characters");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateFilePath(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return ValidationResult(false, "File path cannot be empty", "Please enter a file path");
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return ValidationResult(false, "File does not exist", "Please enter a valid file path");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateFileExtension(const QString& fileName, const QStringList& allowedExtensions)
{
    if (fileName.isEmpty()) {
        return ValidationResult(false, "Filename cannot be empty", "Please enter a filename");
    }
    
    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix().toLower();
    
    if (!allowedExtensions.contains(extension, Qt::CaseInsensitive)) {
        return ValidationResult(false, "Invalid file extension", 
                              QString("Allowed extensions: %1").arg(allowedExtensions.join(", ")));
    }
    
    return ValidationResult(true);
}

bool Validator::isValidFileSize(qint64 fileSize, qint64 maxSize)
{
    return fileSize > 0 && fileSize <= maxSize;
}

Validator::ValidationResult Validator::validateDomainName(const QString& domain)
{
    if (domain.isEmpty()) {
        return ValidationResult(false, "Domain name cannot be empty", "Please enter a domain name");
    }
    
    QRegularExpression domainRegex(DOMAIN_PATTERN);
    if (!domainRegex.match(domain).hasMatch()) {
        return ValidationResult(false, "Invalid domain name", "Please enter a valid domain name");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateHostname(const QString& hostname)
{
    if (hostname.isEmpty()) {
        return ValidationResult(false, "Hostname cannot be empty", "Please enter a hostname");
    }
    
    if (hostname.length() > 253) {
        return ValidationResult(false, "Hostname too long", "Hostname must be 253 characters or less");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validatePort(const QString& port)
{
    if (port.isEmpty()) {
        return ValidationResult(false, "Port cannot be empty", "Please enter a port number");
    }
    
    bool ok;
    int portNumber = port.toInt(&ok);
    
    if (!ok || portNumber < 1 || portNumber > 65535) {
        return ValidationResult(false, "Invalid port number", "Port must be between 1 and 65535");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateNetworkAddress(const QString& address)
{
    // Try to validate as IP address first, then as domain name
    ValidationResult ipResult = validateIpAddress(address);
    if (ipResult.isValid) {
        return ipResult;
    }
    
    return validateDomainName(address);
}

Validator::ValidationResult Validator::validateRegex(const QString& str, const QString& pattern, const QString& errorMessage)
{
    QRegularExpression regex(pattern);
    if (regex.match(str).hasMatch()) {
        return ValidationResult(true);
    }
    
    QString message = errorMessage.isEmpty() ? "String does not match required pattern" : errorMessage;
    return ValidationResult(false, message, "Please check the format");
}

Validator::ValidationResult Validator::validateCustom(const QString& str, std::function<bool(const QString&)> validator, const QString& errorMessage)
{
    if (validator(str)) {
        return ValidationResult(true);
    }
    
    QString message = errorMessage.isEmpty() ? "Validation failed" : errorMessage;
    return ValidationResult(false, message, "Please check the input");
}

QList<Validator::ValidationResult> Validator::validateBatch(const QStringList& values, std::function<ValidationResult(const QString&)> validator)
{
    QList<ValidationResult> results;
    
    for (const QString& value : values) {
        results.append(validator(value));
    }
    
    return results;
}

bool Validator::validateAll(const QStringList& values, std::function<ValidationResult(const QString&)> validator)
{
    for (const QString& value : values) {
        if (!validator(value).isValid) {
            return false;
        }
    }
    
    return true;
}

bool Validator::validateAny(const QStringList& values, std::function<ValidationResult(const QString&)> validator)
{
    for (const QString& value : values) {
        if (validator(value).isValid) {
            return true;
        }
    }
    
    return false;
}

Validator::ValidationResult Validator::validateMultiple(const QString& str, const QList<std::function<ValidationResult(const QString&)>>& validators)
{
    for (const auto& validator : validators) {
        ValidationResult result = validator(str);
        if (!result.isValid) {
            return result;
        }
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateEither(const QString& str, const QList<std::function<ValidationResult(const QString&)>>& validators)
{
    ValidationResult lastResult;
    
    for (const auto& validator : validators) {
        ValidationResult result = validator(str);
        if (result.isValid) {
            return result;
        }
        lastResult = result;
    }
    
    return lastResult;
}

Validator::ValidationResult Validator::validatePostalCode(const QString& postalCode, const QString& country)
{
    Q_UNUSED(country)
    
    if (postalCode.isEmpty()) {
        return ValidationResult(false, "Postal code cannot be empty", "Please enter a postal code");
    }
    
    // Simple validation - in a real implementation, you would have country-specific patterns
    if (postalCode.length() < 3 || postalCode.length() > 10) {
        return ValidationResult(false, "Invalid postal code length", "Postal code should be 3-10 characters");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateBankAccount(const QString& account, const QString& country)
{
    Q_UNUSED(country)
    
    if (account.isEmpty()) {
        return ValidationResult(false, "Bank account cannot be empty", "Please enter a bank account number");
    }
    
    if (account.length() < 8 || account.length() > 20) {
        return ValidationResult(false, "Invalid account length", "Bank account should be 8-20 characters");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateTaxId(const QString& taxId, const QString& country)
{
    Q_UNUSED(country)
    
    if (taxId.isEmpty()) {
        return ValidationResult(false, "Tax ID cannot be empty", "Please enter a tax ID");
    }
    
    if (taxId.length() < 5 || taxId.length() > 15) {
        return ValidationResult(false, "Invalid tax ID length", "Tax ID should be 5-15 characters");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateUsername(const QString& username)
{
    if (username.isEmpty()) {
        return ValidationResult(false, "Username cannot be empty", "Please enter a username");
    }
    
    QRegularExpression usernameRegex(USERNAME_PATTERN);
    if (!usernameRegex.match(username).hasMatch()) {
        return ValidationResult(false, "Invalid username format", "Username should be 3-20 characters, letters, numbers, and underscores only");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateDisplayName(const QString& displayName)
{
    if (displayName.isEmpty()) {
        return ValidationResult(false, "Display name cannot be empty", "Please enter a display name");
    }
    
    if (displayName.length() > 50) {
        return ValidationResult(false, "Display name too long", "Display name must be 50 characters or less");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateCompanyName(const QString& companyName)
{
    if (companyName.isEmpty()) {
        return ValidationResult(false, "Company name cannot be empty", "Please enter a company name");
    }
    
    if (companyName.length() > 100) {
        return ValidationResult(false, "Company name too long", "Company name must be 100 characters or less");
    }
    
    return ValidationResult(true);
}

Validator::ValidationResult Validator::validateProductCode(const QString& productCode)
{
    if (productCode.isEmpty()) {
        return ValidationResult(false, "Product code cannot be empty", "Please enter a product code");
    }
    
    if (productCode.length() < 3 || productCode.length() > 20) {
        return ValidationResult(false, "Invalid product code length", "Product code should be 3-20 characters");
    }
    
    return ValidationResult(true);
}

bool Validator::containsSqlInjection(const QString& str)
{
    QStringList sqlKeywords = {"SELECT", "INSERT", "UPDATE", "DELETE", "DROP", "UNION", "OR", "AND", "--", "/*", "*/"};
    
    QString upperStr = str.toUpper();
    for (const QString& keyword : sqlKeywords) {
        if (upperStr.contains(keyword)) {
            return true;
        }
    }
    
    return false;
}

bool Validator::containsXss(const QString& str)
{
    QStringList xssPatterns = {"<script", "</script>", "javascript:", "onload=", "onerror=", "onclick="};
    
    QString lowerStr = str.toLower();
    for (const QString& pattern : xssPatterns) {
        if (lowerStr.contains(pattern)) {
            return true;
        }
    }
    
    return false;
}

bool Validator::containsMaliciousCode(const QString& str)
{
    return containsSqlInjection(str) || containsXss(str);
}

Validator::ValidationResult Validator::validateSecureInput(const QString& str)
{
    if (containsMaliciousCode(str)) {
        return ValidationResult(false, "Input contains potentially malicious code", "Please remove any script or SQL code");
    }
    
    return ValidationResult(true);
}

QString Validator::sanitize(const QString& str)
{
    QString result = str;
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");
    result.replace("'", "&#39;");
    result.replace("&", "&amp;");
    return result;
}

QString Validator::escape(const QString& str)
{
    return sanitize(str);
}

QString Validator::normalize(const QString& str)
{
    return str.normalized(QString::NormalizationForm_C).trimmed();
}

QString Validator::generateSuggestion(const QString& str, const QString& pattern)
{
    Q_UNUSED(str)
    Q_UNUSED(pattern)
    
    // In a real implementation, this would analyze the input and suggest corrections
    return "Please check the format and try again";
}

bool Validator::isValidEmailFormat(const QString& email)
{
    QRegularExpression emailRegex(EMAIL_PATTERN);
    return emailRegex.match(email).hasMatch();
}

bool Validator::isValidUrlFormat(const QString& url)
{
    QUrl qurl(url);
    return qurl.isValid() && !qurl.scheme().isEmpty();
}

bool Validator::isValidIpv4(const QString& ip)
{
    QRegularExpression ipv4Regex(IPV4_PATTERN);
    return ipv4Regex.match(ip).hasMatch();
}

bool Validator::isValidIpv6(const QString& ip)
{
    QRegularExpression ipv6Regex(IPV6_PATTERN);
    return ipv6Regex.match(ip).hasMatch();
}

bool Validator::isValidMacFormat(const QString& mac)
{
    QRegularExpression macRegex(MAC_PATTERN);
    return macRegex.match(mac).hasMatch();
}

bool Validator::luhnCheck(const QString& cardNumber)
{
    int sum = 0;
    bool alternate = false;
    
    for (int i = cardNumber.length() - 1; i >= 0; --i) {
        int digit = cardNumber[i].digitValue();
        
        if (alternate) {
            digit *= 2;
            if (digit > 9) {
                digit = (digit % 10) + 1;
            }
        }
        
        sum += digit;
        alternate = !alternate;
    }
    
    return (sum % 10) == 0;
}

QString Validator::getCardType(const QString& cardNumber)
{
    if (cardNumber.startsWith("4")) {
        return "Visa";
    } else if (cardNumber.startsWith("5") || cardNumber.startsWith("2")) {
        return "MasterCard";
    } else if (cardNumber.startsWith("3")) {
        return "American Express";
    } else if (cardNumber.startsWith("6")) {
        return "Discover";
    }
    
    return "Unknown";
}

QRegularExpression Validator::getRegex(const QString& pattern)
{
    if (s_regexCache.contains(pattern)) {
        return s_regexCache[pattern];
    }
    
    QRegularExpression regex(pattern);
    s_regexCache[pattern] = regex;
    return regex;
}