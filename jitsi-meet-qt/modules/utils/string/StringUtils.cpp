#include "StringUtils.h"
#include <QRandomGenerator>
#include <QUuid>
#include <QUrl>
#include <QStringConverter>
#include <QDateTime>
#include <QDebug>

StringUtils::StringUtils(QObject* parent)
    : QObject(parent)
{
}

QString StringUtils::trim(const QString& str)
{
    return str.trimmed();
}

QString StringUtils::trimLeft(const QString& str)
{
    int start = 0;
    while (start < str.length() && str[start].isSpace()) {
        ++start;
    }
    return str.mid(start);
}

QString StringUtils::trimRight(const QString& str)
{
    int end = str.length() - 1;
    while (end >= 0 && str[end].isSpace()) {
        --end;
    }
    return str.left(end + 1);
}

QString StringUtils::trimAll(const QString& str)
{
    QString result = str;
    result.replace(QRegularExpression("\\s+"), " ");
    return result.trimmed();
}

QString StringUtils::toCase(const QString& str, CaseMode mode)
{
    switch (mode) {
        case Lower:
            return str.toLower();
        case Upper:
            return str.toUpper();
        case Title:
            return str.toLower().replace(0, 1, str[0].toUpper());
        case Camel:
            return toCamelCase(str);
        case Pascal:
            return toPascalCase(str);
        case Snake:
            return toSnakeCase(str);
        case Kebab:
            return toKebabCase(str);
        default:
            return str;
    }
}

QString StringUtils::toCamelCase(const QString& str)
{
    QStringList words = str.split(QRegularExpression("[\\s_-]+"), Qt::SkipEmptyParts);
    if (words.isEmpty()) {
        return QString();
    }
    
    QString result = words[0].toLower();
    for (int i = 1; i < words.size(); ++i) {
        if (!words[i].isEmpty()) {
            result += words[i][0].toUpper() + words[i].mid(1).toLower();
        }
    }
    
    return result;
}

QString StringUtils::toPascalCase(const QString& str)
{
    QStringList words = str.split(QRegularExpression("[\\s_-]+"), Qt::SkipEmptyParts);
    QString result;
    
    for (const QString& word : words) {
        if (!word.isEmpty()) {
            result += word[0].toUpper() + word.mid(1).toLower();
        }
    }
    
    return result;
}

QString StringUtils::toSnakeCase(const QString& str)
{
    QString result = str;
    result.replace(QRegularExpression("([a-z])([A-Z])"), "\\1_\\2");
    result.replace(QRegularExpression("[\\s-]+"), "_");
    return result.toLower();
}

QString StringUtils::toKebabCase(const QString& str)
{
    QString result = str;
    result.replace(QRegularExpression("([a-z])([A-Z])"), "\\1-\\2");
    result.replace(QRegularExpression("[\\s_]+"), "-");
    return result.toLower();
}

bool StringUtils::isEmpty(const QString& str)
{
    return str.isEmpty();
}

bool StringUtils::isBlank(const QString& str)
{
    return str.trimmed().isEmpty();
}

bool StringUtils::isNumeric(const QString& str)
{
    if (str.isEmpty()) {
        return false;
    }
    
    bool ok;
    str.toDouble(&ok);
    return ok;
}

bool StringUtils::isAlpha(const QString& str)
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

bool StringUtils::isAlphaNumeric(const QString& str)
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

bool StringUtils::equals(const QString& str1, const QString& str2, Qt::CaseSensitivity cs)
{
    return str1.compare(str2, cs) == 0;
}

bool StringUtils::startsWith(const QString& str, const QString& prefix, Qt::CaseSensitivity cs)
{
    return str.startsWith(prefix, cs);
}

bool StringUtils::endsWith(const QString& str, const QString& suffix, Qt::CaseSensitivity cs)
{
    return str.endsWith(suffix, cs);
}

bool StringUtils::contains(const QString& str, const QString& substring, Qt::CaseSensitivity cs)
{
    return str.contains(substring, cs);
}

int StringUtils::indexOf(const QString& str, const QString& substring, int from, Qt::CaseSensitivity cs)
{
    return str.indexOf(substring, from, cs);
}

int StringUtils::lastIndexOf(const QString& str, const QString& substring, int from, Qt::CaseSensitivity cs)
{
    return str.lastIndexOf(substring, from, cs);
}

QString StringUtils::replace(const QString& str, const QString& before, const QString& after, Qt::CaseSensitivity cs)
{
    QString result = str;
    return result.replace(before, after, cs);
}

