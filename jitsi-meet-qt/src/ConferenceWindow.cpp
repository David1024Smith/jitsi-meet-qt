#include "../include/ConferenceWindow.h"
// #include "modules/camera/include/CameraFactory.h"  // Temporarily disabled
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSplitter>
#include <QListWidgetItem>
#include <QIcon>
#include <QtMath>
#include <QDebug>
#include <QEvent>

ConferenceWindow::ConferenceWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_conferenceManager(nullptr)
    , m_mediaManager(nullptr)
    // , m_chatManager(nullptr) // 暂时禁用聊天功能
    // , m_screenShareManager(nullptr)  // 使用模块化版本
    , m_cameraManager(nullptr)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_navigationBar(nullptr)
    , m_videoArea(nullptr)
    , m_videoLayout(nullptr)
    , m_videoScrollArea(nullptr)
    , m_mainVideoWidget(nullptr)
    , m_mainSplitter(nullptr)
    , m_sidePanel(nullptr)
    , m_sidePanelLayout(nullptr)
    , m_chatPanel(nullptr)
    , m_chatLayout(nullptr)
    , m_chatDisplay(nullptr)
    , m_chatInput(nullptr)
    , m_sendButton(nullptr)
    , m_chatUnreadLabel(nullptr)
    , m_chatPanelVisible(false)
    , m_participantsPanel(nullptr)
    , m_participantsLayout(nullptr)
    , m_participantsList(nullptr)
    , m_participantsCountLabel(nullptr)
    , m_participantsPanelVisible(false)
    , m_controlPanel(nullptr)
    , m_controlLayout(nullptr)
    , m_muteAudioButton(nullptr)
    , m_muteVideoButton(nullptr)
    , m_screenShareButton(nullptr)
    , m_noVideoLabel(nullptr)
    , m_chatToggleButton(nullptr)
    , m_participantsToggleButton(nullptr)
    , m_statusBar(nullptr)
    , m_statusLayout(nullptr)
    , m_connectionStatusLabel(nullptr)
    , m_participantCountLabel(nullptr)
    , m_connectionProgress(nullptr)
    , m_isInConference(false)
    , m_isAudioMuted(false)
    , m_isVideoMuted(false)
    , m_isScreenSharing(false)
    , m_participantCount(0)
    , m_unreadChatCount(0)
    , m_videoGridColumns(2)
    , m_videoGridRows(2)
    , m_videoWidgetSize(320, 240)
{
    setupUI();
    setupManagers();
    setupConnections();
    applyStyles();
    
    // 设置窗口属性
    setWindowTitle(tr("Jitsi Meet Conference"));
    setMinimumSize(800, 600);
    resize(1200, 800);
}

ConferenceWindow::~ConferenceWindow()
{
    // 离开会议
    if (m_isInConference) {
        leaveConference();
    }
    
    // 清理管理器
    if (m_conferenceManager) {
        delete m_conferenceManager;
    }
    if (m_mediaManager) {
        delete m_mediaManager;
    }
    // if (m_chatManager) {
    //     delete m_chatManager;
    // } // 暂时禁用聊天功能
    // if (m_screenShareManager) {
    //     delete m_screenShareManager;
    // }  // 使用模块化版本
}

void ConferenceWindow::setupUI()
{
    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    createMainLayout();
    createVideoArea();
    createControlPanel();
    createChatPanel();
    createParticipantsPanel();
    createStatusBar();
}

void ConferenceWindow::setupManagers()
{
    qDebug() << "Setting up ConferenceWindow managers...";
    
    // 创建会议管理器
    m_conferenceManager = new ConferenceManager(this);
    qDebug() << "ConferenceManager created";
    
    // 创建媒体管理器
    m_mediaManager = new MediaManager(this);
    qDebug() << "MediaManager created";
    
    // 创建模块化摄像头管理器 - 暂时禁用
    // m_cameraManager = CameraFactory::instance()->createLocalCameraInterface("conference_local");
    // if (m_cameraManager) {
    //     qDebug() << "CameraManager created successfully";
    //     
    //     // 连接摄像头管理器信号
    //     connect(m_cameraManager, &ICameraManager::cameraStarted,
    //             this, &ConferenceWindow::onLocalVideoStarted);
    //     connect(m_cameraManager, &ICameraManager::cameraStopped,
    //             this, &ConferenceWindow::onLocalVideoStopped);
    //     connect(m_cameraManager, &ICameraManager::errorOccurred,
    //             [this](const QString& error) {
    //                 qWarning() << "Camera error:" << error;
    //             });
    // } else {
        qWarning() << "Failed to create CameraManager";
    // }
    
    // 创建聊天管理器 (暂时禁用)
    // m_chatManager = new ChatManager(this);
    // qDebug() << "ChatManager created";
    
    // 创建屏幕共享管理器 (使用模块化版本)
    // m_screenShareManager = new ScreenShareManager(this);
    // qDebug() << "ScreenShareManager created";
    
    // 设置管理器之间的依赖关系
    m_mediaManager->setWebRTCEngine(m_conferenceManager->webRTCEngine());
    // m_chatManager->setXMPPClient(m_conferenceManager->xmppClient()); // 暂时禁用聊天功能
    
    qDebug() << "All ConferenceWindow managers initialized";
}

