#include "ConfigurationManager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>
#include <QMutexLocker>
#include <QUrl>
#include <QSettings>
#ifdef Q_OS_WIN
#include <QSettings>
#endif

// 静态成员初始化
std::unique_ptr<ConfigurationManager> ConfigurationManager::m_instance = nullptr;
QMutex ConfigurationManager::m_mutex;

// 配置键常量定义
const QString ConfigurationManager::KEY_DEFAULT_SERVER_URL = "server/defaultUrl";
const QString ConfigurationManager::KEY_SERVER_TIMEOUT = "server/timeout";
const QString ConfigurationManager::KEY_CUSTOM_SERVERS = "server/customServers";
const QString ConfigurationManager::KEY_MAIN_WINDOW_SIZE = "ui/mainWindowSize";
const QString ConfigurationManager::KEY_MAIN_WINDOW_POSITION = "ui/mainWindowPosition";
const QString ConfigurationManager::KEY_MAIN_WINDOW_MAXIMIZED = "ui/mainWindowMaximized";
const QString ConfigurationManager::KEY_CURRENT_THEME = "ui/currentTheme";
const QString ConfigurationManager::KEY_CURRENT_LANGUAGE = "ui/currentLanguage";
const QString ConfigurationManager::KEY_RECENT_MEETINGS = "meetings/recentMeetings";
const QString ConfigurationManager::KEY_SYSTEM_TRAY_ENABLED = "ui/systemTrayEnabled";
const QString ConfigurationManager::KEY_MINIMIZE_TO_TRAY = "ui/minimizeToTray";
const QString ConfigurationManager::KEY_AUTO_START = "system/autoStart";
const QString ConfigurationManager::KEY_DEFAULT_DISPLAY_NAME = "user/defaultDisplayName";
const QString ConfigurationManager::KEY_DEFAULT_MUTED = "audio/defaultMuted";
const QString ConfigurationManager::KEY_DEFAULT_VIDEO_DISABLED = "video/defaultVideoDisabled";

// 默认值常量定义
const QString ConfigurationManager::DEFAULT_SERVER_URL = "https://meet.jit.si";
const int ConfigurationManager::DEFAULT_SERVER_TIMEOUT = 30000; // 30秒
const QSize ConfigurationManager::DEFAULT_WINDOW_SIZE = QSize(1200, 800);
const QString ConfigurationManager::DEFAULT_THEME = "default";
const QString ConfigurationManager::DEFAULT_LANGUAGE = "zh_CN";

/**
 * @brief 获取配置管理器的单例实例
 */
ConfigurationManager* ConfigurationManager::instance()
{
    QMutexLocker locker(&m_mutex);
    if (!m_instance) {
        m_instance = std::unique_ptr<ConfigurationManager>(new ConfigurationManager());
    }
    return m_instance.get();
}

/**
 * @brief 私有构造函数
 */
ConfigurationManager::ConfigurationManager(QObject *parent)
    : QObject(parent)
{
    // 设置配置文件路径
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir configDir(configPath);
    if (!configDir.exists()) {
        configDir.mkpath(".");
    }
    
    QString settingsFile = configDir.absoluteFilePath("jitsi-meet-qt.ini");
    m_settings = std::make_unique<QSettings>(settingsFile, QSettings::IniFormat);
    
    qDebug() << "配置文件路径:" << settingsFile;
    
    // 初始化默认配置
    initializeDefaults();
    
    // 迁移旧版本配置
    migrateOldSettings();
}

/**
 * @brief 析构函数
 */
ConfigurationManager::~ConfigurationManager()
{
    if (m_settings) {
        m_settings->sync();
    }
}

/**
 * @brief 初始化默认配置
 */
