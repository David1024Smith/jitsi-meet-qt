#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <QObject>
#include <QString>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

/**
 * @brief 协议处理器，处理jitsi-meet://协议链接
 */
class ProtocolHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ProtocolHandler(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ProtocolHandler();
    
    /**
     * @brief 注册协议处理器
     * @return 注册成功返回true
     */
    bool registerProtocol();
    
    /**
     * @brief 注销协议处理器
     */
    void unregisterProtocol();
    
    /**
     * @brief 解析协议URL
     * @param url 协议URL
     * @return 解析后的会议URL
     */
    QString parseProtocolUrl(const QString& url);
    
    /**
     * @brief 检查是否为有效的协议URL
     * @param url 要检查的URL
     * @return 是否有效
     */
    bool isValidProtocolUrl(const QString& url) const;

signals:
    /**
     * @brief 接收到协议URL信号
     * @param url 协议URL
     */
    void protocolUrlReceived(const QString& url);

private:
    /**
     * @brief 提取房间信息
     * @param url 协议URL
     * @return 房间信息
     */
    QString extractRoomInfo(const QString& url) const;
    
    /**
     * @brief 构建会议URL
     * @param serverUrl 服务器URL
     * @param roomName 房间名
     * @return 完整的会议URL
     */
    QString buildConferenceUrl(const QString& serverUrl, const QString& roomName) const;
    
#ifdef Q_OS_WIN
    /**
     * @brief 在Windows注册表中注册协议
     * @return 注册成功返回true
     */
    bool registerWindowsProtocol();
    
    /**
     * @brief 从Windows注册表中注销协议
     */
    void unregisterWindowsProtocol();
    
    /**
     * @brief 获取应用程序可执行文件路径
     * @return 可执行文件路径
     */
    QString getExecutablePath() const;
#endif

private:
    static const QString PROTOCOL_SCHEME;
    bool m_registered;
};

#endif // PROTOCOLHANDLER_H