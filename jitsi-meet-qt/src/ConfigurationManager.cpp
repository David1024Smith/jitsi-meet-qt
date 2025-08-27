#include "ConfigurationManager.h"
#include "WindowStateManager.h"
#include "JitsiConstants.h"
#include "models/RecentItem.h"

#include <QStandardPaths>
#include <QDir>
#include <QUrl>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

ConfigurationManager::ConfigurationManager(QObject* parent)
    : QObject(parent)
    , m_settings(nullptr)
    , m_configLoaded(false)
    , m_windowStateManager(nullptr)
{
    // 创建配置目录
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    
    // 创建QSettings实例
    QString configFile = configDir + "/config.ini";
    m_settings = new QSettings(configFile, QSettings::IniFormat, this);
    
    qDebug() << "Configuration file:" << configFile;
    
    // 设置默认值
    setDefaults();
    
    // 加载配置
    m_config = loadConfiguration();
    
    // 创建窗口状态管理器
    m_windowStateManager = new WindowStateManager(this, this);
}

ConfigurationManager::~ConfigurationManager()
{
    if (m_settings) {
        saveConfiguration(m_config);
        m_settings->sync();
    }
}

ApplicationSettings ConfigurationManager::loadConfiguration()
{
    ApplicationSettings config;
    
    if (!m_settings) {
        qWarning() << "Settings not initialized, using defaults";
        return config;
    }
    
    // 加载服务器配置
    config.defaultServerUrl = m_settings->value(JitsiConstants::ConfigKeys::SERVER_URL, 
                                               JitsiConstants::DEFAULT_SERVER_URL).toString();
    config.serverTimeout = m_settings->value(JitsiConstants::ConfigKeys::SERVER_TIMEOUT, 
                                            JitsiConstants::DEFAULT_SERVER_TIMEOUT).toInt();
    
    // 加载界面配置
    config.language = m_settings->value(JitsiConstants::ConfigKeys::LANGUAGE, 
                                       JitsiConstants::DEFAULT_LANGUAGE).toString();
    config.darkMode = m_settings->value(JitsiConstants::ConfigKeys::DARK_MODE, false).toBool();
    
    // 加载窗口配置
    config.windowGeometry = m_settings->value(JitsiConstants::ConfigKeys::WINDOW_GEOMETRY, 
                                             QRect(100, 100, JitsiConstants::DEFAULT_WINDOW_WIDTH, 
                                                  JitsiConstants::DEFAULT_WINDOW_HEIGHT)).toRect();
    config.maximized = m_settings->value(JitsiConstants::ConfigKeys::WINDOW_MAXIMIZED, false).toBool();
    config.rememberWindowState = m_settings->value(JitsiConstants::ConfigKeys::REMEMBER_WINDOW_STATE, true).toBool();
    
    // 加载功能配置
    config.autoJoinAudio = m_settings->value(JitsiConstants::ConfigKeys::AUTO_JOIN_AUDIO, true).toBool();
    config.autoJoinVideo = m_settings->value(JitsiConstants::ConfigKeys::AUTO_JOIN_VIDEO, false).toBool();
    config.maxRecentItems = m_settings->value(JitsiConstants::ConfigKeys::MAX_RECENT_ITEMS, 
                                             JitsiConstants::MAX_RECENT_ITEMS).toInt();
    
    // 加载最近使用的URL
    config.recentUrls = m_settings->value(JitsiConstants::ConfigKeys::RECENT_URLS, QStringList()).toStringList();
    
    // 验证并修复配置
    config = validateAndFixSettings(config);
    
    m_configLoaded = true;
    
    qDebug() << "Configuration loaded:" << config.toString();
    
    return config;
}

