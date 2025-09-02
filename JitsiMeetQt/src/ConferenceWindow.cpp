#include "ConferenceWindow.h"
#include "ConfigurationManager.h"
#include "JitsiMeetAPI.h"

#include <QApplication>
#include <QDesktopServices>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QSplitter>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QSizePolicy>
#include <QScreen>
#include <QWindow>
// WebEngine temporarily disabled due to MSVC configuration issues
// #include <QWebEngineView>
// #include <QWebEnginePage>
// #include <QWebEngineProfile>
// #include <QWebEngineSettings>
#include <QLabel>
#include <QWidget>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrlQuery>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

/**
 * @brief ConferenceWindow构造函数
 * @param parent 父窗口
 */
ConferenceWindow::ConferenceWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    // , m_webView(nullptr)  // 注释掉WebEngine相关代码
    , m_statusDisplay(nullptr)
    , m_webContainer(nullptr)
    , m_toolbar(nullptr)
    , m_muteAction(nullptr)
    , m_cameraAction(nullptr)
    , m_screenShareAction(nullptr)
    , m_chatAction(nullptr)
    , m_fullscreenAction(nullptr)
    , m_leaveAction(nullptr)
    , m_settingsAction(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
    , m_participantCountLabel(nullptr)
    , m_connectionTimer(nullptr)
    , m_reconnectTimer(nullptr)
    , m_networkManager(nullptr)
    , m_configManager(ConfigurationManager::instance())
    , m_jitsiAPI(nullptr)
    , m_currentUrl()
    , m_currentRoom()
    , m_currentServer()
    , m_displayName()
    , m_isInConference(false)
    , m_isLoading(false)
    , m_isMuted(false)
    , m_isCameraOff(false)
    , m_isScreenSharing(false)
    , m_isChatVisible(false)
    , m_isFullscreen(false)
    , m_participantCount(0)
    , m_loadProgress(0)
    , m_reconnectAttempts(0)
{
    // 初始化组件
    initializeUI();
    initializeToolbar();
    
    // 创建定时器
    m_connectionTimer = new QTimer(this);
    m_connectionTimer->setSingleShot(true);
    connect(m_connectionTimer, &QTimer::timeout, this, &ConferenceWindow::onConnectionTimeout);
    
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &ConferenceWindow::onReconnectTimer);
    
    // 创建网络管理器
    m_networkManager = new QNetworkAccessManager(this);
    
    // 创建Jitsi Meet API
    m_jitsiAPI = new JitsiMeetAPI(this);
    
    // 恢复窗口状态
    restoreWindowState();
}

/**
 * @brief ConferenceWindow析构函数
 */
ConferenceWindow::~ConferenceWindow()
{
    // 清理资源
    
    // 保存窗口状态
    saveWindowState();
    
    // 离开会议
    if (m_isInConference) {
        leaveConference();
    }
    
    // QTextBrowser不需要特殊清理
    // 父对象会自动清理子对象
}

/**
 * @brief 初始化UI界面
 */
void ConferenceWindow::initializeUI()
{
    
    // 设置窗口属性
    setWindowTitle(tr("Jitsi Meet Qt"));
    setWindowIcon(QIcon(":/icons/app.svg"));
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    // 确保窗口有标准的控制按钮（最小化、最大化、关闭）
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    
    // 设置窗口可以调整大小
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 创建中央窗口部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主布局
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // 创建状态栏
    QStatusBar* statusBar = this->statusBar();
    
    // 创建状态标签
    m_statusLabel = new QLabel(tr("就绪"), this);
    statusBar->addWidget(m_statusLabel);
    
    // 创建进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    statusBar->addPermanentWidget(m_progressBar);
    
    // 创建参与者数量标签
    m_participantCountLabel = new QLabel(tr("参与者: 0"), this);
    statusBar->addPermanentWidget(m_participantCountLabel);
}

/**
 * @brief 初始化WebEngine
 */
void ConferenceWindow::initializeWebEngine()
{
    
    // 创建Web容器
    m_webContainer = new QWidget(this);
    QVBoxLayout* containerLayout = new QVBoxLayout(m_webContainer);
    
    // 创建状态显示标签
    m_statusDisplay = new QLabel("Jitsi Meet 会议界面（简化版）", this);
    m_statusDisplay->setAlignment(Qt::AlignCenter);
    m_statusDisplay->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; padding: 20px; font-size: 14px; }");
    
    // 创建Web视图（使用QWebEngineView）- 注释掉WebEngine相关代码
    // m_webView = new QWebEngineView(this);
    
    // 连接WebEngine信号 - 注释掉WebEngine相关代码
    // connect(m_webView, &QWebEngineView::loadStarted, this, &ConferenceWindow::onLoadStarted);
    // connect(m_webView, &QWebEngineView::loadProgress, this, &ConferenceWindow::onLoadProgress);
    // connect(m_webView, &QWebEngineView::loadFinished, this, &ConferenceWindow::onLoadFinished);
    // connect(m_webView, &QWebEngineView::titleChanged, this, &ConferenceWindow::onTitleChanged);
    // connect(m_webView, &QWebEngineView::urlChanged, this, &ConferenceWindow::onUrlChanged);
    
    // 添加到容器布局
    containerLayout->addWidget(m_statusDisplay);
    // containerLayout->addWidget(m_webView);  // 注释掉WebEngine相关代码
    
    // 添加到主布局
    m_mainLayout->addWidget(m_webContainer);
    
    // 设置基本配置
    setupWebEngineSettings();
}

/**
 * @brief 设置WebEngine配置
 */
