#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QStringConverter>
#include <QLocale>

/**
 * @brief 字符串工具类
 * 
 * StringUtils提供各种字符串处理功能，包括格式化、验证、
 * 转换、编码和本地化等实用函数。
 */
class StringUtils : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 字符串编码枚举
     */
    enum Encoding {
        UTF8,           ///< UTF-8编码
        UTF16,          ///< UTF-16编码
        Latin1,         ///< Latin-1编码
        ASCII,          ///< ASCII编码
        Local8Bit       ///< 本地8位编码
    };
    Q_ENUM(Encoding)

    /**
     * @brief 大小写转换模式枚举
     */
    enum CaseMode {
        Lower,          ///< 小写
        Upper,          ///< 大写
        Title,          ///< 标题格式
        Camel,          ///< 驼峰格式
        Pascal,         ///< 帕斯卡格式
        Snake,          ///< 蛇形格式
        Kebab           ///< 短横线格式
    };
    Q_ENUM(CaseMode)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit StringUtils(QObject* parent = nullptr);

    // 基础字符串操作
    static QString trim(const QString& str);
    static QString trimLeft(const QString& str);
    static QString trimRight(const QString& str);
    static QString trimAll(const QString& str);
    
    // 大小写转换
    static QString toCase(const QString& str, CaseMode mode);
    static QString toCamelCase(const QString& str);
    static QString toPascalCase(const QString& str);
    static QString toSnakeCase(const QString& str);
    static QString toKebabCase(const QString& str);
    
    // 字符串验证
    static bool isEmpty(const QString& str);
    static bool isBlank(const QString& str);
    static bool isNumeric(const QString& str);
    static bool isAlpha(const QString& str);
    static bool isAlphaNumeric(const QString& str);
    
    // 字符串比较
    static bool equals(const QString& str1, const QString& str2, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    static bool startsWith(const QString& str, const QString& prefix, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    static bool endsWith(const QString& str, const QString& suffix, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    static bool contains(const QString& str, const QString& substring, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    
    // 字符串搜索和替换
    static int indexOf(const QString& str, const QString& substring, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    static int lastIndexOf(const QString& str, const QString& substring, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    static QString replace(const QString& str, const QString& before, const QString& after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    static QString replaceAll(const QString& str, const QStringList& patterns, const QStringList& replacements);
    
    // 字符串分割和连接
    static QStringList split(const QString& str, const QString& separator, Qt::SplitBehavior behavior = Qt::KeepEmptyParts);
    static QStringList splitByRegex(const QString& str, const QRegularExpression& regex);
    static QString join(const QStringList& list, const QString& separator);
    static QString joinWithAnd(const QStringList& list, const QString& separator = ", ", const QString& lastSeparator = " and ");
    
    // 字符串格式化
    static QString format(const QString& format, const QVariantList& args);
    static QString sprintf(const QString& format, ...);
    static QString leftPad(const QString& str, int width, QChar fillChar = ' ');
    static QString rightPad(const QString& str, int width, QChar fillChar = ' ');
    static QString center(const QString& str, int width, QChar fillChar = ' ');
    
    // 字符串截取和省略
    static QString left(const QString& str, int length);
    static QString right(const QString& str, int length);
    static QString mid(const QString& str, int position, int length = -1);
    static QString truncate(const QString& str, int maxLength, const QString& suffix = "...");
    static QString ellipsis(const QString& str, int maxLength, const QString& ellipsisStr = "...");
    
    // 编码转换
    static QByteArray toBytes(const QString& str, Encoding encoding = UTF8);
    static QString fromBytes(const QByteArray& bytes, Encoding encoding = UTF8);
    static QString toHex(const QString& str);
    static QString fromHex(const QString& hexStr);
    static QString toBase64(const QString& str);
    static QString fromBase64(const QString& base64Str);
    
    // URL编码
    static QString urlEncode(const QString& str);
    static QString urlDecode(const QString& str);
    static QString htmlEncode(const QString& str);
    static QString htmlDecode(const QString& str);
    
    // 正则表达式
    static bool matches(const QString& str, const QString& pattern);
    static QStringList findAll(const QString& str, const QString& pattern);
    static QString extract(const QString& str, const QString& pattern, int group = 0);
    static QStringList extractAll(const QString& str, const QString& pattern, int group = 0);
    
    // 字符串生成
    static QString random(int length, const QString& charset = QString());
    static QString randomAlpha(int length);
    static QString randomNumeric(int length);
    static QString randomAlphaNumeric(int length);
    static QString uuid();
    
    // 字符串度量
    static int length(const QString& str);
    static int byteLength(const QString& str, Encoding encoding = UTF8);
    static int wordCount(const QString& str);
    static int lineCount(const QString& str);
    
    // 字符串清理
    static QString removeWhitespace(const QString& str);
    static QString removeNonPrintable(const QString& str);
    static QString removeAccents(const QString& str);
    static QString normalize(const QString& str);
    
    // 本地化支持
    static QString localize(const QString& str, const QLocale& locale = QLocale());
    static QString formatNumber(double number, int precision = -1, const QLocale& locale = QLocale());
    static QString formatCurrency(double amount, const QString& currency, const QLocale& locale = QLocale());
    static QString formatDateTime(const QDateTime& dateTime, const QString& format = QString(), const QLocale& locale = QLocale());
    
private:
    static QString generateRandomString(int length, const QString& charset);
    static QRegularExpression createRegex(const QString& pattern, bool caseSensitive = true);
};

#endif // STRINGUTILS_H