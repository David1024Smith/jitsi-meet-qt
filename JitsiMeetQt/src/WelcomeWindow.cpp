#include "WelcomeWindow.h"
#include "ConfigurationManager.h"
#include "ProtocolHandler.h"
#include <type_traits>
#include <QApplication>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QRegularExpression>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QJsonParseError>
#include <QKeySequence>
#include <QShortcut>
#include <QHeaderView>
#include <QScrollBar>
#include <QToolTip>
#include <QWhatsThis>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QScreen>

/**
 * @brief WelcomeWindow构造函数
 * @param parent 父窗口
 */
WelcomeWindow::WelcomeWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_leftPanel(nullptr)
    , m_leftLayout(nullptr)
    , m_joinGroup(nullptr)
    , m_joinLayout(nullptr)
    , m_urlLabel(nullptr)
    , m_urlEdit(nullptr)
    , m_displayNameLabel(nullptr)
    , m_displayNameEdit(nullptr)
    , m_serverLabel(nullptr)
    , m_serverCombo(nullptr)
    , m_passwordLabel(nullptr)
    , m_passwordEdit(nullptr)
    , m_buttonLayout(nullptr)
    , m_joinButton(nullptr)
    , m_createButton(nullptr)
    , m_actionLayout(nullptr)
    , m_settingsButton(nullptr)
    , m_aboutButton(nullptr)
    , m_exitButton(nullptr)
    , m_rightPanel(nullptr)
    , m_rightLayout(nullptr)
    , m_historyGroup(nullptr)
    , m_historyLayout(nullptr)
    , m_historyList(nullptr)
    , m_clearHistoryButton(nullptr)
    , m_infoGroup(nullptr)
    , m_infoLayout(nullptr)
    , m_infoText(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
    , m_serverCheckTimer(nullptr)
    , m_urlValidationTimer(nullptr)
    , m_networkManager(nullptr)
    , m_urlCompleter(nullptr)
    , m_urlModel(nullptr)
    , m_nameCompleter(nullptr)
    , m_nameModel(nullptr)
    , m_configManager(nullptr)
    , m_protocolHandler(nullptr)
    , m_isValidatingUrl(false)
    , m_isCheckingServer(false)
{
    // 获取配置管理器和协议处理器实例
    m_configManager = ConfigurationManager::instance();
    // ProtocolHandler需要MainApplication实例，暂时设为nullptr
    m_protocolHandler = nullptr;
    
    // 初始化网络管理器
    m_networkManager = new QNetworkAccessManager();
    
    // 初始化定时器
    m_serverCheckTimer = new QTimer();
    m_serverCheckTimer->setSingleShot(true);
    m_serverCheckTimer->setInterval(SERVER_CHECK_TIMEOUT);
    
    m_urlValidationTimer = new QTimer();
    m_urlValidationTimer->setSingleShot(true);
    m_urlValidationTimer->setInterval(URL_VALIDATION_DELAY);
    
    // 写入调试信息
    FILE* debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "WelcomeWindow构造函数开始\n");
        fclose(debugFile);
    }
    
    // 初始化UI
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始初始化UI\n");
        fclose(debugFile);
    }
    initializeUI();
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "UI初始化完成\n");
        fclose(debugFile);
    }
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始初始化布局\n");
        fclose(debugFile);
    }
    initializeLayout();
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "布局初始化完成\n");
        fclose(debugFile);
    }
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始初始化自动完成\n");
        fclose(debugFile);
    }
    initializeAutoComplete();
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "自动完成初始化完成\n");
        fclose(debugFile);
    }
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始初始化连接\n");
        fclose(debugFile);
    }
    initializeConnections();
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "连接初始化完成\n");
        fclose(debugFile);
    }
    
    // 加载数据
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始加载会议历史\n");
        fclose(debugFile);
    }
    loadMeetingHistory();
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "会议历史加载完成\n");
        fclose(debugFile);
    }
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始加载服务器列表\n");
        fclose(debugFile);
    }
    loadServerList();
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "服务器列表加载完成\n");
        fclose(debugFile);
    }
    
    // 恢复窗口状态
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始恢复窗口状态\n");
        fclose(debugFile);
    }
    restoreWindowState();
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "窗口状态恢复完成\n");
        fclose(debugFile);
    }
    
    // 更新UI状态
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始更新UI状态\n");
        fclose(debugFile);
    }
    updateUIState();
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "UI状态更新完成\n");
        fclose(debugFile);
    }
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "WelcomeWindow构造函数完成\n");
        fclose(debugFile);
    }
}