void ConferenceWindow::setupWebEngineSettings()
{
    // 设置WebEngine配置
    
    // 注释掉WebEngine相关代码
    /*
    if (!m_webView) {
        return;
    }
    
    // 获取WebEngine页面和设置
    QWebEnginePage* page = m_webView->page();
    QWebEngineSettings* settings = page->settings();
    
    // 启用必要的功能
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, true);
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    settings->setAttribute(QWebEngineSettings::WebRTCPublicInterfacesOnly, false);
    
    // 设置用户代理
    QString userAgent = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36";
    page->profile()->setHttpUserAgent(userAgent);
    */
    
    // 设置状态显示
    if (m_statusDisplay) {
        m_statusDisplay->setText("Jitsi Meet 客户端已就绪");
    }
}

/**
 * @brief 初始化工具栏
 */
void ConferenceWindow::initializeToolbar()
{
    
    // 创建工具栏
    m_toolbar = addToolBar(tr("会议控制"));
    m_toolbar->setMovable(false);
    m_toolbar->setFloatable(false);
    
    // 创建动作
    m_muteAction = new QAction(QIcon(":/icons/microphone.svg"), tr("静音"), this);
    m_muteAction->setCheckable(true);
    m_muteAction->setToolTip(tr("切换麦克风静音状态"));
    connect(m_muteAction, &QAction::triggered, this, &ConferenceWindow::onMuteAction);
    
    m_cameraAction = new QAction(QIcon(":/icons/camera.svg"), tr("摄像头"), this);
    m_cameraAction->setCheckable(true);
    m_cameraAction->setToolTip(tr("切换摄像头开关状态"));
    connect(m_cameraAction, &QAction::triggered, this, &ConferenceWindow::onCameraAction);
    
    m_screenShareAction = new QAction(QIcon(":/icons/screen-share.svg"), tr("屏幕共享"), this);
    m_screenShareAction->setCheckable(true);
    m_screenShareAction->setToolTip(tr("切换屏幕共享状态"));
    connect(m_screenShareAction, &QAction::triggered, this, &ConferenceWindow::onScreenShareAction);
    
    m_chatAction = new QAction(QIcon(":/icons/chat.svg"), tr("聊天"), this);
    m_chatAction->setCheckable(true);
    m_chatAction->setToolTip(tr("显示/隐藏聊天面板"));
    connect(m_chatAction, &QAction::triggered, this, &ConferenceWindow::onChatAction);
    
    m_fullscreenAction = new QAction(QIcon(":/icons/fullscreen.svg"), tr("全屏"), this);
    m_fullscreenAction->setCheckable(true);
    m_fullscreenAction->setToolTip(tr("切换全屏模式"));
    connect(m_fullscreenAction, &QAction::triggered, this, &ConferenceWindow::onFullscreenAction);
    
    m_leaveAction = new QAction(QIcon(":/icons/leave.svg"), tr("离开"), this);
    m_leaveAction->setToolTip(tr("离开会议"));
    connect(m_leaveAction, &QAction::triggered, this, &ConferenceWindow::onLeaveAction);
    
    m_settingsAction = new QAction(QIcon(":/icons/settings.svg"), tr("设置"), this);
    m_settingsAction->setToolTip(tr("打开设置"));
    connect(m_settingsAction, &QAction::triggered, this, &ConferenceWindow::onSettingsAction);
    
    // 添加动作到工具栏
    m_toolbar->addAction(m_muteAction);
    m_toolbar->addAction(m_cameraAction);
    m_toolbar->addAction(m_screenShareAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_chatAction);
    m_toolbar->addAction(m_fullscreenAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_leaveAction);
    m_toolbar->addAction(m_settingsAction);
    
    // 初始状态下禁用控制按钮
    updateToolbarState();
}

/**
 * @brief 初始化JavaScript桥接
 */
void ConferenceWindow::initializeJavaScriptBridge()
{
    
    // 注释掉WebEngine相关代码
    /*
    if (!m_webView) {
        return;
    }
    
    // 注入JavaScript代码将在页面加载完成后执行
    // 这里可以设置JavaScript与Qt的通信桥接
    
    // 连接JavaScript控制台消息
    connect(m_webView->page(), &QWebEnginePage::javaScriptConsoleMessage,
            [this](QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID) {
                qDebug() << "JavaScript Console:" << message << "at line" << lineNumber << "in" << sourceID;
            });
    */
}

/**
 * @brief 注入JavaScript代码
 */
void ConferenceWindow::injectJavaScript()
{
    // 注入JavaScript代码
    
    // 注释掉WebEngine相关代码
    /*
    if (!m_webView) {
        return;
    }
    
    QString script = R"(
        // 创建Qt与Jitsi Meet的桥接对象
        window.qtJitsiMeet = {
            // 会议控制方法
            toggleMute: function() {
                if (window.APP && window.APP.conference) {
                    window.APP.conference.toggleAudioMuted();
                }
            },
            
            toggleCamera: function() {
                if (window.APP && window.APP.conference) {
                    window.APP.conference.toggleVideoMuted();
                }
            },
            
            toggleScreenShare: function() {
                if (window.APP && window.APP.conference) {
                    window.APP.conference.toggleScreenSharing();
                }
            },
            
            toggleChat: function() {
                if (window.APP && window.APP.UI) {
                    window.APP.UI.toggleChat();
                }
            },
            
            leaveConference: function() {
                if (window.APP && window.APP.conference) {
                    window.APP.conference.hangup();
                }
            },
            
            setDisplayName: function(name) {
                if (window.APP && window.APP.conference) {
                    window.APP.conference.changeLocalDisplayName(name);
                }
            },
            
            sendChatMessage: function(message) {
                if (window.APP && window.APP.conference) {
                    window.APP.conference.sendTextMessage(message);
                }
            }
        };
        
        // 监听Jitsi Meet事件
        if (window.APP) {
            // 会议加入事件
            window.APP.conference.addConferenceListener('conference.joined', function() {
                console.log('Qt: Conference joined');
            });
            
            // 会议离开事件
            window.APP.conference.addConferenceListener('conference.left', function() {
                console.log('Qt: Conference left');
            });
            
            // 参与者事件
            window.APP.conference.addConferenceListener('participant.joined', function(id, user) {
                console.log('Qt: Participant joined:', id, user.getDisplayName());
            });
            
            window.APP.conference.addConferenceListener('participant.left', function(id, user) {
                console.log('Qt: Participant left:', id);
            });
            
            // 音视频状态变化
            window.APP.conference.addConferenceListener('track.audioLevelsChanged', function(audioLevels) {
                // 音频级别变化
            });
        }
    )";
    
    m_webView->page()->runJavaScript(script);
    */
}

