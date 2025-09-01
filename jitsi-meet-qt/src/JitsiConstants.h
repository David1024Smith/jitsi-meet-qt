#ifndef JITSICONSTANTS_H
#define JITSICONSTANTS_H

#include <QString>

/**
 * @brief Jitsi常量定义
 * 
 * 包含Jitsi应用中使用的各种常量定义
 */
namespace JitsiConstants {

// 应用信息
const QString APP_NAME = "Jitsi Meet";
const QString APP_VERSION = "1.0.0";
const QString APP_ORGANIZATION = "Jitsi";
const QString APP_DOMAIN = "jitsi.org";

// 协议信息
const QString PROTOCOL_SCHEME = "jitsi-meet";
const QString PROTOCOL_HOST = "join";
const QString PROTOCOL_PREFIX = "jitsi-meet://";
const QString DEFAULT_SERVER = "meet.jit.si";

// 配置键
const QString CONFIG_SERVER_URL = "serverUrl";
const QString CONFIG_DISPLAY_NAME = "displayName";
const QString CONFIG_AUDIO_MUTED = "startWithAudioMuted";
const QString CONFIG_VIDEO_MUTED = "startWithVideoMuted";
const QString CONFIG_SHOW_WATERMARK = "showWatermark";
const QString CONFIG_WELCOME_PAGE_ENABLED = "welcomePageEnabled";

// 会议参数
const QString PARAM_ROOM = "room";
const QString PARAM_SUBJECT = "subject";
const QString PARAM_TOKEN = "token";
const QString PARAM_CONFIG = "config";
const QString PARAM_INTERFACE_CONFIG = "interfaceConfig";

// 错误代码
const int ERROR_INVALID_URL = 1001;
const int ERROR_MISSING_ROOM = 1002;
const int ERROR_SERVER_UNREACHABLE = 1003;
const int ERROR_AUTH_FAILED = 1004;

// 事件名称
const QString EVENT_READY = "ready";
const QString EVENT_JOINED = "conferenceJoined";
const QString EVENT_LEFT = "conferenceLeft";
const QString EVENT_PARTICIPANT_JOINED = "participantJoined";
const QString EVENT_PARTICIPANT_LEFT = "participantLeft";
const QString EVENT_AUDIO_MUTE_CHANGED = "audioMuteStatusChanged";
const QString EVENT_VIDEO_MUTE_CHANGED = "videoMuteStatusChanged";

} // namespace JitsiConstants

#endif // JITSICONSTANTS_H