/**
 * @brief WelcomeWindow析构函数
 */
WelcomeWindow::~WelcomeWindow()
{
    // 保存窗口状态
    saveWindowState();
}

/**
 * @brief 设置会议URL或房间名
 * @param url 会议URL或房间名
 */
void WelcomeWindow::setMeetingUrl(const QString& url)
{
    if (m_urlEdit) {
        m_urlEdit->setText(url);
        onUrlChanged(url);
    }
}

/**
 * @brief 获取当前输入的会议URL或房间名
 * @return 会议URL或房间名
 */
QString WelcomeWindow::getMeetingUrl() const
{
    return m_urlEdit ? m_urlEdit->text().trimmed() : QString();
}

/**
 * @brief 设置显示名称
 * @param displayName 显示名称
 */
void WelcomeWindow::setDisplayName(const QString& displayName)
{
    if (m_displayNameEdit) {
        m_displayNameEdit->setText(displayName);
    }
}

/**
 * @brief 获取显示名称
 * @return 显示名称
 */
QString WelcomeWindow::getDisplayName() const
{
    return m_displayNameEdit ? m_displayNameEdit->text().trimmed() : QString();
}

/**
 * @brief 设置服务器URL
 * @param serverUrl 服务器URL
 */
void WelcomeWindow::setServerUrl(const QString& serverUrl)
{
    if (m_serverCombo) {
        int index = m_serverCombo->findText(serverUrl);
        if (index >= 0) {
            m_serverCombo->setCurrentIndex(index);
        } else {
            m_serverCombo->setEditText(serverUrl);
        }
    }
}

/**
 * @brief 获取服务器URL
 * @return 服务器URL
 */
QString WelcomeWindow::getServerUrl() const
{
    return m_serverCombo ? m_serverCombo->currentText().trimmed() : QString();
}

/**
 * @brief 刷新会议历史列表
 */
void WelcomeWindow::refreshMeetingHistory()
{
    loadMeetingHistory();
    updateHistoryDisplay();
}

/**
 * @brief 清除输入字段
 */
void WelcomeWindow::clearInputs()
{
    if (m_urlEdit) {
        m_urlEdit->clear();
    }
    if (m_passwordEdit) {
        m_passwordEdit->clear();
    }
    updateUIState();
}

/**
 * @brief 窗口关闭事件处理
 * @param event 关闭事件
 */
void WelcomeWindow::closeEvent(QCloseEvent *event)
{
    saveWindowState();
    emit windowClosed();
    QMainWindow::closeEvent(event);
}

/**
 * @brief 窗口显示事件处理
 * @param event 显示事件
 */
void WelcomeWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    
    // 聚焦到URL输入框
    if (m_urlEdit) {
        m_urlEdit->setFocus(Qt::OtherFocusReason);
        m_urlEdit->selectAll();
    }
    
    // 刷新会议历史
    refreshMeetingHistory();
}

/**
 * @brief 窗口大小改变事件处理
 * @param event 大小改变事件
 */
void WelcomeWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    // 保存窗口大小
    if (m_configManager) {
        m_configManager->setMainWindowSize(size());
    }
}

/**
 * @brief 加入会议按钮点击处理
 */
void WelcomeWindow::onJoinMeeting()
{
    if (!validateInput()) {
        return;
    }
    
    QString url = getMeetingUrl();
    QString displayName = getDisplayName();
    QString password = m_passwordEdit ? m_passwordEdit->text().trimmed() : QString();
    
    // 添加到历史记录
    addToHistory(url, displayName, getServerUrl());
    
    // 发射加入会议信号
    emit joinMeetingRequested(url, displayName, password);
}

/**
 * @brief 创建会议按钮点击处理
 */
void WelcomeWindow::onCreateMeeting()
{
    if (!validateInput()) {
        return;
    }
    
    QString roomName = getMeetingUrl();
    QString serverUrl = getServerUrl();
    QString displayName = getDisplayName();
    QString password = m_passwordEdit ? m_passwordEdit->text().trimmed() : QString();
    
    // 添加到历史记录
    addToHistory(roomName, displayName, serverUrl);
    
    // 发射创建会议信号
    emit createMeetingRequested(roomName, serverUrl, displayName, password);
}

