#ifndef CLOUDSTORAGE_H
#define CLOUDSTORAGE_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QMutex>
#include <memory>

/**
 * @brief 云端存储后端类
 * 
 * 提供云端设置存储和同步功能，支持多种云服务提供商。
 * 包含离线缓存、冲突解决、增量同步等高级功能。
 */
class CloudStorage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(QString authToken READ authToken WRITE setAuthToken NOTIFY authTokenChanged)
    Q_PROPERTY(bool autoSync READ isAutoSyncEnabled WRITE setAutoSyncEnabled NOTIFY autoSyncChanged)
    Q_PROPERTY(int syncInterval READ syncInterval WRITE setSyncInterval NOTIFY syncIntervalChanged)
    Q_PROPERTY(bool offlineMode READ isOfflineMode NOTIFY offlineModeChanged)
    Q_PROPERTY(ConnectionStatus connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)

public:
    /**
     * @brief 连接状态枚举
     */
    enum ConnectionStatus {
        Disconnected,   ///< 未连接
        Connecting,     ///< 连接中
        Connected,      ///< 已连接
        Syncing,        ///< 同步中
        Error           ///< 错误状态
    };
    Q_ENUM(ConnectionStatus)

    /**
     * @brief 同步状态枚举
     */
    enum SyncStatus {
        NotSynced,      ///< 未同步
        Synced,         ///< 已同步
        Pending,        ///< 待同步
        Conflict,       ///< 冲突状态
        Failed          ///< 同步失败
    };
    Q_ENUM(SyncStatus)

    /**
     * @brief 冲突解决策略
     */
    enum ConflictResolution {
        KeepLocal,      ///< 保留本地
        KeepRemote,     ///< 保留远程
        Merge,          ///< 合并
        AskUser,        ///< 询问用户
        Timestamp       ///< 按时间戳
    };
    Q_ENUM(ConflictResolution)

    /**
     * @brief 云服务提供商
     */
    enum CloudProvider {
        CustomProvider, ///< 自定义服务
        AWSProvider,    ///< Amazon Web Services
        AzureProvider,  ///< Microsoft Azure
        GCPProvider,    ///< Google Cloud Platform
        DropboxProvider,///< Dropbox
        OneDriveProvider ///< Microsoft OneDrive
    };
    Q_ENUM(CloudProvider)

    explicit CloudStorage(QObject* parent = nullptr);
    explicit CloudStorage(const QString& serverUrl, const QString& authToken = QString(), QObject* parent = nullptr);
    ~CloudStorage();

    /**
     * @brief 初始化云存储
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 获取服务器URL
     * @return 服务器URL
     */
    QString serverUrl() const;

    /**
     * @brief 设置服务器URL
     * @param url 服务器URL
     */
    void setServerUrl(const QString& url);

    /**
     * @brief 获取认证令牌
     * @return 认证令牌
     */
    QString authToken() const;

    /**
     * @brief 设置认证令牌
     * @param token 认证令牌
     */
    void setAuthToken(const QString& token);

    /**
     * @brief 检查是否启用自动同步
     * @return 是否启用自动同步
     */
    bool isAutoSyncEnabled() const;

    /**
     * @brief 启用/禁用自动同步
     * @param enabled 是否启用
     */
    void setAutoSyncEnabled(bool enabled);

    /**
     * @brief 获取同步间隔
     * @return 同步间隔（秒）
     */
    int syncInterval() const;

    /**
     * @brief 设置同步间隔
     * @param interval 同步间隔（秒）
     */
    void setSyncInterval(int interval);

    /**
     * @brief 检查是否为离线模式
     * @return 是否为离线模式
     */
    bool isOfflineMode() const;

    /**
     * @brief 获取连接状态
     * @return 连接状态
     */
    ConnectionStatus connectionStatus() const;

    // 认证和连接
    /**
     * @brief 连接到云服务
     * @return 连接是否成功
     */
    bool connect();

    /**
     * @brief 断开连接
     */
    void disconnect();

    /**
     * @brief 使用用户名密码认证
     * @param username 用户名
     * @param password 密码
     * @return 认证是否成功
     */
    bool authenticate(const QString& username, const QString& password);

    /**
     * @brief 使用OAuth认证
     * @param oauthToken OAuth令牌
     * @return 认证是否成功
     */
    bool authenticateOAuth(const QString& oauthToken);

    /**
     * @brief 刷新认证令牌
     * @return 刷新是否成功
     */
    bool refreshToken();

    /**
     * @brief 检查认证状态
     * @return 是否已认证
     */
    bool isAuthenticated() const;

    // 数据操作
    /**
     * @brief 设置值
     * @param key 键
     * @param value 值
     */
    void setValue(const QString& key, const QVariant& value);

    /**
     * @brief 获取值
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 检查是否包含键
     * @param key 键
     * @return 是否包含
     */
    bool contains(const QString& key) const;

    /**
     * @brief 移除键值对
     * @param key 键
     */
    void remove(const QString& key);

    /**
     * @brief 获取所有键
     * @return 键列表
     */
    QStringList allKeys() const;

    /**
     * @brief 清除所有数据
     */
    void clear();

    // 同步操作
    /**
     * @brief 同步到云端
     * @return 同步是否成功
     */
    bool syncToCloud();

    /**
     * @brief 从云端同步
     * @return 同步是否成功
     */
    bool syncFromCloud();

    /**
     * @brief 双向同步
     * @return 同步是否成功
     */
    bool bidirectionalSync();

    /**
     * @brief 获取同步状态
     * @param key 键（可选，不指定则获取整体状态）
     * @return 同步状态
     */
    SyncStatus syncStatus(const QString& key = QString()) const;

    /**
     * @brief 获取最后同步时间
     * @return 最后同步时间
     */
    QDateTime lastSyncTime() const;

    /**
     * @brief 检查是否有待同步的更改
     * @return 是否有待同步更改
     */
    bool hasPendingChanges() const;

    // 冲突处理
    /**
     * @brief 设置冲突解决策略
     * @param strategy 解决策略
     */
    void setConflictResolution(ConflictResolution strategy);

    /**
     * @brief 获取冲突解决策略
     * @return 解决策略
     */
    ConflictResolution conflictResolution() const;

    /**
     * @brief 获取冲突列表
     * @return 冲突键列表
     */
    QStringList conflicts() const;

    /**
     * @brief 解决冲突
     * @param key 冲突键
     * @param resolution 解决方案
     * @return 解决是否成功
     */
    bool resolveConflict(const QString& key, ConflictResolution resolution);

    /**
     * @brief 解决所有冲突
     * @param resolution 解决方案
     * @return 解决是否成功
     */
    bool resolveAllConflicts(ConflictResolution resolution);

    // 离线缓存
    /**
     * @brief 启用/禁用离线缓存
     * @param enabled 是否启用
     */
    void setOfflineCacheEnabled(bool enabled);

    /**
     * @brief 检查是否启用离线缓存
     * @return 是否启用离线缓存
     */
    bool isOfflineCacheEnabled() const;

    /**
     * @brief 获取缓存大小
     * @return 缓存大小（字节）
     */
    qint64 cacheSize() const;

    /**
     * @brief 清除缓存
     */
    void clearCache();

    /**
     * @brief 压缩缓存
     */
    void compactCache();

    // 云服务配置
    /**
     * @brief 设置云服务提供商
     * @param provider 提供商类型
     * @param config 配置参数
     */
    void setCloudProvider(CloudProvider provider, const QVariantMap& config = QVariantMap());

    /**
     * @brief 获取云服务提供商
     * @return 提供商类型
     */
    CloudProvider cloudProvider() const;

    /**
     * @brief 设置请求超时
     * @param timeout 超时时间（毫秒）
     */
    void setRequestTimeout(int timeout);

    /**
     * @brief 获取请求超时
     * @return 超时时间（毫秒）
     */
    int requestTimeout() const;

    /**
     * @brief 设置重试次数
     * @param retries 重试次数
     */
    void setMaxRetries(int retries);

    /**
     * @brief 获取重试次数
     * @return 重试次数
     */
    int maxRetries() const;

    // 统计和监控
    /**
     * @brief 获取统计信息
     * @return 统计信息
     */
    QVariantMap statistics() const;

    /**
     * @brief 获取网络使用情况
     * @return 网络使用情况
     */
    QVariantMap networkUsage() const;

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

    // 导入导出
    /**
     * @brief 导出到JSON对象
     * @return JSON对象
     */
    QJsonObject exportToJson() const;

    /**
     * @brief 从JSON对象导入
     * @param json JSON对象
     * @param merge 是否合并
     * @return 导入是否成功
     */
    bool importFromJson(const QJsonObject& json, bool merge = false);

