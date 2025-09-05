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
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEnginePermission>
#include <QWebChannel>
#include <QTimer>
#include <QLabel>
#include <QWidget>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrlQuery>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QRegularExpression>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

/**
 * @brief ConferenceWindow构造函数
 * @param parent 父窗口
 */
ConferenceWindow::ConferenceWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_webView(nullptr)
    , m_webPage(nullptr)
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
    , m_connectionTimer(nullptr)
    , m_reconnectTimer(nullptr)
    , m_networkManager(nullptr)
    , m_configManager(ConfigurationManager::instance())
    , m_jitsiAPI(nullptr)
    , m_networkDiagnostics(nullptr)
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
    , m_lastMemoryUsage(0)
    , m_peakMemoryUsage(0)
    , m_memoryCleanupCount(0)
    , m_reconnectAttempts(0)
{
    // 初始化组件
    initializeUI();
    initializeToolbar();
    initializeWebEngine();
    
    // 创建定时器
    m_connectionTimer = new QTimer(this);
    m_connectionTimer->setSingleShot(true);
    connect(m_connectionTimer, &QTimer::timeout, this, &ConferenceWindow::onConnectionTimeout);
    
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &ConferenceWindow::onReconnectTimer);
    
    // 创建内存监控定时器
    m_memoryMonitorTimer = new QTimer(this);
    m_memoryMonitorTimer->setInterval(MEMORY_MONITOR_INTERVAL);
    connect(m_memoryMonitorTimer, &QTimer::timeout, this, &ConferenceWindow::onMemoryMonitorTimer);
    m_memoryMonitorTimer->start(); // 立即开始内存监控
    
    // 创建网络管理器
    m_networkManager = new QNetworkAccessManager(this);
    
    // 创建Jitsi Meet API
    m_jitsiAPI = new JitsiMeetAPI(this);
    
    // 创建网络诊断工具
    m_networkDiagnostics = new NetworkDiagnostics(this);
    connect(m_networkDiagnostics, &NetworkDiagnostics::diagnosisCompleted,
            this, &ConferenceWindow::onNetworkDiagnosticsCompleted);
    connect(m_networkDiagnostics, &NetworkDiagnostics::diagnosisProgress,
            this, [this](int progress, const QString& currentStep) {
                qDebug() << "ConferenceWindow: 网络诊断进度:" << progress << "% -" << currentStep;
            });
    
    // 连接JitsiMeetAPI信号槽
    connect(m_jitsiAPI, &JitsiMeetAPI::roomJoined, this, 
            [this](const QString& roomName, bool success) {
                Q_UNUSED(roomName);
                if (success) {
                    onConferenceJoined();
                }
            });
    connect(m_jitsiAPI, &JitsiMeetAPI::roomLeft, this, 
            [this](const QString& roomName) {
                Q_UNUSED(roomName);
                onConferenceLeft();
            });
    connect(m_jitsiAPI, &JitsiMeetAPI::serverConnected, this, &ConferenceWindow::onApiConnected);
    connect(m_jitsiAPI, &JitsiMeetAPI::serverDisconnected, this, &ConferenceWindow::onApiDisconnected);
    connect(m_jitsiAPI, &JitsiMeetAPI::participantsUpdated, this, 
            [this](const QString& roomName, const QJsonArray& participants) {
                Q_UNUSED(roomName); // 忽略roomName参数
                // 处理参与者更新逻辑
                qDebug() << "ConferenceWindow: 参与者列表更新，当前参与者数量:" << participants.size();
            });
    // 使用lambda表达式适配chatMessageReceived信号的参数差异
    connect(m_jitsiAPI, &JitsiMeetAPI::chatMessageReceived, this, 
            [this](const QString& roomName, const QString& senderId, const QString& message, qint64 timestamp) {
                Q_UNUSED(roomName); // 忽略roomName参数
                onChatMessageReceived(senderId, message, timestamp);
            });
    // 连接API错误信号
    connect(m_jitsiAPI, &JitsiMeetAPI::apiError, this, 
            [this](const QString& operation, const QString& error, const QJsonObject& details) {
                Q_UNUSED(details);
                qDebug() << "ConferenceWindow: API错误 -" << operation << ":" << error;
                onApiError(error);
            });
    
    // 恢复窗口状态
    restoreWindowState();
}

/**
 * @brief 内存监控定时器槽函数
 * 定期检查内存使用情况，当超过阈值时触发清理
 */
void ConferenceWindow::onMemoryMonitorTimer()
{
    qint64 currentMemory = 0;
    
#ifdef Q_OS_WIN
    // Windows平台获取内存使用量
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        currentMemory = pmc.WorkingSetSize / (1024 * 1024); // 转换为MB
    }
#else
    // Linux/macOS平台获取内存使用量
    QFile file("/proc/self/status");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString line;
        while (stream.readLineInto(&line)) {
            if (line.startsWith("VmRSS:")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                if (parts.size() >= 2) {
                    currentMemory = parts[1].toLongLong() / 1024; // 转换为MB
                }
                break;
            }
        }
    }
#endif
    
    // 更新内存统计
    if (currentMemory > 0) {
        m_lastMemoryUsage = currentMemory;
        if (currentMemory > m_peakMemoryUsage) {
            m_peakMemoryUsage = currentMemory;
        }
        
        qDebug() << "ConferenceWindow: 当前内存使用:" << currentMemory << "MB, 峰值:" << m_peakMemoryUsage << "MB";
        
        // 检查是否需要清理内存
        if (currentMemory > MEMORY_THRESHOLD) {
            qDebug() << "ConferenceWindow: 内存使用超过阈值(" << MEMORY_THRESHOLD << "MB)，开始清理";
            performMemoryCleanup();
        }
    }
}

/**
 * @brief 执行内存清理操作
 * 包括WebEngine缓存清理、JavaScript垃圾回收等
 */