/**
 * @brief 设置按钮点击处理
 */
void WelcomeWindow::onSettings()
{
    emit settingsRequested();
}

/**
 * @brief 关于按钮点击处理
 */
void WelcomeWindow::onAbout()
{
    QMessageBox::about(this, tr("关于 Jitsi Meet Qt"),
        tr("<h3>Jitsi Meet Qt</h3>"
           "<p>版本: 1.0.0</p>"
           "<p>基于Qt的Jitsi Meet桌面客户端</p>"
           "<p>Copyright © 2024</p>"
           "<p>使用Qt %1构建</p>").arg(QT_VERSION_STR));
}

/**
 * @brief 退出按钮点击处理
 */
void WelcomeWindow::onExit()
{
    QApplication::quit();
}

/**
 * @brief 会议历史项目双击处理
 * @param item 被双击的项目
 */
void WelcomeWindow::onHistoryItemDoubleClicked(QListWidgetItem* item)
{
    if (!item) {
        return;
    }
    
    // 从项目数据中获取会议信息
    QJsonObject meetingData = item->data(Qt::UserRole).toJsonObject();
    
    QString url = meetingData.value("url").toString();
    QString displayName = meetingData.value("displayName").toString();
    QString serverUrl = meetingData.value("serverUrl").toString();
    
    // 设置到输入框
    setMeetingUrl(url);
    setDisplayName(displayName);
    setServerUrl(serverUrl);
    
    // 自动加入会议
    onJoinMeeting();
}

/**
 * @brief 会议历史项目选择改变处理
 */
void WelcomeWindow::onHistorySelectionChanged()
{
    if (!m_historyList) {
        return;
    }
    
    QListWidgetItem* currentItem = m_historyList->currentItem();
    if (!currentItem) {
        return;
    }
    
    // 从项目数据中获取会议信息
    QJsonObject meetingData = currentItem->data(Qt::UserRole).toJsonObject();
    
    QString url = meetingData.value("url").toString();
    QString displayName = meetingData.value("displayName").toString();
    QString serverUrl = meetingData.value("serverUrl").toString();
    
    // 设置到输入框（但不自动加入）
    setMeetingUrl(url);
    setDisplayName(displayName);
    setServerUrl(serverUrl);
}

/**
 * @brief 服务器选择改变处理
 * @param index 选择的索引
 */
void WelcomeWindow::onServerChanged(int index)
{
    Q_UNUSED(index)
    
    QString serverUrl = getServerUrl();
    if (!serverUrl.isEmpty() && serverUrl != m_lastCheckedServer) {
        checkServerAvailability(serverUrl);
    }
    
    updateUIState();
}

/**
 * @brief URL输入改变处理
 * @param text 输入的文本
 */
void WelcomeWindow::onUrlChanged(const QString& text)
{
    Q_UNUSED(text)
    
    // 延迟验证URL
    if (m_urlValidationTimer) {
        m_urlValidationTimer->start();
    }
    
    updateUIState();
}

/**
 * @brief 显示名称输入改变处理
 * @param text 输入的文本
 */
void WelcomeWindow::onDisplayNameChanged(const QString& text)
{
    Q_UNUSED(text)
    updateUIState();
}

/**
 * @brief 服务器可用性检查完成处理
 * @param available 服务器是否可用
 * @param serverUrl 服务器URL
 */
void WelcomeWindow::onServerAvailabilityChecked(bool available, const QString& serverUrl)
{
    if (m_statusLabel) {
        if (available) {
            m_statusLabel->setText(tr("服务器 %1 可用").arg(serverUrl));
        } else {
            m_statusLabel->setText(tr("服务器 %1 不可用").arg(serverUrl));
        }
    }
    
    m_isCheckingServer = false;
    updateUIState();
}

/**
 * @brief 服务器检查超时处理
 */
void WelcomeWindow::onServerCheckTimeout()
{
    m_isCheckingServer = false;
    if (m_statusLabel) {
        m_statusLabel->setText(tr("服务器检查超时"));
    }
    updateUIState();
}

/**
 * @brief URL验证定时器超时处理
 */
