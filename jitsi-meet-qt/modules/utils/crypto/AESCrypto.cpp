#include "AESCrypto.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>
#include <QMutexLocker>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/err.h>

AESCrypto::AESCrypto(QObject* parent)
    : ICryptoHandler(parent)
    , m_initialized(false)
    , m_defaultMode(CBC)
    , m_defaultPadding(PKCS7)
    , m_opensslContext(nullptr)
{
}

AESCrypto::~AESCrypto()
{
    cleanup();
}

bool AESCrypto::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    if (!initializeOpenSSL()) {
        return false;
    }
    
    m_initialized = true;
    return true;
}

void AESCrypto::cleanup()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    cleanupOpenSSL();
    m_initialized = false;
}

ICryptoHandler::OperationResult AESCrypto::encrypt(const QByteArray& data, const QByteArray& key, 
                                                  Algorithm algorithm, Mode mode, Padding padding,
                                                  QByteArray& result)
{
    if (!m_initialized) {
        return AlgorithmError;
    }
    
    if (!validateAESParameters(algorithm, mode, padding)) {
        return InvalidKey;
    }
    
    if (!validateKeyLength(key, algorithm)) {
        return InvalidKey;
    }
    
    if (data.isEmpty()) {
        return InvalidData;
    }
    
    try {
        QByteArray iv = generateIV(16); // AES block size is always 16
        QByteArray encryptedData = performAESOperation(data, key, iv, algorithm, mode, true);
        
        if (encryptedData.isEmpty()) {
            return AlgorithmError;
        }
        
        // Prepend IV to encrypted data
        result = iv + encryptedData;
        
        emitOperationCompleted("AES Encrypt", Success);
        return Success;
        
    } catch (...) {
        return UnknownError;
    }
}

ICryptoHandler::OperationResult AESCrypto::decrypt(const QByteArray& data, const QByteArray& key,
                                                  Algorithm algorithm, Mode mode, Padding padding,
                                                  QByteArray& result)
{
    if (!m_initialized) {
        return AlgorithmError;
    }
    
    if (!validateAESParameters(algorithm, mode, padding)) {
        return InvalidKey;
    }
    
    if (!validateKeyLength(key, algorithm)) {
        return InvalidKey;
    }
    
    if (data.size() < 16) { // At least IV size
        return InsufficientData;
    }
    
    try {
        // Extract IV from the beginning
        QByteArray iv = data.left(16);
        QByteArray encryptedData = data.mid(16);
        
        QByteArray decryptedData = performAESOperation(encryptedData, key, iv, algorithm, mode, false);
        
        if (decryptedData.isEmpty()) {
            return AlgorithmError;
        }
        
        result = decryptedData;
        
        emitOperationCompleted("AES Decrypt", Success);
        return Success;
        
    } catch (...) {
        return UnknownError;
    }
}

ICryptoHandler::OperationResult AESCrypto::encryptAsymmetric(const QByteArray& data, const QByteArray& publicKey,
                                                            Algorithm algorithm, QByteArray& result)
{
    Q_UNUSED(data)
    Q_UNUSED(publicKey)
    Q_UNUSED(algorithm)
    Q_UNUSED(result)
    
    // AES is symmetric encryption, not asymmetric
    return AlgorithmError;
}

ICryptoHandler::OperationResult AESCrypto::decryptAsymmetric(const QByteArray& data, const QByteArray& privateKey,
                                                            Algorithm algorithm, QByteArray& result)
{
    Q_UNUSED(data)
    Q_UNUSED(privateKey)
    Q_UNUSED(algorithm)
    Q_UNUSED(result)
    
    // AES is symmetric encryption, not asymmetric
    return AlgorithmError;
}

ICryptoHandler::OperationResult AESCrypto::hash(const QByteArray& data, HashAlgorithm algorithm, QByteArray& result)
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