/**
 * @brief 加载会议URL
 * @param url 会议URL
 * @param displayName 显示名称
 * @param password 会议密码
 * @return 是否成功开始加载
 */
bool ConferenceWindow::loadConference(const QString& url, const QString& displayName, const QString& password)
{
    // 加载会议URL
    
    if (url.isEmpty()) {
        qWarning() << "ConferenceWindow: 会议URL为空";
        return false;
    }
    
    // 保存参数
    m_currentUrl = url;
    m_displayName = displayName.isEmpty() ? m_configManager->getDefaultDisplayName() : displayName;
    
    // 解析URL获取房间信息
    QJsonObject urlInfo = parseConferenceUrl(url);
    m_currentRoom = urlInfo["room"].toString();
    m_currentServer = urlInfo["server"].toString();
    
    // 构建完整URL
    QString fullUrl = url;
    QUrlQuery query;
    
    // 添加显示名称
    if (!m_displayName.isEmpty()) {
        query.addQueryItem("displayName", m_displayName);
    }
    
    // 添加密码
    if (!password.isEmpty()) {
        query.addQueryItem("password", password);
    }
    
    // 添加其他配置参数
    bool audioMuted = m_configManager->getValue("defaultAudioMuted", false).toBool();
    bool videoMuted = m_configManager->getValue("defaultVideoMuted", false).toBool();
    query.addQueryItem("config.startWithAudioMuted", audioMuted ? "true" : "false");
    query.addQueryItem("config.startWithVideoMuted", videoMuted ? "true" : "false");
    query.addQueryItem("config.prejoinPageEnabled", "false");
    query.addQueryItem("config.disableDeepLinking", "true");
    
    if (!query.isEmpty()) {
        QUrl qurl(fullUrl);
        qurl.setQuery(query);
        fullUrl = qurl.toString();
    }
    
    // 构建最终URL
    
    // 开始加载
    m_isLoading = true;
    m_connectionTimer->start(CONNECTION_TIMEOUT);
    
    // 注释掉WebEngine相关代码
    /*
    // 使用QWebEngineView加载实际的Jitsi Meet页面
    m_webView->load(QUrl(fullUrl));
    
    // 初始化WebEngine（如果还未初始化）
    if (!m_webView->page()) {
        initializeWebEngine();
    }
    */
    
    // 更新UI
    updateWindowTitle();
    showLoadingIndicator(true);
    
    return true;
}

/**
 * @brief 加载会议房间
 * @param roomName 房间名称
 * @param serverUrl 服务器URL
 * @param displayName 显示名称
 * @param password 会议密码
 * @return 是否成功开始加载
 */
bool ConferenceWindow::loadRoom(const QString& roomName, const QString& serverUrl, 
                               const QString& displayName, const QString& password)
{
    // 加载会议房间
    
    if (roomName.isEmpty()) {
        qWarning() << "ConferenceWindow: 房间名称为空";
        return false;
    }
    
    // 构建会议URL
    QString server = serverUrl.isEmpty() ? m_configManager->getDefaultServerUrl() : serverUrl;
    QString url = buildConferenceUrl(roomName, server, displayName, password);
    
    return loadConference(url, displayName, password);
}

/**
 * @brief 构建会议URL
 * @param roomName 房间名称
 * @param serverUrl 服务器URL
 * @param displayName 显示名称
 * @param password 密码
 * @return 完整的会议URL
 */
QString ConferenceWindow::buildConferenceUrl(const QString& roomName, const QString& serverUrl, 
                                           const QString& /* displayName */, const QString& /* password */) const
{
    QString server = serverUrl;
    if (server.isEmpty()) {
        server = m_configManager->getDefaultServerUrl();
    }
    
    // 确保服务器URL格式正确
    if (!server.startsWith("http://") && !server.startsWith("https://")) {
        server = "https://" + server;
    }
    
    // 移除末尾的斜杠
    if (server.endsWith("/")) {
        server.chop(1);
    }
    
    // 构建URL
    QString url = server + "/" + roomName;
    
    qDebug() << "ConferenceWindow: 构建的会议URL:" << url;
    return url;
}

/**
 * @brief 解析会议URL
 * @param url 会议URL
 * @return 解析结果
 */
QJsonObject ConferenceWindow::parseConferenceUrl(const QString& url) const
{
    QJsonObject result;
    QUrl qurl(url);
    
    QString host = qurl.host();
    QString path = qurl.path();
    
    // 提取服务器URL
    QString server = qurl.scheme() + "://" + host;
    if (qurl.port() != -1) {
        server += ":" + QString::number(qurl.port());
    }
    
    // 提取房间名称
    QString room = path;
    if (room.startsWith("/")) {
        room = room.mid(1);
    }
    
    result["server"] = server;
    result["room"] = room;
    result["host"] = host;
    result["path"] = path;
    
    return result;
}

