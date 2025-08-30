#include "RSACrypto.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>
#include <QMutexLocker>
#include <QJsonObject>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

RSACrypto::RSACrypto(QObject* parent)
    : ICryptoHandler(parent)
    , m_initialized(false)
    , m_defaultPadding(OAEP_Padding)
    , m_opensslContext(nullptr)
    , m_randomGenerator(nullptr)
{
}

RSACrypto::~RSACrypto()
{
    cleanup();
}

bool RSACrypto::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    if (!initializeOpenSSLRSA()) {
        return false;
    }
    
    m_initialized = true;
    return true;
}

void RSACrypto::cleanup()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    cleanupOpenSSLRSA();
    m_initialized = false;
}

ICryptoHandler::OperationResult RSACrypto::encrypt(const QByteArray& data, const QByteArray& key, 
                                                  Algorithm algorithm, Mode mode, Padding padding,
                                                  QByteArray& result)
{
    Q_UNUSED(mode)
    Q_UNUSED(padding)
    
    return encryptAsymmetric(data, key, algorithm, result);
}

ICryptoHandler::OperationResult RSACrypto::decrypt(const QByteArray& data, const QByteArray& key,
                                                  Algorithm algorithm, Mode mode, Padding padding,
                                                  QByteArray& result)
{
    Q_UNUSED(mode)
    Q_UNUSED(padding)
    
    return decryptAsymmetric(data, key, algorithm, result);
}

ICryptoHandler::OperationResult RSACrypto::encryptAsymmetric(const QByteArray& data, const QByteArray& publicKey,
                                                            Algorithm algorithm, QByteArray& result)
{
    if (!m_initialized) {
        return AlgorithmError;
    }
    
    if (!isAlgorithmSupported(algorithm)) {
        return AlgorithmError;
    }
    
    if (data.isEmpty() || publicKey.isEmpty()) {
        return InvalidData;
    }
    
    try {
        result = performRSAOperation(data, publicKey, algorithm, m_defaultPadding, true, true);
        
        if (result.isEmpty()) {
            return AlgorithmError;
        }
        
        emitOperationCompleted("RSA Encrypt", Success);
        return Success;
        
    } catch (...) {
        return UnknownError;
    }
}

ICryptoHandler::OperationResult RSACrypto::decryptAsymmetric(const QByteArray& data, const QByteArray& privateKey,
                                                            Algorithm algorithm, QByteArray& result)
{
    if (!m_initialized) {
        return AlgorithmError;
    }
    
    if (!isAlgorithmSupported(algorithm)) {
        return AlgorithmError;
    }
    
    if (data.isEmpty() || privateKey.isEmpty()) {
        return InvalidData;
    }
    
    try {
        result = performRSAOperation(data, privateKey, algorithm, m_defaultPadding, false, false);
        
        if (result.isEmpty()) {
            return AlgorithmError;
        }
        
        emitOperationCompleted("RSA Decrypt", Success);
        return Success;
        
    } catch (...) {
        return UnknownError;
    }
}

ICryptoHandler::OperationResult RSACrypto::hash(const QByteArray& data, HashAlgorithm algorithm, QByteArray& result)
{
    QCryptographicHash::Algorithm qtAlgorithm;
    
    switch (algorithm) {
        case MD5:
            qtAlgorithm = QCryptographicHash::Md5;
            break;
        case SHA1:
            qtAlgorithm = QCryptographicHash::Sha1;
            break;
        case SHA224:
            qtAlgorithm = QCryptographicHash::Sha224;
            break;
        case SHA256:
            qtAlgorithm = QCryptographicHash::Sha256;
            break;
        case SHA384:
            qtAlgorithm = QCryptographicHash::Sha384;
            break;
        case SHA512:
            qtAlgorithm = QCryptographicHash::Sha512;
            break;
        default:
            return AlgorithmError;
    }
    
    result = QCryptographicHash::hash(data, qtAlgorithm);
    return Success;
}

