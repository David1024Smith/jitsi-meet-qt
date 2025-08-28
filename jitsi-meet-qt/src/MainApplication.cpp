#include "MainApplication.h"
#include "WindowManager.h"
#include "ConfigurationManager.h"
#include "ProtocolHandler.h"
#include "TranslationManager.h"
#include "PerformanceManager.h"
#include "MemoryLeakDetector.h"
#include "StartupOptimizer.h"
#include "OptimizedRecentManager.h"
#include "MemoryProfiler.h"
#include "JitsiConstants.h"

#include <QDir>
#include <QStandardPaths>
#include <QLocalSocket>
#include <QTimer>
#include <QDebug>
#include <QCommandLineParser>
#include <QCommandLineOption>

// 静态成员初始化
MainApplication* MainApplication::s_instance = nullptr;

MainApplication::MainApplication(int argc, char *argv[])
    : QApplication(argc, argv)
    , m_localServer(nullptr)
    , m_isFirstInstance(false)
    , m_windowManager(nullptr)
    , m_configManager(nullptr)
    , m_protocolHandler(nullptr)
    , m_translationManager(nullptr)
    , m_performanceManager(nullptr)
    , m_memoryLeakDetector(nullptr)
    , m_startupOptimizer(nullptr)
    , m_recentManager(nullptr)
    , m_memoryProfiler(nullptr)
    , m_showWelcome(true)
{
    // 设置应用程序信息
    setApplicationName(JitsiConstants::APP_NAME);
    setApplicationVersion(JitsiConstants::APP_VERSION);
    setOrganizationName(JitsiConstants::APP_ORGANIZATION);
    setOrganizationDomain(JitsiConstants::APP_DOMAIN);
    
    // 设置单例实例
    s_instance = this;
    
    // 解析命令行参数
    parseCommandLineArguments();
    
    // 设置单例模式
    m_isFirstInstance = setupSingleInstance();
    
    if (m_isFirstInstance) {
        // 第一个实例，初始化应用程序
        initializeManagers();
        registerProtocolHandler();
        
        // 连接退出信号
        connect(this, &QApplication::aboutToQuit, this, &MainApplication::onAboutToQuit);
        
        qDebug() << "MainApplication initialized as first instance";
    } else {
        // 不是第一个实例，发送消息给第一个实例然后退出
        QString message = m_startupUrl.isEmpty() ? "activate" : m_startupUrl;
        if (sendMessageToFirstInstance(message)) {
            qDebug() << "Message sent to first instance:" << message;
        }
        
        // 延迟退出，确保消息发送完成
        QTimer::singleShot(100, this, &QApplication::quit);
    }
}

MainApplication::~MainApplication()
{
    if (m_localServer) {
        m_localServer->close();
        delete m_localServer;
    }
    
    s_instance = nullptr;
}

MainApplication* MainApplication::instance()
{
    return s_instance;
}

void MainApplication::handleProtocolUrl(const QString& url)
{
    if (!m_windowManager) {
        qWarning() << "WindowManager not initialized";
        return;
    }
    
    qDebug() << "Handling protocol URL:" << url;
    
    if (m_protocolHandler && m_protocolHandler->isValidProtocolUrl(url)) {
        QString parsedUrl = m_protocolHandler->parseProtocolUrl(url);
        if (!parsedUrl.isEmpty()) {
            // 显示会议窗口并加载URL
            QVariantMap data;
            data["url"] = parsedUrl;
            m_windowManager->showWindow(WindowManager::ConferenceWindow, data);
        } else {
            // URL解析失败，显示欢迎窗口
            m_windowManager->showWindow(WindowManager::WelcomeWindow);
        }
    } else {
        // 无效的协议URL，显示欢迎窗口
        m_windowManager->showWindow(WindowManager::WelcomeWindow);
    }
}

WindowManager* MainApplication::windowManager() const
{
    return m_windowManager;
}

ConfigurationManager* MainApplication::configurationManager() const
{
    return m_configManager;
}

ProtocolHandler* MainApplication::protocolHandler() const
{
    return m_protocolHandler;
}

