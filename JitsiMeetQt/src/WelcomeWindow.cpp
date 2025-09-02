#include "WelcomeWindow.h"
#include "ConfigurationManager.h"
#include "ProtocolHandler.h"

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
    m_networkManager = new QNetworkAccessManager(this);
    
    // 初始化定时器
    m_serverCheckTimer = new QTimer(this);
    m_serverCheckTimer->setSingleShot(true);
    m_serverCheckTimer->setInterval(SERVER_CHECK_TIMEOUT);
    
    m_urlValidationTimer = new QTimer(this);
    m_urlValidationTimer->setSingleShot(true);
    m_urlValidationTimer->setInterval(URL_VALIDATION_DELAY);
    
    // 初始化UI
    initializeUI();
    initializeLayout();
    initializeAutoComplete();
    initializeConnections();
    
    // 加载数据
    loadMeetingHistory();
    loadServerList();
    
    // 恢复窗口状态
    restoreWindowState();
    
    // 更新UI状态
    updateUIState();
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
        m_urlEdit->setFocus();
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
    
    QString url = meetingData["url"].toString();
    QString displayName = meetingData["displayName"].toString();
    QString serverUrl = meetingData["serverUrl"].toString();
    
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
    
    QString url = meetingData["url"].toString();
    QString displayName = meetingData["displayName"].toString();
    QString serverUrl = meetingData["serverUrl"].toString();
    
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
                if (urlData.contains("roomName")) {
                    info += tr("房间名: %1<br>").arg(urlData["roomName"].toString());
                }
                if (urlData.contains("serverUrl")) {
                    info += tr("服务器: %1<br>").arg(urlData["serverUrl"].toString());
                }
                if (urlData.contains("displayName")) {
                    info += tr("显示名: %1<br>").arg(urlData["displayName"].toString());
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
    setWindowTitle(tr("Jitsi Meet Qt - 欢迎"));
    setWindowIcon(QIcon(":/icons/app.png"));
    setMinimumSize(800, 600);
    
    // 创建中央窗口部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    // 创建左侧面板
    m_leftPanel = new QWidget(this);
    m_leftPanel->setMinimumWidth(400);
    
    // 创建加入会议组
    m_joinGroup = new QGroupBox(tr("加入或创建会议"), this);
    
    // 创建输入控件
    m_urlLabel = new QLabel(tr("会议URL或房间名:"), this);
    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText(tr("输入会议URL或房间名"));
    
    m_displayNameLabel = new QLabel(tr("显示名称:"), this);
    m_displayNameEdit = new QLineEdit(this);
    m_displayNameEdit->setPlaceholderText(tr("输入您的显示名称"));
    
    m_serverLabel = new QLabel(tr("服务器:"), this);
    m_serverCombo = new QComboBox(this);
    m_serverCombo->setEditable(true);
    m_serverCombo->setPlaceholderText(tr("选择或输入服务器URL"));
    
    m_passwordLabel = new QLabel(tr("会议密码 (可选):"), this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("输入会议密码"));
    
    // 创建按钮
    m_joinButton = new QPushButton(tr("加入会议"), this);
    m_joinButton->setDefault(true);
    m_createButton = new QPushButton(tr("创建会议"), this);
    
    m_settingsButton = new QPushButton(tr("设置"), this);
    m_aboutButton = new QPushButton(tr("关于"), this);
    m_exitButton = new QPushButton(tr("退出"), this);
    
    // 创建右侧面板
    m_rightPanel = new QWidget(this);
    m_rightPanel->setMinimumWidth(300);
    
    // 创建历史记录组
    m_historyGroup = new QGroupBox(tr("会议历史"), this);
    m_historyList = new QListWidget(this);
    m_clearHistoryButton = new QPushButton(tr("清除历史"), this);
    
    // 创建信息组
    m_infoGroup = new QGroupBox(tr("会议信息"), this);
    m_infoText = new QTextEdit(this);
    m_infoText->setReadOnly(true);
    m_infoText->setMaximumHeight(150);
    
    // 创建状态栏
    m_statusLabel = new QLabel(tr("就绪"), this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addWidget(m_progressBar);
}

/**
 * @brief 初始化布局
 */
void WelcomeWindow::initializeLayout()
{
    // 主布局
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->addWidget(m_splitter);
    
    // 左侧面板布局
    m_leftLayout = new QVBoxLayout(m_leftPanel);
    
    // 加入会议组布局
    m_joinLayout = new QGridLayout(m_joinGroup);
    m_joinLayout->addWidget(m_urlLabel, 0, 0);
    m_joinLayout->addWidget(m_urlEdit, 0, 1);
    m_joinLayout->addWidget(m_displayNameLabel, 1, 0);
    m_joinLayout->addWidget(m_displayNameEdit, 1, 1);
    m_joinLayout->addWidget(m_serverLabel, 2, 0);
    m_joinLayout->addWidget(m_serverCombo, 2, 1);
    m_joinLayout->addWidget(m_passwordLabel, 3, 0);
    m_joinLayout->addWidget(m_passwordEdit, 3, 1);
    
    // 按钮布局
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addWidget(m_joinButton);
    m_buttonLayout->addWidget(m_createButton);
    m_joinLayout->addLayout(m_buttonLayout, 4, 0, 1, 2);
    
    // 动作按钮布局
    m_actionLayout = new QHBoxLayout();
    m_actionLayout->addWidget(m_settingsButton);
    m_actionLayout->addWidget(m_aboutButton);
    m_actionLayout->addStretch();
    m_actionLayout->addWidget(m_exitButton);
    
    // 左侧面板布局组装
    m_leftLayout->addWidget(m_joinGroup);
    m_leftLayout->addLayout(m_actionLayout);
    m_leftLayout->addStretch();
    
    // 右侧面板布局
    m_rightLayout = new QVBoxLayout(m_rightPanel);
    
    // 历史记录组布局
    m_historyLayout = new QVBoxLayout(m_historyGroup);
    m_historyLayout->addWidget(m_historyList);
    m_historyLayout->addWidget(m_clearHistoryButton);
    
    // 信息组布局
    m_infoLayout = new QVBoxLayout(m_infoGroup);
    m_infoLayout->addWidget(m_infoText);
    
    // 右侧面板布局组装
    m_rightLayout->addWidget(m_historyGroup, 1);
    m_rightLayout->addWidget(m_infoGroup);
    
    // 添加到分割器
    m_splitter->addWidget(m_leftPanel);
    m_splitter->addWidget(m_rightPanel);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 0);
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
    // URL自动完成
    m_urlModel = new QStringListModel(this);
    m_urlCompleter = new QCompleter(m_urlModel, this);
    m_urlCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_urlEdit->setCompleter(m_urlCompleter);
    
    // 显示名称自动完成
    m_nameModel = new QStringListModel(this);
    m_nameCompleter = new QCompleter(m_nameModel, this);
    m_nameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_displayNameEdit->setCompleter(m_nameCompleter);
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
    QJsonArray history = recentMeetings["meetings"].toArray();
    for (const QJsonValue& value : history) {
        QJsonObject meeting = value.toObject();
        
        QString url = meeting["url"].toString();
        QString displayName = meeting["displayName"].toString();
        QString serverUrl = meeting["serverUrl"].toString();
        qint64 timestamp = meeting["timestamp"].toVariant().toLongLong();
        
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
        QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm");
        
        QString itemText = QString("%1\n%2\n%3").arg(url, displayName, timeStr);
        
        QListWidgetItem* item = new QListWidgetItem(itemText, m_historyList);
        item->setData(Qt::UserRole, meeting);
        item->setToolTip(QString("URL: %1\n显示名: %2\n服务器: %3\n时间: %4")
                        .arg(url, displayName, serverUrl, timeStr));
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