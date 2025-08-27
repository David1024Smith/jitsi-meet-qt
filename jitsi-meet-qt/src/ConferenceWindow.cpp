#include "ConferenceWindow.h"
#include "JitsiConstants.h"
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QDebug>

ConferenceWindow::ConferenceWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_navigationBar(nullptr)
    , m_webView(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_errorLabel(nullptr)
    , m_progressTimer(nullptr)
    , m_serverUrl(JitsiConstants::DEFAULT_SERVER_URL)
    , m_isLoading(false)
{
    setupUI();
    setupWebEngine();
    setupConnections();
    applyStyles();
    
    // 设置窗口属性
    setWindowTitle(tr("welcome_title"));
    setMinimumSize(800, 600);
    resize(1200, 800);
}

ConferenceWindow::~ConferenceWindow()
{
    // Qt handles cleanup automatically for child widgets
}

void ConferenceWindow::setupUI()
{
    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主布局
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // 创建导航栏
    m_navigationBar = new NavigationBar(this);
    m_navigationBar->setButtonConfiguration(NavigationBar::BackButton | NavigationBar::SettingsButton);
    m_navigationBar->setTitle(tr("welcome_title"));
    
    // 创建Web视图
    m_webView = new QWebEngineView(this);
    
    // 创建进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat(tr("loading_conference"));
    
    // 创建状态标签
    m_statusLabel = new QLabel(this);
    m_statusLabel->setText(tr("loading_conference"));
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setVisible(false);
    
    // 创建错误标签
    m_errorLabel = new QLabel(this);
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setVisible(false);
    m_errorLabel->setWordWrap(true);
    
    // 创建进度定时器
    m_progressTimer = new QTimer(this);
    m_progressTimer->setSingleShot(true);
    m_progressTimer->setInterval(3000); // 3秒后隐藏进度条
    
    // 添加到布局
    m_mainLayout->addWidget(m_navigationBar);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addWidget(m_statusLabel);
    m_mainLayout->addWidget(m_errorLabel);
    m_mainLayout->addWidget(m_webView, 1); // 给WebView最大的空间
}

void ConferenceWindow::setupWebEngine()
{
    // 获取WebEngine设置
    QWebEngineSettings* settings = m_webView->settings();
    
    // 启用必要的Web功能
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, true);
    settings->setAttribute(QWebEngineSettings::AllowGeolocationOnInsecureOrigins, true);
    
    // 启用媒体功能
    settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
    settings->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, true);
    
    // 设置用户代理
    QWebEngineProfile* profile = m_webView->page()->profile();
    QString userAgent = profile->httpUserAgent();
    userAgent += " JitsiMeetQt/1.0.0";
    profile->setHttpUserAgent(userAgent);
    
    // 启用持久化Cookie
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
}

void ConferenceWindow::setupConnections()
{
    // 导航栏连接
    connect(m_navigationBar, &NavigationBar::backClicked, 
            this, &ConferenceWindow::onBackButtonClicked);
    
    // WebView连接
    connect(m_webView, &QWebEngineView::loadStarted, 
            this, &ConferenceWindow::onLoadStarted);
    connect(m_webView, &QWebEngineView::loadProgress, 
            this, &ConferenceWindow::onLoadProgress);
    connect(m_webView, &QWebEngineView::loadFinished, 
            this, &ConferenceWindow::onLoadFinished);
    
    // 进度定时器连接
    connect(m_progressTimer, &QTimer::timeout, 
            this, &ConferenceWindow::hideProgressBar);
}

void ConferenceWindow::applyStyles()
{
    setStyleSheet(
        "ConferenceWindow {"
        "    background-color: #ffffff;"
        "}"
        
        "QProgressBar {"
        "    border: 1px solid #ccc;"
        "    border-radius: 4px;"
        "    text-align: center;"
        "    font-size: 12px;"
        "    height: 20px;"
        "    margin: 5px;"
        "}"
        
        "QProgressBar::chunk {"
        "    background-color: #007bff;"
        "    border-radius: 3px;"
        "}"
        
        "QLabel#statusLabel {"
        "    color: #666;"
        "    font-size: 14px;"
        "    padding: 10px;"
        "}"
        
        "QLabel#errorLabel {"
        "    color: #dc3545;"
        "    font-size: 14px;"
        "    padding: 20px;"
        "    background-color: #f8d7da;"
        "    border: 1px solid #f5c6cb;"
        "    border-radius: 4px;"
        "    margin: 10px;"
        "}"
    );
}

void ConferenceWindow::loadConference(const QString& url)
{
    if (url.isEmpty()) {
        showError(tr("conference_error"));
        return;
    }
    
    QString jitsiUrl = buildJitsiUrl(url);
    
    if (!isValidUrl(jitsiUrl)) {
        showError(tr("conference_error") + ": " + url);
        return;
    }
    
    m_currentUrl = jitsiUrl;
    
    qDebug() << "Loading conference:" << m_currentUrl;
    
    // 显示加载状态
    showLoading();
    
    // 加载URL
    m_webView->load(QUrl(m_currentUrl));
}