TranslationManager* MainApplication::translationManager() const
{
    return m_translationManager;
}

void MainApplication::onSecondInstance(const QString& arguments)
{
    qDebug() << "Second instance detected with arguments:" << arguments;
    
    if (arguments == "activate") {
        // 激活当前窗口
        if (m_windowManager) {
            auto currentWindow = m_windowManager->currentWindow();
            if (currentWindow) {
                currentWindow->raise();
                currentWindow->activateWindow();
            }
        }
    } else {
        // 处理协议URL
        handleProtocolUrl(arguments);
    }
}

void MainApplication::onAboutToQuit()
{
    qDebug() << "Application about to quit";
    
    // 保存配置
    if (m_configManager) {
        // 配置会在析构时自动保存
    }
    
    // 清理资源
    if (m_localServer) {
        m_localServer->close();
    }
}

void MainApplication::onNewConnection()
{
    QLocalSocket* socket = m_localServer->nextPendingConnection();
    if (socket) {
        connect(socket, &QLocalSocket::readyRead, this, &MainApplication::onSocketReadyRead);
        connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
    }
}

void MainApplication::onSocketReadyRead()
{
    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    QString message = QString::fromUtf8(data);
    
    qDebug() << "Received message from second instance:" << message;
    
    // 处理消息
    onSecondInstance(message);
    
    socket->disconnectFromServer();
}

bool MainApplication::setupSingleInstance()
{
    m_serverName = QString("%1_SingleInstance_%2")
                   .arg(applicationName())
                   .arg(QDir::home().absolutePath().replace('/', '_').replace('\\', '_'));
    
    // 尝试连接到现有服务器
    QLocalSocket socket;
    socket.connectToServer(m_serverName);
    
    if (socket.waitForConnected(1000)) {
        // 服务器已存在，说明已有实例在运行
        socket.disconnectFromServer();
        return false;
    }
    
    // 创建本地服务器
    m_localServer = new QLocalServer(this);
    
    // 清理可能存在的旧服务器
    QLocalServer::removeServer(m_serverName);
    
    if (!m_localServer->listen(m_serverName)) {
        qWarning() << "Failed to create local server:" << m_localServer->errorString();
        delete m_localServer;
        m_localServer = nullptr;
        return false;
    }
    
    connect(m_localServer, &QLocalServer::newConnection, this, &MainApplication::onNewConnection);
    
    return true;
}

void MainApplication::registerProtocolHandler()
{
    if (m_protocolHandler) {
        if (m_protocolHandler->registerProtocol()) {
            qDebug() << "Protocol handler registered successfully";
        } else {
            qWarning() << "Failed to register protocol handler";
        }
    }
}

void MainApplication::initializeManagers()
{
    qDebug() << "Initializing all application managers...";
    
    // 创建启动优化器并开始优化
    m_startupOptimizer = new StartupOptimizer(this);
    m_startupOptimizer->setOptimizationLevel(StartupOptimizer::Moderate);
    m_startupOptimizer->enableFastStartup();
    
    // 创建性能管理器
    m_performanceManager = new PerformanceManager(this);
    m_performanceManager->startStartupTimer();
    
    // 创建内存泄漏检测器（仅在调试模式下）
#ifdef QT_DEBUG
    m_memoryLeakDetector = new MemoryLeakDetector(this);
    m_memoryLeakDetector->startLeakDetection();
    
    // 创建内存分析器（仅在调试模式下）
    m_memoryProfiler = new MemoryProfiler(this);
    m_memoryProfiler->startProfiling();
#endif
    
    // 创建配置管理器（必须首先创建，其他组件依赖它）
    m_configManager = new ConfigurationManager(this);
    qDebug() << "ConfigurationManager initialized";
    
    // 创建优化的最近项目管理器
    m_recentManager = new OptimizedRecentManager(this);
    m_recentManager->loadRecentItemsAsync();
    
    // 创建翻译管理器
    m_translationManager = new TranslationManager(this);
    qDebug() << "TranslationManager initialized";
    
    // 创建协议处理器
    m_protocolHandler = new ProtocolHandler(this);
    qDebug() << "ProtocolHandler initialized";
    
    // 创建窗口管理器
    m_windowManager = new WindowManager(this);
    
    // 设置窗口管理器的依赖组件
    m_windowManager->setConfigurationManager(m_configManager);
    m_windowManager->setTranslationManager(m_translationManager);
    qDebug() << "WindowManager initialized and configured";
    
    // 建立核心信号连接
    setupCoreConnections();
    
    // 初始化翻译系统
    m_translationManager->initialize();
    
    // 连接性能管理器信号
    if (m_performanceManager) {
        connect(m_performanceManager, &PerformanceManager::memoryWarning,
                this, &MainApplication::onMemoryWarning);
    }
    
    // 连接内存泄漏检测器信号
#ifdef QT_DEBUG
    if (m_memoryLeakDetector) {
        connect(m_memoryLeakDetector, &MemoryLeakDetector::memoryLeakDetected,
                this, &MainApplication::onMemoryLeakDetected);
    }
#endif
    
    qDebug() << "All managers initialized successfully";
    
    // 标记启动完成
    if (m_performanceManager) {
        m_performanceManager->markStartupComplete();
    }
    
    // 显示初始窗口
    showInitialWindow();
}