void ConferenceWindow::performMemoryCleanup()
{
    if (!m_webView || !m_webPage) {
        return;
    }
    
    qDebug() << "ConferenceWindow: 开始执行内存清理，第" << (m_memoryCleanupCount + 1) << "次";
    
    // 1. 清理WebEngine缓存
    QWebEngineProfile* profile = m_webPage->profile();
    if (profile) {
        // 清理HTTP缓存
        profile->clearHttpCache();
        
        // 清理所有网站数据（包括localStorage、sessionStorage等）
        profile->clearAllVisitedLinks();
        
        qDebug() << "ConferenceWindow: WebEngine缓存已清理";
    }
    
    // 2. 执行JavaScript垃圾回收
    QString gcScript = R"(
        // 强制执行垃圾回收
        if (window.gc) {
            window.gc();
        }
        
        // 清理可能的内存泄漏
        if (window.APP && window.APP.store) {
            // 清理Redux store中的非必要数据
            try {
                window.APP.store.dispatch({
                    type: 'CLEAR_CACHE'
                });
            } catch(e) {
                console.log('清理store缓存失败:', e);
            }
        }
        
        // 清理WebRTC连接缓存
        if (window.JitsiMeetJS) {
            try {
                // 清理音视频轨道缓存
                const conference = window.APP.conference;
                if (conference && conference._room) {
                    conference._room.cleanupLocalTracks();
                }
            } catch(e) {
                console.log('清理WebRTC缓存失败:', e);
            }
        }
        
        'memory_cleanup_completed';
    )";
    
    executeJavaScript(gcScript, [this](const QVariant& result) {
        qDebug() << "ConferenceWindow: JavaScript垃圾回收完成:" << result.toString();
    });
    
    // 3. Qt对象垃圾回收
    QCoreApplication::processEvents();
    
    // 4. 清理网络缓存
    if (m_networkManager) {
        m_networkManager->clearAccessCache();
        qDebug() << "ConferenceWindow: 网络缓存已清理";
    }
    
    // 5. 如果内存使用仍然很高，考虑重新加载页面
    if (m_lastMemoryUsage > MEMORY_THRESHOLD * 1.5) {
        qDebug() << "ConferenceWindow: 内存使用过高，重新加载页面";
        
        // 保存当前状态
        QString currentUrl = m_currentUrl;
        QString currentRoom = m_currentRoom;
        QString currentServer = m_currentServer;
        QString displayName = m_displayName;
        
        // 重新加载
        if (!currentUrl.isEmpty()) {
            QTimer::singleShot(1000, [this, currentUrl, displayName]() {
                loadConference(currentUrl, displayName);
            });
        }
    }
    
    m_memoryCleanupCount++;
    qDebug() << "ConferenceWindow: 内存清理完成，总清理次数:" << m_memoryCleanupCount;
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
    
}

/**
 * @brief 初始化WebEngine
 */
void ConferenceWindow::initializeWebEngine()
{
    qDebug() << "ConferenceWindow: 初始化WebEngine";
    
    // 创建Web容器
    m_webContainer = new QWidget(this);
    QVBoxLayout* containerLayout = new QVBoxLayout(m_webContainer);
    
    // 创建Web视图（使用QWebEngineView）
    m_webView = new QWebEngineView(this);
    
    // 获取Web页面
    m_webPage = m_webView->page();
    
    // 设置WebEngine配置
    setupWebEngineSettings();
    
    // 连接WebEngine信号
    connect(m_webView, &QWebEngineView::loadStarted, this, &ConferenceWindow::onLoadStarted);
    connect(m_webView, &QWebEngineView::loadProgress, this, &ConferenceWindow::onLoadProgress);
    connect(m_webView, &QWebEngineView::loadFinished, this, &ConferenceWindow::onLoadFinished);
    connect(m_webView, &QWebEngineView::titleChanged, this, &ConferenceWindow::onTitleChanged);
    connect(m_webView, &QWebEngineView::urlChanged, this, &ConferenceWindow::onUrlChanged);
    
    // 添加到容器布局
    containerLayout->addWidget(m_webView);
    
    // 初始化JavaScript桥接
    initializeJavaScriptBridge();
    
    // 添加到主布局
    m_mainLayout->addWidget(m_webContainer);
}

// 删除重复的setupWebEngineSettings函数定义，保留后面的正确版本

/**
 * @brief 初始化工具栏
 */
void ConferenceWindow::initializeToolbar()
{
    
    // 创建工具栏
    m_toolbar = addToolBar(tr("会议控制"));
    m_toolbar->setObjectName("conferenceToolbar");  // 设置对象名称以避免警告
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
 * @brief 设置WebEngine配置
 */
void ConferenceWindow::setupWebEngineSettings()
{
    qDebug() << "ConferenceWindow: 设置WebEngine配置";
    
    if (!m_webView) {
        return;
    }
    
    QWebEngineSettings* settings = m_webView->settings();
    
    // 启用JavaScript
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    
    // 启用本地存储
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    
    // 启用WebGL
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    
    // 启用媒体访问
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    
    // 启用自动播放媒体
    settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
    
    // 允许运行不安全内容（用于开发环境）
    settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, true);
    
    // 启用全屏支持
    settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    
    // 允许本地内容访问远程URL
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    
    // 启用WebRTC功能
    settings->setAttribute(QWebEngineSettings::WebRTCPublicInterfacesOnly, false);
    
    // === 性能优化设置 ===
    // 启用硬件加速
    settings->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);
    
    // 启用GPU光栅化
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    
    // 启用DNS预取
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    
    // 启用触摸图标
    settings->setAttribute(QWebEngineSettings::TouchIconsEnabled, false); // 禁用以节省内存
    
    // 启用焦点在页面加载时
    settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    
    // 启用PDF查看器插件
    settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, false); // 禁用以节省内存
    
    // 设置默认字体大小（优化渲染性能）
    settings->setFontSize(QWebEngineSettings::DefaultFontSize, 14);
    settings->setFontSize(QWebEngineSettings::DefaultFixedFontSize, 13);
    settings->setFontSize(QWebEngineSettings::MinimumFontSize, 8);
    settings->setFontSize(QWebEngineSettings::MinimumLogicalFontSize, 6);
    
    // 设置用户代理 - 使用最新Chrome用户代理以确保兼容性
    QString userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";
    m_webView->page()->profile()->setHttpUserAgent(userAgent);
    
    // === 配置WebEngine Profile以优化性能 ===
    QWebEngineProfile* profile = m_webView->page()->profile();
    
    // 设置缓存策略
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    profile->setHttpCacheMaximumSize(100 * 1024 * 1024); // 100MB缓存
    
    // 设置持久化Cookie策略
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    
    // 启用拼写检查（可选，可能影响性能）
    profile->setSpellCheckEnabled(false); // 禁用以提升性能
    
    // 设置下载路径
    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    profile->setDownloadPath(downloadPath);
    
    // 设置状态显示
    if (m_statusDisplay) {
        m_statusDisplay->setText("Jitsi Meet 客户端已就绪（性能优化已启用）");
    }
    
    qDebug() << "ConferenceWindow: WebEngine性能优化配置完成";
}