void ConfigurationManager::initializeDefaults()
{
    // 如果是首次运行，设置默认值
    if (!m_settings->contains(KEY_DEFAULT_SERVER_URL)) {
        m_settings->setValue(KEY_DEFAULT_SERVER_URL, DEFAULT_SERVER_URL);
    }
    
    if (!m_settings->contains(KEY_SERVER_TIMEOUT)) {
        m_settings->setValue(KEY_SERVER_TIMEOUT, DEFAULT_SERVER_TIMEOUT);
    }
    
    if (!m_settings->contains(KEY_MAIN_WINDOW_SIZE)) {
        m_settings->setValue(KEY_MAIN_WINDOW_SIZE, DEFAULT_WINDOW_SIZE);
    }
    
    if (!m_settings->contains(KEY_CURRENT_THEME)) {
        m_settings->setValue(KEY_CURRENT_THEME, DEFAULT_THEME);
    }
    
    if (!m_settings->contains(KEY_CURRENT_LANGUAGE)) {
        m_settings->setValue(KEY_CURRENT_LANGUAGE, DEFAULT_LANGUAGE);
    }
    
    if (!m_settings->contains(KEY_SYSTEM_TRAY_ENABLED)) {
        m_settings->setValue(KEY_SYSTEM_TRAY_ENABLED, true);
    }
    
    if (!m_settings->contains(KEY_MINIMIZE_TO_TRAY)) {
        m_settings->setValue(KEY_MINIMIZE_TO_TRAY, false);
    }
    
    if (!m_settings->contains(KEY_AUTO_START)) {
        m_settings->setValue(KEY_AUTO_START, false);
    }
    
    if (!m_settings->contains(KEY_DEFAULT_MUTED)) {
        m_settings->setValue(KEY_DEFAULT_MUTED, false);
    }
    
    if (!m_settings->contains(KEY_DEFAULT_VIDEO_DISABLED)) {
        m_settings->setValue(KEY_DEFAULT_VIDEO_DISABLED, false);
    }
    
    // 设置默认显示名称
    if (!m_settings->contains(KEY_DEFAULT_DISPLAY_NAME)) {
        m_settings->setValue(KEY_DEFAULT_DISPLAY_NAME, tr("用户"));
    }
    
    m_settings->sync();
}

/**
 * @brief 迁移旧版本配置
 */
void ConfigurationManager::migrateOldSettings()
{
    // 检查配置版本，如果需要可以进行配置迁移
    QString configVersion = m_settings->value("system/configVersion", "1.0").toString();
    
    if (configVersion == "1.0") {
        // 当前版本，无需迁移
        qDebug() << "配置版本:" << configVersion << "，无需迁移";
    }
    
    // 更新配置版本
    m_settings->setValue("system/configVersion", "1.0");
}

/**
 * @brief 验证配置值的有效性
 */
bool ConfigurationManager::validateValue(const QString& key, const QVariant& value) const
{
    if (key == KEY_DEFAULT_SERVER_URL) {
        QUrl url(value.toString());
        return url.isValid() && (url.scheme() == "http" || url.scheme() == "https");
    }
    
    if (key == KEY_SERVER_TIMEOUT) {
        int timeout = value.toInt();
        return timeout >= 5000 && timeout <= 300000; // 5秒到5分钟
    }
    
    if (key == KEY_MAIN_WINDOW_SIZE) {
        QSize size = value.toSize();
        return size.width() >= 800 && size.height() >= 600;
    }
    
    return true; // 其他值默认有效
}

// ========== 服务器配置 ==========

/**
 * @brief 获取默认的Jitsi Meet服务器URL
 */
QString ConfigurationManager::getDefaultServerUrl() const
{
    return m_settings->value(KEY_DEFAULT_SERVER_URL, DEFAULT_SERVER_URL).toString();
}

/**
 * @brief 设置默认的Jitsi Meet服务器URL
 */
void ConfigurationManager::setDefaultServerUrl(const QString& url)
{
    if (validateValue(KEY_DEFAULT_SERVER_URL, url)) {
        m_settings->setValue(KEY_DEFAULT_SERVER_URL, url);
        emit valueChanged(KEY_DEFAULT_SERVER_URL, url);
        emit serverConfigChanged();
    } else {
        qWarning() << "无效的服务器URL:" << url;
    }
}

/**
 * @brief 获取服务器连接超时时间
 */
int ConfigurationManager::getServerTimeout() const
{
    return m_settings->value(KEY_SERVER_TIMEOUT, DEFAULT_SERVER_TIMEOUT).toInt();
}

/**
 * @brief 设置服务器连接超时时间
 */
