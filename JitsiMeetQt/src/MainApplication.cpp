#include "MainApplication.h"
#include "ConferenceWindow.h"
#include "WelcomeWindow.h"
#include "SettingsDialog.h"
#include "ConfigurationManager.h"
#include "ProtocolHandler.h"

#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QUrl>
#include <QRegularExpression>
#include <QDebug>
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

// 静态成员初始化
MainApplication* MainApplication::s_instance = nullptr;

MainApplication::MainApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_showAction(nullptr)
    , m_settingsAction(nullptr)
    , m_aboutAction(nullptr)
    , m_quitAction(nullptr)
    , m_translator(nullptr)
    , m_currentLanguage("zh_CN")
    , m_initialized(false)
    , m_conferenceWindowReady(false)
{
    QElapsedTimer constructorTimer;
    constructorTimer.start();
    
    // 设置单例实例
    s_instance = this;
    
    // 设置应用程序信息
    setApplicationName("Jitsi Meet Qt");
    setApplicationVersion("1.0.0");
    setOrganizationName("Jitsi Meet Qt");
    setOrganizationDomain("jitsi.org");
    
    // 设置应用程序图标
    setWindowIcon(QIcon(":/images/logo.svg"));
    
    qDebug() << QString("MainApplication 构造函数完成，耗时: %1ms").arg(constructorTimer.elapsed());
}

MainApplication::~MainApplication()
{
    qDebug() << "MainApplication 析构函数";
    s_instance = nullptr;
}

MainApplication* MainApplication::instance()
{
    return s_instance;
}

bool MainApplication::initialize()
{
    if (m_initialized) {
        qDebug() << "应用程序已经初始化";
        return true;
    }
    
    QElapsedTimer initTimer, stageTimer;
    initTimer.start();
    stageTimer.start();
    
    qDebug() << "开始初始化应用程序...";
    
    try {
        // 初始化配置管理器
        stageTimer.restart();
        m_configManager = ConfigurationManager::instance();// 配置管理器已通过单例模式自动初始化
        qDebug() << QString("配置管理器初始化耗时: %1ms").arg(stageTimer.elapsed());
        
        // 初始化协议处理器
        stageTimer.restart();
        m_protocolHandler = std::make_unique<ProtocolHandler>(this);
        if (!m_protocolHandler->registerProtocol()) {
            qWarning() << "协议注册失败，但应用程序将继续运行";
        }
        qDebug() << QString("协议处理器初始化耗时: %1ms").arg(stageTimer.elapsed());
        
        // 连接协议处理器信号
        connect(m_protocolHandler.get(), &ProtocolHandler::protocolUrlReceived,
                this, &MainApplication::handleProtocolUrl);
        
        // 初始化翻译
        stageTimer.restart();
        initializeTranslations();
        qDebug() << QString("翻译初始化耗时: %1ms").arg(stageTimer.elapsed());
        
        // 启动异步资源预加载
        stageTimer.restart();
        preloadResourcesAsync();
        qDebug() << QString("异步资源预加载启动耗时: %1ms").arg(stageTimer.elapsed());
        
        // 为了确保UI立即可用，先同步加载基础样式
         stageTimer.restart();
         loadStyleSheet("default");
         qDebug() << QString("基础样式表同步加载耗时: %1ms").arg(stageTimer.elapsed());
        
        // 初始化系统托盘 - 暂时跳过以避免初始化问题
        // if (QSystemTrayIcon::isSystemTrayAvailable()) {
        //     initializeSystemTray();
        // } else {
        //     qWarning() << "系统托盘不可用";
        // }
        
        // 创建窗口实例
        stageTimer.restart();
        m_welcomeWindow = std::make_unique<WelcomeWindow>();
        qDebug() << QString("WelcomeWindow创建耗时: %1ms").arg(stageTimer.elapsed());
        connect(m_welcomeWindow.get(), &WelcomeWindow::joinMeetingRequested,
                this, [this](const QString& url, const QString& displayName, const QString& password) {
                    Q_UNUSED(password)
                    qDebug() << "收到加入会议请求 - URL:" << url << "显示名:" << displayName;
                    
                    // 直接使用用户输入的URL和当前选择的服务器
                    QString serverUrl = m_welcomeWindow->getServerUrl();
                    if (serverUrl.isEmpty()) {
                        serverUrl = m_configManager->getDefaultServerUrl();
                    }
                    
                    // 如果URL包含完整的会议链接，解析出房间名
                    QString roomName = url;
                    if (url.contains("://")) {
                        QUrl meetingUrl(url);
                        if (meetingUrl.isValid()) {
                            roomName = meetingUrl.path();
                            if (roomName.startsWith("/")) {
                                roomName = roomName.mid(1);
                            }
                            if (!meetingUrl.host().isEmpty()) {
                                serverUrl = QString("%1://%2").arg(meetingUrl.scheme()).arg(meetingUrl.host());
                            }
                        }
                    }
                    
                    qDebug() << "解析后 - 房间名:" << roomName << "服务器:" << serverUrl;
                    showConferenceWindow(roomName, serverUrl);
                });
        connect(m_welcomeWindow.get(), &WelcomeWindow::createMeetingRequested,
                this, [this](const QString& roomName, const QString& serverUrl, const QString& displayName, const QString& password) {
                    Q_UNUSED(password)
                    qDebug() << "收到创建会议请求 - 房间名:" << roomName << "服务器:" << serverUrl << "显示名:" << displayName;
                    showConferenceWindow(roomName, serverUrl);
                });
        connect(m_welcomeWindow.get(), &WelcomeWindow::settingsRequested,
                this, &MainApplication::showSettingsDialog);
        
        // 启动ConferenceWindow的异步并行初始化
        qDebug() << "开始ConferenceWindow异步初始化";
        initializeConferenceWindowAsync();
        
        stageTimer.restart();
        m_settingsDialog = std::make_unique<SettingsDialog>();
        qDebug() << QString("SettingsDialog创建耗时: %1ms").arg(stageTimer.elapsed());
        
        m_initialized = true;
        qDebug() << QString("应用程序初始化完成，总耗时: %1ms").arg(initTimer.elapsed());
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "应用程序初始化异常:" << e.what();
        return false;
    }
}