/**
 * @brief 初始化JavaScript桥接
 */
void ConferenceWindow::initializeJavaScriptBridge()
{
    qDebug() << "ConferenceWindow: 初始化JavaScript桥接";
    
    if (!m_webView) {
        return;
    }
    
    // 注意：javaScriptConsoleMessage是受保护的信号，无法直接连接
    // 如需处理JavaScript控制台消息，需要创建QWebEnginePage的子类
    
    // 连接新的权限请求信号（推荐使用）
    connect(m_webView->page(), &QWebEnginePage::permissionRequested,
            this, &ConferenceWindow::onPermissionRequested);
    
    // 注册JavaScript桥接对象
    QWebChannel* channel = new QWebChannel(this);
    channel->registerObject(QStringLiteral("qtBridge"), this);
    m_webView->page()->setWebChannel(channel);
}

/**
 * @brief 注入JavaScript代码
 */
void ConferenceWindow::injectJavaScript()
{
    qDebug() << "ConferenceWindow: 注入JavaScript代码";
    
    if (!m_webView) {
        return;
    }
    
    // 将长JavaScript代码分割成多个部分以避免编译器字符串长度限制
    QString script1 = R"(
        // 创建增强的Qt与Jitsi Meet桥接对象 - 参考Electron版本的JitsiMeetExternalAPI
        window.qtJitsiMeetAPI = {
            // 事件系统 - 模拟JitsiMeetExternalAPI的事件处理
            _eventHandlers: {},
            _isReady: false,
            _conference: null,
            
            // 事件监听器管理 - 兼容JitsiMeetExternalAPI接口
            on: function(eventName, handler) {
                if (!this._eventHandlers[eventName]) {
                    this._eventHandlers[eventName] = [];
                }
                this._eventHandlers[eventName].push(handler);
                console.log('Qt: 注册事件监听器:', eventName);
            },
            
            off: function(eventName, handler) {
                if (this._eventHandlers[eventName]) {
                    const index = this._eventHandlers[eventName].indexOf(handler);
                    if (index > -1) {
                        this._eventHandlers[eventName].splice(index, 1);
                        console.log('Qt: 移除事件监听器:', eventName);
                    }
                }
            },
            
            // 触发事件 - 内部使用
            _emit: function(eventName, data) {
                console.log('Qt: 触发事件:', eventName, data);
                if (this._eventHandlers[eventName]) {
                    this._eventHandlers[eventName].forEach(function(handler) {
                        try {
                            handler(data);
                        } catch (e) {
                            console.error('Qt: 事件处理器错误:', e);
                        }
                    });
                }
                // 同时通知Qt桥接
                if (window.qtBridge) {
                    window.qtBridge.onJitsiEvent(eventName, JSON.stringify(data || {}));
                }
            },
            
            // 命令执行系统 - 参考Electron版本的executeCommand模式
            executeCommand: function(command, ...args) {
                try {
                    console.log('Qt: 执行命令:', command, args);
                    
                    // 优先使用APP.conference API
                    if (window.APP && window.APP.conference) {
                        switch (command) {
                            case 'toggleAudio':
                                window.APP.conference.toggleAudioMuted();
                                return true;
                            case 'toggleVideo':
                                window.APP.conference.toggleVideoMuted();
                                return true;
                            case 'toggleShareScreen':
                                window.APP.conference.toggleScreenSharing();
                                return true;
                            case 'hangup':
                                window.APP.conference.hangup();
                                return true;
                            case 'sendChatMessage':
                                if (args[0]) {
                                    window.APP.conference.sendTextMessage(args[0]);
                                    return true;
                                }
                                break;
                            case 'setDisplayName':
                                if (args[0]) {
                                    window.APP.conference.changeLocalDisplayName(args[0]);
                                    return true;
                                }
                                break;
                            case 'muteEveryone':
                                // 这个功能需要主持人权限
                                if (window.APP.conference.isModerator && window.APP.conference.isModerator()) {
                                    window.APP.conference.muteParticipant(window.APP.conference.getMyUserId());
                                    return true;
                                }
                                break;
                            default:
                                console.warn('Qt: 未知命令:', command);
                                return false;
                        }
                    } else if (window.parent && window.parent.postMessage) {
                        // 回退到postMessage API
                        window.parent.postMessage({
                            type: 'jitsi_meet_command',
                            command: command,
                            args: args
                        }, '*');
                        return true;
                    }
                } catch (e) {
                    console.error('Qt: executeCommand错误:', e);
                }
                return false;
            },
            
            // 便捷方法 - 兼容JitsiMeetExternalAPI接口
            toggleAudio: function() { return this.executeCommand('toggleAudio'); },
            toggleVideo: function() { return this.executeCommand('toggleVideo'); },
            toggleShareScreen: function() { return this.executeCommand('toggleShareScreen'); },
            hangup: function() { return this.executeCommand('hangup'); },
            sendChatMessage: function(message) { return this.executeCommand('sendChatMessage', message); },
            setDisplayName: function(name) { return this.executeCommand('setDisplayName', name); }
        };
    )";
    
    QString script2 = R"(
        
        // 保持向后兼容性
        window.qtJitsiMeet = {
            // 会议控制方法
            toggleMute: function() {
                try {
                    if (window.APP && window.APP.conference) {
                        window.APP.conference.toggleAudioMuted();
                        return true;
                    }
                    // 尝试使用iframe API
                    if (window.parent && window.parent.postMessage) {
                        window.parent.postMessage({
                            type: 'jitsi_meet_command',
                            command: 'toggleAudio'
                        }, '*');
                        return true;
                    }
                } catch (e) {
                    console.error('Qt: toggleMute error:', e);
                }
                return false;
            },
            
            toggleCamera: function() {
                try {
                    if (window.APP && window.APP.conference) {
                        window.APP.conference.toggleVideoMuted();
                        return true;
                    }
                    // 尝试使用iframe API
                    if (window.parent && window.parent.postMessage) {
                        window.parent.postMessage({
                            type: 'jitsi_meet_command',
                            command: 'toggleVideo'
                        }, '*');
                        return true;
                    }
                } catch (e) {
                    console.error('Qt: toggleCamera error:', e);
                }
                return false;
            },
            
            toggleScreenShare: function() {
                try {
                    if (window.APP && window.APP.conference) {
                        window.APP.conference.toggleScreenSharing();
                        return true;
                    }
                    // 尝试使用iframe API
                    if (window.parent && window.parent.postMessage) {
                        window.parent.postMessage({
                            type: 'jitsi_meet_command',
                            command: 'toggleShareScreen'
                        }, '*');
                        return true;
                    }
                } catch (e) {
                    console.error('Qt: toggleScreenShare error:', e);
                }
                return false;
            },
            
            toggleChat: function() {
                try {
                    if (window.APP && window.APP.UI) {
                        window.APP.UI.toggleChat();
                        return true;
                    }
                    // 尝试使用iframe API
                    if (window.parent && window.parent.postMessage) {
                        window.parent.postMessage({
                            type: 'jitsi_meet_command',
                            command: 'toggleChat'
                        }, '*');
                        return true;
                    }
                } catch (e) {
                    console.error('Qt: toggleChat error:', e);
                }
                return false;
            },
            
            leaveConference: function() {
                try {
                    if (window.APP && window.APP.conference) {
                        window.APP.conference.hangup();
                        return true;
                    }
                    // 尝试使用iframe API
                    if (window.parent && window.parent.postMessage) {
                        window.parent.postMessage({
                            type: 'jitsi_meet_command',
                            command: 'hangup'
                        }, '*');
                        return true;
                    }
                } catch (e) {
                    console.error('Qt: leaveConference error:', e);
                }
                return false;
            },
            
            setDisplayName: function(name) {
                try {
                    if (window.APP && window.APP.conference) {
                        window.APP.conference.changeLocalDisplayName(name);
                        return true;
                    }
                    // 尝试使用iframe API
                    if (window.parent && window.parent.postMessage) {
                        window.parent.postMessage({
                            type: 'jitsi_meet_command',
                            command: 'displayName',
                            displayName: name
                        }, '*');
                        return true;
                    }
                } catch (e) {
                    console.error('Qt: setDisplayName error:', e);
                }
                return false;
            },
            
            sendChatMessage: function(message) {
                try {
                    if (window.APP && window.APP.conference) {
                        window.APP.conference.sendTextMessage(message);
                        return true;
                    }
                    // 尝试使用iframe API
                    if (window.parent && window.parent.postMessage) {
                        window.parent.postMessage({
                            type: 'jitsi_meet_command',
                            command: 'sendChatMessage',
                            message: message
                        }, '*');
                        return true;
                    }
                } catch (e) {
                    console.error('Qt: sendChatMessage error:', e);
                }
                return false;
            },
            
            // 获取会议状态
            getConferenceState: function() {
                try {
                    if (window.APP && window.APP.conference) {
                        return {
                            isJoined: window.APP.conference.isJoined(),
                            participantCount: window.APP.conference.getParticipantCount(),
                            isAudioMuted: window.APP.conference.isLocalAudioMuted(),
                            isVideoMuted: window.APP.conference.isLocalVideoMuted()
                        };
                    }
                } catch (e) {
                    console.error('Qt: getConferenceState error:', e);
                }
                return null;
            }
        };
        
        // 增强的Jitsi Meet事件监听器设置 - 参考Electron版本的事件映射
    )";
    
    QString script3 = R"(
        function setupJitsiEventListeners() {
            try {
                console.log('Qt: 设置Jitsi Meet事件监听器');
                
                // 监听iframe API事件 - 兼容外部API模式
                window.addEventListener('message', function(event) {
                    if (event.data && event.data.type === 'jitsi_meet_event') {
                        handleJitsiEvent(event.data);
                    }
                });
                
                // 如果有直接的APP对象，设置内部事件监听
                if (window.APP && window.APP.conference) {
                    console.log('Qt: 检测到APP.conference，设置内部事件监听');
                    
                    // 会议加入事件 - 映射到videoConferenceJoined
                    window.APP.conference.addConferenceListener('conference.joined', function(conferenceInfo) {
                        console.log('Qt: 会议已加入');
                        const eventData = {
                            roomName: conferenceInfo ? conferenceInfo.roomName : '',
                            id: window.APP.conference.getMyUserId ? window.APP.conference.getMyUserId() : '',
                            displayName: window.APP.conference.getLocalDisplayName ? window.APP.conference.getLocalDisplayName() : ''
                        };
                        window.qtJitsiMeetAPI._emit('videoConferenceJoined', eventData);
                        
                        // 通知Qt应用程序会议已加入
                        if (window.qtBridge) {
                            window.qtBridge.onConferenceJoined();
                        }
                    });
                    
                    // 会议离开事件 - 映射到videoConferenceLeft
                    window.APP.conference.addConferenceListener('conference.left', function() {
                        console.log('Qt: 会议已离开');
                        window.qtJitsiMeetAPI._emit('videoConferenceLeft', {});
                        
                        // 通知Qt应用程序会议已离开
                        if (window.qtBridge) {
                            window.qtBridge.onConferenceLeft();
                        }
                    });
                    
                    // 添加会议失败事件监听
                    window.APP.conference.addConferenceListener('conference.failed', function(error) {
                        console.log('Qt: 会议失败:', error);
                        window.qtJitsiMeetAPI._emit('videoConferenceFailed', {
                            error: error
                        });
                        
                        // 通知Qt应用程序会议失败
                        if (window.qtBridge) {
                            window.qtBridge.onConferenceFailed(error);
                        }
                    });
                    
                    // 添加会议错误事件监听
                    window.APP.conference.addConferenceListener('conference.error', function(error) {
                        console.log('Qt: 会议错误:', error);
                        window.qtJitsiMeetAPI._emit('videoConferenceError', {
                            error: error
                        });
                    });
                    
                    // 参与者加入事件 - 映射到participantJoined
                    window.APP.conference.addConferenceListener('participant.joined', function(id, user) {
                        const displayName = user && user.getDisplayName ? user.getDisplayName() : '';
                        console.log('Qt: 参与者加入:', id, displayName);
                        window.qtJitsiMeetAPI._emit('participantJoined', {
                            id: id,
                            displayName: displayName
                        });
                    });
                    
                    // 参与者离开事件 - 映射到participantLeft
                    window.APP.conference.addConferenceListener('participant.left', function(id, user) {
                        const displayName = user && user.getDisplayName ? user.getDisplayName() : '';
                        console.log('Qt: 参与者离开:', id, displayName);
                        window.qtJitsiMeetAPI._emit('participantLeft', {
                            id: id,
                            displayName: displayName
                        });
                    });
                    
                    // 聊天消息接收事件 - 映射到incomingMessage
                    window.APP.conference.addConferenceListener('message.received', function(id, text, ts) {
                        console.log('Qt: 收到聊天消息:', text);
                        window.qtJitsiMeetAPI._emit('incomingMessage', {
                            from: id,
                            message: text,
                            timestamp: ts
                        });
                    });
                    
                    // 音频静音状态变化 - 映射到audioMuteStatusChanged
                    window.APP.conference.addConferenceListener('audio.muted', function(muted) {
                        console.log('Qt: 音频静音状态变化:', muted);
                        window.qtJitsiMeetAPI._emit('audioMuteStatusChanged', { muted: muted });
                    });
                    
                    // 视频静音状态变化 - 映射到videoMuteStatusChanged
                    window.APP.conference.addConferenceListener('video.muted', function(muted) {
                        console.log('Qt: 视频静音状态变化:', muted);
                        window.qtJitsiMeetAPI._emit('videoMuteStatusChanged', { muted: muted });
                    });
                    
                    // 屏幕共享状态变化 - 映射到screenSharingStatusChanged
                    window.APP.conference.addConferenceListener('screen.sharing.toggled', function(isSharing) {
                        console.log('Qt: 屏幕共享状态变化:', isSharing);
                        window.qtJitsiMeetAPI._emit('screenSharingStatusChanged', { on: isSharing });
                    });
                    
                    // 音频级别变化事件
                    window.APP.conference.addConferenceListener('track.audioLevelsChanged', function(audioLevels) {
                        window.qtJitsiMeetAPI._emit('audioLevelsChanged', { audioLevels: audioLevels });
                    });
                    
                    // 视频类型变化事件
                    window.APP.conference.addConferenceListener('track.videoTypeChanged', function(participantId, videoType) {
                        window.qtJitsiMeetAPI._emit('videoTypeChanged', {
                            participantId: participantId,
                            videoType: videoType
                        });
                    });
                    
                    // 会议准备就绪事件
                    window.APP.conference.addConferenceListener('conference.ready', function() {
                        console.log('Qt: 会议准备就绪');
                        window.qtJitsiMeetAPI._isReady = true;
                        window.qtJitsiMeetAPI._emit('readyToClose', {});
                    });
                }
            } catch (e) {
                console.error('Qt: setupJitsiEventListeners错误:', e);
            }
        }
        
        // 处理iframe API事件 - 增强版本，支持新的事件系统
        function handleJitsiEvent(eventData) {
            try {
                console.log('Qt: 处理iframe API事件:', eventData);
                
                // 确定事件名称（兼容不同的事件格式）
                const eventName = eventData.event || eventData.name || eventData.type;
                const eventPayload = eventData.data || eventData;
                
                // 首先触发新的事件系统
                if (eventName && window.qtJitsiMeetAPI) {
                    window.qtJitsiMeetAPI._emit(eventName, eventPayload);
                }
                
                // 为了向后兼容，保留原有的qtBridge调用
                if (!window.qtBridge) return;
                
                switch (eventName) {
                    case 'videoConferenceJoined':
                        console.log('Qt: 通过iframe API加入会议');
                        window.qtBridge.onConferenceJoined();
                        break;
                    case 'videoConferenceLeft':
                        console.log('Qt: 通过iframe API离开会议');
                        window.qtBridge.onConferenceLeft();
                        break;
                    case 'videoConferenceFailed':
                        console.log('Qt: 通过iframe API会议失败:', eventPayload.error);
                        window.qtBridge.onConferenceFailed(eventPayload.error || 'Unknown error');
                        break;
                    case 'readyToClose':
                        console.log('Qt: 通过iframe API准备关闭');
                        window.qtBridge.onReadyToClose();
                        break;
                    case 'participantJoined':
                        console.log('Qt: 参与者通过iframe API加入:', eventPayload.id);
                        window.qtBridge.onParticipantJoined(eventPayload.id, eventPayload.displayName || '');
                        break;
                    case 'participantLeft':
                        console.log('Qt: 参与者通过iframe API离开:', eventPayload.id);
                        window.qtBridge.onParticipantLeft(eventPayload.id, eventPayload.displayName || '');
                        break;
                    case 'audioMuteStatusChanged':
                        console.log('Qt: 音频静音状态变化 (iframe):', eventPayload.muted);
                        window.qtBridge.onAudioMuteChanged(eventPayload.muted);
                        break;
                    case 'videoMuteStatusChanged':
                        console.log('Qt: 视频静音状态变化 (iframe):', eventPayload.muted);
                        window.qtBridge.onVideoMuteChanged(eventPayload.muted);
                        break;
                    case 'screenSharingStatusChanged':
                        console.log('Qt: 屏幕共享状态变化 (iframe):', eventPayload.on);
                        window.qtBridge.onScreenShareChanged(eventPayload.on);
                        break;
                    case 'incomingMessage':
                        console.log('Qt: 通过iframe API收到聊天消息:', eventPayload.message);
                        window.qtBridge.onChatMessageReceived(eventPayload.from, eventPayload.message, eventPayload.timestamp || Date.now());
                        break;
                    case 'readyToClose':
                        console.log('Qt: 会议准备关闭 (iframe)');
                        if (window.qtBridge.onReadyToClose) {
                            window.qtBridge.onReadyToClose();
                        }
                        break;
                    default:
                        console.log('Qt: 未处理的Jitsi事件:', eventName);
                }
            } catch (e) {
                console.error('Qt: handleJitsiEvent错误:', e);
            }
        }
        
        // 初始化事件监听器
    )";
    
    QString script4 = R"(
        console.log('Qt: 开始初始化事件监听器');
        setupJitsiEventListeners();
        
        // 延迟重试设置事件监听器，以防APP对象还未加载
        setTimeout(function() {
            console.log('Qt: 2秒后重试设置事件监听器');
            setupJitsiEventListeners();
        }, 2000);
        
        setTimeout(function() {
            console.log('Qt: 5秒后重试设置事件监听器');
            setupJitsiEventListeners();
        }, 5000);
        
        // 添加全局错误处理
        window.addEventListener('error', function(event) {
            console.error('Qt: JavaScript错误:', event.error);
            if (window.qtBridge) {
                window.qtBridge.onJavaScriptError(event.error.toString());
            }
        });
        
        // 添加未处理的Promise拒绝处理
        window.addEventListener('unhandledrejection', function(event) {
            console.error('Qt: 未处理的Promise拒绝:', event.reason);
            if (window.qtBridge) {
                window.qtBridge.onJavaScriptError('Promise rejection: ' + event.reason);
            }
        });
        
        // 定期检查会议状态并更新
        setInterval(function() {
            try {
                // 检查新API系统的状态
                if (window.qtJitsiMeetAPI && window.qtBridge) {
                    const state = {
                        isReady: window.qtJitsiMeetAPI._isReady,
                        eventListeners: Object.keys(window.qtJitsiMeetAPI._eventListeners).length,
                        hasAPP: !!window.APP,
                        hasConference: !!(window.APP && window.APP.conference)
                    };
                    window.qtBridge.onConferenceStateUpdate(JSON.stringify(state));
                }
                
                // 兼容旧系统
                if (window.qtJitsiMeet && window.qtBridge) {
                    var oldState = window.qtJitsiMeet.getConferenceState();
                    if (oldState) {
                        window.qtBridge.onConferenceStateUpdate(JSON.stringify(oldState));
                    }
                }
            } catch (e) {
                // 静默处理，避免日志污染
            }
        }, 5000);
        
        // 检测Jitsi Meet是否已加载
        function checkJitsiMeetLoaded() {
            if (window.APP || document.querySelector('[data-jitsi-meet-loaded]')) {
                console.log('Qt: 检测到Jitsi Meet已加载');
                window.qtJitsiMeetAPI._isReady = true;
                window.qtJitsiMeetAPI._emit('readyToClose', {});
                if (window.qtBridge) {
                    window.qtBridge.onJitsiMeetLoaded();
                }
                return true;
            }
            return false;
        }
        
        // 立即检查
        if (!checkJitsiMeetLoaded()) {
            // 如果未加载，定期检查
            var loadCheckInterval = setInterval(function() {
                if (checkJitsiMeetLoaded()) {
                    clearInterval(loadCheckInterval);
                }
            }, 1000);
            
            // 10秒后停止检查
            setTimeout(function() {
                clearInterval(loadCheckInterval);
            }, 10000);
        }
        
        console.log('Qt: JavaScript bridge initialized');
    )";
    
    // 将所有JavaScript代码片段连接起来
    QString fullScript = script1 + script2 + script3 + script4;
    
    m_webView->page()->runJavaScript(fullScript, [this](const QVariant &result) {
        Q_UNUSED(result)
        qDebug() << "ConferenceWindow: JavaScript代码注入完成";
    });
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
    
    // 参考Electron版本添加配置参数
    bool audioMuted = m_configManager->getValue("defaultAudioMuted", false).toBool();
    bool videoMuted = m_configManager->getValue("defaultVideoMuted", false).toBool();
    
    // 添加配置覆盖参数
    query.addQueryItem("config.startWithAudioMuted", audioMuted ? "true" : "false");
    query.addQueryItem("config.startWithVideoMuted", videoMuted ? "true" : "false");
    query.addQueryItem("config.prejoinConfig.enabled", "true"); // 使用prejoinConfig.enabled而不是prejoinPageEnabled
    query.addQueryItem("config.disableDeepLinking", "true");
    query.addQueryItem("config.enableCalendarIntegration", "false");
    
    // 添加界面配置覆盖参数
    query.addQueryItem("interfaceConfig.SHOW_CHROME_EXTENSION_BANNER", "false");
    
    // 添加语言参数
    query.addQueryItem("lang", QLocale::system().name().split('_').first());
    
    if (!query.isEmpty()) {
        QUrl qurl(fullUrl);
        qurl.setQuery(query);
        fullUrl = qurl.toString();
    }
    
    // 开始加载
    m_isLoading = true;
    m_connectionTimer->start(CONNECTION_TIMEOUT);
    
    // 使用QWebEngineView加载实际的Jitsi Meet页面
    qDebug() << "ConferenceWindow: 开始加载会议URL:" << fullUrl;
    m_webView->load(QUrl(fullUrl));
    
    // 初始化WebEngine（如果还未初始化）
    if (!m_webView->page()) {
        qDebug() << "ConferenceWindow: WebEngine页面未初始化，重新初始化";
        initializeWebEngine();
    }
    
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
        // 使用JitsiMeetAPI离开房间
        if (m_jitsiAPI) {
            qDebug() << "ConferenceWindow: 使用JitsiMeetAPI离开房间:" << m_currentRoom;
            m_jitsiAPI->leaveRoom(m_currentRoom);
            
            qDebug() << "ConferenceWindow: 使用JitsiMeetAPI断开服务器连接";
            m_jitsiAPI->disconnectFromServer();
        }
        
        // 通过JavaScript离开会议（作为备用方案）
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
}

