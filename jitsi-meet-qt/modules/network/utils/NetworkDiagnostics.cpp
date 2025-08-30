#include "NetworkDiagnostics.h"
#include "NetworkUtils.h"
#include <QProcess>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include <QRegularExpression>
#include <QDebug>

class NetworkDiagnostics::Private
{
public:
    Private()
        : operatingSystem("")
    {
        operatingSystem = detectOperatingSystem();
    }

    QString operatingSystem;
    
    QString detectOperatingSystem()
    {
#ifdef Q_OS_WIN
        return "Windows";
#elif defined(Q_OS_LINUX)
        return "Linux";
#elif defined(Q_OS_MAC)
        return "macOS";
#else
        return "Unknown";
#endif
    }
};

NetworkDiagnostics::NetworkDiagnostics(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

NetworkDiagnostics::~NetworkDiagnostics()
{
    delete d;
}

QVariantMap NetworkDiagnostics::performFullDiagnostics(const QString& targetHost, int targetPort)
{
    qDebug() << "NetworkDiagnostics: Performing full diagnostics for" << targetHost << ":" << targetPort;
    
    QVariantMap fullResults;
    QVariantList testResults;
    
    emit diagnosticStarted(ConnectivityTest, targetHost);
    
    // 1. 网络接口测试
    emit diagnosticProgress(ConnectivityTest, 10, "Testing network interfaces...");
    QVariantMap interfaceResult = testNetworkInterfaces();
    testResults.append(interfaceResult);
    
    // 2. DNS解析测试
    emit diagnosticProgress(DNSTest, 20, "Testing DNS resolution...");
    QVariantMap dnsResult = testDNSResolution(targetHost);
    testResults.append(dnsResult);
    
    // 3. 连接性测试
    emit diagnosticProgress(ConnectivityTest, 30, "Testing connectivity...");
    QVariantMap connectivityResult = testConnectivity(targetHost, targetPort);
    testResults.append(connectivityResult);
    
    // 4. 延迟测试
    emit diagnosticProgress(LatencyTest, 50, "Testing latency...");
    QVariantMap latencyResult = testLatency(targetHost);
    testResults.append(latencyResult);
    
    // 5. 路由测试
    emit diagnosticProgress(RouteTest, 70, "Testing route...");
    QVariantMap routeResult = testRoute(targetHost);
    testResults.append(routeResult);
    
    // 6. 端口测试
    emit diagnosticProgress(PortTest, 80, "Testing ports...");
    QList<int> commonPorts = {80, 443, 22, 21, 25, 53, 110, 143, 993, 995};
    QVariantMap portResult = testPorts(targetHost, commonPorts);
    testResults.append(portResult);
    
    // 7. 防火墙测试
    emit diagnosticProgress(FirewallTest, 90, "Testing firewall...");
    QVariantMap firewallResult = testFirewall(targetHost, targetPort);
    testResults.append(firewallResult);
    
    emit diagnosticProgress(ConnectivityTest, 100, "Diagnostics completed");
    
    // 分析结果
    QVariantMap analysis = analyzeNetworkIssues(testResults);
    
    fullResults["target_host"] = targetHost;
    fullResults["target_port"] = targetPort;
    fullResults["timestamp"] = QDateTime::currentDateTime();
    fullResults["test_results"] = testResults;
    fullResults["analysis"] = analysis;
    fullResults["suggestions"] = getOptimizationSuggestions(fullResults);
    fullResults["report"] = generateDiagnosticReport(testResults);
    
    emit diagnosticCompleted(ConnectivityTest, fullResults);
    
    qDebug() << "NetworkDiagnostics: Full diagnostics completed";
    return fullResults;
}

QVariantMap NetworkDiagnostics::testConnectivity(const QString& host, int port, int timeout)
{
    qDebug() << "NetworkDiagnostics: Testing connectivity to" << host << ":" << port;
    
    emit diagnosticStarted(ConnectivityTest, QString("%1:%2").arg(host).arg(port));
    
    QTcpSocket socket;
    QElapsedTimer timer;
    timer.start();
    
    socket.connectToHost(host, port);
    bool connected = socket.waitForConnected(timeout);
    
    qint64 connectionTime = timer.elapsed();
    
    QVariantMap data;
    data["host"] = host;
    data["port"] = port;
    data["connection_time"] = connectionTime;
    data["timeout"] = timeout;
    
    DiagnosticStatus status;
    QString message;
    
    if (connected) {
        status = Success;
        message = QString("Successfully connected to %1:%2 in %3ms").arg(host).arg(port).arg(connectionTime);
        socket.disconnectFromHost();
    } else {
        if (connectionTime >= timeout) {
            status = Timeout;
            message = QString("Connection to %1:%2 timed out after %3ms").arg(host).arg(port).arg(timeout);
        } else {
            status = Error;
            message = QString("Failed to connect to %1:%2: %3").arg(host).arg(port).arg(socket.errorString());
        }
    }
    
    data["error_string"] = socket.errorString();
    
    QVariantMap result = createTestResult(ConnectivityTest, status, message, data);
    emit diagnosticCompleted(ConnectivityTest, result);
    
    return result;
}

QVariantMap NetworkDiagnostics::testLatency(const QString& host, int count, int timeout)
{
    qDebug() << "NetworkDiagnostics: Testing latency to" << host << "with" << count << "pings";
    
    emit diagnosticStarted(LatencyTest, host);
    
    QVariantMap data;
    data["host"] = host;
    data["count"] = count;
    data["timeout"] = timeout;
    
    // 使用系统ping命令
    QPair<QString, QStringList> pingCmd = getPingCommand(host, count);
    QString output = executeSystemCommand(pingCmd.first, pingCmd.second, timeout * count + 5000);
    
    QVariantMap pingData = parsePingOutput(output);
    data.unite(pingData);
    
    DiagnosticStatus status;
    QString message;
    
    if (pingData.contains("average_latency")) {
        double avgLatency = pingData["average_latency"].toDouble();
        int packetsLost = pingData.value("packets_lost", 0).toInt();
        double packetLoss = (static_cast<double>(packetsLost) / count) * 100.0;
        
        data["packet_loss_percentage"] = packetLoss;
        
        if (packetLoss == 0 && avgLatency < 100) {
            status = Success;
            message = QString("Excellent latency: %.1fms (0%% packet loss)").arg(avgLatency);
        } else if (packetLoss < 5 && avgLatency < 200) {
            status = Warning;
            message = QString("Good latency: %.1fms (%.1f%% packet loss)").arg(avgLatency).arg(packetLoss);
        } else {
            status = Error;
            message = QString("Poor latency: %.1fms (%.1f%% packet loss)").arg(avgLatency).arg(packetLoss);
        }
    } else {
        status = Error;
        message = QString("Failed to ping %1").arg(host);
    }
    
    QVariantMap result = createTestResult(LatencyTest, status, message, data);
    emit diagnosticCompleted(LatencyTest, result);
    
    return result;
}

QVariantMap NetworkDiagnostics::testDNSResolution(const QString& hostname, const QString& dnsServer)
{
    qDebug() << "NetworkDiagnostics: Testing DNS resolution for" << hostname;
    
    emit diagnosticStarted(DNSTest, hostname);
    
    QVariantMap data;
    data["hostname"] = hostname;
    data["dns_server"] = dnsServer;
    
    QElapsedTimer timer;
    timer.start();
    
    // 使用Qt的DNS解析
    QHostInfo hostInfo = QHostInfo::fromName(hostname);
    qint64 resolutionTime = timer.elapsed();
    
    data["resolution_time"] = resolutionTime;
    
    DiagnosticStatus status;
    QString message;
    
    if (hostInfo.error() == QHostInfo::NoError) {
        QStringList addresses;
        for (const QHostAddress& address : hostInfo.addresses()) {
            addresses.append(address.toString());
        }
        
        data["resolved_addresses"] = addresses;
        data["address_count"] = addresses.size();
        
        status = Success;
        message = QString("DNS resolution successful for %1 (%2 addresses found in %3ms)")
                    .arg(hostname).arg(addresses.size()).arg(resolutionTime);
    } else {
        status = Error;
        message = QString("DNS resolution failed for %1: %2").arg(hostname).arg(hostInfo.errorString());
        data["error_string"] = hostInfo.errorString();
    }
    
    // 如果指定了DNS服务器，也测试使用nslookup
    if (!dnsServer.isEmpty()) {
        QPair<QString, QStringList> nslookupCmd = getNslookupCommand(hostname, dnsServer);
        QString nslookupOutput = executeSystemCommand(nslookupCmd.first, nslookupCmd.second);
        QVariantMap nslookupData = parseNslookupOutput(nslookupOutput);
        data["nslookup_result"] = nslookupData;
    }
    
    QVariantMap result = createTestResult(DNSTest, status, message, data);
    emit diagnosticCompleted(DNSTest, result);
    
    return result;
}

QVariantMap NetworkDiagnostics::testRoute(const QString& host, int maxHops)
{
    qDebug() << "NetworkDiagnostics: Testing route to" << host;
    
    emit diagnosticStarted(RouteTest, host);
    
    QVariantMap data;
    data["host"] = host;
    data["max_hops"] = maxHops;
    
    // 使用系统traceroute命令
    QPair<QString, QStringList> traceCmd = getTracerouteCommand(host, maxHops);
    QString output = executeSystemCommand(traceCmd.first, traceCmd.second, 60000); // 60秒超时
    
    QVariantMap traceData = parseTracerouteOutput(output);
    data.unite(traceData);
    
    DiagnosticStatus status;
    QString message;
    
    if (traceData.contains("hops")) {
        QVariantList hops = traceData["hops"].toList();
        status = Success;
        message = QString("Route trace completed: %1 hops to %2").arg(hops.size()).arg(host);
    } else {
        status = Error;
        message = QString("Route trace failed for %1").arg(host);
    }
    
    QVariantMap result = createTestResult(RouteTest, status, message, data);
    emit diagnosticCompleted(RouteTest, result);
    
    return result;
}

QVariantMap NetworkDiagnostics::testPorts(const QString& host, const QList<int>& ports, int timeout)
{
    qDebug() << "NetworkDiagnostics: Testing ports on" << host;
    
    emit diagnosticStarted(PortTest, host);
    
    QVariantMap data;
    data["host"] = host;
    data["timeout"] = timeout;
    
    QVariantList portResults;
    QStringList openPorts;
    QStringList closedPorts;
    
    for (int port : ports) {
        QTcpSocket socket;
        socket.connectToHost(host, port);
        bool isOpen = socket.waitForConnected(timeout);
        
        QVariantMap portResult;
        portResult["port"] = port;
        portResult["open"] = isOpen;
        
        if (isOpen) {
            openPorts.append(QString::number(port));
            socket.disconnectFromHost();
        } else {
            closedPorts.append(QString::number(port));
        }
        
        portResults.append(portResult);
    }
    
    data["port_results"] = portResults;
    data["open_ports"] = openPorts;
    data["closed_ports"] = closedPorts;
    data["total_ports_tested"] = ports.size();
    data["open_ports_count"] = openPorts.size();
    
    DiagnosticStatus status;
    QString message;
    
    if (openPorts.size() > 0) {
        status = Success;
        message = QString("Port scan completed: %1/%2 ports open on %3")
                    .arg(openPorts.size()).arg(ports.size()).arg(host);
    } else {
        status = Warning;
        message = QString("Port scan completed: No open ports found on %1").arg(host);
    }
    
    QVariantMap result = createTestResult(PortTest, status, message, data);
    emit diagnosticCompleted(PortTest, result);
    
    return result;
}

QVariantMap NetworkDiagnostics::testFirewall(const QString& host, int port)
{
    qDebug() << "NetworkDiagnostics: Testing firewall for" << host << ":" << port;
    
    emit diagnosticStarted(FirewallTest, QString("%1:%2").arg(host).arg(port));
    
    QVariantMap data;
    data["host"] = host;
    data["port"] = port;
    
    // 测试TCP连接
    QTcpSocket tcpSocket;
    tcpSocket.connectToHost(host, port);
    bool tcpConnected = tcpSocket.waitForConnected(5000);
    
    data["tcp_connection"] = tcpConnected;
    
    if (tcpConnected) {
        tcpSocket.disconnectFromHost();
    }
    
    // 测试UDP连接（简单测试）
    QUdpSocket udpSocket;
    udpSocket.connectToHost(host, port);
    bool udpConnected = (udpSocket.state() == QAbstractSocket::ConnectedState);
    
    data["udp_connection"] = udpConnected;
    
    DiagnosticStatus status;
    QString message;
    
    if (tcpConnected) {
        status = Success;
        message = QString("No firewall blocking detected for %1:%2").arg(host).arg(port);
    } else {
        status = Warning;
        message = QString("Possible firewall blocking connection to %1:%2").arg(host).arg(port);
    }
    
    QVariantMap result = createTestResult(FirewallTest, status, message, data);
    emit diagnosticCompleted(FirewallTest, result);
    
    return result;
}

QVariantMap NetworkDiagnostics::testNetworkInterfaces()
{
    qDebug() << "NetworkDiagnostics: Testing network interfaces";
    
    QVariantMap data;
    QVariantList interfaceList;
    
    const auto interfaces = QNetworkInterface::allInterfaces();
    int activeCount = 0;
    
    for (const auto& interface : interfaces) {
        QVariantMap interfaceData;
        interfaceData["name"] = interface.name();
        interfaceData["human_readable_name"] = interface.humanReadableName();
        interfaceData["hardware_address"] = interface.hardwareAddress();
        interfaceData["is_up"] = interface.flags().testFlag(QNetworkInterface::IsUp);
        interfaceData["is_running"] = interface.flags().testFlag(QNetworkInterface::IsRunning);
        interfaceData["is_loopback"] = interface.flags().testFlag(QNetworkInterface::IsLoopBack);
        
        QStringList addresses;
        for (const auto& entry : interface.addressEntries()) {
            addresses.append(entry.ip().toString());
        }
        interfaceData["addresses"] = addresses;
        
        if (interface.flags().testFlag(QNetworkInterface::IsUp) && 
            interface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            activeCount++;
        }
        
        interfaceList.append(interfaceData);
    }
    
    data["interfaces"] = interfaceList;
    data["total_interfaces"] = interfaces.size();
    data["active_interfaces"] = activeCount;
    
    DiagnosticStatus status;
    QString message;
    
    if (activeCount > 0) {
        status = Success;
        message = QString("Network interfaces OK: %1 active interface(s) found").arg(activeCount);
    } else {
        status = Error;
        message = "No active network interfaces found";
    }
    
    return createTestResult(ConnectivityTest, status, message, data);
}

QVariantMap NetworkDiagnostics::testDefaultGateway()
{
    qDebug() << "NetworkDiagnostics: Testing default gateway";
    
    QVariantMap data;
    QString gateway = NetworkUtils::getDefaultGateway();
    
    data["gateway"] = gateway;
    
    DiagnosticStatus status;
    QString message;
    
    if (!gateway.isEmpty()) {
        // 测试网关连通性
        bool reachable = NetworkUtils::isPortReachable(gateway, 80, 3000);
        data["reachable"] = reachable;
        
        if (reachable) {
            status = Success;
            message = QString("Default gateway %1 is reachable").arg(gateway);
        } else {
            status = Warning;
            message = QString("Default gateway %1 found but not reachable").arg(gateway);
        }
    } else {
        status = Error;
        message = "No default gateway found";
    }
    
    return createTestResult(ConnectivityTest, status, message, data);
}

QVariantMap NetworkDiagnostics::testDNSServers()
{
    qDebug() << "NetworkDiagnostics: Testing DNS servers";
    
    QVariantMap data;
    QStringList dnsServers = NetworkUtils::getDNSServers();
    
    data["dns_servers"] = dnsServers;
    data["dns_server_count"] = dnsServers.size();
    
    QVariantList serverTests;
    int workingServers = 0;
    
    for (const QString& server : dnsServers) {
        QVariantMap serverTest = testDNSResolution("google.com", server);
        serverTests.append(serverTest);
        
        if (serverTest["status"].toInt() == Success) {
            workingServers++;
        }
    }
    
    data["server_tests"] = serverTests;
    data["working_servers"] = workingServers;
    
    DiagnosticStatus status;
    QString message;
    
    if (workingServers > 0) {
        status = Success;
        message = QString("DNS servers OK: %1/%2 servers working").arg(workingServers).arg(dnsServers.size());
    } else if (dnsServers.size() > 0) {
        status = Error;
        message = QString("DNS servers found but none are working (%1 servers tested)").arg(dnsServers.size());
    } else {
        status = Error;
        message = "No DNS servers found";
    }
    
    return createTestResult(DNSTest, status, message, data);
}QStri
ng NetworkDiagnostics::generateDiagnosticReport(const QVariantList& results)
{
    QString report;
    report += "=== Network Diagnostic Report ===\n";
    report += QString("Generated: %1\n").arg(QDateTime::currentDateTime().toString());
    report += QString("System: %1\n\n").arg(d->operatingSystem);
    
    for (const QVariant& result : results) {
        QVariantMap resultMap = result.toMap();
        report += formatTestResult(resultMap);
        report += "\n";
    }
    
    return report;
}

QVariantMap NetworkDiagnostics::getNetworkConfiguration()
{
    QVariantMap config;
    
    config["local_ip"] = NetworkUtils::getLocalIPAddress();
    config["connection_type"] = static_cast<int>(NetworkUtils::getConnectionType());
    config["default_gateway"] = NetworkUtils::getDefaultGateway();
    config["dns_servers"] = NetworkUtils::getDNSServers();
    config["network_available"] = NetworkUtils::isNetworkAvailable();
    
    return config;
}

QVariantMap NetworkDiagnostics::getSystemNetworkInfo()
{
    QVariantMap info;
    
    info["operating_system"] = d->operatingSystem;
    info["network_interfaces"] = testNetworkInterfaces();
    info["network_configuration"] = getNetworkConfiguration();
    info["network_stats"] = NetworkUtils::getNetworkStats();
    
    return info;
}

QVariantMap NetworkDiagnostics::diagnoseConnectionIssues(const QString& host, int port)
{
    qDebug() << "NetworkDiagnostics: Diagnosing connection issues for" << host << ":" << port;
    
    QVariantMap diagnosis;
    QStringList issues;
    QStringList suggestions;
    
    // 1. 检查网络接口
    QVariantMap interfaceTest = testNetworkInterfaces();
    if (interfaceTest["status"].toInt() != Success) {
        issues.append("No active network interfaces");
        suggestions.append("Check network adapter settings and ensure network is enabled");
    }
    
    // 2. 检查DNS解析
    QVariantMap dnsTest = testDNSResolution(host);
    if (dnsTest["status"].toInt() != Success) {
        issues.append("DNS resolution failed");
        suggestions.append("Check DNS server settings or try using IP address directly");
    }
    
    // 3. 检查连接性
    QVariantMap connectTest = testConnectivity(host, port);
    if (connectTest["status"].toInt() != Success) {
        issues.append("Cannot establish connection");
        suggestions.append("Check if the service is running and port is correct");
    }
    
    // 4. 检查防火墙
    QVariantMap firewallTest = testFirewall(host, port);
    if (firewallTest["status"].toInt() != Success) {
        issues.append("Possible firewall blocking");
        suggestions.append("Check firewall settings and add exception if needed");
    }
    
    diagnosis["issues"] = issues;
    diagnosis["suggestions"] = suggestions;
    diagnosis["severity"] = issues.isEmpty() ? "none" : (issues.size() > 2 ? "high" : "medium");
    
    return diagnosis;
}

QStringList NetworkDiagnostics::getOptimizationSuggestions(const QVariantMap& diagnosticResults)
{
    QStringList suggestions;
    
    QVariantList testResults = diagnosticResults["test_results"].toList();
    
    for (const QVariant& result : testResults) {
        QVariantMap resultMap = result.toMap();
        int testType = resultMap["test_type"].toInt();
        int status = resultMap["status"].toInt();
        QVariantMap data = resultMap["data"].toMap();
        
        switch (testType) {
            case LatencyTest:
                if (status != Success) {
                    if (data.contains("average_latency")) {
                        double latency = data["average_latency"].toDouble();
                        if (latency > 200) {
                            suggestions.append("High latency detected. Consider using a closer server or check network congestion.");
                        }
                    }
                    if (data.contains("packet_loss_percentage")) {
                        double packetLoss = data["packet_loss_percentage"].toDouble();
                        if (packetLoss > 5) {
                            suggestions.append("High packet loss detected. Check network stability and quality.");
                        }
                    }
                }
                break;
                
            case DNSTest:
                if (status != Success) {
                    suggestions.append("DNS resolution issues detected. Consider using alternative DNS servers (8.8.8.8, 1.1.1.1).");
                }
                break;
                
            case ConnectivityTest:
                if (status != Success) {
                    suggestions.append("Connectivity issues detected. Check network connection and target service availability.");
                }
                break;
                
            case FirewallTest:
                if (status != Success) {
                    suggestions.append("Firewall may be blocking connections. Check firewall settings and add necessary exceptions.");
                }
                break;
        }
    }
    
    if (suggestions.isEmpty()) {
        suggestions.append("Network diagnostics show no major issues. Performance appears to be optimal.");
    }
    
    return suggestions;
}

void NetworkDiagnostics::performQuickCheck(const QString& host)
{
    qDebug() << "NetworkDiagnostics: Performing quick check for" << host;
    
    // 快速检查：连接性 + DNS + 延迟
    QVariantMap connectResult = testConnectivity(host);
    QVariantMap dnsResult = testDNSResolution(host);
    QVariantMap latencyResult = testLatency(host, 3); // 只ping 3次
    
    QVariantList results;
    results.append(connectResult);
    results.append(dnsResult);
    results.append(latencyResult);
    
    QVariantMap quickCheckResult;
    quickCheckResult["target"] = host;
    quickCheckResult["timestamp"] = QDateTime::currentDateTime();
    quickCheckResult["results"] = results;
    
    emit diagnosticCompleted(ConnectivityTest, quickCheckResult);
}

QVariantMap NetworkDiagnostics::createTestResult(TestType testType, DiagnosticStatus status, 
                                               const QString& message, const QVariantMap& data)
{
    QVariantMap result;
    result["test_type"] = static_cast<int>(testType);
    result["status"] = static_cast<int>(status);
    result["message"] = message;
    result["timestamp"] = QDateTime::currentDateTime();
    result["data"] = data;
    
    return result;
}

QString NetworkDiagnostics::executeSystemCommand(const QString& command, const QStringList& arguments, int timeout)
{
    QProcess process;
    process.start(command, arguments);
    
    if (!process.waitForFinished(timeout)) {
        process.kill();
        return QString();
    }
    
    return process.readAllStandardOutput();
}

QVariantMap NetworkDiagnostics::parsePingOutput(const QString& output)
{
    QVariantMap result;
    
    if (output.isEmpty()) {
        return result;
    }
    
    // 解析ping输出（简化版本，实际需要根据不同操作系统调整）
    QRegularExpression latencyRegex(R"(time[<=](\d+(?:\.\d+)?)ms)");
    QRegularExpression lossRegex(R"((\d+)% packet loss)");
    QRegularExpression avgRegex(R"(avg[/=](\d+(?:\.\d+)?))");
    
    QRegularExpressionMatchIterator latencyMatches = latencyRegex.globalMatch(output);
    QList<double> latencies;
    
    while (latencyMatches.hasNext()) {
        QRegularExpressionMatch match = latencyMatches.next();
        latencies.append(match.captured(1).toDouble());
    }
    
    if (!latencies.isEmpty()) {
        double sum = 0;
        for (double latency : latencies) {
            sum += latency;
        }
        result["average_latency"] = sum / latencies.size();
        result["min_latency"] = *std::min_element(latencies.begin(), latencies.end());
        result["max_latency"] = *std::max_element(latencies.begin(), latencies.end());
        result["packets_sent"] = latencies.size();
    }
    
    QRegularExpressionMatch lossMatch = lossRegex.match(output);
    if (lossMatch.hasMatch()) {
        result["packet_loss"] = lossMatch.captured(1).toInt();
    }
    
    return result;
}

QVariantMap NetworkDiagnostics::parseTracerouteOutput(const QString& output)
{
    QVariantMap result;
    QVariantList hops;
    
    if (output.isEmpty()) {
        return result;
    }
    
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        // 简化的traceroute解析
        QRegularExpression hopRegex(R"(^\s*(\d+)\s+(.+))");
        QRegularExpressionMatch match = hopRegex.match(line);
        
        if (match.hasMatch()) {
            QVariantMap hop;
            hop["number"] = match.captured(1).toInt();
            hop["info"] = match.captured(2).trimmed();
            hops.append(hop);
        }
    }
    
    result["hops"] = hops;
    result["hop_count"] = hops.size();
    
    return result;
}

