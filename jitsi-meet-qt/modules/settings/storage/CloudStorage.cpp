#include "CloudStorage.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QCryptographicHash>
#include <QDebug>

class CloudStorage::Private
{
public:
    QString serverUrl;
    QString authToken;
    ConnectionStatus connectionStatus;
    SyncStatus syncStatus;
    ConflictResolution conflictResolution;
    CloudProvider cloudProvider;
    
    bool autoSyncEnabled;
    int syncInterval;
    bool offlineMode;
    bool offlineCacheEnabled;
    int requestTimeout;
    int maxRetries;
    
    QVariantMap localData;
    QVariantMap remoteData;
    QVariantMap conflictData;
    QStringList conflictKeys;
    
    QNetworkAccessManager* networkManager;
    QTimer* syncTimer;
    QMutex dataMutex;
    
    // Statistics
    QVariantMap statistics;
    QVariantMap networkUsage;
    QDateTime lastSyncTime;
    
    // Cache
    QString cacheDir;
    qint64 cacheSize;
    
    Private()
        : connectionStatus(Disconnected)
        , syncStatus(NotSynced)
        , conflictResolution(AskUser)
        , cloudProvider(CustomProvider)
        , autoSyncEnabled(false)
        , syncInterval(300) // 5 minutes
        , offlineMode(false)
        , offlineCacheEnabled(true)
        , requestTimeout(30000) // 30 seconds
        , maxRetries(3)
        , networkManager(nullptr)
        , syncTimer(nullptr)
        , cacheSize(0)
    {
        statistics["syncs"] = 0;
        statistics["uploads"] = 0;
        statistics["downloads"] = 0;
        statistics["conflicts"] = 0;
        statistics["errors"] = 0;
        
        networkUsage["bytesUploaded"] = 0;
        networkUsage["bytesDownloaded"] = 0;
        networkUsage["requests"] = 0;
        
        // Setup cache directory
        cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/CloudStorage";
        QDir().mkpath(cacheDir);
    }
};

CloudStorage::CloudStorage(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
}

CloudStorage::CloudStorage(const QString& serverUrl, const QString& authToken, QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->serverUrl = serverUrl;
    d->authToken = authToken;
}

CloudStorage::~CloudStorage()
{
    if (d->syncTimer) {
        d->syncTimer->stop();
        d->syncTimer->deleteLater();
    }
    
    if (d->networkManager) {
        d->networkManager->deleteLater();
    }
}

bool CloudStorage::initialize()
{
    if (d->serverUrl.isEmpty()) {
        setConnectionStatus(Error);
        emit errorOccurred("Server URL is empty");
        return false;
    }
    
    // Initialize network manager
    if (!d->networkManager) {
        d->networkManager = new QNetworkAccessManager(this);
        // Note: networkAccessibleChanged was removed in Qt 6
        // We'll use a different approach to monitor network status
    }
    
    // Setup sync timer
    if (d->autoSyncEnabled && !d->syncTimer) {
        d->syncTimer = new QTimer(this);
        QObject::connect(d->syncTimer, &QTimer::timeout, this, &CloudStorage::onSyncTimer);
        d->syncTimer->start(d->syncInterval * 1000);
    }
    
    // Load cached data if offline cache is enabled
    if (d->offlineCacheEnabled) {
        // loadLocalCache(); // Method will be implemented below
    }
    
    // Try to connect
    if (!d->offlineMode) {
        return connect();
    }
    
    setConnectionStatus(Disconnected);
    setOfflineMode(true);
    return true;
}

QString CloudStorage::serverUrl() const
{
    return d->serverUrl;
}

void CloudStorage::setServerUrl(const QString& url)
{
    if (d->serverUrl != url) {
        d->serverUrl = url;
        emit serverUrlChanged(url);
        
        // Reconnect if we were connected
        if (d->connectionStatus == Connected) {
            reconnect();
        }
    }
}

QString CloudStorage::authToken() const
{
    return d->authToken;
}

void CloudStorage::setAuthToken(const QString& token)
{
    if (d->authToken != token) {
        d->authToken = token;
        emit authTokenChanged(token);
    }
}

bool CloudStorage::isAutoSyncEnabled() const
{
    return d->autoSyncEnabled;
}