void WelcomeWindow::onUrlValidationTimeout()
{
    QString url = getMeetingUrl();
    if (!url.isEmpty() && url != m_lastValidatedUrl) {
        m_lastValidatedUrl = url;
        
        // 解析URL
        QJsonObject urlData = parseMeetingUrl(url);
        
        // 更新信息显示
        if (m_infoText) {
            QString info;
            if (!urlData.isEmpty()) {
                info += tr("<b>会议信息:</b><br>");
                if (urlData.contains(QStringLiteral("roomName"))) {
                    info += tr("房间名: %1<br>").arg(urlData.value("roomName").toString());
                }
                if (urlData.contains(QStringLiteral("serverUrl"))) {
                    info += tr("服务器: %1<br>").arg(urlData.value("serverUrl").toString());
                }
                if (urlData.contains(QStringLiteral("displayName"))) {
                    info += tr("显示名: %1<br>").arg(urlData.value("displayName").toString());
                }
            } else {
                info = tr("输入会议URL或房间名");
            }
            m_infoText->setHtml(info);
        }
    }
}

/**
 * @brief 初始化用户界面
 */
void WelcomeWindow::initializeUI()
{
    // 设置窗口属性
    setWindowTitle(tr("Jitsi Meet Qt"));
    setWindowIcon(QIcon(":/icons/app.svg"));
    setMinimumSize(QSize(800, 600));
    
    // 创建中央窗口部件
    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);
    
    // 设置窗口背景为蓝色
    m_centralWidget->setStyleSheet("background-color: #0056E0; background-image: url(:/images/background.png); background-position: center; background-repeat: no-repeat; background-attachment: fixed;");
    
    // 创建中央面板
    m_leftPanel = new QWidget();
    m_leftPanel->setObjectName("centralPanel");
    m_leftPanel->setStyleSheet("#centralPanel { background-color: transparent; }");
    
    // 创建标题标签
    QLabel* titleLabel = new QLabel(tr("输入会议名或链接地址"));
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // 创建输入控件
    m_urlEdit = new QLineEdit();
    m_urlEdit->setPlaceholderText(tr("输入会议URL或房间名"));
    m_urlEdit->setMinimumHeight(40);
    m_urlEdit->setStyleSheet("QLineEdit { background-color: white; border-radius: 4px; padding: 8px; font-size: 14px; border: none; }");
    
    // 创建按钮
    m_joinButton = new QPushButton(tr("开始"));
    m_joinButton->setDefault(true);
    m_joinButton->setMinimumHeight(40);
    m_joinButton->setMinimumWidth(80);
    m_joinButton->setStyleSheet("QPushButton { background-color: #0074E0; color: white; border-radius: 4px; font-weight: bold; font-size: 14px; border: none; }"
                              "QPushButton:hover { background-color: #0063C0; }"
                              "QPushButton:pressed { background-color: #0052A0; }");
    
    // 创建最近会议标签
    QLabel* recentLabel = new QLabel(tr("您最近加入的会议"));
    recentLabel->setStyleSheet("color: white; font-size: 14px;");
    
    // 创建历史记录列表
    m_historyList = new QListWidget();
    m_historyList->setStyleSheet("QListWidget { background-color: transparent; border: none; }"
                               "QListWidget::item { background-color: rgba(0, 0, 0, 0.2); color: white; border-radius: 4px; padding: 0; margin: 5px; }"
                               "QListWidget::item:hover { background-color: rgba(0, 0, 0, 0.3); }"
                               "QListWidget::item:selected { background-color: rgba(0, 0, 0, 0.4); }");
    m_historyList->setMaximumHeight(200);
    m_historyList->setSpacing(5);
    m_historyList->setUniformItemSizes(false);
    m_historyList->setWordWrap(true);
    
    // 创建状态栏
    m_statusLabel = new QLabel(tr("就绪"));
    m_statusLabel->setStyleSheet("color: rgba(255, 255, 255, 0.7);");
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addWidget(m_progressBar);
    statusBar()->setStyleSheet("QStatusBar { background-color: #0056E0; color: white; }");
    
    // 创建显示名称控件（即使在简化UI中也需要创建以避免空指针）
    m_displayNameLabel = new QLabel(tr("显示名称:"), this);
    
    FILE* debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始创建m_displayNameEdit\n");
        fclose(debugFile);
    }
    
    m_displayNameEdit = new QLineEdit(this);
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "m_displayNameEdit创建完成，指针地址: %p\n", (void*)m_displayNameEdit);
        fclose(debugFile);
    }
    
    m_displayNameEdit->setPlaceholderText(tr("输入您的显示名称"));
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "m_displayNameEdit占位符文本设置完成\n");
        fclose(debugFile);
    }
    
    // 隐藏显示名称控件（在简化UI中不显示）
    m_displayNameLabel->setVisible(false);
    m_displayNameEdit->setVisible(false);
    
    // 隐藏其他不需要的控件
    m_serverLabel = nullptr;
    m_serverCombo = nullptr;
    m_passwordLabel = nullptr;
    m_passwordEdit = nullptr;
    m_createButton = nullptr;
    m_settingsButton = nullptr;
    m_aboutButton = nullptr;
    m_exitButton = nullptr;
    m_rightPanel = nullptr;
    m_historyGroup = nullptr;
    m_clearHistoryButton = nullptr;
    m_infoGroup = nullptr;
    m_infoText = nullptr;
    m_splitter = nullptr;
    m_joinGroup = nullptr;
}

