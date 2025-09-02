#include "ProtocolHandler.h"
#include "MainApplication.h"
#include "ConfigurationManager.h"
#include <QApplication>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <QSettings>
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <QStandardPaths>
#include <QFileInfo>
#endif

#ifdef Q_OS_MAC
#include <QStandardPaths>
#endif

// 静态常量定义
const QString ProtocolHandler::PROTOCOL_NAME = "jitsi-meet";
const QString ProtocolHandler::PROTOCOL_SCHEME = "jitsi-meet://";
const QString ProtocolHandler::DEFAULT_SERVER = "https://meet.jit.si";

// 正则表达式定义
const QRegularExpression ProtocolHandler::ROOM_NAME_REGEX("^[a-zA-Z0-9][a-zA-Z0-9._-]{0,63}$");
const QRegularExpression ProtocolHandler::SERVER_URL_REGEX("^https?://[a-zA-Z0-9.-]+(?::[0-9]+)?(?:/.*)?$");
const QRegularExpression ProtocolHandler::PROTOCOL_URL_REGEX("^jitsi-meet://(?:([a-zA-Z0-9.-]+(?::[0-9]+)?)/)?([a-zA-Z0-9][a-zA-Z0-9._-]{0,63})(?:\\?(.*))?$");

/**
 * @brief 构造函数
 */
ProtocolHandler::ProtocolHandler(MainApplication* app, QObject *parent)
    : QObject(parent)
    , m_app(app)
    , m_delayTimer(new QTimer(this))
    , m_processingDelay(1000)
    , m_isRegistered(false)
{
    // 注册元类型
    qRegisterMetaType<ProtocolHandler::MeetingInfo>("ProtocolHandler::MeetingInfo");
    
    // 设置定时器
    m_delayTimer->setSingleShot(true);
    connect(m_delayTimer, &QTimer::timeout, this, &ProtocolHandler::processDelayedUrl);
    
    // 连接应用程序激活信号
    if (m_app) {
        connect(qApp, &QApplication::applicationStateChanged, this, &ProtocolHandler::onApplicationActivated);
    }
    
    initialize();
}

/**
 * @brief 析构函数
 */
ProtocolHandler::~ProtocolHandler()
{
    cleanup();
}

/**
 * @brief 初始化协议处理器
 */
void ProtocolHandler::initialize()
{
    logProtocolHandling("初始化协议处理器");
    
    // 检查是否已注册
    m_isRegistered = isProtocolRegistered();
    
    if (m_isRegistered) {
        logProtocolHandling("协议已注册");
    } else {
        logProtocolHandling("协议未注册，尝试注册");
        registerProtocol();
    }
}

/**
 * @brief 清理资源
 */
void ProtocolHandler::cleanup()
{
    if (m_delayTimer && m_delayTimer->isActive()) {
        m_delayTimer->stop();
    }
}

/**
 * @brief 注册协议处理器到系统
 */
bool ProtocolHandler::registerProtocol()
{
    QMutexLocker locker(&m_mutex);
    
    bool success = false;
    
#ifdef Q_OS_WIN
    success = registerWindowsProtocol();
#elif defined(Q_OS_LINUX)
    success = registerLinuxProtocol();
#elif defined(Q_OS_MAC)
    success = registerMacProtocol();
#else
    qWarning() << "不支持的操作系统，无法注册协议";
#endif
    
    if (success) {
        m_isRegistered = true;
        logProtocolHandling("协议注册成功");
        emit protocolRegistrationChanged(true);
    } else {
        logProtocolHandling("协议注册失败");
    }
    
    return success;
}

/**
 * @brief 取消注册协议处理器
 */
bool ProtocolHandler::unregisterProtocol()
{
    QMutexLocker locker(&m_mutex);
    
    bool success = false;
    
#ifdef Q_OS_WIN
    success = unregisterWindowsProtocol();
#elif defined(Q_OS_LINUX)
    success = unregisterLinuxProtocol();
#elif defined(Q_OS_MAC)
    success = unregisterMacProtocol();
#endif
    
    if (success) {
        m_isRegistered = false;
        logProtocolHandling("协议取消注册成功");
        emit protocolRegistrationChanged(false);
    }
    
    return success;
}