void ConfigurationManager::setServerTimeout(int timeout)
{
    if (validateValue(KEY_SERVER_TIMEOUT, timeout)) {
        m_settings->setValue(KEY_SERVER_TIMEOUT, timeout);
        emit valueChanged(KEY_SERVER_TIMEOUT, timeout);
        emit serverConfigChanged();
    } else {
        qWarning() << "无效的超时时间:" << timeout;
    }
}

/**
 * @brief 获取自定义服务器列表
 */
QStringList ConfigurationManager::getCustomServers() const
{
    return m_settings->value(KEY_CUSTOM_SERVERS).toStringList();
}

/**
 * @brief 添加自定义服务器
 */
void ConfigurationManager::addCustomServer(const QString& serverUrl)
{
    QStringList servers = getCustomServers();
    if (!servers.contains(serverUrl)) {
        QUrl url(serverUrl);
        if (url.isValid()) {
            servers.append(serverUrl);
            m_settings->setValue(KEY_CUSTOM_SERVERS, servers);
            emit valueChanged(KEY_CUSTOM_SERVERS, servers);
            emit serverConfigChanged();
        } else {
            qWarning() << "无效的服务器URL:" << serverUrl;
        }
    }
}

/**
 * @brief 移除自定义服务器
 */
void ConfigurationManager::removeCustomServer(const QString& serverUrl)
{
    QStringList servers = getCustomServers();
    if (servers.removeOne(serverUrl)) {
        m_settings->setValue(KEY_CUSTOM_SERVERS, servers);
        emit valueChanged(KEY_CUSTOM_SERVERS, servers);
        emit serverConfigChanged();
    }
}

// ========== 窗口和界面配置 ==========

/**
 * @brief 获取主窗口大小
 */
QSize ConfigurationManager::getMainWindowSize() const
{
    return m_settings->value(KEY_MAIN_WINDOW_SIZE, DEFAULT_WINDOW_SIZE).toSize();
}

/**
 * @brief 设置主窗口大小
 */
void ConfigurationManager::setMainWindowSize(const QSize& size)
{
    if (validateValue(KEY_MAIN_WINDOW_SIZE, size)) {
        m_settings->setValue(KEY_MAIN_WINDOW_SIZE, size);
        emit valueChanged(KEY_MAIN_WINDOW_SIZE, size);
    }
}

/**
 * @brief 获取主窗口位置
 */
QPoint ConfigurationManager::getMainWindowPosition() const
{
    return m_settings->value(KEY_MAIN_WINDOW_POSITION, QPoint(100, 100)).toPoint();
}

/**
 * @brief 设置主窗口位置
 */
void ConfigurationManager::setMainWindowPosition(const QPoint& position)
{
    m_settings->setValue(KEY_MAIN_WINDOW_POSITION, position);
    emit valueChanged(KEY_MAIN_WINDOW_POSITION, position);
}

/**
 * @brief 获取窗口是否最大化
 */
bool ConfigurationManager::isMainWindowMaximized() const
{
    return m_settings->value(KEY_MAIN_WINDOW_MAXIMIZED, false).toBool();
}

/**
 * @brief 设置窗口最大化状态
 */
void ConfigurationManager::setMainWindowMaximized(bool maximized)
{
    m_settings->setValue(KEY_MAIN_WINDOW_MAXIMIZED, maximized);
    emit valueChanged(KEY_MAIN_WINDOW_MAXIMIZED, maximized);
}

/**
 * @brief 获取当前主题名称
 */
QString ConfigurationManager::getCurrentTheme() const
{
    return m_settings->value(KEY_CURRENT_THEME, DEFAULT_THEME).toString();
}

/**
 * @brief 设置当前主题
 */
void ConfigurationManager::setCurrentTheme(const QString& theme)
{
    m_settings->setValue(KEY_CURRENT_THEME, theme);
    emit valueChanged(KEY_CURRENT_THEME, theme);
    emit themeChanged(theme);
}

/**
 * @brief 获取当前语言代码
 */
QString ConfigurationManager::getCurrentLanguage() const
{
    return m_settings->value(KEY_CURRENT_LANGUAGE, DEFAULT_LANGUAGE).toString();
}

/**
 * @brief 设置当前语言
 */