QVariantMap NetworkDiagnostics::parseNslookupOutput(const QString& output)
{
    QVariantMap result;
    QStringList addresses;
    
    if (output.isEmpty()) {
        return result;
    }
    
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        // 查找IP地址
        QRegularExpression ipRegex(R"(\b(?:\d{1,3}\.){3}\d{1,3}\b)");
        QRegularExpressionMatchIterator matches = ipRegex.globalMatch(line);
        
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            QString ip = match.captured(0);
            if (!addresses.contains(ip)) {
                addresses.append(ip);
            }
        }
    }
    
    result["addresses"] = addresses;
    result["address_count"] = addresses.size();
    
    return result;
}

QString NetworkDiagnostics::detectOperatingSystem()
{
    return d->detectOperatingSystem();
}

QPair<QString, QStringList> NetworkDiagnostics::getPingCommand(const QString& host, int count)
{
    QStringList args;
    
#ifdef Q_OS_WIN
    args << "-n" << QString::number(count) << host;
    return qMakePair(QString("ping"), args);
#else
    args << "-c" << QString::number(count) << host;
    return qMakePair(QString("ping"), args);
#endif
}

QPair<QString, QStringList> NetworkDiagnostics::getTracerouteCommand(const QString& host, int maxHops)
{
    QStringList args;
    
#ifdef Q_OS_WIN
    args << "-h" << QString::number(maxHops) << host;
    return qMakePair(QString("tracert"), args);
#else
    args << "-m" << QString::number(maxHops) << host;
    return qMakePair(QString("traceroute"), args);
#endif
}