/**
 * @brief 初始化布局
 */
void WelcomeWindow::initializeLayout()
{
    // 主布局
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // 创建中央面板布局
    m_leftLayout = new QVBoxLayout(m_leftPanel);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftLayout->setSpacing(15);
    m_leftLayout->setAlignment(Qt::AlignCenter);
    
    // 添加标题标签
    QLabel* titleLabel = new QLabel(tr("输入会议名或链接地址"));
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    m_leftLayout->addWidget(titleLabel);
    
    // 创建URL输入布局
    QHBoxLayout* urlLayout = new QHBoxLayout();
    urlLayout->addWidget(m_urlEdit, 1);
    urlLayout->addWidget(m_joinButton);
    m_leftLayout->addLayout(urlLayout);
    
    // 添加空白区域
    m_leftLayout->addStretch();
    
    // 添加最近会议标签
    QLabel* recentLabel = new QLabel(tr("您最近加入的会议"));
    recentLabel->setStyleSheet("color: white; font-size: 14px;");
    m_leftLayout->addWidget(recentLabel);
    
    // 添加历史记录列表
    m_leftLayout->addWidget(m_historyList);
    
    // 添加空白区域
    m_leftLayout->addStretch();
    
    // 添加中央面板到主布局
    m_mainLayout->addWidget(m_leftPanel, 1);
    
    // 设置窗口布局
    m_centralWidget->setLayout(m_mainLayout);
}

/**
 * @brief 初始化连接
 */
void WelcomeWindow::initializeConnections()
{
    // 按钮连接
    connect(m_joinButton, &QPushButton::clicked, this, &WelcomeWindow::onJoinMeeting);
    connect(m_createButton, &QPushButton::clicked, this, &WelcomeWindow::onCreateMeeting);
    connect(m_settingsButton, &QPushButton::clicked, this, &WelcomeWindow::onSettings);
    connect(m_aboutButton, &QPushButton::clicked, this, &WelcomeWindow::onAbout);
    connect(m_exitButton, &QPushButton::clicked, this, &WelcomeWindow::onExit);
    connect(m_clearHistoryButton, &QPushButton::clicked, this, [this]() {
        if (m_configManager) {
            m_configManager->clearMeetingHistory();
            refreshMeetingHistory();
        }
    });
    
    // 输入控件连接
    connect(m_urlEdit, &QLineEdit::textChanged, this, &WelcomeWindow::onUrlChanged);
    connect(m_displayNameEdit, &QLineEdit::textChanged, this, &WelcomeWindow::onDisplayNameChanged);
    connect(m_serverCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WelcomeWindow::onServerChanged);
    
    // 历史记录连接
    connect(m_historyList, &QListWidget::itemDoubleClicked, this, &WelcomeWindow::onHistoryItemDoubleClicked);
    connect(m_historyList, &QListWidget::itemSelectionChanged, this, &WelcomeWindow::onHistorySelectionChanged);
    
    // 定时器连接
    connect(m_serverCheckTimer, &QTimer::timeout, this, &WelcomeWindow::onServerCheckTimeout);
    connect(m_urlValidationTimer, &QTimer::timeout, this, &WelcomeWindow::onUrlValidationTimeout);
    
    // 回车键连接
    connect(m_urlEdit, &QLineEdit::returnPressed, this, &WelcomeWindow::onJoinMeeting);
    connect(m_displayNameEdit, &QLineEdit::returnPressed, this, &WelcomeWindow::onJoinMeeting);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &WelcomeWindow::onJoinMeeting);
}