ICryptoHandler::OperationResult RSACrypto::hmac(const QByteArray& data, const QByteArray& key,
                                               HashAlgorithm algorithm, QByteArray& result)
{
    // Simple HMAC implementation using Qt
    QByteArray ipad(64, 0x36);
    QByteArray opad(64, 0x5c);
    
    QByteArray actualKey = key;
    if (actualKey.length() > 64) {
        hash(actualKey, algorithm, actualKey);
    }
    if (actualKey.length() < 64) {
        actualKey = actualKey.leftJustified(64, 0);
    }
    
    for (int i = 0; i < 64; ++i) {
        ipad[i] = ipad[i] ^ actualKey[i];
        opad[i] = opad[i] ^ actualKey[i];
    }
    
    QByteArray innerHash;
    hash(ipad + data, algorithm, innerHash);
    
    return hash(opad + innerHash, algorithm, result);
}

ICryptoHandler::OperationResult RSACrypto::generateKeyPair(Algorithm algorithm, KeyPair& keyPair)
{
    if (!m_initialized) {
        return AlgorithmError;
    }
    
    if (!isAlgorithmSupported(algorithm)) {
        return AlgorithmError;
    }
    
    int keySize = getRSAKeySize(algorithm);
    if (keySize == 0) {
        return AlgorithmError;
    }
    
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        return AlgorithmError;
    }
    
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return AlgorithmError;
    }
    
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keySize) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return AlgorithmError;
    }
    
    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return AlgorithmError;
    }
    
    // Extract public key
    BIO* pubBio = BIO_new(BIO_s_mem());
    if (PEM_write_bio_PUBKEY(pubBio, pkey) != 1) {
        BIO_free(pubBio);
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        return AlgorithmError;
    }
    
    char* pubData;
    long pubLen = BIO_get_mem_data(pubBio, &pubData);
    keyPair.publicKey = QByteArray(pubData, pubLen);
    BIO_free(pubBio);
    
    // Extract private key
    BIO* privBio = BIO_new(BIO_s_mem());
    if (PEM_write_bio_PrivateKey(privBio, pkey, nullptr, nullptr, 0, nullptr, nullptr) != 1) {
        BIO_free(privBio);
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        return AlgorithmError;
    }
    
    char* privData;
    long privLen = BIO_get_mem_data(privBio, &privData);
    keyPair.privateKey = QByteArray(privData, privLen);
    BIO_free(privBio);
    
    keyPair.algorithm = algorithm;
    
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    
    return Success;
}