void CloudStorage::setAutoSyncEnabled(bool enabled)
{
    if (d->autoSyncEnabled != enabled) {
        d->autoSyncEnabled = enabled;
        emit autoSyncChanged(enabled);
        
        if (enabled && !d->syncTimer) {
            d->syncTimer = new QTimer(this);
            QObject::connect(d->syncTimer, &QTimer::timeout, this, &CloudStorage::onSyncTimer);
            d->syncTimer->start(d->syncInterval * 1000);
        } else if (!enabled && d->syncTimer) {
            d->syncTimer->stop();
            d->syncTimer->deleteLater();
            d->syncTimer = nullptr;
        }
    }
}

int CloudStorage::syncInterval() const
{
    return d->syncInterval;
}

void CloudStorage::setSyncInterval(int interval)
{
    if (d->syncInterval != interval) {
        d->syncInterval = interval;
        emit syncIntervalChanged(interval);
        
        if (d->syncTimer && d->syncTimer->isActive()) {
            d->syncTimer->setInterval(interval * 1000);
        }
    }
}

bool CloudStorage::isOfflineMode() const
{
    return d->offlineMode;
}

CloudStorage::ConnectionStatus CloudStorage::connectionStatus() const
{
    return d->connectionStatus;
}

bool CloudStorage::connect()
{
    if (d->serverUrl.isEmpty()) {
        emit errorOccurred("Server URL is empty");
        return false;
    }
    
    setConnectionStatus(Connecting);
    
    // Test connection with a simple ping
    QNetworkRequest request = createRequest("/ping");
    QNetworkReply* reply = d->networkManager->get(request);
    
    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            setConnectionStatus(Connected);
            setOfflineMode(false);
            emit authenticated();
            
            // Start initial sync
            if (d->autoSyncEnabled) {
                scheduleSync();
            }
        } else {
            setConnectionStatus(Error);
            setOfflineMode(true);
            emit networkError(reply->errorString());
        }
        
        reply->deleteLater();
    });
    
    return true;
}

void CloudStorage::disconnect()
{
    setConnectionStatus(Disconnected);
    setOfflineMode(true);
    
    if (d->syncTimer) {
        d->syncTimer->stop();
    }
}

bool CloudStorage::authenticate(const QString& username, const QString& password)
{
    if (d->serverUrl.isEmpty()) {
        emit authenticationFailed("Server URL is empty");
        return false;
    }
    
    QNetworkRequest request = createRequest("/auth/login");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject authData;
    authData["username"] = username;
    authData["password"] = password;
    
    QJsonDocument doc(authData);
    QNetworkReply* reply = d->networkManager->post(request, doc.toJson());
    
    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = response.object();
            
            if (obj.contains("token")) {
                setAuthToken(obj["token"].toString());
                setConnectionStatus(Connected);
                emit authenticated();
            } else {
                emit authenticationFailed("Invalid response format");
            }
        } else {
            emit authenticationFailed(reply->errorString());
        }
        
        reply->deleteLater();
    });
    
    return true;
}

bool CloudStorage::authenticateOAuth(const QString& oauthToken)
{
    setAuthToken(oauthToken);
    return connect();
}

bool CloudStorage::refreshToken()
{
    if (d->authToken.isEmpty()) {
        return false;
    }
    
    QNetworkRequest request = createRequest("/auth/refresh");
    QNetworkReply* reply = d->networkManager->post(request, QByteArray());
    
    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = response.object();
            
            if (obj.contains("token")) {
                setAuthToken(obj["token"].toString());
            }
        }
        
        reply->deleteLater();
    });
    
    return true;
}

bool CloudStorage::isAuthenticated() const
{
    return !d->authToken.isEmpty() && d->connectionStatus == Connected;
}

void CloudStorage::setValue(const QString& key, const QVariant& value)
{
    QMutexLocker locker(&d->dataMutex);
    
    QVariant oldValue = d->localData.value(key);
    if (oldValue != value) {
        d->localData[key] = value;
        
        // Update cache if enabled
        if (d->offlineCacheEnabled) {
            updateLocalCache(QJsonObject::fromVariantMap(d->localData));
        }
        
        emit dataChanged(key, value);
        
        // Schedule sync if auto-sync is enabled
        if (d->autoSyncEnabled && !d->offlineMode) {
            scheduleSync();
        }
    }
}

QVariant CloudStorage::value(const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&d->dataMutex);
    // updateStatistics("read"); // Cannot call non-const method from const method
    
    return d->localData.value(key, defaultValue);
}