/**
 * @brief 获取当前会议URL
 * @return 当前会议URL
 */
QString ConferenceWindow::getCurrentUrl() const
{
    return m_currentUrl;
}

/**
 * @brief 获取当前房间名称
 * @return 当前房间名称
 */
QString ConferenceWindow::getCurrentRoom() const
{
    return m_currentRoom;
}

/**
 * @brief 检查是否正在会议中
 * @return 是否在会议中
 */
bool ConferenceWindow::isInConference() const
{
    return m_isInConference;
}

/**
 * @brief 离开当前会议
 */
void ConferenceWindow::leaveConference()
{
    qDebug() << "ConferenceWindow: 离开会议";
    
    if (m_isInConference) {
        // 通过JavaScript离开会议
        executeJavaScript("if (window.qtJitsiMeet) { window.qtJitsiMeet.leaveConference(); }");
        
        // 更新状态
        m_isInConference = false;
        updateToolbarState();
        updateWindowTitle();
        
        // 发送信号
        emit conferenceLeft(m_currentRoom);
    }
    
    // 清空当前信息
    m_currentUrl.clear();
    m_currentRoom.clear();
    m_participantCount = 0;
    
    // 更新UI
    m_statusLabel->setText(tr("已离开会议"));
    m_participantCountLabel->setText(tr("参与者: 0"));
}

/**
 * @brief 加入会议
 * @param roomName 房间名称
 * @param serverUrl 服务器URL
 */
void ConferenceWindow::joinConference(const QString& roomName, const QString& serverUrl)
{
    qDebug() << "加入会议:" << roomName << "服务器:" << serverUrl;
    
    // 使用loadRoom方法加载会议
    if (loadRoom(roomName, serverUrl)) {
        show();
        raise();
        activateWindow();
    } else {
        qWarning() << "无法加入会议:" << roomName;
        showError(QString("无法加入会议: %1").arg(roomName));
    }
}

/**
 * @brief 切换静音状态
 */
void ConferenceWindow::toggleMute()
{
    qDebug() << "ConferenceWindow: 切换静音状态";
    
    if (m_isInConference) {
        executeJavaScript("if (window.qtJitsiMeet) { window.qtJitsiMeet.toggleMute(); }");
    }
}

/**
 * @brief 切换摄像头状态
 */
void ConferenceWindow::toggleCamera()
{
    qDebug() << "ConferenceWindow: 切换摄像头状态";
    
    if (m_isInConference) {
        executeJavaScript("if (window.qtJitsiMeet) { window.qtJitsiMeet.toggleCamera(); }");
    }
}

/**
 * @brief 切换屏幕共享
 */
void ConferenceWindow::toggleScreenShare()
{
    qDebug() << "ConferenceWindow: 切换屏幕共享";
    
    if (m_isInConference) {
        executeJavaScript("if (window.qtJitsiMeet) { window.qtJitsiMeet.toggleScreenShare(); }");
    }
}

/**
 * @brief 切换聊天面板
 */
void ConferenceWindow::toggleChat()
{
    qDebug() << "ConferenceWindow: 切换聊天面板";
    
    if (m_isInConference) {
        executeJavaScript("if (window.qtJitsiMeet) { window.qtJitsiMeet.toggleChat(); }");
        m_isChatVisible = !m_isChatVisible;
        m_chatAction->setChecked(m_isChatVisible);
    }
}

/**
 * @brief 切换全屏模式
 */
void ConferenceWindow::toggleFullscreen()
{
    qDebug() << "ConferenceWindow: 切换全屏模式";
    
    if (m_isFullscreen) {
        showNormal();
        m_toolbar->setVisible(true);
        statusBar()->setVisible(true);
        m_isFullscreen = false;
    } else {
        showFullScreen();
        m_toolbar->setVisible(false);
        statusBar()->setVisible(false);
        m_isFullscreen = true;
    }
    
    m_fullscreenAction->setChecked(m_isFullscreen);
}

/**
 * @brief 设置显示名称
 * @param displayName 显示名称
 */
void ConferenceWindow::setDisplayName(const QString& displayName)
{
    qDebug() << "ConferenceWindow: 设置显示名称:" << displayName;
    
    m_displayName = displayName;
    
    if (m_isInConference) {
        QString escapedName = displayName;
        escapedName.replace("'", "\\'");  // 转义单引号
        QString script = QString("if (window.qtJitsiMeet) { window.qtJitsiMeet.setDisplayName('%1'); }")
                        .arg(escapedName);
        executeJavaScript(script);
    }
}

/**
 * @brief 发送聊天消息
 * @param message 消息内容
 */
void ConferenceWindow::sendChatMessage(const QString& message)
{
    qDebug() << "ConferenceWindow: 发送聊天消息:" << message;
    
    if (m_isInConference && !message.isEmpty()) {
        QString escapedMessage = message;
        escapedMessage.replace("'", "\\'");  // 转义单引号
        QString script = QString("if (window.qtJitsiMeet) { window.qtJitsiMeet.sendChatMessage('%1'); }")
                        .arg(escapedMessage);
        executeJavaScript(script);
    }
}

/**
 * @brief 执行JavaScript代码
 * @param script JavaScript代码
 * @param callback 回调函数
 */
void ConferenceWindow::executeJavaScript(const QString& script, std::function<void(const QVariant&)> callback)
{
    qDebug() << "ConferenceWindow: 执行JavaScript:" << script;
    
    // 注释掉WebEngine相关代码
    /*
    if (!m_webView) {
        if (callback) {
            callback(QVariant());
        }
        return;
    }
    
    if (callback) {
        m_webView->page()->runJavaScript(script, [callback](const QVariant& result) {
            callback(result);
        });
    } else {
        m_webView->page()->runJavaScript(script);
    }
    */
}

