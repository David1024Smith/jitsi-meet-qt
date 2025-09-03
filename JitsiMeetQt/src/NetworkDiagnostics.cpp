#include "NetworkDiagnostics.h"
#include <QUrl>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QNetworkProxy>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
NetworkDiagnostics::NetworkDiagnostics(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_timeoutTimer(nullptr)
    , m_tcpSocket(nullptr)
    , m_targetPort(443)
    , m_currentStepIndex(0)
{
    // 初始化网络管理器
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &NetworkDiagnostics::onHttpRequestFinished);
    
    // 初始化超时定时器
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(DIAGNOSTIC_TIMEOUT);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &NetworkDiagnostics::onDiagnosticTimeout);
    
    // 初始化TCP套接字
    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, &QTcpSocket::stateChanged,
            this, &NetworkDiagnostics::onTcpConnectionStateChanged);
    connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            [this](QAbstractSocket::SocketError error) {
                DiagnosticResult result;
                result.success = false;
                result.operation = "TCP连接测试";
                result.message = QString("TCP连接失败: %1").arg(m_tcpSocket->errorString());
                result.responseTime = m_stepStartTime.msecsTo(QDateTime::currentDateTime());
                result.details["error"] = static_cast<int>(error);
                result.details["errorString"] = m_tcpSocket->errorString();
                completeCurrentStep(result);
            });
    
    qDebug() << "NetworkDiagnostics: 网络诊断工具初始化完成";
}

/**
 * @brief 析构函数
 */
NetworkDiagnostics::~NetworkDiagnostics()
{
    if (m_timeoutTimer && m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }
    
    if (m_tcpSocket && m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        m_tcpSocket->abort();
    }
    
    qDebug() << "NetworkDiagnostics: 网络诊断工具已销毁";
}

/**
 * @brief 开始完整的网络诊断
 * @param serverUrl 要诊断的服务器URL
 */
void NetworkDiagnostics::startDiagnosis(const QString& serverUrl)
{
    qDebug() << "NetworkDiagnostics: 开始诊断服务器:" << serverUrl;
    
    m_targetUrl = serverUrl;
    m_results.clear();
    m_currentStepIndex = 0;
    
    // 解析URL
    if (!parseUrl(serverUrl, m_targetHostname, m_targetPort)) {
        DiagnosticResult result;
        result.success = false;
        result.operation = "URL解析";
        result.message = "无效的URL格式";
        result.responseTime = 0;
        result.details["url"] = serverUrl;
        
        emit diagnosticStepCompleted(result);
        emit diagnosisCompleted(false, "URL格式无效，无法进行诊断");
        return;
    }
    
    // 设置诊断步骤
    m_diagnosticSteps.clear();
    m_diagnosticSteps << "网络接口检查" << "代理设置检测" << "DNS解析测试" 
                     << "TCP连接测试" << "HTTP连接测试";
    
    // 开始第一个步骤
    executeNextStep();
}

/**
 * @brief 测试DNS解析
 * @param hostname 主机名
 */
void NetworkDiagnostics::testDnsResolution(const QString& hostname)
{
    qDebug() << "NetworkDiagnostics: 测试DNS解析:" << hostname;
    
    m_stepStartTime = QDateTime::currentDateTime();
    m_timeoutTimer->start();
    
    // 执行DNS查询
    int lookupId = QHostInfo::lookupHost(hostname, this, 
                                        SLOT(onDnsLookupFinished(QHostInfo)));
    
    if (lookupId == -1) {
        DiagnosticResult result;
        result.success = false;
        result.operation = "DNS解析";
        result.message = "DNS查询启动失败";
        result.responseTime = 0;
        result.details["hostname"] = hostname;
        completeCurrentStep(result);
    }
}

/**
 * @brief 测试TCP连接
 * @param hostname 主机名
 * @param port 端口号
 */