void ConferenceWindow::setupConnections()
{
    qDebug() << "Setting up ConferenceWindow signal connections...";
    
    // 导航栏连接
    connect(m_navigationBar, &NavigationBar::backClicked,
            this, &ConferenceWindow::onBackButtonClicked);
    connect(m_navigationBar, &NavigationBar::settingsClicked,
            this, &ConferenceWindow::settingsRequested);
    
    // 控制按钮连接
    connect(m_muteAudioButton, &QPushButton::clicked,
            this, &ConferenceWindow::onMuteAudioClicked);
    connect(m_muteVideoButton, &QPushButton::clicked,
            this, &ConferenceWindow::onMuteVideoClicked);
    connect(m_screenShareButton, &QPushButton::clicked,
            this, &ConferenceWindow::onScreenShareClicked);
    connect(m_chatToggleButton, &QPushButton::clicked,
            this, &ConferenceWindow::onChatToggleClicked);
    connect(m_participantsToggleButton, &QPushButton::clicked,
            this, &ConferenceWindow::onParticipantsToggleClicked);
    
    // 聊天输入连接
    connect(m_sendButton, &QPushButton::clicked,
            this, &ConferenceWindow::onSendChatMessage);
    connect(m_chatInput, &QLineEdit::returnPressed,
            this, &ConferenceWindow::onChatInputReturnPressed);
    
    // 参与者列表连接
    connect(m_participantsList, &QListWidget::itemClicked,
            this, &ConferenceWindow::onParticipantItemClicked);
    
    // 会议管理器信号连接
    if (m_conferenceManager) {
        connect(m_conferenceManager, &ConferenceManager::connectionStateChanged,
                this, &ConferenceWindow::onConnectionStateChanged);
        connect(m_conferenceManager, &ConferenceManager::conferenceStateChanged,
                this, &ConferenceWindow::onConferenceStateChanged);
        connect(m_conferenceManager, &ConferenceManager::conferenceJoined,
                this, &ConferenceWindow::onConferenceJoined);
        connect(m_conferenceManager, &ConferenceManager::conferenceLeft,
                this, &ConferenceWindow::onConferenceLeft);
        connect(m_conferenceManager, &ConferenceManager::participantJoined,
                this, &ConferenceWindow::onParticipantJoined);
        connect(m_conferenceManager, &ConferenceManager::participantLeft,
                this, &ConferenceWindow::onParticipantLeft);
        connect(m_conferenceManager, &ConferenceManager::participantUpdated,
                this, &ConferenceWindow::onParticipantUpdated);
        connect(m_conferenceManager, &ConferenceManager::localMediaStateChanged,
                this, &ConferenceWindow::onLocalMediaStateChanged);
        connect(m_conferenceManager, &ConferenceManager::screenShareStateChanged,
                this, &ConferenceWindow::onScreenShareStateChanged);
        connect(m_conferenceManager, &ConferenceManager::errorOccurred,
                this, &ConferenceWindow::onErrorOccurred);
    }
    
    // 媒体管理器信号连接
    if (m_mediaManager) {
        connect(m_mediaManager, &MediaManager::localVideoStarted,
                this, &ConferenceWindow::onLocalVideoStarted);
        connect(m_mediaManager, &MediaManager::localVideoStopped,
                this, &ConferenceWindow::onLocalVideoStopped);
        // Remote video signals are handled by ConferenceManager
        // connect(m_conferenceManager, &ConferenceManager::remoteVideoReceived,
        //         this, &ConferenceWindow::onRemoteVideoReceived);
        // connect(m_conferenceManager, &ConferenceManager::remoteVideoRemoved,
        //         this, &ConferenceWindow::onRemoteVideoRemoved);
    }
    
    // 聊天管理器信号连接 (暂时禁用)
    // if (m_chatManager) {
    //     // connect(m_chatManager, &ChatManager::messageReceived,
    //     //         this, &ConferenceWindow::onChatMessageReceived);  // 暂时禁用，签名不匹配
    //     connect(m_chatManager, &ChatManager::messageSent,
    //             this, &ConferenceWindow::onChatMessageSent);
    //     // Note: unreadCountChanged signal doesn't exist in ChatManager, removing this connection
    // }
    
    // 屏幕共享管理器信号连接 (使用模块化版本)
    // if (m_screenShareManager) {
    //     connect(m_screenShareManager, &ScreenShareManager::shareStarted,
    //         this, &ConferenceWindow::onScreenShareStarted);
    //     connect(m_screenShareManager, &ScreenShareManager::shareStopped,
    //             this, &ConferenceWindow::onScreenShareStopped);
    //     // connect(m_screenShareManager, &ScreenShareManager::remoteScreenShareReceived,
    //     //         this, &ConferenceWindow::onRemoteScreenShareReceived);
    //     // connect(m_screenShareManager, &ScreenShareManager::remoteScreenShareRemoved,
    //     //         this, &ConferenceWindow::onRemoteScreenShareRemoved);
    // }
    
    qDebug() << "All ConferenceWindow signal connections established";
}



void ConferenceWindow::createMainLayout()
{
    // 创建主布局
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // 创建导航栏
    m_navigationBar = new NavigationBar(this);
    m_navigationBar->setButtonConfiguration(NavigationBar::BackButton | NavigationBar::SettingsButton);
    m_navigationBar->setTitle(tr("Jitsi Meet Conference"));
    
    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setChildrenCollapsible(false);
    
    // 创建侧边面板
    m_sidePanel = new QWidget(this);
    m_sidePanelLayout = new QVBoxLayout(m_sidePanel);
    m_sidePanelLayout->setContentsMargins(5, 5, 5, 5);
    m_sidePanelLayout->setSpacing(5);
    
    // 设置分割器比例
    m_mainSplitter->setSizes({800, 300});
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 0);
}