/**
 * @brief 页面加载开始处理
 */
void ConferenceWindow::onLoadStarted()
{
    qDebug() << "ConferenceWindow: 页面开始加载";
    
    m_isLoading = true;
    m_loadProgress = 0;
    
    showLoadingIndicator(true);
    m_statusLabel->setText(tr("正在连接..."));
    
    // 启动连接超时定时器
    m_connectionTimer->start(CONNECTION_TIMEOUT);
}

/**
 * @brief 页面加载进度处理
 * @param progress 加载进度
 */
void ConferenceWindow::onLoadProgress(int progress)
{
    m_loadProgress = progress;
    m_progressBar->setValue(progress);
    
    m_statusLabel->setText(tr("正在加载... %1%").arg(progress));
}

/**
 * @brief 页面加载完成处理
 * @param success 是否加载成功
 */
void ConferenceWindow::onLoadFinished(bool success)
{
    qDebug() << "ConferenceWindow: 页面加载完成, 成功:" << success;
    
    m_isLoading = false;
    m_connectionTimer->stop();
    showLoadingIndicator(false);
    
    if (success) {
        m_statusLabel->setText(tr("已连接"));
        m_reconnectAttempts = 0; // 重置重连次数
    } else {
        m_statusLabel->setText(tr("连接失败"));
        showError(tr("无法连接到会议服务器"));
        
        // 尝试重连
        if (m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
            m_reconnectAttempts++;
            m_statusLabel->setText(tr("准备重连... (%1/%2)").arg(m_reconnectAttempts).arg(MAX_RECONNECT_ATTEMPTS));
            m_reconnectTimer->start(RECONNECT_DELAY);
        } else {
            emit conferenceLoadFailed(tr("连接失败，已达到最大重试次数"));
        }
    }
}

/**
 * @brief 页面标题变化处理
 * @param title 新标题
 */
void ConferenceWindow::onTitleChanged(const QString& title)
{
    qDebug() << "ConferenceWindow: 页面标题变化:" << title;
    updateWindowTitle();
}

/**
 * @brief 页面URL变化处理
 * @param url 新URL
 */
void ConferenceWindow::onUrlChanged(const QUrl& url)
{
    qDebug() << "ConferenceWindow: 页面URL变化:" << url.toString();
    
    m_currentUrl = url.toString();
    
    // 解析新的URL信息
    QJsonObject urlInfo = parseConferenceUrl(m_currentUrl);
    m_currentRoom = urlInfo["room"].toString();
    m_currentServer = urlInfo["server"].toString();
    
    updateWindowTitle();
}

/**
 * @brief 处理会议加入事件
 */
void ConferenceWindow::onConferenceJoined()
{
    qDebug() << "ConferenceWindow: 会议已加入";
    
    m_isInConference = true;
    m_statusLabel->setText(tr("已加入会议"));
    
    updateToolbarState();
    updateWindowTitle();
    
    emit conferenceJoined(m_currentRoom);
}

/**
 * @brief 处理会议离开事件
 */
void ConferenceWindow::onConferenceLeft()
{
    qDebug() << "ConferenceWindow: 会议已离开";
    
    m_isInConference = false;
    m_participantCount = 0;
    
    updateToolbarState();
    updateWindowTitle();
    
    m_statusLabel->setText(tr("已离开会议"));
    m_participantCountLabel->setText(tr("参与者: 0"));
    
    emit conferenceLeft(m_currentRoom);
}

/**
 * @brief 处理参与者加入事件
 * @param participantId 参与者ID
 * @param displayName 显示名称
 */
void ConferenceWindow::onParticipantJoined(const QString& participantId, const QString& displayName)
{
    qDebug() << "ConferenceWindow: 参与者加入:" << participantId << displayName;
    
    m_participantCount++;
    m_participantCountLabel->setText(tr("参与者: %1").arg(m_participantCount));
    
    emit participantCountChanged(m_participantCount);
}

/**
 * @brief 处理参与者离开事件
 * @param participantId 参与者ID
 */
void ConferenceWindow::onParticipantLeft(const QString& participantId)
{
    qDebug() << "ConferenceWindow: 参与者离开:" << participantId;
    
    if (m_participantCount > 0) {
        m_participantCount--;
    }
    m_participantCountLabel->setText(tr("参与者: %1").arg(m_participantCount));
    
    emit participantCountChanged(m_participantCount);
}

/**
 * @brief 处理聊天消息接收
 * @param senderId 发送者ID
 * @param message 消息内容
 * @param timestamp 时间戳
 */
void ConferenceWindow::onChatMessageReceived(const QString& senderId, const QString& message, qint64 timestamp)
{
    qDebug() << "ConferenceWindow: 收到聊天消息:" << senderId << message;
    
    emit chatMessageReceived(senderId, message, timestamp);
}

/**
 * @brief 处理音频状态变化
 * @param muted 是否静音
 */
void ConferenceWindow::onAudioMuteChanged(bool muted)
{
    qDebug() << "ConferenceWindow: 音频状态变化:" << muted;
    
    m_isMuted = muted;
    m_muteAction->setChecked(muted);
    m_muteAction->setIcon(QIcon(muted ? ":/icons/microphone-off.svg" : ":/icons/microphone.svg"));
    m_muteAction->setText(muted ? tr("取消静音") : tr("静音"));
}

/**
 * @brief 处理视频状态变化
 * @param muted 是否关闭摄像头
 */
