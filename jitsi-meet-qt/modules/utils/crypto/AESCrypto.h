#ifndef AESCRYPTO_H
#define AESCRYPTO_H

#include "../interfaces/ICryptoHandler.h"
#include <QMutex>

/**
 * @brief AES加密处理器
 * 
 * AESCrypto实现了AES（高级加密标准）算法的加密和解密功能，
 * 支持AES-128、AES-192、AES-256以及多种加密模式。
 */
class AESCrypto : public ICryptoHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AESCrypto(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~AESCrypto() override;

    // ICryptoHandler接口实现
    bool initialize() override;
    void cleanup() override;
    
    OperationResult encrypt(const QByteArray& data, const QByteArray& key, 
                           Algorithm algorithm, Mode mode, Padding padding,
                           QByteArray& result) override;
    
    OperationResult decrypt(const QByteArray& data, const QByteArray& key,
                           Algorithm algorithm, Mode mode, Padding padding,
                           QByteArray& result) override;
    
    OperationResult encryptAsymmetric(const QByteArray& data, const QByteArray& publicKey,
                                     Algorithm algorithm, QByteArray& result) override;
    
    OperationResult decryptAsymmetric(const QByteArray& data, const QByteArray& privateKey,
                                     Algorithm algorithm, QByteArray& result) override;
    
    OperationResult hash(const QByteArray& data, HashAlgorithm algorithm, QByteArray& result) override;
    
    OperationResult hmac(const QByteArray& data, const QByteArray& key,
                        HashAlgorithm algorithm, QByteArray& result) override;
    
    OperationResult generateKeyPair(Algorithm algorithm, KeyPair& keyPair) override;
    
    OperationResult generateRandomKey(int length, QByteArray& key) override;
    
    OperationResult sign(const QByteArray& data, const QByteArray& privateKey,
                        Algorithm algorithm, QByteArray& signature) override;
    
    bool verify(const QByteArray& data, const QByteArray& signature,
               const QByteArray& publicKey, Algorithm algorithm) override;
    
    QList<Algorithm> supportedAlgorithms() const override;
    QList<HashAlgorithm> supportedHashAlgorithms() const override;
    bool isAlgorithmSupported(Algorithm algorithm) const override;
    bool isHashAlgorithmSupported(HashAlgorithm algorithm) const override;
    QString name() const override;
    QString version() const override;

    /**
     * @brief 简化的AES加密方法
     * @param data 原始数据
     * @param password 密码
     * @param algorithm AES算法类型
     * @return 加密后的数据
     */
    QByteArray encryptAES(const QByteArray& data, const QString& password, Algorithm algorithm = AES_256);

    /**
     * @brief 简化的AES解密方法
     * @param encryptedData 加密数据
     * @param password 密码
     * @param algorithm AES算法类型
     * @return 解密后的数据
     */
    QByteArray decryptAES(const QByteArray& encryptedData, const QString& password, Algorithm algorithm = AES_256);

    /**
     * @brief 从密码生成密钥
     * @param password 密码
     * @param salt 盐值
     * @param keyLength 密钥长度
     * @param iterations 迭代次数
     * @return 生成的密钥
     */
    QByteArray deriveKeyFromPassword(const QString& password, const QByteArray& salt, 
                                    int keyLength, int iterations = 10000);

    /**
     * @brief 生成随机盐值
     * @param length 盐值长度
     * @return 随机盐值
     */
    QByteArray generateSalt(int length = 16);

    /**
     * @brief 生成随机初始化向量
     * @param length IV长度
     * @return 随机IV
     */
    QByteArray generateIV(int length = 16);

    /**
     * @brief 设置默认加密模式
     * @param mode 加密模式
     */
    void setDefaultMode(Mode mode);

    /**
     * @brief 获取默认加密模式
     * @return 加密模式
     */
    Mode defaultMode() const;

    /**
     * @brief 设置默认填充模式
     * @param padding 填充模式
     */
    void setDefaultPadding(Padding padding);

    /**
     * @brief 获取默认填充模式
     * @return 填充模式
     */
    Padding defaultPadding() const;

private:
    /**
     * @brief 验证AES参数
     * @param algorithm 算法类型
     * @param mode 加密模式
     * @param padding 填充模式
     * @return 参数是否有效
     */
    bool validateAESParameters(Algorithm algorithm, Mode mode, Padding padding) const;

    /**
     * @brief 获取密钥长度
     * @param algorithm 算法类型
     * @return 密钥长度（字节）
     */
    int getKeyLength(Algorithm algorithm) const;

    /**
     * @brief 获取块大小
     * @param algorithm 算法类型
     * @return 块大小（字节）
     */
    int getBlockSize(Algorithm algorithm) const;

    /**
     * @brief 应用PKCS7填充
     * @param data 原始数据
     * @param blockSize 块大小
     * @return 填充后的数据
     */
    QByteArray applyPKCS7Padding(const QByteArray& data, int blockSize) const;

    /**
     * @brief 移除PKCS7填充
     * @param data 填充后的数据
     * @return 原始数据
     */
    QByteArray removePKCS7Padding(const QByteArray& data) const;

    /**
     * @brief 执行AES加密核心操作
     * @param data 原始数据
     * @param key 密钥
     * @param iv 初始化向量
     * @param algorithm 算法类型
     * @param mode 加密模式
     * @param encrypt 是否为加密操作
     * @return 处理后的数据
     */
    QByteArray performAESOperation(const QByteArray& data, const QByteArray& key, 
                                  const QByteArray& iv, Algorithm algorithm, 
                                  Mode mode, bool encrypt) const;

    /**
     * @brief 初始化OpenSSL
     * @return 初始化是否成功
     */
    bool initializeOpenSSL();

    /**
     * @brief 清理OpenSSL
     */
    void cleanupOpenSSL();

private:
    bool m_initialized;                 ///< 是否已初始化
    Mode m_defaultMode;                 ///< 默认加密模式
    Padding m_defaultPadding;           ///< 默认填充模式
    
    mutable QMutex m_mutex;             ///< 线程安全互斥锁
    
    // OpenSSL相关成员
    void* m_opensslContext;             ///< OpenSSL上下文（避免包含OpenSSL头文件）
};

#endif // AESCRYPTO_H