void ConferenceWindow::createVideoArea()
{
    // 创建视频区域容器
    QWidget* videoContainer = new QWidget(this);
    QVBoxLayout* videoContainerLayout = new QVBoxLayout(videoContainer);
    videoContainerLayout->setContentsMargins(0, 0, 0, 0);
    videoContainerLayout->setSpacing(0);
    
    // 添加导航栏
    videoContainerLayout->addWidget(m_navigationBar);
    
    // 创建视频显示区域
    m_videoArea = new QWidget(this);
    m_videoLayout = new QGridLayout(m_videoArea);
    m_videoLayout->setContentsMargins(10, 10, 10, 10);
    m_videoLayout->setSpacing(10);
    
    // 创建滚动区域
    m_videoScrollArea = new QScrollArea(this);
    m_videoScrollArea->setWidget(m_videoArea);
    m_videoScrollArea->setWidgetResizable(true);
    m_videoScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_videoScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // 创建无视频提示标签
    m_noVideoLabel = new QLabel(tr("No participants in conference"), this);
    m_noVideoLabel->setAlignment(Qt::AlignCenter);
    m_noVideoLabel->setStyleSheet("QLabel { color: #666; font-size: 16px; }");
    m_videoLayout->addWidget(m_noVideoLabel, 0, 0, Qt::AlignCenter);
    
    videoContainerLayout->addWidget(m_videoScrollArea, 1);
    
    // 添加到主分割器
    m_mainSplitter->addWidget(videoContainer);
}

void ConferenceWindow::createControlPanel()
{
    // 创建控制面板
    m_controlPanel = new QWidget(this);
    m_controlLayout = new QHBoxLayout(m_controlPanel);
    m_controlLayout->setContentsMargins(10, 5, 10, 5);
    m_controlLayout->setSpacing(10);
    
    // 创建控制按钮
    m_muteAudioButton = new QPushButton(tr("Mute Audio"), this);
    m_muteAudioButton->setCheckable(true);
    m_muteAudioButton->setIcon(QIcon(":/icons/microphone.svg"));
    
    m_muteVideoButton = new QPushButton(tr("Mute Video"), this);
    m_muteVideoButton->setCheckable(true);
    m_muteVideoButton->setIcon(QIcon(":/icons/camera.svg"));
    
    m_screenShareButton = new QPushButton(tr("Share Screen"), this);
    m_screenShareButton->setCheckable(true);
    m_screenShareButton->setIcon(QIcon(":/icons/screen-share.svg"));
    
    m_chatToggleButton = new QPushButton(tr("Chat"), this);
    m_chatToggleButton->setCheckable(true);
    m_chatToggleButton->setIcon(QIcon(":/icons/chat.svg"));
    
    m_participantsToggleButton = new QPushButton(tr("Participants"), this);
    m_participantsToggleButton->setCheckable(true);
    m_participantsToggleButton->setIcon(QIcon(":/icons/participants.svg"));
    
    // 添加按钮到布局
    m_controlLayout->addWidget(m_muteAudioButton);
    m_controlLayout->addWidget(m_muteVideoButton);
    m_controlLayout->addWidget(m_screenShareButton);
    m_controlLayout->addStretch();
    m_controlLayout->addWidget(m_chatToggleButton);
    m_controlLayout->addWidget(m_participantsToggleButton);
    
    // 添加控制面板到视频容器
    QWidget* videoContainer = qobject_cast<QWidget*>(m_mainSplitter->widget(0));
    if (videoContainer) {
        QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(videoContainer->layout());
        if (layout) {
            layout->addWidget(m_controlPanel);
        }
    }
}

void ConferenceWindow::createChatPanel()
{
    // 创建聊天面板
    m_chatPanel = new QWidget(this);
    m_chatLayout = new QVBoxLayout(m_chatPanel);
    m_chatLayout->setContentsMargins(0, 0, 0, 0);
    m_chatLayout->setSpacing(5);
    
    // 聊天标题
    QLabel* chatTitle = new QLabel(tr("Chat"), this);
    chatTitle->setStyleSheet("QLabel { font-weight: bold; padding: 5px; background-color: #f0f0f0; }");
    m_chatLayout->addWidget(chatTitle);
    
    // 聊天显示区域
    m_chatDisplay = new QTextEdit(this);
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setMaximumHeight(200);
    m_chatLayout->addWidget(m_chatDisplay);
    
    // 聊天输入区域
    QWidget* chatInputWidget = new QWidget(this);
    QHBoxLayout* chatInputLayout = new QHBoxLayout(chatInputWidget);
    chatInputLayout->setContentsMargins(0, 0, 0, 0);
    chatInputLayout->setSpacing(5);
    
    m_chatInput = new QLineEdit(this);
    m_chatInput->setPlaceholderText(tr("Type a message..."));
    
    m_sendButton = new QPushButton(tr("Send"), this);
    m_sendButton->setMaximumWidth(60);
    
    chatInputLayout->addWidget(m_chatInput);
    chatInputLayout->addWidget(m_sendButton);
    
    m_chatLayout->addWidget(chatInputWidget);
    
    // 未读消息指示器
    m_chatUnreadLabel = new QLabel(this);
    m_chatUnreadLabel->setStyleSheet("QLabel { background-color: red; color: white; border-radius: 10px; padding: 2px 6px; }");
    m_chatUnreadLabel->setVisible(false);
    
    // 初始隐藏聊天面板
    m_chatPanel->setVisible(false);
    
    // 添加到侧边面板
    m_sidePanelLayout->addWidget(m_chatPanel);
}