void ConferenceWindow::onVideoMuteChanged(bool muted)
{
    qDebug() << "ConferenceWindow: 视频状态变化:" << muted;
    
    m_isCameraOff = muted;
    m_cameraAction->setChecked(muted);
    m_cameraAction->setIcon(QIcon(muted ? ":/icons/camera-off.svg" : ":/icons/camera.svg"));
    m_cameraAction->setText(muted ? tr("开启摄像头") : tr("关闭摄像头"));
}

/**
 * @brief 处理屏幕共享状态变化
 * @param sharing 是否正在共享
 */
void ConferenceWindow::onScreenShareChanged(bool sharing)
{
    qDebug() << "ConferenceWindow: 屏幕共享状态变化:" << sharing;
    
    m_isScreenSharing = sharing;
    m_screenShareAction->setChecked(sharing);
    m_screenShareAction->setText(sharing ? tr("停止共享") : tr("屏幕共享"));
}

/**
 * @brief 处理工具栏静音动作
 */
void ConferenceWindow::onMuteAction()
{
    toggleMute();
}

/**
 * @brief 处理工具栏摄像头动作
 */
void ConferenceWindow::onCameraAction()
{
    toggleCamera();
}

/**
 * @brief 处理工具栏屏幕共享动作
 */
void ConferenceWindow::onScreenShareAction()
{
    toggleScreenShare();
}

/**
 * @brief 处理工具栏聊天动作
 */
void ConferenceWindow::onChatAction()
{
    toggleChat();
}

/**
 * @brief 处理工具栏全屏动作
 */
void ConferenceWindow::onFullscreenAction()
{
    toggleFullscreen();
}

/**
 * @brief 处理工具栏离开动作
 */
void ConferenceWindow::onLeaveAction()
{
    leaveConference();
    close();
}

/**
 * @brief 处理工具栏设置动作
 */
void ConferenceWindow::onSettingsAction()
{
    qDebug() << "ConferenceWindow: 打开设置";
    // TODO: 实现设置对话框
}

/**
 * @brief 处理连接超时
 */
void ConferenceWindow::onConnectionTimeout()
{
    qDebug() << "ConferenceWindow: 连接超时";
    
    if (m_isLoading) {
        m_isLoading = false;
        showLoadingIndicator(false);
        
        // 尝试重连
        if (m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
            m_reconnectAttempts++;
            m_statusLabel->setText(tr("连接超时，准备重连... (%1/%2)").arg(m_reconnectAttempts).arg(MAX_RECONNECT_ATTEMPTS));
            qDebug() << "ConferenceWindow: 准备第" << m_reconnectAttempts << "次重连";
            
            // 启动重连定时器
            m_reconnectTimer->start(RECONNECT_DELAY);
        } else {
            showError(tr("连接超时，已达到最大重试次数"));
            emit conferenceLoadFailed(tr("连接超时，已达到最大重试次数"));
        }
    }
}

/**
 * @brief 处理重连定时器
 */
void ConferenceWindow::onReconnectTimer()
{
    qDebug() << "ConferenceWindow: 尝试重连";
    
    // 停止重连定时器
    m_reconnectTimer->stop();
    
    if (!m_currentUrl.isEmpty()) {
        m_statusLabel->setText(tr("正在重连..."));
        
        // 重置加载状态
        m_isLoading = true;
        showLoadingIndicator(true);
        
        // 启动连接超时定时器
        m_connectionTimer->start(CONNECTION_TIMEOUT);
        
        // 使用系统默认浏览器重新打开会议链接
        qDebug() << "ConferenceWindow: 重新打开会议链接:" << m_currentUrl;
        
        // 尝试通过网络请求检查连接状态
        if (m_networkManager) {
            QUrl url(m_currentUrl);
            QNetworkRequest request(url);
            request.setRawHeader("User-Agent", "JitsiMeetQt/1.0");
            request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
            
            QNetworkReply* reply = m_networkManager->get(request);
            connect(reply, &QNetworkReply::finished, this, &ConferenceWindow::onNetworkReplyFinished);
            connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
                    this, QOverload<QNetworkReply::NetworkError>::of(&ConferenceWindow::onNetworkError));
        }
        
        // 使用内嵌WebEngine重新加载会议页面
        // 注释掉浏览器调用，改为内嵌显示
        // QDesktopServices::openUrl(QUrl(m_currentUrl));
        
        // 如果有WebEngine视图，重新加载页面
        if (m_webContainer && m_webContainer->findChild<QWidget*>()) {
            // 重新加载当前URL到WebEngine
            qDebug() << "ConferenceWindow: 使用内嵌WebEngine重新加载会议页面";
            // 这里应该调用WebEngine的load方法，但由于当前使用的是简化版本
            // 暂时保持网络请求检查，不调用外部浏览器
        }
    }
}

/**
 * @brief 处理重连请求
 */
void ConferenceWindow::onReconnectRequested()
{
    qDebug() << "ConferenceWindow: 重连请求";
    
    // 重置重连计数器
    m_reconnectAttempts = 0;
    
    // 停止所有定时器
    m_connectionTimer->stop();
    m_reconnectTimer->stop();
    
    if (!m_currentUrl.isEmpty()) {
        m_statusLabel->setText(tr("正在重连..."));
        
        // 重置加载状态
        m_isLoading = true;
        showLoadingIndicator(true);
        
        // 启动连接超时定时器
        m_connectionTimer->start(CONNECTION_TIMEOUT);
        
        // 重新打开会议链接
        qDebug() << "ConferenceWindow: 手动重连，重新打开会议链接:" << m_currentUrl;
        QDesktopServices::openUrl(QUrl(m_currentUrl));
    }
}

// 删除重复的onLoadStarted和onLoadProgress方法定义，这些方法已在前面定义过

