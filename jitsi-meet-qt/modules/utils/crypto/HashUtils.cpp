#include "HashUtils.h"
#include <QFile>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QDebug>
#include <QMutexLocker>
#include <QStringConverter>

QMutex HashUtils::s_mutex;

HashUtils::HashUtils(QObject* parent)
    : QObject(parent)
{
}

HashUtils::~HashUtils()
{
}

HashUtils::HashResult HashUtils::hash(const QByteArray& data, HashAlgorithm algorithm)
{
    QElapsedTimer timer;
    timer.start();
    
    HashResult result = performHash(data, algorithm);
    result.processingTime = timer.elapsed();
    
    if (result.isValid()) {
        result.hexString = result.hash.toHex();
        result.base64String = result.hash.toBase64();
    }
    
    return result;
}

HashUtils::HashResult HashUtils::hash(const QString& text, HashAlgorithm algorithm, const QString& encoding)
{
    // In Qt 6, use QStringConverter instead of QTextCodec
    QByteArray data;
    if (encoding == "UTF-8") {
        data = text.toUtf8();
    } else if (encoding == "Latin1") {
        data = text.toLatin1();
    } else {
        data = text.toUtf8(); // Default to UTF-8
    }
    return hash(data, algorithm);
}

HashUtils::HashResult HashUtils::hashFile(const QString& filePath, HashAlgorithm algorithm)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return HashResult();
    }
    
    QElapsedTimer timer;
    timer.start();
    
    QCryptographicHash::Algorithm qtAlgorithm = toQtAlgorithm(algorithm);
    if (qtAlgorithm == static_cast<QCryptographicHash::Algorithm>(-1)) {
        return HashResult();
    }
    
    QCryptographicHash hasher(qtAlgorithm);
    
    const int bufferSize = 64 * 1024; // 64KB buffer
    char buffer[bufferSize];
    
    while (!file.atEnd()) {
        qint64 bytesRead = file.read(buffer, bufferSize);
        if (bytesRead > 0) {
            hasher.addData(buffer, bytesRead);
        }
    }
    
    HashResult result;
    result.hash = hasher.result();
    result.algorithm = algorithm;
    result.processingTime = timer.elapsed();
    result.hexString = result.hash.toHex();
    result.base64String = result.hash.toBase64();
    
    return result;
}

HashUtils::HashResult HashUtils::hmac(const QByteArray& data, const QByteArray& key, HashAlgorithm algorithm)
{
    QElapsedTimer timer;
    timer.start();
    
    HashResult result = performHMAC(data, key, algorithm);
    result.processingTime = timer.elapsed();
    
    if (result.isValid()) {
        result.hexString = result.hash.toHex();
        result.base64String = result.hash.toBase64();
    }
    
    return result;
}

HashUtils::HashResult HashUtils::hmac(const QString& text, const QString& key, HashAlgorithm algorithm)
{
    return hmac(text.toUtf8(), key.toUtf8(), algorithm);
}

bool HashUtils::verify(const QByteArray& data, const QByteArray& expectedHash, HashAlgorithm algorithm)
{
    HashResult result = hash(data, algorithm);
    return secureCompare(result.hash, expectedHash);
}

bool HashUtils::verifyFile(const QString& filePath, const QByteArray& expectedHash, HashAlgorithm algorithm)
{
    HashResult result = hashFile(filePath, algorithm);
    return secureCompare(result.hash, expectedHash);
}

QList<HashUtils::HashResult> HashUtils::hashBatch(const QList<QByteArray>& dataList, HashAlgorithm algorithm)
{
    QList<HashResult> results;
    
    for (const QByteArray& data : dataList) {
        results.append(hash(data, algorithm));
    }
    
    return results;
}

QList<HashUtils::HashResult> HashUtils::hashFilesBatch(const QStringList& filePaths, HashAlgorithm algorithm)
{
    QList<HashResult> results;
    
    for (const QString& filePath : filePaths) {
        results.append(hashFile(filePath, algorithm));
    }
    
    return results;
}

HashUtils::HashResult HashUtils::hashPassword(const QString& password, const QByteArray& salt, 
                                             int iterations, HashAlgorithm algorithm)
{
    QByteArray actualSalt = salt;
    if (actualSalt.isEmpty()) {
        actualSalt = generateSalt();
    }
    
    QElapsedTimer timer;
    timer.start();
    
    QByteArray derivedKey = performPBKDF2(password, actualSalt, iterations, getHashLength(algorithm), algorithm);
    
    HashResult result;
    result.hash = derivedKey;
    result.algorithm = algorithm;
    result.processingTime = timer.elapsed();
    result.hexString = result.hash.toHex();
    result.base64String = result.hash.toBase64();
    
    return result;
}

