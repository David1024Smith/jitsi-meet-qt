#include "NetworkMonitor.h"
#include <QRegularExpression>
#include <QDebug>
#include <QCoreApplication>
#include <QNetworkInformation>
#include <QHostInfo>
#include <QProcess>

NetworkMonitor::NetworkMonitor(QObject *parent)
    : BaseMonitor("NetworkMonitor", parent)
    , m_monitoringMode(BasicMode)
    , m_monitoredInterface("auto")
    , m_networkManager(nullptr)
    , m_periodicTimer(nullptr)
    , m_latencyTimer(nullptr)
    , m_currentQuality(Fair)
    , m_currentScore(50)
    , m_isConnected(false)
    , m_isInternetConnected(false)
    , m_currentSpeedTest(nullptr)
    , m_latencySocket(nullptr)
{
    // 初始化网络管理器
    m_networkManager = new QNetworkAccessManager(this);
    // Note: networkAccessibleChanged signal is deprecated in newer Qt versions
    // connect(m_networkManager, &QNetworkAccessManager::networkAccessibleChanged,
    //         this, &NetworkMonitor::handleNetworkAccessibilityChanged);
    
    // 初始化定时器
    m_periodicTimer = new QTimer(this);
    m_periodicTimer->setInterval(30000); // 30秒检查一次
    connect(m_periodicTimer, &QTimer::timeout,
            this, &NetworkMonitor::performPeriodicCheck);
    
    m_latencyTimer = new QTimer(this);
    m_latencyTimer->setInterval(10000); // 10秒测试一次延迟
    connect(m_latencyTimer, &QTimer::timeout,
            this, &NetworkMonitor::handleLatencyTestFinished);
    
    // 设置默认延迟测试主机
    m_latencyTestHosts << "8.8.8.8" << "1.1.1.1" << "208.67.222.222";
    
    // 初始化历史数据容器
    m_bandwidthHistory.reserve(1440); // 24小时数据
    m_latencyHistory.reserve(1440);
    m_jitterHistory.reserve(1440);
    m_packetLossHistory.reserve(1440);
}

NetworkMonitor::~NetworkMonitor()
{
    if (isTracking()) {
        stopTracking();
    }
    
    if (m_currentSpeedTest) {
        m_currentSpeedTest->abort();
        m_currentSpeedTest->deleteLater();
    }
    
    if (m_latencySocket) {
        m_latencySocket->deleteLater();
    }
}

QString NetworkMonitor::version() const
{
    return "1.0.0";
}

QString NetworkMonitor::description() const
{
    return "Network performance monitor for tracking bandwidth, latency, and connection quality";
}

void NetworkMonitor::setMonitoringMode(MonitoringMode mode)
{
    QMutexLocker locker(&m_dataMutex);
    if (m_monitoringMode != mode) {
        m_monitoringMode = mode;
        
        // 根据模式调整监控频率
        switch (mode) {
        case BasicMode:
            m_periodicTimer->setInterval(60000); // 1分钟
            break;
        case BandwidthMode:
        case QualityMode:
            m_periodicTimer->setInterval(30000); // 30秒
            break;
        case LatencyMode:
            m_periodicTimer->setInterval(10000); // 10秒
            break;
        case ComprehensiveMode:
            m_periodicTimer->setInterval(5000);  // 5秒
            break;
        }
        
        qDebug() << "NetworkMonitor: Monitoring mode changed to" << mode;
    }
}

NetworkMonitor::MonitoringMode NetworkMonitor::monitoringMode() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_monitoringMode;
}

QList<QNetworkInterface> NetworkMonitor::getAllInterfaces() const
{
    return QNetworkInterface::allInterfaces();
}

QList<QNetworkInterface> NetworkMonitor::getActiveInterfaces() const
{
    QList<QNetworkInterface> activeInterfaces;
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface& interface : allInterfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            interface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            activeInterfaces.append(interface);
        }
    }
    
    return activeInterfaces;
}

QNetworkInterface NetworkMonitor::getPrimaryInterface() const
{
    QList<QNetworkInterface> activeInterfaces = getActiveInterfaces();
    
    // 优先选择以太网接口
    for (const QNetworkInterface& interface : activeInterfaces) {
        if (getInterfaceType(interface) == Ethernet) {
            return interface;
        }
    }
    
    // 其次选择WiFi接口
    for (const QNetworkInterface& interface : activeInterfaces) {
        if (getInterfaceType(interface) == WiFi) {
            return interface;
        }
    }
    
    // 返回第一个活动接口
    if (!activeInterfaces.isEmpty()) {
        return activeInterfaces.first();
    }
    
    return QNetworkInterface();
}