/**
 * @brief 初始化自动完成
 */
void WelcomeWindow::initializeAutoComplete()
{
    FILE* debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始创建URL模型\n");
        fclose(debugFile);
    }
    
    // URL自动完成
    m_urlModel = new QStringListModel(this);
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "URL模型创建完成，开始创建URL完成器\n");
        fclose(debugFile);
    }
    
    m_urlCompleter = new QCompleter(m_urlModel, this);
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "URL完成器创建完成，设置大小写敏感性\n");
        fclose(debugFile);
    }
    
    m_urlCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始设置URL编辑器的完成器\n");
        fclose(debugFile);
    }
    
    m_urlEdit->setCompleter(m_urlCompleter);
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "URL完成器设置完成，开始创建名称模型\n");
        fclose(debugFile);
    }
    
    // 显示名称自动完成
    m_nameModel = new QStringListModel(this);
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "名称模型创建完成，开始创建名称完成器\n");
        fclose(debugFile);
    }
    
    m_nameCompleter = new QCompleter(m_nameModel, this);
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "名称完成器创建完成，设置大小写敏感性\n");
        fclose(debugFile);
    }
    
    m_nameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "开始设置显示名称编辑器的完成器\n");
        fclose(debugFile);
    }
    
    // 检查指针是否有效
    if (m_displayNameEdit == nullptr) {
        debugFile = fopen("debug_startup.txt", "a");
        if (debugFile) {
            fprintf(debugFile, "错误：m_displayNameEdit为空指针\n");
            fclose(debugFile);
        }
        return;
    }
    
    if (m_nameCompleter == nullptr) {
        debugFile = fopen("debug_startup.txt", "a");
        if (debugFile) {
            fprintf(debugFile, "错误：m_nameCompleter为空指针\n");
            fclose(debugFile);
        }
        return;
    }
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "指针检查通过，开始调用setCompleter\n");
        fclose(debugFile);
    }
    
    try {
        m_displayNameEdit->setCompleter(m_nameCompleter);
        
        debugFile = fopen("debug_startup.txt", "a");
        if (debugFile) {
            fprintf(debugFile, "setCompleter调用成功\n");
            fclose(debugFile);
        }
    } catch (...) {
        debugFile = fopen("debug_startup.txt", "a");
        if (debugFile) {
            fprintf(debugFile, "setCompleter调用时发生异常\n");
            fclose(debugFile);
        }
        return;
    }
    
    debugFile = fopen("debug_startup.txt", "a");
    if (debugFile) {
        fprintf(debugFile, "自动完成初始化全部完成\n");
        fclose(debugFile);
    }
}

/**
 * @brief 加载会议历史
 */
