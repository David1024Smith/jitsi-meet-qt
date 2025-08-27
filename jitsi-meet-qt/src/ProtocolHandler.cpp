#include "ProtocolHandler.h"
#include "JitsiConstants.h"

#include <QApplication>
#include <QUrl>
#include <QRegularExpression>
#include <QDebug>

#ifdef Q_OS_WIN
#include <QDir>
#include <QStandardPaths>
#include <windows.h>
#endif

// 静态常量定义
const QString ProtocolHandler::PROTOCOL_SCHEME = JitsiConstants::PROTOCOL_SCHEME;

ProtocolHandler::ProtocolHandler(QObject* parent)
    : QObject(parent)
    , m_registered(false)
{
    qDebug() << "ProtocolHandler created";
}

ProtocolHandler::~ProtocolHandler()
{
    if (m_registered) {
        unregisterProtocol();
    }
}

bool ProtocolHandler::registerProtocol()
{
    if (m_registered) {
        qDebug() << "Protocol already registered";
        return true;
    }
    
#ifdef Q_OS_WIN
    bool success = registerWindowsProtocol();
    if (success) {
        m_registered = true;
        qDebug() << "Protocol registered successfully";
    } else {
        qWarning() << "Failed to register protocol";
    }
    return success;
#else
    qWarning() << "Protocol registration not implemented for this platform";
    return false;
#endif
}

void ProtocolHandler::unregisterProtocol()
{
    if (!m_registered) {
        return;
    }
    
#ifdef Q_OS_WIN
    unregisterWindowsProtocol();
#endif
    
    m_registered = false;
    qDebug() << "Protocol unregistered";
}

QString ProtocolHandler::parseProtocolUrl(const QString& url)
{
    if (!isValidProtocolUrl(url)) {
        qWarning() << "Invalid protocol URL:" << url;
        return QString();
    }
    
    qDebug() << "Parsing protocol URL:" << url;
    
    // 移除协议前缀
    QString cleanUrl = url;
    if (cleanUrl.startsWith(JitsiConstants::PROTOCOL_PREFIX)) {
        cleanUrl = cleanUrl.mid(JitsiConstants::PROTOCOL_PREFIX.length());
    }
    
    // 解析URL格式
    // 支持的格式:
    // 1. room-name -> https://meet.jit.si/room-name
    // 2. server.com/room-name -> https://server.com/room-name
    // 3. https://server.com/room-name -> https://server.com/room-name
    
    if (cleanUrl.isEmpty()) {
        qWarning() << "Empty room name in protocol URL";
        return QString();
    }
    
    // 如果已经是完整的HTTP(S) URL，直接返回
    if (cleanUrl.startsWith("http://") || cleanUrl.startsWith("https://")) {
        return cleanUrl;
    }
    
    // 检查是否包含服务器地址
    if (cleanUrl.contains('/')) {
        // 格式: server.com/room-name
        return "https://" + cleanUrl;
    } else {
        // 格式: room-name，使用默认服务器
        return JitsiConstants::DEFAULT_SERVER_URL + "/" + cleanUrl;
    }
}

bool ProtocolHandler::isValidProtocolUrl(const QString& url) const
{
    if (url.isEmpty()) {
        return false;
    }
    
    // 检查协议前缀
    if (!url.startsWith(JitsiConstants::PROTOCOL_PREFIX)) {
        return false;
    }
    
    // 提取房间信息
    QString roomInfo = extractRoomInfo(url);
    if (roomInfo.isEmpty()) {
        return false;
    }
    
    // 验证房间名格式（允许字母、数字、连字符、下划线、点号、斜杠）
    QRegularExpression roomRegex("^[a-zA-Z0-9._/-]+$");
    return roomRegex.match(roomInfo).hasMatch();
}

QString ProtocolHandler::extractRoomInfo(const QString& url) const
{
    if (!url.startsWith(JitsiConstants::PROTOCOL_PREFIX)) {
        return QString();
    }
    
    return url.mid(JitsiConstants::PROTOCOL_PREFIX.length());
}

QString ProtocolHandler::buildConferenceUrl(const QString& serverUrl, const QString& roomName) const
{
    QString cleanServerUrl = serverUrl;
    if (cleanServerUrl.endsWith('/')) {
        cleanServerUrl.chop(1);
    }
    
    QString cleanRoomName = roomName;
    if (cleanRoomName.startsWith('/')) {
        cleanRoomName = cleanRoomName.mid(1);
    }
    
    return cleanServerUrl + "/" + cleanRoomName;
}

#ifdef Q_OS_WIN
bool ProtocolHandler::registerWindowsProtocol()
{
    QString executablePath = getExecutablePath();
    if (executablePath.isEmpty()) {
        qWarning() << "Failed to get executable path";
        return false;
    }
    
    // 注册协议到注册表
    QSettings registry("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);
    
    // 设置协议根键
    QString protocolKey = PROTOCOL_SCHEME;
    registry.setValue(protocolKey + "/.", QString("URL:%1 Protocol").arg(JitsiConstants::APP_NAME));
    registry.setValue(protocolKey + "/URL Protocol", "");
    registry.setValue(protocolKey + "/DefaultIcon/.", QString("\"%1\",0").arg(executablePath));
    
    // 设置命令
    QString command = QString("\"%1\" \"%2\"").arg(executablePath).arg("%1");
    registry.setValue(protocolKey + "/shell/open/command/.", command);
    
    registry.sync();
    
    qDebug() << "Windows protocol registered with command:" << command;
    
    return true;
}

void ProtocolHandler::unregisterWindowsProtocol()
{
    QSettings registry("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);
    registry.remove(PROTOCOL_SCHEME);
    registry.sync();
    
    qDebug() << "Windows protocol unregistered";
}

QString ProtocolHandler::getExecutablePath() const
{
    QString path = QApplication::applicationFilePath();
    return QDir::toNativeSeparators(path);
}
#endif