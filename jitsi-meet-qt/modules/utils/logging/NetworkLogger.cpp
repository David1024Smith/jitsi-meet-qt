#include "NetworkLogger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QSslConfiguration>
#include <QAuthenticator>
#include <QDebug>

NetworkLogger::NetworkLogger(const NetworkConfig& config, QObject* parent)
    : ILogger(parent)
    , m_logLevel(Info)
    , m_format("{timestamp} [{level}] {category}: {message}")
    , m_enabled(true)
    , m_config(config)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_batchTimer(new QTimer(this))
    , m_batchInterval(5000) // 5秒
    , m_connected(false)
{
    // 设置网络管理器
    m_networkManager->setTransferTimeout(m_config.timeout);
    
    // 连接信号
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &NetworkLogger::onRequestFinished);
    
    // 设置批量发送定时器
    m_batchTimer->setSingleShot(false);
    connect(m_batchTimer, &QTimer::timeout, this, &NetworkLogger::onBatchTimer);
}

NetworkLogger::~NetworkLogger()
{
    cleanup();
}

bool NetworkLogger::initialize()
{
    QMutexLocker locker(&m_bufferMutex);
    
    // 测试网络连接
    if (!testConnection()) {
        emit errorOccurred("Failed to connect to log server");
        return false;
    }
    
    m_connected = true;
    
    // 启动批量发送定时器
    if (m_batchInterval > 0 && m_config.batchSize > 1) {
        m_batchTimer->start(m_batchInterval);
    }
    
    return true;
}

void NetworkLogger::cleanup()
{
    QMutexLocker locker(&m_bufferMutex);
    
    // 停止定时器
    if (m_batchTimer) {
        m_batchTimer->stop();
    }
    
    // 发送剩余的日志
    if (!m_logBuffer.isEmpty()) {
        sendBatch();
    }
    
    // 等待所有请求完成
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
        QNetworkReply* reply = it.key();
        if (reply && !reply->isFinished()) {
            reply->abort();
        }
    }
    
    m_pendingRequests.clear();
    m_retryCount.clear();
    m_connected = false;
}

void NetworkLogger::log(const LogEntry& entry)
{
    if (!shouldLog(entry.level) || !m_enabled) {
        return;
    }
    
    QMutexLocker locker(&m_bufferMutex);
    
    // 添加到缓冲区
    addToBuffer(entry);
    
    // 如果不使用批量发送或缓冲区已满，立即发送
    if (m_config.batchSize <= 1 || m_logBuffer.size() >= m_config.batchSize) {
        sendBatch();
    }
    
    // 发出信号
    emit logRecorded(entry);
}

void NetworkLogger::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_bufferMutex);
    m_logLevel = level;
}

ILogger::LogLevel NetworkLogger::logLevel() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_logLevel;
}

void NetworkLogger::setFormat(const QString& format)
{
    QMutexLocker locker(&m_bufferMutex);
    m_format = format;
}

QString NetworkLogger::format() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_format;
}

bool NetworkLogger::isEnabled() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_enabled;
}

void NetworkLogger::setEnabled(bool enabled)
{
    QMutexLocker locker(&m_bufferMutex);
    m_enabled = enabled;
}

QString NetworkLogger::name() const
{
    return "NetworkLogger";
}

void NetworkLogger::flush()
{
    QMutexLocker locker(&m_bufferMutex);
    
    if (!m_logBuffer.isEmpty()) {
        sendBatch();
    }
}

void NetworkLogger::setNetworkConfig(const NetworkConfig& config)
{
    QMutexLocker locker(&m_bufferMutex);
    
    m_config = config;
    m_networkManager->setTransferTimeout(m_config.timeout);
}

NetworkLogger::NetworkConfig NetworkLogger::networkConfig() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_config;
}

void NetworkLogger::setBatchInterval(int interval)
{
    QMutexLocker locker(&m_bufferMutex);
    
    m_batchInterval = interval;
    
    if (m_batchTimer) {
        if (interval > 0 && m_config.batchSize > 1) {
            m_batchTimer->start(interval);
        } else {
            m_batchTimer->stop();
        }
    }
}

int NetworkLogger::batchInterval() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_batchInterval;
}

void NetworkLogger::setConnectionTimeout(int timeout)
{
    QMutexLocker locker(&m_bufferMutex);
    
    m_config.timeout = timeout;
    m_networkManager->setTransferTimeout(timeout);
}

int NetworkLogger::connectionTimeout() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_config.timeout;
}

bool NetworkLogger::isConnected() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_connected;
}

int NetworkLogger::bufferedLogCount() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_logBuffer.size();
}

QJsonObject NetworkLogger::getStatistics() const
{
    QMutexLocker locker(&m_statisticsMutex);
    
    QJsonObject stats;
    stats["totalSent"] = static_cast<qint64>(m_statistics.totalSent);
    stats["totalFailed"] = static_cast<qint64>(m_statistics.totalFailed);
    stats["totalBytes"] = static_cast<qint64>(m_statistics.totalBytes);
    stats["lastSent"] = m_statistics.lastSent.toString(Qt::ISODate);
    stats["averageLatency"] = m_statistics.averageLatency;
    stats["bufferedCount"] = bufferedLogCount();
    stats["connected"] = m_connected;
    
    return stats;
}

