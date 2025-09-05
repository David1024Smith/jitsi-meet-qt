#include "WelcomeWindow.h"
#include "ConfigurationManager.h"
#include "ProtocolHandler.h"
#include "Logger.h"
#include <QString>
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
#include <QSvgRenderer>
#include <QFile>
#include <QIODevice>

/**
 * @brief WelcomeWindow构造函数
 * @param parent 父窗口
 */
WelcomeWindow::WelcomeWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_sidebarPanel(nullptr)
    , m_sidebarLayout(nullptr)
    , m_logoLabel(nullptr)
    , m_sidebarSettingsButton(nullptr)
    , m_helpButton(nullptr)
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
    // 添加构造函数开始的调试输出
    // Logger::instance().info("WelcomeWindow构造函数开始");
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
    // Logger::instance().info("initializeUI函数开始执行");

    // 初始化UI
    // Logger::instance().info("开始初始化UI");
    initializeUI();
    // Logger::instance().info("UI初始化完成");

    // Logger::instance().info("开始初始化布局");
    initializeLayout();
    // Logger::instance().info("布局初始化完成");

    // Logger::instance().info("开始初始化自动完成");
    initializeAutoComplete();
    // Logger::instance().info("自动完成初始化完成");

    // Logger::instance().info("开始初始化连接");
    initializeConnections();
    // Logger::instance().info("连接初始化完成");

    // 加载数据
    // Logger::instance().info("开始加载会议历史");
    loadMeetingHistory();
    // Logger::instance().info("会议历史加载完成");

    // Logger::instance().info("开始加载服务器列表");
    loadServerList();
    // Logger::instance().info("服务器列表加载完成");

    // 恢复窗口状态
    // Logger::instance().info("开始恢复窗口状态");
    restoreWindowState();
    // Logger::instance().info("窗口状态恢复完成");

    // 更新UI状态
    // Logger::instance().info("开始更新UI状态");
    updateUIState();
    // Logger::instance().info("UI状态更新完成");

    // Logger::instance().info("WelcomeWindow构造函数完成");
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
void WelcomeWindow::setMeetingUrl(const QString &url)
{
    if (m_urlEdit)
    {
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
void WelcomeWindow::setDisplayName(const QString &displayName)
{
    if (m_displayNameEdit)
    {
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
void WelcomeWindow::setServerUrl(const QString &serverUrl)
{
    if (m_serverCombo)
    {
        int index = m_serverCombo->findText(serverUrl);
        if (index >= 0)
        {
            m_serverCombo->setCurrentIndex(index);
        }
        else
        {
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
    if (m_urlEdit)
    {
        m_urlEdit->clear();
    }
    if (m_passwordEdit)
    {
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
    if (m_urlEdit)
    {
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
    if (m_configManager)
    {
        m_configManager->setMainWindowSize(size());
    }
}

/**
 * @brief 加入会议按钮点击处理
 */
void WelcomeWindow::onJoinMeeting()
{
    if (!validateInput())
    {
        return;
    }

    QString url = getMeetingUrl();
    QString displayName = getDisplayName();
    QString password = m_passwordEdit ? m_passwordEdit->text().trimmed() : QString();
    QString serverUrl = getServerUrl();
    
    // 确保服务器URL不为空，使用默认服务器
    if (serverUrl.isEmpty() && m_configManager) {
        serverUrl = m_configManager->getDefaultServerUrl();
    }
    if (serverUrl.isEmpty()) {
        serverUrl = "https://meet.jit.si";
    }

    // 添加到历史记录
    addToHistory(url, displayName, serverUrl);

    // 发射加入会议信号
    emit joinMeetingRequested(url, displayName, password);
}

/**
 * @brief 创建会议按钮点击处理
 */
void WelcomeWindow::onCreateMeeting()
{
    if (!validateInput())
    {
        return;
    }

    QString roomName = getMeetingUrl();
    QString serverUrl = getServerUrl();
    QString displayName = getDisplayName();
    QString password = m_passwordEdit ? m_passwordEdit->text().trimmed() : QString();
    
    // 确保服务器URL不为空，使用默认服务器
    if (serverUrl.isEmpty() && m_configManager) {
        serverUrl = m_configManager->getDefaultServerUrl();
    }
    if (serverUrl.isEmpty()) {
        serverUrl = "https://meet.jit.si";
    }

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
                          "<p>使用Qt %1构建</p>")
                           .arg(QT_VERSION_STR));
}

/**
 * @brief 退出按钮点击处理
 */
void WelcomeWindow::onExit()
{
    QApplication::quit();
}

/**
 * @brief 侧边栏设置按钮点击事件处理
 */
void WelcomeWindow::onSidebarSettings()
{
    // 调用现有的设置功能
    onSettings();
}

/**
 * @brief 帮助按钮点击事件处理
 */
void WelcomeWindow::onHelp()
{
    // 调用现有的关于功能
    onAbout();
}

/**
 * @brief 会议历史项目双击处理
 * @param item 被双击的项目
 */
void WelcomeWindow::onHistoryItemDoubleClicked(QListWidgetItem *item)
{
    if (!item)
    {
        return;
    }

    // 从项目数据中获取会议信息
    QJsonObject meetingData = item->data(Qt::UserRole).toJsonObject();

    // 获取会议数据，使用正确的字段名
    QString roomName = meetingData.value("roomName").toString();
    QString displayName = meetingData.value("displayName").toString();
    QString serverUrl = meetingData.value("serverUrl").toString();
    QString fullUrl = meetingData.value("fullUrl").toString();

    // 优先使用fullUrl，如果没有则构建URL
    QString meetingUrl = fullUrl;
    if (meetingUrl.isEmpty() && !roomName.isEmpty() && !serverUrl.isEmpty())
    {
        meetingUrl = serverUrl + "/" + roomName;
    }
    
    // 如果仍然没有URL，使用roomName作为URL
    if (meetingUrl.isEmpty())
    {
        meetingUrl = roomName;
    }

    // 设置到输入框
    setMeetingUrl(meetingUrl);
    if (!displayName.isEmpty())
    {
        setDisplayName(displayName);
    }
    if (!serverUrl.isEmpty())
    {
        setServerUrl(serverUrl);
    }

    // 自动加入会议
    onJoinMeeting();
}

/**
 * @brief 会议历史项目选择改变处理
 */
void WelcomeWindow::onHistorySelectionChanged()
{
    if (!m_historyList)
    {
        return;
    }

    QListWidgetItem *currentItem = m_historyList->currentItem();
    if (!currentItem)
    {
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
    if (!serverUrl.isEmpty() && serverUrl != m_lastCheckedServer)
    {
        checkServerAvailability(serverUrl);
    }

    updateUIState();
}

/**
 * @brief URL输入改变处理
 * @param text 输入的文本
 */
void WelcomeWindow::onUrlChanged(const QString &text)
{
    Q_UNUSED(text)

    // 延迟验证URL
    if (m_urlValidationTimer)
    {
        m_urlValidationTimer->start();
    }

    updateUIState();
}

/**
 * @brief 显示名称输入改变处理
 * @param text 输入的文本
 */
void WelcomeWindow::onDisplayNameChanged(const QString &text)
{
    Q_UNUSED(text)
    updateUIState();
}

/**
 * @brief 服务器可用性检查完成处理
 * @param available 服务器是否可用
 * @param serverUrl 服务器URL
 */
void WelcomeWindow::onServerAvailabilityChecked(bool available, const QString &serverUrl)
{
    if (m_statusLabel)
    {
        if (available)
        {
            m_statusLabel->setText(tr("服务器 %1 可用").arg(serverUrl));
        }
        else
        {
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
    if (m_statusLabel)
    {
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
    if (!url.isEmpty() && url != m_lastValidatedUrl)
    {
        m_lastValidatedUrl = url;

        // 解析URL
        QJsonObject urlData = parseMeetingUrl(url);

        // 更新信息显示
        if (m_infoText)
        {
            QString info;
            if (!urlData.isEmpty())
            {
                info += tr("<b>会议信息:</b><br>");
                if (urlData.contains(QStringLiteral("roomName")))
                {
                    info += tr("房间名: %1<br>").arg(urlData.value("roomName").toString());
                }
                if (urlData.contains(QStringLiteral("serverUrl")))
                {
                    info += tr("服务器: %1<br>").arg(urlData.value("serverUrl").toString());
                }
                if (urlData.contains(QStringLiteral("displayName")))
                {
                    info += tr("显示名: %1<br>").arg(urlData.value("displayName").toString());
                }
            }
            else
            {
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
    // Logger::instance().info("WelcomeWindow构造函数开始");

    // 设置窗口属性
    setWindowTitle(tr("Jitsi Meet Qt"));
    setWindowIcon(QIcon(":/icons/app.svg"));
    setMinimumSize(QSize(800, 600));

    // Logger::instance().info("窗口属性设置完成");

    // 创建中央窗口部件
    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);

    // 设置窗口背景为蓝色
    m_centralWidget->setStyleSheet("background-color: #0056E0;");

    // Logger::instance().info("中央窗口部件创建完成");

    // 创建左侧菜单栏
    m_sidebarPanel = new QWidget();
    m_sidebarPanel->setObjectName("sidebarPanel");
    m_sidebarPanel->setFixedWidth(70);
    m_sidebarPanel->setStyleSheet("#sidebarPanel { "
                                  "background: rgba(0, 0, 0, 0.8); "
                                  "border-right: 1px solid rgba(255, 255, 255, 0.08); "
                                  "}");

    // 创建左侧菜单栏布局
    m_sidebarLayout = new QVBoxLayout(m_sidebarPanel);
    m_sidebarLayout->setContentsMargins(10, 20, 10, 20);
    m_sidebarLayout->setSpacing(15);
    m_sidebarLayout->setAlignment(Qt::AlignHCenter);

    // Logger::instance().info("左侧菜单栏创建完成，准备创建Logo");

    // 创建Logo标签
    // Logger::instance().info("准备创建Logo标签");

    m_logoLabel = new QLabel();
    // Logger::instance().info("Logo标签创建成功");

    m_logoLabel->setFixedSize(40, 40);
    m_logoLabel->setAlignment(Qt::AlignCenter);

    // Logger::instance().info("Logo标签属性设置完成");

    // 优先使用SVG格式的logo
    qDebug() << "开始加载Logo图片";
    // 添加文件调试输出
    // Logger::instance().info("开始加载Logo图片");

    QPixmap logoPixmap;
    // 使用Qt高DPI最佳实践：根据设备像素比例渲染高分辨率图标
    QFile svgFile(":/images/logo.svg");
    if (svgFile.open(QIODevice::ReadOnly))
    {
        QByteArray svgData = svgFile.readAll();
        svgFile.close();

        QSvgRenderer svgRenderer(svgData);
        if (svgRenderer.isValid())
        {
            // 获取设备像素比例以渲染高分辨率图标
            const qreal devicePixelRatio = this->devicePixelRatio();
            const int baseIconSize = 32;
            const int actualIconSize = baseIconSize * devicePixelRatio;

            // 渲染高分辨率QPixmap
            logoPixmap = QPixmap(actualIconSize, actualIconSize);
            logoPixmap.fill(Qt::transparent);
            QPainter painter(&logoPixmap);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            svgRenderer.render(&painter);
            painter.end();

            // 设置正确的设备像素比例，这样Qt就知道这是高分辨率图像
            logoPixmap.setDevicePixelRatio(devicePixelRatio);

            qDebug() << QString("Logo高DPI图标加载成功，设备像素比例: %1, 渲染尺寸: %2x%3")
                            .arg(devicePixelRatio)
                            .arg(actualIconSize)
                            .arg(actualIconSize);
            // Logger::instance().info(QString("SVG Logo加载成功，尺寸: %1x%2").arg(logoPixmap.width()).arg(logoPixmap.height()));
        }
        else
        {
            qDebug() << "SVG渲染器无效";
            // Logger::instance().warning("SVG渲染器无效");
        }
    }
    else
    {
        qDebug() << "无法打开SVG文件，错误:" << svgFile.errorString();
        // Logger::instance().warning(QString("无法打开SVG文件，错误: %1").arg(svgFile.errorString()));
    }
    if (!logoPixmap.isNull())
    {
        m_logoLabel->setPixmap(logoPixmap);
        qDebug() << "Logo设置到标签完成";
        // Logger::instance().info("Logo设置到标签完成");
    }
    else
    {
        qDebug() << "Logo加载完全失败，无法设置到标签";
        // Logger::instance().error("Logo加载完全失败，无法设置到标签");
    }
    m_logoLabel->setStyleSheet("QLabel { "
                               "background: transparent; "
                               "border: none; "
                               "}");

    // 创建设置按钮
    printf("=== 开始创建设置按钮 ===\n");
    fflush(stdout);
    m_sidebarSettingsButton = new QPushButton();
    m_sidebarSettingsButton->setFixedSize(50, 50); // 增大按钮尺寸
    m_sidebarSettingsButton->setVisible(true);     // 确保按钮可见
    printf("设置按钮创建完成，尺寸: %dx%d\n", m_sidebarSettingsButton->width(), m_sidebarSettingsButton->height());
    fflush(stdout);

    // 优先加载SVG格式的设置图标
    QIcon settingsIcon;
    QPixmap settingsPixmap;

    // 使用Qt高DPI最佳实践：根据设备像素比例渲染高分辨率图标
    QFile settingsSvgFile(":/icons/settings.svg");
    if (settingsSvgFile.open(QIODevice::ReadOnly))
    {
        QByteArray settingsSvgData = settingsSvgFile.readAll();
        settingsSvgFile.close();

        QSvgRenderer settingsSvgRenderer(settingsSvgData);
        if (settingsSvgRenderer.isValid())
        {
            // 获取设备像素比例以渲染高分辨率图标
            const qreal devicePixelRatio = this->devicePixelRatio();
            const int baseIconSize = 32;
            const int actualIconSize = baseIconSize * devicePixelRatio;

            // 渲染高分辨率QPixmap
            settingsPixmap = QPixmap(actualIconSize, actualIconSize);
            settingsPixmap.fill(Qt::transparent);
            QPainter settingsPainter(&settingsPixmap);
            settingsPainter.setRenderHint(QPainter::Antialiasing, true);
            settingsPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            settingsSvgRenderer.render(&settingsPainter);
            settingsPainter.end();

            // 设置正确的设备像素比例，这样Qt就知道这是高分辨率图像
            settingsPixmap.setDevicePixelRatio(devicePixelRatio);

            settingsIcon = QIcon(settingsPixmap);
            qDebug() << QString("设置按钮高DPI图标加载成功，设备像素比例: %1, 渲染尺寸: %2x%3")
                            .arg(devicePixelRatio)
                            .arg(actualIconSize)
                            .arg(actualIconSize);
        }
        else
        {
            qDebug() << "设置按钮SVG渲染器无效，使用直接加载方式";
            settingsIcon = QIcon(":/icons/settings.svg");
        }
    }
    else
    {
        qDebug() << "无法打开设置按钮SVG文件，使用直接加载方式";
        settingsIcon = QIcon(":/icons/settings.svg");
    }

    if (!settingsIcon.isNull())
    {
        m_sidebarSettingsButton->setIcon(settingsIcon);
        m_sidebarSettingsButton->setIconSize(QSize(32, 32));
        qDebug() << "设置按钮图标设置成功";
        // 圆形透明背景按钮
        m_sidebarSettingsButton->setStyleSheet("QPushButton { "
                                               "background: transparent; "
                                               "border: none; "
                                               "border-radius: 25px; "
                                               "padding: 0px; "
                                               "min-width: 50px; "
                                               "min-height: 50px; "
                                               "max-width: 50px; "
                                               "max-height: 50px; "
                                               "} "
                                               "QPushButton:hover { "
                                               "background: rgba(255, 255, 255, 0.1); "
                                               "border-radius: 25px; "
                                               "} "
                                               "QPushButton:pressed { "
                                               "background: rgba(255, 255, 255, 0.2); "
                                               "border-radius: 25px; "
                                               "}");
    }
    else
    {
        // 如果图标加载失败，使用圆形透明样式
        m_sidebarSettingsButton->setStyleSheet("QPushButton { "
                                               "background: transparent; "
                                               "border: none; "
                                               "border-radius: 25px; "
                                               "color: white; "
                                               "font-size: 12px; "
                                               "font-weight: bold; "
                                               "padding: 0px; "
                                               "min-width: 50px; "
                                               "min-height: 50px; "
                                               "max-width: 50px; "
                                               "max-height: 50px; "
                                               "} "
                                               "QPushButton:hover { "
                                               "background: rgba(255, 255, 255, 0.1); "
                                               "border-radius: 25px; "
                                               "} "
                                               "QPushButton:pressed { "
                                               "background: rgba(255, 255, 255, 0.2); "
                                               "border-radius: 25px; "
                                               "}");
        m_sidebarSettingsButton->setText(" ");
        qDebug() << "设置按钮图标加载失败，使用文本显示: ⚙";
    }
    m_sidebarSettingsButton->setToolTip(tr("设置"));
    qDebug() << "设置按钮样式设置完成";

    // 创建帮助按钮
    printf("=== 开始创建帮助按钮 ===\n");
    fflush(stdout);
    m_helpButton = new QPushButton();
    m_helpButton->setFixedSize(50, 50); // 设置为正方形以创建圆形按钮
    m_helpButton->setVisible(true);     // 确保按钮可见
    printf("帮助按钮创建完成，尺寸: %dx%d\n", m_helpButton->width(), m_helpButton->height());
    fflush(stdout);

    // 优先加载SVG格式的帮助图标
    QIcon helpIcon;
    QPixmap helpPixmap;

    // 使用Qt高DPI最佳实践：根据设备像素比例渲染高分辨率图标
    QFile helpSvgFile(":/icons/help.svg");
    if (helpSvgFile.open(QIODevice::ReadOnly))
    {
        QByteArray helpSvgData = helpSvgFile.readAll();
        helpSvgFile.close();

        QSvgRenderer helpSvgRenderer(helpSvgData);
        if (helpSvgRenderer.isValid())
        {
            // 获取设备像素比例以渲染高分辨率图标
            const qreal devicePixelRatio = this->devicePixelRatio();
            const int baseIconSize = 32;
            const int actualIconSize = baseIconSize * devicePixelRatio;

            // 渲染高分辨率QPixmap
            helpPixmap = QPixmap(actualIconSize, actualIconSize);
            helpPixmap.fill(Qt::transparent);
            QPainter helpPainter(&helpPixmap);
            helpPainter.setRenderHint(QPainter::Antialiasing, true);
            helpPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            helpSvgRenderer.render(&helpPainter);
            helpPainter.end();

            // 设置正确的设备像素比例，这样Qt就知道这是高分辨率图像
            helpPixmap.setDevicePixelRatio(devicePixelRatio);

            helpIcon = QIcon(helpPixmap);
            qDebug() << QString("帮助按钮高DPI图标加载成功，设备像素比例: %1, 渲染尺寸: %2x%3")
                            .arg(devicePixelRatio)
                            .arg(actualIconSize)
                            .arg(actualIconSize);
        }
        else
        {
            qDebug() << "帮助按钮SVG渲染器无效，使用文本显示";
        }
    }
    else
    {
        qDebug() << "无法打开帮助按钮SVG文件，使用文本显示";
    }

    if (!helpIcon.isNull())
    {
        m_helpButton->setIcon(helpIcon);
        qDebug() << "帮助按钮图标设置成功";
        m_helpButton->setIconSize(QSize(32, 32));
        // 圆形透明背景按钮
        m_helpButton->setStyleSheet("QPushButton { "
                                    "background: transparent; "
                                    "border: none; "
                                    "border-radius: 25px; "
                                    "padding: 0px; "
                                    "min-width: 50px; "
                                    "min-height: 50px; "
                                    "max-width: 50px; "
                                    "max-height: 50px; "
                                    "} "
                                    "QPushButton:hover { "
                                    "background: rgba(255, 255, 255, 0.1); "
                                    "border-radius: 25px; "
                                    "} "
                                    "QPushButton:pressed { "
                                    "background: rgba(255, 255, 255, 0.2); "
                                    "border-radius: 25px; "
                                    "}");
    }
    else
    {
        // 如果没有图标文件，使用圆形透明样式
        m_helpButton->setStyleSheet("QPushButton { "
                                    "background: transparent; "
                                    "border: none; "
                                    "border-radius: 25px; "
                                    "color: white; "
                                    "font-size: 12px; "
                                    "font-weight: bold; "
                                    "padding: 0px; "
                                    "min-width: 50px; "
                                    "min-height: 50px; "
                                    "max-width: 50px; "
                                    "max-height: 50px; "
                                    "} "
                                    "QPushButton:hover { "
                                    "background: rgba(255, 255, 255, 0.1); "
                                    "border-radius: 25px; "
                                    "} "
                                    "QPushButton:pressed { "
                                    "background: rgba(255, 255, 255, 0.2); "
                                    "border-radius: 25px; "
                                    "}");
        m_helpButton->setText(" ");
        qDebug() << "帮助按钮使用文本显示: ?";
    }
    m_helpButton->setToolTip(tr("帮助"));
    qDebug() << "帮助按钮样式设置完成";

    // 添加组件到侧边栏布局
    printf("=== 开始添加组件到侧边栏布局 ===\n");
    fflush(stdout);
    m_sidebarLayout->addWidget(m_logoLabel, 0, Qt::AlignCenter);
    printf("Logo标签已添加到布局\n");
    fflush(stdout);
    m_sidebarLayout->addStretch();
    printf("弹性空间已添加到布局\n");
    fflush(stdout);
    m_sidebarLayout->addWidget(m_sidebarSettingsButton, 0, Qt::AlignCenter);
    printf("设置按钮已添加到布局，按钮可见性: %s\n", m_sidebarSettingsButton->isVisible() ? "true" : "false");
    fflush(stdout);
    m_sidebarLayout->addSpacing(10);
    printf("间距已添加到布局\n");
    fflush(stdout);
    m_sidebarLayout->addWidget(m_helpButton, 0, Qt::AlignCenter);
    printf("帮助按钮已添加到布局，按钮可见性: %s\n", m_helpButton->isVisible() ? "true" : "false");
    fflush(stdout);
    printf("侧边栏布局组件添加完成，总组件数: %d\n", m_sidebarLayout->count());
    fflush(stdout);

    // 创建中央面板
    m_leftPanel = new QWidget();
    m_leftPanel->setObjectName("centralPanel");
    m_leftPanel->setStyleSheet("#centralPanel { background-color: transparent; }");

    // 创建标题标签
    QLabel *titleLabel = new QLabel(tr("输入会议名或链接地址"));
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
    QLabel *recentLabel = new QLabel(tr("您最近加入的会议"));
    recentLabel->setStyleSheet("color: white; font-size: 14px;");

    // 创建历史记录列表
    m_historyList = new QListWidget();
    m_historyList->setObjectName("meetingHistoryList");
    m_historyList->setMaximumHeight(200);
    m_historyList->setSpacing(5);
    m_historyList->setUniformItemSizes(false);
    m_historyList->setWordWrap(true);
    m_historyList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_historyList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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

    // Logger::instance().info("开始创建m_displayNameEdit");

    m_displayNameEdit = new QLineEdit(this);

    // Logger::instance().info(QString("m_displayNameEdit创建完成，指针地址: %1").arg(QString::number(reinterpret_cast<quintptr>(m_displayNameEdit), 16)));

    m_displayNameEdit->setPlaceholderText(tr("输入您的显示名称"));

    // Logger::instance().info("m_displayNameEdit占位符文本设置完成");

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
    // 创建分割器来管理侧边栏和主内容区域
    m_splitter = new QSplitter(Qt::Horizontal, m_centralWidget);
    m_splitter->setChildrenCollapsible(false);

    // 添加侧边栏到分割器
    m_splitter->addWidget(m_sidebarPanel);

    // 主布局
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // 添加分割器到主布局
    m_mainLayout->addWidget(m_splitter);

    // 创建中央面板布局
    m_leftLayout = new QVBoxLayout(m_leftPanel);
    m_leftLayout->setContentsMargins(20, 20, 20, 20);
    m_leftLayout->setSpacing(15);
    m_leftLayout->setAlignment(Qt::AlignCenter);

    // 添加标题标签
    QLabel *titleLabel = new QLabel(tr("输入会议名或链接地址"));
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    m_leftLayout->addWidget(titleLabel);

    // 创建URL输入布局
    QHBoxLayout *urlLayout = new QHBoxLayout();
    urlLayout->addWidget(m_urlEdit, 1);
    urlLayout->addWidget(m_joinButton);
    m_leftLayout->addLayout(urlLayout);

    // 添加空白区域
    m_leftLayout->addStretch();

    // 添加最近会议标签
    QLabel *recentLabel = new QLabel(tr("您最近加入的会议"));
    recentLabel->setStyleSheet("color: white; font-size: 14px;");
    m_leftLayout->addWidget(recentLabel);

    // 添加历史记录列表
    m_leftLayout->addWidget(m_historyList);

    // 添加空白区域
    m_leftLayout->addStretch();

    // 添加中央面板到分割器
    m_splitter->addWidget(m_leftPanel);

    // 设置分割器的比例，侧边栏固定70像素，主内容区域占剩余空间
    m_splitter->setSizes({70, 800});

    // 设置窗口布局
    m_centralWidget->setLayout(m_mainLayout);

    // 确保侧边栏在最前面显示，避免被其他组件遮挡
    m_sidebarPanel->raise();
    printf("侧边栏面板已提升到最前层\n");
    fflush(stdout);
}

/**
 * @brief 初始化连接
 */
void WelcomeWindow::initializeConnections()
{
    // 按钮连接
    connect(m_joinButton, &QPushButton::clicked, this, &WelcomeWindow::onJoinMeeting);
    if (m_createButton)
    {
        connect(m_createButton, &QPushButton::clicked, this, &WelcomeWindow::onCreateMeeting);
    }
    if (m_settingsButton)
    {
        connect(m_settingsButton, &QPushButton::clicked, this, &WelcomeWindow::onSettings);
    }
    if (m_aboutButton)
    {
        connect(m_aboutButton, &QPushButton::clicked, this, &WelcomeWindow::onAbout);
    }
    if (m_exitButton)
    {
        connect(m_exitButton, &QPushButton::clicked, this, &WelcomeWindow::onExit);
    }

    // 侧边栏按钮连接
    connect(m_sidebarSettingsButton, &QPushButton::clicked, this, &WelcomeWindow::onSidebarSettings);
    connect(m_helpButton, &QPushButton::clicked, this, &WelcomeWindow::onHelp);
    if (m_clearHistoryButton)
    {
        connect(m_clearHistoryButton, &QPushButton::clicked, this, [this]()
                {
            if (m_configManager) {
                m_configManager->clearMeetingHistory();
                refreshMeetingHistory();
            } });
    }

    // 输入控件连接
    connect(m_urlEdit, &QLineEdit::textChanged, this, &WelcomeWindow::onUrlChanged);
    connect(m_displayNameEdit, &QLineEdit::textChanged, this, &WelcomeWindow::onDisplayNameChanged);
    if (m_serverCombo)
    {
        connect(m_serverCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WelcomeWindow::onServerChanged);
    }

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
    // Logger::instance().info("开始创建URL模型");

    // URL自动完成
    m_urlModel = new QStringListModel(this);

    // Logger::instance().info("URL模型创建完成，开始创建URL完成器");

    m_urlCompleter = new QCompleter(m_urlModel, this);

    // Logger::instance().info("URL完成器创建完成，设置大小写敏感性");

    m_urlCompleter->setCaseSensitivity(Qt::CaseInsensitive);

    // Logger::instance().info("开始设置URL编辑器的完成器");

    m_urlEdit->setCompleter(m_urlCompleter);

    // Logger::instance().info("URL完成器设置完成，开始创建名称模型");

    // 显示名称自动完成
    m_nameModel = new QStringListModel(this);

    // Logger::instance().info("名称模型创建完成，开始创建名称完成器");

    m_nameCompleter = new QCompleter(m_nameModel, this);

    // Logger::instance().info("名称完成器创建完成，设置大小写敏感性");

    m_nameCompleter->setCaseSensitivity(Qt::CaseInsensitive);

    // Logger::instance().info("开始设置显示名称编辑器的完成器");

    // 检查指针是否有效
    if (m_displayNameEdit == nullptr)
    {
        // Logger::instance().error("错误：m_displayNameEdit为空指针");
        return;
    }

    if (m_nameCompleter == nullptr)
    {
        // Logger::instance().error("错误：m_nameCompleter为空指针");
        return;
    }

    // Logger::instance().info("指针检查通过，开始调用setCompleter");

    try
    {
        m_displayNameEdit->setCompleter(m_nameCompleter);

        // Logger::instance().info("setCompleter调用成功");
    }
    catch (...)
    {
        // Logger::instance().error("setCompleter调用时发生异常");
        return;
    }

    // Logger::instance().info("自动完成初始化全部完成");
}

/**
 * @brief 加载会议历史
 */
/**
 * @brief 加载会议历史记录
 * 按照jitsi-meet-electron的格式显示会议信息，包含房间名、服务器地址、会议时间和时间差
 */
void WelcomeWindow::loadMeetingHistory()
{
    if (!m_configManager || !m_historyList)
    {
        return;
    }

    m_historyList->clear();

    QJsonObject recentMeetings = m_configManager->getRecentMeetings();
    QJsonArray history = recentMeetings.value("meetings").toArray();
    
    for (const QJsonValue &value : history)
    {
        QJsonObject meeting = value.toObject();

        QString fullUrl = meeting.value("fullUrl").toString();
        QString roomName = meeting.value("roomName").toString();
        QString displayName = meeting.value("displayName").toString();
        QString serverUrl = meeting.value("serverUrl").toString();
        QString timestampStr = meeting.value("timestamp").toString();
        
        // 解析时间戳
        QDateTime dateTime = QDateTime::fromString(timestampStr, Qt::ISODate);
        qint64 timestamp = dateTime.toSecsSinceEpoch();
        
        // 如果没有房间名，从URL中提取
        if (roomName.isEmpty() && !fullUrl.isEmpty())
        {
            roomName = extractRoomName(fullUrl);
        }
        
        // 如果仍然没有房间名，使用完整URL
        if (roomName.isEmpty())
        {
            roomName = fullUrl.isEmpty() ? meeting.value("url").toString() : fullUrl;
        }

        QListWidgetItem *item = new QListWidgetItem(m_historyList);
        item->setData(Qt::UserRole, meeting);
        item->setText(roomName); // 设置基本文本为房间名

        // 创建自定义小部件来显示会议信息（仿照jitsi-meet-electron的ConferenceCard）
        QWidget *itemWidget = new QWidget();
        QVBoxLayout *itemLayout = new QVBoxLayout(itemWidget);
        itemLayout->setContentsMargins(12, 10, 12, 10);
        itemLayout->setSpacing(4);

        // 第一行：会议名称（房间名）
        QLabel *roomLabel = new QLabel(roomName, itemWidget);
        roomLabel->setObjectName("meetingRoomName");
        roomLabel->setProperty("class", "meeting-room-name");
        roomLabel->setWordWrap(true);
        itemLayout->addWidget(roomLabel);

        // 第二行：服务器地址
        QString formattedServerUrl = serverUrl;
        if (formattedServerUrl.isEmpty())
        {
            formattedServerUrl = "未知服务器";
        }
        else
        {
            formattedServerUrl = formatServerUrl(serverUrl);
            if (formattedServerUrl.isEmpty())
            {
                formattedServerUrl = serverUrl; // 如果格式化后为空，使用原始URL
            }
        }
        
        QLabel *serverLabel = new QLabel(formattedServerUrl, itemWidget);
        serverLabel->setObjectName("meetingServerUrl");
        serverLabel->setProperty("class", "meeting-server-url");
        serverLabel->setWordWrap(true);
        itemLayout->addWidget(serverLabel);

        // 第三行：会议时间
        QString formattedTime = formatMeetingTime(timestamp);
        QLabel *timeLabel = new QLabel(formattedTime, itemWidget);
        timeLabel->setObjectName("meetingTime");
        timeLabel->setProperty("class", "meeting-time");
        itemLayout->addWidget(timeLabel);

        // 第四行：距离当前时间的时间差
        QString relativeTime = getRelativeTime(timestamp);
        QLabel *relativeTimeLabel = new QLabel(relativeTime, itemWidget);
        relativeTimeLabel->setObjectName("meetingRelativeTime");
        relativeTimeLabel->setProperty("class", "meeting-relative-time");
        itemLayout->addWidget(relativeTimeLabel);

        // 设置小部件样式（使用CSS类名）
        itemWidget->setObjectName("meetingHistoryItem");
        itemWidget->setProperty("class", "meeting-history-item");

        m_historyList->setItemWidget(item, itemWidget);

        // 设置项目高度（增加高度以容纳四行信息）
        item->setSizeHint(QSize(item->sizeHint().width(), 100));

        // 设置详细的工具提示
        item->setToolTip(QString("会议名称: %1\n服务器地址: %2\n显示名称: %3\n会议时间: %4\n时间差: %5")
                             .arg(roomName, formattedServerUrl, displayName, 
                                  dateTime.toString("yyyy-MM-dd hh:mm:ss"), relativeTime));
    }

    // 更新自动完成数据
    QStringList urls, names;
    for (const QJsonValue &value : history)
    {
        QJsonObject meeting = value.toObject();
        QString url = meeting["fullUrl"].toString();
        if (url.isEmpty()) {
            url = meeting["url"].toString();
        }
        QString displayName = meeting["displayName"].toString();

        if (!url.isEmpty() && !urls.contains(url))
        {
            urls.append(url);
        }
        if (!displayName.isEmpty() && !names.contains(displayName))
        {
            names.append(displayName);
        }
    }

    if (m_urlModel)
    {
        m_urlModel->setStringList(urls);
    }
    if (m_nameModel)
    {
        m_nameModel->setStringList(names);
    }
}

/**
 * @brief 加载服务器列表
 */
void WelcomeWindow::loadServerList()
{
    if (!m_configManager || !m_serverCombo)
    {
        return;
    }

    m_serverCombo->clear();

    // 添加默认服务器
    QString defaultServer = m_configManager->getDefaultServerUrl();
    if (!defaultServer.isEmpty())
    {
        m_serverCombo->addItem(defaultServer);
    }

    // 添加常用服务器
    QStringList servers;
    servers << "meet.jit.si" << "8x8.vc" << "jitsi.riot.im";

    for (const QString &server : servers)
    {
        if (server != defaultServer)
        {
            m_serverCombo->addItem(server);
        }
    }

    // 设置当前服务器
    if (!defaultServer.isEmpty())
    {
        m_serverCombo->setCurrentText(defaultServer);
    }
}

/**
 * @brief 保存窗口状态
 */
void WelcomeWindow::saveWindowState()
{
    if (!m_configManager)
    {
        return;
    }

    m_configManager->setMainWindowSize(size());
    m_configManager->setMainWindowPosition(pos());
    m_configManager->setMainWindowMaximized(isMaximized());

    if (m_splitter)
    {
        m_configManager->setValue("WelcomeWindow/splitterState", m_splitter->saveState());
    }
}

/**
 * @brief 恢复窗口状态
 */
void WelcomeWindow::restoreWindowState()
{
    if (!m_configManager)
    {
        return;
    }

    // 恢复窗口大小和位置
    QSize size = m_configManager->getMainWindowSize();
    if (size.isValid())
    {
        resize(size);
    }

    QPoint pos = m_configManager->getMainWindowPosition();
    if (!pos.isNull())
    {
        move(pos);
    }

    if (m_configManager->isMainWindowMaximized())
    {
        showMaximized();
    }

    // 恢复分割器状态
    if (m_splitter)
    {
        QByteArray splitterState = m_configManager->getValue("WelcomeWindow/splitterState").toByteArray();
        if (!splitterState.isEmpty())
        {
            m_splitter->restoreState(splitterState);
        }
    }

    // 恢复默认显示名称
    QString defaultDisplayName = m_configManager->getDefaultDisplayName();
    if (!defaultDisplayName.isEmpty())
    {
        setDisplayName(defaultDisplayName);
    }
}

/**
 * @brief 验证输入
 * @return 输入是否有效
 */
/**
 * @brief 验证输入，如果显示名称为空则自动使用默认显示名称
 * @return 验证是否通过
 */
bool WelcomeWindow::validateInput()
{
    QString url = getMeetingUrl();
    QString displayName = getDisplayName();

    if (url.isEmpty())
    {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入会议URL或房间名"));
        m_urlEdit->setFocus();
        return false;
    }

    // 如果显示名称为空，尝试使用默认显示名称
    if (displayName.isEmpty())
    {
        if (m_configManager)
        {
            QString defaultName = m_configManager->getDefaultDisplayName();
            if (!defaultName.isEmpty())
            {
                // 使用默认显示名称
                setDisplayName(defaultName);
                displayName = defaultName;
            }
            else
            {
                // 如果没有默认显示名称，使用"用户"作为默认值
                QString fallbackName = tr("用户");
                setDisplayName(fallbackName);
                displayName = fallbackName;
            }
        }
        else
        {
            // 如果配置管理器不可用，使用"用户"作为默认值
            QString fallbackName = tr("用户");
            setDisplayName(fallbackName);
            displayName = fallbackName;
        }
    }

    return true;
}

/**
 * @brief 解析会议URL
 * @param url 会议URL
 * @return 解析结果
 */
QJsonObject WelcomeWindow::parseMeetingUrl(const QString &url)
{
    QJsonObject result;

    if (url.isEmpty())
    {
        return result;
    }

    // 使用协议处理器解析URL
    if (m_protocolHandler)
    {
        ProtocolHandler::MeetingInfo meetingInfo = m_protocolHandler->parseProtocolUrl(url);
        if (meetingInfo.isValid)
        {
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
    if (qurl.isValid() && !qurl.host().isEmpty())
    {
        result["serverUrl"] = qurl.host();
        QString path = qurl.path();
        if (path.startsWith("/"))
        {
            path = path.mid(1);
        }
        if (!path.isEmpty())
        {
            result["roomName"] = path;
        }
    }
    else
    {
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
void WelcomeWindow::checkServerAvailability(const QString &serverUrl)
{
    if (serverUrl.isEmpty() || m_isCheckingServer)
    {
        return;
    }

    m_isCheckingServer = true;
    m_lastCheckedServer = serverUrl;

    if (m_statusLabel)
    {
        m_statusLabel->setText(tr("检查服务器 %1...").arg(serverUrl));
    }

    if (m_progressBar)
    {
        m_progressBar->setVisible(true);
        m_progressBar->setRange(0, 0); // 无限进度条
    }

    // 启动超时定时器
    if (m_serverCheckTimer)
    {
        m_serverCheckTimer->start();
    }

    // 构建检查URL
    QString checkUrl = serverUrl;
    if (!checkUrl.startsWith("http"))
    {
        checkUrl = "https://" + checkUrl;
    }
    if (!checkUrl.endsWith("/"))
    {
        checkUrl += "/";
    }
    checkUrl += "config.js";

    // 发送网络请求
    QUrl url(checkUrl);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "JitsiMeetQt/1.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, serverUrl]()
            {
        reply->deleteLater();
        
        if (m_serverCheckTimer) {
            m_serverCheckTimer->stop();
        }
        
        if (m_progressBar) {
            m_progressBar->setVisible(false);
        }
        
        bool available = (reply->error() == QNetworkReply::NoError);
        onServerAvailabilityChecked(available, serverUrl); });
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

    if (m_joinButton)
    {
        m_joinButton->setEnabled(canJoin);
    }

    if (m_createButton)
    {
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
void WelcomeWindow::addToHistory(const QString &url, const QString &displayName, const QString &serverUrl)
{
    if (!m_configManager || url.isEmpty())
    {
        return;
    }

    // 从URL中提取房间名称
    QString roomName = url;
    if (url.contains("/"))
    {
        roomName = url.split("/").last();
    }

    m_configManager->addMeetingRecord(roomName, serverUrl, displayName);
}

/**
 * @brief 格式化会议时间显示
 * @param timestamp 时间戳（秒）
 * @return 格式化的时间字符串
 */
QString WelcomeWindow::formatMeetingTime(qint64 timestamp) const
{
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
    QDateTime now = QDateTime::currentDateTime();
    
    // 如果是今天，显示时间
    if (dateTime.date() == now.date())
    {
        return dateTime.toString("今天 hh:mm");
    }
    // 如果是昨天
    else if (dateTime.date() == now.date().addDays(-1))
    {
        return dateTime.toString("昨天 hh:mm");
    }
    // 如果是本周内
    else if (dateTime.date() >= now.date().addDays(-7))
    {
        QStringList weekDays = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
        return weekDays[dateTime.date().dayOfWeek() % 7] + " " + dateTime.toString("hh:mm");
    }
    // 其他情况显示完整日期
    else
    {
        return dateTime.toString("yyyy-MM-dd hh:mm");
    }
}

/**
 * @brief 计算相对时间差
 * @param timestamp 时间戳（秒）
 * @return 相对时间字符串（如"2小时前"、"昨天"等）
 */
QString WelcomeWindow::getRelativeTime(qint64 timestamp) const
{
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
    QDateTime now = QDateTime::currentDateTime();
    
    qint64 diffSeconds = dateTime.secsTo(now);
    
    if (diffSeconds < 60)
    {
        return "刚刚";
    }
    else if (diffSeconds < 3600) // 小于1小时
    {
        int minutes = diffSeconds / 60;
        return QString("%1分钟前").arg(minutes);
    }
    else if (diffSeconds < 86400) // 小于1天
    {
        int hours = diffSeconds / 3600;
        return QString("%1小时前").arg(hours);
    }
    else if (diffSeconds < 604800) // 小于1周
    {
        int days = diffSeconds / 86400;
        return QString("%1天前").arg(days);
    }
    else if (diffSeconds < 2592000) // 小于1月
    {
        int weeks = diffSeconds / 604800;
        return QString("%1周前").arg(weeks);
    }
    else if (diffSeconds < 31536000) // 小于1年
    {
        int months = diffSeconds / 2592000;
        return QString("%1个月前").arg(months);
    }
    else
    {
        int years = diffSeconds / 31536000;
        return QString("%1年前").arg(years);
    }
}

/**
 * @brief 提取房间名称（去除URL参数）
 * @param url 完整的会议URL
 * @return 清理后的房间名称
 */
QString WelcomeWindow::extractRoomName(const QString &url) const
{
    QString roomName = url;
    
    // 去除协议前缀
    if (roomName.startsWith("http://") || roomName.startsWith("https://"))
    {
        QUrl parsedUrl(roomName);
        roomName = parsedUrl.path();
        if (roomName.startsWith("/"))
        {
            roomName = roomName.mid(1);
        }
    }
    
    // 去除查询参数
    if (roomName.contains("?"))
    {
        roomName = roomName.split("?").first();
    }
    
    // 去除锚点
    if (roomName.contains("#"))
    {
        roomName = roomName.split("#").first();
    }
    
    // 如果包含路径分隔符，取最后一部分
    if (roomName.contains("/"))
    {
        roomName = roomName.split("/").last();
    }
    
    return roomName.isEmpty() ? url : roomName;
}

/**
 * @brief 格式化服务器URL显示
 * @param serverUrl 服务器URL
 * @return 格式化的服务器地址（去除协议前缀）
 */
QString WelcomeWindow::formatServerUrl(const QString &serverUrl) const
{
    QString formatted = serverUrl;
    
    // 去除协议前缀
    if (formatted.startsWith("https://"))
    {
        formatted = formatted.mid(8);
    }
    else if (formatted.startsWith("http://"))
    {
        formatted = formatted.mid(7);
    }
    
    // 去除尾部的斜杠
    if (formatted.endsWith("/"))
    {
        formatted = formatted.left(formatted.length() - 1);
    }
    
    return formatted;
}