void NetworkMonitor::setMonitoredInterface(const QString& interfaceName)
{
    QMutexLocker locker(&m_dataMutex);
    m_monitoredInterface = interfaceName;
}

QString NetworkMonitor::monitoredInterface() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_monitoredInterface;
}

NetworkMonitor::InterfaceType NetworkMonitor::getInterfaceType(const QNetworkInterface& interface) const
{
    QString name = interface.name().toLower();
    
    if (name.contains("eth") || name.contains("en") || name.contains("lan")) {
        return Ethernet;
    } else if (name.contains("wlan") || name.contains("wifi") || name.contains("wl")) {
        return WiFi;
    } else if (name.contains("ppp") || name.contains("cell") || name.contains("wwan")) {
        return Cellular;
    } else if (name.contains("lo") || name.contains("loopback")) {
        return Loopback;
    } else if (name.contains("vpn") || name.contains("tun") || name.contains("tap")) {
        return VPN;
    }
    
    return Unknown;
}

bool NetworkMonitor::isNetworkConnected() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_isConnected;
}

bool NetworkMonitor::isInternetConnected() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_isInternetConnected;
}

NetworkMonitor::QualityLevel NetworkMonitor::getNetworkQuality() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_currentQuality;
}

int NetworkMonitor::getNetworkScore() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_currentScore;
}

void NetworkMonitor::setLatencyTestHosts(const QStringList& hosts)
{
    QMutexLocker locker(&m_dataMutex);
    m_latencyTestHosts = hosts;
}

QStringList NetworkMonitor::latencyTestHosts() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_latencyTestHosts;
}

bool NetworkMonitor::initializeMonitor()
{
    qDebug() << "NetworkMonitor: Initializing network monitor...";
    
    if (!initializeInterfaceMonitoring()) {
        addError("Failed to initialize interface monitoring");
        return false;
    }
    
    // 检查初始网络状态
    m_isConnected = !getActiveInterfaces().isEmpty();
    
    // 启动定期检查
    m_periodicTimer->start();
    
    if (m_monitoringMode >= LatencyMode) {
        m_latencyTimer->start();
    }
    
    qDebug() << "NetworkMonitor: Initialized successfully";
    qDebug() << "  Active Interfaces:" << getActiveInterfaces().size();
    qDebug() << "  Network Connected:" << m_isConnected;
    
    return true;
}

ResourceUsage NetworkMonitor::collectResourceUsage()
{
    ResourceUsage usage;
    usage.timestamp = QDateTime::currentDateTime();
    // ResourceUsage doesn't have resourceType field - it's determined by context
    
    // 更新接口统计信息
    updateInterfaceStatistics();
    
    // 获取主要接口的带宽使用情况
    QString interfaceName = m_monitoredInterface;
    if (interfaceName == "auto") {
        QNetworkInterface primaryInterface = getPrimaryInterface();
        interfaceName = primaryInterface.name();
    }
    
    QPair<qint64, qint64> speeds = calculateNetworkSpeed(interfaceName);
    usage.network.receiveSpeed = speeds.first;
    usage.network.sendSpeed = speeds.second;
    
    // 根据监控模式收集不同级别的数据
    if (m_monitoringMode >= LatencyMode) {
        if (!m_latencyTestHosts.isEmpty()) {
            double avgLatency = performLatencyTest(m_latencyTestHosts.first());
            usage.network.latency = avgLatency;
        }
    }
    
    if (m_monitoringMode >= QualityMode) {
        QualityLevel quality = calculateNetworkQuality();
        // Store quality and score in network usage structure
        // Note: These could be stored in a custom metrics map if needed
        
        QMutexLocker locker(&m_dataMutex);
        m_currentQuality = quality;
    }
    
    if (m_monitoringMode == ComprehensiveMode) {
        usage.network.connectionCount = getConnectionCount();
        // Store connection status in network usage structure
    }
    
    // 更新历史数据
    QMutexLocker locker(&m_dataMutex);
    m_lastUpdateTime = usage.timestamp;
    
    // 添加到历史记录
    m_bandwidthHistory.append(qMakePair(usage.network.receiveSpeed, usage.network.sendSpeed));
    if (m_bandwidthHistory.size() > 1440) {
        m_bandwidthHistory.removeFirst();
    }
    
    return usage;
}

QList<IResourceTracker::ResourceType> NetworkMonitor::supportedResourceTypes() const
{
    return {IResourceTracker::Network};
}