bool CloudStorage::contains(const QString& key) const
{
    QMutexLocker locker(&d->dataMutex);
    return d->localData.contains(key);
}

void CloudStorage::remove(const QString& key)
{
    QMutexLocker locker(&d->dataMutex);
    
    if (d->localData.contains(key)) {
        d->localData.remove(key);
        
        // Update cache if enabled
        if (d->offlineCacheEnabled) {
            updateLocalCache(QJsonObject::fromVariantMap(d->localData));
        }
        
        emit dataChanged(key, QVariant());
        
        // Schedule sync if auto-sync is enabled
        if (d->autoSyncEnabled && !d->offlineMode) {
            scheduleSync();
        }
    }
}

QStringList CloudStorage::allKeys() const
{
    QMutexLocker locker(&d->dataMutex);
    return d->localData.keys();
}

void CloudStorage::clear()
{
    QMutexLocker locker(&d->dataMutex);
    
    if (!d->localData.isEmpty()) {
        d->localData.clear();
        
        // Update cache if enabled
        if (d->offlineCacheEnabled) {
            updateLocalCache(QJsonObject());
        }
        
        emit dataChanged(QString(), QVariant());
        
        // Schedule sync if auto-sync is enabled
        if (d->autoSyncEnabled && !d->offlineMode) {
            scheduleSync();
        }
    }
}

// Sync operations
bool CloudStorage::syncToCloud()
{
    if (d->offlineMode || d->connectionStatus != Connected) {
        return false;
    }
    
    setConnectionStatus(Syncing);
    emit syncStarted();
    
    QNetworkRequest request = createRequest("/data");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QMutexLocker locker(&d->dataMutex);
    QJsonDocument doc(QJsonObject::fromVariantMap(d->localData));
    QByteArray data = doc.toJson();
    
    QNetworkReply* reply = d->networkManager->put(request, data);
    
    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        bool success = (reply->error() == QNetworkReply::NoError);
        
        if (success) {
            d->lastSyncTime = QDateTime::currentDateTime();
            updateStatistics("upload", reply->request().header(QNetworkRequest::ContentLengthHeader).toLongLong());
        } else {
            emit networkError(reply->errorString());
        }
        
        setConnectionStatus(Connected);
        emit syncCompleted(success);
        reply->deleteLater();
    });
    
    return true;
}

bool CloudStorage::syncFromCloud()
{
    if (d->offlineMode || d->connectionStatus != Connected) {
        return false;
    }
    
    setConnectionStatus(Syncing);
    emit syncStarted();
    
    QNetworkRequest request = createRequest("/data");
    QNetworkReply* reply = d->networkManager->get(request);
    
    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        bool success = (reply->error() == QNetworkReply::NoError);
        
        if (success) {
            QByteArray data = reply->readAll();
            success = processResponse(data);
            
            if (success) {
                d->lastSyncTime = QDateTime::currentDateTime();
                updateStatistics("download", data.size());
            }
        } else {
            emit networkError(reply->errorString());
        }
        
        setConnectionStatus(Connected);
        emit syncCompleted(success);
        reply->deleteLater();
    });
    
    return true;
}

bool CloudStorage::bidirectionalSync()
{
    if (d->offlineMode || d->connectionStatus != Connected) {
        return false;
    }
    
    // First, get remote data
    if (!syncFromCloud()) {
        return false;
    }
    
    // Then, detect conflicts and resolve them
    detectConflicts(QJsonObject::fromVariantMap(d->remoteData));
    
    // Finally, upload merged data
    return syncToCloud();
}

CloudStorage::SyncStatus CloudStorage::syncStatus(const QString& key) const
{
    if (key.isEmpty()) {
        return d->syncStatus;
    }
    
    // Check if specific key has conflicts
    if (d->conflictKeys.contains(key)) {
        return Conflict;
    }
    
    // Check if key exists in both local and remote
    if (d->localData.contains(key) && d->remoteData.contains(key)) {
        if (d->localData[key] == d->remoteData[key]) {
            return Synced;
        } else {
            return Pending;
        }
    }
    
    return NotSynced;
}

QDateTime CloudStorage::lastSyncTime() const
{
    return d->lastSyncTime;
}

bool CloudStorage::hasPendingChanges() const
{
    // Compare local and remote data
    return d->localData != d->remoteData;
}