/**
 * @brief 检查协议是否已注册
 */
bool ProtocolHandler::isProtocolRegistered() const
{
#ifdef Q_OS_WIN
    return isWindowsProtocolRegistered();
#else
    return m_isRegistered;
#endif
}

/**
 * @brief 处理协议URL
 */
bool ProtocolHandler::handleProtocolUrl(const QString& url)
{
    logProtocolHandling("接收到协议URL", url);
    
    if (!isValidProtocolUrl(url)) {
        QString reason = "无效的协议URL格式";
        logProtocolHandling(reason, url);
        emit invalidUrlReceived(url, reason);
        return false;
    }
    
    // 如果应用程序还在启动过程中，延迟处理
    if (m_app && !m_app->isInitialized()) {
        m_pendingUrl = url;
        m_delayTimer->start(m_processingDelay);
        logProtocolHandling("应用程序未完全启动，延迟处理URL", url);
        return true;
    }
    
    // 立即处理URL
    return processUrl(url);
}

/**
 * @brief 处理URL的内部方法
 */
bool ProtocolHandler::processUrl(const QString& url)
{
    MeetingInfo meetingInfo = parseProtocolUrl(url);
    
    if (!meetingInfo.isValid) {
        QString reason = "URL解析失败";
        logProtocolHandling(reason, url);
        emit invalidUrlReceived(url, reason);
        return false;
    }
    
    m_lastProcessedUrl = url;
    logProtocolHandling(QString("成功解析URL - 房间: %1, 服务器: %2")
                       .arg(meetingInfo.roomName, meetingInfo.serverUrl), url);
    
    // 发送信号给主应用程序
    emit protocolUrlReceived(meetingInfo);
    
    return true;
}

/**
 * @brief 解析协议URL
 */
ProtocolHandler::MeetingInfo ProtocolHandler::parseProtocolUrl(const QString& url) const
{
    MeetingInfo info;
    
    if (!isValidProtocolUrl(url)) {
        return info;
    }
    
    QRegularExpressionMatch match = PROTOCOL_URL_REGEX.match(url);
    if (!match.hasMatch()) {
        return info;
    }
    
    QString server = match.captured(1);
    QString roomName = match.captured(2);
    QString queryString = match.captured(3);
    
    // 设置房间名称
    info.roomName = roomName;
    
    // 设置服务器URL
    if (server.isEmpty()) {
        // 使用默认服务器
        ConfigurationManager* config = ConfigurationManager::instance();
        info.serverUrl = config->getDefaultServerUrl();
    } else {
        // 使用指定服务器
        info.serverUrl = normalizeServerUrl(server);
    }
    
    // 构建完整URL
    info.fullUrl = info.serverUrl + "/" + info.roomName;
    
    // 解析查询参数
    if (!queryString.isEmpty()) {
        QUrl tempUrl;
        tempUrl.setQuery(queryString);
        QUrlQuery query(tempUrl);
        
        // 提取显示名称
        if (query.hasQueryItem("displayName")) {
            info.displayName = query.queryItemValue("displayName");
        }
        
        // 提取密码
        if (query.hasQueryItem("password")) {
            info.password = query.queryItemValue("password");
        }
        
        // 提取其他参数
        info.parameters = parseUrlParameters(tempUrl);
    }
    
    // 验证解析结果
    if (isValidRoomName(info.roomName) && isValidServerUrl(info.serverUrl)) {
        info.isValid = true;
    }
    
    return info;
}

/**
 * @brief 验证URL是否为有效的jitsi-meet协议URL
 */
bool ProtocolHandler::isValidProtocolUrl(const QString& url) const
{
    if (url.isEmpty() || !url.startsWith(PROTOCOL_SCHEME)) {
        return false;
    }
    
    return PROTOCOL_URL_REGEX.match(url).hasMatch();
}

/**
 * @brief 获取支持的协议名称
 */
QString ProtocolHandler::getProtocolName()
{
    return PROTOCOL_NAME;
}