void WelcomeWindow::loadMeetingHistory()
{
    if (!m_configManager || !m_historyList) {
        return;
    }
    
    m_historyList->clear();
    
    QJsonObject recentMeetings = m_configManager->getRecentMeetings();
    QJsonArray history = recentMeetings.value("meetings").toArray();
    for (const QJsonValue& value : history) {
        QJsonObject meeting = value.toObject();
        
        QString url = meeting.value("url").toString();
        QString displayName = meeting.value("displayName").toString();
        QString serverUrl = meeting.value("serverUrl").toString();
        qint64 timestamp = meeting.value("timestamp").toVariant().toLongLong();
        
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
        QString timeStr = dateTime.toString("yyyy-MM-dd");
        
        // 创建自定义显示格式，与Electron版本一致
        QString itemText = url;
        
        QListWidgetItem* item = new QListWidgetItem(m_historyList);
        item->setData(Qt::UserRole, meeting);
        item->setText(itemText);
        
        // 创建自定义小部件来显示会议信息
    QWidget* itemWidget = new QWidget();
    QVBoxLayout* itemLayout = new QVBoxLayout(itemWidget);
    itemLayout->setContentsMargins(5, 5, 5, 5);
    itemLayout->setSpacing(2);
    
    // 添加会议URL标签
    QLabel* urlLabel = new QLabel(url, itemWidget);
    urlLabel->setStyleSheet("color: white; font-weight: bold;");
    itemLayout->addWidget(urlLabel);
    
    // 添加时间标签
    QLabel* timeLabel = new QLabel(timeStr, itemWidget);
    timeLabel->setStyleSheet("color: rgba(255, 255, 255, 0.7); font-size: 11px;");
    itemLayout->addWidget(timeLabel);
    
    m_historyList->setItemWidget(item, itemWidget);
        
        // 设置项目高度
        item->setSizeHint(QSize(item->sizeHint().width(), 60));
        
        // 设置工具提示
        item->setToolTip(QString("URL: %1\n显示名: %2\n服务器: %3\n时间: %4")
                        .arg(url, displayName, serverUrl, dateTime.toString("yyyy-MM-dd hh:mm")));
    }
    
    // 更新自动完成数据
    QStringList urls, names;
    for (const QJsonValue& value : history) {
        QJsonObject meeting = value.toObject();
        QString url = meeting["url"].toString();
        QString displayName = meeting["displayName"].toString();
        
        if (!url.isEmpty() && !urls.contains(url)) {
            urls.append(url);
        }
        if (!displayName.isEmpty() && !names.contains(displayName)) {
            names.append(displayName);
        }
    }
    
    if (m_urlModel) {
        m_urlModel->setStringList(urls);
    }
    if (m_nameModel) {
        m_nameModel->setStringList(names);
    }
}

/**
 * @brief 加载服务器列表
 */
void WelcomeWindow::loadServerList()
{
    if (!m_configManager || !m_serverCombo) {
        return;
    }
    
    m_serverCombo->clear();
    
    // 添加默认服务器
    QString defaultServer = m_configManager->getDefaultServerUrl();
    if (!defaultServer.isEmpty()) {
        m_serverCombo->addItem(defaultServer);
    }
    
    // 添加常用服务器
    QStringList servers;
    servers << "meet.jit.si" << "8x8.vc" << "jitsi.riot.im";
    
    for (const QString& server : servers) {
        if (server != defaultServer) {
            m_serverCombo->addItem(server);
        }
    }
    
    // 设置当前服务器
    if (!defaultServer.isEmpty()) {
        m_serverCombo->setCurrentText(defaultServer);
    }
}

/**
 * @brief 保存窗口状态
 */
void WelcomeWindow::saveWindowState()
{
    if (!m_configManager) {
        return;
    }
    
    m_configManager->setMainWindowSize(size());
    m_configManager->setMainWindowPosition(pos());
    m_configManager->setMainWindowMaximized(isMaximized());
    
    if (m_splitter) {
        m_configManager->setValue("WelcomeWindow/splitterState", m_splitter->saveState());
    }
}

/**
 * @brief 恢复窗口状态
 */
void WelcomeWindow::restoreWindowState()
{
    if (!m_configManager) {
        return;
    }
    
    // 恢复窗口大小和位置
    QSize size = m_configManager->getMainWindowSize();
    if (size.isValid()) {
        resize(size);
    }
    
    QPoint pos = m_configManager->getMainWindowPosition();
    if (!pos.isNull()) {
        move(pos);
    }
    
    if (m_configManager->isMainWindowMaximized()) {
        showMaximized();
    }
    
    // 恢复分割器状态
    if (m_splitter) {
        QByteArray splitterState = m_configManager->getValue("WelcomeWindow/splitterState").toByteArray();
        if (!splitterState.isEmpty()) {
            m_splitter->restoreState(splitterState);
        }
    }
    
    // 恢复默认显示名称
    QString defaultDisplayName = m_configManager->getDefaultDisplayName();
    if (!defaultDisplayName.isEmpty()) {
        setDisplayName(defaultDisplayName);
    }
}

/**
 * @brief 验证输入
 * @return 输入是否有效
 */
bool WelcomeWindow::validateInput()
{
    QString url = getMeetingUrl();
    QString displayName = getDisplayName();
    
    if (url.isEmpty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入会议URL或房间名"));
        m_urlEdit->setFocus();
        return false;
    }
    
    if (displayName.isEmpty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入显示名称"));
        m_displayNameEdit->setFocus();
        return false;
    }
    
    return true;
}