// Private helper methods
void CloudStorage::setConnectionStatus(ConnectionStatus status)
{
    if (d->connectionStatus != status) {
        d->connectionStatus = status;
        emit connectionStatusChanged(status);
    }
}

void CloudStorage::setOfflineMode(bool offline)
{
    if (d->offlineMode != offline) {
        d->offlineMode = offline;
        emit offlineModeChanged(offline);
    }
}

QNetworkRequest CloudStorage::createRequest(const QString& endpoint) const
{
    QUrl url(d->serverUrl + endpoint);
    QNetworkRequest request(url);
    
    // Set headers
    request.setRawHeader("User-Agent", "JitsiMeet-Qt/1.0");
    request.setRawHeader("Accept", "application/json");
    
    if (!d->authToken.isEmpty()) {
        request.setRawHeader("Authorization", "Bearer " + d->authToken.toUtf8());
    }
    
    // Set timeout
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(d->requestTimeout);
    
    return request;
}

bool CloudStorage::processResponse(const QByteArray& data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        emit errorOccurred("JSON parse error: " + error.errorString());
        return false;
    }
    
    QJsonObject obj = doc.object();
    QMutexLocker locker(&d->dataMutex);
    
    d->remoteData = obj.toVariantMap();
    
    // Detect conflicts
    detectConflicts(obj);
    
    // Update cache
    if (d->offlineCacheEnabled) {
        updateLocalCache(obj);
    }
    
    return true;
}

void CloudStorage::detectConflicts(const QJsonObject& remoteData)
{
    d->conflictKeys.clear();
    
    QVariantMap remote = remoteData.toVariantMap();
    
    for (auto it = d->localData.constBegin(); it != d->localData.constEnd(); ++it) {
        const QString& key = it.key();
        const QVariant& localValue = it.value();
        
        if (remote.contains(key)) {
            const QVariant& remoteValue = remote[key];
            
            if (localValue != remoteValue) {
                d->conflictKeys.append(key);
                emit conflictDetected(key, localValue, remoteValue);
            }
        }
    }
    
    if (!d->conflictKeys.isEmpty()) {
        updateStatistics("conflicts", d->conflictKeys.size());
    }
}

/**
 * @brief 处理网络回复
 * @param reply 网络回复对象
 */
void CloudStorage::handleNetworkReply(QNetworkReply* reply)
{
    if (!reply) {
        return;
    }

    // 检查网络错误
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = QString("网络请求失败: %1").arg(reply->errorString());
        emit networkError(errorMsg);
        emit errorOccurred(errorMsg);
        
        // 更新统计信息
        updateStatistics("errors");
        return;
    }

    // 读取响应数据
    QByteArray responseData = reply->readAll();
    
    // 更新网络使用统计
    updateStatistics("requests");
    updateStatistics("download", responseData.size());
    
    // 处理响应数据
    if (!responseData.isEmpty()) {
        processResponse(responseData);
    }
}

void CloudStorage::updateLocalCache(const QJsonObject& data)
{
    QString cacheFile = d->cacheDir + "/cache.json";
    
    QFile file(cacheFile);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(data);
        qint64 written = file.write(doc.toJson());
        file.close();
        
        d->cacheSize = written;
    }
}

void CloudStorage::loadLocalCache()
{
    QString cacheFile = d->cacheDir + "/cache.json";
    
    QFile file(cacheFile);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error == QJsonParseError::NoError) {
            QMutexLocker locker(&d->dataMutex);
            d->localData = doc.object().toVariantMap();
        }
        
        d->cacheSize = data.size();
    }
}

void CloudStorage::scheduleSync()
{
    // Implement debounced sync scheduling
    if (d->syncTimer && !d->syncTimer->isActive()) {
        d->syncTimer->start(1000); // Sync after 1 second of inactivity
    }
}

void CloudStorage::updateStatistics(const QString& operation, qint64 bytes)
{
    d->statistics[operation] = d->statistics[operation].toInt() + 1;
    
    if (bytes > 0) {
        if (operation == "upload") {
            d->networkUsage["bytesUploaded"] = d->networkUsage["bytesUploaded"].toLongLong() + bytes;
        } else if (operation == "download") {
            d->networkUsage["bytesDownloaded"] = d->networkUsage["bytesDownloaded"].toLongLong() + bytes;
        }
    }
}

