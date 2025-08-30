#include "NetworkUtils.h"
#include <QNetworkInterface>
#include <QHostInfo>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QUrl>
#include <QProcess>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>

bool NetworkUtils::isNetworkAvailable()
{
    // 检查是否有活动的网络接口
    const auto interfaces = getActiveInterfaces();
    for (const auto& interface : interfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            interface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            return true;
        }
    }
    return false;
}

NetworkUtils::ConnectionType NetworkUtils::getConnectionType()
{
    const auto interfaces = getActiveInterfaces();
    
    for (const auto& interface : interfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }
        
        const QString name = interface.name().toLower();
        
        // 检查以太网连接
        if (name.contains("eth") || name.contains("en")) {
            return Ethernet;
        }
        
        // 检查WiFi连接
        if (name.contains("wlan") || name.contains("wifi") || name.contains("wl")) {
            return WiFi;
        }
        
        // 检查移动网络连接
        if (name.contains("ppp") || name.contains("mobile") || name.contains("cellular")) {
            return Mobile;
        }
        
        // 检查VPN连接
        if (name.contains("vpn") || name.contains("tun") || name.contains("tap")) {
            return VPN;
        }
    }
    
    return Unknown;
}

QString NetworkUtils::getLocalIPAddress(bool preferIPv4)
{
    const auto interfaces = getActiveInterfaces();
    
    QString ipv4Address;
    QString ipv6Address;
    
    for (const auto& interface : interfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }
        
        const auto addresses = interface.addressEntries();
        for (const auto& entry : addresses) {
            const QHostAddress address = entry.ip();
            
            if (address.protocol() == QAbstractSocket::IPv4Protocol) {
                if (ipv4Address.isEmpty() && !isPrivateIPAddress(address.toString())) {
                    ipv4Address = address.toString();
                }
            } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
                if (ipv6Address.isEmpty() && !address.isLinkLocal()) {
                    ipv6Address = address.toString();
                }
            }
        }
    }
    
    if (preferIPv4 && !ipv4Address.isEmpty()) {
        return ipv4Address;
    }
    
    if (!ipv6Address.isEmpty()) {
        return ipv6Address;
    }
    
    return ipv4Address;
}

QList<QNetworkInterface> NetworkUtils::getNetworkInterfaces()
{
    return QNetworkInterface::allInterfaces();
}

QList<QNetworkInterface> NetworkUtils::getActiveInterfaces()
{
    QList<QNetworkInterface> activeInterfaces;
    const auto allInterfaces = QNetworkInterface::allInterfaces();
    
    for (const auto& interface : allInterfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            interface.flags().testFlag(QNetworkInterface::IsRunning)) {
            activeInterfaces.append(interface);
        }
    }
    
    return activeInterfaces;
}

QStringList NetworkUtils::resolveHostname(const QString& hostname)
{
    QStringList addresses;
    
    QHostInfo hostInfo = QHostInfo::fromName(hostname);
    if (hostInfo.error() == QHostInfo::NoError) {
        const auto addressList = hostInfo.addresses();
        for (const auto& address : addressList) {
            addresses.append(address.toString());
        }
    }
    
    return addresses;
}

bool NetworkUtils::isPortReachable(const QString& host, int port, int timeout)
{
    QTcpSocket socket;
    socket.connectToHost(host, port);
    
    return socket.waitForConnected(timeout);
}

int NetworkUtils::pingHost(const QString& host, int timeout)
{
    QElapsedTimer timer;
    timer.start();
    
    QTcpSocket socket;
    socket.connectToHost(host, 80);  // 使用HTTP端口进行连接测试
    
    if (socket.waitForConnected(timeout)) {
        socket.disconnectFromHost();
        return static_cast<int>(timer.elapsed());
    }
    
    return -1;  // 连接失败
}

QVariantMap NetworkUtils::getNetworkStats()
{
    QVariantMap stats;
    
    // 获取网络接口统计信息
    const auto interfaces = getActiveInterfaces();
    qint64 totalBytesReceived = 0;
    qint64 totalBytesSent = 0;
    
    for (const auto& interface : interfaces) {
        if (!interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            // 注意：Qt没有直接提供网络流量统计，这里只是示例结构
            // 实际实现可能需要平台特定的代码
        }
    }
    
    stats["interfaces_count"] = interfaces.size();
    stats["bytes_received"] = totalBytesReceived;
    stats["bytes_sent"] = totalBytesSent;
    stats["connection_type"] = static_cast<int>(getConnectionType());
    stats["local_ip"] = getLocalIPAddress();
    
    return stats;
}

QString NetworkUtils::formatBandwidth(qint64 bytes, int precision)
{
    const QStringList units = {"B/s", "KB/s", "MB/s", "GB/s", "TB/s"};
    
    double size = bytes;
    int unitIndex = 0;
    
    while (size >= 1024.0 && unitIndex < units.size() - 1) {
        size /= 1024.0;
        ++unitIndex;
    }
    
    return QString::number(size, 'f', precision) + " " + units[unitIndex];
}

