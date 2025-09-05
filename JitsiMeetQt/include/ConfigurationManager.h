#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QSize>
#include <QPoint>
#include <QUrl>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMutex>
#include <memory>

// 前向声明
class DatabaseManager;

/**
 * @brief 配置管理器类 - 负责应用程序配置的读取、写入和管理
 * 
 * 该类管理以下配置项：
 * - 服务器设置（默认服务器URL、超时时间等）
 * - 用户界面设置（窗口大小、位置、主题等）
 * - 会议历史记录
 * - 用户偏好设置
 * - 协议处理设置
 */
class ConfigurationManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取配置管理器的单例实例
     * @return ConfigurationManager的单例指针
     */
    static ConfigurationManager* instance();
    
    /**
     * @brief 析构函数
     */
    ~ConfigurationManager();
    
    // ========== 服务器配置 ==========
    
    /**
     * @brief 获取默认的Jitsi Meet服务器URL
     * @return 默认服务器URL
     */
    QString getDefaultServerUrl() const;
    
    /**
     * @brief 设置默认的Jitsi Meet服务器URL
     * @param url 服务器URL
     */
    void setDefaultServerUrl(const QString& url);
    
    /**
     * @brief 获取服务器连接超时时间（毫秒）
     * @return 超时时间
     */
    int getServerTimeout() const;
    
    /**
     * @brief 设置服务器连接超时时间
     * @param timeout 超时时间（毫秒）
     */
    void setServerTimeout(int timeout);
    
    /**
     * @brief 获取自定义服务器列表
     * @return 服务器URL列表
     */
    QStringList getCustomServers() const;
    
    /**
     * @brief 添加自定义服务器
     * @param serverUrl 服务器URL
     */
    void addCustomServer(const QString& serverUrl);
    
    /**
     * @brief 移除自定义服务器
     * @param serverUrl 服务器URL
     */
    void removeCustomServer(const QString& serverUrl);
    
    // ========== 窗口和界面配置 ==========
    
    /**
     * @brief 获取主窗口大小
     * @return 窗口大小
     */
    QSize getMainWindowSize() const;
    
    /**
     * @brief 设置主窗口大小
     * @param size 窗口大小
     */
    void setMainWindowSize(const QSize& size);
    
    /**
     * @brief 获取主窗口位置
     * @return 窗口位置
     */
    QPoint getMainWindowPosition() const;
    
    /**
     * @brief 设置主窗口位置
     * @param position 窗口位置
     */
    void setMainWindowPosition(const QPoint& position);
    
    /**
     * @brief 获取窗口是否最大化
     * @return 是否最大化
     */
    bool isMainWindowMaximized() const;
    
    /**
     * @brief 设置窗口最大化状态
     * @param maximized 是否最大化
     */
    void setMainWindowMaximized(bool maximized);
    
    /**
     * @brief 获取当前主题名称
     * @return 主题名称
     */
    QString getCurrentTheme() const;
    
    /**
     * @brief 设置当前主题
     * @param theme 主题名称
     */
    void setCurrentTheme(const QString& theme);
    
    /**
     * @brief 获取当前语言代码
     * @return 语言代码（如"zh_CN", "en_US"）
     */
    QString getCurrentLanguage() const;
    
    /**
     * @brief 设置当前语言
     * @param language 语言代码
     */
    void setCurrentLanguage(const QString& language);
    
    // ========== 会议历史记录 ==========
    
    /**
     * @brief 获取最近的会议记录
     * @param maxCount 最大记录数量
     * @return 会议记录列表（JSON格式）
     */
    QJsonObject getRecentMeetings(int maxCount = 10) const;
    
    /**
     * @brief 添加会议记录
     * @param roomName 房间名称
     * @param serverUrl 服务器URL
     * @param displayName 显示名称
     */
    void addMeetingRecord(const QString& roomName, const QString& serverUrl, const QString& displayName = QString());
    
    /**
     * @brief 清除会议历史记录
     */
    void clearMeetingHistory();
    
    /**
     * @brief 删除指定的会议记录
     * @param roomName 房间名称
     * @param serverUrl 服务器URL
     * @return 删除是否成功
     */
    bool deleteMeetingRecord(const QString& roomName, const QString& serverUrl);
    
    // ========== 用户偏好设置 ==========
    
    /**
     * @brief 获取是否启用系统托盘
     * @return 是否启用
     */
    bool isSystemTrayEnabled() const;
    
    /**
     * @brief 设置系统托盘启用状态
     * @param enabled 是否启用
     */
    void setSystemTrayEnabled(bool enabled);
    
    /**
     * @brief 获取是否最小化到托盘
     * @return 是否最小化到托盘
     */
    bool isMinimizeToTray() const;
    
    /**
     * @brief 设置最小化到托盘
     * @param minimize 是否最小化到托盘
     */
    void setMinimizeToTray(bool minimize);
    
    /**
     * @brief 获取是否开机自启动
     * @return 是否自启动
     */
    bool isAutoStart() const;
    
    /**
     * @brief 设置开机自启动
     * @param autoStart 是否自启动
     */
    void setAutoStart(bool autoStart);
    
    /**
     * @brief 获取默认显示名称
     * @return 显示名称
     */
    QString getDefaultDisplayName() const;
    
    /**
     * @brief 设置默认显示名称
     * @param name 显示名称
     */
    void setDefaultDisplayName(const QString& name);
    
    // ========== 音视频设置 ==========
    
    /**
     * @brief 获取是否默认静音
     * @return 是否静音
     */
    bool isDefaultMuted() const;
    
    /**
     * @brief 设置默认静音状态
     * @param muted 是否静音
     */
    void setDefaultMuted(bool muted);
    
    /**
     * @brief 获取是否默认关闭摄像头
     * @return 是否关闭摄像头
     */
    bool isDefaultVideoDisabled() const;
    
    /**
     * @brief 设置默认摄像头状态
     * @param disabled 是否关闭摄像头
     */
    void setDefaultVideoDisabled(bool disabled);
    
    // ========== 通用配置方法 ==========
    
    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    
    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 配置值
     */
    void setValue(const QString& key, const QVariant& value);
    
    /**
     * @brief 同步配置到磁盘
     */
    void sync();
    
    /**
     * @brief 重置所有配置为默认值
     */
    void resetToDefaults();
    
    /**
     * @brief 导出配置到文件
     * @param filePath 文件路径
     * @return 是否成功
     */
    bool exportSettings(const QString& filePath) const;
    
    /**
     * @brief 从文件导入配置
     * @param filePath 文件路径
     * @return 是否成功
     */
    bool importSettings(const QString& filePath);