void MainApplication::showWelcomeWindow()
{
    qDebug() << "显示欢迎窗口";
    
    if (!m_welcomeWindow) {
        qCritical() << "欢迎窗口未初始化";
        return;
    }
    
    // 隐藏会议窗口
    if (m_conferenceWindow && m_conferenceWindow->isVisible()) {
        m_conferenceWindow->hide();
    }
    
    m_welcomeWindow->show();
    m_welcomeWindow->raise();
    m_welcomeWindow->activateWindow();
}

void MainApplication::showConferenceWindow(const QString &roomName, const QString &serverUrl)
{
    qDebug() << "显示会议窗口 - 房间:" << roomName << "服务器:" << serverUrl;
    
    // 检查ConferenceWindow是否已准备就绪
    if (!isConferenceWindowReady()) {
        qDebug() << "ConferenceWindow未初始化，开始初始化...";
        initializeConferenceWindowAsync();
        
        // 连接一次性信号，等待初始化完成后再显示窗口
        QMetaObject::Connection* connection = new QMetaObject::Connection();
        *connection = connect(this, &MainApplication::conferenceWindowReady, this, 
            [this, roomName, serverUrl, connection]() {
                disconnect(*connection);
                delete connection;
                showConferenceWindow(roomName, serverUrl);
            });
        return;
    }
    
    // 隐藏欢迎窗口
    if (m_welcomeWindow && m_welcomeWindow->isVisible()) {
        m_welcomeWindow->hide();
    }
    
    // 设置会议参数并显示
    QString finalServerUrl = serverUrl.isEmpty() ? 
        m_configManager->getDefaultServerUrl() : serverUrl;
    
    m_conferenceWindow->joinConference(roomName, finalServerUrl);
    m_conferenceWindow->show();
    m_conferenceWindow->raise();
    m_conferenceWindow->activateWindow();
}

void MainApplication::showSettingsDialog()
{
    qDebug() << "显示设置对话框";
    
    if (!m_settingsDialog) {
        qCritical() << "设置对话框未初始化";
        return;
    }
    
    m_settingsDialog->show();
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
}

void MainApplication::handleProtocolUrl(const ProtocolHandler::MeetingInfo &meetingInfo)
{
    qDebug() << "处理协议URL:" << meetingInfo.fullUrl;
    
    if (meetingInfo.isValid) {
        showConferenceWindow(meetingInfo.roomName, meetingInfo.serverUrl);
    } else {
        qWarning() << "无效的协议URL:" << meetingInfo.fullUrl;
        QMessageBox::warning(nullptr, tr("错误"), 
                           tr("无效的会议链接: %1").arg(meetingInfo.fullUrl));
    }
}

ConfigurationManager* MainApplication::configurationManager() const
{
    return m_configManager;
}