// Slot implementations
/**
 * @brief 网络回复完成处理槽函数
 */
void CloudStorage::onNetworkReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    handleNetworkReply(reply);
    reply->deleteLater();
}

void CloudStorage::onSyncTimer()
{
    if (d->autoSyncEnabled && !d->offlineMode && d->connectionStatus == Connected) {
        bidirectionalSync();
    }
}

void CloudStorage::onNetworkAccessibleChanged(bool accessible)
{
    bool wasOffline = d->offlineMode;
    bool isOffline = !accessible; // Simplified since Accessible enum was removed in Qt 6
    
    if (wasOffline != isOffline) {
        setOfflineMode(isOffline);
        
        if (!isOffline && wasOffline) {
            // Coming back online, try to reconnect
            connect();
        }
    }
}

// Public slot implementations
void CloudStorage::forceSync()
{
    if (!d->offlineMode) {
        bidirectionalSync();
    }
}

void CloudStorage::reconnect()
{
    disconnect();
    connect();
}

void CloudStorage::goOffline()
{
    setOfflineMode(true);
    setConnectionStatus(Disconnected);
}

void CloudStorage::goOnline()
{
    setOfflineMode(false);
    connect();
}

void CloudStorage::refresh()
{
    if (!d->offlineMode) {
        syncFromCloud();
    }
}

// Additional getters and setters
void CloudStorage::setConflictResolution(ConflictResolution strategy)
{
    d->conflictResolution = strategy;
}

CloudStorage::ConflictResolution CloudStorage::conflictResolution() const
{
    return d->conflictResolution;
}

QStringList CloudStorage::conflicts() const
{
    return d->conflictKeys;
}

void CloudStorage::setOfflineCacheEnabled(bool enabled)
{
    d->offlineCacheEnabled = enabled;
    
    if (enabled) {
        // Save current data to cache
        updateLocalCache(QJsonObject::fromVariantMap(d->localData));
    }
}

bool CloudStorage::isOfflineCacheEnabled() const
{
    return d->offlineCacheEnabled;
}

qint64 CloudStorage::cacheSize() const
{
    return d->cacheSize;
}

void CloudStorage::clearCache()
{
    QDir cacheDir(d->cacheDir);
    cacheDir.removeRecursively();
    cacheDir.mkpath(".");
    d->cacheSize = 0;
}

void CloudStorage::compactCache()
{
    // Rewrite cache file to remove fragmentation
    updateLocalCache(QJsonObject::fromVariantMap(d->localData));
}

QVariantMap CloudStorage::statistics() const
{
    QVariantMap stats = d->statistics;
    stats["lastSyncTime"] = d->lastSyncTime;
    stats["conflictCount"] = d->conflictKeys.size();
    stats["cacheSize"] = d->cacheSize;
    return stats;
}

QVariantMap CloudStorage::networkUsage() const
{
    return d->networkUsage;
}

void CloudStorage::resetStatistics()
{
    d->statistics.clear();
    d->networkUsage.clear();
    
    d->statistics["syncs"] = 0;
    d->statistics["uploads"] = 0;
    d->statistics["downloads"] = 0;
    d->statistics["conflicts"] = 0;
    d->statistics["errors"] = 0;
    
    d->networkUsage["bytesUploaded"] = 0;
    d->networkUsage["bytesDownloaded"] = 0;
    d->networkUsage["requests"] = 0;
}

QJsonObject CloudStorage::exportToJson() const
{
    QMutexLocker locker(&d->dataMutex);
    return QJsonObject::fromVariantMap(d->localData);
}

bool CloudStorage::importFromJson(const QJsonObject& json, bool merge)
{
    QMutexLocker locker(&d->dataMutex);
    
    QVariantMap importedData = json.toVariantMap();
    
    if (merge) {
        for (auto it = importedData.constBegin(); it != importedData.constEnd(); ++it) {
            d->localData[it.key()] = it.value();
        }
    } else {
        d->localData = importedData;
    }
    
    // Update cache if enabled
    if (d->offlineCacheEnabled) {
        updateLocalCache(QJsonObject::fromVariantMap(d->localData));
    }
    
    // Schedule sync if auto-sync is enabled
    if (d->autoSyncEnabled && !d->offlineMode) {
        scheduleSync();
    }
    
    return true;
}