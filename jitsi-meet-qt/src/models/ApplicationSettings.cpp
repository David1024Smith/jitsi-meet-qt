#include "models/ApplicationSettings.h"
#include "JitsiConstants.h"

#include <QUrl>
#include <QDebug>

ApplicationSettings::ApplicationSettings()
{
    resetToDefaults();
}

ApplicationSettings::ApplicationSettings(const ApplicationSettings& other)
    : defaultServerUrl(other.defaultServerUrl)
    , serverTimeout(other.serverTimeout)
    , language(other.language)
    , darkMode(other.darkMode)
    , windowGeometry(other.windowGeometry)
    , maximized(other.maximized)
    , rememberWindowState(other.rememberWindowState)
    , autoJoinAudio(other.autoJoinAudio)
    , autoJoinVideo(other.autoJoinVideo)
    , maxRecentItems(other.maxRecentItems)
    , recentUrls(other.recentUrls)
{
}

ApplicationSettings& ApplicationSettings::operator=(const ApplicationSettings& other)
{
    if (this != &other) {
        defaultServerUrl = other.defaultServerUrl;
        serverTimeout = other.serverTimeout;
        language = other.language;
        darkMode = other.darkMode;
        windowGeometry = other.windowGeometry;
        maximized = other.maximized;
        rememberWindowState = other.rememberWindowState;
        autoJoinAudio = other.autoJoinAudio;
        autoJoinVideo = other.autoJoinVideo;
        maxRecentItems = other.maxRecentItems;
        recentUrls = other.recentUrls;
    }
    return *this;
}

bool ApplicationSettings::operator==(const ApplicationSettings& other) const
{
    return defaultServerUrl == other.defaultServerUrl &&
           serverTimeout == other.serverTimeout &&
           language == other.language &&
           darkMode == other.darkMode &&
           windowGeometry == other.windowGeometry &&
           maximized == other.maximized &&
           rememberWindowState == other.rememberWindowState &&
           autoJoinAudio == other.autoJoinAudio &&
           autoJoinVideo == other.autoJoinVideo &&
           maxRecentItems == other.maxRecentItems &&
           recentUrls == other.recentUrls;
}

bool ApplicationSettings::operator!=(const ApplicationSettings& other) const
{
    return !(*this == other);
}

bool ApplicationSettings::isValid() const
{
    // 验证服务器URL
    QUrl url(defaultServerUrl);
    if (!url.isValid() || (url.scheme() != "http" && url.scheme() != "https")) {
        return false;
    }
    
    // 验证超时时间
    if (serverTimeout <= 0 || serverTimeout > 300) {
        return false;
    }
    
    // 验证窗口几何
    if (windowGeometry.width() < JitsiConstants::MIN_WINDOW_WIDTH ||
        windowGeometry.height() < JitsiConstants::MIN_WINDOW_HEIGHT) {
        return false;
    }
    
    // 验证最大最近项目数
    if (maxRecentItems < 0 || maxRecentItems > 100) {
        return false;
    }
    
    // 验证语言代码
    if (language.isEmpty()) {
        return false;
    }
    
    return true;
}

void ApplicationSettings::resetToDefaults()
{
    defaultServerUrl = JitsiConstants::DEFAULT_SERVER_URL;
    serverTimeout = JitsiConstants::DEFAULT_SERVER_TIMEOUT;
    language = JitsiConstants::DEFAULT_LANGUAGE;
    darkMode = false;
    windowGeometry = QRect(100, 100, JitsiConstants::DEFAULT_WINDOW_WIDTH, 
                          JitsiConstants::DEFAULT_WINDOW_HEIGHT);
    maximized = false;
    rememberWindowState = true;
    autoJoinAudio = true;
    autoJoinVideo = false;
    maxRecentItems = JitsiConstants::MAX_RECENT_ITEMS;
    recentUrls.clear();
    
    qDebug() << "ApplicationSettings reset to defaults";
}

QVariantMap ApplicationSettings::toVariantMap() const
{
    QVariantMap map;
    
    map["defaultServerUrl"] = defaultServerUrl;
    map["serverTimeout"] = serverTimeout;
    map["language"] = language;
    map["darkMode"] = darkMode;
    map["windowGeometry"] = windowGeometry;
    map["maximized"] = maximized;
    map["rememberWindowState"] = rememberWindowState;
    map["autoJoinAudio"] = autoJoinAudio;
    map["autoJoinVideo"] = autoJoinVideo;
    map["maxRecentItems"] = maxRecentItems;
    map["recentUrls"] = recentUrls;
    
    return map;
}

void ApplicationSettings::fromVariantMap(const QVariantMap& map)
{
    defaultServerUrl = map.value("defaultServerUrl", JitsiConstants::DEFAULT_SERVER_URL).toString();
    serverTimeout = map.value("serverTimeout", JitsiConstants::DEFAULT_SERVER_TIMEOUT).toInt();
    language = map.value("language", JitsiConstants::DEFAULT_LANGUAGE).toString();
    darkMode = map.value("darkMode", false).toBool();
    windowGeometry = map.value("windowGeometry", 
                              QRect(100, 100, JitsiConstants::DEFAULT_WINDOW_WIDTH, 
                                   JitsiConstants::DEFAULT_WINDOW_HEIGHT)).toRect();
    maximized = map.value("maximized", false).toBool();
    rememberWindowState = map.value("rememberWindowState", true).toBool();
    autoJoinAudio = map.value("autoJoinAudio", true).toBool();
    autoJoinVideo = map.value("autoJoinVideo", false).toBool();
    maxRecentItems = map.value("maxRecentItems", JitsiConstants::MAX_RECENT_ITEMS).toInt();
    recentUrls = map.value("recentUrls", QStringList()).toStringList();
    
    // 验证加载的数据
    if (!isValid()) {
        qWarning() << "Loaded settings are invalid, resetting to defaults";
        resetToDefaults();
    }
}

QString ApplicationSettings::toString() const
{
    return QString("ApplicationSettings { "
                  "serverUrl: %1, "
                  "timeout: %2s, "
                  "language: %3, "
                  "darkMode: %4, "
                  "geometry: %5x%6+%7+%8, "
                  "maximized: %9, "
                  "recentItems: %10 }")
           .arg(defaultServerUrl)
           .arg(serverTimeout)
           .arg(language)
           .arg(darkMode ? "true" : "false")
           .arg(windowGeometry.width())
           .arg(windowGeometry.height())
           .arg(windowGeometry.x())
           .arg(windowGeometry.y())
           .arg(maximized ? "true" : "false")
           .arg(recentUrls.size());
}