void NetworkLogger::clearBuffer()
{
    QMutexLocker locker(&m_bufferMutex);
    m_logBuffer.clear();
}

bool NetworkLogger::testConnection()
{
    // 创建测试请求
    QNetworkRequest request;
    request.setUrl(QUrl(buildServerUrl()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    // 添加认证信息
    if (!m_config.apiKey.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_config.apiKey).toUtf8());
    } else if (!m_config.username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_config.username, m_config.password);
        QByteArray encodedCredentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", QString("Basic %1").arg(QString::fromUtf8(encodedCredentials)).toUtf8());
    }
    
    // 发送HEAD请求测试连接
    QNetworkReply* reply = m_networkManager->head(request);
    
    // 等待响应（简化处理）
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(m_config.timeout, &loop, &QEventLoop::quit);
    loop.exec();
    
    bool success = (reply->error() == QNetworkReply::NoError);
    reply->deleteLater();
    
    return success;
}

void NetworkLogger::onBatchTimer()
{
    QMutexLocker locker(&m_bufferMutex);
    
    if (!m_logBuffer.isEmpty()) {
        sendBatch();
    }
}

void NetworkLogger::onRequestFinished(QNetworkReply* reply)
{
    if (!m_pendingRequests.contains(reply)) {
        reply->deleteLater();
        return;
    }
    
    QByteArray requestData = m_pendingRequests.take(reply);
    int retryCount = m_retryCount.take(reply);
    
    bool success = (reply->error() == QNetworkReply::NoError);
    
    if (!success && retryCount < m_config.maxRetries) {
        // 重试请求
        retryRequest(requestData, retryCount + 1);
    } else {
        // 更新统计信息
        int logCount = 1; // 简化处理，假设每个请求包含1条日志
        updateStatistics(success, logCount);
        
        if (!success) {
            emit errorOccurred(QString("Network log request failed: %1").arg(reply->errorString()));
        }
    }
    
    reply->deleteLater();
}

void NetworkLogger::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    // 错误处理在onRequestFinished中进行
}

void NetworkLogger::addToBuffer(const LogEntry& entry)
{
    // 检查缓冲区大小限制
    if (m_logBuffer.size() >= m_config.bufferSize) {
        // 移除最旧的条目
        m_logBuffer.dequeue();
    }
    
    m_logBuffer.enqueue(entry);
}

void NetworkLogger::sendBatch()
{
    if (m_logBuffer.isEmpty() || !m_connected) {
        return;
    }
    
    // 获取要发送的日志条目
    QList<LogEntry> entries;
    int batchSize = qMin(m_config.batchSize, m_logBuffer.size());
    
    for (int i = 0; i < batchSize; ++i) {
        entries.append(m_logBuffer.dequeue());
    }
    
    // 格式化数据
    QByteArray data = formatBatchEntries(entries);
    
    // 压缩数据（如果启用）
    if (m_config.compressionEnabled) {
        data = compressData(data);
    }
    
    // 加密数据（如果启用）
    if (m_config.encryptionEnabled) {
        data = encryptData(data);
    }
    
    // 创建网络请求
    QNetworkRequest request = createRequest(data);
    
    // 发送请求
    QNetworkReply* reply = m_networkManager->post(request, data);
    
    // 记录待处理请求
    m_pendingRequests[reply] = data;
    m_retryCount[reply] = 0;
    
    // 连接错误信号
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &NetworkLogger::onNetworkError);
}

void NetworkLogger::sendLogEntry(const LogEntry& entry)
{
    QList<LogEntry> entries;
    entries.append(entry);
    
    QByteArray data = formatBatchEntries(entries);
    
    if (m_config.compressionEnabled) {
        data = compressData(data);
    }
    
    if (m_config.encryptionEnabled) {
        data = encryptData(data);
    }
    
    QNetworkRequest request = createRequest(data);
    QNetworkReply* reply = m_networkManager->post(request, data);
    
    m_pendingRequests[reply] = data;
    m_retryCount[reply] = 0;
    
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &NetworkLogger::onNetworkError);
}

QNetworkRequest NetworkLogger::createRequest(const QByteArray& data)
{
    QNetworkRequest request;
    request.setUrl(QUrl(buildServerUrl()));
    
    // 设置内容类型
    switch (m_config.format) {
        case JSON:
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            break;
        case XML:
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
            break;
        case PlainText:
            request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
            break;
    }
    
    // 设置内容长度
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.size());
    
    // 添加认证信息
    if (!m_config.apiKey.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_config.apiKey).toUtf8());
    } else if (!m_config.username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_config.username, m_config.password);
        QByteArray encodedCredentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", QString("Basic %1").arg(QString::fromUtf8(encodedCredentials)).toUtf8());
    }
    
    // 设置压缩头
    if (m_config.compressionEnabled) {
        request.setRawHeader("Content-Encoding", "gzip");
    }
    
    // 设置用户代理
    request.setHeader(QNetworkRequest::UserAgentHeader, "JitsiMeetQt-NetworkLogger/1.0");
    
    return request;
}