void NetworkMonitor::handleNetworkAccessibilityChanged(int accessible)
{
    bool wasConnected = m_isConnected;
    m_isConnected = (accessible == 1); // 1 represents Accessible in older Qt versions
    
    if (wasConnected != m_isConnected) {
        qDebug() << "NetworkMonitor: Network accessibility changed to" << accessible;
        
        if (isTracking()) {
            // 触发数据收集
            QTimer::singleShot(1000, this, [this]() {
                collectResourceUsage();
            });
        }
    }
}

void NetworkMonitor::handleLatencyTestFinished()
{
    if (m_monitoringMode < LatencyMode) {
        return;
    }
    
    // 对所有测试主机进行延迟测试
    double totalLatency = 0.0;
    int validTests = 0;
    
    for (const QString& host : m_latencyTestHosts) {
        double latency = performLatencyTest(host);
        if (latency > 0) {
            totalLatency += latency;
            validTests++;
        }
    }
    
    if (validTests > 0) {
        double avgLatency = totalLatency / validTests;
        
        QMutexLocker locker(&m_dataMutex);
        m_latencyHistory.append(avgLatency);
        if (m_latencyHistory.size() > 1440) {
            m_latencyHistory.removeFirst();
        }
    }
}

void NetworkMonitor::performPeriodicCheck()
{
    // 检查网络连接状态
    bool wasConnected = m_isConnected;
    m_isConnected = !getActiveInterfaces().isEmpty();
    
    if (wasConnected != m_isConnected) {
        qDebug() << "NetworkMonitor: Network connection status changed to" << m_isConnected;
    }
    
    // 检查互联网连接
    if (m_isConnected && m_monitoringMode >= QualityMode) {
        // 简单的互联网连接测试
        QNetworkRequest request(QUrl("http://www.google.com"));
        request.setRawHeader("User-Agent", "NetworkMonitor/1.0");
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        
        QNetworkReply* reply = m_networkManager->head(request);
        connect(reply, &QNetworkReply::finished, [this, reply]() {
            bool wasInternetConnected = m_isInternetConnected;
            m_isInternetConnected = (reply->error() == QNetworkReply::NoError);
            
            if (wasInternetConnected != m_isInternetConnected) {
                qDebug() << "NetworkMonitor: Internet connection status changed to" << m_isInternetConnected;
            }
            
            reply->deleteLater();
        });
    }
}

bool NetworkMonitor::initializeInterfaceMonitoring()
{
    // 初始化接口统计信息
    QList<QNetworkInterface> interfaces = getActiveInterfaces();
    
    for (const QNetworkInterface& interface : interfaces) {
        InterfaceStatistics stats;
        stats.timestamp = QDateTime::currentDateTime();
        
        // 读取初始统计信息
        QVariantMap interfaceStats = readInterfaceStatistics(interface.name());
        stats.bytesReceived = interfaceStats.value("bytesReceived", 0).toLongLong();
        stats.bytesSent = interfaceStats.value("bytesSent", 0).toLongLong();
        stats.packetsReceived = interfaceStats.value("packetsReceived", 0).toLongLong();
        stats.packetsSent = interfaceStats.value("packetsSent", 0).toLongLong();
        
        m_interfaceStats[interface.name()] = stats;
        m_lastInterfaceStats[interface.name()] = stats;
    }
    
    return true;
}

void NetworkMonitor::updateInterfaceStatistics()
{
    QList<QNetworkInterface> interfaces = getActiveInterfaces();
    
    for (const QNetworkInterface& interface : interfaces) {
        QVariantMap interfaceStats = readInterfaceStatistics(interface.name());
        
        InterfaceStatistics stats;
        stats.timestamp = QDateTime::currentDateTime();
        stats.bytesReceived = interfaceStats.value("bytesReceived", 0).toLongLong();
        stats.bytesSent = interfaceStats.value("bytesSent", 0).toLongLong();
        stats.packetsReceived = interfaceStats.value("packetsReceived", 0).toLongLong();
        stats.packetsSent = interfaceStats.value("packetsSent", 0).toLongLong();
        
        m_lastInterfaceStats[interface.name()] = m_interfaceStats.value(interface.name());
        m_interfaceStats[interface.name()] = stats;
    }
}