QPair<QString, QStringList> NetworkDiagnostics::getNslookupCommand(const QString& hostname, const QString& dnsServer)
{
    QStringList args;
    args << hostname;
    
    if (!dnsServer.isEmpty()) {
        args << dnsServer;
    }
    
    return qMakePair(QString("nslookup"), args);
}

QVariantMap NetworkDiagnostics::analyzeNetworkIssues(const QVariantList& results)
{
    QVariantMap analysis;
    
    int successCount = 0;
    int warningCount = 0;
    int errorCount = 0;
    
    QStringList criticalIssues;
    QStringList warnings;
    
    for (const QVariant& result : results) {
        QVariantMap resultMap = result.toMap();
        int status = resultMap["status"].toInt();
        QString message = resultMap["message"].toString();
        
        switch (status) {
            case Success:
                successCount++;
                break;
            case Warning:
                warningCount++;
                warnings.append(message);
                break;
            case Error:
            case Timeout:
                errorCount++;
                criticalIssues.append(message);
                break;
        }
    }
    
    analysis["total_tests"] = results.size();
    analysis["success_count"] = successCount;
    analysis["warning_count"] = warningCount;
    analysis["error_count"] = errorCount;
    analysis["critical_issues"] = criticalIssues;
    analysis["warnings"] = warnings;
    
    // 计算整体健康分数
    int totalTests = results.size();
    if (totalTests > 0) {
        int score = (successCount * 100 + warningCount * 50) / totalTests;
        analysis["health_score"] = score;
        
        if (score >= 80) {
            analysis["overall_status"] = "Good";
        } else if (score >= 60) {
            analysis["overall_status"] = "Fair";
        } else {
            analysis["overall_status"] = "Poor";
        }
    }
    
    return analysis;
}