/**
 * @brief 加入会议
 * @param roomName 房间名称
 * @param serverUrl 服务器URL
 */
void ConferenceWindow::joinConference(const QString& roomName, const QString& serverUrl)
{
    qDebug() << "加入会议:" << roomName << "服务器:" << serverUrl;
    
    // 参考Electron版本，只使用WebEngine加载会议页面，不使用REST API
    // 移除JitsiMeetAPI调用，避免双重连接导致的问题
    if (loadRoom(roomName, serverUrl)) {
        qDebug() << "ConferenceWindow: 使用WebEngine加载会议页面:" << roomName;
        show();
        raise();
        activateWindow();
    } else {
        qWarning() << "无法加载会议页面:" << roomName;
        showError(QString("无法加载会议页面: %1").arg(roomName));
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
        // 使用JitsiMeetAPI发送聊天消息
        if (m_jitsiAPI) {
            qDebug() << "ConferenceWindow: 使用JitsiMeetAPI发送聊天消息:" << message;
            m_jitsiAPI->sendChatMessage(m_currentRoom, message);
        }
        
        // 通过JavaScript发送聊天消息（作为备用方案）
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
    
    if (!m_webView) {
        qWarning() << "ConferenceWindow: WebView未初始化，无法执行JavaScript";
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
        
        // 注入JavaScript代码以建立与Jitsi Meet的桥接
        QTimer::singleShot(1000, this, [this]() {
            injectJavaScript();
            qDebug() << "ConferenceWindow: JavaScript注入完成";
        });
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
 * @brief 处理功能权限请求事件
 * @param url 请求权限的URL
 * @param feature 请求的功能
 */
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
    
    emit conferenceLeft(m_currentRoom);
}

/**
 * @brief 处理会议失败事件
 * @param error 错误信息
 */
void ConferenceWindow::onConferenceFailed(const QString& error)
{
    qDebug() << "ConferenceWindow: 会议连接失败: " << error;
    
    m_isInConference = false;
    m_participantCount = 0;
    
    updateToolbarState();
    updateWindowTitle();
    
    m_statusLabel->setText(tr("会议连接失败"));
    
    // 显示错误消息
    QMessageBox::warning(this, tr("会议连接失败"), tr("无法加入会议: %1").arg(error));
    
    // 发出会议失败信号
    emit conferenceFailed(m_currentRoom, error);
}

/**
 * @brief 处理会议准备关闭事件
 */
void ConferenceWindow::onReadyToClose()
{
    qDebug() << "ConferenceWindow: 会议准备关闭";
    
    // 如果在会议中，先离开会议
    if (m_isInConference) {
        leaveConference();
    }
    
    // 发出准备关闭信号
    emit readyToClose();
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
 * @brief 处理Jitsi Meet加载完成事件
 */
void ConferenceWindow::onJitsiMeetLoaded()
{
    qDebug() << "Jitsi Meet已加载完成";
    
    // 隐藏加载指示器
    showLoadingIndicator(false);
    
    // 更新状态
    if (m_statusLabel) {
        m_statusLabel->setText("会议已准备就绪");
    }
    
    // 注入JavaScript代码
    injectJavaScript();
    
    // 如果有显示名称，设置显示名称
    if (!m_displayName.isEmpty()) {
        setDisplayName(m_displayName);
    }
}

/**
 * @brief 处理会议状态更新事件
 * @param state 会议状态JSON对象
 */
void ConferenceWindow::onConferenceStateUpdated(const QJsonObject& state)
{
    qDebug() << "会议状态更新:" << state;
    
    // 更新参与者数量
    if (state.contains("participantCount")) {
        int count = state["participantCount"].toInt();
        m_participantCount = count;
        emit participantCountChanged(count);
    }
    
    // 更新音频状态
    if (state.contains("audioMuted")) {
        bool muted = state["audioMuted"].toBool();
        if (m_isMuted != muted) {
            m_isMuted = muted;
            onAudioMuteChanged(muted);
        }
    }
    
    // 更新视频状态
    if (state.contains("videoMuted")) {
        bool muted = state["videoMuted"].toBool();
        if (m_isCameraOff != muted) {
            m_isCameraOff = muted;
            onVideoMuteChanged(muted);
        }
    }
    
    // 更新屏幕共享状态
    if (state.contains("screenSharing")) {
        bool sharing = state["screenSharing"].toBool();
        if (m_isScreenSharing != sharing) {
            m_isScreenSharing = sharing;
            onScreenShareChanged(sharing);
        }
    }
    
    // 更新会议状态
    if (state.contains("inConference")) {
        bool inConference = state["inConference"].toBool();
        if (m_isInConference != inConference) {
            m_isInConference = inConference;
            if (inConference) {
                onConferenceJoined();
            } else {
                onConferenceLeft();
            }
        }
    }
}

/**
 * @brief 处理JavaScript错误事件
 * @param error 错误信息
 */
void ConferenceWindow::onJavaScriptError(const QString& error)
{
    qWarning() << "JavaScript错误:" << error;
    
    // 显示错误信息
    if (m_statusLabel) {
        m_statusLabel->setText(QString("JavaScript错误: %1").arg(error));
    }
    
    // 可以考虑重新注入JavaScript代码
    QTimer::singleShot(2000, this, [this]() {
        qDebug() << "尝试重新注入JavaScript代码";
        injectJavaScript();
    });
}

/**
 * @brief 处理Promise拒绝事件
 * @param reason 拒绝原因
 */
void ConferenceWindow::onPromiseRejected(const QString& reason)
{
    qWarning() << "Promise被拒绝:" << reason;
    
    // 记录Promise拒绝事件
    if (m_statusLabel) {
        m_statusLabel->setText(QString("操作失败: %1").arg(reason));
    }
    
    // 可以根据拒绝原因采取相应的恢复措施
    if (reason.contains("conference", Qt::CaseInsensitive)) {
        // 会议相关的Promise拒绝，可能需要重新加载
        QTimer::singleShot(3000, this, [this]() {
            if (!m_isInConference && !m_currentUrl.isEmpty()) {
                qDebug() << "尝试重新加载会议";
                if (m_webView) {
                    m_webView->load(QUrl(m_currentUrl));
                }
            }
        });
    }
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
    } else if (type == "jitsiMeetLoaded") {
        // Jitsi Meet加载完成
        onJitsiMeetLoaded();
    } else if (type == "conferenceStateUpdate") {
        // 会议状态更新
        onConferenceStateUpdated(message);
    } else if (type == "javascriptError") {
        // JavaScript错误
        QString error = message["error"].toString();
        QString source = message["source"].toString();
        int line = message["line"].toInt();
        QString detailedError = QString("%1 (来源: %2, 行: %3)").arg(error, source).arg(line);
        onJavaScriptError(detailedError);
    } else if (type == "promiseRejected") {
        // Promise拒绝
        QString reason = message["reason"].toString();
        onPromiseRejected(reason);
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
            // 启动网络诊断
            if (m_networkDiagnostics && !m_currentUrl.isEmpty()) {
                QUrl url(m_currentUrl);
                m_networkDiagnostics->startDiagnosis(url.host());
            }
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
}
    }
    
    emit conferenceLoadFailed(errorMsg);
}

/**
 * @brief 处理API连接成功
 */
void ConferenceWindow::onApiConnected()
{
    qDebug() << "ConferenceWindow: API连接成功";
    // 可以在这里更新UI状态，显示连接成功
}

/**
 * @brief 处理API连接断开
 */
void ConferenceWindow::onApiDisconnected()
{
    qDebug() << "ConferenceWindow: API连接断开";
    // 可以在这里更新UI状态，显示连接断开
}

/**
 * @brief 处理房间加入成功
 * @param roomName 房间名称
 */
void ConferenceWindow::onRoomJoined(const QString& roomName)
{
    qDebug() << "ConferenceWindow: 成功加入房间:" << roomName;
    m_isInConference = true;
    m_currentRoom = roomName;
    
    // 更新UI状态
    enableConferenceControls(true);
    
    // 发射信号
    emit conferenceJoined(roomName);
}

/**
 * @brief 处理房间离开
 * @param roomName 房间名称
 */
void ConferenceWindow::onRoomLeft(const QString& roomName)
{
    qDebug() << "ConferenceWindow: 离开房间:" << roomName;
    m_isInConference = false;
    
    // 更新UI状态
    enableConferenceControls(false);
    
    // 发射信号
    emit conferenceLeft(roomName);
}

/**
 * @brief 处理API错误
 * @param error 错误信息
 */
void ConferenceWindow::onApiError(const QString& error)
{
    qWarning() << "ConferenceWindow: API错误:" << error;
    
    // 显示错误消息
    showErrorMessage(tr("API错误: %1").arg(error));
    
    // 发射错误信号
    emit conferenceLoadFailed(error);
}

/**
 * @brief 启用或禁用会议控制按钮
 * @param enabled 是否启用
 */
void ConferenceWindow::enableConferenceControls(bool enabled)
{
    // 这里应该启用或禁用会议相关的UI控件
    // 由于具体的UI控件可能在其他地方定义，这里先提供基本实现
    qDebug() << "ConferenceWindow: 设置会议控制状态:" << enabled;
    
    // TODO: 实际的UI控件启用/禁用逻辑
    // 例如：m_muteButton->setEnabled(enabled);
    //      m_cameraButton->setEnabled(enabled);
    //      m_shareButton->setEnabled(enabled);
}

/**
 * @brief 显示错误消息
 * @param message 错误消息
 */
void ConferenceWindow::showErrorMessage(const QString& message)
{
    qWarning() << "ConferenceWindow: 显示错误消息:" << message;
    
    // 这里可以显示一个消息框或在状态栏显示错误
    if (m_statusLabel) {
        m_statusLabel->setText(message);
        m_statusLabel->setStyleSheet("color: red;");
    }
    
    // 也可以使用QMessageBox显示错误
    // QMessageBox::warning(this, tr("错误"), message);
}

/**
 * @brief 网络诊断完成处理
 * @param success 诊断是否成功
 * @param summary 诊断摘要
 */
void ConferenceWindow::onNetworkDiagnosticsCompleted(bool success, const QString& summary)
{
    qDebug() << "ConferenceWindow: 网络诊断完成:" << success << summary;
    
    // 显示诊断结果
    if (success) {
        m_statusLabel->setText(tr("网络诊断: 成功"));
        qDebug() << "ConferenceWindow: 网络诊断成功:" << summary;
    } else {
        m_statusLabel->setText(tr("网络诊断: 失败"));
        qWarning() << "ConferenceWindow: 网络诊断失败:" << summary;
        
        // 显示错误对话框
        QMessageBox::warning(this, tr("网络诊断"), 
                           tr("网络诊断失败:\n%1").arg(summary));
    }
}

/**
 * @brief 网络诊断错误处理
 * @param error 错误信息
 */
void ConferenceWindow::onNetworkDiagnosticsError(const QString& error)
{
    qWarning() << "ConferenceWindow: 网络诊断错误:" << error;
    
    QString message = tr("网络诊断失败: %1\n\n请手动检查:\n• 网络连接是否正常\n• DNS设置是否正确\n• 防火墙是否阻止连接\n• 代理设置是否正确").arg(error);
    
    QMessageBox::warning(this, tr("网络诊断错误"), message);
}

/**
 * @brief 处理新的权限请求（推荐使用的API）
 * @param permission 权限对象
 */
void ConferenceWindow::onPermissionRequested(QWebEnginePermission permission)
{
    qDebug() << "ConferenceWindow: 收到权限请求 - 类型:" << static_cast<int>(permission.permissionType())
             << "来源:" << permission.origin();
    
    // 自动授权摄像头和麦克风权限
    switch (permission.permissionType()) {
    case QWebEnginePermission::PermissionType::MediaAudioCapture:
        qDebug() << "ConferenceWindow: 授权麦克风访问权限";
        permission.grant();
        break;
    case QWebEnginePermission::PermissionType::MediaVideoCapture:
        qDebug() << "ConferenceWindow: 授权摄像头访问权限";
        permission.grant();
        break;
    case QWebEnginePermission::PermissionType::MediaAudioVideoCapture:
        qDebug() << "ConferenceWindow: 授权音视频访问权限";
        permission.grant();
        break;
    case QWebEnginePermission::PermissionType::DesktopVideoCapture:
        qDebug() << "ConferenceWindow: 授权屏幕共享权限";
        permission.grant();
        break;
    case QWebEnginePermission::PermissionType::DesktopAudioVideoCapture:
        qDebug() << "ConferenceWindow: 授权桌面音视频捕获权限";
        permission.grant();
        break;
    default:
        qDebug() << "ConferenceWindow: 拒绝未知权限请求";
        permission.deny();
        break;
    }
}