/**
 * @brief 构建协议URL
 */
QString ProtocolHandler::buildProtocolUrl(const QString& roomName, 
                                         const QString& serverUrl,
                                         const QString& displayName,
                                         const QString& password)
{
    if (roomName.isEmpty()) {
        return QString();
    }
    
    QString url = PROTOCOL_SCHEME;
    
    // 添加服务器（如果不是默认服务器）
    if (!serverUrl.isEmpty() && serverUrl != DEFAULT_SERVER) {
        QUrl server(serverUrl);
        url += server.host();
        if (server.port() != -1) {
            url += ":" + QString::number(server.port());
        }
        url += "/";
    }
    
    // 添加房间名称
    url += roomName;
    
    // 添加查询参数
    QStringList params;
    if (!displayName.isEmpty()) {
        params << "displayName=" + QUrl::toPercentEncoding(displayName);
    }
    if (!password.isEmpty()) {
        params << "password=" + QUrl::toPercentEncoding(password);
    }
    
    if (!params.isEmpty()) {
        url += "?" + params.join("&");
    }
    
    return url;
}

/**
 * @brief 从HTTP/HTTPS URL转换为协议URL
 */
QString ProtocolHandler::convertFromHttpUrl(const QString& httpUrl)
{
    QUrl url(httpUrl);
    if (!url.isValid() || (url.scheme() != "http" && url.scheme() != "https")) {
        return QString();
    }
    
    QString path = url.path();
    if (path.startsWith("/")) {
        path = path.mid(1);
    }
    
    if (path.isEmpty()) {
        return QString();
    }
    
    QString serverUrl = url.scheme() + "://" + url.host();
    if (url.port() != -1) {
        serverUrl += ":" + QString::number(url.port());
    }
    
    return buildProtocolUrl(path, serverUrl);
}

/**
 * @brief 转换协议URL为HTTP URL
 */
QString ProtocolHandler::convertToHttpUrl(const QString& protocolUrl)
{
    ProtocolHandler handler(nullptr);
    MeetingInfo info = handler.parseProtocolUrl(protocolUrl);
    
    if (!info.isValid) {
        return QString();
    }
    
    return info.fullUrl;
}

/**
 * @brief 设置延迟处理时间
 */
void ProtocolHandler::setProcessingDelay(int delayMs)
{
    m_processingDelay = qMax(0, delayMs);
}

/**
 * @brief 获取最后处理的URL
 */
QString ProtocolHandler::getLastProcessedUrl() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastProcessedUrl;
}

/**
 * @brief 处理延迟的协议URL
 */
void ProtocolHandler::processDelayedUrl()
{
    if (!m_pendingUrl.isEmpty()) {
        QString url = m_pendingUrl;
        m_pendingUrl.clear();
        processUrl(url);
    }
}

/**
 * @brief 处理应用程序激活事件
 */
void ProtocolHandler::onApplicationActivated()
{
    // 当应用程序被激活时，处理待处理的URL
    if (!m_pendingUrl.isEmpty() && m_app && m_app->isInitialized()) {
        processDelayedUrl();
    }
}

#ifdef Q_OS_WIN
/**
 * @brief 注册Windows协议
 */
bool ProtocolHandler::registerWindowsProtocol()
{
    try {
        QString appPath = QApplication::applicationFilePath();
        QString protocolKey = QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(PROTOCOL_NAME);
        
        QSettings registry(protocolKey, QSettings::NativeFormat);
        registry.setValue(".", "Jitsi Meet Qt Protocol");
        registry.setValue("URL Protocol", "");
        
        QString commandKey = protocolKey + "\\shell\\open\\command";
        QSettings commandRegistry(commandKey, QSettings::NativeFormat);
        commandRegistry.setValue(".", QString("\"%1\" \"%2\"").arg(appPath, "%1"));
        
        logProtocolHandling("Windows协议注册完成");
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Windows协议注册失败:" << e.what();
        return false;
    }
}

/**
 * @brief 取消注册Windows协议
 */