public slots:
    /**
     * @brief 强制同步
     */
    void forceSync();

    /**
     * @brief 重新连接
     */
    void reconnect();

    /**
     * @brief 切换到离线模式
     */
    void goOffline();

    /**
     * @brief 切换到在线模式
     */
    void goOnline();

    /**
     * @brief 刷新数据
     */
    void refresh();

signals:
    /**
     * @brief 服务器URL变化信号
     * @param url 新URL
     */
    void serverUrlChanged(const QString& url);

    /**
     * @brief 认证令牌变化信号
     * @param token 新令牌
     */
    void authTokenChanged(const QString& token);

    /**
     * @brief 自动同步设置变化信号
     * @param enabled 是否启用
     */
    void autoSyncChanged(bool enabled);

    /**
     * @brief 同步间隔变化信号
     * @param interval 新间隔
     */
    void syncIntervalChanged(int interval);

    /**
     * @brief 离线模式变化信号
     * @param offline 是否离线
     */
    void offlineModeChanged(bool offline);

    /**
     * @brief 连接状态变化信号
     * @param status 新状态
     */
    void connectionStatusChanged(ConnectionStatus status);

    /**
     * @brief 数据变化信号
     * @param key 变化的键
     * @param value 新值
     */
    void dataChanged(const QString& key, const QVariant& value);

    /**
     * @brief 同步开始信号
     */
    void syncStarted();

    /**
     * @brief 同步完成信号
     * @param success 是否成功
     */
    void syncCompleted(bool success);

    /**
     * @brief 同步进度信号
     * @param progress 进度（0-100）
     */
    void syncProgress(int progress);

    /**
     * @brief 冲突检测信号
     * @param key 冲突键
     * @param localValue 本地值
     * @param remoteValue 远程值
     */
    void conflictDetected(const QString& key, const QVariant& localValue, const QVariant& remoteValue);

    /**
     * @brief 冲突解决信号
     * @param key 冲突键
     * @param resolution 解决方案
     */
    void conflictResolved(const QString& key, ConflictResolution resolution);

    /**
     * @brief 认证成功信号
     */
    void authenticated();

    /**
     * @brief 认证失败信号
     * @param error 错误信息
     */
    void authenticationFailed(const QString& error);

    /**
     * @brief 网络错误信号
     * @param error 错误信息
     */
    void networkError(const QString& error);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    void onNetworkReplyFinished();
    void onSyncTimer();
    void onNetworkAccessibleChanged(bool accessible);

private:
    void setConnectionStatus(ConnectionStatus status);
    void setOfflineMode(bool offline);
    QNetworkRequest createRequest(const QString& endpoint) const;
    void handleNetworkReply(QNetworkReply* reply);
    bool processResponse(const QByteArray& data);
    void detectConflicts(const QJsonObject& remoteData);
    QVariant resolveConflictValue(const QString& key, const QVariant& localValue, const QVariant& remoteValue);
    void updateLocalCache(const QJsonObject& data);
    void scheduleSync();
    void updateStatistics(const QString& operation, qint64 bytes = 0);
    void loadLocalCache();
    QString providerToString(CloudProvider provider) const;
    CloudProvider stringToProvider(const QString& str) const;

    class Private;
    std::unique_ptr<Private> d;
};

#endif // CLOUDSTORAGE_H