void NetworkDiagnostics::testTcpConnection(const QString& hostname, int port)
{
    qDebug() << "NetworkDiagnostics: 测试TCP连接:" << hostname << ":" << port;
    
    m_stepStartTime = QDateTime::currentDateTime();
    m_timeoutTimer->start();
    
    // 连接到主机
    m_tcpSocket->connectToHost(hostname, port);
}

/**
 * @brief 测试HTTP连接
 * @param url 测试URL
 */
void NetworkDiagnostics::testHttpConnection(const QString& url)
{
    qDebug() << "NetworkDiagnostics: 测试HTTP连接:" << url;
    
    m_stepStartTime = QDateTime::currentDateTime();
    m_timeoutTimer->start();
    
    QUrl requestUrl(url);
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "JitsiMeetQt-NetworkDiagnostics/1.0");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    // Qt 5.15.2 兼容性：使用RedirectPolicyAttribute（推荐）或FollowRedirectsAttribute（兼容）
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &NetworkDiagnostics::onNetworkError);
}

/**
 * @brief 检测代理设置
 */
void NetworkDiagnostics::detectProxySettings()
{
    qDebug() << "NetworkDiagnostics: 检测代理设置";
    
    m_stepStartTime = QDateTime::currentDateTime();
    
    DiagnosticResult result;
    result.operation = "代理设置检测";
    result.responseTime = m_stepStartTime.msecsTo(QDateTime::currentDateTime());
    
    QNetworkProxy systemProxy = QNetworkProxy::applicationProxy();
    QJsonObject proxyInfo;
    
    if (systemProxy.type() == QNetworkProxy::NoProxy) {
        result.success = true;
        result.message = "未检测到代理设置";
        proxyInfo["type"] = "NoProxy";
    } else {
        result.success = true;
        result.message = QString("检测到代理: %1:%2")
                        .arg(systemProxy.hostName())
                        .arg(systemProxy.port());
        
        proxyInfo["type"] = "Proxy";
        proxyInfo["hostname"] = systemProxy.hostName();
        proxyInfo["port"] = systemProxy.port();
        proxyInfo["user"] = systemProxy.user();
        
        switch (systemProxy.type()) {
        case QNetworkProxy::HttpProxy:
            proxyInfo["protocol"] = "HTTP";
            break;
        case QNetworkProxy::Socks5Proxy:
            proxyInfo["protocol"] = "SOCKS5";
            break;
        case QNetworkProxy::HttpCachingProxy:
            proxyInfo["protocol"] = "HTTP Caching";
            break;
        default:
            proxyInfo["protocol"] = "Unknown";
            break;
        }
    }
    
    result.details["proxy"] = proxyInfo;
    completeCurrentStep(result);
}

/**
 * @brief 获取网络接口信息
 */
void NetworkDiagnostics::getNetworkInterfaces()
{
    qDebug() << "NetworkDiagnostics: 获取网络接口信息";
    
    m_stepStartTime = QDateTime::currentDateTime();
    
    DiagnosticResult result;
    result.operation = "网络接口检查";
    result.responseTime = m_stepStartTime.msecsTo(QDateTime::currentDateTime());
    
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    QJsonArray interfaceArray;
    int activeInterfaces = 0;
    
    for (const QNetworkInterface& interface : interfaces) {
        if (interface.flags() & QNetworkInterface::IsUp &&
            interface.flags() & QNetworkInterface::IsRunning &&
            !(interface.flags() & QNetworkInterface::IsLoopBack)) {
            
            QJsonObject interfaceInfo;
            interfaceInfo["name"] = interface.name();
            interfaceInfo["humanReadableName"] = interface.humanReadableName();
            interfaceInfo["hardwareAddress"] = QString(interface.hardwareAddress());
            
            QJsonArray addressArray;
            for (const QNetworkAddressEntry& entry : interface.addressEntries()) {
                QJsonObject addressInfo;
                addressInfo["ip"] = entry.ip().toString();
                addressInfo["netmask"] = entry.netmask().toString();
                addressInfo["broadcast"] = entry.broadcast().toString();
                addressArray.append(addressInfo);
            }
            interfaceInfo["addresses"] = addressArray;
            
            interfaceArray.append(interfaceInfo);
            activeInterfaces++;
        }
    }
    
    if (activeInterfaces > 0) {
        result.success = true;
        result.message = QString("检测到 %1 个活动网络接口").arg(activeInterfaces);
    } else {
        result.success = false;
        result.message = "未检测到活动的网络接口";
    }
    
    result.details["interfaces"] = interfaceArray;
    result.details["activeCount"] = activeInterfaces;
    
    completeCurrentStep(result);
}