void ConfigurationManager::saveConfiguration(const ApplicationSettings& config)
{
    if (!m_settings) {
        qWarning() << "Settings not initialized";
        return;
    }
    
    // 验证配置有效性
    ApplicationSettings validConfig = validateAndFixSettings(config);
    
    // 保存服务器配置
    m_settings->setValue(JitsiConstants::ConfigKeys::SERVER_URL, validConfig.defaultServerUrl);
    m_settings->setValue(JitsiConstants::ConfigKeys::SERVER_TIMEOUT, validConfig.serverTimeout);
    
    // 保存界面配置
    m_settings->setValue(JitsiConstants::ConfigKeys::LANGUAGE, validConfig.language);
    m_settings->setValue(JitsiConstants::ConfigKeys::DARK_MODE, validConfig.darkMode);
    
    // 保存窗口配置
    m_settings->setValue(JitsiConstants::ConfigKeys::WINDOW_GEOMETRY, validConfig.windowGeometry);
    m_settings->setValue(JitsiConstants::ConfigKeys::WINDOW_MAXIMIZED, validConfig.maximized);
    m_settings->setValue(JitsiConstants::ConfigKeys::REMEMBER_WINDOW_STATE, validConfig.rememberWindowState);
    
    // 保存功能配置
    m_settings->setValue(JitsiConstants::ConfigKeys::AUTO_JOIN_AUDIO, validConfig.autoJoinAudio);
    m_settings->setValue(JitsiConstants::ConfigKeys::AUTO_JOIN_VIDEO, validConfig.autoJoinVideo);
    m_settings->setValue(JitsiConstants::ConfigKeys::MAX_RECENT_ITEMS, validConfig.maxRecentItems);
    
    // 保存最近使用的URL
    m_settings->setValue(JitsiConstants::ConfigKeys::RECENT_URLS, validConfig.recentUrls);
    
    m_settings->sync();
    
    m_config = validConfig;
    
    qDebug() << "Configuration saved:" << validConfig.toString();
    
    emit configurationChanged();
}

QString ConfigurationManager::serverUrl() const
{
    return m_config.defaultServerUrl;
}

void ConfigurationManager::setServerUrl(const QString& url)
{
    if (m_config.defaultServerUrl != url && validateServerUrl(url)) {
        m_config.defaultServerUrl = url;
        if (m_settings) {
            m_settings->setValue(JitsiConstants::ConfigKeys::SERVER_URL, url);
            m_settings->sync();
        }
        emit serverUrlChanged(url);
        emit configurationChanged();
    }
}

QString ConfigurationManager::language() const
{
    return m_config.language;
}

void ConfigurationManager::setLanguage(const QString& language)
{
    if (m_config.language != language) {
        m_config.language = language;
        if (m_settings) {
            m_settings->setValue(JitsiConstants::ConfigKeys::LANGUAGE, language);
            m_settings->sync();
        }
        emit languageChanged(language);
        emit configurationChanged();
    }
}

QStringList ConfigurationManager::recentUrls() const
{
    return m_config.recentUrls;
}

void ConfigurationManager::addRecentUrl(const QString& url)
{
    if (url.isEmpty() || !validateServerUrl(url)) {
        qWarning() << "Invalid URL, not adding to recent list:" << url;
        return;
    }
    
    // 移除已存在的相同URL
    m_config.recentUrls.removeAll(url);
    
    // 添加到列表开头
    m_config.recentUrls.prepend(url);
    
    // 限制列表大小
    while (m_config.recentUrls.size() > m_config.maxRecentItems) {
        m_config.recentUrls.removeLast();
    }
    
    // 保存到设置
    if (m_settings) {
        m_settings->setValue(JitsiConstants::ConfigKeys::RECENT_URLS, m_config.recentUrls);
        m_settings->sync();
    }
    
    emit configurationChanged();
    
    qDebug() << "Added recent URL:" << url;
}

void ConfigurationManager::clearRecentUrls()
{
    m_config.recentUrls.clear();
    if (m_settings) {
        m_settings->setValue(JitsiConstants::ConfigKeys::RECENT_URLS, QStringList());
        m_settings->sync();
    }
    emit configurationChanged();
    
    qDebug() << "Recent URLs cleared";
}

QRect ConfigurationManager::windowGeometry() const
{
    return m_config.windowGeometry;
}

void ConfigurationManager::setWindowGeometry(const QRect& geometry)
{
    QRect validGeometry = validateWindowGeometry(geometry);
    if (m_config.windowGeometry != validGeometry) {
        m_config.windowGeometry = validGeometry;
        if (m_settings) {
            m_settings->setValue(JitsiConstants::ConfigKeys::WINDOW_GEOMETRY, validGeometry);
            m_settings->sync();
        }
        emit configurationChanged();
    }
}

bool ConfigurationManager::isWindowMaximized() const
{
    return m_config.maximized;
}