void MainApplication::parseCommandLineArguments()
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt version of Jitsi Meet desktop application");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加URL参数选项
    QCommandLineOption urlOption(QStringList() << "u" << "url",
                                "Open specific meeting URL",
                                "url");
    parser.addOption(urlOption);
    
    // 添加隐藏欢迎窗口选项
    QCommandLineOption noWelcomeOption(QStringList() << "no-welcome",
                                      "Don't show welcome window on startup");
    parser.addOption(noWelcomeOption);
    
    parser.process(*this);
    
    // 解析URL参数
    if (parser.isSet(urlOption)) {
        m_startupUrl = parser.value(urlOption);
    }
    
    // 检查位置参数（协议URL）
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        QString firstArg = args.first();
        if (firstArg.startsWith(JitsiConstants::PROTOCOL_PREFIX)) {
            m_startupUrl = firstArg;
        }
    }
    
    // 解析欢迎窗口选项
    if (parser.isSet(noWelcomeOption)) {
        m_showWelcome = false;
    }
    
    qDebug() << "Parsed command line - URL:" << m_startupUrl << "ShowWelcome:" << m_showWelcome;
}

bool MainApplication::sendMessageToFirstInstance(const QString& message)
{
    QLocalSocket socket;
    socket.connectToServer(m_serverName);
    
    if (!socket.waitForConnected(1000)) {
        qWarning() << "Failed to connect to first instance";
        return false;
    }
    
    QByteArray data = message.toUtf8();
    socket.write(data);
    socket.flush();
    
    if (!socket.waitForBytesWritten(1000)) {
        qWarning() << "Failed to send message to first instance";
        return false;
    }
    
    socket.disconnectFromServer();
    return true;
}

void MainApplication::onWindowChanged(int type)
{
    qDebug() << "Window changed to type:" << type;
    
    // 可以在这里添加窗口切换时的额外逻辑
    // 例如更新系统托盘图标、保存状态等
}

void MainApplication::onWindowStateChanged(int type, int state)
{
    qDebug() << "Window state changed - Type:" << type << "State:" << state;
    
    // 可以在这里添加窗口状态改变时的额外逻辑
    // 例如最小化到系统托盘等
}

void MainApplication::onConfigurationChanged()
{
    qDebug() << "Configuration changed, updating components";
    
    // 更新所有组件的配置
    if (m_windowManager && m_configManager) {
        // 通知窗口管理器配置已更改
        // 窗口管理器会自动更新其管理的窗口
    }
    
    if (m_translationManager && m_configManager) {
        // 如果语言设置改变，翻译管理器会自动处理
    }
}

void MainApplication::onMemoryWarning(qint64 memoryUsage)
{
    qWarning() << "Memory warning: Current usage" << memoryUsage / (1024*1024) << "MB";
    
    // 执行内存清理
    if (m_performanceManager) {
        m_performanceManager->performMemoryCleanup();
    }
    
    // 通知窗口管理器进行内存优化
    if (m_windowManager) {
        // 窗口管理器可以清理不必要的缓存
    }
}