QPair<qint64, qint64> NetworkMonitor::calculateNetworkSpeed(const QString& interfaceName)
{
    if (!m_interfaceStats.contains(interfaceName) || !m_lastInterfaceStats.contains(interfaceName)) {
        return qMakePair(0LL, 0LL);
    }
    
    const InterfaceStatistics& current = m_interfaceStats[interfaceName];
    const InterfaceStatistics& last = m_lastInterfaceStats[interfaceName];
    
    qint64 timeDiff = last.timestamp.msecsTo(current.timestamp);
    if (timeDiff <= 0) {
        return qMakePair(0LL, 0LL);
    }
    
    qint64 downloadDiff = current.bytesReceived - last.bytesReceived;
    qint64 uploadDiff = current.bytesSent - last.bytesSent;
    
    // 计算每秒速度
    qint64 downloadSpeed = (downloadDiff * 1000) / timeDiff;
    qint64 uploadSpeed = (uploadDiff * 1000) / timeDiff;
    
    return qMakePair(qMax(0LL, downloadSpeed), qMax(0LL, uploadSpeed));
}

double NetworkMonitor::performLatencyTest(const QString& host)
{
    // 简化的延迟测试实现
    // 实际应用中可能需要使用ICMP ping或其他方法
    
    QProcess pingProcess;
    QStringList arguments;
    
#ifdef Q_OS_WIN
    arguments << "-n" << "1" << host;
    pingProcess.start("ping", arguments);
#else
    arguments << "-c" << "1" << host;
    pingProcess.start("ping", arguments);
#endif
    
    if (pingProcess.waitForFinished(5000)) {
        QString output = pingProcess.readAllStandardOutput();
        
        // 解析ping输出获取延迟时间
        QRegularExpression timeRegex("time[<=]([0-9.]+)");
        QRegularExpressionMatch match = timeRegex.match(output);
        if (match.hasMatch()) {
            return match.captured(1).toDouble();
        }
    }
    
    return 0.0; // 测试失败
}

NetworkMonitor::QualityLevel NetworkMonitor::calculateNetworkQuality()
{
    // 基于延迟、带宽和丢包率计算网络质量
    int score = 100;
    
    // 延迟评分
    if (!m_latencyHistory.isEmpty()) {
        double avgLatency = 0.0;
        for (double latency : m_latencyHistory) {
            avgLatency += latency;
        }
        avgLatency /= m_latencyHistory.size();
        
        if (avgLatency > 200) {
            score -= 40;
        } else if (avgLatency > 100) {
            score -= 30;
        } else if (avgLatency > 50) {
            score -= 20;
        } else if (avgLatency > 20) {
            score -= 10;
        }
    }
    
    // 连接状态评分
    if (!m_isConnected) {
        score -= 50;
    } else if (!m_isInternetConnected) {
        score -= 30;
    }
    
    // 更新评分
    QMutexLocker locker(&m_dataMutex);
    m_currentScore = qMax(0, qMin(100, score));
    
    // 根据评分确定质量等级
    if (m_currentScore >= 90) {
        return Excellent;
    } else if (m_currentScore >= 70) {
        return Good;
    } else if (m_currentScore >= 50) {
        return Fair;
    } else if (m_currentScore >= 30) {
        return Poor;
    } else {
        return Critical;
    }
}

QVariantMap NetworkMonitor::readInterfaceStatistics(const QString& interfaceName)
{
    QVariantMap result;
    
#ifdef Q_OS_LINUX
    QString statsPath = QString("/sys/class/net/%1/statistics").arg(interfaceName);
    
    QFile rxBytesFile(statsPath + "/rx_bytes");
    if (rxBytesFile.open(QIODevice::ReadOnly)) {
        result["bytesReceived"] = rxBytesFile.readAll().trimmed().toLongLong();
    }
    
    QFile txBytesFile(statsPath + "/tx_bytes");
    if (txBytesFile.open(QIODevice::ReadOnly)) {
        result["bytesSent"] = txBytesFile.readAll().trimmed().toLongLong();
    }
    
    QFile rxPacketsFile(statsPath + "/rx_packets");
    if (rxPacketsFile.open(QIODevice::ReadOnly)) {
        result["packetsReceived"] = rxPacketsFile.readAll().trimmed().toLongLong();
    }
    
    QFile txPacketsFile(statsPath + "/tx_packets");
    if (txPacketsFile.open(QIODevice::ReadOnly)) {
        result["packetsSent"] = txPacketsFile.readAll().trimmed().toLongLong();
    }
#else
    // 其他平台的实现
    Q_UNUSED(interfaceName)
    result["bytesReceived"] = 0;
    result["bytesSent"] = 0;
    result["packetsReceived"] = 0;
    result["packetsSent"] = 0;
#endif
    
    return result;
}

int NetworkMonitor::getConnectionCount() const
{
    // 简化实现，返回活动接口数量
    return getActiveInterfaces().size();
}