void ConfigurationManager::setWindowMaximized(bool maximized)
{
    if (m_config.maximized != maximized) {
        m_config.maximized = maximized;
        if (m_settings) {
            m_settings->setValue(JitsiConstants::ConfigKeys::WINDOW_MAXIMIZED, maximized);
            m_settings->sync();
        }
        emit configurationChanged();
    }
}

bool ConfigurationManager::isDarkMode() const
{
    return m_config.darkMode;
}

void ConfigurationManager::setDarkMode(bool darkMode)
{
    if (m_config.darkMode != darkMode) {
        m_config.darkMode = darkMode;
        if (m_settings) {
            m_settings->setValue(JitsiConstants::ConfigKeys::DARK_MODE, darkMode);
            m_settings->sync();
        }
        emit darkModeChanged(darkMode);
        emit configurationChanged();
    }
}

void ConfigurationManager::setDefaults()
{
    m_config.resetToDefaults();
}

bool ConfigurationManager::validateServerUrl(const QString& url) const
{
    if (url.isEmpty()) return false;
    
    QUrl qurl(url);
    return qurl.isValid() && (qurl.scheme() == "http" || qurl.scheme() == "https");
}

QRect ConfigurationManager::validateWindowGeometry(const QRect& geometry) const
{
    QRect validGeometry = geometry;
    
    // 确保窗口大小不小于最小值
    if (validGeometry.width() < JitsiConstants::MIN_WINDOW_WIDTH) {
        validGeometry.setWidth(JitsiConstants::MIN_WINDOW_WIDTH);
    }
    if (validGeometry.height() < JitsiConstants::MIN_WINDOW_HEIGHT) {
        validGeometry.setHeight(JitsiConstants::MIN_WINDOW_HEIGHT);
    }
    
    // 确保窗口在屏幕范围内
    if (QApplication::instance()) {
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->availableGeometry();
            
            // 调整位置确保窗口可见
            if (validGeometry.x() < screenGeometry.x()) {
                validGeometry.moveLeft(screenGeometry.x());
            }
            if (validGeometry.y() < screenGeometry.y()) {
                validGeometry.moveTop(screenGeometry.y());
            }
            
            // 确保窗口不超出屏幕
            if (validGeometry.right() > screenGeometry.right()) {
                validGeometry.moveRight(screenGeometry.right());
            }
            if (validGeometry.bottom() > screenGeometry.bottom()) {
                validGeometry.moveBottom(screenGeometry.bottom());
            }
        }
    }
    
    return validGeometry;
}

ApplicationSettings ConfigurationManager::validateAndFixSettings(const ApplicationSettings& settings) const
{
    ApplicationSettings validSettings = settings;
    
    // 验证并修复服务器URL
    if (!validateServerUrl(validSettings.defaultServerUrl)) {
        qWarning() << "Invalid server URL, using default:" << validSettings.defaultServerUrl;
        validSettings.defaultServerUrl = JitsiConstants::DEFAULT_SERVER_URL;
    }
    
    // 验证并修复服务器超时
    if (validSettings.serverTimeout <= 0 || validSettings.serverTimeout > 300) {
        qWarning() << "Invalid server timeout, using default:" << validSettings.serverTimeout;
        validSettings.serverTimeout = JitsiConstants::DEFAULT_SERVER_TIMEOUT;
    }
    
    // 验证并修复语言设置
    if (validSettings.language.isEmpty()) {
        qWarning() << "Empty language setting, using default";
        validSettings.language = JitsiConstants::DEFAULT_LANGUAGE;
    }
    
    // 验证并修复窗口几何
    validSettings.windowGeometry = validateWindowGeometry(validSettings.windowGeometry);
    
    // 验证并修复最大最近项目数
    if (validSettings.maxRecentItems < 0 || validSettings.maxRecentItems > 100) {
        qWarning() << "Invalid max recent items, using default:" << validSettings.maxRecentItems;
        validSettings.maxRecentItems = JitsiConstants::MAX_RECENT_ITEMS;
    }
    
    // 验证并清理最近URL列表
    QStringList validRecentUrls;
    for (const QString& url : validSettings.recentUrls) {
        if (validateServerUrl(url)) {
            validRecentUrls.append(url);
        } else {
            qWarning() << "Removing invalid recent URL:" << url;
        }
    }
    validSettings.recentUrls = validRecentUrls;
    
    // 限制最近URL列表大小
    while (validSettings.recentUrls.size() > validSettings.maxRecentItems) {
        validSettings.recentUrls.removeLast();
    }
    
    // 使用ApplicationSettings内置验证
    if (!validSettings.isValid()) {
        qWarning() << "Settings validation failed, resetting to defaults";
        validSettings.resetToDefaults();
    }
    
    return validSettings;
}

