#ifndef HASHUTILS_H
#define HASHUTILS_H

#include "../interfaces/ICryptoHandler.h"
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QCryptographicHash>
#include <QMutex>

/**
 * @brief 哈希工具类
 * 
 * HashUtils提供各种哈希算法的计算功能，包括MD5、SHA系列算法、
 * HMAC计算以及文件哈希等实用功能。
 */
class HashUtils : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 哈希算法枚举（扩展版本）
     */
    enum HashAlgorithm {
        MD4,            ///< MD4哈希
        MD5,            ///< MD5哈希
        SHA1,           ///< SHA-1哈希
        SHA224,         ///< SHA-224哈希
        SHA256,         ///< SHA-256哈希
        SHA384,         ///< SHA-384哈希
        SHA512,         ///< SHA-512哈希
        SHA3_224,       ///< SHA3-224哈希
        SHA3_256,       ///< SHA3-256哈希
        SHA3_384,       ///< SHA3-384哈希
        SHA3_512,       ///< SHA3-512哈希
        BLAKE2B,        ///< BLAKE2b哈希
        BLAKE2S         ///< BLAKE2s哈希
    };
    Q_ENUM(HashAlgorithm)

    /**
     * @brief 哈希结果结构
     */
    struct HashResult {
        QByteArray hash;            ///< 哈希值
        HashAlgorithm algorithm;    ///< 使用的算法
        QString hexString;          ///< 十六进制字符串
        QString base64String;       ///< Base64字符串
        qint64 processingTime;      ///< 处理时间（毫秒）
        
        HashResult() : algorithm(SHA256), processingTime(0) {}
        
        bool isValid() const {
            return !hash.isEmpty();
        }
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit HashUtils(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~HashUtils();

    /**
     * @brief 计算数据哈希值
     * @param data 输入数据
     * @param algorithm 哈希算法
     * @return 哈希结果
     */
    static HashResult hash(const QByteArray& data, HashAlgorithm algorithm = SHA256);

    /**
     * @brief 计算字符串哈希值
     * @param text 输入字符串
     * @param algorithm 哈希算法
     * @param encoding 字符串编码
     * @return 哈希结果
     */
    static HashResult hash(const QString& text, HashAlgorithm algorithm = SHA256, 
                          const QString& encoding = "UTF-8");

    /**
     * @brief 计算文件哈希值
     * @param filePath 文件路径
     * @param algorithm 哈希算法
     * @return 哈希结果
     */
    static HashResult hashFile(const QString& filePath, HashAlgorithm algorithm = SHA256);

    /**
     * @brief 计算HMAC值
     * @param data 输入数据
     * @param key HMAC密钥
     * @param algorithm 哈希算法
     * @return 哈希结果
     */
    static HashResult hmac(const QByteArray& data, const QByteArray& key, 
                          HashAlgorithm algorithm = SHA256);

    /**
     * @brief 计算字符串HMAC值
     * @param text 输入字符串
     * @param key HMAC密钥字符串
     * @param algorithm 哈希算法
     * @return 哈希结果
     */
    static HashResult hmac(const QString& text, const QString& key,
                          HashAlgorithm algorithm = SHA256);

    /**
     * @brief 验证哈希值
     * @param data 原始数据
     * @param expectedHash 期望的哈希值
     * @param algorithm 哈希算法
     * @return 验证是否成功
     */
    static bool verify(const QByteArray& data, const QByteArray& expectedHash,
                      HashAlgorithm algorithm = SHA256);

    /**
     * @brief 验证文件哈希值
     * @param filePath 文件路径
     * @param expectedHash 期望的哈希值
     * @param algorithm 哈希算法
     * @return 验证是否成功
     */
    static bool verifyFile(const QString& filePath, const QByteArray& expectedHash,
                          HashAlgorithm algorithm = SHA256);

    /**
     * @brief 批量计算哈希值
     * @param dataList 数据列表
     * @param algorithm 哈希算法
     * @return 哈希结果列表
     */
    static QList<HashResult> hashBatch(const QList<QByteArray>& dataList,
                                      HashAlgorithm algorithm = SHA256);

    /**
     * @brief 批量计算文件哈希值
     * @param filePaths 文件路径列表
     * @param algorithm 哈希算法
     * @return 哈希结果列表
     */
    static QList<HashResult> hashFilesBatch(const QStringList& filePaths,
                                           HashAlgorithm algorithm = SHA256);

    /**
     * @brief 生成密码哈希（带盐值）
     * @param password 密码
     * @param salt 盐值（为空则自动生成）
     * @param iterations 迭代次数
     * @param algorithm 哈希算法
     * @return 哈希结果（包含盐值）
     */
    static HashResult hashPassword(const QString& password, const QByteArray& salt = QByteArray(),
                                  int iterations = 10000, HashAlgorithm algorithm = SHA256);

    /**
     * @brief 验证密码哈希
     * @param password 密码
     * @param hashedPassword 哈希后的密码
     * @param salt 盐值
     * @param iterations 迭代次数
     * @param algorithm 哈希算法
     * @return 验证是否成功
     */
    static bool verifyPassword(const QString& password, const QByteArray& hashedPassword,
                              const QByteArray& salt, int iterations = 10000,
                              HashAlgorithm algorithm = SHA256);

    /**
     * @brief 生成随机盐值
     * @param length 盐值长度
     * @return 随机盐值
     */
    static QByteArray generateSalt(int length = 32);

    /**
     * @brief 计算校验和
     * @param data 输入数据
     * @param algorithm 哈希算法
     * @return 校验和字符串
     */
    static QString checksum(const QByteArray& data, HashAlgorithm algorithm = SHA256);

    /**
     * @brief 计算文件校验和
     * @param filePath 文件路径
     * @param algorithm 哈希算法
     * @return 校验和字符串
     */
    static QString checksumFile(const QString& filePath, HashAlgorithm algorithm = SHA256);

    /**
     * @brief 获取支持的哈希算法列表
     * @return 支持的算法列表
     */
    static QList<HashAlgorithm> supportedAlgorithms();

    /**
     * @brief 检查算法是否支持
     * @param algorithm 哈希算法
     * @return 是否支持
     */
    static bool isAlgorithmSupported(HashAlgorithm algorithm);

    /**
     * @brief 获取哈希算法的输出长度
     * @param algorithm 哈希算法
     * @return 输出长度（字节）
     */
    static int getHashLength(HashAlgorithm algorithm);

    /**
     * @brief 哈希算法转字符串
     * @param algorithm 哈希算法
     * @return 算法字符串
     */
    static QString algorithmToString(HashAlgorithm algorithm);

    /**
     * @brief 字符串转哈希算法
     * @param algorithmStr 算法字符串
     * @return 哈希算法
     */
    static HashAlgorithm stringToAlgorithm(const QString& algorithmStr);

    /**
     * @brief 转换为Qt哈希算法
     * @param algorithm 自定义哈希算法
     * @return Qt哈希算法
     */
    static QCryptographicHash::Algorithm toQtAlgorithm(HashAlgorithm algorithm);

    /**
     * @brief 从Qt哈希算法转换
     * @param qtAlgorithm Qt哈希算法
     * @return 自定义哈希算法
     */
    static HashAlgorithm fromQtAlgorithm(QCryptographicHash::Algorithm qtAlgorithm);

    /**
     * @brief 比较两个哈希值（防时序攻击）
     * @param hash1 哈希值1
     * @param hash2 哈希值2
     * @return 是否相等
     */
    static bool secureCompare(const QByteArray& hash1, const QByteArray& hash2);

    /**
     * @brief 格式化哈希值为可读字符串
     * @param hash 哈希值
     * @param format 格式（hex, base64, base32）
     * @param uppercase 是否大写（仅hex格式）
     * @return 格式化字符串
     */
    static QString formatHash(const QByteArray& hash, const QString& format = "hex", 
                             bool uppercase = false);

signals:
    /**
     * @brief 哈希计算进度信号
     * @param bytesProcessed 已处理字节数
     * @param totalBytes 总字节数
     */
    void progressUpdated(qint64 bytesProcessed, qint64 totalBytes);

    /**
     * @brief 哈希计算完成信号
     * @param result 哈希结果
     */
    void hashCompleted(const HashResult& result);

private:
    /**
     * @brief 执行哈希计算核心操作
     * @param data 输入数据
     * @param algorithm 哈希算法
     * @return 哈希结果
     */
    static HashResult performHash(const QByteArray& data, HashAlgorithm algorithm);

    /**
     * @brief 执行HMAC计算核心操作
     * @param data 输入数据
     * @param key HMAC密钥
     * @param algorithm 哈希算法
     * @return 哈希结果
     */
    static HashResult performHMAC(const QByteArray& data, const QByteArray& key,
                                 HashAlgorithm algorithm);

    /**
     * @brief 执行PBKDF2密钥派生
     * @param password 密码
     * @param salt 盐值
     * @param iterations 迭代次数
     * @param keyLength 密钥长度
     * @param algorithm 哈希算法
     * @return 派生的密钥
     */
    static QByteArray performPBKDF2(const QString& password, const QByteArray& salt,
                                   int iterations, int keyLength, HashAlgorithm algorithm);

private:
    static QMutex s_mutex;              ///< 静态互斥锁
};

// 声明元类型以支持信号槽
Q_DECLARE_METATYPE(HashUtils::HashAlgorithm)
Q_DECLARE_METATYPE(HashUtils::HashResult)

#endif // HASHUTILS_H