void MainApplication::onMemoryLeakDetected(const QList<MemoryLeakDetector::AllocationInfo>& leaks)
{
    qWarning() << "Memory leaks detected:" << leaks.size() << "potential leaks";
    
#ifdef QT_DEBUG
    // 在调试模式下生成详细报告
    if (m_memoryLeakDetector) {
        m_memoryLeakDetector->generateLeakReport();
    }
#endif
}

PerformanceManager* MainApplication::performanceManager() const
{
    return m_performanceManager;
}

OptimizedRecentManager* MainApplication::recentManager() const
{
    return m_recentManager;
}

void MainApplication::setupCoreConnections()
{
    qDebug() << "Setting up core signal connections...";
    
    // 协议处理器连接
    connect(m_protocolHandler, &ProtocolHandler::protocolUrlReceived,
            this, &MainApplication::handleProtocolUrl);
    
    // 配置管理器和翻译管理器连接
    connect(m_configManager, &ConfigurationManager::languageChanged,
            m_translationManager, &TranslationManager::onConfigLanguageChanged);
    
    // 窗口管理器信号连接
    connect(m_windowManager, &WindowManager::windowChanged,
            this, &MainApplication::onWindowChanged);
    connect(m_windowManager, &WindowManager::windowStateChanged,
            this, &MainApplication::onWindowStateChanged);
    connect(m_windowManager, &WindowManager::dataTransferred,
            this, &MainApplication::onDataTransferred);
    connect(m_windowManager, &WindowManager::windowCreated,
            this, &MainApplication::onWindowCreated);
    connect(m_windowManager, &WindowManager::windowDestroyed,
            this, &MainApplication::onWindowDestroyed);
    
    // 配置变更信号到窗口管理器
    connect(m_configManager, &ConfigurationManager::configurationChanged,
            this, &MainApplication::onConfigurationChanged);
    
    // 最近项目管理器连接
    if (m_recentManager) {
        connect(m_recentManager, &OptimizedRecentManager::recentItemsChanged,
                this, &MainApplication::onRecentItemsChanged);
    }
    
    qDebug() << "Core signal connections established";
}

void MainApplication::showInitialWindow()
{
    qDebug() << "Showing initial window...";
    
    if (!m_startupUrl.isEmpty()) {
        qDebug() << "Starting with protocol URL:" << m_startupUrl;
        handleProtocolUrl(m_startupUrl);
    } else if (m_showWelcome) {
        qDebug() << "Showing welcome window";
        m_windowManager->showWindow(WindowManager::WelcomeWindow);
    } else {
        qDebug() << "No initial window to show";
    }
}

void MainApplication::onDataTransferred(int fromType, int toType, const QVariantMap& data)
{
    qDebug() << "Data transferred from window type" << fromType << "to type" << toType 
             << "with" << data.size() << "data items";
    
    // 可以在这里添加数据传递的额外处理逻辑
    // 例如记录用户行为、更新统计信息等
}

void MainApplication::onWindowCreated(int type)
{
    qDebug() << "Window created - type:" << type;
    
    // 窗口创建后的额外处理
    if (m_performanceManager) {
        m_performanceManager->recordWindowCreation(type);
    }
}

void MainApplication::onWindowDestroyed(int type)
{
    qDebug() << "Window destroyed - type:" << type;
    
    // 窗口销毁后的清理工作
    if (m_performanceManager) {
        m_performanceManager->recordWindowDestruction(type);
    }
}

void MainApplication::onRecentItemsChanged()
{
    qDebug() << "Recent items changed, notifying windows";
    
    // 通知相关窗口更新最近项目列表
    if (m_windowManager && m_windowManager->hasWindow(WindowManager::WelcomeWindow)) {
        QVariantMap data;
        data["action"] = "refreshRecentItems";
        m_windowManager->sendDataToWindow(WindowManager::WelcomeWindow, data);
    }
}