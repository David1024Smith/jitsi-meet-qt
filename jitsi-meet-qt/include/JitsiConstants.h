#ifndef JITSICONSTANTS_H
#define JITSICONSTANTS_H

#include <QString>

namespace JitsiConstants {
    // 应用程序信息
    const QString APP_NAME = "Jitsi Meet";
    const QString APP_VERSION = "1.0.0";
    const QString APP_ORGANIZATION = "Jitsi Meet Qt";
    const QString APP_DOMAIN = "jitsi.org";
    
    // 协议相关
    const QString PROTOCOL_SCHEME = "jitsi-meet";
    const QString PROTOCOL_PREFIX = PROTOCOL_SCHEME + "://";
    
    // 默认配置
    const QString DEFAULT_SERVER_URL = "https://meet.jit.si";
    const int DEFAULT_SERVER_TIMEOUT = 30;
    const QString DEFAULT_LANGUAGE = "auto";
    
    // 窗口配置
    const int MIN_WINDOW_WIDTH = 800;
    const int MIN_WINDOW_HEIGHT = 600;
    const int DEFAULT_WINDOW_WIDTH = 1024;
    const int DEFAULT_WINDOW_HEIGHT = 768;
    
    // 最近会议配置
    const int MAX_RECENT_ITEMS = 10;
    
    // 房间名生成配置
    const int ROOM_NAME_UPDATE_INTERVAL = 10000; // 10秒
    const int ROOM_NAME_ANIMATION_SPEED = 70;    // 70毫秒每字符
    
    // 配置键名
    namespace ConfigKeys {
        const QString SERVER_URL = "serverUrl";
        const QString SERVER_TIMEOUT = "serverTimeout";
        const QString LANGUAGE = "language";
        const QString WINDOW_GEOMETRY = "windowGeometry";
        const QString WINDOW_MAXIMIZED = "windowMaximized";
        const QString REMEMBER_WINDOW_STATE = "rememberWindowState";
        const QString RECENT_URLS = "recentUrls";
        const QString DARK_MODE = "darkMode";
        const QString AUTO_JOIN_AUDIO = "autoJoinAudio";
        const QString AUTO_JOIN_VIDEO = "autoJoinVideo";
        const QString MAX_RECENT_ITEMS = "maxRecentItems";
    }
    
    // 错误消息
    namespace ErrorMessages {
        const QString INVALID_URL = "Invalid URL format";
        const QString NETWORK_ERROR = "Network connection error";
        const QString WEBENGINE_ERROR = "Web engine error";
        const QString CONFIG_ERROR = "Configuration error";
        const QString PROTOCOL_ERROR = "Protocol handling error";
    }
    
    // UI文本键（用于国际化）
    namespace UIText {
        const QString WELCOME_TITLE = "welcome.title";
        const QString ENTER_ROOM_URL = "welcome.enterRoomUrl";
        const QString JOIN_BUTTON = "welcome.joinButton";
        const QString RECENT_MEETINGS = "welcome.recentMeetings";
        const QString NO_RECENT_MEETINGS = "welcome.noRecentMeetings";
        const QString SETTINGS = "menu.settings";
        const QString ABOUT = "menu.about";
        const QString BACK = "menu.back";
        const QString LOADING = "conference.loading";
        const QString LOAD_ERROR = "conference.loadError";
    }
}

#endif // JITSICONSTANTS_H