void MainApplication::setLanguage(const QString &language)
{
    qDebug() << "设置语言:" << language;
    
    if (m_currentLanguage == language) {
        return;
    }
    
    m_currentLanguage = language;
    
    // 移除旧的翻译器
    if (m_translator) {
        removeTranslator(m_translator);
        delete m_translator;
        m_translator = nullptr;
    }
    
    // 加载新的翻译
    m_translator = new QTranslator(this);
    QString translationFile = QString(":/translations/jitsi_%1.qm").arg(language);
    
    if (m_translator->load(translationFile)) {
        installTranslator(m_translator);
        qDebug() << "语言切换成功:" << language;
    } else {
        qWarning() << "无法加载翻译文件:" << translationFile;
        delete m_translator;
        m_translator = nullptr;
    }
    
    // 保存到配置
    if (m_configManager) {
        m_configManager->setCurrentLanguage(language);
    }
}

QString MainApplication::currentLanguage() const
{
    return m_currentLanguage;
}

// 主题设置方法已移除，现在只使用单一的main.qss样式文件

// currentTheme方法已移除

bool MainApplication::isInitialized() const
{
    return m_initialized;
}

void MainApplication::quit()
{
    qDebug() << "退出应用程序";
    
    // 保存窗口状态
    if (m_configManager) {
        m_configManager->sync();
    }
    
    QApplication::quit();
}

void MainApplication::showAbout()
{
    QString aboutText = tr(
        "<h3>Jitsi Meet Qt</h3>"
        "<p>版本: %1</p>"
        "<p>基于Qt的Jitsi Meet桌面客户端</p>"
        "<p>Copyright © 2025</p>"
        "<p><a href='https://jitsi.org'>https://jitsi.org</a></p>"
    ).arg(applicationVersion());
    
    QMessageBox::about(nullptr, tr("关于 Jitsi Meet Qt"), aboutText);
}

void MainApplication::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        // 显示主窗口
        if (m_conferenceWindow && m_conferenceWindow->isVisible()) {
            m_conferenceWindow->show();
            m_conferenceWindow->raise();
            m_conferenceWindow->activateWindow();
        } else {
            showWelcomeWindow();
        }
        break;
    default:
        break;
    }
}

bool MainApplication::event(QEvent *event)
{
    // 处理文件打开事件（macOS）
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        QString url = openEvent->url().toString();
        ProtocolHandler::MeetingInfo meetingInfo;
        meetingInfo.fullUrl = url;
        meetingInfo.isValid = parseProtocolUrl(url, meetingInfo.roomName, meetingInfo.serverUrl);
        handleProtocolUrl(meetingInfo);
        return true;
    }
    
    return QApplication::event(event);
}

void MainApplication::onConferenceWindowClosed()
{
    qDebug() << "会议窗口关闭";
    showWelcomeWindow();
}

void MainApplication::onWelcomeWindowClosed()
{
    qDebug() << "欢迎窗口关闭";
    // 如果没有其他窗口显示，则退出应用程序
    if (!m_conferenceWindow || !m_conferenceWindow->isVisible()) {
        quit();
    }
}

void MainApplication::initializeSystemTray()
{
    qDebug() << "初始化系统托盘";
    
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/icons/app.svg"));
    m_trayIcon->setToolTip(tr("Jitsi Meet Qt"));
    
    createTrayMenu();
    
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &MainApplication::onTrayIconActivated);
    
    m_trayIcon->show();
}

void MainApplication::initializeTranslations()
{
    qDebug() << "初始化翻译";
    
    // 从配置中读取语言设置
    if (m_configManager) {
        QString savedLanguage = m_configManager->getCurrentLanguage();
        if (!savedLanguage.isEmpty()) {
            setLanguage(savedLanguage);
            return;
        }
    }
    
    // 使用系统默认语言
    QString systemLocale = QLocale::system().name();
    setLanguage(systemLocale);
}

void MainApplication::initializeTheme()
{
    qDebug() << "初始化主题";
    
    // 直接加载main.qss样式文件
    loadStyleSheet("main");
}

void MainApplication::loadStyleSheet(const QString &themeName)
{
    qDebug() << "同步加载样式表:" << themeName;
    
    // 直接加载main.qss样式文件
    QString styleFile = ":/styles/main.qss";
    QFile file(styleFile);
    
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        setStyleSheet(styleSheet);
        qDebug() << "样式表加载成功:" << styleFile;
    } else {
        qWarning() << "无法加载样式表:" << styleFile;
    }
}

/**
 * @brief 异步预加载静态资源
 */
void MainApplication::preloadResourcesAsync()
{
    qDebug() << "开始异步预加载静态资源...";
    
    // 异步加载样式表
    loadStyleSheetAsync("default");
    
    // 可以在这里添加其他资源的异步加载，如图标、翻译文件等
    QtConcurrent::run([this]() {
        // 预加载应用图标
        QIcon appIcon(":/icons/app.svg");
        
        // 预加载其他常用图标
        QIcon settingsIcon(":/icons/settings.svg");
        QIcon aboutIcon(":/icons/about.svg");
        
        qDebug() << "静态图标资源预加载完成";
    });
}

