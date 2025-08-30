#ifndef RSACRYPTO_H
#define RSACRYPTO_H

#include "../interfaces/ICryptoHandler.h"
#include <QMutex>

/**
 * @brief RSA加密处理器
 * 
 * RSACrypto实现了RSA（Rivest-Shamir-Adleman）非对称加密算法，
 * 支持RSA-1024、RSA-2048、RSA-4096以及数字签名功能。
 */
class RSACrypto : public ICryptoHandler
{
    Q_OBJECT

public:
    /**
     * @brief RSA填充模式枚举
     */
    enum RSAPadding {
        PKCS1_Padding,      ///< PKCS#1 v1.5填充
        OAEP_Padding,       ///< OAEP填充
        PSS_Padding         ///< PSS填充（仅用于签名）
    };
    Q_ENUM(RSAPadding)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit RSACrypto(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~RSACrypto() override;

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
     * @brief 简化的RSA加密方法
     * @param data 原始数据
     * @param publicKeyPem PEM格式公钥
     * @param padding 填充模式
     * @return 加密后的数据
     */
    QByteArray encryptRSA(const QByteArray& data, const QString& publicKeyPem, 
                         RSAPadding padding = OAEP_Padding);

    /**
     * @brief 简化的RSA解密方法
     * @param encryptedData 加密数据
     * @param privateKeyPem PEM格式私钥
     * @param padding 填充模式
     * @return 解密后的数据
     */
    QByteArray decryptRSA(const QByteArray& encryptedData, const QString& privateKeyPem,
                         RSAPadding padding = OAEP_Padding);

    /**
     * @brief 生成RSA密钥对（PEM格式）
     * @param keySize 密钥大小（位）
     * @param publicKeyPem 输出公钥PEM
     * @param privateKeyPem 输出私钥PEM
     * @return 生成是否成功
     */
    bool generateRSAKeyPairPEM(int keySize, QString& publicKeyPem, QString& privateKeyPem);

    /**
     * @brief 从PEM格式加载公钥
     * @param pemData PEM格式数据
     * @return 公钥数据
     */
    QByteArray loadPublicKeyFromPEM(const QString& pemData);

    /**
     * @brief 从PEM格式加载私钥
     * @param pemData PEM格式数据
     * @param password 私钥密码（可选）
     * @return 私钥数据
     */
    QByteArray loadPrivateKeyFromPEM(const QString& pemData, const QString& password = QString());

    /**
     * @brief 将公钥导出为PEM格式
     * @param publicKey 公钥数据
     * @return PEM格式字符串
     */
    QString exportPublicKeyToPEM(const QByteArray& publicKey);

    /**
     * @brief 将私钥导出为PEM格式
     * @param privateKey 私钥数据
     * @param password 加密密码（可选）
     * @return PEM格式字符串
     */
    QString exportPrivateKeyToPEM(const QByteArray& privateKey, const QString& password = QString());

    /**
     * @brief 获取RSA密钥信息
     * @param keyData 密钥数据
     * @param isPrivateKey 是否为私钥
     * @return 密钥信息JSON对象
     */
    QJsonObject getRSAKeyInfo(const QByteArray& keyData, bool isPrivateKey);

    /**
     * @brief 验证RSA密钥对
     * @param publicKey 公钥
     * @param privateKey 私钥
     * @return 密钥对是否匹配
     */
    bool validateKeyPair(const QByteArray& publicKey, const QByteArray& privateKey);

    /**
     * @brief 设置默认RSA填充模式
     * @param padding 填充模式
     */
    void setDefaultRSAPadding(RSAPadding padding);

    /**
     * @brief 获取默认RSA填充模式
     * @return 填充模式
     */
    RSAPadding defaultRSAPadding() const;

    /**
     * @brief 获取RSA密钥的最大加密数据长度
     * @param keySize 密钥大小（位）
     * @param padding 填充模式
     * @return 最大数据长度
     */
    int getMaxEncryptionLength(int keySize, RSAPadding padding) const;

private:
    /**
     * @brief 验证RSA参数
     * @param algorithm 算法类型
     * @param padding 填充模式
     * @return 参数是否有效
     */
    bool validateRSAParameters(Algorithm algorithm, RSAPadding padding) const;

    /**
     * @brief 获取RSA密钥大小
     * @param algorithm 算法类型
     * @return 密钥大小（位）
     */
    int getRSAKeySize(Algorithm algorithm) const;

    /**
     * @brief 执行RSA加密核心操作
     * @param data 原始数据
     * @param key 密钥
     * @param algorithm 算法类型
     * @param padding 填充模式
     * @param encrypt 是否为加密操作
     * @param usePublicKey 是否使用公钥
     * @return 处理后的数据
     */
    QByteArray performRSAOperation(const QByteArray& data, const QByteArray& key,
                                  Algorithm algorithm, RSAPadding padding,
                                  bool encrypt, bool usePublicKey) const;

    /**
     * @brief 执行RSA签名操作
     * @param data 待签名数据
     * @param privateKey 私钥
     * @param algorithm 算法类型
     * @param hashAlgorithm 哈希算法
     * @param padding 填充模式
     * @return 签名数据
     */
    QByteArray performRSASign(const QByteArray& data, const QByteArray& privateKey,
                             Algorithm algorithm, HashAlgorithm hashAlgorithm,
                             RSAPadding padding) const;

    /**
     * @brief 执行RSA签名验证操作
     * @param data 原始数据
     * @param signature 签名数据
     * @param publicKey 公钥
     * @param algorithm 算法类型
     * @param hashAlgorithm 哈希算法
     * @param padding 填充模式
     * @return 验证是否成功
     */
    bool performRSAVerify(const QByteArray& data, const QByteArray& signature,
                         const QByteArray& publicKey, Algorithm algorithm,
                         HashAlgorithm hashAlgorithm, RSAPadding padding) const;

    /**
     * @brief 分块加密大数据
     * @param data 原始数据
     * @param key 密钥
     * @param algorithm 算法类型
     * @param padding 填充模式
     * @param usePublicKey 是否使用公钥
     * @return 加密后的数据
     */
    QByteArray encryptLargeData(const QByteArray& data, const QByteArray& key,
                               Algorithm algorithm, RSAPadding padding, bool usePublicKey) const;

    /**
     * @brief 分块解密大数据
     * @param encryptedData 加密数据
     * @param key 密钥
     * @param algorithm 算法类型
     * @param padding 填充模式
     * @param usePrivateKey 是否使用私钥
     * @return 解密后的数据
     */
    QByteArray decryptLargeData(const QByteArray& encryptedData, const QByteArray& key,
                               Algorithm algorithm, RSAPadding padding, bool usePrivateKey) const;

    /**
     * @brief 初始化OpenSSL RSA
     * @return 初始化是否成功
     */
    bool initializeOpenSSLRSA();

    /**
     * @brief 清理OpenSSL RSA
     */
    void cleanupOpenSSLRSA();

private:
    bool m_initialized;                 ///< 是否已初始化
    RSAPadding m_defaultPadding;        ///< 默认填充模式
    
    mutable QMutex m_mutex;             ///< 线程安全互斥锁
    
    // OpenSSL相关成员
    void* m_opensslContext;             ///< OpenSSL上下文
    void* m_randomGenerator;            ///< 随机数生成器
};

#endif // RSACRYPTO_H