void ConfigurationManager::setCurrentLanguage(const QString& language)
{
    m_settings->setValue(KEY_CURRENT_LANGUAGE, language);
    emit valueChanged(KEY_CURRENT_LANGUAGE, language);
    emit languageChanged(language);
}

// ========== 会议历史记录 ==========

/**
 * @brief 获取最近的会议记录
 */
QJsonObject ConfigurationManager::getRecentMeetings(int maxCount) const
{
    QJsonObject result;
    QJsonArray meetings;
    
    QString meetingsData = m_settings->value(KEY_RECENT_MEETINGS).toString();
    if (!meetingsData.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(meetingsData.toUtf8());
        if (doc.isArray()) {
            QJsonArray allMeetings = doc.array();
            int count = qMin(maxCount, allMeetings.size());
            for (int i = 0; i < count; ++i) {
                meetings.append(allMeetings.at(i));
            }
        }
    }
    
    result["meetings"] = meetings;
    result["count"] = meetings.size();
    return result;
}

/**
 * @brief 添加会议记录
 */
void ConfigurationManager::addMeetingRecord(const QString& roomName, const QString& serverUrl, const QString& displayName)
{
    QJsonObject meeting;
    meeting["roomName"] = roomName;
    meeting["serverUrl"] = serverUrl;
    meeting["displayName"] = displayName;
    meeting["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    meeting["fullUrl"] = serverUrl + "/" + roomName;
    
    // 获取现有记录
    QString meetingsData = m_settings->value(KEY_RECENT_MEETINGS).toString();
    QJsonArray meetings;
    
    if (!meetingsData.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(meetingsData.toUtf8());
        if (doc.isArray()) {
            meetings = doc.array();
        }
    }
    
    // 检查是否已存在相同的会议记录
    for (int i = 0; i < meetings.size(); ++i) {
        QJsonObject existingMeeting = meetings.at(i).toObject();
        if (existingMeeting["roomName"].toString() == roomName && 
            existingMeeting["serverUrl"].toString() == serverUrl) {
            meetings.removeAt(i);
            break;
        }
    }
    
    // 添加新记录到开头
    meetings.prepend(meeting);
    
    // 限制记录数量（最多保留50条）
    while (meetings.size() > 50) {
        meetings.removeLast();
    }
    
    // 保存更新后的记录
    QJsonDocument doc(meetings);
    m_settings->setValue(KEY_RECENT_MEETINGS, QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    emit valueChanged(KEY_RECENT_MEETINGS, meetings);
}

/**
 * @brief 清除会议历史记录
 */
void ConfigurationManager::clearMeetingHistory()
{
    m_settings->remove(KEY_RECENT_MEETINGS);
    emit valueChanged(KEY_RECENT_MEETINGS, QJsonArray());
}

// ========== 用户偏好设置 ==========

/**
 * @brief 获取是否启用系统托盘
 */
bool ConfigurationManager::isSystemTrayEnabled() const
{
    return m_settings->value(KEY_SYSTEM_TRAY_ENABLED, true).toBool();
}

/**
 * @brief 设置系统托盘启用状态
 */
void ConfigurationManager::setSystemTrayEnabled(bool enabled)
{
    m_settings->setValue(KEY_SYSTEM_TRAY_ENABLED, enabled);
    emit valueChanged(KEY_SYSTEM_TRAY_ENABLED, enabled);
}

/**
 * @brief 获取是否最小化到托盘
 */
bool ConfigurationManager::isMinimizeToTray() const
{
    return m_settings->value(KEY_MINIMIZE_TO_TRAY, false).toBool();
}

/**
 * @brief 设置最小化到托盘
 */
void ConfigurationManager::setMinimizeToTray(bool minimize)
{
    m_settings->setValue(KEY_MINIMIZE_TO_TRAY, minimize);
    emit valueChanged(KEY_MINIMIZE_TO_TRAY, minimize);
}

/**
 * @brief 获取是否开机自启动
 */
bool ConfigurationManager::isAutoStart() const
{
    return m_settings->value(KEY_AUTO_START, false).toBool();
}

/**
 * @brief 设置开机自启动
 */
void ConfigurationManager::setAutoStart(bool autoStart)
{
    m_settings->setValue(KEY_AUTO_START, autoStart);
    
#ifdef Q_OS_WIN
    // Windows注册表设置
    QSettings registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (autoStart) {
        QString appPath = QApplication::applicationFilePath();
        registry.setValue("JitsiMeetQt", QDir::toNativeSeparators(appPath));
    } else {
        registry.remove("JitsiMeetQt");
    }
#endif
    
    emit valueChanged(KEY_AUTO_START, autoStart);
}

/**
 * @brief 获取默认显示名称
 */
QString ConfigurationManager::getDefaultDisplayName() const
{
    return m_settings->value(KEY_DEFAULT_DISPLAY_NAME).toString();
}

/**
 * @brief 设置默认显示名称
 */
void ConfigurationManager::setDefaultDisplayName(const QString& name)
{
    m_settings->setValue(KEY_DEFAULT_DISPLAY_NAME, name);
    emit valueChanged(KEY_DEFAULT_DISPLAY_NAME, name);
}

// ========== 音视频设置 ==========

/**
 * @brief 获取是否默认静音
 */
bool ConfigurationManager::isDefaultMuted() const
{
    return m_settings->value(KEY_DEFAULT_MUTED, false).toBool();
}

/**
 * @brief 设置默认静音状态
 */
void ConfigurationManager::setDefaultMuted(bool muted)
{
    m_settings->setValue(KEY_DEFAULT_MUTED, muted);
    emit valueChanged(KEY_DEFAULT_MUTED, muted);
}

/**
 * @brief 获取是否默认关闭摄像头
 */
bool ConfigurationManager::isDefaultVideoDisabled() const
{
    return m_settings->value(KEY_DEFAULT_VIDEO_DISABLED, false).toBool();
}

/**
 * @brief 设置默认摄像头状态
 */
void ConfigurationManager::setDefaultVideoDisabled(bool disabled)
{
    m_settings->setValue(KEY_DEFAULT_VIDEO_DISABLED, disabled);
    emit valueChanged(KEY_DEFAULT_VIDEO_DISABLED, disabled);
}

// ========== 通用配置方法 ==========

/**
 * @brief 获取配置值
 */
QVariant ConfigurationManager::getValue(const QString& key, const QVariant& defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

/**
 * @brief 设置配置值
 */
void ConfigurationManager::setValue(const QString& key, const QVariant& value)
{
    if (validateValue(key, value)) {
        m_settings->setValue(key, value);
        emit valueChanged(key, value);
    }
}

/**
 * @brief 同步配置到磁盘
 */
void ConfigurationManager::sync()
{
    m_settings->sync();
}

/**
 * @brief 重置所有配置为默认值
 */
void ConfigurationManager::resetToDefaults()
{
    m_settings->clear();
    initializeDefaults();
    qDebug() << "配置已重置为默认值";
}

/**
 * @brief 导出配置到文件
 */
bool ConfigurationManager::exportSettings(const QString& filePath) const
{
    try {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "无法打开文件进行写入:" << filePath;
            return false;
        }
        
        // 创建配置的JSON表示
        QJsonObject config;
        for (const QString& key : m_settings->allKeys()) {
            config[key] = QJsonValue::fromVariant(m_settings->value(key));
        }
        
        QJsonDocument doc(config);
        file.write(doc.toJson());
        
        qDebug() << "配置已导出到:" << filePath;
        return true;
    } catch (const std::exception& e) {
        qWarning() << "导出配置时发生异常:" << e.what();
        return false;
    }
}

/**
 * @brief 从文件导入配置
 */
bool ConfigurationManager::importSettings(const QString& filePath)
{
    try {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "无法打开文件进行读取:" << filePath;
            return false;
        }
        
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        if (!doc.isObject()) {
            qWarning() << "无效的配置文件格式";
            return false;
        }
        
        QJsonObject config = doc.object();
        for (auto it = config.begin(); it != config.end(); ++it) {
            setValue(it.key(), it.value().toVariant());
        }
        
        sync();
        qDebug() << "配置已从文件导入:" << filePath;
        return true;
    } catch (const std::exception& e) {
        qWarning() << "导入配置时发生异常:" << e.what();
        return false;
    }
}