/**
 * @brief 异步加载样式表
 * @param themeName 主题名称
 */
void MainApplication::loadStyleSheetAsync(const QString &themeName)
{
    qDebug() << "异步加载样式表:" << themeName;
    
    // 使用QtConcurrent在后台线程加载样式表
    QFuture<QString> future = QtConcurrent::run([themeName]() -> QString {
        QString styleFile = ":/styles/main.qss";
        QFile file(styleFile);
        
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&file);
            QString styleSheet = stream.readAll();
            qDebug() << "样式表异步加载完成:" << styleFile;
            return styleSheet;
        } else {
            qWarning() << "无法异步加载样式表:" << styleFile;
            return QString();
        }
    });
    
    // 监听加载完成
    QFutureWatcher<QString>* watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher]() {
        QString styleSheet = watcher->result();
        if (!styleSheet.isEmpty()) {
            onStyleSheetLoaded(styleSheet);
        }
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

/**
 * @brief 样式表异步加载完成处理
 * @param styleSheet 加载的样式表内容
 */
void MainApplication::onStyleSheetLoaded(const QString &styleSheet)
{
    // 在主线程中应用样式表
    setStyleSheet(styleSheet);
    qDebug() << "异步加载的样式表已应用";
}

void MainApplication::createTrayMenu()
{
    m_trayMenu = new QMenu();
    
    m_showAction = new QAction(tr("显示"), this);
    connect(m_showAction, &QAction::triggered, this, &MainApplication::showWelcomeWindow);
    
    m_settingsAction = new QAction(tr("设置"), this);
    connect(m_settingsAction, &QAction::triggered, this, &MainApplication::showSettingsDialog);
    
    m_aboutAction = new QAction(tr("关于"), this);
    connect(m_aboutAction, &QAction::triggered, this, &MainApplication::showAbout);
    
    m_quitAction = new QAction(tr("退出"), this);
    connect(m_quitAction, &QAction::triggered, this, &MainApplication::quit);
    
    m_trayMenu->addAction(m_showAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_settingsAction);
    m_trayMenu->addAction(m_aboutAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_quitAction);
    
    m_trayIcon->setContextMenu(m_trayMenu);
}

bool MainApplication::parseProtocolUrl(const QString &url, QString &roomName, QString &serverUrl)
{
    qDebug() << "解析协议URL:" << url;
    
    // jitsi-meet://room-name 或 jitsi-meet://server.com/room-name
    QRegularExpression regex(R"(^jitsi-meet://(?:([^/]+)/)?(.+)$)");
    QRegularExpressionMatch match = regex.match(url);
    
    if (!match.hasMatch()) {
        qWarning() << "URL格式不匹配:" << url;
        return false;
    }
    
    QString server = match.captured(1);
    roomName = match.captured(2);
    
    if (server.isEmpty()) {
        // 使用默认服务器
        serverUrl = m_configManager ? m_configManager->getDefaultServerUrl() : "https://meet.jit.si";
    } else {
        serverUrl = QString("https://%1").arg(server);
    }
    
    qDebug() << "解析结果 - 房间:" << roomName << "服务器:" << serverUrl;
    return true;
}

/**
 * @brief 异步初始化ConferenceWindow（并行初始化）
 */
void MainApplication::initializeConferenceWindowAsync()
{
    if (m_conferenceWindowReady) {
        qDebug() << "ConferenceWindow已经初始化完成";
        return;
    }
    
    if (m_conferenceWindow) {
        qDebug() << "ConferenceWindow异步初始化已在进行中";
        return;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "开始在主线程中创建ConferenceWindow";
    
    // 在主线程中创建ConferenceWindow（延迟WebEngine初始化）
    m_conferenceWindow = std::make_unique<ConferenceWindow>();
    
    // 连接信号
    connect(m_conferenceWindow.get(), &ConferenceWindow::conferenceLeft,
            this, &MainApplication::onConferenceWindowClosed);
    
    // 标记为已准备就绪
    m_conferenceWindowReady = true;
    
    qDebug() << QString("ConferenceWindow在主线程中创建完成，耗时: %1ms").arg(timer.elapsed());
    
    // 发射准备就绪信号
    emit conferenceWindowReady();
}

/**
 * @brief 检查ConferenceWindow是否已准备就绪
 */
bool MainApplication::isConferenceWindowReady() const
{
    return m_conferenceWindowReady && m_conferenceWindow != nullptr;
}