/**
 * @brief DNS解析完成槽函数
 * @param hostInfo 主机信息
 */
void NetworkDiagnostics::onDnsLookupFinished(const QHostInfo& hostInfo)
{
    m_timeoutTimer->stop();
    
    DiagnosticResult result;
    result.operation = "DNS解析";
    result.responseTime = m_stepStartTime.msecsTo(QDateTime::currentDateTime());
    
    if (hostInfo.error() == QHostInfo::NoError) {
        QList<QHostAddress> addresses = hostInfo.addresses();
        if (!addresses.isEmpty()) {
            result.success = true;
            result.message = QString("DNS解析成功，找到 %1 个IP地址").arg(addresses.size());
            
            QJsonArray addressArray;
            for (const QHostAddress& address : addresses) {
                addressArray.append(address.toString());
            }
            result.details["addresses"] = addressArray;
            result.details["hostname"] = hostInfo.hostName();
        } else {
            result.success = false;
            result.message = "DNS解析成功但未找到IP地址";
        }
    } else {
        result.success = false;
        result.message = QString("DNS解析失败: %1").arg(hostInfo.errorString());
        result.details["error"] = static_cast<int>(hostInfo.error());
        result.details["errorString"] = hostInfo.errorString();
    }
    
    result.details["hostname"] = m_targetHostname;
    completeCurrentStep(result);
}

/**
 * @brief TCP连接状态变化槽函数
 */
void NetworkDiagnostics::onTcpConnectionStateChanged()
{
    QAbstractSocket::SocketState state = m_tcpSocket->state();
    
    if (state == QAbstractSocket::ConnectedState) {
        m_timeoutTimer->stop();
        
        DiagnosticResult result;
        result.success = true;
        result.operation = "TCP连接测试";
        result.message = QString("TCP连接成功 (%1:%2)").arg(m_targetHostname).arg(m_targetPort);
        result.responseTime = m_stepStartTime.msecsTo(QDateTime::currentDateTime());
        result.details["hostname"] = m_targetHostname;
        result.details["port"] = m_targetPort;
        result.details["localAddress"] = m_tcpSocket->localAddress().toString();
        result.details["localPort"] = m_tcpSocket->localPort();
        
        m_tcpSocket->disconnectFromHost();
        completeCurrentStep(result);
    }
}

/**
 * @brief HTTP请求完成槽函数
 */
void NetworkDiagnostics::onHttpRequestFinished()
{
    m_timeoutTimer->stop();
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    DiagnosticResult result;
    result.operation = "HTTP连接测试";
    result.responseTime = m_stepStartTime.msecsTo(QDateTime::currentDateTime());
    
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    if (reply->error() == QNetworkReply::NoError) {
        result.success = true;
        result.message = QString("HTTP连接成功 (状态码: %1)").arg(statusCode);
        result.details["statusCode"] = statusCode;
        result.details["contentType"] = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        result.details["contentLength"] = reply->header(QNetworkRequest::ContentLengthHeader).toInt();
        result.details["server"] = QString(reply->rawHeader("Server"));
    } else {
        result.success = false;
        result.message = QString("HTTP连接失败: %1").arg(reply->errorString());
        result.details["error"] = static_cast<int>(reply->error());
        result.details["errorString"] = reply->errorString();
        result.details["statusCode"] = statusCode;
    }
    
    result.details["url"] = reply->url().toString();
    reply->deleteLater();
    
    completeCurrentStep(result);
}