/**
 * @brief 解析会议URL
 * @param url 会议URL
 * @return 解析结果
 */
QJsonObject WelcomeWindow::parseMeetingUrl(const QString& url)
{
    QJsonObject result;
    
    if (url.isEmpty()) {
        return result;
    }
    
    // 使用协议处理器解析URL
    if (m_protocolHandler) {
        ProtocolHandler::MeetingInfo meetingInfo = m_protocolHandler->parseProtocolUrl(url);
        if (meetingInfo.isValid) {
            QJsonObject urlData;
            urlData["roomName"] = meetingInfo.roomName;
            urlData["serverUrl"] = meetingInfo.serverUrl;
            urlData["fullUrl"] = meetingInfo.fullUrl;
            urlData["displayName"] = meetingInfo.displayName;
            return urlData;
        }
    }
    
    // 简单的URL解析
    QUrl qurl(url);
    if (qurl.isValid() && !qurl.host().isEmpty()) {
        result["serverUrl"] = qurl.host();
        QString path = qurl.path();
        if (path.startsWith("/")) {
            path = path.mid(1);
        }
        if (!path.isEmpty()) {
            result["roomName"] = path;
        }
    } else {
        // 假设是房间名
        result["roomName"] = url;
        result["serverUrl"] = getServerUrl();
    }
    
    return result;
}

/**
 * @brief 检查服务器可用性
 * @param serverUrl 服务器URL
 */
void WelcomeWindow::checkServerAvailability(const QString& serverUrl)
{
    if (serverUrl.isEmpty() || m_isCheckingServer) {
        return;
    }
    
    m_isCheckingServer = true;
    m_lastCheckedServer = serverUrl;
    
    if (m_statusLabel) {
        m_statusLabel->setText(tr("检查服务器 %1...").arg(serverUrl));
    }
    
    if (m_progressBar) {
        m_progressBar->setVisible(true);
        m_progressBar->setRange(0, 0); // 无限进度条
    }
    
    // 启动超时定时器
    if (m_serverCheckTimer) {
        m_serverCheckTimer->start();
    }
    
    // 构建检查URL
    QString checkUrl = serverUrl;
    if (!checkUrl.startsWith("http")) {
        checkUrl = "https://" + checkUrl;
    }
    if (!checkUrl.endsWith("/")) {
        checkUrl += "/";
    }
    checkUrl += "config.js";
    
    // 发送网络请求
    QUrl url(checkUrl);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "JitsiMeetQt/1.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, serverUrl]() {
        reply->deleteLater();
        
        if (m_serverCheckTimer) {
            m_serverCheckTimer->stop();
        }
        
        if (m_progressBar) {
            m_progressBar->setVisible(false);
        }
        
        bool available = (reply->error() == QNetworkReply::NoError);
        onServerAvailabilityChecked(available, serverUrl);
    });
}

/**
 * @brief 更新UI状态
 */
void WelcomeWindow::updateUIState()
{
    QString url = getMeetingUrl();
    QString displayName = getDisplayName();
    
    bool hasUrl = !url.isEmpty();
    bool hasDisplayName = !displayName.isEmpty();
    bool canJoin = hasUrl && hasDisplayName && !m_isCheckingServer;
    
    if (m_joinButton) {
        m_joinButton->setEnabled(canJoin);
    }
    
    if (m_createButton) {
        m_createButton->setEnabled(canJoin);
    }
}

/**
 * @brief 更新会议历史显示
 */
void WelcomeWindow::updateHistoryDisplay()
{
    // 历史记录已在loadMeetingHistory中更新
    // 这里可以添加额外的显示逻辑
}

/**
 * @brief 添加到会议历史
 * @param url 会议URL
 * @param displayName 显示名称
 * @param serverUrl 服务器URL
 */
void WelcomeWindow::addToHistory(const QString& url, const QString& displayName, const QString& serverUrl)
{
    if (!m_configManager || url.isEmpty()) {
        return;
    }
    
    // 从URL中提取房间名称
    QString roomName = url;
    if (url.contains("/")) {
        roomName = url.split("/").last();
    }
    
    m_configManager->addMeetingRecord(roomName, serverUrl, displayName);
}