ICryptoHandler::OperationResult RSACrypto::generateRandomKey(int length, QByteArray& key)
{
    if (length <= 0) {
        return InvalidData;
    }
    
    key.resize(length);
    if (RAND_bytes(reinterpret_cast<unsigned char*>(key.data()), length) != 1) {
        // Fallback to Qt random generator
        for (int i = 0; i < length; ++i) {
            key[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
        }
    }
    
    return Success;
}

ICryptoHandler::OperationResult RSACrypto::sign(const QByteArray& data, const QByteArray& privateKey,
                                               Algorithm algorithm, QByteArray& signature)
{
    if (!m_initialized) {
        return AlgorithmError;
    }
    
    if (!isAlgorithmSupported(algorithm)) {
        return AlgorithmError;
    }
    
    if (data.isEmpty() || privateKey.isEmpty()) {
        return InvalidData;
    }
    
    try {
        signature = performRSASign(data, privateKey, algorithm, SHA256, PSS_Padding);
        
        if (signature.isEmpty()) {
            return AlgorithmError;
        }
        
        emitOperationCompleted("RSA Sign", Success);
        return Success;
        
    } catch (...) {
        return UnknownError;
    }
}

bool RSACrypto::verify(const QByteArray& data, const QByteArray& signature,
                      const QByteArray& publicKey, Algorithm algorithm)
{
    if (!m_initialized) {
        return false;
    }
    
    if (!isAlgorithmSupported(algorithm)) {
        return false;
    }
    
    if (data.isEmpty() || signature.isEmpty() || publicKey.isEmpty()) {
        return false;
    }
    
    try {
        return performRSAVerify(data, signature, publicKey, algorithm, SHA256, PSS_Padding);
    } catch (...) {
        return false;
    }
}

QList<ICryptoHandler::Algorithm> RSACrypto::supportedAlgorithms() const
{
    return {RSA_1024, RSA_2048, RSA_4096};
}

QList<ICryptoHandler::HashAlgorithm> RSACrypto::supportedHashAlgorithms() const
{
    return {MD5, SHA1, SHA224, SHA256, SHA384, SHA512};
}

bool RSACrypto::isAlgorithmSupported(Algorithm algorithm) const
{
    return supportedAlgorithms().contains(algorithm);
}

bool RSACrypto::isHashAlgorithmSupported(HashAlgorithm algorithm) const
{
    return supportedHashAlgorithms().contains(algorithm);
}

QString RSACrypto::name() const
{
    return "RSA Crypto Handler";
}

QString RSACrypto::version() const
{
    return "1.0.0";
}

QByteArray RSACrypto::encryptRSA(const QByteArray& data, const QString& publicKeyPem, RSAPadding padding)
{
    QByteArray publicKey = loadPublicKeyFromPEM(publicKeyPem);
    if (publicKey.isEmpty()) {
        return QByteArray();
    }
    
    QByteArray result = performRSAOperation(data, publicKey, RSA_2048, padding, true, true);
    return result;
}

QByteArray RSACrypto::decryptRSA(const QByteArray& encryptedData, const QString& privateKeyPem, RSAPadding padding)
{
    QByteArray privateKey = loadPrivateKeyFromPEM(privateKeyPem);
    if (privateKey.isEmpty()) {
        return QByteArray();
    }
    
    QByteArray result = performRSAOperation(encryptedData, privateKey, RSA_2048, padding, false, false);
    return result;
}

bool RSACrypto::generateRSAKeyPairPEM(int keySize, QString& publicKeyPem, QString& privateKeyPem)
{
    Algorithm algorithm;
    switch (keySize) {
        case 1024: algorithm = RSA_1024; break;
        case 2048: algorithm = RSA_2048; break;
        case 4096: algorithm = RSA_4096; break;
        default: return false;
    }
    
    KeyPair keyPair;
    if (generateKeyPair(algorithm, keyPair) != Success) {
        return false;
    }
    
    publicKeyPem = QString::fromUtf8(keyPair.publicKey);
    privateKeyPem = QString::fromUtf8(keyPair.privateKey);
    
    return true;
}

QByteArray RSACrypto::loadPublicKeyFromPEM(const QString& pemData)
{
    BIO* bio = BIO_new_mem_buf(pemData.toUtf8().constData(), -1);
    if (!bio) {
        return QByteArray();
    }
    
    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    
    if (!pkey) {
        return QByteArray();
    }
    
    BIO* outBio = BIO_new(BIO_s_mem());
    if (PEM_write_bio_PUBKEY(outBio, pkey) != 1) {
        BIO_free(outBio);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }
    
    char* data;
    long len = BIO_get_mem_data(outBio, &data);
    QByteArray result(data, len);
    
    BIO_free(outBio);
    EVP_PKEY_free(pkey);
    
    return result;
}

QByteArray RSACrypto::loadPrivateKeyFromPEM(const QString& pemData, const QString& password)
{
    BIO* bio = BIO_new_mem_buf(pemData.toUtf8().constData(), -1);
    if (!bio) {
        return QByteArray();
    }
    
    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, 
                                            password.isEmpty() ? nullptr : const_cast<char*>(password.toUtf8().constData()));
    BIO_free(bio);
    
    if (!pkey) {
        return QByteArray();
    }
    
    BIO* outBio = BIO_new(BIO_s_mem());
    if (PEM_write_bio_PrivateKey(outBio, pkey, nullptr, nullptr, 0, nullptr, nullptr) != 1) {
        BIO_free(outBio);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }
    
    char* data;
    long len = BIO_get_mem_data(outBio, &data);
    QByteArray result(data, len);
    
    BIO_free(outBio);
    EVP_PKEY_free(pkey);
    
    return result;
}

QString RSACrypto::exportPublicKeyToPEM(const QByteArray& publicKey)
{
    return QString::fromUtf8(publicKey);
}

QString RSACrypto::exportPrivateKeyToPEM(const QByteArray& privateKey, const QString& password)
{
    Q_UNUSED(password) // For now, we don't encrypt the private key
    return QString::fromUtf8(privateKey);
}

QJsonObject RSACrypto::getRSAKeyInfo(const QByteArray& keyData, bool isPrivateKey)
{
    Q_UNUSED(keyData)
    Q_UNUSED(isPrivateKey)
    
    QJsonObject info;
    info["type"] = "RSA";
    info["format"] = "PEM";
    info["isPrivate"] = isPrivateKey;
    
    return info;
}