signals:
    /**
     * @brief 配置值改变信号
     * @param key 配置键
     * @param value 新值
     */
    void valueChanged(const QString& key, const QVariant& value);
    
    /**
     * @brief 主题改变信号
     * @param theme 新主题名称
     */
    void themeChanged(const QString& theme);
    
    /**
     * @brief 语言改变信号
     * @param language 新语言代码
     */
    void languageChanged(const QString& language);
    
    /**
     * @brief 服务器配置改变信号
     */
    void serverConfigChanged();

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit ConfigurationManager(QObject *parent = nullptr);
    
    /**
     * @brief 初始化默认配置
     */
    void initializeDefaults();
    
    /**
     * @brief 迁移旧版本配置
     */
    void migrateOldSettings();
    
    /**
     * @brief 验证配置值的有效性
     * @param key 配置键
     * @param value 配置值
     * @return 是否有效
     */
    bool validateValue(const QString& key, const QVariant& value) const;

private:
    static std::unique_ptr<ConfigurationManager> m_instance; ///< 单例实例
    static QMutex m_mutex; ///< 线程安全互斥锁
    
    std::unique_ptr<QSettings> m_settings; ///< Qt设置对象
    DatabaseManager* m_databaseManager; ///< 数据库管理器
    
    // 配置键常量
    static const QString KEY_DEFAULT_SERVER_URL;
    static const QString KEY_SERVER_TIMEOUT;
    static const QString KEY_CUSTOM_SERVERS;
    static const QString KEY_MAIN_WINDOW_SIZE;
    static const QString KEY_MAIN_WINDOW_POSITION;
    static const QString KEY_MAIN_WINDOW_MAXIMIZED;
    static const QString KEY_CURRENT_THEME;
    static const QString KEY_CURRENT_LANGUAGE;
    static const QString KEY_RECENT_MEETINGS;
    static const QString KEY_SYSTEM_TRAY_ENABLED;
    static const QString KEY_MINIMIZE_TO_TRAY;
    static const QString KEY_AUTO_START;
    static const QString KEY_DEFAULT_DISPLAY_NAME;
    static const QString KEY_DEFAULT_MUTED;
    static const QString KEY_DEFAULT_VIDEO_DISABLED;
    
    // 默认值常量
    static const QString DEFAULT_SERVER_URL;
    static const int DEFAULT_SERVER_TIMEOUT;
    static const QSize DEFAULT_WINDOW_SIZE;
    static const QString DEFAULT_THEME;
    static const QString DEFAULT_LANGUAGE;
};

#endif // CONFIGURATIONMANAGER_H