bool ProtocolHandler::unregisterWindowsProtocol()
{
    try {
        QString protocolKey = QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(PROTOCOL_NAME);
        QSettings registry(protocolKey, QSettings::NativeFormat);
        registry.clear();
        
        logProtocolHandling("Windows协议取消注册完成");
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Windows协议取消注册失败:" << e.what();
        return false;
    }
}

/**
 * @brief 检查Windows协议注册状态
 */
bool ProtocolHandler::isWindowsProtocolRegistered() const
{
    QString protocolKey = QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(PROTOCOL_NAME);
    QSettings registry(protocolKey, QSettings::NativeFormat);
    return registry.contains("URL Protocol");
}
#endif

#ifdef Q_OS_LINUX
/**
 * @brief 注册Linux协议
 */
bool ProtocolHandler::registerLinuxProtocol()
{
    // 在Linux上创建.desktop文件
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QString desktopFile = desktopPath + "/jitsi-meet-qt.desktop";
    
    QDir().mkpath(desktopPath);
    
    QFile file(desktopFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "[Desktop Entry]\n";
        out << "Name=Jitsi Meet Qt\n";
        out << "Exec=" << QApplication::applicationFilePath() << " %u\n";
        out << "Type=Application\n";
        out << "MimeType=x-scheme-handler/" << PROTOCOL_NAME << ";\n";
        out << "Categories=Network;AudioVideo;\n";
        
        logProtocolHandling("Linux协议注册完成");
        return true;
    }
    
    return false;
}

/**
 * @brief 取消注册Linux协议
 */
bool ProtocolHandler::unregisterLinuxProtocol()
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QString desktopFile = desktopPath + "/jitsi-meet-qt.desktop";
    
    return QFile::remove(desktopFile);
}
#endif

#ifdef Q_OS_MAC
/**
 * @brief 注册macOS协议
 */
bool ProtocolHandler::registerMacProtocol()
{
    // macOS协议注册通常在Info.plist中完成
    // 这里可以添加运行时检查和注册逻辑
    logProtocolHandling("macOS协议注册（通过Info.plist）");
    return true;
}

/**
 * @brief 取消注册macOS协议
 */
bool ProtocolHandler::unregisterMacProtocol()
{
    // macOS协议取消注册
    return true;
}
#endif

/**
 * @brief 验证房间名称的有效性
 */
bool ProtocolHandler::isValidRoomName(const QString& roomName) const
{
    if (roomName.isEmpty() || roomName.length() > 64) {
        return false;
    }
    
    return ROOM_NAME_REGEX.match(roomName).hasMatch();
}

/**
 * @brief 验证服务器URL的有效性
 */
bool ProtocolHandler::isValidServerUrl(const QString& serverUrl) const
{
    if (serverUrl.isEmpty()) {
        return false;
    }
    
    QUrl url(serverUrl);
    return url.isValid() && (url.scheme() == "http" || url.scheme() == "https");
}

/**
 * @brief 标准化服务器URL
 */
QString ProtocolHandler::normalizeServerUrl(const QString& serverUrl) const
{
    QString normalized = serverUrl;
    
    // 如果没有协议前缀，添加https
    if (!normalized.startsWith("http://") && !normalized.startsWith("https://")) {
        normalized = "https://" + normalized;
    }
    
    // 移除末尾的斜杠
    if (normalized.endsWith("/")) {
        normalized.chop(1);
    }
    
    return normalized;
}

/**
 * @brief 解析URL参数
 */
QStringList ProtocolHandler::parseUrlParameters(const QUrl& url) const
{
    QStringList parameters;
    QUrlQuery query(url);
    
    for (const auto& item : query.queryItems()) {
        parameters << item.first + "=" + item.second;
    }
    
    return parameters;
}

/**
 * @brief 记录协议处理日志
 */
void ProtocolHandler::logProtocolHandling(const QString& message, const QString& url) const
{
    if (url.isEmpty()) {
        qDebug() << "[ProtocolHandler]" << message;
    } else {
        qDebug() << "[ProtocolHandler]" << message << "- URL:" << url;
    }
}