QString NetworkUtils::formatLatency(int milliseconds)
{
    if (milliseconds < 0) {
        return "N/A";
    }
    
    if (milliseconds < 1000) {
        return QString::number(milliseconds) + " ms";
    }
    
    double seconds = milliseconds / 1000.0;
    return QString::number(seconds, 'f', 2) + " s";
}

bool NetworkUtils::isValidIPAddress(const QString& address)
{
    QHostAddress hostAddress(address);
    return !hostAddress.isNull();
}

bool NetworkUtils::isValidPort(int port)
{
    return port > 0 && port <= 65535;
}

QString NetworkUtils::getDefaultGateway()
{
    // 这是一个简化的实现，实际可能需要平台特定的代码
#ifdef Q_OS_WIN
    QProcess process;
    process.start("route", QStringList() << "print" << "0.0.0.0");
    process.waitForFinished();
    
    QString output = process.readAllStandardOutput();
    QRegularExpression regex(R"(0\.0\.0\.0\s+0\.0\.0\.0\s+(\d+\.\d+\.\d+\.\d+))");
    QRegularExpressionMatch match = regex.match(output);
    
    if (match.hasMatch()) {
        return match.captured(1);
    }
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    QProcess process;
    process.start("ip", QStringList() << "route" << "show" << "default");
    process.waitForFinished();
    
    QString output = process.readAllStandardOutput();
    QRegularExpression regex(R"(default via (\d+\.\d+\.\d+\.\d+))");
    QRegularExpressionMatch match = regex.match(output);
    
    if (match.hasMatch()) {
        return match.captured(1);
    }
#endif
    
    return QString();
}

QStringList NetworkUtils::getDNSServers()
{
    QStringList dnsServers;
    
    // 这是一个简化的实现，实际可能需要平台特定的代码
#ifdef Q_OS_WIN
    QProcess process;
    process.start("nslookup", QStringList());
    process.waitForFinished();
    
    QString output = process.readAllStandardOutput();
    QRegularExpression regex(R"(Server:\s+(\d+\.\d+\.\d+\.\d+))");
    QRegularExpressionMatchIterator matches = regex.globalMatch(output);
    
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        dnsServers.append(match.captured(1));
    }
#elif defined(Q_OS_LINUX)
    QFile file("/etc/resolv.conf");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.startsWith("nameserver")) {
                QStringList parts = line.split(" ", Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    dnsServers.append(parts[1]);
                }
            }
        }
    }
#endif
    
    return dnsServers;
}

bool NetworkUtils::isPrivateIPAddress(const QString& address)
{
    QHostAddress hostAddress(address);
    
    if (hostAddress.protocol() == QAbstractSocket::IPv4Protocol) {
        quint32 ip = hostAddress.toIPv4Address();
        
        // 10.0.0.0/8
        if ((ip & 0xFF000000) == 0x0A000000) return true;
        
        // 172.16.0.0/12
        if ((ip & 0xFFF00000) == 0xAC100000) return true;
        
        // 192.168.0.0/16
        if ((ip & 0xFFFF0000) == 0xC0A80000) return true;
        
        // 127.0.0.0/8 (loopback)
        if ((ip & 0xFF000000) == 0x7F000000) return true;
    }
    
    return false;
}

int NetworkUtils::calculateNetworkQuality(int latency, double packetLoss, int bandwidth)
{
    int score = 100;
    
    // 延迟评分 (0-40分)
    if (latency > 500) {
        score -= 40;
    } else if (latency > 200) {
        score -= 30;
    } else if (latency > 100) {
        score -= 20;
    } else if (latency > 50) {
        score -= 10;
    }
    
    // 丢包率评分 (0-30分)
    if (packetLoss > 10.0) {
        score -= 30;
    } else if (packetLoss > 5.0) {
        score -= 20;
    } else if (packetLoss > 2.0) {
        score -= 10;
    } else if (packetLoss > 1.0) {
        score -= 5;
    }
    
    // 带宽评分 (0-30分)
    if (bandwidth < 100) {  // < 100 kbps
        score -= 30;
    } else if (bandwidth < 500) {  // < 500 kbps
        score -= 20;
    } else if (bandwidth < 1000) {  // < 1 Mbps
        score -= 10;
    } else if (bandwidth < 2000) {  // < 2 Mbps
        score -= 5;
    }
    
    return qMax(0, qMin(100, score));
}

int NetworkUtils::generateRandomPort(int minPort, int maxPort)
{
    return QRandomGenerator::global()->bounded(minPort, maxPort + 1);
}

bool NetworkUtils::isValidUrl(const QString& url)
{
    QUrl qurl(url);
    return qurl.isValid() && !qurl.scheme().isEmpty() && !qurl.host().isEmpty();
}

QString NetworkUtils::extractHostFromUrl(const QString& url)
{
    QUrl qurl(url);
    return qurl.host();
}

int NetworkUtils::extractPortFromUrl(const QString& url)
{
    QUrl qurl(url);
    return qurl.port();
}