bool RSACrypto::validateKeyPair(const QByteArray& publicKey, const QByteArray& privateKey)
{
    // Simple validation by trying to encrypt/decrypt a test message
    QByteArray testData = "test message";
    QByteArray encrypted = performRSAOperation(testData, publicKey, RSA_2048, OAEP_Padding, true, true);
    if (encrypted.isEmpty()) {
        return false;
    }
    
    QByteArray decrypted = performRSAOperation(encrypted, privateKey, RSA_2048, OAEP_Padding, false, false);
    return decrypted == testData;
}

void RSACrypto::setDefaultRSAPadding(RSAPadding padding)
{
    m_defaultPadding = padding;
}

RSACrypto::RSAPadding RSACrypto::defaultRSAPadding() const
{
    return m_defaultPadding;
}

int RSACrypto::getMaxEncryptionLength(int keySize, RSAPadding padding) const
{
    int overhead = 0;
    switch (padding) {
        case PKCS1_Padding:
            overhead = 11;
            break;
        case OAEP_Padding:
            overhead = 42; // 2 + 2 * hash_length (SHA-1 = 20)
            break;
        case PSS_Padding:
            overhead = 0; // PSS is for signing only
            break;
    }
    
    return (keySize / 8) - overhead;
}

bool RSACrypto::validateRSAParameters(Algorithm algorithm, RSAPadding padding) const
{
    Q_UNUSED(padding)
    
    return isAlgorithmSupported(algorithm);
}

int RSACrypto::getRSAKeySize(Algorithm algorithm) const
{
    switch (algorithm) {
        case RSA_1024: return 1024;
        case RSA_2048: return 2048;
        case RSA_4096: return 4096;
        default: return 0;
    }
}

QByteArray RSACrypto::performRSAOperation(const QByteArray& data, const QByteArray& key,
                                         Algorithm algorithm, RSAPadding padding,
                                         bool encrypt, bool usePublicKey) const
{
    Q_UNUSED(algorithm)
    Q_UNUSED(padding)
    
    // Load key
    BIO* bio = BIO_new_mem_buf(key.constData(), key.length());
    if (!bio) {
        return QByteArray();
    }
    
    EVP_PKEY* pkey = nullptr;
    if (usePublicKey) {
        pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    } else {
        pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    }
    BIO_free(bio);
    
    if (!pkey) {
        return QByteArray();
    }
    
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return QByteArray();
    }
    
    int result = 0;
    if (encrypt) {
        result = EVP_PKEY_encrypt_init(ctx);
    } else {
        result = EVP_PKEY_decrypt_init(ctx);
    }
    
    if (result <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }
    
    // Set padding
    int opensslPadding = RSA_PKCS1_OAEP_PADDING; // Default to OAEP
    switch (padding) {
        case PKCS1_Padding:
            opensslPadding = RSA_PKCS1_PADDING;
            break;
        case OAEP_Padding:
            opensslPadding = RSA_PKCS1_OAEP_PADDING;
            break;
        case PSS_Padding:
            opensslPadding = RSA_PKCS1_PSS_PADDING;
            break;
    }
    
    EVP_PKEY_CTX_set_rsa_padding(ctx, opensslPadding);
    
    size_t outlen = 0;
    if (encrypt) {
        result = EVP_PKEY_encrypt(ctx, nullptr, &outlen, 
                                 reinterpret_cast<const unsigned char*>(data.constData()), 
                                 data.length());
    } else {
        result = EVP_PKEY_decrypt(ctx, nullptr, &outlen,
                                 reinterpret_cast<const unsigned char*>(data.constData()),
                                 data.length());
    }
    
    if (result <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }
    
    QByteArray output;
    output.resize(outlen);
    
    if (encrypt) {
        result = EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char*>(output.data()), &outlen,
                                 reinterpret_cast<const unsigned char*>(data.constData()),
                                 data.length());
    } else {
        result = EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char*>(output.data()), &outlen,
                                 reinterpret_cast<const unsigned char*>(data.constData()),
                                 data.length());
    }
    
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    
    if (result <= 0) {
        return QByteArray();
    }
    
    output.resize(outlen);
    return output;
}