void ConferenceWindow::createParticipantsPanel()
{
    // 创建参与者面板
    m_participantsPanel = new QWidget(this);
    m_participantsLayout = new QVBoxLayout(m_participantsPanel);
    m_participantsLayout->setContentsMargins(0, 0, 0, 0);
    m_participantsLayout->setSpacing(5);
    
    // 参与者标题
    m_participantsCountLabel = new QLabel(tr("Participants (0)"), this);
    m_participantsCountLabel->setStyleSheet("QLabel { font-weight: bold; padding: 5px; background-color: #f0f0f0; }");
    m_participantsLayout->addWidget(m_participantsCountLabel);
    
    // 参与者列表
    m_participantsList = new QListWidget(this);
    m_participantsList->setMaximumHeight(150);
    m_participantsLayout->addWidget(m_participantsList);
    
    // 初始隐藏参与者面板
    m_participantsPanel->setVisible(false);
    
    // 添加到侧边面板
    m_sidePanelLayout->addWidget(m_participantsPanel);
    
    // 添加弹性空间
    m_sidePanelLayout->addStretch();
}

void ConferenceWindow::createStatusBar()
{
    // 创建状态栏
    m_statusBar = new QWidget(this);
    m_statusLayout = new QHBoxLayout(m_statusBar);
    m_statusLayout->setContentsMargins(10, 5, 10, 5);
    m_statusLayout->setSpacing(10);
    
    // 连接状态标签
    m_connectionStatusLabel = new QLabel(tr("Disconnected"), this);
    m_connectionStatusLabel->setStyleSheet("QLabel { color: #666; }");
    
    // 参与者计数标签
    m_participantCountLabel = new QLabel(tr("0 participants"), this);
    m_participantCountLabel->setStyleSheet("QLabel { color: #666; }");
    
    // 连接进度条
    m_connectionProgress = new QProgressBar(this);
    m_connectionProgress->setMaximumWidth(200);
    m_connectionProgress->setVisible(false);
    
    m_statusLayout->addWidget(m_connectionStatusLabel);
    m_statusLayout->addWidget(m_connectionProgress);
    m_statusLayout->addStretch();
    m_statusLayout->addWidget(m_participantCountLabel);
    
    // 添加状态栏到视频容器
    QWidget* videoContainer = qobject_cast<QWidget*>(m_mainSplitter->widget(0));
    if (videoContainer) {
        QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(videoContainer->layout());
        if (layout) {
            layout->addWidget(m_statusBar);
        }
    }
    
    // 完成主布局
    m_mainSplitter->addWidget(m_sidePanel);
    m_mainLayout->addWidget(m_mainSplitter);
}



void ConferenceWindow::applyStyles()
{
    setStyleSheet(
        "ConferenceWindow {"
        "    background-color: #f5f5f5;"
        "}"
        
        "QVideoWidget {"
        "    border: 2px solid #ddd;"
        "    border-radius: 8px;"
        "    background-color: #000;"
        "}"
        
        "QPushButton {"
        "    background-color: #007bff;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "    min-width: 80px;"
        "}"
        
        "QPushButton:hover {"
        "    background-color: #0056b3;"
        "}"
        
        "QPushButton:pressed {"
        "    background-color: #004085;"
        "}"
        
        "QPushButton:checked {"
        "    background-color: #dc3545;"
        "}"
        
        "QPushButton:checked:hover {"
        "    background-color: #c82333;"
        "}"
        
        "QTextEdit {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    font-size: 12px;"
        "}"
        
        "QLineEdit {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 6px;"
        "    font-size: 12px;"
        "}"
        
        "QListWidget {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    font-size: 12px;"
        "}"
        
        "QListWidget::item {"
        "    padding: 4px;"
        "    border-bottom: 1px solid #eee;"
        "}"
        
        "QListWidget::item:selected {"
        "    background-color: #007bff;"
        "    color: white;"
        "}"
        
        "QProgressBar {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    text-align: center;"
        "    font-size: 12px;"
        "    height: 20px;"
        "}"
        
        "QProgressBar::chunk {"
        "    background-color: #007bff;"
        "    border-radius: 3px;"
        "}"
        
        "QLabel {"
        "    color: #333;"
        "    font-size: 12px;"
        "}"
        
        "QSplitter::handle {"
        "    background-color: #ddd;"
        "    width: 2px;"
        "}"
        
        "QScrollArea {"
        "    border: none;"
        "    background-color: transparent;"
        "}"
    );
}

void ConferenceWindow::retranslateUi()
{
    // 更新窗口标题
    if (m_isInConference) {
        // 保持当前会议标题
    } else {
        setWindowTitle(tr("Jitsi Meet Conference"));
    }
    
    // 更新导航栏
    if (m_navigationBar) {
        m_navigationBar->retranslateUi();
    }
    
    // 更新控制按钮
    if (m_muteAudioButton) {
        m_muteAudioButton->setText(m_isAudioMuted ? tr("Unmute Audio") : tr("Mute Audio"));
    }
    if (m_muteVideoButton) {
        m_muteVideoButton->setText(m_isVideoMuted ? tr("Start Video") : tr("Stop Video"));
    }
    if (m_screenShareButton) {
        m_screenShareButton->setText(m_isScreenSharing ? tr("Stop Sharing") : tr("Share Screen"));
    }
    if (m_chatToggleButton) {
        m_chatToggleButton->setText(tr("Chat"));
    }
    if (m_participantsToggleButton) {
        m_participantsToggleButton->setText(tr("Participants"));
    }
    
    // 更新其他UI元素
    if (m_sendButton) {
        m_sendButton->setText(tr("Send"));
    }
    if (m_chatInput) {
        m_chatInput->setPlaceholderText(tr("Type a message..."));
    }
    if (m_noVideoLabel) {
        m_noVideoLabel->setText(tr("No participants in conference"));
    }
    if (m_connectionStatusLabel) {
        // 连接状态会通过事件更新
    }
    if (m_participantCountLabel) {
        m_participantCountLabel->setText(tr("%1 participants").arg(m_participantCount));
    }
    if (m_participantsCountLabel) {
        m_participantsCountLabel->setText(tr("Participants (%1)").arg(m_participantCount));
    }
}

void ConferenceWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void ConferenceWindow::joinConference(const QString& url)
{
    if (url.isEmpty()) {
        showError(tr("Invalid conference URL"));
        return;
    }
    
    m_currentUrl = url;
    
    qDebug() << "Joining conference:" << m_currentUrl;
    
    // 显示连接状态
    showLoading(tr("Connecting to conference..."));
    
    // 使用新的模块化摄像头管理器
    if (m_cameraManager) {
        qDebug() << "Starting camera using CameraManager";
        
        // 创建视频预览组件 - 暂时禁用
        // QVideoWidget* videoWidget = m_cameraManager->createPreviewWidget(this);
        // if (videoWidget) {
        //     qDebug() << "Video preview widget created";
        // }
        // 
        // // 启动摄像头
        // if (m_cameraManager->startWithPreset(ICameraDevice::StandardQuality)) {
        //     qDebug() << "Camera started successfully with CameraManager";
        // } else {
        //     qWarning() << "Failed to start camera with CameraManager";
        // }
    } else {
        // 回退到原有的MediaManager方式
        qDebug() << "Falling back to MediaManager for camera";
        
        // 确保本地视频组件存在
        if (!m_mediaManager->localVideoWidget()) {
            qDebug() << "Creating local video widget for conference";
            QVideoWidget* localVideo = new QVideoWidget(this);
            localVideo->setMinimumSize(320, 240);
            localVideo->setStyleSheet("background-color: black; border: 2px solid blue;");
            
            // 设置给MediaManager
            m_mediaManager->setLocalVideoWidget(localVideo);
            qDebug() << "Local video widget created and set to MediaManager";
        }
        
        // 启动本地视频
        m_mediaManager->startLocalVideo();
    }
    
    // 启动音频（仍使用MediaManager）
    m_mediaManager->startLocalAudio();
    
    // 加入会议
    m_conferenceManager->joinConference(url);
    
    m_isInConference = true;
    updateControlButtons();
}

void ConferenceWindow::leaveConference()
{
    if (!m_isInConference) {
        return;
    }
    
    qDebug() << "Leaving conference";
    
    // 停止媒体流
    m_mediaManager->stopLocalVideo();
    m_mediaManager->stopLocalAudio();
    
    // 停止屏幕共享 (使用模块化版本)
    // if (m_isScreenSharing) {
    //     m_screenShareManager->stopScreenShare();
    // }
    
    // 离开会议
    m_conferenceManager->leaveConference();
    
    // 清理UI状态
    m_isInConference = false;
    m_isAudioMuted = false;
    m_isVideoMuted = false;
    m_isScreenSharing = false;
    m_participantCount = 0;
    m_unreadChatCount = 0;
    
    // 清理视频部件
    for (auto it = m_videoWidgets.begin(); it != m_videoWidgets.end(); ++it) {
        removeVideoWidget(it.key());
    }
    m_videoWidgets.clear();
    
    // 更新UI
    updateControlButtons();
    updateParticipantsList();
    hideLoading();
    
    emit conferenceLeft();
}

// 会议管理器事件处理
void ConferenceWindow::onConnectionStateChanged(ConferenceManager::ConnectionState state)
{
    QString statusText;
    switch (state) {
        case ConferenceManager::Disconnected:
            statusText = tr("Disconnected");
            m_connectionProgress->setVisible(false);
            break;
        case ConferenceManager::Connecting:
            statusText = tr("Connecting...");
            m_connectionProgress->setVisible(true);
            m_connectionProgress->setRange(0, 0); // 不确定进度
            break;
        case ConferenceManager::Connected:
            statusText = tr("Connected");
            m_connectionProgress->setVisible(false);
            hideLoading();
            break;
        case ConferenceManager::Reconnecting:
            statusText = tr("Reconnecting...");
            m_connectionProgress->setVisible(true);
            break;
        case ConferenceManager::Failed:
            statusText = tr("Connection Failed");
            m_connectionProgress->setVisible(false);
            showError(tr("Failed to connect to conference"));
            break;
    }
    
    m_connectionStatusLabel->setText(statusText);
}

void ConferenceWindow::onConferenceStateChanged(ConferenceManager::ConferenceState state)
{
    switch (state) {
        case ConferenceManager::Idle:
            break;
        case ConferenceManager::Joining:
            showLoading(tr("Joining conference..."));
            break;
        case ConferenceManager::InConference:
            hideLoading();
            break;
        case ConferenceManager::Leaving:
            showLoading(tr("Leaving conference..."));
            break;
        case ConferenceManager::Error:
            showError(tr("Conference error occurred"));
            break;
    }
}

void ConferenceWindow::onConferenceJoined(const ConferenceManager::ConferenceInfo& info)
{
    qDebug() << "Conference joined successfully:" << info.roomName;
    
    // 更新窗口标题
    setWindowTitle(QString("Jitsi Meet - %1").arg(info.roomName));
    
    // 设置聊天管理器的当前房间 - 暂时禁用
    // m_chatManager->setCurrentRoom(info.roomName);
    
    hideLoading();
    emit conferenceJoined(m_currentUrl);
}

void ConferenceWindow::onConferenceLeft()
{
    qDebug() << "Conference left";
    
    // 重置窗口标题
    setWindowTitle(tr("Jitsi Meet Conference"));
    
    hideLoading();
    emit conferenceLeft();
}

void ConferenceWindow::onParticipantJoined(const ConferenceManager::ParticipantInfo& participant)
{
    qDebug() << "Participant joined:" << participant.displayName;
    
    m_participantCount++;
    addParticipantToList(participant);
    updateParticipantsList();
}