void ConferenceWindow::setServerUrl(const QString& serverUrl)
{
    if (!serverUrl.isEmpty() && serverUrl != m_serverUrl) {
        m_serverUrl = serverUrl;
        qDebug() << "Server URL updated to:" << m_serverUrl;
    }
}

QString ConferenceWindow::currentUrl() const
{
    return m_currentUrl;
}

void ConferenceWindow::onLoadStarted()
{
    m_isLoading = true;
    showLoading();
    qDebug() << "Conference load started";
}

void ConferenceWindow::onLoadProgress(int progress)
{
    if (m_progressBar && m_progressBar->isVisible()) {
        m_progressBar->setValue(progress);
        
        if (progress < 100) {
            m_progressBar->setFormat(QString(tr("loading_conference") + " %1%").arg(progress));
        } else {
            m_progressBar->setFormat(tr("loading_conference"));
        }
    }
}

void ConferenceWindow::onLoadFinished(bool success)
{
    m_isLoading = false;
    
    if (success) {
        qDebug() << "Conference loaded successfully";
        hideLoading();
        
        // 启动定时器，3秒后隐藏进度条
        m_progressTimer->start();
        
        // 发射会议加入信号
        emit conferenceJoined(m_currentUrl);
        
    } else {
        qDebug() << "Conference load failed";
        showError(tr("conference_error"));
        emit loadingError(tr("conference_error"));
    }
}

void ConferenceWindow::onBackButtonClicked()
{
    // 确认是否要离开会议
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "离开会议",
        "确定要离开当前会议吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        // 停止加载
        if (m_isLoading) {
            m_webView->stop();
        }
        
        // 清空当前URL
        m_currentUrl.clear();
        
        // 发射返回信号
        emit backToWelcome();
    }
}

void ConferenceWindow::hideProgressBar()
{
    if (m_progressBar) {
        m_progressBar->setVisible(false);
    }
}

void ConferenceWindow::closeEvent(QCloseEvent* event)
{
    // 如果正在会议中，询问用户是否确定关闭
    if (!m_currentUrl.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "关闭应用程序",
            "确定要关闭Jitsi Meet吗？这将结束当前会议。",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        
        if (reply == QMessageBox::No) {
            event->ignore();
            return;
        }
    }
    
    // 停止WebView
    if (m_webView) {
        m_webView->stop();
    }
    
    event->accept();
}

void ConferenceWindow::showError(const QString& message)
{
    hideLoading();
    
    if (m_errorLabel) {
        m_errorLabel->setText(message);
        m_errorLabel->setVisible(true);
    }
    
    // 隐藏WebView
    if (m_webView) {
        m_webView->setVisible(false);
    }
}

void ConferenceWindow::showLoading()
{
    // 隐藏错误标签
    if (m_errorLabel) {
        m_errorLabel->setVisible(false);
    }
    
    // 显示WebView
    if (m_webView) {
        m_webView->setVisible(true);
    }
    
    // 显示进度条和状态
    if (m_progressBar) {
        m_progressBar->setValue(0);
        m_progressBar->setVisible(true);
    }
    
    if (m_statusLabel) {
        m_statusLabel->setText("正在连接到会议...");
        m_statusLabel->setVisible(true);
    }
}

void ConferenceWindow::hideLoading()
{
    if (m_statusLabel) {
        m_statusLabel->setVisible(false);
    }
}

QString ConferenceWindow::buildJitsiUrl(const QString& input)
{
    QString trimmedInput = input.trimmed();
    
    // 如果已经是完整的URL，直接返回
    if (trimmedInput.startsWith("http://") || trimmedInput.startsWith("https://")) {
        return trimmedInput;
    }
    
    // 如果是jitsi-meet://协议，转换为https
    if (trimmedInput.startsWith("jitsi-meet://")) {
        QString roomPart = trimmedInput.mid(13); // 移除 "jitsi-meet://"
        return m_serverUrl + "/" + roomPart;
    }
    
    // 否则作为房间名处理
    QString roomName = trimmedInput;
    
    // 移除可能的域名部分
    if (roomName.contains("/")) {
        QStringList parts = roomName.split("/");
        roomName = parts.last();
    }
    
    // 构建完整URL
    return m_serverUrl + "/" + roomName;
}

bool ConferenceWindow::isValidUrl(const QString& url)
{
    QUrl qurl(url);
    
    // 检查URL是否有效
    if (!qurl.isValid()) {
        return false;
    }
    
    // 检查协议
    QString scheme = qurl.scheme().toLower();
    if (scheme != "http" && scheme != "https") {
        return false;
    }
    
    // 检查主机名
    if (qurl.host().isEmpty()) {
        return false;
    }
    
    return true;
}

void ConferenceWindow::retranslateUi()
{
    // Update window title
    setWindowTitle(tr("welcome_title"));
    
    // Update navigation bar
    if (m_navigationBar) {
        m_navigationBar->setTitle(tr("welcome_title"));
        m_navigationBar->retranslateUi();
    }
    
    // Update progress bar
    if (m_progressBar) {
        m_progressBar->setFormat(tr("loading_conference"));
    }
    
    // Update status label
    if (m_statusLabel) {
        m_statusLabel->setText(tr("loading_conference"));
    }
}