#ifndef ICRYPTOHANDLER_H
#define ICRYPTOHANDLER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QStringList>

/**
 * @brief 加密处理器接口
 * 
 * ICryptoHandler定义了加密处理器的标准接口，支持对称加密、
 * 非对称加密、哈希计算和数字签名等密码学操作。
 */
class ICryptoHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 加密算法枚举
     */
    enum Algorithm {
        AES_128,        ///< AES 128位
        AES_192,        ///< AES 192位
        AES_256,        ///< AES 256位
        RSA_1024,       ///< RSA 1024位
        RSA_2048,       ///< RSA 2048位
        RSA_4096        ///< RSA 4096位
    };
    Q_ENUM(Algorithm)

    /**
     * @brief 哈希算法枚举
     */
    enum HashAlgorithm {
        MD5,            ///< MD5哈希
        SHA1,           ///< SHA-1哈希
        SHA224,         ///< SHA-224哈希
        SHA256,         ///< SHA-256哈希
        SHA384,         ///< SHA-384哈希
        SHA512          ///< SHA-512哈希
    };
    Q_ENUM(HashAlgorithm)

    /**
     * @brief 加密模式枚举
     */
    enum Mode {
        ECB,            ///< 电子密码本模式
        CBC,            ///< 密码块链接模式
        CFB,            ///< 密码反馈模式
        OFB,            ///< 输出反馈模式
        GCM             ///< 伽罗瓦/计数器模式
    };
    Q_ENUM(Mode)

    /**
     * @brief 填充模式枚举
     */
    enum Padding {
        NoPadding,      ///< 无填充
        PKCS7,          ///< PKCS#7填充
        ISO10126,       ///< ISO 10126填充
        ANSI_X923       ///< ANSI X9.23填充
    };
    Q_ENUM(Padding)

    /**
     * @brief 操作结果枚举
     */
    enum OperationResult {
        Success,            ///< 操作成功
        InvalidKey,         ///< 无效密钥
        InvalidData,        ///< 无效数据
        AlgorithmError,     ///< 算法错误
        InsufficientData,   ///< 数据不足
        UnknownError        ///< 未知错误
    };
    Q_ENUM(OperationResult)

    /**
     * @brief 密钥对结构
     */
    struct KeyPair {
        QByteArray publicKey;   ///< 公钥
        QByteArray privateKey;  ///< 私钥
        Algorithm algorithm;    ///< 算法类型
        
        KeyPair() : algorithm(RSA_2048) {}
        
        bool isValid() const {
            return !publicKey.isEmpty() && !privateKey.isEmpty();
        }
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ICryptoHandler(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief 虚析构函数
     */
    virtual ~ICryptoHandler() = default;

    /**
     * @brief 初始化加密处理器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 清理加密处理器
     */
    virtual void cleanup() = 0;

    /**
     * @brief 对称加密
     * @param data 原始数据
     * @param key 加密密钥
     * @param algorithm 加密算法
     * @param mode 加密模式
     * @param padding 填充模式
     * @param result 输出加密数据
     * @return 操作结果
     */
    virtual OperationResult encrypt(const QByteArray& data, const QByteArray& key, 
                                   Algorithm algorithm, Mode mode, Padding padding,
                                   QByteArray& result) = 0;

    /**
     * @brief 对称解密
     * @param data 加密数据
     * @param key 解密密钥
     * @param algorithm 加密算法
     * @param mode 加密模式
     * @param padding 填充模式
     * @param result 输出原始数据
     * @return 操作结果
     */
    virtual OperationResult decrypt(const QByteArray& data, const QByteArray& key,
                                   Algorithm algorithm, Mode mode, Padding padding,
                                   QByteArray& result) = 0;

    /**
     * @brief 非对称加密（使用公钥）
     * @param data 原始数据
     * @param publicKey 公钥
     * @param algorithm 加密算法
     * @param result 输出加密数据
     * @return 操作结果
     */
    virtual OperationResult encryptAsymmetric(const QByteArray& data, const QByteArray& publicKey,
                                             Algorithm algorithm, QByteArray& result) = 0;

    /**
     * @brief 非对称解密（使用私钥）
     * @param data 加密数据
     * @param privateKey 私钥
     * @param algorithm 加密算法
     * @param result 输出原始数据
     * @return 操作结果
     */
    virtual OperationResult decryptAsymmetric(const QByteArray& data, const QByteArray& privateKey,
                                             Algorithm algorithm, QByteArray& result) = 0;

    /**
     * @brief 计算哈希值
     * @param data 输入数据
     * @param algorithm 哈希算法
     * @param result 输出哈希值
     * @return 操作结果
     */
    virtual OperationResult hash(const QByteArray& data, HashAlgorithm algorithm, QByteArray& result) = 0;

    /**
     * @brief 计算HMAC
     * @param data 输入数据
     * @param key HMAC密钥
     * @param algorithm 哈希算法
     * @param result 输出HMAC值
     * @return 操作结果
     */
    virtual OperationResult hmac(const QByteArray& data, const QByteArray& key,
                                HashAlgorithm algorithm, QByteArray& result) = 0;

    /**
     * @brief 生成密钥对
     * @param algorithm 算法类型
     * @param keyPair 输出密钥对
     * @return 操作结果
     */
    virtual OperationResult generateKeyPair(Algorithm algorithm, KeyPair& keyPair) = 0;

    /**
     * @brief 生成随机密钥
     * @param length 密钥长度（字节）
     * @param key 输出密钥
     * @return 操作结果
     */
    virtual OperationResult generateRandomKey(int length, QByteArray& key) = 0;

    /**
     * @brief 数字签名
     * @param data 待签名数据
     * @param privateKey 私钥
     * @param algorithm 签名算法
     * @param signature 输出签名
     * @return 操作结果
     */
    virtual OperationResult sign(const QByteArray& data, const QByteArray& privateKey,
                                Algorithm algorithm, QByteArray& signature) = 0;

    /**
     * @brief 验证数字签名
     * @param data 原始数据
     * @param signature 数字签名
     * @param publicKey 公钥
     * @param algorithm 签名算法
     * @return 验证是否成功
     */
    virtual bool verify(const QByteArray& data, const QByteArray& signature,
                       const QByteArray& publicKey, Algorithm algorithm) = 0;

    /**
     * @brief 获取支持的算法列表
     * @return 支持的算法列表
     */
    virtual QList<Algorithm> supportedAlgorithms() const = 0;

    /**
     * @brief 获取支持的哈希算法列表
     * @return 支持的哈希算法列表
     */
    virtual QList<HashAlgorithm> supportedHashAlgorithms() const = 0;

    /**
     * @brief 检查算法是否支持
     * @param algorithm 算法类型
     * @return 是否支持
     */
    virtual bool isAlgorithmSupported(Algorithm algorithm) const = 0;

    /**
     * @brief 检查哈希算法是否支持
     * @param algorithm 哈希算法
     * @return 是否支持
     */
    virtual bool isHashAlgorithmSupported(HashAlgorithm algorithm) const = 0;

    /**
     * @brief 获取处理器名称
     * @return 处理器名称
     */
    virtual QString name() const = 0;

    /**
     * @brief 获取处理器版本
     * @return 处理器版本
     */
    virtual QString version() const = 0;

    /**
     * @brief 算法转字符串
     * @param algorithm 算法类型
     * @return 算法字符串
     */
    static QString algorithmToString(Algorithm algorithm) {
        switch (algorithm) {
            case AES_128: return "AES-128";
            case AES_192: return "AES-192";
            case AES_256: return "AES-256";
            case RSA_1024: return "RSA-1024";
            case RSA_2048: return "RSA-2048";
            case RSA_4096: return "RSA-4096";
            default: return "Unknown";
        }
    }

    /**
     * @brief 哈希算法转字符串
     * @param algorithm 哈希算法
     * @return 算法字符串
     */
    static QString hashAlgorithmToString(HashAlgorithm algorithm) {
        switch (algorithm) {
            case MD5: return "MD5";
            case SHA1: return "SHA-1";
            case SHA224: return "SHA-224";
            case SHA256: return "SHA-256";
            case SHA384: return "SHA-384";
            case SHA512: return "SHA-512";
            default: return "Unknown";
        }
    }

    /**
     * @brief 操作结果转字符串
     * @param result 操作结果
     * @return 结果字符串
     */
    static QString resultToString(OperationResult result) {
        switch (result) {
            case Success: return "Success";
            case InvalidKey: return "Invalid key";
            case InvalidData: return "Invalid data";
            case AlgorithmError: return "Algorithm error";
            case InsufficientData: return "Insufficient data";
            case UnknownError: return "Unknown error";
            default: return "Unknown result";
        }
    }

signals:
    /**
     * @brief 加密操作完成信号
     * @param operation 操作类型
     * @param result 操作结果
     */
    void operationCompleted(const QString& operation, OperationResult result);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 进度更新信号（用于大数据加密）
     * @param bytesProcessed 已处理字节数
     * @param totalBytes 总字节数
     */
    void progressUpdated(qint64 bytesProcessed, qint64 totalBytes);

protected:
    /**
     * @brief 验证密钥长度
     * @param key 密钥
     * @param algorithm 算法类型
     * @return 密钥是否有效
     */
    virtual bool validateKeyLength(const QByteArray& key, Algorithm algorithm) const {
        switch (algorithm) {
            case AES_128: return key.size() == 16;
            case AES_192: return key.size() == 24;
            case AES_256: return key.size() == 32;
            case RSA_1024:
            case RSA_2048:
            case RSA_4096: return !key.isEmpty(); // RSA密钥长度验证更复杂
            default: return false;
        }
    }

    /**
     * @brief 发出操作完成信号
     * @param operation 操作类型
     * @param result 操作结果
     */
    void emitOperationCompleted(const QString& operation, OperationResult result) {
        emit operationCompleted(operation, result);
    }
};

// 声明元类型以支持信号槽
Q_DECLARE_METATYPE(ICryptoHandler::Algorithm)
Q_DECLARE_METATYPE(ICryptoHandler::HashAlgorithm)
Q_DECLARE_METATYPE(ICryptoHandler::Mode)
Q_DECLARE_METATYPE(ICryptoHandler::Padding)
Q_DECLARE_METATYPE(ICryptoHandler::OperationResult)
Q_DECLARE_METATYPE(ICryptoHandler::KeyPair)

#endif // ICRYPTOHANDLER_H