QByteArray RSACrypto::performRSASign(const QByteArray& data, const QByteArray& privateKey,
                                    Algorithm algorithm, HashAlgorithm hashAlgorithm,
                                    RSAPadding padding) const
{
    Q_UNUSED(algorithm)
    Q_UNUSED(hashAlgorithm)
    Q_UNUSED(padding)
    
    // Load private key
    BIO* bio = BIO_new_mem_buf(privateKey.constData(), privateKey.length());
    if (!bio) {
        return QByteArray();
    }
    
    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    
    if (!pkey) {
        return QByteArray();
    }
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        EVP_PKEY_free(pkey);
        return QByteArray();
    }
    
    if (EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }
    
    if (EVP_DigestSignUpdate(mdctx, data.constData(), data.length()) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }
    
    size_t siglen = 0;
    if (EVP_DigestSignFinal(mdctx, nullptr, &siglen) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }
    
    QByteArray signature;
    signature.resize(siglen);
    
    if (EVP_DigestSignFinal(mdctx, reinterpret_cast<unsigned char*>(signature.data()), &siglen) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }
    
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
    
    signature.resize(siglen);
    return signature;
}

bool RSACrypto::performRSAVerify(const QByteArray& data, const QByteArray& signature,
                                const QByteArray& publicKey, Algorithm algorithm,
                                HashAlgorithm hashAlgorithm, RSAPadding padding) const
{
    Q_UNUSED(algorithm)
    Q_UNUSED(hashAlgorithm)
    Q_UNUSED(padding)
    
    // Load public key
    BIO* bio = BIO_new_mem_buf(publicKey.constData(), publicKey.length());
    if (!bio) {
        return false;
    }
    
    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    
    if (!pkey) {
        return false;
    }
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        EVP_PKEY_free(pkey);
        return false;
    }
    
    if (EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return false;
    }
    
    if (EVP_DigestVerifyUpdate(mdctx, data.constData(), data.length()) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return false;
    }
    
    int result = EVP_DigestVerifyFinal(mdctx, 
                                      reinterpret_cast<const unsigned char*>(signature.constData()),
                                      signature.length());
    
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
    
    return result == 1;
}

QByteArray RSACrypto::encryptLargeData(const QByteArray& data, const QByteArray& key,
                                      Algorithm algorithm, RSAPadding padding, bool usePublicKey) const
{
    int maxChunkSize = getMaxEncryptionLength(getRSAKeySize(algorithm), padding);
    if (maxChunkSize <= 0) {
        return QByteArray();
    }
    
    QByteArray result;
    int offset = 0;
    
    while (offset < data.length()) {
        int chunkSize = qMin(maxChunkSize, data.length() - offset);
        QByteArray chunk = data.mid(offset, chunkSize);
        
        QByteArray encryptedChunk = performRSAOperation(chunk, key, algorithm, padding, true, usePublicKey);
        if (encryptedChunk.isEmpty()) {
            return QByteArray();
        }
        
        result.append(encryptedChunk);
        offset += chunkSize;
    }
    
    return result;
}

QByteArray RSACrypto::decryptLargeData(const QByteArray& encryptedData, const QByteArray& key,
                                      Algorithm algorithm, RSAPadding padding, bool usePrivateKey) const
{
    int keySize = getRSAKeySize(algorithm) / 8; // Convert bits to bytes
    if (keySize <= 0 || encryptedData.length() % keySize != 0) {
        return QByteArray();
    }
    
    QByteArray result;
    int offset = 0;
    
    while (offset < encryptedData.length()) {
        QByteArray chunk = encryptedData.mid(offset, keySize);
        
        QByteArray decryptedChunk = performRSAOperation(chunk, key, algorithm, padding, false, usePrivateKey);
        if (decryptedChunk.isEmpty()) {
            return QByteArray();
        }
        
        result.append(decryptedChunk);
        offset += keySize;
    }
    
    return result;
}

bool RSACrypto::initializeOpenSSLRSA()
{
    // Initialize OpenSSL
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    
    return true;
}

void RSACrypto::cleanupOpenSSLRSA()
{
    EVP_cleanup();
    ERR_free_strings();
}