void ConferenceWindow::onParticipantLeft(const QString& jid)
{
    qDebug() << "Participant left:" << jid;
    
    m_participantCount--;
    removeParticipantFromList(jid);
    removeVideoWidget(jid);
    updateParticipantsList();
}

void ConferenceWindow::onParticipantUpdated(const ConferenceManager::ParticipantInfo& participant)
{
    updateParticipantInList(participant);
}

void ConferenceWindow::onLocalMediaStateChanged(bool audioMuted, bool videoMuted)
{
    m_isAudioMuted = audioMuted;
    m_isVideoMuted = videoMuted;
    updateMediaButtons();
}

void ConferenceWindow::onScreenShareStateChanged(bool isSharing, const QString& participantJid)
{
    if (participantJid.isEmpty() || participantJid == m_conferenceManager->localParticipant().jid) {
        // 本地屏幕共享状态变化
        m_isScreenSharing = isSharing;
        updateScreenShareButton();
    }
}

void ConferenceWindow::onErrorOccurred(const JitsiError& error)
{
    qDebug() << "Conference error:" << error.message();
    showError(error.message());
}

QString ConferenceWindow::currentUrl() const
{
    return m_currentUrl;
}

// 媒体管理器事件处理
void ConferenceWindow::onLocalVideoStarted()
{
    qDebug() << "Local video started";
    
    QVideoWidget* localVideo = nullptr;
    
    // 优先使用CameraManager的视频组件 - 暂时禁用
    // if (m_cameraManager) {
    //     localVideo = m_cameraManager->previewWidget();
    //     qDebug() << "Using CameraManager video widget";
    // }
    
    // 回退到MediaManager
    if (!localVideo) {
        localVideo = m_mediaManager->localVideoWidget();
        if (!localVideo) {
            // 如果MediaManager还没有本地视频组件，创建一个
            qDebug() << "Creating local video widget for MediaManager";
            localVideo = new QVideoWidget(this);
            localVideo->setMinimumSize(320, 240);
            localVideo->setStyleSheet("background-color: black; border: 2px solid green;");
            
            // 设置给MediaManager
            m_mediaManager->setLocalVideoWidget(localVideo);
            
            // 强制启动摄像头显示
            m_mediaManager->forceStartCameraDisplay();
            
            qDebug() << "Local video widget created and set to MediaManager";
        }
    }
    
    if (localVideo) {
        addVideoWidget("local", localVideo);
        qDebug() << "Local video widget added to layout";
    } else {
        qWarning() << "No local video widget available";
    }
}

void ConferenceWindow::onLocalVideoStopped()
{
    qDebug() << "Local video stopped";
    removeVideoWidget("local");
}

void ConferenceWindow::onRemoteVideoReceived(const QString& participantId, QVideoWidget* widget)
{
    qDebug() << "Remote video received from:" << participantId;
    addVideoWidget(participantId, widget);
}

void ConferenceWindow::onRemoteVideoRemoved(const QString& participantId)
{
    qDebug() << "Remote video removed from:" << participantId;
    removeVideoWidget(participantId);
}

// 聊天管理器事件处理
void ConferenceWindow::onChatMessageReceived(ChatMessage* message)
{
    addChatMessage(message);
    
    // 如果聊天面板未显示，增加未读计数
    if (!m_chatPanelVisible) {
        m_unreadChatCount++;
        updateChatUnreadIndicator();
    }
}

void ConferenceWindow::onChatMessageSent(const QString& messageId)
{
    // 消息发送成功，可以在这里更新UI状态
    Q_UNUSED(messageId)
    // Note: We don't have the message object here, just the ID
}

void ConferenceWindow::onUnreadCountChanged(int count)
{
    m_unreadChatCount = count;
    updateChatUnreadIndicator();
}

// 屏幕共享管理器事件处理
void ConferenceWindow::onScreenShareStarted()
{
    qDebug() << "Screen share started";
    
    // QVideoWidget* screenShareWidget = m_screenShareManager->localScreenShareWidget();
    // if (screenShareWidget) {
    //     setMainVideoWidget(screenShareWidget);
    // }
}

void ConferenceWindow::onScreenShareStopped()
{
    qDebug() << "Screen share stopped";
    
    // 恢复到正常视频布局
    updateVideoLayout();
}

void ConferenceWindow::onRemoteScreenShareReceived(const QString& participantId, QVideoWidget* widget)
{
    qDebug() << "Remote screen share received from:" << participantId;
    setMainVideoWidget(widget);
}

void ConferenceWindow::onRemoteScreenShareRemoved(const QString& participantId)
{
    qDebug() << "Remote screen share removed from:" << participantId;
    updateVideoLayout();
}

// UI事件处理
void ConferenceWindow::onBackButtonClicked()
{
    if (m_isInConference) {
        // 确认是否要离开会议
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Leave Conference"),
            tr("Are you sure you want to leave the conference?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            leaveConference();
            emit backToWelcome();
        }
    } else {
        emit backToWelcome();
    }
}

void ConferenceWindow::onMuteAudioClicked()
{
    bool muted = !m_isAudioMuted;
    m_conferenceManager->setAudioMuted(muted);
    
    if (muted) {
        m_mediaManager->stopLocalAudio();
    } else {
        m_mediaManager->startLocalAudio();
    }
}

void ConferenceWindow::onMuteVideoClicked()
{
    bool muted = !m_isVideoMuted;
    m_conferenceManager->setVideoMuted(muted);
    
    if (muted) {
        m_mediaManager->stopLocalVideo();
    } else {
        m_mediaManager->startLocalVideo();
    }
}