/**
 * @brief 网络错误处理槽函数
 * @param error 网络错误
 */
void NetworkDiagnostics::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    // 错误处理在onHttpRequestFinished中进行
}

/**
 * @brief 诊断超时槽函数
 */
void NetworkDiagnostics::onDiagnosticTimeout()
{
    DiagnosticResult result;
    result.success = false;
    result.operation = m_diagnosticSteps.value(m_currentStepIndex, "未知操作");
    result.message = "操作超时";
    result.responseTime = DIAGNOSTIC_TIMEOUT;
    result.details["timeout"] = true;
    
    completeCurrentStep(result);
}

/**
 * @brief 执行下一个诊断步骤
 */
void NetworkDiagnostics::executeNextStep()
{
    if (m_currentStepIndex >= m_diagnosticSteps.size()) {
        // 所有步骤完成
        bool overallSuccess = true;
        for (const DiagnosticResult& result : m_results) {
            if (!result.success) {
                overallSuccess = false;
                break;
            }
        }
        
        emit diagnosisCompleted(overallSuccess, generateSummary());
        return;
    }
    
    QString currentStep = m_diagnosticSteps[m_currentStepIndex];
    int progress = (m_currentStepIndex * 100) / m_diagnosticSteps.size();
    
    emit diagnosisProgress(progress, currentStep);
    
    // 执行对应的诊断步骤
    if (currentStep == "网络接口检查") {
        getNetworkInterfaces();
    } else if (currentStep == "代理设置检测") {
        detectProxySettings();
    } else if (currentStep == "DNS解析测试") {
        testDnsResolution(m_targetHostname);
    } else if (currentStep == "TCP连接测试") {
        testTcpConnection(m_targetHostname, m_targetPort);
    } else if (currentStep == "HTTP连接测试") {
        testHttpConnection(m_targetUrl);
    }
}

/**
 * @brief 完成当前诊断步骤
 * @param result 诊断结果
 */
void NetworkDiagnostics::completeCurrentStep(const DiagnosticResult& result)
{
    m_results.append(result);
    emit diagnosticStepCompleted(result);
    
    m_currentStepIndex++;
    
    // 延迟执行下一步，避免界面阻塞
    QTimer::singleShot(100, this, &NetworkDiagnostics::executeNextStep);
}

/**
 * @brief 生成诊断摘要
 * @return 诊断摘要字符串
 */
QString NetworkDiagnostics::generateSummary() const
{
    QStringList summary;
    summary << QString("网络诊断完成 - 目标: %1").arg(m_targetUrl);
    summary << "";
    
    int successCount = 0;
    for (const DiagnosticResult& result : m_results) {
        QString status = result.success ? "✓" : "✗";
        summary << QString("%1 %2: %3").arg(status, result.operation, result.message);
        if (result.success) {
            successCount++;
        }
    }
    
    summary << "";
    summary << QString("总计: %1/%2 项测试通过").arg(successCount).arg(m_results.size());
    
    if (successCount == m_results.size()) {
        summary << "网络连接正常";
    } else {
        summary << "检测到网络连接问题，请检查网络设置";
    }
    
    return summary.join("\n");
}

/**
 * @brief 解析URL获取主机名和端口
 * @param url URL字符串
 * @param hostname 输出主机名
 * @param port 输出端口号
 * @return 解析是否成功
 */
bool NetworkDiagnostics::parseUrl(const QString& url, QString& hostname, int& port) const
{
    QUrl parsedUrl(url);
    
    if (!parsedUrl.isValid()) {
        return false;
    }
    
    hostname = parsedUrl.host();
    if (hostname.isEmpty()) {
        return false;
    }
    
    port = parsedUrl.port();
    if (port == -1) {
        // 使用默认端口
        if (parsedUrl.scheme().toLower() == "https") {
            port = 443;
        } else if (parsedUrl.scheme().toLower() == "http") {
            port = 80;
        } else {
            port = 443; // 默认使用HTTPS端口
        }
    }
    
    return true;
}