QString NetworkDiagnostics::formatTestResult(const QVariantMap& result)
{
    QString formatted;
    
    int testType = result["test_type"].toInt();
    int status = result["status"].toInt();
    QString message = result["message"].toString();
    QDateTime timestamp = result["timestamp"].toDateTime();
    
    QString testTypeName;
    switch (testType) {
        case ConnectivityTest: testTypeName = "Connectivity Test"; break;
        case LatencyTest: testTypeName = "Latency Test"; break;
        case BandwidthTest: testTypeName = "Bandwidth Test"; break;
        case DNSTest: testTypeName = "DNS Test"; break;
        case RouteTest: testTypeName = "Route Test"; break;
        case PortTest: testTypeName = "Port Test"; break;
        case FirewallTest: testTypeName = "Firewall Test"; break;
        default: testTypeName = "Unknown Test"; break;
    }
    
    QString statusName;
    switch (status) {
        case Success: statusName = "SUCCESS"; break;
        case Warning: statusName = "WARNING"; break;
        case Error: statusName = "ERROR"; break;
        case Timeout: statusName = "TIMEOUT"; break;
        default: statusName = "UNKNOWN"; break;
    }
    
    formatted += QString("[%1] %2: %3\n").arg(statusName, testTypeName, message);
    formatted += QString("  Time: %1\n").arg(timestamp.toString());
    
    return formatted;
}