void ConfigurationManager::resetToDefaults()
{
    qDebug() << "Resetting configuration to defaults";
    
    ApplicationSettings defaultSettings;
    saveConfiguration(defaultSettings);
    
    qDebug() << "Configuration reset completed";
}

ApplicationSettings ConfigurationManager::currentConfiguration() const
{
    return m_config;
}

bool ConfigurationManager::validateConfiguration() const
{
    if (!m_settings) {
        return false;
    }
    
    // 检查配置文件是否可读写
    if (!m_settings->isWritable()) {
        qWarning() << "Configuration file is not writable";
        return false;
    }
    
    // 验证当前配置
    if (!m_config.isValid()) {
        qWarning() << "Current configuration is invalid";
        return false;
    }
    
    return true;
}

WindowStateManager* ConfigurationManager::windowStateManager() const
{
    return m_windowStateManager;
}QL
ist<RecentItem> ConfigurationManager::recentItems() const
{
    QList<RecentItem> items;
    
    if (!m_settings) {
        return items;
    }
    
    // 读取JSON格式的最近项目数据
    QString jsonData = m_settings->value("recent/items", QString()).toString();
    if (jsonData.isEmpty()) {
        return items;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse recent items JSON:" << error.errorString();
        return items;
    }
    
    if (!doc.isArray()) {
        qWarning() << "Recent items JSON is not an array";
        return items;
    }
    
    QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        if (value.isObject()) {
            RecentItem item = RecentItem::fromJson(value.toObject());
            if (item.isValid()) {
                items.append(item);
            }
        }
    }
    
    // 按时间戳排序（最新的在前）
    std::sort(items.begin(), items.end());
    
    return items;
}

void ConfigurationManager::addRecentItem(const RecentItem& item)
{
    if (!item.isValid()) {
        qWarning() << "Cannot add invalid recent item";
        return;
    }
    
    QList<RecentItem> items = recentItems();
    
    // 检查是否已存在相同URL的项目
    auto it = std::find(items.begin(), items.end(), item);
    if (it != items.end()) {
        // 更新现有项目
        it->updateAccess();
    } else {
        // 添加新项目
        items.prepend(item);
    }
    
    // 限制最大数量
    int maxItems = m_config.maxRecentItems;
    while (items.size() > maxItems) {
        items.removeLast();
    }
    
    // 保存更新后的列表
    setRecentItems(items);
}

void ConfigurationManager::removeRecentItem(const QString& url)
{
    QList<RecentItem> items = recentItems();
    
    auto it = std::remove_if(items.begin(), items.end(),
                            [&url](const RecentItem& item) {
                                return item.url == url;
                            });
    
    if (it != items.end()) {
        items.erase(it, items.end());
        setRecentItems(items);
    }
}

void ConfigurationManager::clearRecentItems()
{
    if (m_settings) {
        m_settings->remove("recent/items");
        m_settings->sync();
        emit recentItemsChanged();
    }
}

void ConfigurationManager::setRecentItems(const QList<RecentItem>& items)
{
    if (!m_settings) {
        return;
    }
    
    // 转换为JSON格式
    QJsonArray array;
    for (const RecentItem& item : items) {
        if (item.isValid()) {
            array.append(item.toJson());
        }
    }
    
    QJsonDocument doc(array);
    QString jsonData = doc.toJson(QJsonDocument::Compact);
    
    m_settings->setValue("recent/items", jsonData);
    m_settings->sync();
    
    emit recentItemsChanged();
}

int ConfigurationManager::maxRecentItems() const
{
    return m_config.maxRecentItems;
}

void ConfigurationManager::setMaxRecentItems(int maxItems)
{
    if (maxItems <= 0 || maxItems > 100) {
        qWarning() << "Invalid max recent items value:" << maxItems;
        return;
    }
    
    m_config.maxRecentItems = maxItems;
    
    // 如果当前项目数超过新的最大值，则删除多余的项目
    QList<RecentItem> items = recentItems();
    if (items.size() > maxItems) {
        items = items.mid(0, maxItems);
        setRecentItems(items);
    }
    
    saveConfiguration(m_config);
    emit configurationChanged();
}