QByteArray NetworkLogger::formatLogEntry(const LogEntry& entry) const
{
    switch (m_config.format) {
        case JSON: {
            QJsonObject obj;
            obj["timestamp"] = entry.timestamp.toString(Qt::ISODate);
            obj["level"] = levelToString(entry.level);
            obj["category"] = entry.category;
            obj["message"] = entry.message;
            obj["thread"] = entry.thread;
            obj["file"] = entry.file;
            obj["line"] = entry.line;
            
            QJsonDocument doc(obj);
            return doc.toJson(QJsonDocument::Compact);
        }
        case XML: {
            QString xml = QString("<log>"
                                "<timestamp>%1</timestamp>"
                                "<level>%2</level>"
                                "<category>%3</category>"
                                "<message>%4</message>"
                                "<thread>%5</thread>"
                                "<file>%6</file>"
                                "<line>%7</line>"
                                "</log>")
                         .arg(entry.timestamp.toString(Qt::ISODate))
                         .arg(levelToString(entry.level))
                         .arg(entry.category)
                         .arg(entry.message)
                         .arg(entry.thread)
                         .arg(entry.file)
                         .arg(entry.line);
            return xml.toUtf8();
        }
        case PlainText:
        default:
            return formatEntry(entry, m_format).toUtf8();
    }
}

QByteArray NetworkLogger::formatBatchEntries(const QList<LogEntry>& entries) const
{
    switch (m_config.format) {
        case JSON: {
            QJsonArray array;
            for (const LogEntry& entry : entries) {
                QJsonObject obj;
                obj["timestamp"] = entry.timestamp.toString(Qt::ISODate);
                obj["level"] = levelToString(entry.level);
                obj["category"] = entry.category;
                obj["message"] = entry.message;
                obj["thread"] = entry.thread;
                obj["file"] = entry.file;
                obj["line"] = entry.line;
                array.append(obj);
            }
            
            QJsonDocument doc(array);
            return doc.toJson(QJsonDocument::Compact);
        }
        case XML: {
            QString xml = "<logs>";
            for (const LogEntry& entry : entries) {
                xml += QString("<log>"
                             "<timestamp>%1</timestamp>"
                             "<level>%2</level>"
                             "<category>%3</category>"
                             "<message>%4</message>"
                             "<thread>%5</thread>"
                             "<file>%6</file>"
                             "<line>%7</line>"
                             "</log>")
                      .arg(entry.timestamp.toString(Qt::ISODate))
                      .arg(levelToString(entry.level))
                      .arg(entry.category)
                      .arg(entry.message)
                      .arg(entry.thread)
                      .arg(entry.file)
                      .arg(entry.line);
            }
            xml += "</logs>";
            return xml.toUtf8();
        }
        case PlainText:
        default: {
            QString text;
            for (const LogEntry& entry : entries) {
                text += formatEntry(entry, m_format) + "\n";
            }
            return text.toUtf8();
        }
    }
}

QByteArray NetworkLogger::compressData(const QByteArray& data) const
{
    // 简化实现，实际应该使用zlib或其他压缩库
    return qCompress(data);
}

QByteArray NetworkLogger::encryptData(const QByteArray& data) const
{
    // 简化实现，实际应该使用适当的加密算法
    // 这里只是示例，不提供真正的加密
    return data;
}

void NetworkLogger::retryRequest(const QByteArray& data, int retryCount)
{
    if (retryCount >= m_config.maxRetries) {
        updateStatistics(false, 1);
        return;
    }
    
    // 延迟重试
    QTimer::singleShot(1000 * retryCount, [this, data, retryCount]() {
        QNetworkRequest request = createRequest(data);
        QNetworkReply* reply = m_networkManager->post(request, data);
        
        m_pendingRequests[reply] = data;
        m_retryCount[reply] = retryCount;
        
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                this, &NetworkLogger::onNetworkError);
    });
}

QString NetworkLogger::buildServerUrl() const
{
    QString protocol = (m_config.protocol == HTTPS) ? "https" : "http";
    QString url = QString("%1://%2:%3%4")
                 .arg(protocol)
                 .arg(m_config.serverUrl)
                 .arg(m_config.port)
                 .arg(m_config.endpoint);
    return url;
}

void NetworkLogger::updateStatistics(bool success, int logCount)
{
    QMutexLocker locker(&m_statisticsMutex);
    
    if (success) {
        m_statistics.totalSent += logCount;
        m_statistics.lastSent = QDateTime::currentDateTime();
    } else {
        m_statistics.totalFailed += logCount;
    }
}