// 删除重复的onLoadFinished方法定义，该方法已在前面定义过

// 删除重复的方法定义，这些方法已在前面定义过

/**
 * @brief 更新窗口标题
 */
void ConferenceWindow::updateWindowTitle()
{
    QString title = tr("Jitsi Meet Qt");
    
    if (!m_currentRoom.isEmpty()) {
        title += tr(" - %1").arg(m_currentRoom);
        
        if (m_isInConference) {
            title += tr(" (已连接)");
        } else if (m_isLoading) {
            title += tr(" (连接中)");
        }
    }
    
    setWindowTitle(title);
}

/**
 * @brief 更新工具栏状态
 */
void ConferenceWindow::updateToolbarState()
{
    bool enabled = m_isInConference;
    
    m_muteAction->setEnabled(enabled);
    m_cameraAction->setEnabled(enabled);
    m_screenShareAction->setEnabled(enabled);
    m_chatAction->setEnabled(enabled);
    m_leaveAction->setEnabled(enabled || m_isLoading);
}

/**
 * @brief 显示加载指示器
 * @param show 是否显示
 */
void ConferenceWindow::showLoadingIndicator(bool show)
{
    m_progressBar->setVisible(show);
    if (show) {
        m_progressBar->setValue(m_loadProgress);
    }
}

/**
 * @brief 显示错误消息
 * @param error 错误信息
 */
void ConferenceWindow::showError(const QString& error)
{
    qWarning() << "ConferenceWindow: 错误:" << error;
    
    QMessageBox::warning(this, tr("会议错误"), error);
}

/**
 * @brief 保存窗口状态
 */
void ConferenceWindow::saveWindowState()
{
    if (m_configManager) {
        // 使用通用方法保存会议窗口状态
        m_configManager->setValue("conference_window_geometry", saveGeometry());
        m_configManager->setValue("conference_window_state", saveState());
        m_configManager->setValue("conference_window_maximized", isMaximized());
    }
}

/**
 * @brief 恢复窗口状态
 */
void ConferenceWindow::restoreWindowState()
{
    if (m_configManager) {
        // 使用通用方法恢复会议窗口状态
        QByteArray geometry = m_configManager->getValue("conference_window_geometry").toByteArray();
        if (!geometry.isEmpty()) {
            restoreGeometry(geometry);
        }
        
        QByteArray state = m_configManager->getValue("conference_window_state").toByteArray();
        if (!state.isEmpty()) {
            restoreState(state);
        }
        
        if (m_configManager->getValue("conference_window_maximized", false).toBool()) {
            showMaximized();
        }
    }
}

/**
 * @brief 窗口关闭事件
 * @param event 关闭事件
 */
void ConferenceWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "ConferenceWindow: 窗口关闭事件";
    
    // 保存窗口状态
    saveWindowState();
    
    // 离开会议
    if (m_isInConference) {
        leaveConference();
    }
    
    // 发送窗口关闭信号
    emit windowClosed();
    
    event->accept();
}

/**
 * @brief 窗口大小改变事件
 * @param event 大小改变事件
 */
void ConferenceWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    // 保存窗口状态（延迟保存以避免频繁写入）
    static QTimer* saveTimer = nullptr;
    if (!saveTimer) {
        saveTimer = new QTimer(this);
        saveTimer->setSingleShot(true);
        saveTimer->setInterval(1000); // 1秒后保存
        connect(saveTimer, &QTimer::timeout, this, &ConferenceWindow::saveWindowState);
    }
    saveTimer->start();
}

/**
 * @brief 窗口显示事件
 * @param event 显示事件
 */
void ConferenceWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    
    // 恢复窗口状态（仅第一次显示时）
    static bool firstShow = true;
    if (firstShow) {
        firstShow = false;
        restoreWindowState();
    }
}

/**
 * @brief 窗口隐藏事件
 * @param event 隐藏事件
 */
void ConferenceWindow::hideEvent(QHideEvent *event)
{
    QMainWindow::hideEvent(event);
    
    // 保存窗口状态
    saveWindowState();
}

/**
 * @brief 处理网络错误
 * @param error 错误信息
 */
void ConferenceWindow::onNetworkError(const QString& error)
{
    qDebug() << "ConferenceWindow: Network error:" << error;
    
    // 显示错误消息给用户
    QMessageBox::warning(this, tr("网络错误"), 
                        tr("网络连接出现问题：%1").arg(error));
}

/**
 * @brief 处理JavaScript消息
 * @param message JavaScript消息对象
 */
void ConferenceWindow::onJavaScriptMessage(const QJsonObject& message)
{
    qDebug() << "ConferenceWindow: JavaScript message:" << message;
    
    // 处理来自Web页面的JavaScript消息
    QString type = message["type"].toString();
    
    if (type == "conferenceJoined") {
        // 会议加入成功
        QString roomName = message["roomName"].toString();
        emit conferenceJoined(roomName);
    } else if (type == "conferenceLeft") {
        // 会议离开
        onConferenceJoined(); // 调用会议结束处理
    } else if (type == "error") {
        // 错误消息
        QString errorMsg = message["message"].toString();
        onNetworkError(errorMsg);
    }
}

/**
 * @brief 网络请求完成处理
 */