void ConferenceWindow::onScreenShareClicked()
{
    // 使用模块化版本的屏幕共享
    // if (m_isScreenSharing) {
    //     m_screenShareManager->stopScreenShare();
    // } else {
    //     // if (m_screenShareManager->showScreenSelectionDialog()) {
    //     //     // 用户选择了屏幕，开始共享
    //     //     // 实际的开始逻辑在ScreenShareManager中处理
    //     // }
    // }
}

void ConferenceWindow::onChatToggleClicked()
{
    toggleChatPanel();
}

void ConferenceWindow::onParticipantsToggleClicked()
{
    toggleParticipantsPanel();
}

void ConferenceWindow::onSendChatMessage()
{
    QString message = m_chatInput->text().trimmed();
    if (!message.isEmpty()) {
        // m_chatManager->sendMessage(message); // 暂时禁用聊天功能
        m_chatInput->clear();
    }
}

void ConferenceWindow::onChatInputReturnPressed()
{
    onSendChatMessage();
}

void ConferenceWindow::onParticipantItemClicked()
{
    // 可以在这里实现参与者相关的操作，如私聊等
}

// 视频布局管理
void ConferenceWindow::updateVideoLayout()
{
    arrangeVideoWidgets();
}

void ConferenceWindow::arrangeVideoWidgets()
{
    int videoCount = m_videoWidgets.size();
    
    qDebug() << "Arranging video widgets, count:" << videoCount;
    
    if (videoCount == 0) {
        m_noVideoLabel->setVisible(true);
        qDebug() << "No video widgets, showing no video label";
        return;
    }
    
    m_noVideoLabel->setVisible(false);
    
    // 计算网格布局
    m_videoGridColumns = qCeil(qSqrt(videoCount));
    m_videoGridRows = qCeil(static_cast<double>(videoCount) / m_videoGridColumns);
    
    qDebug() << "Video grid layout:" << m_videoGridRows << "x" << m_videoGridColumns;
    
    // 重新排列视频部件
    int row = 0, col = 0;
    for (auto it = m_videoWidgets.begin(); it != m_videoWidgets.end(); ++it) {
        QVideoWidget* widget = it.value();
        QString participantId = it.key();
        
        qDebug() << "Placing video widget for" << participantId << "at position" << row << "," << col;
        qDebug() << "Widget size:" << widget->size() << "visible:" << widget->isVisible();
        
        m_videoLayout->addWidget(widget, row, col);
        
        // 确保组件可见
        widget->show();
        widget->setVisible(true);
        
        col++;
        if (col >= m_videoGridColumns) {
            col = 0;
            row++;
        }
    }
    
    qDebug() << "Video layout arrangement completed";
}

void ConferenceWindow::addVideoWidget(const QString& participantId, QVideoWidget* widget)
{
    if (!widget || m_videoWidgets.contains(participantId)) {
        qDebug() << "Cannot add video widget for" << participantId << "- widget:" << (widget ? "valid" : "null") << "already exists:" << m_videoWidgets.contains(participantId);
        return;
    }
    
    qDebug() << "Adding video widget for participant:" << participantId;
    
    widget->setMinimumSize(m_videoWidgetSize);
    widget->setMaximumSize(m_videoWidgetSize * 2);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 确保组件可见
    widget->show();
    widget->setVisible(true);
    
    // 为本地视频添加特殊样式
    if (participantId == "local") {
        widget->setStyleSheet("background-color: black; border: 3px solid green; border-radius: 5px;");
        qDebug() << "Applied local video styling";
    }
    
    m_videoWidgets[participantId] = widget;
    
    qDebug() << "Video widget added, total widgets:" << m_videoWidgets.size();
    updateVideoLayout();
}

void ConferenceWindow::removeVideoWidget(const QString& participantId)
{
    if (!m_videoWidgets.contains(participantId)) {
        return;
    }
    
    QVideoWidget* widget = m_videoWidgets.take(participantId);
    m_videoLayout->removeWidget(widget);
    widget->setParent(nullptr);
    
    updateVideoLayout();
}

void ConferenceWindow::setMainVideoWidget(QVideoWidget* widget)
{
    if (!widget) {
        return;
    }
    
    // 将指定的视频部件设置为主视频（占据更大空间）
    m_mainVideoWidget = widget;
    
    // 重新安排布局，主视频占据更多空间
    m_videoLayout->addWidget(widget, 0, 0, 2, 2); // 占据2x2的空间
    
    // 其他视频部件排列在侧边
    int row = 0, col = 2;
    for (auto it = m_videoWidgets.begin(); it != m_videoWidgets.end(); ++it) {
        if (it.value() != widget) {
            m_videoLayout->addWidget(it.value(), row, col);
            row++;
            if (row >= 4) { // 最多显示4个侧边视频
                break;
            }
        }
    }
}

// 聊天功能
void ConferenceWindow::addChatMessage(ChatMessage* message)
{
    // 暂时禁用聊天功能
    Q_UNUSED(message);
    /*
    QString messageHtml = QString("<div style='margin-bottom: 5px;'>"
                                 "<b>%1</b> <span style='color: #666; font-size: 10px;'>%2</span><br>"
                                 "%3</div>")
                         .arg(message->senderName())
                         .arg(message->timestamp().toString("hh:mm"))
                         .arg(message->content());
    
    m_chatDisplay->append(messageHtml);
    scrollChatToBottom();
    */
}

void ConferenceWindow::updateChatUnreadIndicator()
{
    if (m_unreadChatCount > 0) {
        m_chatUnreadLabel->setText(QString::number(m_unreadChatCount));
        m_chatUnreadLabel->setVisible(true);
        m_chatToggleButton->setText(QString("Chat (%1)").arg(m_unreadChatCount));
    } else {
        m_chatUnreadLabel->setVisible(false);
        m_chatToggleButton->setText(tr("Chat"));
    }
}

