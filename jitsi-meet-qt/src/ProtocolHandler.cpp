#include "ProtocolHandler.h"
#include "JitsiConstants.h"

#include <QApplication>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QDebug>
#include <QSettings>

#ifdef Q_OS_WIN
#include <QDir>
#include <QStandardPaths>
#include <windows.h>
#endif

const QString ProtocolHandler::PROTOCOL_SCHEME = "jitsi-meet";

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
        return true;
    } else {
        qWarning() << "Failed to register protocol";
        return false;
    }
#else
    // For other platforms, we would implement the registration here
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
    
    QUrl protocolUrl(url);
    QString host = protocolUrl.host();
    QString path = protocolUrl.path();
    
    // Remove leading slash from path
    if (path.startsWith('/')) {
        path = path.mid(1);
    }
    
    // Extract room information
    QString roomInfo = extractRoomInfo(url);
    if (roomInfo.isEmpty()) {
        qWarning() << "Could not extract room information from URL:" << url;
        return QString();
    }
    
    // Build conference URL
    QString serverUrl = host.isEmpty() ? "meet.jit.si" : host;
    return buildConferenceUrl(serverUrl, roomInfo);
}

bool ProtocolHandler::isValidProtocolUrl(const QString& url) const
{
    if (url.isEmpty()) {
        return false;
    }
    
    QUrl protocolUrl(url);
    return protocolUrl.scheme() == PROTOCOL_SCHEME && protocolUrl.isValid();
}

QString ProtocolHandler::extractRoomInfo(const QString& url) const
{
    QUrl protocolUrl(url);
    QString path = protocolUrl.path();
    
    // Remove leading slash
    if (path.startsWith('/')) {
        path = path.mid(1);
    }
    
    // Extract room name from path
    QStringList pathParts = path.split('/', Qt::SkipEmptyParts);
    if (!pathParts.isEmpty()) {
        return pathParts.first();
    }
    
    // Try to get room from query parameters
    QUrlQuery query(protocolUrl);
    if (query.hasQueryItem("room")) {
        return query.queryItemValue("room");
    }
    
    return QString();
}

QString ProtocolHandler::buildConferenceUrl(const QString& serverUrl, const QString& roomName) const
{
    if (serverUrl.isEmpty() || roomName.isEmpty()) {
        return QString();
    }
    
    QString url = QString("https://%1/%2").arg(serverUrl, roomName);
    qDebug() << "Built conference URL:" << url;
    return url;
}

#ifdef Q_OS_WIN
bool ProtocolHandler::registerWindowsProtocol()
{
    QString executablePath = getExecutablePath();
    if (executablePath.isEmpty()) {
        qWarning() << "Could not determine executable path";
        return false;
    }
    
    QSettings registry("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
    
    // Register protocol scheme
    QString protocolKey = QString("%1").arg(PROTOCOL_SCHEME);
    registry.setValue(QString("%1/Default").arg(protocolKey), QString("URL:%1 Protocol").arg(PROTOCOL_SCHEME));
    registry.setValue(QString("%1/URL Protocol").arg(protocolKey), "");
    
    // Register command
    QString commandKey = QString("%1/shell/open/command").arg(protocolKey);
    QString command = QString("\"%1\" \"%2\"").arg(executablePath, "%1");
    registry.setValue(QString("%1/Default").arg(commandKey), command);
    
    registry.sync();
    
    qDebug() << "Registered protocol" << PROTOCOL_SCHEME << "with command:" << command;
    return true;
}

void ProtocolHandler::unregisterWindowsProtocol()
{
    QSettings registry("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
    registry.remove(PROTOCOL_SCHEME);
    registry.sync();
    
    qDebug() << "Unregistered protocol" << PROTOCOL_SCHEME;
}

QString ProtocolHandler::getExecutablePath() const
{
    return QApplication::applicationFilePath();
}
#endif