bool HashUtils::verifyPassword(const QString& password, const QByteArray& hashedPassword,
                              const QByteArray& salt, int iterations, HashAlgorithm algorithm)
{
    QByteArray derivedKey = performPBKDF2(password, salt, iterations, hashedPassword.length(), algorithm);
    return secureCompare(derivedKey, hashedPassword);
}

QByteArray HashUtils::generateSalt(int length)
{
    QByteArray salt;
    salt.resize(length);
    
    for (int i = 0; i < length; ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    
    return salt;
}

QString HashUtils::checksum(const QByteArray& data, HashAlgorithm algorithm)
{
    HashResult result = hash(data, algorithm);
    return result.hexString.toUpper();
}

QString HashUtils::checksumFile(const QString& filePath, HashAlgorithm algorithm)
{
    HashResult result = hashFile(filePath, algorithm);
    return result.hexString.toUpper();
}

QList<HashUtils::HashAlgorithm> HashUtils::supportedAlgorithms()
{
    return {MD4, MD5, SHA1, SHA224, SHA256, SHA384, SHA512, SHA3_224, SHA3_256, SHA3_384, SHA3_512};
}

bool HashUtils::isAlgorithmSupported(HashAlgorithm algorithm)
{
    return supportedAlgorithms().contains(algorithm);
}

int HashUtils::getHashLength(HashAlgorithm algorithm)
{
    switch (algorithm) {
        case MD4: return 16;
        case MD5: return 16;
        case SHA1: return 20;
        case SHA224: return 28;
        case SHA256: return 32;
        case SHA384: return 48;
        case SHA512: return 64;
        case SHA3_224: return 28;
        case SHA3_256: return 32;
        case SHA3_384: return 48;
        case SHA3_512: return 64;
        case BLAKE2B: return 64;
        case BLAKE2S: return 32;
        default: return 0;
    }
}

QString HashUtils::algorithmToString(HashAlgorithm algorithm)
{
    switch (algorithm) {
        case MD4: return "MD4";
        case MD5: return "MD5";
        case SHA1: return "SHA-1";
        case SHA224: return "SHA-224";
        case SHA256: return "SHA-256";
        case SHA384: return "SHA-384";
        case SHA512: return "SHA-512";
        case SHA3_224: return "SHA3-224";
        case SHA3_256: return "SHA3-256";
        case SHA3_384: return "SHA3-384";
        case SHA3_512: return "SHA3-512";
        case BLAKE2B: return "BLAKE2b";
        case BLAKE2S: return "BLAKE2s";
        default: return "Unknown";
    }
}

HashUtils::HashAlgorithm HashUtils::stringToAlgorithm(const QString& algorithmStr)
{
    QString str = algorithmStr.toUpper();
    
    if (str == "MD4") return MD4;
    if (str == "MD5") return MD5;
    if (str == "SHA-1" || str == "SHA1") return SHA1;
    if (str == "SHA-224" || str == "SHA224") return SHA224;
    if (str == "SHA-256" || str == "SHA256") return SHA256;
    if (str == "SHA-384" || str == "SHA384") return SHA384;
    if (str == "SHA-512" || str == "SHA512") return SHA512;
    if (str == "SHA3-224") return SHA3_224;
    if (str == "SHA3-256") return SHA3_256;
    if (str == "SHA3-384") return SHA3_384;
    if (str == "SHA3-512") return SHA3_512;
    if (str == "BLAKE2B") return BLAKE2B;
    if (str == "BLAKE2S") return BLAKE2S;
    
    return SHA256; // Default
}

QCryptographicHash::Algorithm HashUtils::toQtAlgorithm(HashAlgorithm algorithm)
{
    switch (algorithm) {
        case MD4: return QCryptographicHash::Md4;
        case MD5: return QCryptographicHash::Md5;
        case SHA1: return QCryptographicHash::Sha1;
        case SHA224: return QCryptographicHash::Sha224;
        case SHA256: return QCryptographicHash::Sha256;
        case SHA384: return QCryptographicHash::Sha384;
        case SHA512: return QCryptographicHash::Sha512;
        case SHA3_224: return QCryptographicHash::Sha3_224;
        case SHA3_256: return QCryptographicHash::Sha3_256;
        case SHA3_384: return QCryptographicHash::Sha3_384;
        case SHA3_512: return QCryptographicHash::Sha3_512;
        default: return static_cast<QCryptographicHash::Algorithm>(-1);
    }
}

HashUtils::HashAlgorithm HashUtils::fromQtAlgorithm(QCryptographicHash::Algorithm qtAlgorithm)
{
    switch (qtAlgorithm) {
        case QCryptographicHash::Md4: return MD4;
        case QCryptographicHash::Md5: return MD5;
        case QCryptographicHash::Sha1: return SHA1;
        case QCryptographicHash::Sha224: return SHA224;
        case QCryptographicHash::Sha256: return SHA256;
        case QCryptographicHash::Sha384: return SHA384;
        case QCryptographicHash::Sha512: return SHA512;
        case QCryptographicHash::Sha3_224: return SHA3_224;
        case QCryptographicHash::Sha3_256: return SHA3_256;
        case QCryptographicHash::Sha3_384: return SHA3_384;
        case QCryptographicHash::Sha3_512: return SHA3_512;
        default: return SHA256;
    }
}

bool HashUtils::secureCompare(const QByteArray& hash1, const QByteArray& hash2)
{
    if (hash1.length() != hash2.length()) {
        return false;
    }
    
    int result = 0;
    for (int i = 0; i < hash1.length(); ++i) {
        result |= hash1[i] ^ hash2[i];
    }
    
    return result == 0;
}

QString HashUtils::formatHash(const QByteArray& hash, const QString& format, bool uppercase)
{
    if (format.toLower() == "hex") {
        QString hexStr = hash.toHex();
        return uppercase ? hexStr.toUpper() : hexStr.toLower();
    } else if (format.toLower() == "base64") {
        return hash.toBase64();
    } else if (format.toLower() == "base32") {
        // Simple Base32 implementation
        const QString base32Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
        QString result;
        
        for (int i = 0; i < hash.length(); i += 5) {
            quint64 chunk = 0;
            int chunkSize = qMin(5, hash.length() - i);
            
            for (int j = 0; j < chunkSize; ++j) {
                chunk = (chunk << 8) | static_cast<unsigned char>(hash[i + j]);
            }
            
            for (int j = 0; j < (chunkSize * 8 + 4) / 5; ++j) {
                result.prepend(base32Chars[(chunk >> (j * 5)) & 0x1F]);
            }
        }
        
        return result;
    }
    
    return hash.toHex();
}

HashUtils::HashResult HashUtils::performHash(const QByteArray& data, HashAlgorithm algorithm)
{
    QMutexLocker locker(&s_mutex);
    
    HashResult result;
    result.algorithm = algorithm;
    
    QCryptographicHash::Algorithm qtAlgorithm = toQtAlgorithm(algorithm);
    if (qtAlgorithm == static_cast<QCryptographicHash::Algorithm>(-1)) {
        return result;
    }
    
    result.hash = QCryptographicHash::hash(data, qtAlgorithm);
    return result;
}

HashUtils::HashResult HashUtils::performHMAC(const QByteArray& data, const QByteArray& key, HashAlgorithm algorithm)
{
    QMutexLocker locker(&s_mutex);
    
    HashResult result;
    result.algorithm = algorithm;
    
    // HMAC implementation
    int blockSize = 64; // For most hash functions
    if (algorithm == SHA384 || algorithm == SHA512) {
        blockSize = 128;
    }
    
    QByteArray ipad(blockSize, 0x36);
    QByteArray opad(blockSize, 0x5c);
    
    QByteArray actualKey = key;
    if (actualKey.length() > blockSize) {
        HashResult keyHash = performHash(actualKey, algorithm);
        actualKey = keyHash.hash;
    }
    if (actualKey.length() < blockSize) {
        actualKey = actualKey.leftJustified(blockSize, 0);
    }
    
    for (int i = 0; i < blockSize; ++i) {
        ipad[i] = ipad[i] ^ actualKey[i];
        opad[i] = opad[i] ^ actualKey[i];
    }
    
    HashResult innerHash = performHash(ipad + data, algorithm);
    result = performHash(opad + innerHash.hash, algorithm);
    
    return result;
}

QByteArray HashUtils::performPBKDF2(const QString& password, const QByteArray& salt,
                                   int iterations, int keyLength, HashAlgorithm algorithm)
{
    QByteArray passwordBytes = password.toUtf8();
    QByteArray result;
    
    for (int i = 1; result.length() < keyLength; ++i) {
        QByteArray u = salt + QByteArray::number(i);
        QByteArray f;
        
        for (int j = 0; j < iterations; ++j) {
            HashResult temp = performHMAC(u, passwordBytes, algorithm);
            u = temp.hash;
            
            if (j == 0) {
                f = temp.hash;
            } else {
                for (int k = 0; k < temp.hash.length() && k < f.length(); ++k) {
                    f[k] = f[k] ^ temp.hash[k];
                }
            }
        }
        
        result.append(f);
    }
    
    return result.left(keyLength);
}