QString StringUtils::replaceAll(const QString& str, const QStringList& patterns, const QStringList& replacements)
{
    QString result = str;
    
    int minSize = qMin(patterns.size(), replacements.size());
    for (int i = 0; i < minSize; ++i) {
        result.replace(patterns[i], replacements[i]);
    }
    
    return result;
}

QStringList StringUtils::split(const QString& str, const QString& separator, Qt::SplitBehavior behavior)
{
    return str.split(separator, behavior);
}

QStringList StringUtils::splitByRegex(const QString& str, const QRegularExpression& regex)
{
    return str.split(regex);
}

QString StringUtils::join(const QStringList& list, const QString& separator)
{
    return list.join(separator);
}

QString StringUtils::joinWithAnd(const QStringList& list, const QString& separator, const QString& lastSeparator)
{
    if (list.isEmpty()) {
        return QString();
    }
    
    if (list.size() == 1) {
        return list[0];
    }
    
    if (list.size() == 2) {
        return list[0] + lastSeparator + list[1];
    }
    
    QStringList firstPart = list.mid(0, list.size() - 1);
    return firstPart.join(separator) + lastSeparator + list.last();
}

QString StringUtils::format(const QString& format, const QVariantList& args)
{
    QString result = format;
    
    for (int i = 0; i < args.size(); ++i) {
        QString placeholder = QString("{%1}").arg(i);
        result.replace(placeholder, args[i].toString());
    }
    
    return result;
}

QString StringUtils::sprintf(const QString& format, ...)
{
    // This is a simplified version - in a real implementation,
    // you would use va_list to handle variable arguments
    return format;
}

QString StringUtils::leftPad(const QString& str, int width, QChar fillChar)
{
    return str.rightJustified(width, fillChar);
}

QString StringUtils::rightPad(const QString& str, int width, QChar fillChar)
{
    return str.leftJustified(width, fillChar);
}

QString StringUtils::center(const QString& str, int width, QChar fillChar)
{
    if (str.length() >= width) {
        return str;
    }
    
    int padding = width - str.length();
    int leftPadding = padding / 2;
    int rightPadding = padding - leftPadding;
    
    return QString(leftPadding, fillChar) + str + QString(rightPadding, fillChar);
}

QString StringUtils::left(const QString& str, int length)
{
    return str.left(length);
}

QString StringUtils::right(const QString& str, int length)
{
    return str.right(length);
}

QString StringUtils::mid(const QString& str, int position, int length)
{
    return str.mid(position, length);
}

QString StringUtils::truncate(const QString& str, int maxLength, const QString& suffix)
{
    if (str.length() <= maxLength) {
        return str;
    }
    
    return str.left(maxLength - suffix.length()) + suffix;
}

QString StringUtils::ellipsis(const QString& str, int maxLength, const QString& ellipsisStr)
{
    return truncate(str, maxLength, ellipsisStr);
}

QByteArray StringUtils::toBytes(const QString& str, Encoding encoding)
{
    switch (encoding) {
        case UTF8:
            return str.toUtf8();
        case UTF16:
            return QByteArray(reinterpret_cast<const char*>(str.utf16()), str.length() * 2);
        case Latin1:
            return str.toLatin1();
        case ASCII:
            return str.toLocal8Bit(); // Fallback to local 8-bit
        case Local8Bit:
            return str.toLocal8Bit();
        default:
            return str.toUtf8();
    }
}

QString StringUtils::fromBytes(const QByteArray& bytes, Encoding encoding)
{
    switch (encoding) {
        case UTF8:
            return QString::fromUtf8(bytes);
        case UTF16:
            return QString::fromUtf16(reinterpret_cast<const ushort*>(bytes.constData()), bytes.length() / 2);
        case Latin1:
            return QString::fromLatin1(bytes);
        case ASCII:
            return QString::fromLocal8Bit(bytes); // Fallback to local 8-bit
        case Local8Bit:
            return QString::fromLocal8Bit(bytes);
        default:
            return QString::fromUtf8(bytes);
    }
}

QString StringUtils::toHex(const QString& str)
{
    return str.toUtf8().toHex();
}

QString StringUtils::fromHex(const QString& hexStr)
{
    return QString::fromUtf8(QByteArray::fromHex(hexStr.toUtf8()));
}

QString StringUtils::toBase64(const QString& str)
{
    return str.toUtf8().toBase64();
}

QString StringUtils::fromBase64(const QString& base64Str)
{
    return QString::fromUtf8(QByteArray::fromBase64(base64Str.toUtf8()));
}

QString StringUtils::urlEncode(const QString& str)
{
    return QUrl::toPercentEncoding(str);
}

QString StringUtils::urlDecode(const QString& str)
{
    return QUrl::fromPercentEncoding(str.toUtf8());
}