void ConferenceWindow::onNetworkReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    qDebug() << "ConferenceWindow: 网络请求完成，状态码:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    if (reply->error() == QNetworkReply::NoError) {
        // 网络连接正常，停止连接超时定时器
        m_connectionTimer->stop();
        
        if (m_isLoading) {
            m_isLoading = false;
            showLoadingIndicator(false);
            
            // 重置重连计数器
            m_reconnectAttempts = 0;
            
            m_statusLabel->setText(tr("连接成功，会议已在浏览器中打开"));
            qDebug() << "ConferenceWindow: 连接成功";
            
            // 发送会议加入信号
            emit conferenceJoined(m_currentRoom);
        }
    } else {
        qDebug() << "ConferenceWindow: 网络请求失败:" << reply->errorString();
        
        // 如果还在重连范围内，继续尝试
        if (m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
            onConnectionTimeout(); // 触发重连逻辑
        } else {
            m_isLoading = false;
            showLoadingIndicator(false);
            showError(tr("网络连接失败: %1").arg(reply->errorString()));
            emit conferenceLoadFailed(tr("网络连接失败: %1").arg(reply->errorString()));
        }
    }
    
    reply->deleteLater();
}

/**
 * @brief 网络错误处理
 * @param error 网络错误类型
 */
void ConferenceWindow::onNetworkError(QNetworkReply::NetworkError error)
{
    QString errorMsg;
    switch (error) {
        case QNetworkReply::ConnectionRefusedError:
            errorMsg = tr("连接被拒绝");
            break;
        case QNetworkReply::RemoteHostClosedError:
            errorMsg = tr("远程主机关闭连接");
            break;
        case QNetworkReply::HostNotFoundError:
            errorMsg = tr("主机未找到");
            break;
        case QNetworkReply::TimeoutError:
            errorMsg = tr("连接超时");
            break;
        case QNetworkReply::OperationCanceledError:
            errorMsg = tr("操作被取消");
            break;
        case QNetworkReply::SslHandshakeFailedError:
            errorMsg = tr("SSL握手失败");
            break;
        case QNetworkReply::TemporaryNetworkFailureError:
            errorMsg = tr("临时网络故障");
            break;
        case QNetworkReply::NetworkSessionFailedError:
            errorMsg = tr("网络会话失败");
            break;
        case QNetworkReply::BackgroundRequestNotAllowedError:
            errorMsg = tr("后台请求不被允许");
            break;
        case QNetworkReply::TooManyRedirectsError:
            errorMsg = tr("重定向次数过多");
            break;
        case QNetworkReply::InsecureRedirectError:
            errorMsg = tr("不安全的重定向");
            break;
        case QNetworkReply::ProxyConnectionRefusedError:
            errorMsg = tr("代理连接被拒绝");
            break;
        case QNetworkReply::ProxyConnectionClosedError:
            errorMsg = tr("代理连接关闭");
            break;
        case QNetworkReply::ProxyNotFoundError:
            errorMsg = tr("代理未找到");
            break;
        case QNetworkReply::ProxyTimeoutError:
            errorMsg = tr("代理超时");
            break;
        case QNetworkReply::ProxyAuthenticationRequiredError:
            errorMsg = tr("代理需要认证");
            break;
        case QNetworkReply::ContentAccessDenied:
            errorMsg = tr("内容访问被拒绝");
            break;
        case QNetworkReply::ContentOperationNotPermittedError:
            errorMsg = tr("内容操作不被允许");
            break;
        case QNetworkReply::ContentNotFoundError:
            errorMsg = tr("内容未找到");
            break;
        case QNetworkReply::AuthenticationRequiredError:
            errorMsg = tr("需要认证");
            break;
        case QNetworkReply::ContentReSendError:
            errorMsg = tr("内容重发错误");
            break;
        case QNetworkReply::ContentConflictError:
            errorMsg = tr("内容冲突");
            break;
        case QNetworkReply::ContentGoneError:
            errorMsg = tr("内容已消失");
            break;
        case QNetworkReply::InternalServerError:
            errorMsg = tr("内部服务器错误");
            break;
        case QNetworkReply::OperationNotImplementedError:
            errorMsg = tr("操作未实现");
            break;
        case QNetworkReply::ServiceUnavailableError:
            errorMsg = tr("服务不可用");
            break;
        case QNetworkReply::ProtocolUnknownError:
            errorMsg = tr("未知协议");
            break;
        case QNetworkReply::ProtocolInvalidOperationError:
            errorMsg = tr("协议操作无效");
            break;
        case QNetworkReply::UnknownNetworkError:
            errorMsg = tr("未知网络错误");
            break;
        case QNetworkReply::UnknownProxyError:
            errorMsg = tr("未知代理错误");
            break;
        case QNetworkReply::UnknownContentError:
            errorMsg = tr("未知内容错误");
            break;
        case QNetworkReply::ProtocolFailure:
            errorMsg = tr("协议失败");
            break;
        case QNetworkReply::UnknownServerError:
            errorMsg = tr("未知服务器错误");
            break;
        default:
            errorMsg = tr("未知错误: %1").arg(static_cast<int>(error));
            break;
    }
    
    qDebug() << "ConferenceWindow: 网络错误:" << errorMsg << "(错误代码:" << static_cast<int>(error) << ")";
    
    // 如果正在加载，根据错误类型决定是否重连
    if (m_isLoading) {
        // 对于某些可恢复的错误，尝试重连
        bool shouldRetry = false;
        switch (error) {
            case QNetworkReply::TimeoutError:
            case QNetworkReply::TemporaryNetworkFailureError:
            case QNetworkReply::NetworkSessionFailedError:
            case QNetworkReply::RemoteHostClosedError:
                shouldRetry = true;
                break;
            default:
                shouldRetry = false;
                break;
        }
        
        if (shouldRetry && m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
            qDebug() << "ConferenceWindow: 可恢复的网络错误，尝试重连";
            onConnectionTimeout();
        } else {
            m_isLoading = false;
            showLoadingIndicator(false);
            showError(errorMsg);
            emit conferenceLoadFailed(errorMsg);
        }
    }
}