void ConferenceWindow::scrollChatToBottom()
{
    QScrollBar* scrollBar = m_chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

// 参与者列表管理
void ConferenceWindow::updateParticipantsList()
{
    m_participantsCountLabel->setText(tr("Participants (%1)").arg(m_participantCount));
    m_participantCountLabel->setText(tr("%1 participants").arg(m_participantCount));
}

void ConferenceWindow::addParticipantToList(const ConferenceManager::ParticipantInfo& participant)
{
    QListWidgetItem* item = new QListWidgetItem(m_participantsList);
    
    QString statusIcon = "🟢"; // 在线
    if (participant.audioMuted && participant.videoMuted) {
        statusIcon = "🔇"; // 静音
    } else if (participant.audioMuted) {
        statusIcon = "🔇"; // 音频静音
    } else if (participant.videoMuted) {
        statusIcon = "📹"; // 视频静音
    }
    
    item->setText(QString("%1 %2").arg(statusIcon, participant.displayName));
    item->setData(Qt::UserRole, participant.jid);
}

void ConferenceWindow::removeParticipantFromList(const QString& jid)
{
    for (int i = 0; i < m_participantsList->count(); ++i) {
        QListWidgetItem* item = m_participantsList->item(i);
        if (item && item->data(Qt::UserRole).toString() == jid) {
            delete m_participantsList->takeItem(i);
            break;
        }
    }
}

void ConferenceWindow::updateParticipantInList(const ConferenceManager::ParticipantInfo& participant)
{
    for (int i = 0; i < m_participantsList->count(); ++i) {
        QListWidgetItem* item = m_participantsList->item(i);
        if (item && item->data(Qt::UserRole).toString() == participant.jid) {
            QString statusIcon = "🟢";
            if (participant.audioMuted && participant.videoMuted) {
                statusIcon = "🔇";
            } else if (participant.audioMuted) {
                statusIcon = "🔇";
            } else if (participant.videoMuted) {
                statusIcon = "📹";
            }
            
            item->setText(QString("%1 %2").arg(statusIcon, participant.displayName));
            break;
        }
    }
}

// 状态显示
void ConferenceWindow::showConnectionStatus(const QString& status)
{
    m_connectionStatusLabel->setText(status);
}

void ConferenceWindow::showError(const QString& message)
{
    QMessageBox::warning(this, tr("Conference Error"), message);
    hideLoading();
}

void ConferenceWindow::showLoading(const QString& message)
{
    m_connectionStatusLabel->setText(message);
    m_connectionProgress->setVisible(true);
    m_connectionProgress->setRange(0, 0); // 不确定进度
}

void ConferenceWindow::hideLoading()
{
    m_connectionProgress->setVisible(false);
}

// 控制面板更新
void ConferenceWindow::updateControlButtons()
{
    updateMediaButtons();
    updateScreenShareButton();
}

void ConferenceWindow::updateMediaButtons()
{
    m_muteAudioButton->setChecked(m_isAudioMuted);
    m_muteAudioButton->setText(m_isAudioMuted ? tr("Unmute Audio") : tr("Mute Audio"));
    
    m_muteVideoButton->setChecked(m_isVideoMuted);
    m_muteVideoButton->setText(m_isVideoMuted ? tr("Start Video") : tr("Stop Video"));
}

void ConferenceWindow::updateScreenShareButton()
{
    m_screenShareButton->setChecked(m_isScreenSharing);
    m_screenShareButton->setText(m_isScreenSharing ? tr("Stop Sharing") : tr("Share Screen"));
}

// 面板显示控制
void ConferenceWindow::showChatPanel(bool show)
{
    m_chatPanel->setVisible(show);
    m_chatPanelVisible = show;
    
    if (show) {
        // 清除未读计数
        m_unreadChatCount = 0;
        updateChatUnreadIndicator();
        // m_chatManager->markAllAsRead();  // 暂时禁用
    }
}

void ConferenceWindow::showParticipantsPanel(bool show)
{
    m_participantsPanel->setVisible(show);
    m_participantsPanelVisible = show;
}

void ConferenceWindow::toggleChatPanel()
{
    showChatPanel(!m_chatPanelVisible);
    m_chatToggleButton->setChecked(m_chatPanelVisible);
}

void ConferenceWindow::toggleParticipantsPanel()
{
    showParticipantsPanel(!m_participantsPanelVisible);
    m_participantsToggleButton->setChecked(m_participantsPanelVisible);
}

void ConferenceWindow::updateConnectionStatus()
{
    // 这个方法可以定期调用来更新连接状态
    if (m_conferenceManager) {
        ConferenceManager::ConnectionState state = m_conferenceManager->connectionState();
        onConnectionStateChanged(state);
    }
}

// 事件处理
void ConferenceWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    
    // 根据窗口大小调整视频部件大小
    QSize windowSize = event->size();
    int videoWidth = qMax(160, windowSize.width() / (m_videoGridColumns + 1));
    int videoHeight = qMax(120, windowSize.height() / (m_videoGridRows + 1));
    m_videoWidgetSize = QSize(videoWidth, videoHeight);
    
    // 更新视频布局
    updateVideoLayout();
}

void ConferenceWindow::closeEvent(QCloseEvent* event)
{
    // 如果正在会议中，询问用户是否确定关闭
    if (m_isInConference) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Close Application"),
            tr("Are you sure you want to close Jitsi Meet? This will end the current conference."),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        
        if (reply == QMessageBox::No) {
            event->ignore();
            return;
        }
        
        // 离开会议
        leaveConference();
    }
    
    event->accept();
}