QString StringUtils::htmlEncode(const QString& str)
{
    QString result = str;
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");
    result.replace("'", "&#39;");
    return result;
}

QString StringUtils::htmlDecode(const QString& str)
{
    QString result = str;
    result.replace("&amp;", "&");
    result.replace("&lt;", "<");
    result.replace("&gt;", ">");
    result.replace("&quot;", "\"");
    result.replace("&#39;", "'");
    return result;
}

bool StringUtils::matches(const QString& str, const QString& pattern)
{
    QRegularExpression regex(pattern);
    return regex.match(str).hasMatch();
}

QStringList StringUtils::findAll(const QString& str, const QString& pattern)
{
    QStringList results;
    QRegularExpression regex(pattern);
    QRegularExpressionMatchIterator iterator = regex.globalMatch(str);
    
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        results.append(match.captured(0));
    }
    
    return results;
}

QString StringUtils::extract(const QString& str, const QString& pattern, int group)
{
    QRegularExpression regex(pattern);
    QRegularExpressionMatch match = regex.match(str);
    
    if (match.hasMatch() && group < match.capturedTexts().size()) {
        return match.captured(group);
    }
    
    return QString();
}

QStringList StringUtils::extractAll(const QString& str, const QString& pattern, int group)
{
    QStringList results;
    QRegularExpression regex(pattern);
    QRegularExpressionMatchIterator iterator = regex.globalMatch(str);
    
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        if (group < match.capturedTexts().size()) {
            results.append(match.captured(group));
        }
    }
    
    return results;
}

QString StringUtils::random(int length, const QString& charset)
{
    QString defaultCharset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString actualCharset = charset.isEmpty() ? defaultCharset : charset;
    
    return generateRandomString(length, actualCharset);
}

QString StringUtils::randomAlpha(int length)
{
    return generateRandomString(length, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
}

QString StringUtils::randomNumeric(int length)
{
    return generateRandomString(length, "0123456789");
}

QString StringUtils::randomAlphaNumeric(int length)
{
    return generateRandomString(length, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
}

QString StringUtils::uuid()
{
    return QUuid::createUuid().toString();
}

int StringUtils::length(const QString& str)
{
    return str.length();
}

int StringUtils::byteLength(const QString& str, Encoding encoding)
{
    return toBytes(str, encoding).length();
}

int StringUtils::wordCount(const QString& str)
{
    return str.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
}

int StringUtils::lineCount(const QString& str)
{
    return str.split('\n').size();
}

QString StringUtils::removeWhitespace(const QString& str)
{
    QString result = str;
    result.remove(QRegularExpression("\\s"));
    return result;
}

QString StringUtils::removeNonPrintable(const QString& str)
{
    QString result;
    for (const QChar& ch : str) {
        if (ch.isPrint()) {
            result.append(ch);
        }
    }
    return result;
}

QString StringUtils::removeAccents(const QString& str)
{
    QString result = str;
    
    // Simple accent removal - in a real implementation,
    // you would use Unicode normalization
    result.replace("á", "a").replace("à", "a").replace("ä", "a").replace("â", "a");
    result.replace("é", "e").replace("è", "e").replace("ë", "e").replace("ê", "e");
    result.replace("í", "i").replace("ì", "i").replace("ï", "i").replace("î", "i");
    result.replace("ó", "o").replace("ò", "o").replace("ö", "o").replace("ô", "o");
    result.replace("ú", "u").replace("ù", "u").replace("ü", "u").replace("û", "u");
    
    return result;
}

QString StringUtils::normalize(const QString& str)
{
    return str.normalized(QString::NormalizationForm_C);
}

QString StringUtils::localize(const QString& str, const QLocale& locale)
{
    Q_UNUSED(locale)
    // In a real implementation, this would perform localization
    return str;
}

QString StringUtils::formatNumber(double number, int precision, const QLocale& locale)
{
    return locale.toString(number, 'f', precision);
}

QString StringUtils::formatCurrency(double amount, const QString& currency, const QLocale& locale)
{
    return locale.toCurrencyString(amount, currency);
}

QString StringUtils::formatDateTime(const QDateTime& dateTime, const QString& format, const QLocale& locale)
{
    if (format.isEmpty()) {
        return locale.toString(dateTime);
    }
    
    return dateTime.toString(format);
}

QString StringUtils::generateRandomString(int length, const QString& charset)
{
    QString result;
    result.reserve(length);
    
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(charset.length());
        result.append(charset[index]);
    }
    
    return result;
}

QRegularExpression StringUtils::createRegex(const QString& pattern, bool caseSensitive)
{
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (!caseSensitive) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }
    
    return QRegularExpression(pattern, options);
}