ICryptoHandler::OperationResult AESCrypto::hmac(const QByteArray& data, const QByteArray& key,
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

ICryptoHandler::OperationResult AESCrypto::generateKeyPair(Algorithm algorithm, KeyPair& keyPair)
{
    Q_UNUSED(algorithm)
    Q_UNUSED(keyPair)
    
    // AES doesn't use key pairs
    return AlgorithmError;
}

ICryptoHandler::OperationResult AESCrypto::generateRandomKey(int length, QByteArray& key)
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

ICryptoHandler::OperationResult AESCrypto::sign(const QByteArray& data, const QByteArray& privateKey,
                                               Algorithm algorithm, QByteArray& signature)
{
    Q_UNUSED(data)
    Q_UNUSED(privateKey)
    Q_UNUSED(algorithm)
    Q_UNUSED(signature)
    
    // AES doesn't support digital signatures
    return AlgorithmError;
}

bool AESCrypto::verify(const QByteArray& data, const QByteArray& signature,
                      const QByteArray& publicKey, Algorithm algorithm)
{
    Q_UNUSED(data)
    Q_UNUSED(signature)
    Q_UNUSED(publicKey)
    Q_UNUSED(algorithm)
    
    // AES doesn't support digital signatures
    return false;
}

QList<ICryptoHandler::Algorithm> AESCrypto::supportedAlgorithms() const
{
    return {AES_128, AES_192, AES_256};
}

QList<ICryptoHandler::HashAlgorithm> AESCrypto::supportedHashAlgorithms() const
{
    return {MD5, SHA1, SHA224, SHA256, SHA384, SHA512};
}

bool AESCrypto::isAlgorithmSupported(Algorithm algorithm) const
{
    return supportedAlgorithms().contains(algorithm);
}

bool AESCrypto::isHashAlgorithmSupported(HashAlgorithm algorithm) const
{
    return supportedHashAlgorithms().contains(algorithm);
}

QString AESCrypto::name() const
{
    return "AES Crypto Handler";
}

QString AESCrypto::version() const
{
    return "1.0.0";
}

QByteArray AESCrypto::encryptAES(const QByteArray& data, const QString& password, Algorithm algorithm)
{
    QByteArray salt = generateSalt();
    QByteArray key = deriveKeyFromPassword(password, salt, getKeyLength(algorithm));
    
    QByteArray result;
    if (encrypt(data, key, algorithm, m_defaultMode, m_defaultPadding, result) == Success) {
        return salt + result;
    }
    
    return QByteArray();
}

QByteArray AESCrypto::decryptAES(const QByteArray& encryptedData, const QString& password, Algorithm algorithm)
{
    if (encryptedData.size() < 16) {
        return QByteArray();
    }
    
    QByteArray salt = encryptedData.left(16);
    QByteArray data = encryptedData.mid(16);
    QByteArray key = deriveKeyFromPassword(password, salt, getKeyLength(algorithm));
    
    QByteArray result;
    if (decrypt(data, key, algorithm, m_defaultMode, m_defaultPadding, result) == Success) {
        return result;
    }
    
    return QByteArray();
}

QByteArray AESCrypto::deriveKeyFromPassword(const QString& password, const QByteArray& salt, 
                                           int keyLength, int iterations)
{
    // Simple PBKDF2 implementation
    QByteArray passwordBytes = password.toUtf8();
    QByteArray result;
    
    for (int i = 1; result.length() < keyLength; ++i) {
        QByteArray u = salt + QByteArray::number(i);
        QByteArray f;
        
        for (int j = 0; j < iterations; ++j) {
            QByteArray temp;
            hmac(u, passwordBytes, SHA256, temp);
            u = temp;
            
            if (j == 0) {
                f = temp;
            } else {
                for (int k = 0; k < temp.length() && k < f.length(); ++k) {
                    f[k] = f[k] ^ temp[k];
                }
            }
        }
        
        result.append(f);
    }
    
    return result.left(keyLength);
}

QByteArray AESCrypto::generateSalt(int length)
{
    QByteArray salt;
    generateRandomKey(length, salt);
    return salt;
}

QByteArray AESCrypto::generateIV(int length)
{
    QByteArray iv;
    generateRandomKey(length, iv);
    return iv;
}

void AESCrypto::setDefaultMode(Mode mode)
{
    m_defaultMode = mode;
}

ICryptoHandler::Mode AESCrypto::defaultMode() const
{
    return m_defaultMode;
}

void AESCrypto::setDefaultPadding(Padding padding)
{
    m_defaultPadding = padding;
}

ICryptoHandler::Padding AESCrypto::defaultPadding() const
{
    return m_defaultPadding;
}

bool AESCrypto::validateAESParameters(Algorithm algorithm, Mode mode, Padding padding) const
{
    // Check if algorithm is AES
    if (algorithm != AES_128 && algorithm != AES_192 && algorithm != AES_256) {
        return false;
    }
    
    // Check mode compatibility
    if (mode == GCM && padding != NoPadding) {
        return false; // GCM mode doesn't use padding
    }
    
    return true;
}

int AESCrypto::getKeyLength(Algorithm algorithm) const
{
    switch (algorithm) {
        case AES_128: return 16;
        case AES_192: return 24;
        case AES_256: return 32;
        default: return 0;
    }
}

int AESCrypto::getBlockSize(Algorithm algorithm) const
{
    Q_UNUSED(algorithm)
    return 16; // AES block size is always 16 bytes
}

QByteArray AESCrypto::applyPKCS7Padding(const QByteArray& data, int blockSize) const
{
    int paddingLength = blockSize - (data.length() % blockSize);
    QByteArray paddedData = data;
    paddedData.append(QByteArray(paddingLength, static_cast<char>(paddingLength)));
    return paddedData;
}

QByteArray AESCrypto::removePKCS7Padding(const QByteArray& data) const
{
    if (data.isEmpty()) {
        return data;
    }
    
    int paddingLength = static_cast<unsigned char>(data.at(data.length() - 1));
    if (paddingLength > data.length() || paddingLength == 0) {
        return data; // Invalid padding
    }
    
    // Verify padding
    for (int i = data.length() - paddingLength; i < data.length(); ++i) {
        if (static_cast<unsigned char>(data.at(i)) != paddingLength) {
            return data; // Invalid padding
        }
    }
    
    return data.left(data.length() - paddingLength);
}

QByteArray AESCrypto::performAESOperation(const QByteArray& data, const QByteArray& key, 
                                         const QByteArray& iv, Algorithm algorithm, 
                                         Mode mode, bool encrypt) const
{
    Q_UNUSED(mode) // For now, we'll use CBC mode
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return QByteArray();
    }
    
    const EVP_CIPHER* cipher = nullptr;
    switch (algorithm) {
        case AES_128:
            cipher = EVP_aes_128_cbc();
            break;
        case AES_192:
            cipher = EVP_aes_192_cbc();
            break;
        case AES_256:
            cipher = EVP_aes_256_cbc();
            break;
        default:
            EVP_CIPHER_CTX_free(ctx);
            return QByteArray();
    }
    
    QByteArray processedData = data;
    if (encrypt && m_defaultPadding == PKCS7) {
        processedData = applyPKCS7Padding(data, getBlockSize(algorithm));
    }
    
    if (EVP_CipherInit_ex(ctx, cipher, nullptr, 
                         reinterpret_cast<const unsigned char*>(key.constData()),
                         reinterpret_cast<const unsigned char*>(iv.constData()),
                         encrypt ? 1 : 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    QByteArray result;
    result.resize(processedData.length() + EVP_CIPHER_block_size(cipher));
    
    int len = 0;
    int totalLen = 0;
    
    if (EVP_CipherUpdate(ctx, reinterpret_cast<unsigned char*>(result.data()), &len,
                        reinterpret_cast<const unsigned char*>(processedData.constData()),
                        processedData.length()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    totalLen = len;
    
    if (EVP_CipherFinal_ex(ctx, reinterpret_cast<unsigned char*>(result.data()) + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    totalLen += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    result.resize(totalLen);
    
    if (!encrypt && m_defaultPadding == PKCS7) {
        result = removePKCS7Padding(result);
    }
    
    return result;
}

bool AESCrypto::initializeOpenSSL()
{
    // Initialize OpenSSL
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    
    return true;
}

void AESCrypto::cleanupOpenSSL()
{
    EVP_cleanup();
    ERR_free_strings();
}